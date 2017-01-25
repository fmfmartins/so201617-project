CFLAGS = -g -Wall -pedantic -pthread
all: i-banco i-banco-terminal 

i-banco : commandlinereader.o contas.o i-banco.o
	gcc $(CFLAGS) -o i-banco commandlinereader.o contas.o i-banco.o

i-banco-terminal: i-banco-terminal.o commandlinereader.o
	gcc $(CFLAGS) -o i-banco-terminal i-banco-terminal.o commandlinereader.o

contas.o : contas.c contas.h
	gcc -c $(CFLAGS) contas.c

commandlinereader.o : commandlinereader.c commandlinereader.h
	gcc -c $(CFLAGS) commandlinereader.c

i-banco.o : commandlinereader.c commandlinereader.h contas.c contas.h i-banco.c
	gcc -c $(CFLAGS) commandlinereader.c contas.c i-banco.c

i-banco-terminal.o: i-banco-terminal.c i-banco-terminal.h commandlinereader.c commandlinereader.h
	gcc -c $(CFLAGS) i-banco-terminal.c

clean:
	rm -f *.o i-banco i-banco-terminal *.txt
	rm -f /tmp/i-banco-pipe
	rm -f /tmp/i-banco-answer*

zip:
	zip -r SO-Proj.zip *.c *.h Makefile

run:
	make && ./i-banco


