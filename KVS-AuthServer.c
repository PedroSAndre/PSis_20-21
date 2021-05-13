#include "Basic.h"

#define max_waiting_connections 10

int main(void)
{
    int answer;
    int request;
    int kvs_authserver_sock;
    int kvs_localserver_sock;
    struct sockaddr_un kvs_authserver_sock_addr;
    struct sockaddr_un kvs_localserver_sock_addr;

    char * authserver_addr="";



    char * group_id;
    char * secret;

    group_id = malloc(1024*sizeof(char));
    secret = malloc(1024*sizeof(char));



    //Creating socket
    kvs_authserver_sock = socket(AF_INET, SOCK_DGRAM, 0);
    if(kvs_authserver_sock==-1){
        perror("Error creating client socket\n");
        return -1;
    }

    //Binding address
    memset(&kvs_authserver_sock_addr,0,sizeof(struct sockaddr_un));
    kvs_authserver_sock_addr.sun_family=AF_INET;
    strcpy(kvs_authserver_sock_addr.sun_path, authserver_addr); //adress defined in Basic.h
    if(bind(kvs_authserver_sock, &kvs_authserver_sock_addr, sizeof(kvs_authserver_sock_addr)) < 0)
    {
        perror("Error binding socket\n");
        return -2;
    }

    //Waiting for connection cycle
    kvs_localserver_sock=0;
    if(recvfrom(kvs_authserver_sock,&request,sizeof(int),0,&kvs_localserver_sock_addr,sizeof(struct sockaddr_un))<0)
    {
        perror("Error receving connection\n");
        return -3;
    }

    memset(&kvs_localserver_sock_addr,0,sizeof(struct sockaddr_un));
    if(bind(kvs_localserver_sock, &kvs_localserver_sock_addr, sizeof(kvs_localserver_sock_addr)) < 0)
    {
        perror("Error binding socket\n");
        return -2;
    }
    


    if(close(kvs_localserver_sock)<0)
    {
        perror("Error closing connection");
        return -5;
    }
}