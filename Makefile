# build an executable named movies_by_year from main.c
all: main.c 
	gcc --std=gnu99 -o enc_server enc_server.c
	gcc --std=gnu99 -o enc_client enc_client.c
	gcc --std=gnu99 -o dec_server dec_server.c
	gcc --std=gnu99 -o dec_client dec_client.c
	gcc --std=gnu99 -o keygen keygen.c

c:
	gcc --std=gnu99 -o enc_client enc_client.c
	gcc --std=gnu99 -o dec_client dec_client.c

s:
	gcc --std=gnu99 -o enc_server enc_server.c
	gcc --std=gnu99 -o dec_server dec_server.c

e:
	gcc --std=gnu99 -o enc_server enc_server.c
	gcc --std=gnu99 -o enc_client enc_client.c

d:
	gcc --std=gnu99 -o dec_server dec_server.c
	gcc --std=gnu99 -o dec_client dec_client.c

clean: 
	rm enc_server
	rm enc_client
	rm dec_server
	rm dec_client
	rm keygen

test:
	clear
	gcc --std=gnu99 -o smallsh main.c
	./p5testscript 55000 55001

testscript-basic:
	clear
	make
	./p5testscript-basic 55000 55001

testscript-mostbasic:
	clear
	make
	./p5testscript-mostbasic 55000 55001

leak-test:
	clear
	make
	valgrind --leak-check=yes ./enc_server 60000 &

gdb-test:
	clear
	echo Run GDB with 'run'
	make
	gdb ./enc_server 60000 &

keygen:
	clear
	gcc --std=gnu99 -o keygen keygen.c
	./keygen

processes:
	ps -ef | grep niceb

kill:
	./killFiles