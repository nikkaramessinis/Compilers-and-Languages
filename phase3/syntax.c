#include "symboltable.h"
#include <assert.h>

quad* quads=(quad*) 0;
unsigned total=0;
unsigned int currQuad=0;
int count = 0;
unsigned programVarOffset = 0;
unsigned functionLocalOffset = 0;
unsigned formalArgOffset = 0;
unsigned scopeSpaceCounter = 1;


#define EXPAND_SIZE 1024
#define CURR_SIZE (total*sizeof(quad))
#define NEW_SIZE (EXPAND_SIZE*sizeof(quad)+CURR_SIZE)

scopespace_t currscopespace(void){
	if(scopeSpaceCounter == 1)
		return PROGRAM_VAR;
	else
		if(scopeSpaceCounter % 2 == 0)
			return FORMAL_ARG;
		else
			return FUNCTION_LOC;
}

unsigned currscopeoffset(void){
	switch(currscopespace()){
		case PROGRAM_VAR: return programVarOffset;
		case FUNCTION_LOC: return functionLocalOffset;
		case FORMAL_ARG: return formalArgOffset;
		default: assert(0);
	}
}

void incurrscopeoffset(void){
	switch(currscopespace()){
		case PROGRAM_VAR: ++programVarOffset;
		break;
		case FUNCTION_LOC: ++functionLocalOffset;
		break;
		case FORMAL_ARG: ++formalArgOffset;
		break;
		default: assert(0);
	}
}

void enterscopespace(void){
	++scopeSpaceCounter;
}

void exitscopespace(void){
	assert(scopeSpaceCounter > 1);
	--scopeSpaceCounter;
}

void resettemp() { count = 0; }

SymbolTableEntry *new_temp(int scope, int line){
	char *str = malloc(sizeof(4*sizeof(char)));
	sprintf(str, "_t%d", count);
	
	SymbolTableEntry *entry = (SymbolTableEntry *)refersTo(str, scope);
	count++;
	if(entry == NULL){
		if(scope==0){
			entry = (SymbolTableEntry *)insert(true,str,scope,line,GLOBAL);
		}else{
			entry = (SymbolTableEntry *)insert(true,str,scope,line,LOCAL_);
		}
		entry->scope_space = currscopespace();
	  	entry->offset = currscopeoffset();
	  	return entry;
	}else{
		return entry;
	}
	

}

/* ---------------function handling---------------- */
void pushLocalStack(unsigned functionLocalOffset){
	LocalStack *temp = malloc(sizeof(LocalStack));
	LocalStack *temp2 = LocalStackhead;
	temp->functionLocalOffset = functionLocalOffset;
	if(LocalStackhead == NULL){
		LocalStackhead = temp;
		temp->next = NULL;
	}
	else{
		temp->next = temp2;
		LocalStackhead = temp;
	}
}

unsigned popLocalStack()
{
	LocalStack *temp2 = LocalStackhead;
	LocalStackhead = LocalStackhead->next;
	unsigned returnval = temp2->functionLocalOffset;
	free(temp2);
	return returnval;
}

void resetformalargsoffset(void){
	formalArgOffset = 0;
}

void resetfunctionlocalsoffset(void){
	functionLocalOffset = 0;
}

void restorecurrscopeoffset(unsigned n){
	switch(currscopespace()){
		case PROGRAM_VAR: programVarOffset = n; 
		break;
		case FUNCTION_LOC: functionLocalOffset = n;
		break;
		case FORMAL_ARG: formalArgOffset = n;
		break;
		default: assert(0);
	}
}

unsigned nextquadlabel(void){
	return currQuad;
}

void patchlabel(unsigned quadNo, unsigned label){
	assert(quadNo < currQuad);
	quads[quadNo].label = label;
}

void whilepatchlabel(backpatch *head, unsigned label){

	backpatch *tmp = head;

	while(tmp != NULL){

		assert(tmp->label < currQuad);
		quads[tmp->label].label = label;
		tmp = tmp->next;
	}
}

expr *add_front(expr *self){

	expr *temp2 = expr_head;

	if(expr_head == NULL){
		expr_head = self;
		self->next = NULL;
	}else{
		self->next = temp2;
		expr_head = self;
	}
	return expr_head;
 }

/*-----------------------------------------------------------*/

expr *newexpr(expr_t expression){
	expr *tmp = malloc(sizeof(expr));
	memset(tmp, 0, sizeof(expr));
	tmp->type = expression;
	
	return tmp;
}

expr *newexpr_conststring(char* name){
	expr *tmp = malloc(sizeof(expr));
	tmp->type = CONSTSTRING_E;
	tmp->strConst = strdup(name);

	return tmp;
}

/*-----------------while-for handling--------------------*/

void pushLoopCounter(unsigned loop_counter){

	LoopCounterStack *temp = malloc(sizeof(LoopCounterStack));
	LoopCounterStack *temp2 = loopCounterHead;
	temp->loopcounter = loop_counter;
	
	if(loopCounterHead == NULL){
		loopCounterHead = temp;
		temp->next = NULL;
	}
	else{
		temp->next = temp2;
		loopCounterHead = temp;
	}
}

unsigned popLoopCounter()
{
	LoopCounterStack *temp2 = loopCounterHead;
	loopCounterHead = loopCounterHead->next;
	unsigned returnval = temp2->loopcounter;
	free(temp2);
	return returnval;
}

backpatch *pushbackpatchList(unsigned label){

	backpatch *temp = malloc(sizeof(backpatch));
	temp->label = label;
	temp->next = NULL;

	return temp;
}

backpatch *merge(backpatch *stmts_head, backpatch *stmt){

	if(stmts_head==NULL && stmt==NULL){
   		return NULL;
    }
	if(stmt==NULL){
		return stmts_head;
	}

	if(stmts_head == NULL){
		return stmt;
	}else{
		stmt->next = stmts_head;
		stmts_head = stmt;
	}
	return stmts_head;
	
}

void printBackpatch(backpatch *head){
    
    backpatch * temp=head;
    while(temp!=NULL){
        printf("Label:%d\n",temp->label);
        temp=temp->next;
    }
}

/*unsigned popquadNo(backpatch *head){

	backpatch *temp2 = head;
	head = head->next;
	unsigned returnval = temp2->label;
	free(temp2);
	
	return returnval;
	
}*/

/*-------------------------------------------------------*/

void expand(void){
	assert(total == currQuad);
	quad *p = (quad *)malloc(NEW_SIZE);
	if(quads){
		memcpy(p, quads, CURR_SIZE);
		free(quads);
	}
	quads = p;
	total += EXPAND_SIZE;
}

void emit(iopcode iop, expr *result, expr *arg1, expr *arg2, unsigned label, unsigned line){

	if(currQuad == total){
		expand();
	}
	quad *p = quads + currQuad++;
	p->op = iop;
	p->result = result;
	p->arg1 = arg1;
	p->arg2 = arg2;
	p->label = label;
	p->line = line;

}

expr *emit_iftableitem(expr * e, int scope, int line)
{
	if(e->type!=TABLEITEM_E)
		return	e;
	else
	{
		expr *result = newexpr(VAR_E);
		result->sym = new_temp(scope,line);
		emit(TABLEGETELEM,result,e,e->index,-1,line);
		return result;
	}
}


expr *member_item(expr * lvalue,char * name,int scope,int line)
{
	lvalue=emit_iftableitem(lvalue,scope,line);
	expr * item=newexpr(TABLEITEM_E);
	item->sym=lvalue->sym;
	item->index=newexpr_conststring(name);
	return item;
}

expr *newexpr_constnum(double i){
	expr *e = newexpr(CONSTNUM_E);
	e->numConst = i;
	return e;
}

expr *new_expr_constbool(unsigned char c){
	expr *e = newexpr(CONSTBOOL_E);
	e->boolConst = c;
	return e;
}

expr *lvalue_expr(SymbolTableEntry *sym){
	assert(sym);
	expr *e = (expr *)malloc(sizeof(expr));
	memset(e, 0, sizeof(expr));

	e->next = NULL;
	e->sym = sym;

	switch(sym->type_s){
		case VAR_S: e->type = VAR_E; break;
		case PROGRAMFUNC_S : e->type = PROGRAMFUNC_E; break;
		case LIBFUNC_S: e->type = LIBRARYFUNC_E; break;
		default: assert(0);
	}
	return e;
}


void reversedElist(void){

	expr* prev = NULL;
    expr* current = expr_head;
    expr* next = NULL;
    while (current != NULL)
    {
        next  = current->next;  
        current->next = prev;   
        prev = current;
        current = next;
    }
    expr_head = prev;

}

expr *make_call(expr *lvalue, expr *elist_t, int scope, int line){

	expr *func = (expr *)emit_iftableitem(lvalue, scope, line);
	reversedElist();

	expr *tmp = expr_head;
	while(tmp != NULL){
		emit(PARAM, tmp, NULL, NULL, -1, line);
		tmp = tmp->next;
	}
	emit(CALL, func, NULL, NULL, -1, line);
	expr *result = newexpr(VAR_E);
	result->sym = new_temp(scope, line);
	emit(GETRETVAL,result, NULL, NULL, -1, line);

	reversedElist();

	return result;


}

expr *insertExpression(expr *expression){
	expr *tmp = expr_head;
	
	if(expr_head == NULL){
		expr_head = expression;
		expression->next = NULL;
	}else{
		while(tmp->next != NULL){
			tmp = tmp->next;
		}
		tmp->next = expression;
		expression->next = NULL;
	}
	return expression;
}

void insertIndexelem(expr *leftexpr,expr *rightexpr)
{
	indexelem *tmp = indexelem_head;
	
	if(indexelem_head == NULL){
		tmp = malloc(sizeof(indexelem));
		tmp->leftexpr = leftexpr;
		tmp->rightexpr=rightexpr;
		tmp->next = NULL;
		indexelem_head=tmp;

	}else{
		while(tmp->next != NULL){
			tmp = tmp->next;
		}
		tmp->next = malloc(sizeof(indexelem));
		tmp->next->leftexpr = leftexpr;
		tmp->next->rightexpr = rightexpr;
		tmp->next->next = NULL;
	}
	
}
char *stringify_op(iopcode op)
{
	if(op == ASSIGN){return "ASSIGN";}
	else if(op == ADD_IO){return "ADD_IO";}
	else if(op == SUB_IO){return "SUB_IO";}
	else if(op == MUL){return "MUL";}
	else if(op == DIV){return "DIV";}
	else if(op == MOD){return "MOD";}
	else if(op == UMINUS_IO){return "UMINUS_IO";}
	else if(op == AND_IO){return "AND_IO";}
	else if(op == OR_IO){return "OR_IO";}
	else if(op == NOT_IO){return "NOT_IO";}
	else if(op == IF_EQ){return "IF_EQ";}
	else if(op == IF_NOTEQ){return "IF_NOTEQ";}
	else if(op == IF_LESSEQ){return "IF_LESSEQ";}
	else if(op == IF_GREATEREQ){return "IF_GREATEREQ";}
	else if(op == IF_LESS){return "IF_LESS";}
	else if(op == IF_GREATER){return "IF_GREATER";}
	else if(op == JUMP){return "JUMP";}
	else if(op == CALL){return "CALL";}
	else if(op == PARAM){return "PARAM";}
	else if(op == RET){return "RET";}
	else if(op == GETRETVAL){return "GETRETVAL";}
	else if(op == FUNCSTART){return "FUNCSTART";}
	else if(op == FUNCEND){return "FUNCEND";}
	else if(op == TABLECREATE){return "TABLECREATE";}
	else if(op == TABLEGETELEM){return "TABLEGETELEM";}
	else if(op == TABLESETELEM){return "TABLESETELEM";}
}

void checkuminus(expr *e){
	if(e->type == CONSTBOOL_E || e->type == CONSTSTRING_E || e->type == NIL_E
		|| e->type == NEWTABLE_E || e->type == PROGRAMFUNC_E || e->type == LIBRARYFUNC_E || e->type == BOOLEXPR_E){
		printf("Illegal expr to unary -\n");
		exit(0);
	}
}

void printQuads(){
	FILE * fpointer;
	int i;
	fpointer=fopen("quads.txt","w");
	fprintf(fpointer,"%-14s%-14s%-14s%-14s%-14s%-14s\n","#quad",  "opcode" , "result" , "arg1" , "arg2", "label");

	for(i = 0; i < currQuad; i++){
		fprintf(fpointer,"%-14d%-14s",i,stringify_op(quads[i].op));
		if(quads[i].result != NULL){
			if(quads[i].result->strConst!=0){
					fprintf(fpointer,"%-14s", quads[i].result->strConst);
			}else if(quads[i].result->type == CONSTBOOL_E ){
				if(quads[i].result->boolConst==1)
					fprintf(fpointer,"%-14s","\'true\'");
				else
					fprintf(fpointer,"%-14s","\'false\'");
			}else if(quads[i].result->type == CONSTNUM_E){
				fprintf(fpointer,"%-14lf",quads[i].result->numConst);
			}else{
				fprintf(fpointer,"%-14s", quads[i].result->sym->value.varVal->name);
			}
		}else{
			fprintf(fpointer,"              ");
		}
		if(quads[i].arg1 != NULL){
			if(quads[i].arg1->strConst!=0){
					fprintf(fpointer,"%-14s",quads[i].arg1->strConst);
				}else if(quads[i].arg1->type == CONSTBOOL_E ){
					if(quads[i].arg1->boolConst==1)
					fprintf(fpointer,"%-14s","\'true\'");
				else
					fprintf(fpointer,"%-14s","\'false\'");
				}else if(quads[i].arg1->type == CONSTNUM_E){
					fprintf(fpointer,"%-14lf",quads[i].arg1->numConst);
				}else
				{
					fprintf(fpointer,"%-14s", quads[i].arg1->sym->value.varVal->name);
				}
		}else
			fprintf(fpointer,"              ");
		if(quads[i].arg2 != NULL){
			if(quads[i].arg2->strConst!=0 && quads[i].arg2->type == CONSTSTRING_E){
					fprintf(fpointer,"%-14s",quads[i].arg2->strConst);
				}else if(quads[i].arg2->type == CONSTBOOL_E ){
					if(quads[i].arg2->boolConst==1)
					fprintf(fpointer,"%-14s","\'true\'");
				else
					fprintf(fpointer,"%-14s","\'false\'");
				}else if(quads[i].arg2->type == CONSTNUM_E){

					fprintf(fpointer,"%-14lf",quads[i].arg2->numConst);
				}else{
					fprintf(fpointer,"%-14s",quads[i].arg2->sym->value.varVal->name);
				}

		}else
			fprintf(fpointer,"              ");

	if(quads[i].label!=-1){
		fprintf(fpointer,"%-14d", quads[i].label);
	}
		fprintf(fpointer,"\n");
	}


	fclose(fpointer);


}
