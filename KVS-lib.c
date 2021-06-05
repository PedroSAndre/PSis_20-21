#include "Basic.h"
#include "KVS-lib.h"

#include <pthread.h>

//Defined client_sock here as it is only used in this library
int client_sock;

/*establish_conection()
Como explicado no enunciado, esta função cria uma conecção com o servidor local KVS-LocalServer.
Retorna 0 em caso de sucesso ou uma flag de erro definida no Basic.h

Esta função cria primeiro uma conecção com sockets tipo UNIX STREAM and sends a group and secret which will be subject to authentication. 
If authentication was sucessful, returns 0*/
int establish_connection (char * group_id, char * secret)
{
    
    int client_pid=getpid();
    struct sockaddr_un kvs_localserver_sock_addr;
    int answer;
    
    //Socket creation
    client_sock = socket(AF_UNIX, SOCK_STREAM, 0);
    if(client_sock==-1){
        printf("Error creating client socket\n");
        return ERRSCKCREATION;
    }

    //Set 0 kvs_localserver_sock_addr
    memset(&kvs_localserver_sock_addr,0,sizeof(struct sockaddr_un));

    //Assign info of lacal server to KVS-LocalServer 
    kvs_localserver_sock_addr.sun_family=AF_UNIX;
    strcpy(kvs_localserver_sock_addr.sun_path, server_addr);
    
    printf("Connecting to server...\n");
 

    //Connect to local server
    if(connect(client_sock, (struct sockaddr *)&kvs_localserver_sock_addr, sizeof(kvs_localserver_sock_addr)) < 0)
    {
        perror("Error connecting client socket");
        return ERRCONNECTING;
    }
    


    if(write(client_sock,&client_pid,sizeof(int))==-1){
        perror("write(pid) error");
        return ERRWRT;
    }

    //Send group_id to local server
    if(write(client_sock,group_id,group_id_max_size*sizeof(char))==-1){
        perror("write() error");
        return ERRWRT;
    }

    //Send group_id to local server
    if(write(client_sock,secret,secret_max_size*sizeof(char))==-1){
        perror("write() error");
        return ERRWRT;
    }

    //Send secret to local server
    if(read(client_sock,&answer,sizeof(answer))==-1)
    {
        perror("No answer from local server");
        return ERRRD;
    }

    //Check if request was accepted
    if(answer<0){
        printf("Request denied\n");
        return DENIED;
    }

    return 0;
}


/*put_value()
Como explicado no enunciado, esta função coloca um valor para o valor key.
Retorna 1 em caso de sucesso ou uma flag de erro definida no Basic.h

Esta função cria primeiro uma conecção com sockets tipo UNIX STREAM and sends a group and secret which will be subject to authentication. 
If authentication was sucessful, returns 0*/
int put_value(char * key, char * value)
{
    int buf=PUT;
    long int vallen=strlen(value);

    if(write(client_sock,&buf,sizeof(int))==-1){
        perror("write(flag:PUT)  error");
        return ERRWRT;
    }

    if(write(client_sock,key,key_max_size* sizeof(char))==-1){
        perror("write(key)  error");
        return ERRWRT;
    }

    if(write(client_sock,&vallen,sizeof(long int))==-1){
        perror("write(vallen)  error");
        return ERRWRT;
    }

    if(write(client_sock,value,vallen*sizeof(char))==-1){
        perror("write(value)  error");
        return ERRWRT;
    }

    if(read(client_sock,&buf,sizeof(int))==-1)
    {
        perror("No answer from local server");
        return ERRRD;
    }

    return buf;
}


int get_value(char * key, char ** value)
{
    long int answer;
    int buf=GET;

    if(write(client_sock,&buf,sizeof(buf))==-1){
        perror("write(flag:GET)  error");
        return ERRWRT;
    }

    if(write(client_sock,key,key_max_size* sizeof(char))==-1){
        perror("write(key)  error");
        return ERRWRT;
    }



    if(read(client_sock,&answer,sizeof(answer))==-1)
    {
        perror("No answer from local server");
        return ERRRD;
    }
    if(answer==-1){
        perror("No key");
        return DENIED;
    }else if(answer==0){
        perror("No value");
        return -10;
    }

    *value = malloc (answer*sizeof(char));
    if (*value == NULL) {
        perror("Unable to alocate memory");
        return ERRMALLOC;
    }

    if(read(client_sock,*value,answer*sizeof(char))==-1)
    {
        perror("No answer from local server");
        return ERRRD;
    }

    return 1;
}


int delete_value(char * key)
{
    int buf=DEL;

    if(write(client_sock,&buf,sizeof(buf))==-1){
        perror("write(flag:DEL)  error");
        return ERRWRT;
    }
    if(write(client_sock,key,key_max_size* sizeof(char))==-1){
        perror("write(key)  error");
        return ERRWRT;
    }

    if(read(client_sock,&buf,sizeof(int))==-1)
    {
        perror("No answer from local server");
        return ERRRD;
    }

    return buf;
}

int register_callback(char * key, void (*callback_function)(char *)){
    int answer=CALL;
    pthread_t thread_id;
    
    if(write(client_sock,&answer,sizeof(int))==-1){
        perror("write(flag:CALL)  error");
        return ERRWRT;
    }

    if(write(client_sock,key, key_max_size* sizeof(char))==-1){
        perror("write(key)  error");
        return ERRWRT;
    }
    if(read(client_sock, &answer,sizeof(int)) ==-1){
        perror("No answer from local server");
        return ERRRD;
    }
    
    if(answer==1){
        if(pthread_create(&thread_id,NULL,(void *)callback_function,(void *)key)<0)
        {
            perror("Error creating thread");
            return ERRPTHR;
        }
    }else if(answer==DISCONNECTED){
        printf("Something went wrong\n");
    }else if(answer==0){
        return DENIED;
    }
    return 1;
}

int close_connection()
{
    int buf=CLS;

    if(write(client_sock,&buf,sizeof(buf))==-1){
        perror("write(flag:CLS)  error");
        return ERRWRT;
    }
    if(close(client_sock)<0)
    {
        perror("Error closing connection");
        return ERRCLS;
    }
    return 1;
}


