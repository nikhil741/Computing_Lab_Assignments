%{
#include "y.tab.h"
%}
%%
[0-9]+\.?|[+-][0-9]*\.[0-9]+  {return NUM;}
[ \t]       ;
\n         return 0;
.          return yytext[0];
%%
yywrap()
{}
