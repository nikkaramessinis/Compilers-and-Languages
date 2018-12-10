#ifndef SYMBOLTABLE_H_INCLUDED
#define SYMBOLTABLE_H_INCLUDED
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef int bool;
#define true 1
#define false 0
#define MAX_SIZE 500

extern unsigned programVarOffset;
extern unsigned functionLocalOffset;
extern unsigned formalArgOffset;
extern unsigned scopeSpaceCounter;

typedef struct Variable {
	const char *name;
	unsigned int scope;
	unsigned int line;
}Variable;

typedef struct Function {
	const char *name;
	unsigned int scope;
	unsigned int line;
	unsigned iaddress;
	unsigned totallocals;
}Function;

enum SymbolTableType {
	GLOBAL, LOCAL_, FORMAL,
	USERFUNC, LIBFUNC, NOTHING
};

typedef enum scopespace_t{
	PROGRAM_VAR,
	FUNCTION_LOC,
	FORMAL_ARG
}scopespace_t;

enum symbol_t{
	VAR_S,
	PROGRAMFUNC_S,
	LIBFUNC_S
};

typedef struct SymbolTableEntry {
	bool isActive;
	unsigned int offset;
	union {
		Variable *varVal;
		Function *funcVal;
	}value;
	enum SymbolTableType type;
	enum symbol_t type_s;
	enum scopespace_t scope_space;
	struct SymbolTableEntry *next;
	struct SymbolTableEntry *scopenext;
}SymbolTableEntry;

SymbolTableEntry *head;

typedef struct LinkedListScope{
	SymbolTableEntry *token;
	int scope;
	struct LinkedListScope *next;
}LinkedListScope;

LinkedListScope *headList;
typedef struct FunctionRange{
	int  scopeFunction;
	struct FunctionRange *next;
}FunctionRange;

FunctionRange *f_head;

typedef enum expr_t{
	VAR_E,
	TABLEITEM_E,
	PROGRAMFUNC_E,
	LIBRARYFUNC_E,
	ARITHEXPR_E,
	BOOLEXPR_E,
	ASSIGNEXPR_E,
	NEWTABLE_E,
	CONSTNUM_E,
	CONSTBOOL_E,
	CONSTSTRING_E,
	NIL_E
}expr_t;

typedef enum iopcode{

	ASSIGN, ADD_IO, SUB_IO,
	MUL, DIV, MOD,
	UMINUS_IO, AND_IO, OR_IO,
	NOT_IO, IF_EQ, IF_NOTEQ,
	IF_LESSEQ, IF_GREATEREQ, IF_LESS,
	IF_GREATER, JUMP, CALL, PARAM,
	RET, GETRETVAL, FUNCSTART,
	FUNCEND, TABLECREATE, TABLEGETELEM,
	TABLESETELEM
}iopcode;

typedef struct backpatch{
	unsigned label;
	struct  backpatch *next;
}backpatch;

typedef struct expr{
	
	expr_t type;
	SymbolTableEntry *sym;
	struct expr *index;
	double numConst;
	char *strConst;
	unsigned char boolConst;
	struct expr *next;

}expr;

typedef struct statements{
    backpatch *breaklist;
	backpatch *contlist;

}statements;

statements *stmthead;

typedef struct forprefix_t{
	unsigned test;
	unsigned enter;
}forprefix_t;

typedef struct indexelem{
	expr * leftexpr;
	expr * rightexpr;
	struct indexelem * next;
}indexelem;

expr *expr_head;
indexelem * indexelem_head;

typedef struct quad{

	iopcode op;
	expr *result;
	expr *arg1;
	expr *arg2;
	unsigned label;
	unsigned line;
}quad;


typedef struct LocalStack
{
	unsigned functionLocalOffset;
	struct LocalStack * next;
}LocalStack;

LocalStack * LocalStackhead;

typedef struct LoopCounterStack{
	unsigned int loopcounter;
	struct LoopCounterStack *next;
}LoopCounterStack;

LoopCounterStack *loopCounterHead;

typedef struct call_t{
	expr *elist_t;
	unsigned char method;
	char *name;
}call_t;

#endif