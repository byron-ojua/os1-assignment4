# build an executable named movies_by_year from main.c
all: main.c 
	gcc --std=gnu99 -o enc_server enc_server.c
	gcc --std=gnu99 -o enc_client enc_client.c
	gcc --std=gnu99 -o dec_server dec_server.c
	gcc --std=gnu99 -o dec_client dec_client.c
	gcc --std=gnu99 -o keygen keygen.c

clean: 
	rm enc_server
	rm enc_client
	rm dec_server
	rm dec_client
	rm keygen

test:
	clear
	gcc --std=gnu99 -o smallsh main.c
	./p5testscript-1

testscript-basic:
	clear
	gcc --std=gnu99 -o smallsh main.c
	./basicp3testscript 2>&1

leak-test:
	clear
	gcc --std=gnu99 -o smallsh main.c
	valgrind --leak-check=yes ./smallsh

gdb-test:
	clear
	echo Run GDB with 'run'
	gcc -g --std=gnu99 -o smallsh main.c
	gdb ./smallsh

rand-test:
	clear
	gcc --std=gnu99 -o randport randport.c
	./randport

keygen:
	clear
	gcc --std=gnu99 -o keygen keygen.c
	./keygen