#include "KVS_temp.h"

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <pthread.h>


#define server_addr "sockets/localserver"


int establish_connection (char * group_id, char * secret)
{
    int client_sock;
    int kvs_localserver_sock;
    struct sockaddr_un client_sock_addr;
    struct sockaddr_un kvs_localserver_sock_addr;
    char * client_addr;
    char * client_request[2]={group_id,secret};
    int answer;

    client_addr=malloc(20*sizeof(char));
    

    client_sock = socket(AF_UNIX, SOCK_STREAM, 0);
    if(client_sock==-1){
        printf("Error creating client socket\n");
        return -1;
    }
    kvs_localserver_sock = socket(AF_UNIX, SOCK_STREAM, 0);
    if(kvs_localserver_sock==-1){
        printf("Error creating kvs_localserver socket\n");
        return -10;
    }


    memset(&kvs_localserver_sock_addr,0,sizeof(struct sockaddr_un));
    kvs_localserver_sock_addr.sun_family=AF_UNIX;
    strcpy(kvs_localserver_sock_addr.sun_path, server_addr);
    
    printf("Connecting to server...\n");
    if(connect(client_sock, &kvs_localserver_sock_addr, sizeof(kvs_localserver_sock_addr)) < 0)
    {
        printf("Error connecting client socket\n");
        return -2;
    }
    write(client_sock,&client_request,sizeof(client_request));
    read(client_sock,&answer,sizeof(answer));
    if(answer<0){
        printf("Request denied\n");
        return -3;
    }
    return 0;
}