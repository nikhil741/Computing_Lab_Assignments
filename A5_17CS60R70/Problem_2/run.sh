flex -l calc.l
yacc -vd calc.y
gcc lex.yy.c y.tab.c -lm -ll
./a.out