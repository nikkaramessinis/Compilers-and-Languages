all : al.c exe

exe : al.c
	gcc -o exe al.c

al.c : alpha.l
	flex --outfile=al.c alpha.l

clean:	exe al.c
	rm -f al.c exe