#Our main target : 'compute'
compute : compute.c expression_evaluator.o
	gcc compute.c expression_evaluator.o -o compute -lm
#Dependencies
expression_evaluator.o : expression_evaluator.c
	gcc -c expression_evaluator.c -o expression_evaluator.o
