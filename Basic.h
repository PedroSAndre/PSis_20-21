//A .h with libraries and defenitions necessary for all files 
#ifndef _Basic_
#define _Basic_
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <sys/un.h>
#include <netinet/in.h>

//Address defenition
#define server_addr "sockets/localserver"
#define part_client_addr "sockets/client"

//Flags defenition
#define PUT 1
#define GET 2
#define DEL 3
#define CLS 4

#endif
