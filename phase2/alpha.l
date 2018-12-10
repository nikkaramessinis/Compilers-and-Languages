%{
#include <string.h>
#include<stdlib.h>
#include<stdio.h>
#include "parser.h"


/*typedef enum{
IF,
ELSE,
WHILE,
FOR,
FUNCTION,
RETURN,
BREAK,
CONTINUE,
AND,
NOT,
OR,
LOCAL,
TRUE,
FALSE,
NIL,
UNDEFINED_KEY
}keywords;

typedef enum{
ASSIGN,
PLUS,
MINUS,
MULTIPLY,
DIVISION,
MODULO,
EQUAL,
NOT_EQUAL,
INCREMENT,
DECREMENT,
GREATER_THAN,
LESS_THAN,
GREATER_EQUAL,
LESS_EQUAL,
UNDEFINED_OP
}operators;

typedef enum{
LEFT_CURLY_BRACKET,
RIGHT_CURLY_BRACKET,
LEFT_BRACKET,
RIGHT_BRACKET,
LEFT_PARENTHESIS,
RIGHT_PARENTHESIS,
SEMICOLON,
COMMA,
COLON,
DOUBLE_COLON,
DOT, 
DOUBLE_DOT,
UNDEFINED_PUNCT
}punctuation;

typedef enum{
KEYWORD,
OPERATOR,
INTCONST,
REALCONST,
STRING,
PUNCTUATION,
IDENTIFIER,
COMMENT,
UNDEFINED_CAT
}category;*/

int i = 0;
int increment(){
	i++;
	return i;
}


typedef struct comment{
	int start_line;
	struct comment* next;
}comment_t;

comment_t *chead;

void insert_line(int start_line){
	comment_t *tmp = chead;
	if(chead == NULL){
		comment_t *lines = malloc(sizeof(comment_t));
		lines->start_line = start_line;
		lines->next = NULL;
		chead = lines;
	}else{
		while(tmp->next != NULL){
			tmp = tmp->next;
		}
		comment_t *lines = malloc(sizeof(comment_t));
		lines->start_line = start_line;
		tmp->next = lines;
		lines->next = NULL;
	}
}


int getStartLine(){
	comment_t *tmp = chead;
	while(tmp->next != NULL){
		tmp = tmp->next;
	}
		return tmp->start_line;
}


void freeLastElem(){
	comment_t *tmp=chead;
	comment_t *t;
	if(chead->next==NULL){
		free(chead);
		chead=NULL;
	}else{
		while(tmp->next!=NULL){
			t=tmp;
			tmp=tmp->next;
		}
	free(t->next);
	t->next=NULL;
	return;
	}

}



%}

%option noyywrap
%option yylineno


id      [a-zA-Z][a-zA-Z_0-9]*
keyword if|else|while|for|function|return|break|continue|and|not|or|local|true|false|nil
operator "+"|"-"|"="|"*"|"/"|"%"|"=="|"!="|"++"|"--"|">"|"<"|"<="|">="
integer [0-9]+
real [0-9]*\.[0-9](e|E)?[0-9]*
punctuation "{"|"}"|"["|"]"|"("|")"|";"|","|":"|"::"|"."|".."
comment "//".*
whitespace [ \t\v\r]+


%%
{keyword}	{	
				if(strcmp(yytext,"if")==0){
					return IF;
				}
				if(strcmp(yytext,"else")==0){
					return ELSE;
				}
				if(strcmp(yytext,"while")==0){
					return WHILE;
				}
				if(strcmp(yytext,"for")==0){
					return FOR;
				}
				if(strcmp(yytext,"function")==0){
					return FUNCTION;
				}
				if(strcmp(yytext,"return")==0){
					return RETURN;
				}
				if(strcmp(yytext,"break")==0){
					return BREAK;
				}
				if(strcmp(yytext,"continue")==0){
					return CONTINUE;
				}
				if(strcmp(yytext,"and")==0){
					return AND;
				}
				if(strcmp(yytext,"not")==0){
					return NOT;
				}
				if(strcmp(yytext,"or")==0){
					return OR;
				}
				if(strcmp(yytext,"local")==0){
					return LOCAL;
				}
				if(strcmp(yytext,"true")==0){
					return TRUE;
				}
				if(strcmp(yytext,"false")==0){
					return FALSE;
				}
				if(strcmp(yytext,"nil")==0){
					return NIL;
				}
			}

{punctuation} 	{	
					if(strcmp(yytext,"{")==0){
						return yytext[0];
					}
					else if(strcmp(yytext,"}")==0){
						return yytext[0];
					}
					else if(strcmp(yytext,"[")==0){
						return yytext[0];
					}

					else if(strcmp(yytext,"]")==0){
						return yytext[0];
					}

					else if(strcmp(yytext,"(")==0){
						return yytext[0];
					}
					else if(strcmp(yytext,")")==0){
						return yytext[0];
					}
					else if(strcmp(yytext,";")==0){
						return yytext[0];
					}
					else if(strcmp(yytext,",")==0){
						return yytext[0];
					}
					else if(strcmp(yytext,":")==0){
						return yytext[0];
					}
					else if(strcmp(yytext,"::")==0){
						return DOUBLE_COLON;
					}
					else if(strcmp(yytext,".")==0){
						return yytext[0];
					}
					else if(strcmp(yytext,"..")==0){
						return DOUBLE_DOT;
					}

				}
{operator}	{
				if(strcmp(yytext,"=")==0){
					return yytext[0];
				} 
				else if(strcmp(yytext,"+")==0){
					return yytext[0];
				}
				else if(strcmp(yytext,"-")==0){
					return yytext[0];
				}
				else if(strcmp(yytext,"*")==0){
					return yytext[0];
				}
				else if(strcmp(yytext,"/")==0){
					return yytext[0];
				}
				else if(strcmp(yytext,"%")==0){
					return yytext[0];
				}
				else if(strcmp(yytext,"==")==0){
					return EQUAL;
				}
				else if(strcmp(yytext,"!=")==0){
					return NOT_EQUAL;
				}
				else if(strcmp(yytext,"++")==0){
					return INCREMENT;
				}
				else if(strcmp(yytext,"--")==0){
					return DECREMENT;
				}
				else if(strcmp(yytext,">")==0){
					return yytext[0];
				}
				else if(strcmp(yytext,"<")==0){
					return yytext[0];
				}
				else if(strcmp(yytext,"<=")==0){
					return LESS_EQUAL;
				}
				else if(strcmp(yytext,">=")==0){
					return GREATER_EQUAL;
				}
			}

{id}		{
				yylval.stringValue = strdup(yytext);
				return ID;
			}

\"			{	
				int c, i=0;
				int flag = 0;
				int start_l = yylineno;
				int new_size = 2;
				char *mystring = malloc(100 * sizeof(char));
				while((c = input()) != EOF){
					if(c == '\"'){
						flag = 1;
						break;
					}
					if(c == '\\'){
						c = input();
						if(c == 'n'){
							mystring[i] = '\n';
							i++;
						}else if(c == 't'){
							mystring[i] = '\t';
							i++;
						}else if(c == '\\'){
							mystring[i] = '\\';
							i++;
						}else if(c == '"'){
							mystring[i] = '"';
							i++;
						}
						else{
							printf("Error! Illegal escape character in line %d.\n", yylineno);
							exit(0);
						}
					}else{
						mystring[i] = c;
						i++;
					}
					if(i % 100 == 0){
						mystring = realloc(mystring, 100 * new_size * sizeof(char));
						new_size++;
					}
				}
				if(flag == 0){
					printf("Error! Wrong string format in line %d.\n", start_l); 
					exit(0);
				}
				yylval.stringValue = strdup(yytext);
				return STRING;
			}
			
{real}		{
				yylval.realValue = atof(yytext);
				return REALCONST;
			 }		
{integer}	{
			 	yylval.intValue = atoi(yytext);
			 	return INTCONST;
			}

{whitespace}    {}
[\n]+           {}
{comment}		{ }
"/*"	{	
			int flag = 0;
			int c;
			int nestedloop=0;
			int start = yylineno;
			while((c = input()) != EOF){
				if(c =='/'){
					if ((c = input())=='*'){
						nestedloop++;
						insert_line(yylineno);
					}
					else
						unput(c);
				}
				else if(c == '*'){
					if((c = input()) == '/'){
						if(nestedloop==0){
							char *buffer = malloc(20 * sizeof(char));
							char *buffer2 = malloc(20 * sizeof(char));
							if(start == yylineno){
								sprintf(buffer, "%d", start);
							}else{
								sprintf(buffer, "%d", start);
								sprintf(buffer2, "%d", yylineno);
								strcat(buffer, "-");
								strcat(buffer, buffer2);
							}
							flag=1;
							break;
						}else{
							int line = getStartLine();
							freeLastElem();
							char *line_buff = malloc(20 * sizeof(char));
							char *line_buff2 = malloc(20 * sizeof(char));
							if(line == yylineno){
								sprintf(line_buff, "%d", line);
							}else{
								sprintf(line_buff, "%d", line);
								sprintf(line_buff2, "%d", yylineno);
								strcat(line_buff, "-");
								strcat(line_buff, line_buff2);
							}
							nestedloop--;
						}
					}
					else
						unput(c);
				}
			}
			if(nestedloop!=0 || flag==0){printf("Comment doesn't close.\n"); exit(0);}

		}
.		{printf("Undefined input %s in line %d\n",yytext,yylineno);exit(0);}
