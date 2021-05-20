KVS-lib.o: KVS-lib.h Basic.h KVS-lib.c
	gcc -c KVS-lib.c

TestApp.o: KVS-lib.h Basic.h TestApp.c
	gcc -c TestApp.c

key_value_struct.o: key_value_struct.h Basic.h key_value_struct.c
	gcc -c key_value_struct.c

KVS-LocalServer.o: Basic.h key_value_struct.h KVS-LocalServer.c
	gcc -pthread -c KVS-LocalServer.c

KVS-AuthServer.o: Basic.h KVS-AuthServer.c
	gcc -c KVS-AuthServer.c



App: Basic.h KVS-lib.o TestApp.o
	gcc KVS-lib.o TestApp.o -o TestApp.out

Server: Basic.h key_value_struct.h key_value_struct.o KVS-LocalServer.o
	gcc -pthread key_value_struct.o KVS-LocalServer.o -o KVS-LocalServer.out

AuthServer:  Basic.h KVS-AuthServer.o
	gcc KVS-AuthServer.o -o KVS-AuthServer.out



rm:
	rm TestApp.out KVS-LocalServer.out TestApp.o KVS-LocalServer.o KVS-lib.o key_value_struct.o sockets/* testing_stuff.o testing_stuff.out

test: key_value_struct.h Basic.h testing_stuff.c key_value_struct.c
	gcc -c testing_stuff.c key_value_struct.c
	gcc testing_stuff.o key_value_struct.o -o testing_stuff.out