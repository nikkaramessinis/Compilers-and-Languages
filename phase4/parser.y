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
unsigned int loop_counter = 0;
unsigned int stmt_counter = 0;

%}

%expect 1

%union {
	char* stringValue;
	int intValue;
	double realValue;
	struct SymbolTableEntry *entry;
	struct expr *expression;
	struct call_t *call;
	struct statements *stmnts;
	struct forprefix_t *forprefix_t;
	unsigned unsignedValue;
}

%start program

%token <stringValue> ID 
%token <intValue> INTCONST
%token <realValue> REALCONST
%token <stringValue> STRING NIL
%token IF ELSE WHILE FOR FUNCTION RETURN BREAK CONTINUE AND NOT OR LOCAL TRUE FALSE
%token DOUBLE_COLON DOUBLE_DOT EQUAL NOT_EQUAL LESS_EQUAL GREATER_EQUAL INCREMENT DECREMENT
%type <expression> const lvalue expr term primary assignexpr member objectdef elist indexed call
%type <entry> funcdef funcprefix
%type <intValue> funcbody ifprefix elseprefix 
%type <stringValue> funcname
%type <call> methodcall normcall callsuffix
%type <stmnts> stmt statements whilestmt block ifstmt forstmt
%type <forprefix_t> forprefix
%type<unsignedValue> whilestart whilecond N M

%right '='
%left OR
%left AND
%nonassoc EQUAL NOT_EQUAL
%nonassoc '>' GREATER_EQUAL '<' LESS_EQUAL
%left '+' '-'
%left  '*' '/' '%'
%right NOT INCREMENT DECREMENT UMINUS
%left '.' DOUBLE_DOT
%left '[' ']'
%left '(' ')'


%%

program: program stmt	{ printf("program -> stmt*\n"); }
		|				{ printf("program -> empty\n"); }
		;

stmt: expr ';'			{ printf("stmt -> expr;\n"); $$=malloc(sizeof(statements)); $$->breaklist = NULL; $$->contlist = NULL; resettemp(); }
	| ifstmt 			{ printf("stmt -> ifstmt\n");
							$$=malloc(sizeof(statements));
							//$$->breaklist=NULL;
							//$$->contlist=NULL;
							printf("lalal\n");
							$$->breaklist = (backpatch *)merge($$->breaklist, $1->breaklist);
							printf("kakakakl\n");
							$$->contlist = (backpatch *)merge($$->contlist, $1->contlist);
							printf("sasasaa\n");
							resettemp();
						}
	| whilestmt 		{ printf("stmt -> whilestmt\n"); 
							$$=malloc(sizeof(statements)); 
							$$->breaklist = NULL; 
							$$->contlist = NULL; 
							resettemp();
						}
	| forstmt 			{ printf("stmt -> forstmt\n"); $$=malloc(sizeof(statements)); $$->breaklist = NULL; $$->contlist = NULL; resettemp();}
	| returnstmt 		{ printf("stmt -> returnstmt\n"); $$=malloc(sizeof(statements)); $$->breaklist = NULL; $$->contlist = NULL; resettemp();}
	| BREAK ';' 		{ 	printf("stmt -> break;\n");
							$$=malloc(sizeof(statements));
							if(loop_counter == 0){
								insertError("Error! %s outside while in line %d. \n", "break", yylineno);
							}
							$$->breaklist = (backpatch *)pushbackpatchList(nextquadlabel());
							$$->contlist=NULL;
							emit(JUMP, NULL, NULL, NULL, -1, yylineno);
							resettemp();
						}
	| CONTINUE ';' 		{	printf("stmt -> continue;\n");
							$$=malloc(sizeof(statements)); 
							if(loop_counter == 0)
								insertError("Error! %s outside while in line %d. \n", "continue", yylineno);
							
							$$->contlist = (backpatch *)pushbackpatchList(nextquadlabel());
							$$->breaklist = NULL;
							emit(JUMP, NULL, NULL, NULL, -1, yylineno);
							resettemp();
						}
	| block 			{ printf("stmt -> block\n"); $$=malloc(sizeof(statements)); $$ = $1; resettemp();}
	| funcdef 			{ printf("stmt -> funcdef\n"); 
							$$=malloc(sizeof(statements));
							$$->breaklist = NULL; 
							$$->contlist = NULL; 
							resettemp();
						}
	| ';' 				{ printf("stmt -> ;\n"); $$=malloc(sizeof(statements)); resettemp();}
	;

expr: assignexpr 		{ $$=$1;
						printf("expr -> assignexpr\n"); }
	| expr '+' expr 	{ 	printf("expr -> expr + expr\n"); 
							$$ = (expr*)newexpr(ARITHEXPR_E);
							$$->sym = (SymbolTableEntry *)new_temp(curr_scope, yylineno);
							emit(ADD_IO, $$, $1, $3, -1, yylineno);
						}
	| expr '-' expr 	{ 	printf("expr -> expr - expr\n");
							$$ = (expr*)newexpr(ARITHEXPR_E);
							$$->sym = (SymbolTableEntry *)new_temp(curr_scope, yylineno);
							emit(SUB_IO, $$, $1, $3, -1, yylineno);
						 }
	| expr '*' expr 	{ 	printf("expr -> expr * expr\n"); 
							$$ = (expr*)newexpr(ARITHEXPR_E);
							$$->sym = (SymbolTableEntry *)new_temp(curr_scope, yylineno);
							emit(MUL, $$, $1, $3, -1, yylineno);

						}
	| expr '/' expr 	{ 	printf("expr -> expr / expr\n"); 
							$$ = (expr*)newexpr(ARITHEXPR_E);
							$$->sym = (SymbolTableEntry *)new_temp(curr_scope, yylineno);
							emit(DIV, $$, $1, $3, -1, yylineno);
						}
	| expr '%' expr 	{ 	printf("expr -> expr % expr\n"); 
							$$ = (expr*)newexpr(ARITHEXPR_E);
							$$->sym = (SymbolTableEntry *)new_temp(curr_scope, yylineno);
							emit(MOD, $$, $1, $3, -1, yylineno);
						}
	| expr '>' expr 	{ 	printf("expr -> expr > expr\n"); 
							$$ = (expr*)newexpr(BOOLEXPR_E);
							$$->sym = (SymbolTableEntry *)new_temp(curr_scope, yylineno);
							emit(IF_GREATER, NULL, $1, $3, nextquadlabel()+3, yylineno);
							emit(ASSIGN, $$, new_expr_constbool(0), NULL, -1, yylineno);
							emit(JUMP, NULL, NULL, NULL, nextquadlabel()+2, yylineno);
							emit(ASSIGN, $$, new_expr_constbool(1), NULL, -1, yylineno);
						}
	| expr '<' expr 	{ 	printf("expr -> expr < expr\n"); 
							$$ = (expr*)newexpr(BOOLEXPR_E);
							$$->sym = (SymbolTableEntry *)new_temp(curr_scope, yylineno);
							emit(IF_LESS, NULL, $1, $3, nextquadlabel()+3, yylineno);
							emit(ASSIGN, $$, new_expr_constbool(0), NULL, -1, yylineno);
							emit(JUMP, NULL, NULL, NULL, nextquadlabel()+2, yylineno);
							emit(ASSIGN, $$, new_expr_constbool(1), NULL, -1, yylineno);
						}
	| expr GREATER_EQUAL expr 	{ 	printf("expr -> expr >= expr\n"); 
									$$ = (expr*)newexpr(BOOLEXPR_E);
									$$->sym = (SymbolTableEntry *)new_temp(curr_scope, yylineno);
									emit(IF_GREATEREQ,NULL, $1, $3, nextquadlabel()+3, yylineno);
									emit(ASSIGN, $$, new_expr_constbool(0), NULL, -1, yylineno);
									emit(JUMP, NULL, NULL, NULL, nextquadlabel()+2, yylineno);
									emit(ASSIGN, $$, new_expr_constbool(1), NULL, -1, yylineno);
								}
	| expr LESS_EQUAL expr 	{ 	printf("expr -> expr <= expr\n"); 
								$$ = (expr*)newexpr(BOOLEXPR_E);
								$$->sym = (SymbolTableEntry *)new_temp(curr_scope, yylineno);
								emit(IF_LESSEQ, NULL, $1, $3, nextquadlabel()+3, yylineno);
								emit(ASSIGN, $$, new_expr_constbool(0), NULL, -1, yylineno);
								emit(JUMP, NULL, NULL, NULL, nextquadlabel()+2, yylineno);
								emit(ASSIGN, $$, new_expr_constbool(1), NULL, -1, yylineno);
							}
	| expr AND expr 	{ printf("expr -> expr and expr\n"); 
							$$ = (expr*)newexpr(BOOLEXPR_E);
							$$->sym = (SymbolTableEntry *)new_temp(curr_scope, yylineno);
							emit(AND_IO, $$, $1, $3, -1, yylineno);
						}
	| expr OR expr 		{ 	printf("expr -> expr or expr\n"); 
							$$ = (expr*)newexpr(BOOLEXPR_E);
							$$->sym = (SymbolTableEntry *)new_temp(curr_scope, yylineno);
							emit(OR_IO, $$, $1, $3, -1, yylineno);
						}
	| expr EQUAL expr 	{ 	printf("expr -> expr == expr\n"); 
							$$ = (expr*)newexpr(BOOLEXPR_E);
							$$->sym = (SymbolTableEntry *)new_temp(curr_scope, yylineno);
							emit(IF_EQ, $$, $1, $3, nextquadlabel()+3, yylineno);
							emit(ASSIGN, $$, new_expr_constbool(0), NULL, -1, yylineno);
							emit(JUMP, NULL, NULL, NULL, nextquadlabel()+2, yylineno);
							emit(ASSIGN, $$, new_expr_constbool(1), NULL, -1, yylineno);
						}
	| expr NOT_EQUAL expr 	{ 	printf("expr -> expr != expr\n"); 
								$$ = (expr*)newexpr(BOOLEXPR_E);
								$$->sym = (SymbolTableEntry *)new_temp(curr_scope, yylineno);
								emit(IF_NOTEQ, NULL, $1, $3, nextquadlabel()+3, yylineno);
								emit(ASSIGN, $$, new_expr_constbool(0), NULL, -1, yylineno);
								emit(JUMP, NULL, NULL, NULL, nextquadlabel()+2, yylineno);
								emit(ASSIGN, $$, new_expr_constbool(1), NULL, -1, yylineno);
							}
	| term 				{ $$=$1; printf("expr -> term\n"); }
	;


term: '(' expr ')' 				{ printf("term -> (expr)\n"); $$ = $2;}
	| '-' expr %prec UMINUS 	{ 	printf("term -> -expr\n");
									checkuminus($2);
									$$ = (expr*)newexpr(ARITHEXPR_E);
									$$->sym = (SymbolTableEntry *)new_temp(curr_scope, yylineno);
									emit(UMINUS_IO, $$, $2, NULL,  -1, yylineno);
								}
	| NOT expr 					{ printf("term -> not expr\n"); 
									$$ = (expr *)newexpr(BOOLEXPR_E);
									$$->sym = (SymbolTableEntry *)new_temp(curr_scope, yylineno);
									emit(NOT_IO, $$, $2, NULL, -1, yylineno);
								}
	| INCREMENT lvalue 			{ 	printf("term -> ++lvalue\n");
									if($2 != NULL){
										if($2->type == TABLEITEM_E){
										$$ = (expr *)emit_iftableitem($2, curr_scope, yylineno);
										emit(ADD_IO, $$, $$, newexpr_constnum((double)1), -1, yylineno);
										emit(TABLESETELEM, $$, $2, $2->index, -1, yylineno);
									}else{
										emit(ADD_IO, $2, $2, newexpr_constnum((double)1), -1, yylineno);
										$$ = (expr*)newexpr(ARITHEXPR_E);
										$$->sym = (SymbolTableEntry *)new_temp(curr_scope, yylineno);
										emit(ASSIGN, $$, $2, NULL, -1, yylineno);
									}
									
									SymbolTableEntry *entry = $2->sym;
									if(entry->type == USERFUNC) 
										insertError("Error! Trying to increment function %s in line %d.\n", entry->value.funcVal->name, yylineno);
									else if(entry->type == LIBFUNC)	
										insertError("Error! Trying to increment library function %s in line %d.\n", entry->value.funcVal->name, yylineno);
										
								}
								}
	| lvalue INCREMENT 			{ 	printf("term -> lvalue++\n");
									if($1 != NULL){
										$$ = (expr*)newexpr(VAR_E);
										$$->sym = (SymbolTableEntry *)new_temp(curr_scope, yylineno);

										expr *val;
										if($1->type == TABLEITEM_E){
											val = (expr *)emit_iftableitem($1, curr_scope, yylineno);
											emit(ASSIGN, $$, val, NULL, -1, yylineno);
											emit(ADD_IO, val, val, newexpr_constnum((double)1), -1, NULL);
											emit(TABLESETELEM, val, $1, $1->index, -1, NULL);
										}else{
											emit(ASSIGN, $$, $1, NULL, -1, yylineno);
											emit(ADD_IO, $1, $1, newexpr_constnum((double)1), -1, yylineno);
										}
										
										SymbolTableEntry *entry=$1->sym;
										if(entry->type == USERFUNC) 
											insertError("Error! Trying to increment function %s in line %d.\n", entry->value.funcVal->name, yylineno);
										else if(entry->type == LIBFUNC)	
											insertError("Error! Trying to increment library function %s in line %d.\n", entry->value.funcVal->name, yylineno);
										}
										 
									}
	| DECREMENT lvalue 			{	printf("term -> --lvalue\n");
									if($2 != NULL){
										if($2->type == TABLEITEM_E){
											$$ = (expr *)emit_iftableitem($2, curr_scope, yylineno);
											emit(SUB_IO, $$, $$, newexpr_constnum((double)1), -1, yylineno);
											emit(TABLESETELEM, $$, $2, $2->index, -1, yylineno);
										}else{
											emit(SUB_IO, $2, $2, newexpr_constnum((double)1), -1, yylineno);
											$$ = (expr*)newexpr(ARITHEXPR_E);
											$$->sym = (SymbolTableEntry *)new_temp(curr_scope, yylineno);
											emit(ASSIGN, $$, $2, NULL, -1, yylineno);
										}
									
										SymbolTableEntry *entry=$2->sym;
										if(entry->type == USERFUNC) 
											insertError("Error! Trying to decrement function %s in line %d.\n", entry->value.funcVal->name, yylineno);
										else if(entry->type == LIBFUNC)	
											insertError("Error! Trying to decrement library function %s in line %d.\n", entry->value.funcVal->name, yylineno);
									}
										 
								}
	| lvalue DECREMENT 			{	printf("term -> lvalue--\n");
									if($1 != NULL){
										$$ = (expr*)newexpr(VAR_E);
										$$->sym = (SymbolTableEntry *)new_temp(curr_scope, yylineno);

										expr *val;
										if($1->type == TABLEITEM_E){
											val = (expr *)emit_iftableitem($1, curr_scope, yylineno);
											emit(ASSIGN, $$, val, NULL, -1, yylineno);
											emit(SUB_IO, val, val, newexpr_constnum((double)1), -1, NULL);
											emit(TABLESETELEM, val, $1, $1->index, -1, NULL);
										}else{
											emit(ASSIGN, $$, $1, NULL, -1, yylineno);
											emit(SUB_IO, $1, $1, newexpr_constnum((double)1), -1, yylineno);
										}

										SymbolTableEntry *entry=$1->sym;
										if(entry->type == USERFUNC) 
											insertError("Error! Trying to decrement function %s in line %d.\n", entry->value.funcVal->name, yylineno);
										else if(entry->type == LIBFUNC)	
											insertError("Error! Trying to decrement library function %s in line %d.\n", entry->value.funcVal->name, yylineno);
									}
										
								}
	| primary 					{ $$=$1; printf("term -> primary\n"); }
	;

assignexpr: lvalue '=' expr 	
	{	printf("assignexpr -> lvalue = expr\n");
		if($1 != NULL){
			SymbolTableEntry *entry=$1->sym;
			if(entry!=NULL){
				if(entry->type == USERFUNC){
					insertError("Error! Trying to assign value to function %s in line %d.\n", entry->value.funcVal->name, yylineno);
				}
				else if(entry->type == LIBFUNC)	insertError("Error! Trying to assign value to library function %s in line %d.\n", entry->value.funcVal->name, yylineno);
				}
				if($1->type == TABLEITEM_E){
					emit(TABLESETELEM, $1, $3, $1->index, -1, yylineno);
					$$ = (expr *)emit_iftableitem($1, curr_scope, yylineno);
					$$->type = ASSIGNEXPR_E;

				}else{
					emit(ASSIGN, $1, $3, NULL, -1, yylineno);
					$$ = (expr*)newexpr(ASSIGNEXPR_E);
					$$->sym = (SymbolTableEntry *)new_temp(curr_scope, yylineno);
					emit(ASSIGN, $$, $1, NULL, -1, yylineno);
		        } 
	    }
		
	}
	;

primary: lvalue 		{ printf("primary -> lvalue\n"); 
							if($1 != NULL){
								$$ = (expr *)emit_iftableitem($1, curr_scope, yylineno);
							}
						}
	| call 				{ printf("primary -> call\n"); }
	| objectdef 		{ printf("primary -> objectdef\n");  $$=$1; }
	| '(' funcdef ')' 	{ printf("primary -> (funcdef)\n"); 
							$$ = (expr *)newexpr(PROGRAMFUNC_E);
							$$->sym = $2;
						}
	| const 			{$$=$1; printf("primary -> const\n");}
	;

lvalue: ID 				{printf("lvalue -> id\n"); 
						
						SymbolTableEntry *entry;
						
						if(LookUpAll($1,curr_scope,yylineno,0)==1){

							entry = (SymbolTableEntry*)searchAll($1,curr_scope);
							if(entry->type == USERFUNC){
							}else{
								if(entry->value.varVal->scope < scope_function && entry->value.varVal->scope!=0)
									{
									insertError("Error! %s cannot be accessed in line %d. \n", $1, yylineno);
								}	

							}
							
						}else{
							if(curr_scope == 0)
	  						 	entry = (SymbolTableEntry*)insert(true, $1, curr_scope, yylineno, GLOBAL); 
	  						else
	  							entry = (SymbolTableEntry*)insert(true, $1, curr_scope, yylineno, LOCAL_);

	  						entry->scope_space = currscopespace();
	  						entry->offset = currscopeoffset();
	  						incurrscopeoffset();
						}

						$$ = (expr *)lvalue_expr(entry);

					}
	| LOCAL ID 			{printf("lvalue -> local id\n");
						SymbolTableEntry *entry;
						
						if(curr_scope == 0){
						    if(LookUp($2, curr_scope,yylineno,0) == 0){
								entry =(SymbolTableEntry*)insert(true, $2, curr_scope, yylineno, GLOBAL);
								entry->scope_space = currscopespace();
								entry->offset = currscopeoffset();
								incurrscopeoffset();
							}else{
								entry = (SymbolTableEntry*)searchAll($2, curr_scope);
							}
						 }else{
						    if(LookUp($2, curr_scope,yylineno,1) == 0)
								entry = (SymbolTableEntry*)insert(true, $2, curr_scope, yylineno, LOCAL_);
								entry->scope_space = currscopespace();
								entry->offset = currscopeoffset();
								incurrscopeoffset();
						 }
						 $$ = (expr *)lvalue_expr(entry);
						}
	| DOUBLE_COLON ID 	{printf("lvalue -> ::id\n");
						SymbolTableEntry *entry;
						if(LookUp($2, 0,yylineno,0) == 0){
							insertError("Error! No global declaration of token %s in line %d.\n", $2, yylineno);
							$$ = NULL;
						}else{
							entry = (SymbolTableEntry *)refersTo($2, 0);
							$$ = (expr *)lvalue_expr(entry);
						}
						
						}
	| member 			{ printf("lvalue -> member\n"); $$ = $1;}
	;

member: lvalue '.' ID 		{	
							printf("member -> lvalue.id\n");
							if($1 != NULL){
								$$ = (expr*)member_item($1, $3, curr_scope, yylineno);
							}
							}
	| lvalue '[' expr ']' 	{
							printf("member -> lvalue[expr]\n");
							if($1 != NULL){
								$1 = (expr *)emit_iftableitem($1, curr_scope, yylineno);
								$$ = (expr *)newexpr(TABLEITEM_E);
								$$->sym = $1->sym;
								$$->index = $3;
							}
							}
	
	| call '.' ID 			{ printf("member -> call.id\n"); $$ = (expr*)member_item($1, $3, curr_scope, yylineno); }
	| call '[' expr ']' 	{ printf("member -> call[expr]\n");							
								$$ = (expr*)member_item($1, $3, curr_scope, yylineno);
								$$->sym = $1->sym;
								$$->index = $3;
							}
	;

call: call '(' elist ')' 	{ printf("call -> call(elist)\n"); 
										$$ = (expr *)make_call($1, $3, curr_scope, yylineno);
									}
	| lvalue callsuffix 			{ printf("call -> lvalue callsuffix\n"); 
										if($1 != NULL){
											if($2->method){
												expr *self = $1;
												$1 = (expr *)emit_iftableitem(member_item(self, $2->name, curr_scope, yylineno), curr_scope, yylineno);
												$2->elist_t = (expr *)add_front(self);

											}
											$$ = (expr *)make_call($1, $2->elist_t);
										}
									}
	| '(' funcdef ')' '(' elist ')' { printf("call -> (funcdef)(elist)\n"); 
										expr *func = (expr *)newexpr(PROGRAMFUNC_E);
										func->sym = $2;
										$$ = (expr *)make_call(func, $5, curr_scope, yylineno);
									}
	;

callsuffix: normcall 		{ printf("callsuffix -> normcall\n"); $$ = $1;}
		| methodcall 		{ printf("callsuffix ->	methodcall\n"); $$ = $1;}
		;

normcall: '(' elist ')' 	{ printf("normcall -> (elist)\n");
								$$ = malloc(sizeof(call_t));
								$$->elist_t = $2;
								$$->method = 0;
								$$->name = NULL;
							}
		;

methodcall: DOUBLE_DOT ID '(' elist ')' { 	printf("methodcall -> ..id (elist)\n");
											$$ = malloc(sizeof(call_t));
											$$->elist_t = $4;
											$$->method = 1;
											$$->name = strdup($2);
										}
		;

elist:  expr 					{ printf("elist	-> expr\n"); expr_head = NULL; $$ = (expr *)insertExpression($1); }
		| elist  ',' expr 		{ printf("elist	-> elist , expr*\n"); $$ = (expr *)insertExpression($3);}
		|						{ printf("elist	-> empty\n"); $$ = NULL;}
		;

objectdef: '[' elist ']' 		{	printf("objectdef -> [ elist ]\n");
									$$ = (expr*)newexpr(NEWTABLE_E);
									$$->sym = (SymbolTableEntry *)new_temp(curr_scope, yylineno);
									emit(TABLECREATE, $$, NULL, NULL, -1, yylineno);
									double i = 0;
									expr *tmp = expr_head;
									while(tmp != NULL){
										emit(TABLESETELEM, $$, newexpr_constnum(i++), tmp, -1, yylineno);
										tmp = tmp->next;
									}

								}
		| '[' indexed ']' 		{ 	printf("objectdef -> [ indexed ]\n");

									$$ = (expr*)newexpr(NEWTABLE_E);
									$$->sym = (SymbolTableEntry *)new_temp(curr_scope, yylineno);
									emit(TABLECREATE, $$, NULL, NULL, -1, yylineno);
									indexelem * temp=indexelem_head;
									while(temp!=NULL)
									{
									emit(TABLESETELEM, $$, temp->leftexpr, temp->rightexpr, -1, yylineno);
										temp = temp->next;
									}

								}
		;

indexed: indexedelem 			{ printf("indexed -> indexedelem\n");}
		| indexed ',' indexedelem { printf("indexed	-> indexed , indexedelem\n");}
		;

indexedelem: '{' expr ':' expr '}' { 
									printf("indexedelem -> { expr : expr }\n");
									insertIndexelem($2,$4);
									}
			;

block: '{' {curr_scope++; range_insert(scope_function);} statements '}' {$$ = malloc(sizeof(statements)); $$=$3; range_delete(); scope_function=getlastRange(); hide(curr_scope); curr_scope--; printf("block -> { statements }\n");}
		;



statements:  statements stmt	{ printf("statements -> statements stmt\n");
									if($$ == NULL){
										$$ = malloc(sizeof(statements));
										
									}	

									$$->breaklist = (backpatch *)merge($1->breaklist, $2->breaklist);
									printBackpatch($$->breaklist);
									$$->contlist = (backpatch *)merge($1->contlist, $2->contlist);

								}		
			|					{ printf("statements ->	empty\n"); 
										$$ = malloc(sizeof(statements));
										$$->breaklist = NULL;
										$$->contlist = NULL; }
			;


funcname: ID 	{ $$ = $1; }
		|		{ $$ = (char *)generate_name(); } 		
		;

funcprefix: FUNCTION funcname
			{ 
				int ret_val = LookUp($2,curr_scope,yylineno,1);
					if(ret_val == 0){
						$$ = (SymbolTableEntry *)insert(true, $2, curr_scope, yylineno, USERFUNC);
						$$->value.funcVal->iaddress = nextquadlabel(); 
						emit(FUNCSTART, lvalue_expr($$), NULL, NULL, -1, yylineno);

						pushLocalStack(functionLocalOffset);
						enterscopespace();
						resetformalargsoffset();
					}
					else if(ret_val == 1)
						insertError("Error!Redeclaration of token %s in line %d.\n", $2, yylineno); 
			 }

funcargs: '(' idlist ')' {	scope_function = curr_scope + 1;
							enterscopespace();
							resetfunctionlocalsoffset();
							pushLoopCounter(loop_counter);
							loop_counter = 0;
						}
funcbody: block	{
					exitscopespace();
					loop_counter = popLoopCounter();
				}


funcdef: funcprefix funcargs funcbody	{	printf("funcdef -> function id (idlist) block\n");
	
											exitscopespace();
											$1->value.funcVal->totallocals = functionLocalOffset;
											functionLocalOffset = popLocalStack();
											$$=$1;
											emit(FUNCEND, lvalue_expr($1), NULL, NULL,-1,yylineno);
										}

const: INTCONST  			{  	printf("const -> intconst\n"); 
								$$ = (expr*)newexpr(CONSTNUM_E);
								$$->numConst = $1;
							}
		| REALCONST 		{ 	printf("const -> realconst\n");
								$$ = (expr*)newexpr(CONSTNUM_E);
								$$->numConst = $1;						
							}
		| STRING 			{ 	printf("const -> string\n");
								$$ = (expr *)newexpr_conststring($1);
							}
		| NIL 				{ 	printf("const -> nil\n");
								$$ = (expr*)newexpr(NIL_E);
								$$->strConst = strdup("nil");
							}
		| TRUE 				{ 	printf("const -> true\n");
								$$ = (expr*)newexpr(CONSTBOOL_E);
								$$->boolConst=1;
							}
		| FALSE 			{ 	printf("const -> false\n");
								$$ = (expr*)newexpr(CONSTBOOL_E);
								$$->boolConst=0;
							}
		;

idlist: ID 						{curr_scope++; printf("idlist -> id\n");
								SymbolTableEntry *entry;
								int ret_val = LookUp($1, curr_scope,yylineno, 1);
								if(ret_val == 0){
									entry = (SymbolTableEntry *)insert(true, $1, curr_scope, yylineno, FORMAL);
									entry->scope_space = currscopespace();
	  								entry->offset = currscopeoffset();
	  								incurrscopeoffset();
								}
								else if(ret_val == 1)
									insertError("Error! Formal redeclaration of token %s in line %d.\n", $1, yylineno);
								curr_scope--; 
								}
		| idlist ',' ID 		{curr_scope++; printf("idlist -> idlist , id\n");
								SymbolTableEntry *entry;
								int ret_val=LookUp($3, curr_scope,yylineno,1);
								if(ret_val== 0){
									entry = (SymbolTableEntry *)insert (true, $3, curr_scope, yylineno, FORMAL);
									entry->scope_space = currscopespace();
	  								entry->offset = currscopeoffset();
									incurrscopeoffset();
								}
								else if(ret_val==1)
									insertError("Error!Formal redeclaration of token %s in line %d.\n", $3, yylineno);
								curr_scope--; 
								
								}
		|						{printf("idlist	-> empty\n"); }
		;

ifprefix: IF '(' expr ')'	{ 	emit(IF_EQ, NULL, $3, new_expr_constbool(1), nextquadlabel()+2, yylineno);
								$$ =  nextquadlabel();
								emit(JUMP, NULL, NULL, NULL, -1, yylineno);
							}
		;

elseprefix: ELSE 	{ 	$$ = nextquadlabel();
						emit(JUMP, NULL, NULL, NULL, -1, yylineno);
					}
			;

ifstmt: ifprefix stmt 	{ printf("ifstmt -> if ( expr ) stmt\n");
							patchlabel($1, nextquadlabel());
							$$ = $2;
						}
		| ifprefix stmt elseprefix stmt 	{ 	printf("ifstmt -> if ( expr ) stmt else stmt\n");
												$$=malloc(sizeof(statements));
												
												$$->breaklist=(backpatch *)merge($2->breaklist,$4->breaklist);
												$$->contlist=(backpatch *)merge($2->contlist,$4->contlist);
												patchlabel($1, $3+1);
												patchlabel($3, nextquadlabel());
											}
		;

whilestart:	WHILE 	{ $$ = nextquadlabel(); ++loop_counter; }
			;

whilecond: '(' expr ')'	{ 	emit(IF_EQ, NULL, $2, new_expr_constbool(1), nextquadlabel()+2, yylineno);
								$$ = nextquadlabel();
								emit(JUMP, NULL, NULL, NULL, -1, yylineno);
							}
			;

whilestmt: whilestart whilecond stmt 	{ 	
											printf("kakaka");
											printf("whilestmt -> while ( expr ) stmt\n");
											printf("kakaka");
											emit(JUMP, NULL, NULL, NULL, $1, yylineno);
											printf("tevakas");
											patchlabel($2, nextquadlabel());
											$$ = $3;
											printf("eim katw");
											whilepatchlabel($3->breaklist, nextquadlabel());
											printf("sou xtupaw koudouni");
											whilepatchlabel($3->contlist, $1);
											printf("kateva");
											--loop_counter;
										}
		;

N:		{ $$ = nextquadlabel(); emit(JUMP, NULL, NULL, NULL, -1, yylineno); }
	;

M:		{ $$ = nextquadlabel(); }
	;


forprefix: FOR '(' elist ';' M expr ';'		{	$$ = malloc(sizeof(forprefix_t));
												$$->test = $5;
												$$->enter = nextquadlabel();
												emit(IF_EQ, NULL, $6, new_expr_constbool(1), -1, yylineno);
												++loop_counter;
											}
		;

forstmt: forprefix N elist ')' N stmt N 	{printf("forstmt -> for (elist; expr; elist) stmt\n");
												patchlabel($1->enter, $5 + 1);		//true jump
												patchlabel($2, nextquadlabel());	//false jump
												patchlabel($5, $1->test);			//loop jump
												patchlabel($7, $2 + 1);				//closure jump
												$$ = $6;

												whilepatchlabel($6->breaklist, nextquadlabel());	//false jump
												whilepatchlabel($6->contlist, $2 + 1);			//closure jump
												--loop_counter;
											}
		;

returnstmt: RETURN expr ';' { printf("returnstmt -> return expr;\n"); emit(RET, $2, NULL, NULL, -1, yylineno); }
		| RETURN ';' { printf("returnstmt -> return expr;\n"); emit(RET, NULL, NULL, NULL, -1, yylineno); } 
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
	printQuads();
	generate_all();
	create_txt();
	return 0;
}