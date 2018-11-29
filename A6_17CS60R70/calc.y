%code requires
{
	//#define YYSTYPE double
	struct expression_data{
		double exp_val;
		char o_p[1024];	
	};
}
%union{
	float fval;
	struct expression_data exp_data;
}

%{
#include<ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
%}


%token<fval> NUMBER
%type <exp_data> e

%left '+' '-'
%left '*' '/'
%right '^'

%%

S : S e '\n' { printf("Answer: %.2lf \nEnter Expression:\n", $2.exp_val); }
| S '\n'
|
;

e :e e		{$$.exp_val = $1.exp_val ; sprintf($$.o_p,"{%s  %s}",$1.o_p,$2.o_p); printf("Optimizing!!(withoutOperator) :%s:\n", $$.o_p);}
| e e '^' e 	{$$.exp_val = pow($1.exp_val, $4.exp_val); sprintf($$.o_p,"{%s ^ %s}",$1.o_p,$4.o_p); printf("Optimizing!!in Power :%s:\n", $$.o_p);}
| e e '*' e 	{$$.exp_val = $1.exp_val * $4.exp_val; sprintf($$.o_p,"{%s * %s}",$1.o_p,$4.o_p); printf("Optimizing!!in multiply :%s:\n", $$.o_p);}
| e e '/' e 	{$$.exp_val = $1.exp_val / $4.exp_val; sprintf($$.o_p,"{%s / %s}",$1.o_p,$4.o_p); printf("Optimizing!!in divide :%s:\n", $$.o_p);}
| e e '+' e 	{$$.exp_val = $1.exp_val + $4.exp_val; sprintf($$.o_p,"{%s + %s}",$1.o_p,$4.o_p); printf("Optimizing!!in plus :%s:\n", $$.o_p);}
| e e '-' e 	{$$.exp_val = $1.exp_val - $4.exp_val; sprintf($$.o_p,"{%s - %s}",$1.o_p,$4.o_p); printf("Optimizing!!in minus :%s:\n", $$.o_p);}
| e '^' e 	{$$.exp_val = pow($1.exp_val,$3.exp_val); sprintf($$.o_p,"{%s ^ %s}",$1.o_p,$3.o_p); printf("in power :%s:\n", $$.o_p);}
| e '*' e 	{$$.exp_val = $1.exp_val * $3.exp_val; sprintf($$.o_p,"{%s * %s}",$1.o_p,$3.o_p); printf("in Multiply :%s:\n", $$.o_p);}
| e '/' e 	{$$.exp_val = $1.exp_val / $3.exp_val; sprintf($$.o_p,"{%s / %s}",$1.o_p,$3.o_p); printf("in divide :%s:\n", $$.o_p);}
| e '+' e 	{$$.exp_val = $1.exp_val + $3.exp_val; sprintf($$.o_p,"{%s + %s}",$1.o_p,$3.o_p); printf("in plus :%s:\n", $$.o_p);}
| e '-' e 	{$$.exp_val = $1.exp_val - $3.exp_val; sprintf($$.o_p,"{%s - %s}",$1.o_p,$3.o_p); printf("in minus :%s:\n", $$.o_p);}
| '(' e ')'	{$$.exp_val = $2.exp_val; }
| NUMBER        {$$.exp_val = $1; sprintf($$.o_p,"%f",$1);printf("in Number :%s:\n",$$.o_p);}       
;
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
