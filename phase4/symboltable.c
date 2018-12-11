#include "symboltable.h"

SymbolTableEntry* hashArray[MAX_SIZE];
char **Error_tbl;
int counter=1;
int error_counter=1;

int hash_func(char *token) {
	char *i = token;
	int size = strlen(token);
	int total = 0;
	int y;
	for (y = 0; y < size; y++) {
		total += (int)i[y];
	}


	return (total % 500);
}

char* generate_name(){
	char *str=malloc(sizeof(4*sizeof(char)));
	sprintf(str, "$f%d", counter);
	counter++;
	return str;
}


void hide(int scope) {
	
	LinkedListScope *tmp = headList;
	
	SymbolTableEntry* tmpentry = NULL;

	if (headList == NULL) {
		return;
	} else {
		while (tmp != NULL && tmp->scope != scope) {
			tmp = tmp->next;
		}
		if (tmp == NULL) {
			return;
		} else if (tmp->scope == scope) {
			tmpentry = tmp->token;
			while (tmpentry != NULL) {	
				tmpentry->isActive = false;	
				tmpentry = tmpentry->scopenext;
			}
		}
	}

}
int length=0;
void insertError(char *error, const char *token, int line){
	length = strlen(error)+ strlen(token);
	Error_tbl = realloc(Error_tbl, (error_counter+1) * length * sizeof(char *));
	Error_tbl[error_counter-1] = malloc(length * sizeof(char ));
	sprintf(Error_tbl[error_counter-1], error, token, line);
	error_counter++;
}

int LookUpLibFunc(char *name, int localFlag, int line) {
	
	int flag = 0;

	
		if (strcmp(name , "print") == 0) flag = 1;
		else if (strcmp(name , "input") == 0) flag = 1;
		else if (strcmp(name , "objectmemberkeys" ) == 0) flag = 1;
		else if (strcmp(name , "objecttotalmembers") == 0) flag = 1;
		else if (strcmp(name , "objectcopy") == 0) flag = 1;
		else if (strcmp(name , "argument" ) == 0) flag = 1;
		else if (strcmp(name , "totalarguments") == 0) flag = 1;
		else if (strcmp(name , "typeof") == 0) flag = 1;
		else if (strcmp(name , "strtonum" ) == 0) flag = 1;
		else if (strcmp(name , "sqrt") == 0) flag = 1;
		else if (strcmp(name , "cos" ) == 0) flag = 1;
		else if (strcmp(name , "sin") == 0) flag = 1;
	

	if(flag == 1 && localFlag == 1){
	
			insertError("Error! Trying to shadow library function with token %s in line %d.\n", name, line);
		
		return 2;
	}else if(flag == 1 && localFlag == 0){
		
			//insertError("Error! Trying to shadow library function with token %s in line %d.\n",name, line);
		
		return 1;
	}else{
		return 0;
	}
	
}

int LookUp(char *name, int scope, int line, int localFlag) {
	
	LinkedListScope *tmp = headList;
	SymbolTableEntry *tmpentry = NULL;
	
	if(LookUpLibFunc(name,localFlag,line)==2){
		return 2;
	}
	
	if (headList == NULL) {
		return 0;
	} else {
		while (tmp != NULL && tmp->scope != scope) {
			tmp = tmp->next;
		}
		if (tmp == NULL) {
			return 0;
		} else if (tmp->scope == scope) {
			tmpentry = tmp->token;
			while (tmpentry != NULL) {
				if (tmpentry->type == USERFUNC || tmpentry-> type == LIBFUNC) {
					if (strcmp(tmpentry->value.funcVal->name, name)==0 && tmpentry->isActive == true){
						return 1;
					}

				} else {
					if (strcmp(tmpentry->value.varVal->name, name)==0 && tmpentry->isActive == true){
						return 1;
					}
				}
				tmpentry = tmpentry->scopenext;
			}
			return 0;
		}
	}

}

int LookUpAll(char *name, int scope,int line, int localFlag){

	int i;
	for(i = scope; i >= 0; i--){
		if(LookUp(name, i, line, localFlag) == 1 || LookUp(name, i, line, localFlag) == 2){
			return 1;
		}
	}
	return 0;
}

SymbolTableEntry *refersTo(char *name, int scope){

	LinkedListScope *tmp = headList;
	SymbolTableEntry *tmpentry = NULL;
	
	if (headList == NULL) {
		return NULL;
	} else {
		while (tmp != NULL && tmp->scope != scope) {
			tmp = tmp->next;
		}
		if (tmp == NULL) {
			return NULL;
		} else if (tmp->scope == scope) {
			tmpentry = tmp->token;
			while (tmpentry != NULL) {
				if (tmpentry->type == USERFUNC) {
					if (strcmp(tmpentry->value.funcVal->name, name)==0){
						return tmpentry;
					}

				} else {
					if (strcmp(tmpentry->value.varVal->name, name)==0){
						return tmpentry;
					}
				}
				tmpentry = tmpentry->scopenext;
			}
			return NULL;
		}

	}
}

SymbolTableEntry *searchAll(char *name, int scope){
	int i;
	for(i = scope; i >= 0; i--){
		SymbolTableEntry *node = malloc(sizeof(SymbolTableEntry));
		node = refersTo(name, i);
		if(node != NULL){
			return node;
		}
	}
	return NULL;
}


void range_insert(int scopeFunction){
	
	FunctionRange *node=malloc(sizeof(FunctionRange));
	node->scopeFunction=scopeFunction;
    node->next=NULL;
	FunctionRange *tmp=f_head;
	if(f_head==NULL){
		f_head=node;
	}else{
		while(tmp->next!=NULL){
			tmp=tmp->next;
		}
		tmp->next=node;
	}
	
	
}

void range_delete(){
	FunctionRange *tmp = f_head;
	FunctionRange *prev = f_head;
	if(f_head==NULL){
		return;
	}else{
		if(f_head->next==NULL){
			free(f_head);
			f_head=NULL;
		}else{
			while(tmp->next!=NULL){
				prev=tmp;
				tmp=tmp->next;
			}
			free(prev->next);
			prev->next=NULL;
			return;
		}
	}
}
int  getlastRange(){
	FunctionRange *tmp = f_head;
	int range;
	if(f_head==NULL){
		return 0;
	}else{
		while(tmp->next!=NULL){
		tmp=tmp->next;
		}
		range=tmp->scopeFunction;
		return range;
	}
	
}

void scope_insert(SymbolTableEntry *token) {


	if (headList == NULL) {
		LinkedListScope *node = malloc(sizeof(LinkedListScope));
		node->next = NULL;
		node->token = token;
		node->scope = 0;
		headList = node;
	} else {
		LinkedListScope *tmp = headList;
		if (token->type == LIBFUNC || token->type == USERFUNC) {
			while (tmp->next != NULL && token->value.funcVal->scope != tmp->scope) {
				tmp = tmp->next;
			}

			if (token->value.funcVal->scope == tmp->scope) {
				SymbolTableEntry *tmpentry = tmp->token;
				while (tmpentry->scopenext != NULL) {
					tmpentry = tmpentry->scopenext;
				}

				tmpentry->scopenext = token;

			} else if (tmp->next == NULL) {
				LinkedListScope* node = malloc(sizeof(LinkedListScope));
				node->next = NULL;
				node->token = token;
				node->scope = token->value.funcVal->scope;
				tmp->next = node;
			}
		} else {
			while (tmp->next != NULL && token->value.varVal->scope != tmp->scope) {
				tmp = tmp->next;
			}

			if (token->value.varVal->scope == tmp->scope) {
				SymbolTableEntry *tmpentry = tmp->token;
				while (tmpentry->scopenext != NULL) {
					tmpentry = tmpentry->scopenext;
				}

				tmpentry->scopenext = token;

			} else if (tmp->next == NULL) {
				LinkedListScope* node = malloc(sizeof(LinkedListScope));
				node->next = NULL;
				node->token = token;
				node->scope = token->value.varVal->scope;
				tmp->next = node;
			}

		}

	}


}

char *stringify(enum SymbolTableType current)
{
	if(current==GLOBAL){return "GLOBAL";}
	else if(current==LOCAL_){return "LOCAL_";}
	else if(current==USERFUNC){return "USERFUNC";}
	else if(current==LIBFUNC){return "LIBFUNC";}
	else if(current==FORMAL){return "FORMAL";}
}


SymbolTableEntry *insert(bool isActive, char *name, int scope , int line, enum SymbolTableType type) {

	int arraynum = hash_func(name);
	head = hashArray[arraynum];
	SymbolTableEntry* token;
	
	if (type == LIBFUNC) {

		Function* temp = malloc(sizeof(Function));
		temp->name = name;
		temp->line = line;
		temp->scope = scope;
		token = malloc(sizeof(SymbolTableEntry));
		token->scopenext = NULL;
		token->next = NULL;
		token->isActive = isActive;
		token->type = type;
		token->type_s = LIBFUNC_S;
		token->value.funcVal = temp;
		
		SymbolTableEntry* tmp = head;

		if (head == NULL) {
			hashArray[arraynum] = token;
			head = token;
		}
		else {
			while (tmp->next != NULL) {
				tmp = tmp->next;
			}
			tmp->next = token;
		}
	}
	else if (type == USERFUNC) {
		Function* temp = malloc(sizeof(Function));
		temp->name = name;
		temp->line = line;
		temp->scope = scope;
		token = malloc(sizeof(SymbolTableEntry));
		token->scopenext = NULL;
		token->next = NULL;
		token->isActive = isActive;
		token->type = type;
		token->type_s = PROGRAMFUNC_S;
		token->value.funcVal = temp;


		SymbolTableEntry* tmp = head;

		if (head == NULL) {
			hashArray[arraynum] = token;
			head = token;
		}
		else {
			while (tmp->next != NULL) {
				tmp = tmp->next;
			}
			tmp->next = token;
		}
	}
	else {
		Variable* temp = malloc(sizeof(Variable));
		temp->name = name;
		temp->line = line;
		temp->scope = scope;
		token = malloc(sizeof(SymbolTableEntry));
		token->scopenext = NULL;
		token->isActive = isActive;
		token->type = type;
		token->type_s = VAR_S;
		token->value.varVal = temp;


		SymbolTableEntry* tmp = head;
		if (head == NULL) {

			hashArray[arraynum] = token;
			head = token;
		}
		else {

			while (tmp->next != NULL) {
				tmp = tmp->next;
			}
			tmp->next = token;
		}

	}

	scope_insert(token);
	return token;

}

void  init_hash() {
	int i;
	for ( i = 0; i < MAX_SIZE; i++ ) {
		hashArray[i] = NULL;

	}
	insert(true, "print", 0 , 0, LIBFUNC);
	insert(true, "input", 0 , 0, LIBFUNC);
	insert(true, "objectmemberkeys", 0 , 0, LIBFUNC);
	insert(true, "objecttotalmembers", 0 , 0, LIBFUNC);
	insert(true, "objectcopy", 0 , 0, LIBFUNC);
	insert(true, "totalarguments", 0 , 0, LIBFUNC);
	insert(true, "argument", 0 , 0, LIBFUNC);
	insert(true, "typeof", 0 , 0, LIBFUNC);
	insert(true, "strtonum", 0 , 0, LIBFUNC);
	insert(true, "sqrt", 0 , 0, LIBFUNC);
	insert(true, "cos", 0 , 0, LIBFUNC);
	insert(true, "sin", 0 , 0, LIBFUNC);
	return ;
}

/*void Print_hash() {
	int i = 0;
	SymbolTableEntry* tmphash = hashArray[i];
	for (i = 0; i < MAX_SIZE; i++) {
		tmphash = hashArray[i];
		while (tmphash != NULL) { //edw gt xreiazete jexwristw gia libfunc
			if (tmphash->type == LIBFUNC)
				printf("%s Scope %d\n", tmphash->value.funcVal->name, tmphash->value.funcVal->scope);
			else if (tmphash->type == USERFUNC)
				printf("%s Scope %d\n", tmphash->value.funcVal->name, tmphash->value.funcVal->scope);
			else
				printf("%s Scope %d\n", tmphash->value.varVal->name, tmphash->value.varVal->scope);
			tmphash = tmphash->next;
		}
	}
}*/

char *stringify_scope_space(scopespace_t scope_space)
{
	if(scope_space == PROGRAM_VAR){return "PROGRAM_VAR";}
	else if(scope_space == FUNCTION_LOC){return "FUNCTION_LOC";}
	else if(scope_space == FORMAL_ARG){return "FORMAL_ARG";}
}

void PrintScopeList() {
	
	LinkedListScope *tmp = headList;
	while (tmp != NULL)
	{
		SymbolTableEntry *tmpentry;
		tmpentry = tmp->token;
		
		while (tmpentry != NULL) {
			if (tmpentry->type == USERFUNC || tmpentry-> type == LIBFUNC) {
				printf("Key = %s  %s  line = %d  scope = %d  isActive = %d\n", tmpentry->value.funcVal->name, stringify(tmpentry->type), tmpentry->value.funcVal->line, tmpentry->value.funcVal->scope, tmpentry->isActive);

			} else
			{
				printf("Key = %s  %s  line = %d  scope = %d  isActive = %d offset = %d scope_space = %s\n", 
					tmpentry->value.varVal->name, stringify(tmpentry->type), 
					tmpentry->value.varVal->line, tmpentry->value.varVal->scope, 
					tmpentry->isActive, tmpentry->offset, stringify_scope_space(tmpentry->scope_space));
			}
			tmpentry = tmpentry->scopenext;
		}
		tmp = tmp->next;

	}
}


void errors(){
	
	int i = 0;
	
	while(i < error_counter - 1){
		printf("%s",Error_tbl[i]);
		i++;
	}
}
