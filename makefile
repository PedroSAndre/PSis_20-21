KVS-lib.o: KVS-lib.h Basic.h KVS-lib.c
	gcc -c KVS-lib.c

TestApp.o: KVS-lib.h Basic.h TestApp.c
	gcc -c TestApp.c

App: KVS-lib.o TestApp.o
	gcc KVS-lib.o TestApp.o -o TestApp.out

Server: Basic.h KVS-LocalServer.c
	gcc -pthread KVS-LocalServer.c -o KVS-LocalServer.out

rm:
	rm TestApp.out TestApp.o KVS-LocalServer.out KVS-lib.o sockets/*