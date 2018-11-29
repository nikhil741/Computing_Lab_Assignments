yacc -vd calc.y
flex -d calc.l
gcc lex.yy.c y.tab.c -lm -ll
./a.out
