#ifndef TERMINAL_CODE_H_INCLUDED
#define TERMINAL_CODE_H_INCLUDED

typedef enum vmopcode{
	ASSIGN_V, ADD_V, SUB_V,
	MUL_V, DIV_V, MOD_V,
	UMINUS_V, AND_V, OR_V,
	NOT_V, JUMP_V, JEQ_V, JNE_V,
	JLE_V, JGE_V, JLT_V,
	JGT_V, CALL_V, PUSHARG_V,
	FUNCENTER_V, FUNCEXIT_V, NEWTABLE_V,
	TABLEGETELEM_V, TABLESETELEM_V, NOP_V
}vmopcode;

typedef enum vmarg_t{
	LABEL_A = 0,
	GLOBAL_A = 1,
	FORMAL_A = 2,
	LOCAL_A = 3,
	NUMBER_A = 4,
	STRING_A = 5,
	BOOL_A = 6,
	NIL_A = 7,
	USERFUNC_A = 8,
	LIBFUNC_A = 9,
	RETVAL_A = 10,
	UNDEF_A=11
}vmarg_t;

typedef struct vmarg{
	vmarg_t type;
	unsigned val;
}vmarg;

typedef struct instruction{
	vmopcode opcode;
	vmarg result;
	vmarg arg1;
	vmarg arg2;
	unsigned srcLine;
}instruction;

typedef struct userfunc{
	unsigned address;
	unsigned localSize;
	char *id;
	unsigned userfunc_scope;
}userfunc;

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

struct avm_table;

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

#define AVM_STACKSIZE 4096
#define AVM_WIPEOUT(m) memset(&(m), 0, sizeof(m))

avm_memcell stack[AVM_STACKSIZE];
static void avm_initstack(void);

avm_memcell *avm_tablegetelem(avm_memcell *key);
void avm_tablesetelem(avm_memcell *key, avm_memcell *value);

#define AVM_TABLE_HASHSIZE 211

typedef struct avm_table_bucket{
	avm_memcell key;
	avm_memcell value;
	struct avm_table_bucket *next;
}avm_table_bucket;

typedef struct avm_table{
	unsigned refCounter;
	avm_table_bucket *strIndexed[AVM_TABLE_HASHSIZE];
	avm_table_bucket *numIndexed[AVM_TABLE_HASHSIZE];
	unsigned total;
}avm_table;

avm_table *avm_tablenew(void);
void avm_tabledestroy(avm_table *t);
void avm_tabledecrefcounter(avm_table *t);
void avm_tablebucketsinit(avm_table_bucket **p);
void avm_memcellclear(avm_memcell *m);
void avm_tablebucketsdestroy(avm_table_bucket **p);

void make_operand(expr *e, vmarg *arg);

typedef struct incomplete_jump{
	unsigned instrNo;
	unsigned iaddress;
	struct incomplete_jump *next;
}incomplete_jump;

incomplete_jump *ij_head;
unsigned ij_total = 0;

void add_incomplete_jump(unsigned instrNo, unsigned iaddress);

extern void generate_ADD(quad q);
extern void generate_SUB(quad q);
extern void generate_MUL(quad q);
extern void generate_DIV(quad q);
extern void generate_MOD(quad q);
extern void generate_NEWTABLE(quad q);
extern void generate_TABLEGETELEM(quad q);
extern void generate_TABLESETELEM(quad q);
extern void generate_ASSIGN(quad q);
extern void generate_NOP();
extern void generate_JUMP(quad q);
extern void generate_IF_EQ(quad q);
extern void generate_IF_NOTEQ(quad q);
extern void generate_IF_GREATER(quad q);
extern void generate_IF_GREATEREQ(quad q);
extern void generate_IF_LESS(quad q);
extern void generate_IF_LESSEQ(quad q);
extern void generate_NOT(quad q);
extern void generate_OR(quad q);
extern void generate_PARAM(quad q);
extern void generate_CALL(quad q);
extern void generate_GETRETVAL(quad q);
extern void generate_FUNCSTART(quad q);
extern void generate_RETURN(quad q);
extern void generate_FUNCEND(quad q);
extern void generate_UMINUS(quad q);

typedef struct ReturnList{
	unsigned target_label;
	struct ReturnList *next;
}ReturnList;

typedef struct FuncStack{
	const char *id;
	unsigned taddress;
	unsigned totallocals;
	ReturnList *retlist;
	struct FuncStack *next;
}FuncStack;

FuncStack *funcstack_head;

#endif