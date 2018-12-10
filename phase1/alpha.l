%{
#include <string.h>
#include<stdlib.h>
#include<stdio.h>
#define YY_DECL int alpha_yylex(void *ylval)

typedef enum{
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
}category;

int i = 0;
int increment(){
	i++;
	return i;
}

typedef struct token{
	int line_no;
	int token_no;
	char *description;
	category cat;
	void *val;
	keywords key;
	punctuation pun;
	operators op;
	struct token *next;
}alpha_token_t;

alpha_token_t *head;

void insert(int line_no, int token_no, char *description, category cat, void *val, keywords key, punctuation pun, operators op){
	
	alpha_token_t *tmp = head;
	
	if(head == NULL){
		alpha_token_t *list = malloc(sizeof(alpha_token_t));
		list->line_no = line_no;
		list->token_no = token_no;
		list->cat = cat;
		list->description = strdup(description);
		list->key = key;
		list->pun = pun;
		list->op = op;
		if(cat == KEYWORD || cat == PUNCTUATION || cat == OPERATOR || cat==COMMENT){
			list->val = val;
		}else if(cat == IDENTIFIER || cat==STRING ){
			list->val = strdup(val);
		}
		else if(cat == INTCONST){
			list->val=(int*)val;
		}else if(cat == REALCONST){
			list->val = (double*)val;
		}else if(cat == STRING){
			list->val = strdup(val);
		}
		list->next=NULL;
		head=list;
		}
	else{
		while(tmp->next != NULL){
			tmp = tmp->next;
		}
		alpha_token_t *list = malloc(sizeof(alpha_token_t));
		list->line_no = line_no;
		list->token_no = token_no;
		list->cat = cat;
		list->description = strdup(description);
		list->key = key;
		list->pun = pun;
		list->op = op;
		list->next = NULL;
		if(cat == KEYWORD || cat == PUNCTUATION || cat == OPERATOR || cat==COMMENT){
			list->val = val;;
		}else if(cat == IDENTIFIER){
			list->val = strdup(val);
		}
		else if(cat == INTCONST){
			list->val=(int*)val;
		}
		else if(cat == REALCONST){
			list->val = (double*)val;
		}else if(cat == STRING){
			list->val = strdup(val);
		}
		tmp->next = list;
	
	}
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


char *stringify_keywords(keywords key){
	switch(key){
		case IF: return "IF";
		case ELSE: return "ELSE";
		case WHILE: return "WHILE";
		case FOR: return "FOR";
		case FUNCTION: return "FUNCTION";
		case RETURN: return "RETURN";
		case BREAK: return "BREAK";
		case CONTINUE: return "CONTINUE";
		case AND: return "AND";
		case NOT: return "NOT";
		case OR: return "OR";
		case LOCAL: return "LOCAL";
		case TRUE: return "TRUE";
		case FALSE: return "FALSE";
		case NIL: return "NIL";
		default: return "UNDEFINED_KEY";
	}
}


char *stringify_category(category cat){
	switch(cat){
		case INTCONST: return "INTCONST";
		case REALCONST: return "REALCONST";
		case STRING: return "STRING";
		case IDENTIFIER: return "IDENTIFIER";
		case COMMENT: return "COMMENT";
		case OPERATOR: return "OPERATOR";
		case PUNCTUATION: return "PUNCTUATION";
		case KEYWORD: return "KEYWORD";
		default: return "UNDEFINED_CAT";
	}
}

char *stringify_punctuation(punctuation pun){
	switch(pun){
		case LEFT_CURLY_BRACKET: return "{";
		case RIGHT_CURLY_BRACKET: return "}";
		case LEFT_BRACKET: return "[";
		case RIGHT_BRACKET: return "]";
		case LEFT_PARENTHESIS: return "(";
		case RIGHT_PARENTHESIS: return ")";
		case SEMICOLON: return ";";
		case COMMA: return ",";
		case COLON: return ":";
		case DOUBLE_COLON: return "::";
		case DOT: return ".";
		case DOUBLE_DOT: return "..";
		default: return "UNDEFINED_PUNCT";
	}
	
}

char *stringify_operator(operators op){
	switch(op){
		case ASSIGN: return "=";
		case PLUS: return "+";
		case MINUS: return "-";
		case MULTIPLY: return "*";
		case DIVISION: return "/";
		case MODULO: return "%";
		case EQUAL: return "==";
		case NOT_EQUAL: return "!=";
		case INCREMENT: return "++";
		case DECREMENT: return "--";
		case GREATER_THAN: return ">";
		case LESS_THAN: return "<";
		case GREATER_EQUAL: return ">=";
		case LESS_EQUAL: return "<=";
		default: return "UNDEFINED_OP";
	}
}

void PrintList(){
    
	alpha_token_t *tmp = head;
	int number = 0;
    while(tmp != NULL){
		if(tmp->cat == KEYWORD){
			printf("%d: #%d \"%s\" %s %s <-enumerated\n",tmp->line_no, tmp->token_no, tmp->description, stringify_category(tmp->cat), stringify_keywords(tmp->key));
		}
		else if(tmp->cat == PUNCTUATION){
			printf("%d: #%d \"%s\" %s %s <-enumerated\n",tmp->line_no, tmp->token_no, tmp->description, stringify_category(tmp->cat), stringify_punctuation(tmp->pun));
		}
		else if(tmp->cat == OPERATOR){
			printf("%d: #%d \"%s\" %s %s <-enumerated\n",tmp->line_no, tmp->token_no, tmp->description, stringify_category(tmp->cat), stringify_operator(tmp->op));
		}
		else if(tmp->cat == IDENTIFIER){
			printf("%d: #%d \"%s\" %s %s <-char *\n",tmp->line_no, tmp->token_no, tmp->description, stringify_category(tmp->cat) ,tmp->val);
		}
		else if(tmp->cat == INTCONST){
			printf("%d: #%d \"%s\" %s %d <-integer\n",tmp->line_no, tmp->token_no, tmp->description, stringify_category(tmp->cat), tmp->val);
		}
		else if(tmp->cat == REALCONST){
			double value=*(double *)tmp->val;
			printf("%d: #%d \"%s\" %s %lf <-double\n",tmp->line_no, tmp->token_no, tmp->description, stringify_category(tmp->cat), value);
		}
		else if(tmp->cat == COMMENT){
			printf("%d: #%d \"%s\" %s %s <-enumerated\n",tmp->line_no, tmp->token_no, tmp->description, stringify_category(tmp->cat), tmp->val);
		}
		else if(tmp->cat == STRING){
			printf("%d: #%d \"%s\" %s %s <-char *\n",tmp->line_no, tmp->token_no, tmp->description, stringify_category(tmp->cat), tmp->val);
		}
		tmp = tmp->next;
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
		free(chead);chead=NULL;
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


void cleanlist(alpha_token_t *head){
 
	alpha_token_t *node;
		
	while(head!=NULL){
		node=head;
		head=head->next;
		free(node);
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
					insert(yylineno, increment(), yytext, KEYWORD, NULL, IF, UNDEFINED_PUNCT, UNDEFINED_OP);
				}
				if(strcmp(yytext,"else")==0){
					insert(yylineno, increment(), yytext, KEYWORD, NULL, ELSE, UNDEFINED_PUNCT, UNDEFINED_OP);
				}
				if(strcmp(yytext,"while")==0){
					insert(yylineno, increment(), yytext, KEYWORD, NULL, WHILE, UNDEFINED_PUNCT, UNDEFINED_OP);
				}
				if(strcmp(yytext,"for")==0){
					insert(yylineno, increment(), yytext, KEYWORD, NULL, FOR, UNDEFINED_PUNCT, UNDEFINED_OP);
				}
				if(strcmp(yytext,"function")==0){
					insert(yylineno, increment(), yytext, KEYWORD, NULL, FUNCTION, UNDEFINED_PUNCT, UNDEFINED_OP);
				}
				if(strcmp(yytext,"return")==0){
					insert(yylineno, increment(), yytext, KEYWORD, NULL, RETURN, UNDEFINED_PUNCT, UNDEFINED_OP);
				}
				if(strcmp(yytext,"break")==0){
					insert(yylineno, increment(), yytext, KEYWORD, NULL, BREAK, UNDEFINED_PUNCT, UNDEFINED_OP);
				}
				if(strcmp(yytext,"continue")==0){
					insert(yylineno, increment(), yytext, KEYWORD, NULL, CONTINUE, UNDEFINED_PUNCT, UNDEFINED_OP);
				}
				if(strcmp(yytext,"and")==0){
					insert(yylineno, increment(), yytext, KEYWORD, NULL, AND, UNDEFINED_PUNCT, UNDEFINED_OP);
				}
				if(strcmp(yytext,"not")==0){
					insert(yylineno, increment(), yytext, KEYWORD, NULL, NOT, UNDEFINED_PUNCT, UNDEFINED_OP);
				}
				if(strcmp(yytext,"or")==0){
					insert(yylineno, increment(), yytext, KEYWORD, NULL, OR, UNDEFINED_PUNCT, UNDEFINED_OP);
				}
				if(strcmp(yytext,"local")==0){
					insert(yylineno, increment(), yytext, KEYWORD, NULL, LOCAL, UNDEFINED_PUNCT, UNDEFINED_OP);
				}
				if(strcmp(yytext,"true")==0){
					insert(yylineno, increment(), yytext, KEYWORD, NULL, TRUE, UNDEFINED_PUNCT, UNDEFINED_OP);
				}
				if(strcmp(yytext,"false")==0){
					insert(yylineno, increment(), yytext, KEYWORD, NULL, FALSE, UNDEFINED_PUNCT, UNDEFINED_OP);
				}
				if(strcmp(yytext,"nil")==0){
					insert(yylineno, increment(), yytext, KEYWORD, NULL, NIL, UNDEFINED_PUNCT, UNDEFINED_OP);
				}
			}

{punctuation} 	{	
					if(strcmp(yytext,"{")==0){
						insert(yylineno, increment(), yytext, PUNCTUATION, NULL, UNDEFINED_KEY, LEFT_CURLY_BRACKET, UNDEFINED_OP);
					}
					else if(strcmp(yytext,"}")==0){
						insert(yylineno, increment(), yytext, PUNCTUATION, NULL, UNDEFINED_KEY, RIGHT_CURLY_BRACKET, UNDEFINED_OP);
					}
					else if(strcmp(yytext,"[")==0){
						insert(yylineno, increment(), yytext, PUNCTUATION, NULL, UNDEFINED_KEY, LEFT_BRACKET, UNDEFINED_OP);
					}

					else if(strcmp(yytext,"]")==0){
						insert(yylineno, increment(), yytext, PUNCTUATION, NULL, UNDEFINED_KEY, RIGHT_BRACKET, UNDEFINED_OP);
					}

					else if(strcmp(yytext,"(")==0){
						insert(yylineno, increment(), yytext, PUNCTUATION, NULL, UNDEFINED_KEY, LEFT_PARENTHESIS, UNDEFINED_OP);
					}
					else if(strcmp(yytext,")")==0){
						insert(yylineno, increment(), yytext, PUNCTUATION, NULL, UNDEFINED_KEY, RIGHT_PARENTHESIS, UNDEFINED_OP);
					}
					else if(strcmp(yytext,";")==0){
						insert(yylineno, increment(), yytext, PUNCTUATION, NULL, UNDEFINED_KEY, SEMICOLON, UNDEFINED_OP);
					}
					else if(strcmp(yytext,",")==0){
						insert(yylineno, increment(), yytext, PUNCTUATION, NULL, UNDEFINED_KEY, COMMA, UNDEFINED_OP);
					}
					else if(strcmp(yytext,":")==0){
						insert(yylineno, increment(), yytext, PUNCTUATION, NULL, UNDEFINED_KEY, COLON, UNDEFINED_OP);
					}
					else if(strcmp(yytext,"::")==0){
						insert(yylineno, increment(), yytext, PUNCTUATION, NULL, UNDEFINED_KEY, DOUBLE_COLON, UNDEFINED_OP);
					}
					else if(strcmp(yytext,".")==0){
						insert(yylineno, increment(), yytext, PUNCTUATION, NULL, UNDEFINED_KEY, DOT, UNDEFINED_OP);
					}
					else if(strcmp(yytext,"..")==0){
						insert(yylineno, increment(), yytext, PUNCTUATION, NULL, UNDEFINED_KEY, DOUBLE_DOT, UNDEFINED_OP);
					}

				}
{operator}	{
				if(strcmp(yytext,"=")==0){
					insert(yylineno, increment(), yytext, OPERATOR, NULL, UNDEFINED_KEY, UNDEFINED_PUNCT, ASSIGN);
				} 
				else if(strcmp(yytext,"+")==0){
					insert(yylineno, increment(), yytext, OPERATOR, NULL, UNDEFINED_KEY, UNDEFINED_PUNCT, PLUS);
				}
				else if(strcmp(yytext,"-")==0){
					insert(yylineno, increment(), yytext, OPERATOR, NULL, UNDEFINED_KEY, UNDEFINED_PUNCT, MINUS);
				}
				else if(strcmp(yytext,"*")==0){
					insert(yylineno, increment(), yytext, OPERATOR, NULL, UNDEFINED_KEY, UNDEFINED_PUNCT, MULTIPLY);
				}
				else if(strcmp(yytext,"/")==0){
					insert(yylineno, increment(), yytext, OPERATOR, NULL, UNDEFINED_KEY, UNDEFINED_PUNCT, DIVISION);
				}
				else if(strcmp(yytext,"%")==0){
					insert(yylineno, increment(), yytext, OPERATOR, NULL, UNDEFINED_KEY, UNDEFINED_PUNCT, MODULO);
				}
				else if(strcmp(yytext,"==")==0){
					insert(yylineno, increment(), yytext, OPERATOR, NULL, UNDEFINED_KEY, UNDEFINED_PUNCT, EQUAL);
				}
				else if(strcmp(yytext,"!=")==0){
					insert(yylineno, increment(), yytext, OPERATOR, NULL, UNDEFINED_KEY, UNDEFINED_PUNCT, NOT_EQUAL);
				}
				else if(strcmp(yytext,"++")==0){
					insert(yylineno, increment(), yytext, OPERATOR, NULL, UNDEFINED_KEY, UNDEFINED_PUNCT, INCREMENT);
				}
				else if(strcmp(yytext,"--")==0){
					insert(yylineno, increment(), yytext, OPERATOR, NULL, UNDEFINED_KEY, UNDEFINED_PUNCT, DECREMENT);
				}
				else if(strcmp(yytext,">")==0){
					insert(yylineno, increment(), yytext, OPERATOR, NULL, UNDEFINED_KEY, UNDEFINED_PUNCT, GREATER_THAN);
				}
				else if(strcmp(yytext,"<")==0){
					insert(yylineno, increment(), yytext, OPERATOR, NULL, UNDEFINED_KEY, UNDEFINED_PUNCT, LESS_THAN);
				}
				else if(strcmp(yytext,"<=")==0){
					insert(yylineno, increment(), yytext, OPERATOR, NULL, UNDEFINED_KEY, UNDEFINED_PUNCT, LESS_EQUAL);
				}
				else if(strcmp(yytext,">=")==0){
					insert(yylineno, increment(), yytext, OPERATOR, NULL, UNDEFINED_KEY, UNDEFINED_PUNCT, GREATER_EQUAL);
				}
			}

{id}		{
				insert(yylineno, increment(), yytext, IDENTIFIER, yytext, UNDEFINED_KEY, UNDEFINED_PUNCT, UNDEFINED_OP);
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
				insert(yylineno, increment(), mystring, STRING, mystring, UNDEFINED_KEY, UNDEFINED_PUNCT, UNDEFINED_OP);
			}
			
{real}		{		double y = atof(yytext);   
					double *f = malloc(sizeof(double));
					*f = y;
			
				insert(yylineno, increment(), yytext, REALCONST, f, UNDEFINED_KEY, UNDEFINED_PUNCT, UNDEFINED_OP);
			 }		
{integer}	{int *x = (int *)atoi(yytext);  insert(yylineno, increment(), yytext, INTCONST, x, UNDEFINED_KEY, UNDEFINED_PUNCT, UNDEFINED_OP);}

{whitespace}    {}
"\n"            { }
{comment}		{insert(yylineno, increment(), " " , COMMENT, "LINE_COMMENT", UNDEFINED_KEY, UNDEFINED_PUNCT, UNDEFINED_OP);}
"/*"	{	
			int flag = 0;
			int c;int nestedloop=0;
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
							insert(yylineno, increment(), buffer, COMMENT, "BLOCK_COMMENT", UNDEFINED_KEY, UNDEFINED_PUNCT, UNDEFINED_OP);
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
							insert(yylineno, increment(), line_buff , COMMENT, "BLOCK_NESTED_COMMENT", UNDEFINED_KEY, UNDEFINED_PUNCT, UNDEFINED_OP);
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
%%



int main(int argc,char**argv)
{
	if (argc > 1){
		if(!(yyin = fopen(argv[1],"r"))){
			fprintf(stderr,"Cannot read file: %s\n",argv[1]);
			return 1;
			}
	}
	else{
		yyin=stdin;
	}
		
	alpha_yylex(head);
	PrintList();
	cleanlist(head);
	return 0;

}