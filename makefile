KVS-lib.o: KVS-lib.h Basic.h KVS-lib.c
	gcc -g -c KVS-lib.c

TestApp.o: KVS-lib.h Basic.h TestApp.c
	gcc -g -c TestApp.c

key_value_struct.o: key_value_struct.h Basic.h key_value_struct.c
	gcc -g -c key_value_struct.c

KVS-LocalServer.o: Basic.h key_value_struct.h KVS-LocalServer.c
	gcc -g -pthread -c KVS-LocalServer.c



App: Basic.h KVS-lib.o TestApp.o
	gcc KVS-lib.o TestApp.o -o TestApp.out

Server: Basic.h key_value_struct.h key_value_struct.o KVS-LocalServer.o
	gcc -pthread key_value_struct.o KVS-LocalServer.o -o KVS-LocalServer.out



rm:
	rm TestApp.out KVS-LocalServer.out TestApp.o KVS-LocalServer.o KVS-lib.o key_value_struct.o sockets/*