#include "Basic.h"

#include <pthread.h>

#define max_waiting_connections 10

//Declaration of functions writen in the end
int createAndBindServerSocket(int * localserver_sock, struct sockaddr_un * localserver_sock_addr);
void acceptConnections(void *arg);
void handleConnection(void *arg);

//Global variables (to be shared across all server threads)
int server_status = 1; //1 -> ON; 0 -> OFF 

int main(void)
{
    int cycle_status = 1;
    int selector = 0;
    pthread_t acepting_thread_ptid;

    remove(server_addr); //To remove later


    if(pthread_create(&acepting_thread_ptid,NULL,(void *)&acceptConnections,NULL)<0)
    {
        perror("Error creating thread");
        return -6;
    }

    printf("*****Welcome to KVS Local Server*****\n");
    //Main control cycle
    while(cycle_status)
    {
        printf("Select the desired option:\n0) Shutdown server\n1) Create a group\n2) Delete a group\n3) Show group info\n4) Show app status\n\n");
        scanf("%d", &selector);
        printf("Option selected: %d\n\n", selector);
        if(selector==0)
        {
            server_status=0;
            cycle_status=0;
        }
    }
    pthread_join(acepting_thread_ptid,NULL);

    return 0;
}

//Functions used to simplify code
int createAndBindServerSocket(int * localserver_sock, struct sockaddr_un * localserver_sock_addr)
{

    //Creating socket
    *localserver_sock = socket(AF_UNIX, SOCK_STREAM, 0);
    if(*localserver_sock==-1){
        perror("Error creating main local server socket\n");
        return -1;
    }

    //Binding address
    memset(localserver_sock_addr,0,sizeof(struct sockaddr_un));
    localserver_sock_addr->sun_family=AF_UNIX;
    strcpy(localserver_sock_addr->sun_path, server_addr); //adress defined in Basic.h
    if(bind(*localserver_sock, localserver_sock_addr, sizeof(*localserver_sock_addr)) < 0)
    {
        perror("Error binding socket\n");
        return -2;
    }

    return 0;
}


//Thread functions
void acceptConnections(void *arg)
{
    int aux;
    int kvs_localserver_sock;
    int client_sock;
    struct sockaddr_un kvs_localserver_sock_addr;

    aux = createAndBindServerSocket(&kvs_localserver_sock, &kvs_localserver_sock_addr);
    if(aux<0)
    {
        pthread_exit((void *)&aux);
    }

    //To fix later
    pthread_t ptid;

    //Waiting for connection cycle
    client_sock=0;
    if(listen(kvs_localserver_sock,max_waiting_connections)<0)
    {
        perror("Error listening for connections\n");
        pthread_exit((void *)-3);
    }

    client_sock = accept(kvs_localserver_sock,NULL,NULL);
    if(client_sock<0)
    {
        perror("Error connecting");
        pthread_exit((void *)-4);
    }

    if(pthread_create(&ptid,NULL,(void *)&handleConnection,(void *)&client_sock)<0)
    {
        perror("Error creating thread");
        pthread_exit((void *)-6);
    }

    pthread_join(ptid,NULL);
    pthread_exit((void *)0);
}

void handleConnection(void *arg)
{
    int client_sock;
    int answer;
    int client_PID;

    char * group_id;
    char * secret;
    group_id = malloc(1024*sizeof(char));
    secret = malloc(1024*sizeof(char));

    client_sock = *((int *)arg);

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
        pthread_exit((void *)-5);
    }

    pthread_exit((void *)0);
}