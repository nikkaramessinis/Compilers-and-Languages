#include <assert.h>
#include "symboltable.h"
#include "terminal_code.h"
#include "vm.h"


unsigned ij_total = 0;
unsigned global = 0;

double *numConsts;
unsigned totalNumConsts = 0;
char **stringConsts;
unsigned totalStringConsts = 0;
char **namedLibfuncs;
unsigned totalNamedLibfuncs = 0;
userfunc *userFuncs;
unsigned totalUserFuncs = 0;

unsigned total_instructions = 0;
unsigned currInstruction = 0;
instruction* instructions = (instruction*) 0;

unsigned currprocessedquadNo = 0;

extern quad* quads;
extern currQuad;

#define EXPAND_SIZE_I 1024
#define CURR_SIZE_I (total_instructions * sizeof(instruction))
#define NEW_SIZE_I (EXPAND_SIZE_I * sizeof(instruction)+CURR_SIZE_I)


char *stringify_vmarg_t(vmarg_t type)
{	
	if(type == LABEL_A){return "LABEL";}
	else if(type == GLOBAL_A){return "GLOBAL";}
	else if(type == FORMAL_A){return "FORMAL";}
	else if(type == LOCAL_A){return "LOCAL";}
	else if(type == NUMBER_A){return "NUMBER";}
	else if(type == STRING_A){return "STRING";}
	else if(type == BOOL_A){return "BOOL";}
	else if(type == NIL_A){return "NIL";}
	else if(type == USERFUNC_A){return "USERFUNC";}
	else if(type == LIBFUNC_A){return "LIBFUNC";}
	else if(type == RETVAL_A){return "RETVAL";}
	else if(type == RETVAL_A){return "UNDEF";}
	else{
		assert(0);
	}
    
} 
char *stringify_vmopcode(vmopcode code)
{
	if(code == ASSIGN_V){return "ASSIGN";}
	else if(code == ADD_V){return "ADD";}
	else if(code == SUB_V){return "SUB";}
	else if(code == MUL_V){return "MUL";}
	else if(code == DIV_V){return "DIV";}
	else if(code == MOD_V){return "MOD";}
	else if(code == UMINUS_V){return "UMINUS";}
	else if(code == AND_V){return "AND";}
	else if(code == OR_V){return "OR";}
	else if(code == NOT_V){return "NOT";}
	else if(code == JUMP_V){return "JUMP";}
	else if(code == JEQ_V){return "JEQ";}
	else if(code == JNE_V){return "JNE";}
	else if(code == JLE_V){return "JLE";}
	else if(code == JGE_V){return "JGE";}
	else if(code == JLT_V){return "JLT";}
	else if(code == JGT_V){return "JGT";}
	else if(code == CALL_V){return "CALL";}
	else if(code == PUSHARG_V){return "PUSHARG";}
	else if(code == FUNCENTER_V){return "FUNCENTER";}
	else if(code == FUNCEXIT_V){return "FUNCEXIT";}
	else if(code == NEWTABLE_V){return "NEWTABLE";}
	else if(code == TABLEGETELEM_V){return "TABLEGETELEM";}
	else if(code == TABLESETELEM_V){return "TABLESETELEM";}
	else if(code == NOP_V){return "NOP_V";}
	else assert(0);
}


/* Automatic garbage collection for tables when
	reference counter gets zero
*/

unsigned consts_newstring(char *s){
	unsigned i;
	for(i = 0; i < totalStringConsts; i++){
		if(strcmp(stringConsts[i], s) == 0)
			return i;
	}
	unsigned length = strlen(s)+1;
	stringConsts = realloc(stringConsts, (totalStringConsts + 1) * length * sizeof(char *));
	stringConsts[totalStringConsts] = malloc(length * sizeof(char));
	stringConsts[totalStringConsts] = s;
	totalStringConsts++;
	
	return totalStringConsts - 1;
}

unsigned consts_newnumber(double n){
	unsigned i;
	for(i = 0; i < totalNumConsts; i++){
		if(numConsts[i] == n)
			return i;
	}
	numConsts = realloc(numConsts, (totalNumConsts + 1) * sizeof(double));
	numConsts[totalNumConsts] = n;
	totalNumConsts++;
	
	return totalNumConsts - 1;
}
unsigned libfuncs_newused(const char *s){
	unsigned i;
	for(i = 0; i < totalNamedLibfuncs; i++){
		if(strcmp(namedLibfuncs[i], s) == 0){
			return i;
		}
	}
	unsigned length = strlen(s);
	namedLibfuncs = realloc(namedLibfuncs, (totalNamedLibfuncs + 1) * length * sizeof(char *));
	namedLibfuncs[totalNamedLibfuncs] = malloc(length * sizeof(char));
	namedLibfuncs[totalNamedLibfuncs] = strdup(s);
	totalNamedLibfuncs++;
	
	return totalNamedLibfuncs - 1;

}
unsigned userfuncs_newfunc(SymbolTableEntry *sym){
	unsigned i;
	for(i = 0; i < totalUserFuncs; i++){
		if(strcmp(userFuncs[i].id, sym->value.funcVal->name) == 0 && userFuncs[i].userfunc_scope == sym->value.funcVal->scope){
			//return i;
			return userFuncs[i].address ;
		}
	}
	userFuncs = realloc(userFuncs, (totalUserFuncs + 1) * sizeof(userfunc));
	userFuncs[totalUserFuncs].address = sym->value.funcVal->iaddress;
	userFuncs[totalUserFuncs].localSize = sym->value.funcVal->totallocals;
	userFuncs[totalUserFuncs].id = strdup(sym->value.funcVal->name);
	userFuncs[totalUserFuncs].userfunc_scope = sym->value.funcVal->scope;
	totalUserFuncs++;
	
	//return totalUserFuncs - 1;
	return userFuncs[totalUserFuncs - 1].address;
}

void make_operand(expr *e, vmarg *arg){
	if(e != NULL){
		switch(e->type){
			case VAR_E: { 
				assert(e->sym);
				arg->val = e->sym->offset;
				switch(e->sym->scope_space){
					case PROGRAM_VAR: {
						arg->type = GLOBAL_A;
						global++;
						break;
					}
					case FUNCTION_LOC: arg->type = LOCAL_A; break;
					case FORMAL_ARG: arg->type = FORMAL_A; break;
					default: assert(0);
				}
				break;
			}
			case ASSIGNEXPR_E: { 
				assert(e->sym);
				arg->val = e->sym->offset;
				switch(e->sym->scope_space){
					case PROGRAM_VAR: {
						arg->type = GLOBAL_A;
						global++;
						break;
					}
					case FUNCTION_LOC: arg->type = LOCAL_A; break;
					case FORMAL_ARG: arg->type = FORMAL_A; break;
					default: assert(0);
				}
				break;
			}
			case TABLEITEM_E: { 
				assert(e->sym);
				arg->val = e->sym->offset;
				switch(e->sym->scope_space){
					case PROGRAM_VAR: {
						arg->type = GLOBAL_A;
						global++;
						break;
					}
					case FUNCTION_LOC: arg->type = LOCAL_A; break;
					case FORMAL_ARG: arg->type = FORMAL_A; break;
					default: assert(0);
				}
				break;
			}
			case ARITHEXPR_E: {
				
				assert(e->sym);
				arg->val = e->sym->offset;
				switch(e->sym->scope_space){
					case PROGRAM_VAR: {
						arg->type = GLOBAL_A;
						global++;
						break;
					}
					case FUNCTION_LOC: arg->type = LOCAL_A; break;
					case FORMAL_ARG: arg->type = FORMAL_A; break;
					default: assert(0);
				}
				break;
			}
			case BOOLEXPR_E: {
				assert(e->sym);
				arg->val = e->sym->offset;
				switch(e->sym->scope_space){
					case PROGRAM_VAR: {
						arg->type = GLOBAL_A;
						global++;
						break;
					}
					case FUNCTION_LOC: arg->type = LOCAL_A; break;
					case FORMAL_ARG: arg->type = FORMAL_A; break;
					default: assert(0);
				}
				break;
			}
			case NEWTABLE_E:{ 
				assert(e->sym);
				arg->val = e->sym->offset;
				switch(e->sym->scope_space){
					case PROGRAM_VAR: {
						arg->type = GLOBAL_A;
						global++; 
						break;
					}
					case FUNCTION_LOC: arg->type = LOCAL_A; break;
					case FORMAL_ARG: arg->type = FORMAL_A; break;
					default: assert(0);
				}
				break;
			}
			case CONSTBOOL_E: {
				arg->val = e->boolConst;
				arg->type = BOOL_A;

				break;
			}
			case CONSTSTRING_E: {
				arg->val = consts_newstring(e->strConst);
				arg->type = STRING_A;
				break;
			}
			case CONSTNUM_E: {
				arg->val = consts_newnumber(e->numConst);
				arg->type = NUMBER_A;
				break;
			}
			case NIL_E: {
				arg->val = consts_newstring(e->strConst);
				arg->type = NIL_A;
				break; 
				}
			case PROGRAMFUNC_E: { 
				arg->type = USERFUNC_A;
				//arg->val = e->sym->value.funcVal->iaddress;
				arg->val = userfuncs_newfunc(e->sym);
				break;
			}
			case LIBRARYFUNC_E: {
				arg->type = LIBFUNC_A;
				arg->val = libfuncs_newused(e->sym->value.funcVal->name);
				break;
			}
			default: assert(0);
		}
	}else{
		arg->type = UNDEF_A;
		arg->val = -1;
	}
}

/*	Helper functions to produce common arguments
	for generated instructions like 1,0,true,false
	and function return values
*/
void make_numberoperand(vmarg *arg, double val){
	arg->val = consts_newnumber(val);
	arg->type = NUMBER_A;
}

void make_booloperand(vmarg *arg, unsigned val){
	arg->val = val;
	arg->type = BOOL_A;
}

void make_retvaloperand(vmarg *arg){
	arg->type = RETVAL_A;
	arg->val = 0;
}


unsigned nextinstructionlabel(void){
	return currInstruction;
}


void patch_incomplete_jumps(){
	incomplete_jump *tmp = ij_head;
	
	while(tmp != NULL){
		if(tmp->iaddress == currQuad){
			instructions[tmp->instrNo].result.val = currInstruction;
		}else{
			instructions[tmp->instrNo].result.val = quads[tmp->iaddress].taddress;
		}
		

		tmp = tmp->next;
	}
	
}	

void expand_instruction(void){
	assert(total_instructions == currInstruction);
	instruction *t = (instruction *)malloc(NEW_SIZE_I);
	if(t){
		memcpy(t, instructions, CURR_SIZE_I);
		free(instructions);
	}
	instructions = t;
	total_instructions += EXPAND_SIZE_I;
}

void emit_instruction(instruction t){

	if(currInstruction == total_instructions){
		expand_instruction();
	}
	//instruction *p = instructions + currInstruction++;
	//p = &t;
	instructions[currInstruction++] = t;
}



void generate(unsigned op, quad q){
	instruction t;
	t.opcode = op;
	make_operand(q.arg1, &t.arg1);
	make_operand(q.arg2, &t.arg2);
	make_operand(q.result, &t.result);
	quads[currprocessedquadNo].taddress = nextinstructionlabel();
	emit_instruction(t);	
}

void generate_ADD(quad q){
	generate(ADD_V, q);
}

void generate_SUB(quad q){
	generate(SUB_V, q);
}

void generate_MUL(quad q){
	generate(MUL_V, q);
}

void generate_DIV(quad q){
	generate(DIV_V, q);
}

void generate_MOD(quad q){
	generate(MOD_V, q);
}

void generate_NEWTABLE(quad q){ 
	generate(NEWTABLE_V, q);
}

void generate_TABLEGETELEM(quad q){
	generate(TABLEGETELEM_V, q);
}

void generate_TABLESETELEM(quad q){
	generate(TABLESETELEM_V, q);
}

void generate_ASSIGN(quad q){
	generate(ASSIGN_V, q);
}

void generate_NOP(){
	instruction t;
	t.opcode = NOP_V;
	emit_instruction(t);	
}

void add_incomplete_jump(unsigned instrNo, unsigned iaddress){
	
	incomplete_jump *j = malloc(sizeof(incomplete_jump));
	j->instrNo = instrNo;
	j->iaddress = iaddress;
	j->next = NULL;
	
	if(ij_head == NULL){
		ij_head = j;	
	}else{
		incomplete_jump *tmp = ij_head;
		while(tmp->next != NULL){
			tmp = tmp->next;
		}
		tmp->next = j;
	}
}

unsigned currprocessedquad(void){
	return currprocessedquadNo;
}

void generate_relational(unsigned op, quad q){
	instruction t;
	t.opcode = op;
	make_operand(q.arg1, &t.arg1);
	make_operand(q.arg2, &t.arg2);

	t.result.type = LABEL_A;

	if(q.label < currprocessedquad()){
		t.result.val = quads[q.label].taddress;
	}else{
		add_incomplete_jump(nextinstructionlabel(), q.label);
	}
	quads[currprocessedquadNo].taddress = nextinstructionlabel();
	emit_instruction(t);
}

void generate_JUMP(quad q){ 
	generate_relational(JUMP_V, q);
}

void generate_IF_EQ(quad q){
	generate_relational(JEQ_V, q);
}

void generate_IF_NOTEQ(quad q){
	generate_relational(JNE_V, q);
}

void generate_IF_GREATER(quad q){
	generate_relational(JGT_V, q);
}

void generate_IF_GREATEREQ(quad q){
	generate_relational(JGE_V, q);
}

void generate_IF_LESS(quad q){
	generate_relational(JLT_V, q);
}

void generate_IF_LESSEQ(quad q){
	generate_relational(JLE_V, q);
}

void reset_operand(vmarg *arg){
	arg->val = -1;
}

void generate_NOT(quad q){
	quads[currprocessedquadNo].taddress = nextinstructionlabel();
	instruction t;
	t.opcode = JEQ_V;
	make_operand(q.arg1, &t.arg1);
	make_booloperand(&t.arg2, 0);
	t.result.type = LABEL_A;
	t.result.val = nextinstructionlabel() + 3;
	emit_instruction(t);	

	t.opcode = ASSIGN_V;
	make_booloperand(&t.arg1, 0);
	reset_operand(&t.arg2); 
	make_operand(q.result, &t.result);
	emit_instruction(t);	

	t.opcode = JUMP_V;
	reset_operand(&t.arg1);
	reset_operand(&t.arg2);
	t.result.type = LABEL_A;
	t.result.val = nextinstructionlabel() + 2;
	emit_instruction(t); 	

	t.opcode = ASSIGN_V;
	make_booloperand(&t.arg1, 1);
	reset_operand(&t.arg2);
	make_operand(q.result, &t.result);
	emit_instruction(t);	
}

void generate_OR(quad q){
	quads[currprocessedquadNo].taddress = nextinstructionlabel();
	instruction t;

	t.opcode = JEQ_V;
	make_operand(q.arg1, &t.arg1);
	make_booloperand(&t.arg2, 1);
	t.result.type = LABEL_A;
	t.result.val = nextinstructionlabel() + 4;
	emit_instruction(t);	

	make_operand(q.arg2, &t.arg1);
	t.result.val = nextinstructionlabel() + 3;
	emit_instruction(t);	

	t.opcode = ASSIGN_V;
	make_booloperand(&t.arg1, 0);
	reset_operand(&t.arg2);
	make_operand(q.result, &t.result);
	emit_instruction(t); 	

	t.opcode = JUMP_V;
	reset_operand(&t.arg1);
	reset_operand(&t.arg2);
	t.result.type = LABEL_A;
	t.result.val = nextinstructionlabel() + 2;
	emit_instruction(t);	

	t.opcode = ASSIGN_V;
	make_booloperand(&t.arg1, 1);
	reset_operand(&t.arg2);
	make_operand(q.result, &t.result);
	emit_instruction(t);	
}

void generate_AND(quad q){
	quads[currprocessedquadNo].taddress = nextinstructionlabel();
	instruction t;

	t.opcode = JEQ_V;
	make_operand(q.arg1, &t.arg1);
	make_booloperand(&t.arg2, 0);
	t.result.type = LABEL_A;
	t.result.val = nextinstructionlabel() + 4;
	emit_instruction(t);

	make_operand(q.arg2, &t.arg1);
	t.result.val = nextinstructionlabel() + 3;
	emit_instruction(t);

	t.opcode = ASSIGN_V;
	make_booloperand(&t.arg1, 1);
	reset_operand(&t.arg2);
	make_operand(q.result, &t.result);
	emit_instruction(t);

	t.opcode = JUMP_V;
	reset_operand(&t.arg1);
	reset_operand(&t.arg2);
	t.result.type = LABEL_A;
	t.result.val = nextinstructionlabel() + 2;
	emit_instruction(t);

	t.opcode = ASSIGN_V;
	make_booloperand(&t.arg1, 0);
	reset_operand(&t.arg2);
	make_operand(q.result, &t.result);
	emit_instruction(t);
}

void generate_PARAM(quad q){ 
	quads[currprocessedquadNo].taddress = nextinstructionlabel();
	
	instruction t;
	t.opcode = PUSHARG_V;
	make_operand(q.result, &t.result);
	reset_operand(&t.arg1);
	reset_operand(&t.arg2);
	emit_instruction(t);	
}

void generate_CALL(quad q){
	quads[currprocessedquadNo].taddress = nextinstructionlabel();
	instruction t;
	t.opcode = CALL_V;
	//make_operand(q.arg1, &t.arg1);
	reset_operand(&t.arg1);
	reset_operand(&t.arg2);
//	make_operand(q.arg2, &t.arg2);
	make_operand(q.result, &t.result);
	emit_instruction(t);	
}

void generate_GETRETVAL(quad q){

	quads[currprocessedquadNo].taddress = nextinstructionlabel();
	instruction t;
	t.opcode = ASSIGN_V;
	make_operand(q.result, &t.result);
	make_retvaloperand(&t.arg1);
	//reset_operand(&t.arg2);
	emit_instruction(t);	
}

void pushFuncStack(FuncStack *fs){

	FuncStack *temp2 = funcstack_head;
	
	if(funcstack_head == NULL){
		funcstack_head = fs;
		fs->next = NULL;
	}
	else{
		fs->next = temp2;
		funcstack_head = fs;
	}
}

FuncStack *usersfunctions_add(const char *id, unsigned taddress, unsigned totallocals){
	FuncStack *fs = malloc(sizeof(FuncStack));
	fs->id = id;
	fs->taddress = taddress;
	fs->totallocals = totallocals;
	fs->retlist = NULL;
	fs->next = NULL;

	return fs;
}

void generate_FUNCSTART(quad q){ 
	SymbolTableEntry *f;
	f = q.result->sym;
	f->value.funcVal->iaddress = nextinstructionlabel();
	quads[currprocessedquadNo].taddress = nextinstructionlabel();

	FuncStack *fs = usersfunctions_add(f->value.funcVal->name, f->value.funcVal->iaddress, f->value.funcVal->totallocals);
	pushFuncStack(fs);
	
	instruction t;
	t.opcode = FUNCENTER_V;
	reset_operand(&t.arg1);
	reset_operand(&t.arg2);
	make_operand(q.result, &t.result);
	emit_instruction(t);
}

FuncStack *topFunc(){
	return funcstack_head;
}

ReturnList *append(ReturnList *ret, unsigned instr_label){
	ReturnList *r = malloc(sizeof(ReturnList));
	
	r->target_label = instr_label;
	
	r->next=NULL;
	ReturnList *tmp = ret;
	if(ret == NULL){
		ret = r;
		ret->next=NULL;
	}else{
		while(tmp->next != NULL){
			tmp = tmp->next;
		}
		tmp->next = r;
		
	}
	return ret;
}

void generate_RETURN(quad q){
	quads[currprocessedquadNo].taddress = nextinstructionlabel();
	instruction t;
	t.opcode = ASSIGN_V;
	make_retvaloperand(&t.result);
	make_operand(q.result, &t.arg1);
	reset_operand(&t.arg2);
	emit_instruction(t); 

	FuncStack *f = topFunc();
	f->retlist = append(f->retlist, nextinstructionlabel());

	t.opcode = JUMP_V;
	reset_operand(&t.arg1);
	reset_operand(&t.arg2);
	t.result.type = LABEL_A;
	emit_instruction(t); 	
}

FuncStack *pop(){

	FuncStack *temp2 = funcstack_head;
	funcstack_head = funcstack_head->next;
	temp2->next = NULL;

	return temp2;
}

void backpatch_return(ReturnList *ret, unsigned label){

	ReturnList *tmp = ret;

	while(tmp != NULL){
		assert(tmp->target_label < currInstruction);
		instructions[tmp->target_label].result.type = LABEL_A;
		instructions[tmp->target_label].result.val = label;
		tmp = tmp->next;
	}
}

void generate_FUNCEND(quad q){ 
	FuncStack *f = pop();
	backpatch_return(f->retlist, nextinstructionlabel());
	quads[currprocessedquadNo].taddress = nextinstructionlabel();

	instruction t;
	t.opcode = FUNCEXIT_V;
	reset_operand(&t.arg1);
	reset_operand(&t.arg2);
	make_operand(q.result, &t.result);
	emit_instruction(t);
}

void generate_UMINUS(quad q){
	instruction t;
	t.opcode = MUL_V;
	//reset_operand(&t.result);
	make_operand(q.result, &t.result);
	make_operand(q.arg1, &t.arg1);
	t.arg2.val = consts_newnumber(-1);
	t.arg2.type = NUMBER_A;
	emit_instruction(t);

}

typedef void (*generator_func_t)(quad);

generator_func_t generators[] = {
	generate_ASSIGN,
	generate_ADD,
	generate_SUB,
	generate_MUL,
	generate_DIV,
	generate_MOD,
	generate_UMINUS,
	generate_AND,
	generate_OR,
	generate_NOT,
	generate_IF_EQ,
    generate_IF_NOTEQ,
	generate_IF_LESSEQ,
	generate_IF_GREATEREQ,
	generate_IF_LESS,
	generate_IF_GREATER,
	generate_JUMP,
	generate_CALL,
	generate_PARAM,
	generate_RETURN,
	generate_GETRETVAL,
	generate_FUNCSTART,
	generate_FUNCEND,
	generate_NEWTABLE,
    generate_TABLEGETELEM,
    generate_TABLESETELEM,
    generate_NOP
 	
};

void generate_all(void){
	for(currprocessedquadNo = 0; currprocessedquadNo < currQuad; ++currprocessedquadNo){	
		(*generators[quads[currprocessedquadNo].op])(quads[currprocessedquadNo]);
	}
	patch_incomplete_jumps();

}

FILE *ipointer,*rpointer;

void magicnumber(){
	fprintf(ipointer, "magicnumber	%u\n", 340200501);
	fprintf(rpointer, "magicnumber	%u\n", 340200501);
}

void strings(){
	if(totalStringConsts != 0){
		unsigned i;
		unsigned j;
		fprintf(ipointer, "\nstrings	total: %u\n", totalStringConsts);
		fprintf(rpointer, "\nstrings	total: %u\n", totalStringConsts);
		for(i = 0; i < totalStringConsts; i++){
			char *tmp=stringConsts[i];
				for(j=0;tmp[j]!='\0';j++){	
					if(tmp[j]=='\n')
					{
					
						tmp[j]='~';
						
						}
					}
			fprintf(ipointer, "%s	size: %u\n", stringConsts[i], strlen(stringConsts[i])+1);
			fprintf(rpointer, "%s	size: %u\n", stringConsts[i], strlen(stringConsts[i])+1);
		}
	}
}

void numbers(){
	if(totalNumConsts != 0){
		unsigned i;
		fprintf(ipointer, "\nnumbers	total: %u\n", totalNumConsts);
		fprintf(rpointer, "\nnumbers	total: %u\n", totalNumConsts);
		for(i = 0; i < totalNumConsts; i++){
			fprintf(ipointer, "%lf\n", numConsts[i]);
			fprintf(rpointer, "%lf\n", numConsts[i]);
		}
	}
}

void userfunctions(){
	if(totalUserFuncs != 0){
		unsigned i;
		fprintf(ipointer, "\nuserfunctions	total: %u\n", totalUserFuncs);
		fprintf(rpointer, "\nuserfunctions	total: %u\n", totalUserFuncs);
		for(i = 0; i < totalUserFuncs; i++){
			fprintf(ipointer, "address %u\n", userFuncs[i].address);
			fprintf(rpointer, "address %u\n", userFuncs[i].address);
			fprintf(ipointer, "local size %u\n", userFuncs[i].localSize);
			fprintf(rpointer, "local size %u\n", userFuncs[i].localSize);
			fprintf(ipointer, "id %s\n", userFuncs[i].id);
			fprintf(rpointer, "id %s\n", userFuncs[i].id);
		}
	}
}

void libfunctions(){
	if(totalNamedLibfuncs != 0){
		unsigned i;
		fprintf(ipointer, "\nlibfunctions	total: %u\n", totalNamedLibfuncs);
		fprintf(rpointer, "\nlibfunctions	total: %u\n", totalNamedLibfuncs);
		for(i = 0; i < totalNamedLibfuncs; i++){
			fprintf(ipointer, "%s 	size: %u\n", namedLibfuncs[i], strlen(namedLibfuncs[i]) + 1);
			fprintf(rpointer, "%s 	size: %u\n", namedLibfuncs[i], strlen(namedLibfuncs[i]) + 1);
		}
	}
}

void arrays(){
	strings();
	numbers();
	userfunctions();
	libfunctions();
}

void final_code(){
	int curinstrNo;
	fprintf(ipointer, "\nglobalVariables: %u\n", global_variables());
	fprintf(ipointer, "\ntotalInstructions: %u\n", currInstruction);
	fprintf(ipointer,"\n%-14s%-14s%-14s%-14s\n", "opcode" , "result" , "arg1" , "arg2");
	for(curinstrNo = 0; curinstrNo < currInstruction; ++curinstrNo){	
			fprintf(ipointer,"%-14d", instructions[curinstrNo].opcode);
			if(instructions[curinstrNo].result.val != -1){
				fprintf(ipointer,"(%-7d)%-7u", instructions[curinstrNo].result.type, instructions[curinstrNo].result.val);
			}else{
				fprintf(ipointer,"(%-7d)%-7d",-1,-1);
			}
			if(instructions[curinstrNo].arg1.val != -1){
				fprintf(ipointer,"(%-7d)%-7u", instructions[curinstrNo].arg1.type, instructions[curinstrNo].arg1.val);
			}else{
				fprintf(ipointer,"(%-7d)%-7d", -1,-1);
			}
			if(instructions[curinstrNo].arg2.val != -1){
				fprintf(ipointer,"(%-7d)%-7u", instructions[curinstrNo].arg2.type, instructions[curinstrNo].arg2.val);
			}else{
				fprintf(ipointer,"(%-7d)%-7d",-1, -1);
			}
			fprintf(ipointer,"\n");
		}
		fclose(ipointer);
}
 
void final_code1(){
	int curinstrNo;
	fprintf(rpointer, "\ngloabalVariables: %u\n", global_variables());
	fprintf(rpointer, "\ntotalInstructions: %u\n", currInstruction);
	fprintf(rpointer,"\n%-14s%-14s%-14s%-14s%-14s\n","#No" ,"opcode" , "result" , "arg1" , "arg2");
	for(curinstrNo = 0; curinstrNo < currInstruction; ++curinstrNo){
			fprintf(rpointer,"%-14d",curinstrNo) ;
			fprintf(rpointer,"%-14s", stringify_vmopcode(instructions[curinstrNo].opcode));

			if(instructions[curinstrNo].result.val != -1){
				fprintf(rpointer,"(%-7s)%-7u", stringify_vmarg_t(instructions[curinstrNo].result.type), instructions[curinstrNo].result.val);
			}else{
				fprintf(rpointer,"              ");
			}
			if(instructions[curinstrNo].arg1.val != -1){
				fprintf(rpointer,"(%-7s)%-7u", stringify_vmarg_t(instructions[curinstrNo].arg1.type), instructions[curinstrNo].arg1.val);
			}else{
				fprintf(rpointer,"              ");
			}
			if(instructions[curinstrNo].arg2.val != -1){
				fprintf(rpointer,"(%-7s)%-7u", stringify_vmarg_t(instructions[curinstrNo].arg2.type), instructions[curinstrNo].arg2.val);
			}else{
				fprintf(rpointer,"              ");
			}
			fprintf(rpointer,"\n");
		}
		fclose(rpointer);
}

void avmbinaryfile(){
	magicnumber();
	arrays();
	final_code();
	final_code1();
}

void create_txt(){
	int i;
	ipointer = fopen("instructions.abc","w");
	rpointer=fopen("instructions.txt","w");
	avmbinaryfile();

}