##Individual file compilation
#Structs
key_value_struct.o: Basic.h key_value_struct.h key_value_struct.c
	gcc -g -c key_value_struct.c

group_table_struct.o: Basic.h group_table_struct.h key_value_struct.h group_table_struct.c
	gcc -g -c group_table_struct.c

app_status_struct.o: Basic.h app_status_struct.h app_status_struct.c
	gcc -g -c app_status_struct.c


#Main files
KVS-lib.o: KVS-lib.h Basic.h KVS-lib.c
	gcc -g -pthread -c KVS-lib.c

TestApp.o: KVS-lib.h Basic.h TestApp.c
	gcc -g -pthread -c TestApp.c

KVS-LocalServer.o: Basic.h key_value_struct.h group_table_struct.h KVS-LocalServer.c
	gcc -g -pthread -c KVS-LocalServer.c

Authserver.o: Basic.h Authserver.h Authserver.c
	gcc -g -c Authserver.c

KVS-AuthServer.o: Basic.h Authserver.h KVS-AuthServer.c
	gcc -g -c KVS-AuthServer.c



##Linking
App: KVS-lib.o TestApp.o
	gcc -g -pthread KVS-lib.o TestApp.o -o TestApp.out

LocalServer: key_value_struct.o group_table_struct.o app_status_struct.o KVS-LocalServer.o
	gcc -g -pthread key_value_struct.o group_table_struct.o app_status_struct.o KVS-LocalServer.o -o KVS-LocalServer.out

AuthServer: Auth_group_secret.o KVS-AuthServer.o
	gcc -g KVS-AuthServer.o Authserver.o -o KVS-AuthServer.out



clearall:
	rm TestApp.out KVS-LocalServer.out TestApp.o KVS-LocalServer.o KVS-lib.o key_value_struct.o group_table_struct.o app_status_struct.o sockets/*