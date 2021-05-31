//A .h with libraries and defenitions necessary for all files 
#ifndef _Basic_
#define _Basic_
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <unistd.h>
#include <sys/un.h>
#include <netinet/in.h>
#include <time.h>

//Address defenition
#define server_addr "sockets/localserver"
#define part_client_addr "sockets/client"

//Constant definition
#define max_waiting_connections 10
#define timeout 5
#define input_string_max_size 1024 //needs to be qual or bigger than all other string sizes
#define secret_max_size 512
#define key_max_size 512
#define group_id_max_size 1024
//Tables sizes
#define key_value_table_size 1021
#define n_groups_max 1021

//Flags defenition
#define WAIT 0
#define PUT 1
#define GET 2
#define DEL 3
#define CLS 4
#define CMP 5

#endif
