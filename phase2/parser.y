%{

#include <stdio.h>
#include <stdlib.h>
#include "symboltable.h"
#include "parser.h"
int yyerror(char* yaccProvidedMessage);
int yylex(void);

extern int yylineno;
extern char* yytext;
extern FILE* yyin;

unsigned int curr_scope = 0;
unsigned int scope_function = 0;
unsigned int whileFlag = 0;

%}

%expect 1

%union {
	char* stringValue;
	int intValue;
	double realValue;
	struct SymbolTableEntry *entry;
}

%start program

%token <stringValue> ID
%token <intValue> INTCONST
%token <floatValue> REALCONST
%token <stringValue> STRING
%token IF ELSE WHILE FOR FUNCTION RETURN BREAK CONTINUE AND NOT OR LOCAL TRUE FALSE NIL
%token DOUBLE_COLON DOUBLE_DOT EQUAL NOT_EQUAL LESS_EQUAL GREATER_EQUAL INCREMENT DECREMENT
%type <entry> lvalue

%left '(' ')'
%left '[' ']'
%left '.' DOUBLE_DOT
%right NOT INCREMENT DECREMENT UMINUS
%left  '*' '/' '%'
%left '+' '-'
%nonassoc '>' GREATER_EQUAL '<' LESS_EQUAL
%nonassoc EQUAL NOT_EQUAL
%left AND
%left OR
%left '='


%%

program: program stmt	{ printf("program -> stmt*\n"); }
		|				{ printf("program -> empty\n"); }
		;

stmt: expr ';'			{ printf("stmt -> expr;\n"); }
	| ifstmt 			{ printf("stmt -> ifstmt\n"); }
	| whilestmt 		{ printf("stmt -> whilestmt\n"); }
	| forstmt 			{ printf("stmt -> forstmt\n"); }
	| returnstmt 		{ printf("stmt -> returnstmt\n"); }
	| BREAK ';' 		{ 
						if(whileFlag == 0){
							insertError("Error! %s outside while in line %d. \n", "break", yylineno);
						} 
						else	printf("stmt -> break;\n"); }
	| CONTINUE ';' 		{
						if(whileFlag == 0)
							insertError("Error! %s outside while in line %d. \n", "continue", yylineno);
						else	printf("stmt -> continue;\n"); 
						}
	| block 			{ printf("stmt -> block\n"); }
	| funcdef 			{ printf("stmt -> funcdef\n"); }
	| ';' 				{ printf("stmt -> ;\n"); }
	;

expr: assignexpr 		{ printf("expr -> assignexpr\n"); }
	| expr '+' expr 	{ printf("expr -> expr + expr\n"); }
	| expr '-' expr 	{ printf("expr -> expr - expr\n"); }
	| expr '*' expr 	{ printf("expr -> expr * expr\n"); }
	| expr '/' expr 	{ printf("expr -> expr / expr\n"); }
	| expr '%' expr 	{ printf("expr -> expr % expr\n"); }
	| expr '>' expr 	{ printf("expr -> expr > expr\n"); }
	| expr '<' expr 	{ printf("expr -> expr < expr\n"); }
	| expr GREATER_EQUAL expr 	{ printf("expr -> expr >= expr\n"); }
	| expr LESS_EQUAL expr 	{ printf("expr -> expr <= expr\n"); }
	| expr AND expr 	{ printf("expr -> expr and expr\n"); }
	| expr OR expr 		{ printf("expr -> expr or expr\n"); }
	| expr EQUAL expr 	{ printf("expr -> expr == expr\n"); }
	| expr NOT_EQUAL expr 	{ printf("expr -> expr != expr\n"); }
	| term 				{ printf("expr -> term\n"); }
	;

term: '(' expr ')' 				{ printf("term -> (expr)\n"); }
	| '-' expr %prec UMINUS 	{ printf("term -> -expr\n"); }
	| NOT expr 					{ printf("term -> not expr\n"); }
	| INCREMENT lvalue 			{ SymbolTableEntry *entry=$2;
								if(entry->type == USERFUNC) 
									insertError("Error! Trying to increment function %s in line %d.\n", entry->value.funcVal->name, yylineno);
								else if(entry->type == LIBFUNC)	
									insertError("Error! Trying to increment library function %s in line %d.\n", entry->value.funcVal->name, yylineno);
								else	printf("term -> ++lvalue\n"); 
								}
	| lvalue INCREMENT 			{ SymbolTableEntry *entry=$1;
								if(entry->type == USERFUNC) 
									insertError("Error! Trying to increment function %s in line %d.\n", entry->value.funcVal->name, yylineno);
								else if(entry->type == LIBFUNC)	
									insertError("Error! Trying to increment library function %s in line %d.\n", entry->value.funcVal->name, yylineno);
								else	printf("term -> lvalue++\n"); 
								}
	| DECREMENT lvalue 			{ SymbolTableEntry *entry=$2;
								if(entry->type == USERFUNC) 
									insertError("Error! Trying to decrement function %s in line %d.\n", entry->value.funcVal->name, yylineno);
								else if(entry->type == LIBFUNC)	
									insertError("Error! Trying to decrement library function %s in line %d.\n", entry->value.funcVal->name, yylineno);
								else	printf("term -> --lvalue\n"); 
								}
	| lvalue DECREMENT 			{ SymbolTableEntry *entry=$1;
								if(entry->type == USERFUNC) 
									insertError("Error! Trying to decrement function %s in line %d.\n", entry->value.funcVal->name, yylineno);
								else if(entry->type == LIBFUNC)	
									insertError("Error! Trying to decrement library function %s in line %d.\n", entry->value.funcVal->name, yylineno);
								else	printf("term -> lvalue--\n"); 
								}
	| primary 					{ printf("term -> primary\n"); }
	;

assignexpr: lvalue '=' expr 	
	{	SymbolTableEntry *entry=$1;
		if(entry!=NULL){
		if(entry->type == USERFUNC){
			insertError("Error! Trying to assign value to function %s in line %d.\n", entry->value.funcVal->name, yylineno);
		}
		else if(entry->type == LIBFUNC)	insertError("Error! Trying to assign value to library function %s in line %d.\n", entry->value.funcVal->name, yylineno);
		printf("assignexpr -> lvalue = expr\n"); 
	}}
	;

primary: lvalue 		{ printf("primary -> lvalue\n"); }
	| call 				{ printf("primary -> call\n"); }
	| objectdef 		{ printf("primary -> objectdef\n"); }
	| '(' funcdef ')' 	{ printf("primary -> (funcdef)\n"); }
	| const 			{ printf("primary -> const\n"); }
	;

lvalue: ID 				{printf("lvalue -> id\n"); 
						if(LookUpAll($1,curr_scope,yylineno,0)==1){
							$$ =(SymbolTableEntry*)searchAll($1,curr_scope);
							SymbolTableEntry *entry = $$;
							if(entry->type == USERFUNC){
							}else{
								if(entry->value.varVal->scope < scope_function && entry->value.varVal->scope!=0)
									{
									insertError("Error! %s cannot be accessed in line %d. \n", $1, yylineno);
								}	

							}
							
						}else{
							if(curr_scope == 0)
	  						 	$$ = (SymbolTableEntry*)insert(true, $1, curr_scope, yylineno, GLOBAL); 
	  						else
	  							$$ = (SymbolTableEntry*)insert(true, $1, curr_scope, yylineno, LOCAL_); 
						}						
		}
	| LOCAL ID 			{printf("lvalue -> local id\n");
						if(curr_scope == 0){
						    if(LookUp($2, curr_scope,yylineno,0) == 0)
								$$ =(SymbolTableEntry*)insert(true, $2, curr_scope, yylineno, GLOBAL);
							else{
								$$ =(SymbolTableEntry*)searchAll($2, curr_scope);
							}
						 }else{
						    if(LookUp($2, curr_scope,yylineno,1) == 0)
								$$ =(SymbolTableEntry*)insert(true, $2, curr_scope, yylineno, LOCAL_);
						 }
						}
	| DOUBLE_COLON ID 	{$$=NULL;
						printf("lvalue -> ::id\n"); 
						if(LookUp($2, 0,yylineno,0) == 0) insertError("Error! No global declaration of token %s in line %d.\n", $2, yylineno);}
	| member 			{ printf("lvalue -> member\n"); }
	;

member: lvalue '.' ID 		{ printf("member -> lvalue.id\n"); 
							if(LookUpAll($1, curr_scope,yylineno,0) == 0){
								if(curr_scope == 0)
									insert(true, $1, curr_scope, yylineno, GLOBAL);
								else
									insert(true, $1, curr_scope, yylineno, LOCAL_);
							}
							 
							}
	| lvalue '[' expr ']' 	{printf("member -> lvalue[expr]\n"); 
							if(LookUpAll($1, curr_scope,yylineno,0) == 0){
								if(curr_scope == 0)
									insert(true, $1, curr_scope, yylineno, GLOBAL);
								else
									insert(true, $1, curr_scope, yylineno, LOCAL_);
							}
							
							}
	
	| call '.' ID 			{ printf("member -> call.id\n"); }
	| call '[' expr ']' 	{ printf("member -> call[expr]\n"); }
	;

call: call '(' elist ')' 			{ printf("call -> call(elist)\n"); }
	| lvalue callsuffix 			{ printf("call -> lvalue callsuffix\n"); 
									
									}
	| '(' funcdef ')' '(' elist ')' { printf("call -> (funcdef)(elist)\n"); }
	;

callsuffix: normcall 		{ printf("callsuffix -> normcall\n"); }
		| methodcall 		{ printf("callsuffix ->	methodcall\n"); }
		;

normcall: '(' elist ')' 	{ printf("normcall -> (elist)\n"); }
		;

methodcall: DOUBLE_DOT ID '(' elist ')' { printf("methodcall ->	..id (elist)\n"); }
		;

elist:  expr 					{ printf("elist	-> expr\n"); }
		| elist  ',' expr 		{ printf("elist	-> elist , expr*\n"); }
		|						{ printf("elist	-> empty\n"); }
		;

objectdef: '[' elist ']' 		{ printf("objectdef	-> [ elist ]\n");}
		| '[' indexed ']' 		{ printf("objectdef	-> [ indexed ]\n");}
		;

indexed: indexedelem 			{ printf("indexed -> indexedelem\n");}
		| indexed ',' indexedelem { printf("indexed	-> indexed , indexedelem\n");}
		;

indexedelem: '{' expr ':' expr '}' { printf("indexedelem -> { expr : expr }\n");}
			;

block: '{' {curr_scope++; range_insert(scope_function);} statements '}' {range_delete(); scope_function=getlastRange(); hide(curr_scope); curr_scope--; printf("block -> { statements }\n");}
		;

statements: statements stmt { printf("statements -> statements stmt\n");}
			|				{ printf("statements ->	empty\n");}
			;

funcdef: FUNCTION ID {int ret_val=LookUp($2,curr_scope,yylineno,1);
					if(ret_val== 0)
						insert (true,$2,curr_scope,yylineno,USERFUNC);
					else if(ret_val==1)
					insertError("Error!Redeclaration of token %s in line %d.\n", $2, yylineno);} '(' idlist ')' {scope_function = curr_scope+1;} block
		{ printf("funcdef -> function id (idlist) block\n");

 		}
		| FUNCTION {insert (true, generate_name(), curr_scope, yylineno, USERFUNC);} '(' idlist ')' {scope_function=curr_scope+1; } block 
		{ 	printf("funcdef -> function  (idlist) block\n");
		}
		;

const: INTCONST  			{ printf("const ->	intconst\n"); }
		| REALCONST 		{ printf("const -> realconst\n");}
		| STRING 			{ printf("const -> string\n");}
		| NIL 				{ printf("const -> nil\n");}
		| TRUE 				{ printf("const -> true\n");}
		| FALSE 			{ printf("const -> false\n");}
		;

idlist: ID 						{curr_scope++; printf("idlist -> id\n");
								int ret_val = LookUp($1, curr_scope,yylineno, 1);
								if(ret_val == 0) 
									insert(true, $1, curr_scope, yylineno, FORMAL);
								else if(ret_val == 1)
									insertError("Error! Formal redeclaration of token %s in line %d.\n", $1, yylineno);
								curr_scope--;}
		| idlist ',' ID 		{curr_scope++; printf("idlist -> idlist , id\n");
								int ret_val=LookUp($3, curr_scope,yylineno,1);
								if( ret_val== 0)
									insert (true, $3, curr_scope, yylineno, FORMAL);
								else if(ret_val==1)
									insertError("Error!Formal redeclaration of token %s in line %d.\n", $3, yylineno);
								 curr_scope--;}
		|						{printf("idlist	-> empty\n");}
		;

ifstmt: IF '(' expr ')' stmt ELSE stmt {printf("ifstmt -> if ( expr ) stmt else stmt\n");}
		| IF '(' expr ')' stmt {printf("ifstmt -> if ( expr ) stmt\n");}
		;

whilestmt: WHILE '(' expr ')' {whileFlag = 1;} stmt  	{ whileFlag=0; printf("whilestmt -> while ( expr ) stmt\n");}
		;

forstmt: FOR '(' elist ';' expr ';' elist ')' stmt 	{printf("forstmt -> for ( elist; expr; elist) stmt\n");}
		;

returnstmt: RETURN expr ';' { printf("returnstmt -> return expr;\n");}
		| RETURN ';' { printf("returnstmt -> return expr;\n");} 
		;

%%


int yyerror(char* yaccProvidedMessage){
	fprintf(stderr, "%s: at line %d before token: %s\n", yaccProvidedMessage, yylineno, yytext);
	fprintf(stderr, "INPUT NOT VALID\n");
}

int main(int argc, char** argv){
	init_hash();
	if(argc > 1){
		if(!(yyin = fopen(argv[1], "r"))){
			fprintf(stderr, "Cannot read file: %s\n", argv[1]);
			return 1;
		}
	}else{
		yyin = stdin;
	}
	yyparse();
	PrintScopeList();
	errors();
	return 0;
}