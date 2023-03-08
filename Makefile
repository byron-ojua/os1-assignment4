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
	make
	./p5testscript 61000 61001

testscript-basic:
	clear
	make
	./p5testscript-basic 61000 61001

testscript-mostbasic:
	clear
	make
	./p5testscript-mostbasic 61000 61001

leak-test:
	clear
	make
	valgrind --leak-check=yes ./enc_server 61000 &

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

kill-all:
	ps -ef | grep niceb | awk '{print $2}' | xargs kill -9 {} \;