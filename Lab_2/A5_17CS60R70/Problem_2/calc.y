%{
#include<stdio.h>
#include<stdlib.h>
%}
%token NUMBER
%left '+' '-'
%left '*' '/'
%right '^'

%%
e : e '+' e
|   e '-' e
|   e '*' e
|   e '/' e
|   e '^' e
|   '(' e ')'
|   '-' e
|   '+' e
| NUMBER         ;
%%

int main(){
	printf("Enter Expression\n");
	yyparse();
	printf("Valid Expression \n");
}

int yyerror(){
	printf("Invalid Expresion!!\n"); 
	exit(0);
}