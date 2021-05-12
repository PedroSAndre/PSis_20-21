#include "Basic.h"

#define max_waiting_connections 10
int main(void)
{
    int answer;
    int kvs_localserver_sock;
    int client_sock;
    struct sockaddr_un kvs_localserver_sock_addr;
    struct sockaddr_un client_sock_addr;

    char * group_id;
    group_id = malloc(1024*sizeof(char));

    remove(server_addr); //To remove later

    //Creating socket
    kvs_localserver_sock = socket(AF_UNIX, SOCK_STREAM, 0);
    if(kvs_localserver_sock==-1){
        printf("Error creating client socket\n");
        return -1;
    }

    //Binding address
    memset(&kvs_localserver_sock_addr,0,sizeof(struct sockaddr_un));
    kvs_localserver_sock_addr.sun_family=AF_UNIX;
    strcpy(kvs_localserver_sock_addr.sun_path, server_addr); //adress defined in Basic.h
    if(bind(kvs_localserver_sock, &kvs_localserver_sock_addr, sizeof(kvs_localserver_sock_addr)) < 0)
    {
        printf("Error binding socket\n");
        return -2;
    }

    //Waiting for connection
    client_sock=0;
    if(listen(kvs_localserver_sock,max_waiting_connections)<0)
    {
        printf("Error listening for connections\n");
        return -3;
    }
    client_sock = accept(kvs_localserver_sock,NULL,NULL);
    if(client_sock<0)
    {
        perror("Error connecting");
        return -4;
    }
    printf("Client adress: %s\n", client_sock_addr.sun_path);
    read(client_sock,group_id,(1024*sizeof(char)));
    printf("Group_ID: %s\n\n", group_id);
    answer=1;
    write(client_sock,&answer,sizeof(answer));
    if(close(client_sock)<0)
    {
        perror("Error closing connection");
        return -5;
    }
}