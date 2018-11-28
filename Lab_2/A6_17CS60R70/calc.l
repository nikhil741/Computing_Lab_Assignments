%{
#include <stdio.h>
#include "y.tab.h"
#include <string.h>
%}

%%
[ \t]                            ;//Remove WHitespaces


[0-9]                        {yylval.fval = atof(yytext) + 10; return NUMBER;} //SingleDigitAdding10
[0][0-9]*                    { yylval.fval = atof(yytext)*0.1; return NUMBER;} //Starting 0 multyply by 0.1
[a-zA-Z]                     { yylval.fval = (float)yytext[0]; return NUMBER;} //Send Ascii value
"nikhil"                     { yylval.fval = (float)rand(); printf("See the Magic:\n"); return NUMBER;} //Random NUmber
[**]                         {return '^';} //Power for **

            
[0-9]+\.?|[+-][0-9]*\.[0-9]+ {yylval.fval = atof(yytext); return NUMBER;}  //Number
[+-/*//^]+   		     {printf("Operation Optimizing to last Operator\n"); return yytext[0];} //Optimize operator to first one

\n                            return '\n'; //Return New Line
.                             return yytext[0];	//send character
%%
yywrap()
{}
