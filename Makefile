CFLAGS = -g -Wall -pedantic -pthread

i-banco : commandlinereader.o contas.o i-banco.o
	gcc $(CFLAGS) -o i-banco commandlinereader.o contas.o i-banco.o

contas.o : contas.c contas.h
	gcc -c $(CFLAGS) contas.c

commandlinereader.o : commandlinereader.c commandlinereader.h
	gcc -c $(CFLAGS) commandlinereader.c

i-banco.o : commandlinereader.c commandlinereader.h contas.c contas.h i-banco.c
	gcc -c $(CFLAGS) commandlinereader.c contas.c i-banco.c

clean:
	rm -f *.o i-banco i-banco-terminal
	rm -f /tmp/i-banco-pipe

zip:
	zip -r SO-Proj.zip *.c *.h Makefile input.txt

run:
	make && ./i-banco

runinput:
	make && ./i-banco <input.txt

terminator: commandlinereader.c commandlinereader.h
	make
	gcc -c $(CFLAGS) i-banco-terminal.c
	make terminal1
	echo SKYNET
terminal1: i-banco-terminal.o commandlinereader.o
	gcc $(CFLAGS) -o i-banco-terminal i-banco-terminal.o commandlinereader.o
