#include "Basic.h"
#include "KVS-lib.h"


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
    printf("%s\n",client_addr);
    strcpy(client_sock_addr.sun_path, client_addr); //adress defined in Basic.h


    if(connect(client_sock, &kvs_localserver_sock_addr, sizeof(kvs_localserver_sock_addr)) < 0)
    {
        perror("Error connecting client socket");
        return -3;
    }
    


    if(write(client_sock,&client_pid,sizeof(int))==-1){
        perror("write(pid) error");
        return -4;
    }

    if(write(client_sock,group_id,1024*sizeof(char))==-1){
        perror("write() error");
        return -4;
    }

    if(write(client_sock,secret,1024*sizeof(char))==-1){
        perror("write() error");
        return -5;
    }

    if(read(client_sock,&answer,sizeof(answer))==-1)
    {
        perror("No answer from local server");
        return -6;
    }
    if(answer<0){
        perror("Request denied");
        return -7;
    }
    return 0;
}



int put_value(char * key, char * value)
{
    int buf=PUT;
    long int vallen=strlen(value);

    if(write(client_sock,&buf,sizeof(buf))==-1){
        perror("write(flag)  error");
        return -1;
    }

    if(write(client_sock,&key,sizeof(key))==-1){
        perror("write(key)  error");
        return -2;
    }

    if(write(client_sock,&vallen,sizeof(long int))==-1){
        perror("write(vallen)  error");
        return -3;
    }

    if(write(client_sock,&value,vallen*sizeof(char))==-1){
        perror("write(value)  error");
        return -4;
    }

    return 1;
}


int get_value(char * key, char ** value)
{
    long int answer;
    int buf=GET;

    if(write(client_sock,&buf,sizeof(buf))==-1){
        perror("write(flag)  error");
        return -1;
    }

    if(write(client_sock,&key,sizeof(key))==-1){
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
    }elseif(answer==0){
        perror("No value");
        return -5;
    }

    *value = malloc (answer*sizeof(char));
    if (*value == NULL) {
        perror("Unable to alocate memory");
        return -6
    }

    if(read(client_sock,*value,answer*sizeof(char))==-1)
    {
        perror("No answer from local server");
        return -4;
    }
    printf("%s",*value);

    return 1;
}


int delete_value(char * key)
{
    int buf=DEL;

    if(write(client_sock,&buf,sizeof(buf))==-1){
        perror("write(flag)  error");
        return -1;
    }
    if(write(client_sock,&key,sizeof(key))==-1){
        perror("write(key)  error");
        return -2;
    }
    return 1;
}

int close_connection()
{
    int buf=CLS;

    if(write(client_sock,&buf,sizeof(buf))==-1){
        perror("write(flag)  error");
        return -1;
    }
    close(client_sock);
}
