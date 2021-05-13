#include "Basic.h"

#define max_waiting_connections 10


int main(void)
{
    int answer;
    int kvs_localserver_sock;
    int client_sock;
    struct sockaddr_un kvs_localserver_sock_addr;
    struct sockaddr_un client_sock_addr;
    int client_sock_addr_size;
    int client_PID;

    char * group_id;
    char * secret;
    group_id = malloc(1024*sizeof(char));
    secret = malloc(1024*sizeof(char));

    remove(server_addr); //To remove later

    //Creating socket
    kvs_localserver_sock = socket(AF_UNIX, SOCK_STREAM, 0);
    if(kvs_localserver_sock==-1){
        perror("Error creating client socket\n");
        return -1;
    }

    //Binding address
    memset(&kvs_localserver_sock_addr,0,sizeof(struct sockaddr_un));
    kvs_localserver_sock_addr.sun_family=AF_UNIX;
    strcpy(kvs_localserver_sock_addr.sun_path, server_addr); //adress defined in Basic.h
    if(bind(kvs_localserver_sock, &kvs_localserver_sock_addr, sizeof(kvs_localserver_sock_addr)) < 0)
    {
        perror("Error binding socket\n");
        return -2;
    }

    //Waiting for connection cycle
    client_sock=0;
    if(listen(kvs_localserver_sock,max_waiting_connections)<0)
    {
        perror("Error listening for connections\n");
        return -3;
    }

    memset(&client_sock_addr,0,sizeof(struct sockaddr_un));
    client_sock_addr_size = 0;
    client_sock = accept(kvs_localserver_sock,NULL,NULL);
    if(client_sock<0)
    {
        perror("Error connecting");
        return -4;
    }
    read(client_sock,&client_PID,sizeof(client_PID));
    read(client_sock,group_id,(1024*sizeof(char)));
    read(client_sock,secret,(1024*sizeof(char)));
    printf("Client_PID: %d\n", client_PID);
    printf("Group_ID: %s\n", group_id);
    printf("Secret: %s\n", secret);

    answer=1;
    write(client_sock,&answer,sizeof(answer));
    read(client_sock,&answer,sizeof(answer));
    if(close(client_sock)<0)
    {
        perror("Error closing connection");
        return -5;
    }
}