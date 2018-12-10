#ifndef SYMBOLTABLE_H_INCLUDED
#define SYMBOLTABLE_H_INCLUDED
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef int bool;
#define true 1
#define false 0
#define MAX_SIZE 500


typedef struct Variable {
	const char *name;
	unsigned int scope;
	unsigned int line;
}Variable;

typedef struct Function {
	const char *name;
	unsigned int scope;
	unsigned int line;
}Function;

enum SymbolTableType {
	GLOBAL, LOCAL_, FORMAL,
	USERFUNC, LIBFUNC, NOTHING
};

typedef struct SymbolTableEntry {
	bool isActive;
	union {
		Variable *varVal;
		Function *funcVal;
	}value;
	enum SymbolTableType type;
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
#endif