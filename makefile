KVS-lib.o: KVS-lib.h Basic.h KVS-lib.c
	gcc -c KVS-lib.c

TestApp.o: KVS-lib.h Basic.h TestApp.c
	gcc -c TestApp.c

App: KVS-lib.o TestApp.o
	gcc KVS-lib.o TestApp.o -o TestApp.out

Server: Basic.h KVS-LocalServer.c
	gcc KVS-LocalServer.c -o KVS-LocalServer.out

AuthServer: Basic.h KVS-AuthServer.c
	gcc KVS-AuthServer.c -o KVS-AuthServer.out

rm: TestApp.out TestApp.o KVS-LocalServer.out KVS-lib.o
	rm TestApp.out TestApp.o KVS-LocalServer.out KVS-lib.o sockets/*