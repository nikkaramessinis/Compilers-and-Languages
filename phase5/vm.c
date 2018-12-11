#include <assert.h>
#include <stdarg.h>
#include <math.h>
#include "symboltable.h"
#include "vm.h"

#define AVM_NUMACTUALS_OFFSET +4
#define AVM_SAVEDPC_OFFSET +3
#define AVM_SAVEDTOP_OFFSET +2
#define AVM_SAVEDTOPSP_OFFSET +1


#define execute_add execute_arithmetic
#define execute_sub execute_arithmetic
#define execute_mul execute_arithmetic
#define execute_div execute_arithmetic
#define execute_mod execute_arithmetic


double *numConsts;
unsigned totalNumConsts = 0;
char **stringConsts;
unsigned totalStringConsts = 0;
char **namedLibfuncs;
unsigned totalNamedLibfuncs = 0;
userfunc *userFuncs;
unsigned totalUserFuncs = 0;
unsigned tablecounter=0;
unsigned char executionFinished = 0;
unsigned pc = 0;
instruction* code = (instruction*) 0;
unsigned currLine = 0;
unsigned codeSize = 0;
unsigned totalActuals = 0;
unsigned globals = 0;

 
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

static void avm_initstack(void){
	unsigned i;
	for(i = 0; i < AVM_STACKSIZE; ++i){
		AVM_WIPEOUT(stack[i]);
		stack[i].type = UNDEF_M;
	}
}

void avm_tablebucketsinit(avm_table_bucket **p){
	unsigned i;
	for(i = 0; i < AVM_TABLE_HASHSIZE; ++i)
		p[i] = (avm_table_bucket *)0;
}


double consts_getnumber(unsigned index){
	return numConsts[index];
}

char* consts_getstring(unsigned index){
	return stringConsts[index];
}

char* libfuncs_getused(unsigned index){

	return namedLibfuncs[index];
}

avm_memcell *avm_translate_operand(vmarg * arg,avm_memcell *reg)
{
	switch(arg->type){
		case GLOBAL_A : return &stack[AVM_STACKSIZE - 1 - arg->val];
		case LOCAL_A : return &stack[topsp - arg->val];
		case FORMAL_A : return &stack[topsp + AVM_STACKENV_SIZE + 1 + arg->val];
		case RETVAL_A: return &retval;
		case NUMBER_A: {
			reg->type = NUMBER_M;
			reg->data.numVal = consts_getnumber(arg->val);
			return reg;
		}
		case STRING_A:{
			reg->type=STRING_M;
			reg->data.strVal=strdup(consts_getstring(arg->val));
			return reg;
			
		}
		case BOOL_A:{
			reg->type=BOOL_M;
			reg->data.boolVal=arg->val;
			return reg;
		}
		case NIL_A:{ 
			reg->type = NIL_M;
			return reg;
		}
		case USERFUNC_A: {
			reg->type=USERFUNC_M;
			reg->data.funcVal=arg->val;
			return reg;
		}
		case LIBFUNC_A: { 
			reg->type=LIBFUNC_M;
			reg->data.libfuncVal=libfuncs_getused(arg->val);
			return reg;
		}
		default:assert(0);
	}
}
 void execute_assign(instruction *t){
	avm_memcell* lv = avm_translate_operand(&t->result,(avm_memcell*)0);
	avm_memcell* rv = avm_translate_operand(&t->arg1,&ax);
	assert(lv && (&stack[AVM_STACKSIZE - 1] >= lv && lv > &stack[top] || lv == &retval ));
	assert(rv);
	avm_assign(lv, rv);}
 
 
 void execute_add(instruction *t);
 void execute_sub(instruction *t);
 void execute_mul(instruction *t);
 void execute_div(instruction *t);
 void execute_mod(instruction *t);
 void execute_uminus(instruction *t){}
 
 void execute_and(instruction *t){}
 void execute_or(instruction *t){}
 void execute_not(instruction *t){}
 

 
char* number_tostring(avm_memcell* n){ 

	char buffer[50];
	sprintf(buffer, "%lf", n->data.numVal);

	return  strdup(buffer);
}

char* string_tostring(avm_memcell* n){
	char *str=strdup( n->data.strVal);
	return str;
}
char* bool_tostring(avm_memcell*  n){
	//char *buffer = malloc(sizeof(char));
	//sprintf(buffer, "%u", n->data.boolVal);
	if( n->data.boolVal==1){
		return strdup("true");
	//	return buffer;
	}else{
		return strdup("false");
	}
	//return 0;
}
char* userfunc_tostring(avm_memcell* n){
	char buffer [150];
	sprintf(buffer, "%u", n->data.funcVal);

	return strdup(buffer);
}

char* libfunc_tostring(avm_memcell* n){
	return n->data.libfuncVal;
}

char* nil_tostring(avm_memcell* n){ 
	return "NIL";
}

char* undef_tostring(avm_memcell* n){ 
	return "UNDEFINED";
} 
 void avm_tableincrefcounter(avm_table *t);
 
char* table_tostring(avm_memcell* n){

	
	char * tmp =NULL; 
	int size=10 * sizeof(char);
	tmp=realloc(tmp, 10 * sizeof(char ));
	int k=0;
	strcpy(tmp," [");
	k++;
	int found=0; 
	unsigned i=0;
	
	while(i<AVM_TABLE_HASHSIZE && found<n->data.tableVal->total){
	
		if(n->data.tableVal->strIndexed[i]!=NULL){
			found++;
			size+=1*sizeof(char);
			tmp=realloc(tmp,size);
			strcat(tmp,"{");
			size+=strlen(n->data.tableVal->strIndexed[i]->key.data.strVal)*sizeof(char);
			tmp=realloc(tmp,size);
			 strcat(tmp,n->data.tableVal->strIndexed[i]->key.data.strVal);
				size+=1*sizeof(char);
			tmp=realloc(tmp,size);
			strcat(tmp,":");	
	 if(n->data.tableVal->strIndexed[i]->value.type==NUMBER_M){
		 char *num= number_tostring(&(n->data.tableVal->strIndexed[i]->value));
		 size+=strlen(num)*sizeof(char);
		  tmp=realloc(tmp,size);
		 strcat(tmp,num);
	 }
	 else if(n->data.tableVal->strIndexed[i]->value.type==STRING_M){
		  char *num= string_tostring(&(n->data.tableVal->strIndexed[i]->value));
		 size+=strlen(num)*sizeof(char);
		  tmp=realloc(tmp,size);
		 strcat(tmp,num);
	 }
	 else if(n->data.tableVal->strIndexed[i]->value.type==BOOL_M){ 
	 char *num= bool_tostring(&(n->data.tableVal->strIndexed[i]->value));
		 size+=strlen(num)*sizeof(char);
		  tmp=realloc(tmp,size);
		 strcat(tmp,num);}
		 //itan se sxolio
	else if(n->data.tableVal->strIndexed[i]->value.type==TABLE_M){
		  char *num= table_tostring(&(n->data.tableVal->strIndexed[i]->value));
		 size+=strlen(num)*sizeof(char);
	 tmp=realloc(tmp,size);
	 strcat(tmp,num);
	 }
	 //-----------------------------
	 else if(n->data.tableVal->strIndexed[i]->value.type==USERFUNC_M){
		  char *num= userfunc_tostring(&(n->data.tableVal->strIndexed[i]->value));
		 size+=strlen(num)*sizeof(char);
		  tmp=realloc(tmp,size);
		 strcat(tmp,num);
	 }
	 else if(n->data.tableVal->strIndexed[i]->value.type==LIBFUNC_M){
		  char *num= libfunc_tostring(&(n->data.tableVal->strIndexed[i]->value));
		 size+=strlen(num)*sizeof(char);
		  tmp=realloc(tmp,size);
		 strcat(tmp,num);
	 }
	 else if(n->data.tableVal->strIndexed[i]->value.type==NIL_M){
		  char *num= nil_tostring(&(n->data.tableVal->strIndexed[i]->value));
		 size+=strlen(num)*sizeof(char);
		  tmp=realloc(tmp,size);
		 strcat(tmp,num);
	 }
	 else if(n->data.tableVal->strIndexed[i]->value.type==UNDEF_M){
		  char *num= undef_tostring(&(n->data.tableVal->strIndexed[i]->value));
		 size+=strlen(num)*sizeof(char);
		  tmp=realloc(tmp,size);
		 strcat(tmp,num);
	 }
	        size+=2*sizeof(char);
			tmp=realloc(tmp,size);
			strcat(tmp,"},");
	  }
	
		
	i++;
    }
	i=0;

	
	while(i<AVM_TABLE_HASHSIZE && found<n->data.tableVal->total){
	
		if(n->data.tableVal->numIndexed[i]!=NULL){
			found++;
			size+=1*sizeof(char);
			tmp=realloc(tmp,size);
			strcat(tmp,"{");
			char *numnum= number_tostring(&(n->data.tableVal->numIndexed[i]->key));
			size+=strlen(numnum)*sizeof(char);
			tmp=realloc(tmp,size);
			 strcat(tmp,numnum);
			size+=1*sizeof(char);
			tmp=realloc(tmp,size);
			strcat(tmp,":");	
	 if(n->data.tableVal->numIndexed[i]->value.type==NUMBER_M){
		 char *num= number_tostring(&(n->data.tableVal->numIndexed[i]->value));
		 size+=strlen(num)*sizeof(char);
		  tmp=realloc(tmp,size);
		 strcat(tmp,num);
	 }
	 else if(n->data.tableVal->numIndexed[i]->value.type==STRING_M){
		  char *num= string_tostring(&(n->data.tableVal->numIndexed[i]->value));
		 size+=strlen(num)*sizeof(char);
		  tmp=realloc(tmp,size);
		 strcat(tmp,num);
	 }
	 else if(n->data.tableVal->numIndexed[i]->value.type==BOOL_M){ 
	 char *num= bool_tostring(&(n->data.tableVal->numIndexed[i]->value));
		 size+=strlen(num)*sizeof(char);
		  tmp=realloc(tmp,size);
		 strcat(tmp,num);}
		 //sxolio-------------------------
	 else if(n->data.tableVal->numIndexed[i]->value.type==TABLE_M){
		 char *num= table_tostring(&(n->data.tableVal->numIndexed[i]->value));
	 size+=strlen(num)*sizeof(char);
	  tmp=realloc(tmp,size);
	 strcat(tmp,num);
	}
	//---------------------------------------
	 else if(n->data.tableVal->numIndexed[i]->value.type==USERFUNC_M){
		  char *num= userfunc_tostring(&(n->data.tableVal->numIndexed[i]->value));
		 size+=strlen(num)*sizeof(char);
		  tmp=realloc(tmp,size);
		 strcat(tmp,num);
	 }
	 else if(n->data.tableVal->numIndexed[i]->value.type==LIBFUNC_M){
		  char *num= libfunc_tostring(&(n->data.tableVal->numIndexed[i]->value));
		 size+=strlen(num)*sizeof(char);
		  tmp=realloc(tmp,size);
		 strcat(tmp,num);
	 }
	 else if(n->data.tableVal->numIndexed[i]->value.type==NIL_M){
		  char *num= nil_tostring(&(n->data.tableVal->numIndexed[i]->value));
		 size+=strlen(num)*sizeof(char);
		  tmp=realloc(tmp,size);
		 strcat(tmp,num);
	 }
	 else if(n->data.tableVal->numIndexed[i]->value.type==UNDEF_M){
		  char *num= undef_tostring(&(n->data.tableVal->numIndexed[i]->value));
		 size+=strlen(num)*sizeof(char);
		  tmp=realloc(tmp,size);
		 strcat(tmp,num);
	 }
	        size+=2*sizeof(char);
			tmp=realloc(tmp,size);
			strcat(tmp,"},");
	  }
	
		
	i++;
    }
	
	strcat(tmp,"] \0");
	return tmp;
	
	
}


tostring_func_t tostringFuncs[]={
  number_tostring,
  string_tostring,
  bool_tostring,
  table_tostring,
  userfunc_tostring,
  libfunc_tostring,
  nil_tostring,
  undef_tostring
};


char* avm_tostring(avm_memcell* m){
	assert(m->type >= 0 && m->type <= UNDEF_M);
	return (*tostringFuncs[m->type])(m);
}


void avm_error(char* format, ...){
    va_list args;
    va_start(args, format);
    vprintf(format, args);
    va_end(args);
   
    executionFinished = 1;
}
 
void avm_calllibfunc(char* funcName);

void avm_dec_top(void){
	if(!top){
		avm_error("stack overflow");	
		executionFinished = 1;
		exit(1);
	}else
		--top;
}

void avm_push_envvalue(unsigned val){
	stack[top].type = NUMBER_M;
	stack[top].data.numVal = val;
	avm_dec_top();
	
}

void avm_callsaveenvironment(void){
	avm_push_envvalue(totalActuals);
	avm_push_envvalue(pc + 1);
	avm_push_envvalue(top + totalActuals + 2);
	avm_push_envvalue(topsp);
}
 
 void execute_call(instruction *t){ 
	 avm_memcell* func = avm_translate_operand(&t->result, &ax);
	  
	 assert(func);
	 avm_callsaveenvironment();
	 switch (func->type){
		 case USERFUNC_M: {
			pc = func->data.funcVal;
			assert(code[pc].opcode == FUNCENTER_V);
			break;
		}
		case STRING_M: avm_calllibfunc(func->data.strVal); break;
		case LIBFUNC_M: avm_calllibfunc(func->data.libfuncVal); break;
		default: {
			char *s = avm_tostring(func);
			avm_error("call: cannot bind \'%s\' to function!", s);
			if(s!=NULL){
			free(s);
			}
			executionFinished = 1;
		}
	}
 }
 void execute_pusharg(instruction *instr){
	 //o savvid to kanei me arg1
	 avm_memcell* arg=avm_translate_operand(&instr->result,&ax);
	 assert(arg);
	 
	 avm_assign(&stack[top],arg);
	 ++totalActuals;
	 avm_dec_top();
 }
 
 userfunc *avm_getfuncinfo(unsigned p){
     unsigned i;
	 
	for(i=0; i < totalUserFuncs; i++){
	
		if(p == userFuncs[i].address){
			return (userFuncs+i);
		}
		
	}
	 return NULL;
 }
 
 void execute_funcenter(instruction *instr){
	 avm_memcell *func = avm_translate_operand(&instr->result, &ax);
	 assert(func);
	 assert(pc == func->data.funcVal);
	 /*Callee actions here*/
	 totalActuals = 0;
	 
	 userfunc *funcInfo = avm_getfuncinfo(pc); 	
	 topsp = top;
	 top = top - funcInfo->localSize;
	 
 }

 unsigned avm_get_envvalue(unsigned i){
	 assert(stack[i].type == NUMBER_M);
	 unsigned val = (unsigned) stack[i].data.numVal;
	 assert(stack[i].data.numVal == ((double)val));
	 
	 return val;
 } 
 

 extern void memclear_string(avm_memcell* m){
	assert(m->data.strVal);
	free(m->data.strVal); 
 }
 
 
 void avm_memcellclear(avm_memcell *m);
 void avm_tablebucketsdestroy(avm_table_bucket **p){
	unsigned i;
	avm_table_bucket *b;
	for(i = 0; i < AVM_TABLE_HASHSIZE; ++i, ++p){
		for(b = *p; b;){
			avm_table_bucket *del = b;
			b = b->next;
			avm_memcellclear(&del->key);
			avm_memcellclear(&del->value);
			free(del);
		}
		p[i] = NULL;
	}
}

 void avm_tabledestroy(avm_table *t){
	
	avm_tablebucketsdestroy(t->strIndexed); 
	avm_tablebucketsdestroy(t->numIndexed);
	if(t!=NULL){
 	 free(t);
	}
}

 
 void avm_tabledecrefcounter(avm_table *t){
	assert(t->refCounter > 0);
	if(!--t->refCounter){
		avm_tabledestroy(t);
	}
}


 extern void memclear_table(avm_memcell* m){
	 assert(m->data.tableVal);
	 avm_tabledecrefcounter(m->data.tableVal);
 }
 
 
typedef void (*memclear_func_t)(avm_memcell*);
memclear_func_t memclearFuncs[]={
	0,
	memclear_string,
	0,
	memclear_table,
	0,
	0,
	0,
	0
 };
 
 
 void avm_memcellclear(avm_memcell *m){
	 if(m->type!=UNDEF_M){
		 memclear_func_t f =memclearFuncs[m->type];
		 if(f)
			(*f)(m);
		m->type=UNDEF_M;
	 }
 }
 
 void execute_funcexit(instruction *unused){
	
	 unsigned oldTop = top;
	 top = avm_get_envvalue(topsp + AVM_SAVEDTOP_OFFSET);
	 pc = avm_get_envvalue(topsp + AVM_SAVEDPC_OFFSET);
	 topsp = avm_get_envvalue(topsp + AVM_SAVEDTOPSP_OFFSET);
	 
	 while(++oldTop <= top)
		 avm_memcellclear(&stack[oldTop]);
 }
 
typedef void (*library_func_t)(void);
void libfunc_print();
void libfunc_typeof();
void libfunc_totalarguments();
void libfunc_input();
void libfunc_objecttotalmembers();
void libfunc_sqrt();
void libfunc_cos();
void libfunc_sin();
void libfunc_strtonum();
void libfunc_objectmemberkeys();
void libfunc_objectcopy();
 
 library_func_t libFuncs[]={
	libfunc_print,
	libfunc_typeof,
	libfunc_totalarguments,
	libfunc_input,
	libfunc_objecttotalmembers,
	libfunc_sqrt,
	libfunc_cos,
	libfunc_sin,
	libfunc_strtonum,
	libfunc_objectmemberkeys,
	libfunc_objectcopy
	
};
 
 library_func_t avm_getlibraryfunc(char *id){

	 if(strcmp(id,"print") == 0){
		return *libFuncs[0];
	 }else if(strcmp(id,"typeof")==0){
		 return *libFuncs[1];
	 }else if(strcmp(id,"totalargumnets")==0){
		 return *libFuncs[2];
	 }else if(strcmp(id,"input")==0){
		 return *libFuncs[3];
	 }else if(strcmp(id,"objecttotalmembers")==0){
	      return *libFuncs[4];
	}else if(strcmp(id, "sqrt") == 0){
		return *libFuncs[5];
	}else if(strcmp(id, "cos") == 0){
		return *libFuncs[6];
	}else if(strcmp(id, "sin") == 0){
		return *libFuncs[7];
	}else if(strcmp(id, "strtonum") == 0){
		return *libFuncs[8];
	}else if(strcmp(id, "objectmemberkeys") == 0){
		return *libFuncs[9];
	}else if(strcmp(id, "objectcopy") == 0){
		return *libFuncs[10];
	}
 }
 
 void avm_calllibfunc(char *id){
	 library_func_t f = avm_getlibraryfunc(id);
	 if(!f){
		avm_error("unsupported lib func \'%s\' called!", id);
		executionFinished = 1;
	}else{
		topsp = top;
		totalActuals = 0;
		(*f)();
		if(!executionFinished){
		execute_funcexit((instruction*)0);}
	 }
 }

void execute_nop(instruction *t){}

void execute_jump(instruction *t){
	pc = t->result.val;
}

unsigned avm_totalactuals(void){
	return avm_get_envvalue(topsp + AVM_NUMACTUALS_OFFSET);
}

avm_memcell *avm_getactual(unsigned i){
	assert(i < avm_totalactuals());
	return &stack[topsp + AVM_STACKENV_SIZE + 1 + i];
}

void libfunc_input(){
	
	avm_memcellclear(&retval);
	int length=0;
	char *tmp=NULL;
	char c;
	int flag=0;
	int tmp_c;
	while((c=getc(stdin))!='\n'){
		length++;
		tmp = realloc(tmp, length * sizeof(char ));
		tmp[length-1] = c;
		if(c<=57 && c>=48){
			flag=flag+1;
		}else{
			flag=flag+2;
		}
	}
	length++;
	tmp = realloc(tmp, length * sizeof(char ));
		tmp[length-1] = '\0';
	if(strcmp(tmp,"true") == 0){
		retval.type=BOOL_M;
		retval.data.boolVal = 1;
	}else if(strcmp(tmp,"false")==0){
		retval.type=BOOL_M;
		retval.data.boolVal = 0;
	}else if(strcmp(tmp,"nil")==0){
		retval.type=NIL_M;
		retval.data.strVal = strdup("nil");
	}else if(flag==(length-1)){
		retval.type=NUMBER_M;
		retval.data.numVal = atof(tmp);
	}else{
		retval.type=STRING_M;
		retval.data.strVal = strdup(tmp);
	}
	
	
}
void libfunc_objecttotalmembers(){
	avm_memcellclear(&retval);
	
	unsigned n;
	n = avm_totalactuals();
	avm_memcell *t=avm_getactual(0);
	avm_tableincrefcounter(t->data.tableVal);
	retval.type=NUMBER_M;
	retval.data.numVal = (double)t->data.tableVal->total;
}

void libfunc_print(){  
	unsigned n,i,j;
	n = avm_totalactuals();

	for(i = 0; i < n; ++i){ 
		char *s = avm_tostring(avm_getactual(i));
		for(j = 0; s[j] != '\0'; j++){		
			if(s[j] == '~'){
				s[j] = '\n';	
			}
		}
	puts(s);
		free(s);
	}
}

void libfunc_sqrt(){
	unsigned n = avm_totalactuals();

	if(n > 1){
		avm_error("'sqrt' has too many arguments\n");
		executionFinished = 1;
	}

	if(avm_getactual(0)->data.numVal < 0){
		avm_error("argument of 'sqrt' is negative\n");
		executionFinished = 1;
	}

	if(avm_getactual(0)->type != NUMBER_M){
		avm_error("The argument of 'sqrt' is not a number\n");
		executionFinished = 1;
	}else{
		avm_memcellclear(&retval);
		retval.type = NUMBER_M;
		retval.data.numVal = abs(sqrt(avm_getactual(0)->data.numVal));
	}
}

void libfunc_cos(){
	unsigned n = avm_totalactuals();

	if(n > 1){
		avm_error("'cos' has too many arguments\n");
		executionFinished = 1;
	}

	if(avm_getactual(0)->type != NUMBER_M){
		avm_error("The argument of 'cos' is not a number\n");
		executionFinished = 1;
	}else{
		avm_memcellclear(&retval);
		retval.type = NUMBER_M;
		retval.data.numVal = cos(avm_getactual(0)->data.numVal);
	}
}

void libfunc_sin(){
	unsigned n = avm_totalactuals();

	if(n > 1){
		avm_error("'sin' has too many arguments\n");
		executionFinished = 1;
	}

	if(avm_getactual(0)->type != NUMBER_M){
		avm_error("The argument of 'sin' is not a number\n");
		executionFinished = 1;
	}else{
		avm_memcellclear(&retval);
		retval.type = NUMBER_M;
		retval.data.numVal = sin(avm_getactual(0)->data.numVal);
	}
}

void libfunc_strtonum(){

	unsigned n = avm_totalactuals();
	char *validity;
	
	if(n > 1){
		avm_error("'strtonum' has too many arguments\n");
		executionFinished = 1;
	}

	if(avm_getactual(0)->type != STRING_M){
		avm_error("The argument of 'strtonum' is not a string\n");
		executionFinished = 1;
	}else{
		avm_memcellclear(&retval);
		retval.type = NUMBER_M;
		retval.data.numVal = strtod(avm_getactual(0)->data.strVal, &validity);
		if(avm_getactual(0)->data.strVal == validity){
			avm_error("string argument of 'strtonum' cannot convert to number\n");
			executionFinished = 1;
		}
	}
}

void avm_registerlibfunc(char *id, library_func_t addr){

}

 execute_func_t executeFuncs[] = {
	execute_assign,
	execute_add,
	execute_sub,
	execute_mul,
	execute_div,
	execute_mod,
	execute_uminus,
	execute_and,
	execute_or,
	execute_not,
	execute_jump,
	execute_jeq,
	execute_jne,
	execute_jle,
	execute_jge,
	execute_jlt,
	execute_jgt,
	execute_call,
	execute_pusharg,
	execute_funcenter,
	execute_funcexit,
	execute_newtable,
	execute_tablegetelem,
	execute_tablesetelem,
	execute_nop
};

#define AVM_ENDING_PC codeSize

void execute_cycle(void){
	
	if(executionFinished){
		return;
	}
	else if(pc == AVM_ENDING_PC){
		executionFinished=1;
		return;
	}else{
		assert(pc<AVM_ENDING_PC);
		instruction* instr = code + pc;
		assert ( instr->opcode>=0 && instr->opcode <= 24);
	if(instr->srcLine)
		currLine=instr->srcLine;
	unsigned oldPC=pc;
	(*executeFuncs[instr->opcode])(instr);
	if(pc==oldPC)
		++pc;
	}
}

void avm_tableincrefcounter(avm_table *t){
	++t->refCounter;
}

void avm_warning(char* format, ...){
    va_list args;
    va_start(args, format);
    vprintf(format, args);
    va_end(args);
   
    executionFinished = 1;
}

void avm_assign(avm_memcell* lv,avm_memcell* rv){ 

	if(lv == rv){
		return;
	}
	
	
	if(lv->type==TABLE_M &&
	    rv->type==TABLE_M &&
		lv->data.tableVal== rv->data.tableVal){
		return;
	}

		
	  if(rv->type==UNDEF_M){
			
			avm_warning("assigning from 'undef' content!");
			}
	
	avm_memcellclear(lv);
	memcpy(lv,rv,sizeof(avm_memcell));

	if(lv->type == STRING_M)
		lv->data.strVal=strdup(rv->data.strVal);
	else if(lv->type == TABLE_M){
		avm_tableincrefcounter(lv->data.tableVal);
	}
}
 
 
double add_impl(double x, double y){ return x+y; }
double sub_impl(double x, double y){ return x-y; }
double mul_impl(double x, double y){ return x*y; }
double div_impl(double x, double y){ return x/y; }
 
double mod_impl(double x, double y){ 
	return ((unsigned) x) %((unsigned)y); 
}
				
				
arithmetic_func_t arithmeticFuncs[]={
	add_impl,
	sub_impl,
	mul_impl,
	div_impl,
	mod_impl
};

void execute_arithmetic(instruction* instr){
	avm_memcell* lv = avm_translate_operand(&instr->result, (avm_memcell*)0);
	avm_memcell* rv1 = avm_translate_operand(&instr->arg1, &ax);
	avm_memcell* rv2 = avm_translate_operand(&instr->arg2, &bx);

	assert(lv &&(&stack[AVM_STACKSIZE-1]>= lv && lv>&stack[top]|| lv==&retval));
	assert(rv1 && rv2);
	
	if(rv1->type != NUMBER_M || rv2->type!= NUMBER_M){
		avm_error("not a number in arithmetic!");
		executionFinished=1;
	}else{
		arithmetic_func_t op=arithmeticFuncs[instr->opcode-ADD_V];
		avm_memcellclear(lv);
		lv->type = NUMBER_M;
		lv->data.numVal=(*op)(rv1->data.numVal ,rv2->data.numVal);
	}
	
}

typedef unsigned char (*tobool_func_t)(avm_memcell*);

unsigned char number_tobool(avm_memcell *m){
	return m->data.numVal != 0;
}

unsigned char string_tobool(avm_memcell *m){
	return m->data.strVal[0] != 0;
}

unsigned char bool_tobool(avm_memcell *m){
	return m->data.boolVal;
}

unsigned char table_tobool(avm_memcell *m){
	return 1;
}

unsigned char userfunc_tobool(avm_memcell *m){
	return 1;
}

unsigned char libfunc_tobool(avm_memcell *m){
	return 1;
}

unsigned char nil_tobool(avm_memcell *m){
	return 0;
}

unsigned char undef_tobool(avm_memcell *m){
	assert(0);
	return 0;
}

tobool_func_t toboolFuncs[] = {
	number_tobool,
	string_tobool,
	bool_tobool,
	table_tobool,
	userfunc_tobool,
	libfunc_tobool,
	nil_tobool,
	undef_tobool
};

unsigned char avm_tobool(avm_memcell *m){
	assert(m->type >= 0  && m->type < UNDEF_M);
	
	return (*toboolFuncs[m->type])(m);
}

char *typeStrings[] = {
	"number",
	"string",
	"bool",
	"table",
	"userfunc",
	"libfunc",
	"nil",
	"undef"
};

void execute_jeq(instruction *instr){
	assert(instr->result.type == LABEL_A);
	
	avm_memcell *rv1 = avm_translate_operand(&instr->arg1, &ax);
	avm_memcell *rv2 = avm_translate_operand(&instr->arg2, &bx);
	
	unsigned char result = 0;
	
	if(rv1->type == UNDEF_M || rv2->type == UNDEF_M){
		avm_error("'undef' involved in equality!"); 
	}else if(rv1->type == NIL_M || rv2->type == NIL_M){
		result = rv1->type == NIL_M && rv2->type == NIL_M;
	}else if(rv1->type == BOOL_M || rv2->type == BOOL_M){
		result = (avm_tobool(rv1) == avm_tobool(rv2));
	}else if(rv1->type != rv2->type){
		avm_error("%s == %s is illegal!", typeStrings[rv1->type], typeStrings[rv2->type]);
	}else{
		if(rv1->type==NUMBER_M){
			if(rv1->data.numVal==rv2->data.numVal){
				result=1;
			}
		}else if(rv1->type == STRING_M){
			if(strcmp(rv1->data.strVal,rv2->data.strVal)==0)
				result =  1;
		}else if(rv1->type == TABLE_M){
			
			if(rv1->data.tableVal == rv2->data.tableVal)
				result = 1;
		}else if(rv1->type == USERFUNC_M){
			if(rv1->data.funcVal == rv2->data.funcVal)
				result = 1;
		}else if(rv1->type == LIBFUNC_M){
			if(rv1->data.libfuncVal == rv2->data.libfuncVal)
				result = 1;
		}
			
	}
	
	if(!executionFinished && result){
		pc = instr->result.val;
	}
}
 void execute_jne(instruction *instr){
	assert(instr->result.type == LABEL_A);
	
	avm_memcell *rv1 = avm_translate_operand(&instr->arg1, &ax);
	avm_memcell *rv2 = avm_translate_operand(&instr->arg2, &bx);
	
	unsigned char result = 0;
	
	if(rv1->type == UNDEF_M || rv2->type == UNDEF_M){
		avm_error("'undef' involved in equality!"); 
	}else if(rv1->type == NIL_M || rv2->type == NIL_M){
		if( rv1->type != NIL_M ){
			result = 1;
		}else if( rv2->type != NIL_M ){
			result=1;
		}else{
			result=0;
		}
	}else if(rv1->type == BOOL_M || rv2->type == BOOL_M){
		result = (avm_tobool(rv1) != avm_tobool(rv2));
	}else if(rv1->type != rv2->type){
		avm_error("%s == %s is illegal!", typeStrings[rv1->type], typeStrings[rv2->type]);
	}else{

	if(rv1->type==NUMBER_M){
		if(rv1->data.numVal != rv2->data.numVal){
			result=1;
		}
	}else if(rv1->type == STRING_M){
		if(strcmp(rv1->data.strVal,rv2->data.strVal)!=0)
			result = 1;
	}else if(rv1->type == TABLE_M){
		if(rv1->data.tableVal != rv2->data.tableVal)
			result = 1;
	}else if(rv1->type == USERFUNC_M){
		if(rv1->data.funcVal != rv2->data.funcVal)
			result = 1;
	}else if(rv1->type == LIBFUNC_M){
		if(rv1->data.libfuncVal != rv2->data.libfuncVal)
			result = 1;
	}
		/*TODO ---> Equality check with dispatching*/
	}
	
	if(!executionFinished && result){
		pc = instr->result.val;
	}
 }
 void execute_jle(instruction *instr){
	assert(instr->result.type == LABEL_A);
	
	avm_memcell *rv1 = avm_translate_operand(&instr->arg1, &ax);
	avm_memcell *rv2 = avm_translate_operand(&instr->arg2, &bx);
	
	unsigned char result = 0;
	
	if(rv1->type == UNDEF_M || rv2->type == UNDEF_M){
		avm_error("'undef' involved in less equal!"); 
	}else if(rv1->type == NIL_M || rv2->type == NIL_M){
		avm_error("'nil' involved in less equal!");
	}else if(rv1->type == BOOL_M || rv2->type == BOOL_M){
		avm_error("invalid use of comparison operator on constant boolean");
	}else if(rv1->type != rv2->type){
		avm_error("%s == %s is illegal!", typeStrings[rv1->type], typeStrings[rv2->type]);
	}else{

		if(rv1->type==NUMBER_M){
			if(rv1->data.numVal <= rv2->data.numVal){
				result=1;
			}
		}else if(rv1->type == STRING_M){
			if(strcmp(rv1->data.strVal,rv2->data.strVal)<=0)
				result = 1;
		}else if(rv1->type == TABLE_M){
			if(rv1->data.tableVal <= rv2->data.tableVal)
				result = 1;
		}else if(rv1->type == USERFUNC_M){
			if(rv1->data.funcVal <= rv2->data.funcVal)
				result = 1;
		}else if(rv1->type == LIBFUNC_M){
			if(rv1->data.libfuncVal <= rv2->data.libfuncVal)
				result = 1;
		}
			/*TODO ---> Equality check with dispatching*/
	}
	
	if(!executionFinished && result){
		pc = instr->result.val;
	}
 }
 void execute_jge(instruction *instr)
    {assert(instr->result.type == LABEL_A);
	
	avm_memcell *rv1 = avm_translate_operand(&instr->arg1, &ax);
	avm_memcell *rv2 = avm_translate_operand(&instr->arg2, &bx);
	
	unsigned char result = 0;
	
	if(rv1->type == UNDEF_M || rv2->type == UNDEF_M){
		avm_error("'undef' involved in less equal!"); 
	}else if(rv1->type == NIL_M || rv2->type == NIL_M){
		avm_error("'nil' involved in less equal!");
	}else if(rv1->type == BOOL_M || rv2->type == BOOL_M){
		avm_error("invalid use of comparison operator on constant boolean");
	}else if(rv1->type != rv2->type){
		avm_error("%s == %s is illegal!", typeStrings[rv1->type], typeStrings[rv2->type]);
	}else{

		if(rv1->type==NUMBER_M){
			if(rv1->data.numVal >= rv2->data.numVal){
				result=1;
			}
		}else if(rv1->type == STRING_M){
			if(strcmp(rv1->data.strVal,rv2->data.strVal)>=0)
				result = 1;
		}else if(rv1->type == TABLE_M){
			if(rv1->data.tableVal >= rv2->data.tableVal)
				result = 1;
		}else if(rv1->type == USERFUNC_M){
			if(rv1->data.funcVal >= rv2->data.funcVal)
				result = 1;
		}else if(rv1->type == LIBFUNC_M){
			if(rv1->data.libfuncVal >= rv2->data.libfuncVal)
				result = 1;
		}
			/*TODO ---> Equality check with dispatching*/
	}
	
	if(!executionFinished && result){
		pc = instr->result.val;
	}}
 void execute_jlt(instruction *instr){
	 assert(instr->result.type == LABEL_A);
	
	avm_memcell *rv1 = avm_translate_operand(&instr->arg1, &ax);
	avm_memcell *rv2 = avm_translate_operand(&instr->arg2, &bx);
	
	unsigned char result = 0;
	
	if(rv1->type == UNDEF_M || rv2->type == UNDEF_M){
		avm_error("'undef' involved in less equal!"); 
	}else if(rv1->type == NIL_M || rv2->type == NIL_M){
		avm_error("'nil' involved in less equal!");
	}else if(rv1->type == BOOL_M || rv2->type == BOOL_M){
		avm_error("invalid use of comparison operator on constant boolean");
	}else if(rv1->type != rv2->type){
		avm_error("%s == %s is illegal!", typeStrings[rv1->type], typeStrings[rv2->type]);
	}else{

		if(rv1->type==NUMBER_M){
			if(rv1->data.numVal < rv2->data.numVal){
				result=1;
			}
		}else if(rv1->type == STRING_M){
			if(strcmp(rv1->data.strVal,rv2->data.strVal)<0)
				result = 1;
		}else if(rv1->type == TABLE_M){
			if(rv1->data.tableVal < rv2->data.tableVal)
				result = 1;
		}else if(rv1->type == USERFUNC_M){
			if(rv1->data.funcVal < rv2->data.funcVal)
				result = 1;
		}else if(rv1->type == LIBFUNC_M){
			if(rv1->data.libfuncVal < rv2->data.libfuncVal)
				result = 1;
		}
			/*TODO ---> Equality check with dispatching*/
	}
	
	if(!executionFinished && result){
		pc = instr->result.val;
	}
 }
 void execute_jgt(instruction *instr){

 	assert(instr->result.type == LABEL_A);
	
	avm_memcell *rv1 = avm_translate_operand(&instr->arg1, &ax);
	avm_memcell *rv2 = avm_translate_operand(&instr->arg2, &bx);
	
	unsigned char result = 0;
	
	if(rv1->type == UNDEF_M || rv2->type == UNDEF_M){
		avm_error("'undef' involved in less equal!"); 
	}else if(rv1->type == NIL_M || rv2->type == NIL_M){
		avm_error("'nil' involved in less equal!");
	}else if(rv1->type == BOOL_M || rv2->type == BOOL_M){
		avm_error("invalid use of comparison operator on constant boolean");
	}else if(rv1->type != rv2->type){
		avm_error("%s == %s is illegal!", typeStrings[rv1->type], typeStrings[rv2->type]);
	}else{

		if(rv1->type==NUMBER_M){
			if(rv1->data.numVal > rv2->data.numVal){
				result=1;
			}
		}else if(rv1->type == STRING_M){
			if(strcmp(rv1->data.strVal,rv2->data.strVal)>0)
				result = 1;
		}else if(rv1->type == TABLE_M){
			if(rv1->data.tableVal > rv2->data.tableVal)
				result = 1;
		}else if(rv1->type == USERFUNC_M){
			if(rv1->data.funcVal > rv2->data.funcVal)
				result = 1;
		}else if(rv1->type == LIBFUNC_M){
			if(rv1->data.libfuncVal > rv2->data.libfuncVal)
				result = 1;
		}
			/*TODO ---> Equality check with dispatching*/
	}
	
	if(!executionFinished && result){
		pc = instr->result.val;
	}
 }
 
void libfunc_typeof(void){
	unsigned n = avm_totalactuals();
	
	if(n != 1){
		avm_error("one argument (not %d) expected in 'typeof'!", n); 
	}
	else{
		avm_memcellclear(&retval);
		retval.type = STRING_M;
		retval.data.strVal = strdup(typeStrings[avm_getactual(0)->type]);
	}
}

avm_table *avm_tablenew(void){
	avm_table *t = (avm_table *)malloc(sizeof(avm_table));
	AVM_WIPEOUT(*t);

	t->refCounter = t->total = 0;
	avm_tablebucketsinit(t->numIndexed);
	avm_tablebucketsinit(t->strIndexed);
    //avm_tablebucketsinit(t->tableIndexed);
	return t;
}


void execute_newtable(instruction *instr){ 
	avm_memcell *lv = avm_translate_operand(&instr->result, (avm_memcell *)0);
	assert(lv && (&stack[AVM_STACKSIZE-1] >= lv && lv > &stack[top] || lv == &retval));
	
	avm_memcellclear(lv);
	lv->type = TABLE_M;
	lv->data.tableVal = (avm_table *)avm_tablenew();	
	avm_tableincrefcounter(lv->data.tableVal);
}

avm_memcell *avm_tablegetelem(avm_table *table, avm_memcell *index){
	
	if(index->type == NUMBER_M){
		int hash = hash_numbers(index->data.numVal);
		avm_table_bucket *head = table->numIndexed[hash];
		avm_table_bucket *tmp = head;
		if(head == NULL){
			return NULL;
		}
		while(tmp != NULL ){
			if(tmp->key.data.numVal == index->data.numVal){
				
			return &tmp->value;
		}
			tmp = tmp->next;
		}
		
		if(tmp==NULL){return NULL;}
		
		
		
		
	}else if(index->type == STRING_M){ 
		int hash = hash_strings(index->data.strVal);
		avm_table_bucket *head = table->strIndexed[hash];
	
		avm_table_bucket *tmp = head;
		if(head == NULL){
			return NULL;
		}

		while(tmp != NULL ){
			 if(strcmp(tmp->key.data.strVal, index->data.strVal) == 0) {
			return &tmp->value;
		}
			tmp = tmp->next;
		}
		if(tmp==NULL){
			return NULL;
		}
		
		
	}
	
	else if(index->type == TABLE_M){
		
		unsigned  hashnumber=index->data.tableVal->tableposition;
		int hash = hash_numbers(hashnumber);
		avm_table_bucket *head = table->tableIndexed[hash];
		avm_table_bucket *tmp = head;
		if(head == NULL){
			return NULL;
		}
		if(tmp->key.data.tableVal == index->data.tableVal) {
			return &tmp->value;
		}
		return NULL;
		
	}
}

void avm_tablesetelem(avm_table *table, avm_memcell *index, avm_memcell *content){
		
	if(index->type == NUMBER_M){
		int hash = hash_numbers(index->data.numVal);
		avm_table_bucket *head = table->numIndexed[hash];

		avm_table_bucket *newelem = malloc(sizeof(avm_table_bucket));
		avm_table_bucket *tmp = head;
		avm_table_bucket *prev = head;

		newelem->key = *index;
		newelem->value = *content;
		newelem->next = NULL;
		 if(content->type == TABLE_M){
			 avm_tableincrefcounter(table);
		 }
		
		if(head == NULL){
			table->total++;
			table->numIndexed[hash] = newelem;
			head = newelem;
		}else{
			while(tmp != NULL ){
				if(tmp->key.data.numVal == index->data.numVal){
					break;
				}
				prev = tmp;
				tmp = tmp->next;
			}
            if(tmp == NULL){
				table->total++;
				prev->next = newelem;
				tmp = prev->next;
			}
			else if(tmp->key.data.numVal == index->data.numVal){
				tmp->value = *content;
			}
			
		}

	}else if(index->type == STRING_M){
		int hash = hash_strings(index->data.strVal);
		avm_table_bucket *head = table->strIndexed[hash];
		avm_table_bucket *newelem = malloc(sizeof(avm_table_bucket));
		avm_table_bucket *tmp = head;
		avm_table_bucket *prev = head;
		
		newelem->key = *index;
		newelem->value = *content;
		newelem->next = NULL;
		 if(content->type == TABLE_M){
			 avm_tableincrefcounter(table);
		 }
		
		if(head == NULL){
			table->total++;
			table->strIndexed[hash] = newelem;
			head = newelem;
		}else{
			while(tmp != NULL ){
				if(strcmp(tmp->key.data.strVal,index->data.strVal) == 0){
					break;
				}
				prev = tmp;
				tmp = tmp->next;
			}
			
			if(tmp == NULL){
				table->total++;
				prev->next = newelem;
				tmp = prev->next;
			}
			else if(strcmp(tmp->key.data.strVal, index->data.strVal) == 0){
				tmp->value = *content;
			}
		}
	}else if(index->type == TABLE_M){
		
		int hash = hash_numbers(tablecounter);
		avm_table_bucket *head = table->tableIndexed[hash];
		avm_table_bucket *newelem = malloc(sizeof(avm_table_bucket));
		avm_table_bucket *tmp = head;
		avm_table_bucket *prev = head;

		newelem->key = *index;
		newelem->key.data.tableVal->tableposition=tablecounter;
		newelem->value = *content;
		newelem->next = NULL;
		 if(content->type == TABLE_M){
			 avm_tableincrefcounter(table);
		 }
		
		if(head == NULL){
			table->total++;
			table->tableIndexed[hash] = newelem;
			head = newelem;
		}else{
		 	table->tableIndexed[hash] = newelem;
		}
          tablecounter++;
	}
}

void execute_tablegetelem(instruction *instr){
	avm_memcell *lv = avm_translate_operand(&instr->result, (avm_memcell*)0);
	avm_memcell *t = avm_translate_operand(&instr->arg1, (avm_memcell*)0);
	avm_memcell *i = avm_translate_operand(&instr->arg2, &ax);

	assert(lv && &stack[AVM_STACKSIZE-1] >= lv && lv > &stack[top] || lv == &retval);
	assert(t && &stack[AVM_STACKSIZE-1] >= t && t > &stack[top]);
	assert(i);
		
	avm_memcellclear(lv);
	
	lv->type = NIL_M;
		
	if(t->type != TABLE_M){
			
		avm_error("illegal use of type %s as table!", typeStrings[t->type]);
	}else{
			
		avm_memcell *content = avm_tablegetelem(t->data.tableVal, i);
		if(content){
			
			avm_assign(lv, content);
		
		}else{
			char *ts = avm_tostring(t);
			char *is = avm_tostring(i);
			avm_warning("%s[%s] not found!", ts, is); 
			free(ts);
			free(is);
		}
	}
}

void execute_tablesetelem(instruction *instr){
	avm_memcell *t = avm_translate_operand(&instr->result, (avm_memcell *)0);
	avm_memcell *i = avm_translate_operand(&instr->arg1, &ax);
	avm_memcell *c = avm_translate_operand(&instr->arg2, &bx);

	assert(t && &stack[AVM_STACKSIZE-1] >= t && t > &stack[top]);
	assert(i && c);
	
	if(t->type != TABLE_M){
		avm_error("illegal use of type %s as table!", typeStrings[t->type]);
	}else{
		avm_tablesetelem(t->data.tableVal, i, c);
	}
}

void libfunc_totalarguments(void){
	unsigned p_topsp = avm_get_envvalue(topsp + AVM_SAVEDTOPSP_OFFSET);
	avm_memcellclear(&retval);
	
	if(!p_topsp){
		avm_error("'totalarguments' called outside a function!");
		retval.type = NIL_M;
	}else{
		retval.type = NUMBER_M;
		retval.data.numVal = avm_get_envvalue(p_topsp + AVM_NUMACTUALS_OFFSET);
	}
}

void libfunc_objectmemberkeys(){	
	
	unsigned n = avm_totalactuals();

	if(n > 1){
		avm_error("'objectmemberkeys' has too many arguments\n");
		executionFinished = 1;
	}
	if(avm_getactual(0)->type != TABLE_M){
		avm_error("argument of 'objectmemberkeys' is not a table\n");
		executionFinished = 1;
	}
	
	unsigned i;
	unsigned position = 0;
	avm_table *t = avm_tablenew();
	avm_table_bucket *tmp;
	avm_memcell *index, *content;
	
	for(i = 0; i < AVM_TABLE_HASHSIZE; i++){
		tmp = avm_getactual(0)->data.tableVal->numIndexed[i];
		while(tmp != NULL){
			index = malloc(sizeof(avm_memcell));
			content = malloc(sizeof(avm_memcell));

			*content = tmp->key;
			index->data.numVal = position;
			index->type = NUMBER_M;
			
			avm_tablesetelem(t, index, content);
			position++;
			tmp = tmp->next;
		}
	}
	for(i = 0; i < AVM_TABLE_HASHSIZE; i++){
		tmp = avm_getactual(0)->data.tableVal->strIndexed[i];
		while(tmp != NULL){
			index = malloc(sizeof(avm_memcell));
			content = malloc(sizeof(avm_memcell));

			*content = tmp->key;
			index->data.numVal = position;
			index->type = NUMBER_M;
			
			avm_tablesetelem(t, index, content);
			position++;
			tmp = tmp->next;
		}
	}


	retval.type = TABLE_M;
	retval.data.tableVal = t;
}

void libfunc_objectcopy(){

	unsigned n = avm_totalactuals();

	if(n > 1){
		avm_error("'objectcopy' has too many arguments\n");
		executionFinished = 1;
	}

	if(avm_getactual(0)->type != TABLE_M){
		avm_error("argument of 'objectcopy' is not a table\n");
		executionFinished = 1;
	}
	
	unsigned i;
	avm_table *t = avm_tablenew();
	avm_table_bucket *tmp;

	for(i = 0; i < AVM_TABLE_HASHSIZE; i++){
		tmp = avm_getactual(0)->data.tableVal->numIndexed[i];
		while(tmp != NULL){
			avm_memcell *index = malloc(sizeof(avm_memcell));
			avm_memcell *content = malloc(sizeof(avm_memcell));

			*index = tmp->key;
			*content = tmp->value;
			avm_tablesetelem(t, index, content);
			tmp = tmp->next;
		}
	}
	for(i = 0; i < AVM_TABLE_HASHSIZE; i++){
		tmp = avm_getactual(0)->data.tableVal->strIndexed[i];
		while(tmp != NULL){
			avm_memcell *index = malloc(sizeof(avm_memcell));
			avm_memcell *content = malloc(sizeof(avm_memcell));

			*index = tmp->key;
			*content = tmp->value;
			avm_tablesetelem(t, index, content);
			tmp = tmp->next;
		}
	}

	retval.type = TABLE_M;
	retval.data.tableVal = t;
}

void avm_initialize(void){
	
	avm_initstack();
	
	avm_registerlibfunc("print", libfunc_print);
	avm_registerlibfunc("typeof", libfunc_typeof);
	avm_registerlibfunc("totalarguments", libfunc_totalarguments);
	avm_registerlibfunc("input", libfunc_input);
	avm_registerlibfunc("objecttotalmembers", libfunc_objecttotalmembers);
	//same for the others
}


void read_instructions(){
	FILE *file;
	file = fopen("instructions.abc", "rb");
	char line[128];
	char *token;
	int i;
	
	if (file) {
		fgets(line, sizeof(line), file);
		token = strtok(line, "\t");
		if(strcmp(token, "magicnumber") == 0){
		
			token = strtok(NULL, "\t ");
			if(atoi(token) != 340200501){
				
				printf("Number is not magic!\n");
				exit(0);
			}
			
			fgets(line, sizeof(line), file);
			fgets(line, sizeof(line), file);
			token = strtok(line, "\t: ");
		}

		if(strcmp(token, "strings") == 0){
			token = strtok(NULL, "\t :");
			if(strcmp(token, "total") == 0){
				token = strtok(NULL, "\t ");
				totalStringConsts = atoi(token);
				stringConsts = malloc(totalStringConsts * sizeof(char *));
				
				for(i = 0;  i < totalStringConsts; ++i){
					fgets(line, sizeof(line), file);
					token = strtok(line, "\t");
					int size=atoi(strtok(NULL, "size:\t "));
					stringConsts[i] =(char*) malloc(size*sizeof(char));
					stringConsts[i] =strdup(token);
				}
				if(totalStringConsts == 0){stringConsts = NULL;}
			}
			fgets(line, sizeof(line), file);
			fgets(line, sizeof(line), file);
			token = strtok(line, "\t: ");
		}

		if(strcmp(token, "numbers") == 0){
			token = strtok(NULL, "\t: ");
			if(strcmp(token, "total") == 0){
				token = strtok(NULL, "\t ");
				totalNumConsts = atoi(token);
				numConsts = malloc(sizeof(double) * totalNumConsts);
				for(i = 0;  i < totalNumConsts; ++i){
					fgets(line, sizeof(line), file);
					token = strtok(line, "\t ");
					numConsts[i] = atof(token);
				}
				if(totalNumConsts==0){numConsts=NULL;}
			}
			fgets(line, sizeof(line), file);
			fgets(line, sizeof(line), file);
			token = strtok(line, "\t: ");
		}
		if(strcmp(token, "userfunctions") == 0){
			token = strtok(NULL, "\t :");
			if(strcmp(token, "total") == 0){
				token = strtok(NULL, "\t ");
				totalUserFuncs = atoi(token);
				userFuncs = malloc(totalUserFuncs * sizeof(userFuncs));

				for(i = 0; i < totalUserFuncs; ++i){
					fgets(line, sizeof(line), file);
					token = strtok(line, "address\t ");
					userFuncs[i].address = atoi(token);
					fgets(line, sizeof(line), file);
					token = strtok(line, "local size\t ");
					userFuncs[i].localSize = atoi(token);
					fgets(line, sizeof(line), file);
					token = strtok(line, "id\t ");
					userFuncs[i].id = strdup(token);
				}
				if(totalUserFuncs == 0){ userFuncs = NULL; }
			}
			fgets(line, sizeof(line), file);
			fgets(line, sizeof(line), file);
			token = strtok(line, "\t: ");
		}
		if(strcmp(token, "libfunctions") == 0){
			token = strtok(NULL, "\t :");
			if(strcmp(token, "total") == 0){
				token = strtok(NULL, "\t ");
				totalNamedLibfuncs = atoi(token);
				namedLibfuncs = malloc(totalNamedLibfuncs * sizeof(char *));
					if(totalNamedLibfuncs == 0){namedLibfuncs = NULL;}
			}
				for(i = 0;  i < totalNamedLibfuncs; ++i){
					fgets(line, sizeof(line), file);
					token = strtok(line, "\t ");
					int size = atoi(strtok(NULL, "size:\t "));
					namedLibfuncs[i] = malloc(size * sizeof(char));
					namedLibfuncs[i] = strdup(token);
				}
			
			fgets(line, sizeof(line), file);
			fgets(line, sizeof(line), file);
			token = strtok(line, "\t: ");
		}

		if(strcmp(token, "globalVariables") == 0){
			token = strtok(NULL, "\t :\n");
			globals = atoi(token);
			fgets(line, sizeof(line), file);
			fgets(line, sizeof(line), file);
		}
		token = strtok(line, "\t: ");	
		if(strcmp(token, "totalInstructions") == 0){
			token = strtok(NULL, "\t :\n");
			codeSize = atoi(token);
			fgets(line, sizeof(line), file);
			fgets(line, sizeof(line), file);
		
		}
		code = malloc(codeSize * sizeof(instruction));
		for(i = 0; i < codeSize; ++i){
			fgets(line, sizeof(line), file);
			token = strtok(line, "\t:\n ");
			code[i].opcode = atoi(token);
			token = strtok(NULL, "\t ()\n ");
			if(strcmp(token,"-1")!=0){
			code[i].result.type = atoi(token);}
			token = strtok(NULL, "\t () \n");
			if(strcmp(token,"-1")!=0){
			code[i].result.val = atoi(token);}
			
			token = strtok(NULL, "\t () \n");
	
			if(strcmp(token,"-1")!=0){
			code[i].arg1.type = atoi(token);}
			token = strtok(NULL, "\t () ");
			if(strcmp(token,"-1")!=0){
			code[i].arg1.val = atoi(token);}
			token = strtok(NULL, "\t () ");
		
			
		    if(strcmp(token,"-1")!=0){
			code[i].arg2.type = atoi(token);}
			token = strtok(NULL, "\t () ");
			if(strcmp(token,"-1")!=0){
			code[i].arg2.val = atoi(token);}
			
			
		}
	}


	fclose(file);
}

int hash_strings(char *str) {

	int size = strlen(str);
	int total = 0;
	int y;
	for (y = 0; y < size; y++) {
		total += (int)str[y];
	}

	return (total % AVM_TABLE_HASHSIZE);
}

int hash_numbers(double num){
	int x = (int)num;
	if(x<0){
		x=-x;
	}
	return (x % AVM_TABLE_HASHSIZE);
}

int main(){
	
	avm_initialize();
	read_instructions();
	
	top = topsp = AVM_STACKSIZE - globals - 2;
	executionFinished=0;
	while(executionFinished==0){
		execute_cycle();
	}
	return 1;
}






