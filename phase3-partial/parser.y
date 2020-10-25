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
unsigned int partial =0;

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
%type <intValue> funcbody ifprefix elseprefix whilestart whilecond N M
%type <stringValue> funcname
%type <call> methodcall normcall callsuffix
%type <stmnts> stmt statements whilestmt block ifstmt forstmt
%type <forprefix_t> forprefix

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

stmt: expr ';'			{ printf("stmt -> expr;\n"); $$=malloc(sizeof(statements));
						$$->breaklist=NULL;
						$$->contlist=NULL;
						assistquad * p;
                        if(partial==1){
							if($1->andorflag == 1){

								p = (assistquad *)popassistemit();
								$1->truelist = (backpatch *)pushbackpatchList(nextquadlabel());
								emit(p->op, p->result, p->arg1, p->arg2,p->label, p->line);
								p = (assistquad *)popassistemit();
								$1->falselist = (backpatch *)pushbackpatchList(nextquadlabel());
								emit(p->op, p->result, p->arg1, p->arg2,p->label, p->line);
							}
							if($1->andorflag == 2 || $1->notflag == 1){
								
								p = (assistquad *)popassistemit();
								$1->truelist = (backpatch *)pushbackpatchList(nextquadlabel());
								emit(p->op, p->result, p->arg1, p->arg2,p->label, p->line);
								p = (assistquad *)popassistemit();
								$1->falselist = (backpatch *)pushbackpatchList(nextquadlabel());
								emit(p->op, p->result, p->arg1, p->arg2,p->label, p->line);

								p = (assistquad *)popassistemit();
								$1->truelist = (backpatch *)pushbackpatchList(nextquadlabel());
								emit(p->op, p->result, p->arg1, p->arg2,p->label, p->line);
								p = (assistquad *)popassistemit();
								$1->falselist = (backpatch *)merge($1->falselist, (backpatch *)pushbackpatchList(nextquadlabel()));
								emit(p->op, p->result, p->arg1, p->arg2,p->label, p->line);
							
							 
								if($1->notflag == 1){
									$1->truelist = (backpatch *)pushbackpatchList(nextquadlabel());
		                          
		                            emit(IF_EQ, NULL, $1, new_expr_constbool(1), -1, yylineno);
		                            
									$1->falselist = (backpatch *)pushbackpatchList(nextquadlabel());
		                            emit(JUMP,NULL,NULL,NULL,-1,yylineno);
									backpatch *tmp = $1->falselist;
									$1->falselist = $1->truelist;
									$1->truelist = tmp;
		                        }
							}
							expr *temp = (expr*)newexpr(ASSIGNEXPR_E);
							temp->sym = (SymbolTableEntry *)new_temp(curr_scope, yylineno);
                            
                            backpatchlabel($1->truelist,nextquadlabel());
                            emit(ASSIGN,temp,new_expr_constbool(1),NULL,-1,yylineno);
                            emit(JUMP,NULL,NULL,NULL,nextquadlabel()+2,yylineno);
                            
                            backpatchlabel($1->falselist,nextquadlabel());
                            emit(ASSIGN,temp,new_expr_constbool(0),NULL,-1,yylineno);
                            
                            partial=0;
                         }
						resettemp(); 
						}
	| ifstmt 			{ printf("stmt -> ifstmt\n");$$=malloc(sizeof(statements));
						  $$->breaklist = (backpatch *)merge($$->breaklist, $1->breaklist);
						  $$->contlist = (backpatch *)merge($$->contlist, $1->contlist);
							resettemp();
						}
	| whilestmt 		{ printf("stmt -> whilestmt\n"); $$=malloc(sizeof(statements)); 
						$$->breaklist=NULL;
						$$->contlist=NULL;resettemp();}
	| forstmt 			{ printf("stmt -> forstmt\n");
						$$=malloc(sizeof(statements)); 
						$$->breaklist=NULL;
						$$->contlist=NULL;
						resettemp();
						}
	| returnstmt 		{ printf("stmt -> returnstmt\n");$$=malloc(sizeof(statements));$$->breaklist=NULL;
						$$->contlist=NULL; resettemp();}
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
							$$->breaklist =NULL;
							emit(JUMP, NULL, NULL, NULL, -1, yylineno);
							resettemp();
						}
	| block 			{ printf("stmt -> block\n"); $$=malloc(sizeof(statements)); $$ = $1; resettemp();}
	| funcdef 			{ printf("stmt -> funcdef\n");$$=malloc(sizeof(statements));$$->breaklist=NULL;
						$$->contlist=NULL;resettemp();}
	| ';' 				{ printf("stmt -> ;\n");$$=malloc(sizeof(statements));resettemp();}
	;

expr: assignexpr 		{ $$=$1;
						printf("expr -> assignexpr\n");
						}
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
							pushassistemit(IF_GREATER,NULL, $1, $3, -1, yylineno);
							pushassistemit(JUMP,NULL,NULL, NULL, -1, yylineno);
							$$->andorflag = 1;
							partial = 1;
						}
	| expr '<' expr 	{ 	printf("expr -> expr < expr\n");
							$$ = (expr*)newexpr(BOOLEXPR_E);
							$$->sym = (SymbolTableEntry *)new_temp(curr_scope, yylineno);
							$$->andorflag = 1;
							pushassistemit(IF_LESS,NULL, $1, $3, -1, yylineno);
							pushassistemit(JUMP,NULL, NULL, NULL, -1, yylineno);
							partial = 1;
						}
	| expr GREATER_EQUAL expr 	{ 	printf("expr -> expr >= expr\n"); 
									$$ = (expr*)newexpr(BOOLEXPR_E);
									$$->sym = (SymbolTableEntry *)new_temp(curr_scope, yylineno);
									pushassistemit(IF_LESS,NULL, $1, $3, -1, yylineno);
									pushassistemit(JUMP,NULL, NULL, NULL, -1, yylineno);
									$$->andorflag = 1;
									partial = 1;
								}
	| expr LESS_EQUAL expr 	{ 	printf("expr -> expr <= expr\n"); 
								$$ = (expr*)newexpr(BOOLEXPR_E);
								$$->sym = (SymbolTableEntry *)new_temp(curr_scope, yylineno);
								pushassistemit(IF_LESS,NULL, $1, $3, -1, yylineno);
								pushassistemit(JUMP,NULL, NULL, NULL, -1, yylineno);
								$$->andorflag = 1;
								partial = 1;
							}
	| expr AND M expr 	{ printf("expr -> expr and expr\n");

							$$ = (expr*)newexpr(BOOLEXPR_E);
							//$$->sym = (SymbolTableEntry *)new_temp(curr_scope, yylineno);
							
							if($1->type == BOOLEXPR_E && $1->andorflag == 1){
									pushassistemit(IF_EQ, NULL, $1,  new_expr_constbool(1),nextquadlabel()+2,yylineno);
									pushassistemit(JUMP, NULL, NULL, NULL, nextquadlabel()+1, yylineno);
									$$->andorflag=2;
							}
							if($1->andorflag != 1 && $1->andorflag != 2){
							
								$$->notflag=$1->notflag;
								
								if($1->type != BOOLEXPR_E){
									pushassistemit(IF_EQ, NULL, $1,  new_expr_constbool(1),nextquadlabel()+2,yylineno);
									pushassistemit(JUMP, NULL, NULL, NULL, nextquadlabel()+1, yylineno);
									$$->andorflag=2;
								}
							}
							if($4->andorflag != 1 && $4->andorflag != 2){
							if($4->type != BOOLEXPR_E){
								pushassistemit(IF_EQ, NULL, $4,  new_expr_constbool(1), -1, yylineno);
								pushassistemit(JUMP,NULL,NULL,NULL,-1,yylineno);
								$$->andorflag=2;
                            }
							//epeidh dn tha 3erw sto or pou kanw ta emit an ein sto prwto meros h sto deutero 2 gia deutero 
							if($4->notflag==1){
								$$->notflag=2;
							}
							}

							if($4->type == BOOLEXPR_E && $4->andorflag == 1){printf("MPIKA 4 BOOLEXPR\n");
								pushassistemit(IF_EQ, NULL, $4,  new_expr_constbool(1), -1, yylineno);
								pushassistemit(JUMP,NULL,NULL,NULL,-1,yylineno);
								$$->andorflag=2;
							}

                 
                            $$->truelist=(backpatch *)$4->truelist;
                            $$->falselist=(backpatch *)merge($1->falselist,$4->falselist);
                            partial=1;
						}
	| expr OR M expr 	{ 	printf("expr -> expr or expr\n"); 
							$$ = (expr*)newexpr(BOOLEXPR_E);
							//gia to an ein gg easy
							if($1->andorflag != 1 && $1->andorflag != 2){printf("prwto gg\n");
								if($1->type != BOOLEXPR_E){
									$1->truelist = (backpatch *)pushbackpatchList(nextquadlabel());
									emit(IF_EQ,NULL, $1,  new_expr_constbool(1),nextquadlabel()+2,yylineno);
										
									$1->falselist = (backpatch *)pushbackpatchList(nextquadlabel());
									emit(JUMP, NULL, NULL, NULL, nextquadlabel()+1, yylineno);

									if($1->notflag == 1){
										backpatch *tempo=$1->truelist;
										$1->truelist=$1->falselist;
										$1->falselist=tempo;
										$1->notflag=0;
									}
								}
							}

						
							assistquad *p;
							//gia to an ein suntheto me <> klp
							if($1->type == BOOLEXPR_E && $1->andorflag == 1){printf("prwto suntheto\n");
									p = (assistquad*)popassistemit();
									$1->truelist = (backpatch *)pushbackpatchList(nextquadlabel());
									emit(p->op, p->result, p->arg1, p->arg2, p->label, p->line);
								
									p = (assistquad*)popassistemit();
									$1->falselist = (backpatch *)pushbackpatchList(nextquadlabel());
									emit(p->op, p->result, p->arg1, p->arg2, nextquadlabel()+1, p->line);
									$1->andorflag = 0;
								}
								
									//gia to and otan ein se parapanw kanona and 
								if($1->type == BOOLEXPR_E && $1->andorflag == 2){printf("prwto and\n");
									p = (assistquad*)popassistemit();
									if($1->notflag==1){printf("elalalalalalalala");
									$1->falselist = (backpatch *)pushbackpatchList(nextquadlabel());}else{}
									emit(p->op, p->result, p->arg1, p->arg2, nextquadlabel()+2, p->line);
									
									p = (assistquad*)popassistemit();
									if($1->notflag==1){
										//$1->truelist = (backpatch *)pushbackpatchList(nextquadlabel());
									}else{
										$1->falselist = (backpatch *)pushbackpatchList(nextquadlabel());
										}
									emit(p->op, p->result, p->arg1, p->arg2, nextquadlabel()+1, p->line);
									$1->andorflag = 0;
									p = (assistquad*)popassistemit();
									if($1->notflag==2){$1->falselist =(backpatch *)merge($1->falselist, (backpatch *)pushbackpatchList(nextquadlabel()));}else{
									$1->truelist = (backpatch *)merge($1->truelist,(backpatch *)pushbackpatchList(nextquadlabel()));}
									emit(p->op, p->result, p->arg1, p->arg2, p->label, p->line);
								
									p = (assistquad*)popassistemit();
									/*if($1->notflag==2){$1->truelist = (backpatch *)merge($1->truelist,(backpatch *)pushbackpatchList(nextquadlabel()));}else{
									$1->falselist = (backpatch *)merge($1->falselist,(backpatch *)pushbackpatchList(nextquadlabel()));}*/
									emit(p->op, p->result, p->arg1, p->arg2, nextquadlabel()+1, p->line);
									$1->andorflag = 0;
									if($1->type == BOOLEXPR_E ){
									patchlabel($1->falselist->label,nextquadlabel());
                            }
								
							}
								
								
								
							if($4->type == BOOLEXPR_E && $4->andorflag == 1){printf("deutero suntheto\n");
									p = (assistquad*)popassistemit();
									$4->truelist = (backpatch *)pushbackpatchList(nextquadlabel());
									emit(p->op, p->result, p->arg1, p->arg2, p->label, p->line);
								
									p = (assistquad*)popassistemit(); 
									$4->falselist = (backpatch *)pushbackpatchList(nextquadlabel());
									emit(p->op, p->result, p->arg1, p->arg2, nextquadlabel()+1, p->line);
									$1->andorflag = 0;
								}
								
									if($4->andorflag != 1 && $4->andorflag != 2){printf("deutero gg\n");
								if($4->type != BOOLEXPR_E ){
									$4->truelist = (backpatch *)pushbackpatchList(nextquadlabel());
									emit(IF_EQ, NULL, $4,  new_expr_constbool(1),-1,yylineno);
									
									$4->falselist = (backpatch *)pushbackpatchList(nextquadlabel());
									emit(JUMP,NULL,NULL,NULL,-1,yylineno);

									if($4->notflag == 1){
										backpatch *tempo1=$4->truelist;
										$4->truelist=$4->falselist;
										$4->falselist=tempo1;
										$4->notflag=0;
									}
									
								}

								
							}

							
							if($4->type == BOOLEXPR_E && $4->andorflag == 2){printf("2o and \n\n");
									p = (assistquad*)popassistemit();
									$4->truelist = (backpatch *)pushbackpatchList(nextquadlabel());
									emit(p->op, p->result, p->arg1, p->arg2, nextquadlabel()+2, p->line);
								
									p = (assistquad*)popassistemit(); 
									backpatch * temor= (backpatch *)pushbackpatchList(nextquadlabel());
									emit(p->op, p->result, p->arg1, p->arg2,-1, p->line);
									
									
									p = (assistquad*)popassistemit();
									$4->truelist = (backpatch *)pushbackpatchList(nextquadlabel());
									emit(p->op, p->result, p->arg1, p->arg2, p->label, p->line);
								
									p = (assistquad*)popassistemit(); 
									backpatch * temor1= (backpatch *)pushbackpatchList(nextquadlabel());
									$4->falselist = (backpatch *)merge(temor,temor1);
									emit(p->op, p->result, p->arg1, p->arg2, nextquadlabel()+1, p->line);
									$4->andorflag = 0;
									printBackpatch($4->falselist);
								}	
							$$->truelist=(backpatch *)merge($1->truelist,$4->truelist);
							$$->falselist=$4->falselist;
							partial=1;
						}
	| expr EQUAL expr 	{ 	printf("expr -> expr == expr\n"); 
							$$ = (expr*)newexpr(BOOLEXPR_E);
							$$->sym = (SymbolTableEntry *)new_temp(curr_scope, yylineno);
							printf("pws tha thela na eixa ena");
							assistquad*p;
							if($1->type == BOOLEXPR_E){
								p=(assistquad*)popassistemit();
								$1->truelist = (backpatch *)pushbackpatchList(nextquadlabel());
								emit(p->op,p->result,p->arg1,p->arg2,p->label,p->line);
								
								p=(assistquad*)popassistemit();
								$1->falselist = (backpatch *)pushbackpatchList(nextquadlabel());
								emit(p->op,p->result,p->arg1,p->arg2,p->label,p->line);
							    
							    backpatchlabel($1->truelist,nextquadlabel());
								emit(ASSIGN,$$,new_expr_constbool(1),NULL,-1,yylineno);
								emit(JUMP,NULL,NULL,NULL,nextquadlabel()+2,yylineno);
								
								backpatchlabel($1->falselist,nextquadlabel());
								emit(ASSIGN,$$,new_expr_constbool(0),NULL,-1,yylineno);
							}

							if($3->type==BOOLEXPR_E){
								p=(assistquad*)popassistemit();
								$3->truelist = (backpatch *)pushbackpatchList(nextquadlabel());
								emit(p->op,p->result,p->arg1,p->arg2,p->label,p->line);
								
								p=(assistquad*)popassistemit();
								$3->falselist = (backpatch *)pushbackpatchList(nextquadlabel());
								emit(p->op,p->result,p->arg1,p->arg2,p->label,p->line);
							    
							    backpatchlabel($3->truelist,nextquadlabel());
								emit(ASSIGN,$$,new_expr_constbool(1),NULL,-1,yylineno);
								emit(JUMP,NULL,NULL,NULL,nextquadlabel()+2,yylineno);
								
								backpatchlabel($3->falselist,nextquadlabel());
								emit(ASSIGN,$$,new_expr_constbool(0),NULL,-1,yylineno);
							}
							
							relop_actions($$, IF_EQ, $1, $3, -1, yylineno);
							
							partial = 1;
						}
	| expr NOT_EQUAL expr 	{ 	printf("expr -> expr != expr\n"); 
								$$ = (expr*)newexpr(BOOLEXPR_E);
								$$->sym = (SymbolTableEntry *)new_temp(curr_scope, yylineno);
								relop_actions($$, IF_NOTEQ, $1, $3, -1, yylineno);

								partial = 1;
							}
	| term 					{ $$=$1; printf("expr -> term\n");}
	;


term: '(' expr ')' 				{ printf("term -> (expr)\n"); $$ = $2;}
	| '-' expr %prec UMINUS 	{ 	printf("term -> -expr\n");
									checkuminus($2);
									$$ = (expr*)newexpr(ARITHEXPR_E);
									$$->sym = (SymbolTableEntry *)new_temp(curr_scope, yylineno);
									emit(UMINUS_IO, $$, $2, NULL,  -1, yylineno);
								}
	| NOT expr 					{ printf("term -> not expr\n"); 
									$$ =$2;
                                    //$$->sym = $2->sym;
                                    
                                    partial=1;
									
									$$->notflag=1;
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
					emit(TABLESETELEM, $1,$3, $1->index, -1, yylineno);
					$$ = (expr *)emit_iftableitem($1, curr_scope, yylineno);
					$$->type = ASSIGNEXPR_E;

				}else{
                    if(partial==1){

						if($3->notflag==1){
							
							$3->truelist = (backpatch *)pushbackpatchList(nextquadlabel());
                            emit(IF_EQ, NULL, $3, new_expr_constbool(1),-1,yylineno);
                            
							$3->falselist = (backpatch *)pushbackpatchList(nextquadlabel());
                            emit(JUMP,NULL,NULL,NULL,-1,yylineno);
							backpatch * tmp=$3->falselist;
							$3->falselist=$3->truelist;
							$3->truelist=tmp;
                        }
                            expr *temp = (expr*)newexpr(ASSIGNEXPR_E);
							temp->sym = (SymbolTableEntry *)new_temp(curr_scope, yylineno);
                            
                            backpatchlabel($3->truelist,nextquadlabel());
                            emit(ASSIGN,temp,new_expr_constbool(1),NULL,-1,yylineno);
                            emit(JUMP,NULL,NULL,NULL,nextquadlabel()+2,yylineno);
                            
                            backpatchlabel($3->falselist,nextquadlabel());
                            emit(ASSIGN,temp,new_expr_constbool(0),NULL,-1,yylineno);
                            partial=0;
							$3=temp;
                    }
                    
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
								$$ = (expr *)emit_iftableitem($1);
							}
						}
	| call 				{ printf("primary -> call\n"); $$=$1;}
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
								if(LookUpAll($1->sym, curr_scope,yylineno,0) == 0){
									if(curr_scope == 0)
										insert(true, $1->sym, curr_scope, yylineno, GLOBAL);
									else
										insert(true, $1->sym, curr_scope, yylineno, LOCAL_);
								}
								$$ = (expr*)member_item($1, $3, curr_scope, yylineno);
							}
							}
	| lvalue '[' expr ']' 	{
							printf("member -> lvalue[expr]\n");
							if($1 != NULL){
								if(LookUpAll($1->sym, curr_scope,yylineno,0) == 0){
									if(curr_scope == 0)
										insert(true, $1->sym, curr_scope, yylineno, GLOBAL);
									else
										insert(true, $1->sym, curr_scope, yylineno, LOCAL_);
								}
								$1 = (expr *)emit_iftableitem($1);
								$$ = (expr *)newexpr(TABLEITEM_E);
								$$->sym = $1->sym;
								$$->index = $3;
							}
							}
	
	| call '.' ID 			{ printf("member -> call.id\n"); $$ = (expr*)member_item($1, $3, curr_scope, yylineno); }
	| call '[' expr ']' 	{ printf("member -> call[expr]\n");
							$$ = (expr*)member_item($1, $3, curr_scope, yylineno);
							$$->sym=$1->sym;
							$$->index=$3;
							}
	;

call: call '(' elist ')' 	{ printf("call -> call(elist)\n"); 
										$$ = (expr *)make_call($1, $3, curr_scope, yylineno);
									}
	| lvalue callsuffix 			{ printf("call -> lvalue callsuffix\n"); 
										if($1 != NULL){
											if($2->method){
												expr *self = $1;
												$1 = (expr *)emit_iftableitem(member_item(self, $2->name), curr_scope, yylineno);
												$2->elist_t = (expr *)add_front(self);

											}
											$$ = (expr *)make_call($1, $2->elist_t);
										}
									}
	| '(' funcdef ')' '(' elist ')' { printf("call -> (funcdef)(elist)\n"); 
										expr *func = (expr *)newexpr(PROGRAMFUNC_E);
										func->sym = $2;
										
										$$ = (expr *)make_call(func, $5, curr_scope, yylineno);
										printf("1oooo%s\n",$$->sym->value.funcVal->name);
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

block: '{' {curr_scope++; range_insert(scope_function);} statements '}' {$$=$3; range_delete(); scope_function=getlastRange(); hide(curr_scope); curr_scope--; printf("block -> { statements }\n");}
		;



statements:  statements stmt	{ printf("statements -> statements stmt\n");
									if($$ == NULL){
									$$ = malloc(sizeof(statements));
									$1 = malloc(sizeof(statements));
									}	
									$$->breaklist = (backpatch *)merge($1->breaklist, $2->breaklist);
									$$->contlist = (backpatch *)merge($1->contlist, $2->contlist); 
									
								}		
			|					{ printf("statements ->	empty\n"); $$ = NULL; }
			;


funcname: ID 	{ $$ = $1; }
		|		{ $$ = (char *)generate_name(); } 		
		;

funcprefix: FUNCTION funcname
			{ 
				int ret_val = LookUp($2,curr_scope,yylineno,1);
					if(ret_val == 0){
						$$ = (SymbolTableEntry *)insert(true, $2, curr_scope, yylineno, USERFUNC);
						$$->value.funcVal->iaddress = nextquadlabel(); //TODO
						emit(FUNCSTART,lvalue_expr($$), NULL, NULL, -1, yylineno);
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


funcdef: funcprefix funcargs funcbody
{	printf("funcdef -> function id (idlist) block\n");
	
	exitscopespace();
	$1->value.funcVal->totallocals = functionLocalOffset;
	functionLocalOffset = popLocalStack();
	$$=$1;
	emit(FUNCEND, lvalue_expr($1),NULL, NULL,-1,yylineno);
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
								curr_scope--; }
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
		|						{printf("idlist	-> empty\n");}
		;

ifprefix: IF '(' expr ')'	{ 	 printf("ifstmt -> if ( expr ) stmt\n");
                             	$3->sym = (SymbolTableEntry *)new_temp(curr_scope, yylineno);
                             if( $3->notflag==1 || $3->type == BOOLEXPR_E  ){
						 
							if($3->notflag == 1){
								$3->truelist = (backpatch *)pushbackpatchList(nextquadlabel());
	                          
	                            emit(IF_EQ, $3, new_expr_constbool(1), NULL, -1, yylineno);
	                            
								$3->falselist = (backpatch *)pushbackpatchList(nextquadlabel());
	                            emit(JUMP,NULL,NULL,NULL,-1,yylineno);
								backpatch *tmp = $3->falselist;
								$3->falselist = $3->truelist;
								$3->truelist = tmp;
	                        }
							
							expr *temp = (expr*)newexpr(BOOLEXPR_E);
							temp->sym = (SymbolTableEntry *)new_temp(curr_scope, yylineno);
                            backpatchlabel($3->truelist,nextquadlabel());
                            emit(ASSIGN,temp,new_expr_constbool(1),NULL,-1,yylineno);
                            emit(JUMP,NULL,NULL,NULL,nextquadlabel()+2,yylineno);
                            backpatchlabel($3->falselist,nextquadlabel());
                            emit(ASSIGN,temp,new_expr_constbool(0),NULL,-1,yylineno);
                            
                            partial=0;
                         }
                                emit(IF_EQ, NULL, $3, new_expr_constbool(1), nextquadlabel()+2, yylineno);
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

whilecond: '(' expr ')'	{ 	$2->sym = (SymbolTableEntry *)new_temp(curr_scope, yylineno);
                            if( $2->notflag==1 || $2->type == BOOLEXPR_E){
								if($2->notflag == 1){
									$2->truelist = (backpatch *)pushbackpatchList(nextquadlabel());
		                          	emit(IF_EQ, $2, new_expr_constbool(1), NULL, -1, yylineno);
		                            
									$2->falselist = (backpatch *)pushbackpatchList(nextquadlabel());
		                            emit(JUMP,NULL,NULL,NULL,-1,yylineno);
									
									backpatch *tmp = $2->falselist;
									$2->falselist = $2->truelist;
									$2->truelist = tmp;
		                        }
								
								expr *temp = (expr*)newexpr(BOOLEXPR_E);
								temp->sym = (SymbolTableEntry *)new_temp(curr_scope, yylineno);
	                            
	                            backpatchlabel($2->truelist,nextquadlabel());
	                            emit(ASSIGN,temp,new_expr_constbool(1),NULL,-1,yylineno);
	                            emit(JUMP,NULL,NULL,NULL,nextquadlabel()+2,yylineno);
	                            backpatchlabel($2->falselist,nextquadlabel());
	                            emit(ASSIGN,temp,new_expr_constbool(0),NULL,-1,yylineno);
	                            
	                            partial=0;
                         	}

							emit(IF_EQ, NULL, $2, new_expr_constbool(1), nextquadlabel()+2, yylineno);
							$$ = nextquadlabel();
							emit(JUMP, NULL, NULL, NULL, -1, yylineno);
						}
			;

whilestmt: whilestart whilecond stmt 	{ 	printf("whilestmt -> while ( expr ) stmt\n");
											
											emit(JUMP, NULL, NULL, NULL, $1, yylineno);
											patchlabel($2, nextquadlabel());
											$$ = $3;
											backpatchlabel($3->breaklist, nextquadlabel());
											backpatchlabel($3->contlist, $1);
											--loop_counter;
										}
		;

N:		{ $$ = nextquadlabel(); emit(JUMP, NULL, NULL, NULL, -1, yylineno); }
	;

M:		{ $$ = nextquadlabel(); }
	;


forprefix: FOR '(' elist ';' M expr ';'		{	
											$$ = malloc(sizeof(forprefix_t));
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
												$$=$6;
												backpatchlabel($6->breaklist, nextquadlabel());	//false jump
												backpatchlabel($6->contlist, $2 + 1);			//closure jump
												--loop_counter;
											}
		;

returnstmt: RETURN expr ';' { printf("returnstmt -> return expr;\n");

								emit(RET, $2, NULL, NULL, -1, yylineno); 
							}
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
	return 0;
}
