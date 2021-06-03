#include "Basic.h"
#include "KVS-lib.h"

#include <pthread.h>

int client_sock;

int establish_connection (char * group_id, char * secret)
{
    
    int kvs_localserver_sock, client_pid=getpid();
    struct sockaddr_un client_sock_addr;
    struct sockaddr_un kvs_localserver_sock_addr;
    char * client_addr;
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

    memset(&client_sock_addr,0,sizeof(struct sockaddr_un));
    client_sock_addr.sun_family=AF_UNIX;
    sprintf(client_addr,"%s%d",part_client_addr,getpid());
    strcpy(client_sock_addr.sun_path, client_addr); //adress defined in Basic.h


    if(connect(client_sock, (struct sockaddr *)&kvs_localserver_sock_addr, sizeof(kvs_localserver_sock_addr)) < 0)
    {
        perror("Error connecting client socket");
        return -3;
    }
    


    if(write(client_sock,&client_pid,sizeof(int))==-1){
        perror("write(pid) error");
        return -4;
    }

    if(write(client_sock,group_id,group_id_max_size*sizeof(char))==-1){
        perror("write() error");
        return -4;
    }

    if(write(client_sock,secret,secret_max_size*sizeof(char))==-1){
        perror("write() error");
        return -5;
    }

    if(read(client_sock,&answer,sizeof(answer))==-1)
    {
        perror("No answer from local server");
        return -6;
    }
    if(answer<0){
        printf("Request denied\n");
        return -7;
    }
    return 0;
}



int put_value(char * key, char * value)
{
    int buf=PUT;
    long int vallen=strlen(value);

    if(write(client_sock,&buf,sizeof(int))==-1){
        perror("write(flag:PUT)  error");
        return -1;
    }

    if(write(client_sock,key,key_max_size* sizeof(char))==-1){
        perror("write(key)  error");
        return -2;
    }

    if(write(client_sock,&vallen,sizeof(long int))==-1){
        perror("write(vallen)  error");
        return -3;
    }

    if(write(client_sock,value,vallen*sizeof(char))==-1){
        perror("write(value)  error");
        return -4;
    }

    if(read(client_sock,&buf,sizeof(int))==-1)
    {
        perror("No answer from local server");
        return -4;
    }

    return buf;
}


int get_value(char * key, char ** value)
{
    long int answer;
    int buf=GET;

    if(write(client_sock,&buf,sizeof(buf))==-1){
        perror("write(flag:GET)  error");
        return -1;
    }

    if(write(client_sock,key,key_max_size* sizeof(char))==-1){
        perror("write(key)  error");
        return -1;
    }



    if(read(client_sock,&answer,sizeof(answer))==-1)
    {
        perror("No answer from local server");
        return -4;
    }
    if(answer==-1){
        perror("No key");
        return -5;
    }else if(answer==0){
        perror("No value");
        return -5;
    }

    *value = malloc (answer*sizeof(char));
    if (*value == NULL) {
        perror("Unable to alocate memory");
        return -6;
    }

    if(read(client_sock,*value,answer*sizeof(char))==-1)
    {
        perror("No answer from local server");
        return -4;
    }

    return 1;
}


int delete_value(char * key)
{
    int buf=DEL;

    if(write(client_sock,&buf,sizeof(buf))==-1){
        perror("write(flag:DEL)  error");
        return -1;
    }
    if(write(client_sock,key,key_max_size* sizeof(char))==-1){
        perror("write(key)  error");
        return -2;
    }

    if(read(client_sock,&buf,sizeof(int))==-1)
    {
        perror("No answer from local server");
        return -4;
    }

    return buf;
}

int register_callback(char * key, void (*callback_function)(char *)){
    int answer=CALL;
    pthread_t thread_id;
    
    if(write(client_sock,&answer,sizeof(int))==-1){
        perror("write(flag:CALL)  error");
        return -1;
    }

    if(write(client_sock,key, key_max_size* sizeof(char))==-1){
        perror("write(key)  error");
        return -1;
    }
    if(read(client_sock, &answer,sizeof(int)) ==-1){
        perror("No answer from local server");
        return -2;
    }
    
    if(answer==1){
        if(pthread_create(&thread_id,NULL,(void *)callback_function,(void *)key)<0)
        {
            perror("Error creating thread");
            return -3;
        }
    }else{
        printf("Something went wrong\n");
    }
    return 1;
}

int close_connection()
{
    int buf=CLS;

    if(write(client_sock,&buf,sizeof(buf))==-1){
        perror("write(flag:CLS)  error");
        return -1;
    }
    if(close(client_sock)<0)
    {
        perror("Error closing connection");
        return -2;
    }
    return 1;
}


