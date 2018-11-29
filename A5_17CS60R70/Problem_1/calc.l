%{
#include <stdio.h>
%}

special_character  [!@#$%={}:,?\/|;]
letter   [A-Za-z]+

%%
[(] {printf("Found token : LEFT_BRACKET( Lexeme : %s)\n", yytext);}

[)] {printf("Found token : RIGHT_BRACKET( Lexeme : %s)\n", yytext);}

[*] {printf("Found token : MULT_OP( Lexeme : %s)\n", yytext);}

[+] {printf("Found token : PLUS_OP( Lexeme : %s)\n", yytext);}

[-] {printf("Found token : MINUS_OP( Lexeme : %s)\n", yytext);}

[/] {printf("Found token : DIV_OP( Lexeme : %s)\n", yytext);}

"^" {printf("Found token :  POWER_OP( Lexeme : %s)\n", yytext);}

[0-9]+ {printf("Found token : INTEGER ( Lexeme : %s)\n", yytext);}

[0-9]*\.[0-9]+ {printf("Found token : FLOAT ( Lexeme : %s)\n", yytext);}

[ ] {}

['\n'] {return 0;}

({letter}|{special_character}) {printf("Invalid String %s\n", yytext);}

%%

int yywrap(void){
	return 1;
}

int main(){
	printf("Enter Arithmetic Operation");
	yylex();
	return 0;
}
