##Individual file compilation
#Structs
key_value_struct.o: Basic.h key_value_struct.h key_value_struct.c
	gcc -c key_value_struct.c

group_table_struct.o: Basic.h group_table_struct.h key_value_struct.h group_table_struct.c
	gcc -c group_table_struct.c

app_status_struct.o: Basic.h app_status_struct.h app_status_struct.c
	gcc -c app_status_struct.c


#Main files
KVS-lib.o: KVS-lib.h Basic.h KVS-lib.c
	gcc -pthread -c KVS-lib.c

TestApp.o: KVS-lib.h Basic.h TestApp.c
	gcc -pthread -c TestApp.c

Localserver_aux.o: Basic.h Localserver_aux.h Localserver_aux.c
	gcc -c Localserver_aux.c

KVS-LocalServer.o: Basic.h key_value_struct.h group_table_struct.h KVS-LocalServer.c
	gcc -pthread -c KVS-LocalServer.c

Authserver.o: Basic.h Authserver.h Authserver.c
	gcc -c Authserver.c

KVS-AuthServer.o: Basic.h Authserver.h KVS-AuthServer.c
	gcc -pthread -c KVS-AuthServer.c



##Linking
App: KVS-lib.o TestApp.o
	gcc -pthread KVS-lib.o TestApp.o -o TestApp.out

LocalServer: key_value_struct.o group_table_struct.o app_status_struct.o Localserver_aux.o KVS-LocalServer.o
	gcc -pthread key_value_struct.o group_table_struct.o app_status_struct.o Localserver_aux.o KVS-LocalServer.o -o KVS-LocalServer.out

AuthServer: Authserver.o KVS-AuthServer.o
	gcc -pthread KVS-AuthServer.o Authserver.o -o KVS-AuthServer.out


##ALL
All: App LocalServer AuthServer



clearall:
	rm TestApp.out KVS-LocalServer.out TestApp.o KVS-LocalServer.o KVS-lib.o key_value_struct.o group_table_struct.o app_status_struct.o Localserver_aux.o KVS-AuthServer.o KVS-AuthServer.out Authserver.o sockets/*