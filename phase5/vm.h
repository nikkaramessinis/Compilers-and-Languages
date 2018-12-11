#include "terminal_code.h"
#define AVM_STACKSIZE 4096
#define AVM_WIPEOUT(m) memset(&(m), 0, sizeof(m))
#define AVM_TABLE_HASHSIZE 211
#define AVM_STACKENV_SIZE 4

typedef enum avm_memcell_t{
	NUMBER_M = 0,
	STRING_M = 1,
	BOOL_M = 2,
	TABLE_M = 3,
	USERFUNC_M = 4,
	LIBFUNC_M = 5,
	NIL_M = 6,
	UNDEF_M = 7
}avm_memcell_t;

typedef struct avm_memcell{
	avm_memcell_t type;
	union{
		double numVal;
		char* strVal;
		unsigned char boolVal;
		struct avm_table *tableVal;
		unsigned funcVal;
		char *libfuncVal;
	}data;
}avm_memcell;

avm_memcell ax,bx,cx;
avm_memcell retval;
unsigned top,topsp;
double consts_getnumber(unsigned index);
char* consts_getstring(unsigned index);
char* libfuncs_getused(unsigned index);

avm_memcell stack[AVM_STACKSIZE];

static void avm_initstack(void);

typedef void (*execute_func_t)(instruction*);
#define AVM_MAX_INSTRUCTIONS (unsigned) NOP_V

extern void execute_assign(instruction *t);
extern void execute_add(instruction *t);
extern void execute_sub(instruction *t);
extern void execute_mul(instruction *t);
extern void execute_div(instruction *t);
extern void execute_mod(instruction *t);
extern void execute_uminus(instruction *t);
extern void execute_and(instruction *t);
extern void execute_or(instruction *t);
extern void execute_not(instruction *t);
extern void execute_jeq(instruction *t);
extern void execute_jne(instruction *t);
extern void execute_jle(instruction *t);
extern void execute_jge(instruction *t);
extern void execute_jlt(instruction *t);
extern void execute_jgt(instruction *t);
extern void execute_call(instruction *t);
extern void execute_pusharg(instruction *t);
extern void execute_funcenter(instruction *t);
extern void execute_funcexit(instruction *t);
extern void execute_newtable(instruction *t);
extern void execute_tablegetelem(instruction *t);
extern void execute_tablesetelem(instruction *t);
extern void execute_nop(instruction *t);

extern void avm_warning(char* format, ...);
extern void avm_assign(avm_memcell* lv,avm_memcell* rv);



typedef struct avm_table_bucket{
	avm_memcell key;
	avm_memcell value;
	struct avm_table_bucket *next;
}avm_table_bucket;

typedef struct avm_table{
	unsigned tableposition;
	unsigned refCounter;
	avm_table_bucket *strIndexed[AVM_TABLE_HASHSIZE];
	avm_table_bucket *numIndexed[AVM_TABLE_HASHSIZE];
	avm_table_bucket *tableIndexed[AVM_TABLE_HASHSIZE];
	unsigned total;
}avm_table;


avm_memcell *avm_tablegetelem(avm_table *table, avm_memcell *key);
void avm_tablesetelem(avm_table *table, avm_memcell *key, avm_memcell *value);

extern void avm_error(char* format, ...);
extern char* avm_tostring(avm_memcell*);
extern void avm_calllibfunc(char* funcName);
extern void avm_callsaveenvironment(void);
extern userfunc* avm_getfuncinfo(unsigned address);
typedef char* (*tostring_func_t)(avm_memcell*);

extern char* number_tostring(avm_memcell* );
extern char* string_tostring(avm_memcell* );
extern char* bool_tostring(avm_memcell*);
extern char* table_tostring(avm_memcell*);
extern char* userfunc_tostring(avm_memcell*);
extern char* libfunc_tostring(avm_memcell*);
extern char* nil_tostring(avm_memcell*);
extern char* undef_tostring(avm_memcell*);

typedef double (*arithmetic_func_t)(double x,double y);
