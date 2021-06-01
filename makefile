##Individual file compilation
#Structs
key_value_struct.o: Basic.h key_value_struct.h key_value_struct.c
	gcc -c key_value_struct.c

group_table_struct.o: Basic.h group_table_struct.h group_table_struct.c
	gcc -c group_table_struct.c


#Main files
KVS-lib.o: KVS-lib.h Basic.h KVS-lib.c
	gcc -pthread -c KVS-lib.c

TestApp.o: KVS-lib.h Basic.h TestApp.c
	gcc -pthread -c TestApp.c

KVS-LocalServer.o: Basic.h key_value_struct.h group_table_struct.h KVS-LocalServer.c
	gcc -pthread -c KVS-LocalServer.c

Authserver.o: Basic.h Authserver.h Authserver.c
	gcc -c Authserver.c

KVS-AuthServer.o: Basic.h Authserver.h KVS-AuthServer.c
	gcc -c KVS-AuthServer.c



##Linking
App: KVS-lib.o TestApp.o
	gcc -pthread KVS-lib.o TestApp.o -o TestApp.out

LocalServer: key_value_struct.o group_table_struct.o KVS-LocalServer.o
	gcc -pthread key_value_struct.o group_table_struct.o KVS-LocalServer.o -o KVS-LocalServer.out

AuthServer: Auth_group_secret.o KVS-AuthServer.o
	gcc KVS-AuthServer.o Authserver.o -o KVS-AuthServer.out



clearall:
	rm TestApp.out KVS-LocalServer.out TestApp.o KVS-LocalServer.o KVS-lib.o key_value_struct.o group_table_struct.o sockets/*