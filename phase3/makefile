all :	calc

calc : al.c parser.c symboltable.c syntax.c
	gcc -o calc al.c parser.c symboltable.c syntax.c

al.c : alpha.l
	flex --outfile=al.c alpha.l

parser.c :	parser.y
	bison --yacc --defines --output=parser.c parser.y

clean:	calc al.c parser.c
	rm -f al.c calc parser.c
