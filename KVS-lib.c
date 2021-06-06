#include "Basic.h"
#include "KVS-lib.h"


//Defined client_sock here as it is only used in this library
int client_sock;

/*establish_conection() This function receives as arguments the strings containing the group name and corresponding secret, and tries to open the connection with the 
                        KVS-LocalServer. If successful, all following operations on key-value pairs are done in then context of the provided group_id.
                        
                Arguments:      group_id    - string that contains the group_id of the table we want to acess in KVS-LocalServer
                                secret      - string that contains the secret corresponding to the group, for authentication
                        
                Returns:    0                   - Connection sucessful
                            ERRSCKCREATION      - Error creating socket
                            ERRCONNECTING       - Error connecting to local server
                            DISCONNECTED        - Server disconnected
                            ERRWRT              - Failed to write in socket
                            ERRRD               - Group not found
                            DENIED              - Incorrect group or secret*/
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
        return DISCONNECTED;
    }
    signal(SIGPIPE, SIG_IGN);//Accept write errors
    


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
        return DISCONNECTED;
    }

    //Check if request was accepted
    if(answer<0){
        return answer;
    }

    return 0;
}


/*put_value()   This function tries to assign the provided value to the provided key. If the provided key-pairdoes not exist in the server, it is created.
                If the key-value pair already exists in the server itis updated with the provided value.
                        
                Arguments:      key     - string that contains the key of the pair key-value we want to add
                                value   - string that contains the value of the pair key-value we want to add
                        
                Returns:    1                   - Created the key-value pair sucessfully
                            DISCONNECTED        - Server disconnected
                            ERRWRT              - Failed to write in socket
                            ERRRD               - key not found
                            ERRMALLOC           - Server was unable to alocate memory for the new key-value pairt*/
int put_value(char * key, char * value)
{
    int buf;
    long int vallen=strlen(value);

    buf=PUT;
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
        return DISCONNECTED;
    }

    if(buf==SUCCESS){
        return 1;
    }

    return buf;
}

/*get_value()   This function tries to retrieve the value associated to the provided key. If the provided key-pair exists in the server the corresponding value is 
                “returned” through the value argument. This function shold do a malloc that will store the calue associated with the key.
                        
                Arguments:      key     - string that contains the key of the pair key-value we want to find the value of
                                value   - address to the string that the value will be placed in
                        
                Returns:    1                   - Sucessfully recovered value
                            DISCONNECTED        - Server disconnected
                            ERRWRT              - Failed to write in socket
                            ERRRD               - key not found
                            ERRMALLOC           - Server was unable to alocate memory for the value*/
int get_value(char * key, char ** value)
{
    long int answer;
    int buf;

    buf=GET;

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
        return DISCONNECTED;
    }
    if(answer==ERRRD){
        perror("No key");
        return answer;
    }

    *value = malloc (answer*sizeof(char));
    if (*value == NULL) {
        perror("Unable to alocate memory");
        return ERRMALLOC;
    }

    if(read(client_sock,*value,answer*sizeof(char))==-1)
    {
        perror("No answer from local server");
        return DISCONNECTED;
    }

    return 1;
}

/*delete_value()    This function tries to retrieve delete the pair key-value associated to the provided key.
                        
                Arguments:      key     - string that contains the key of the pair key-value we want to delete
                        
                Returns:    1                   - Sucessfully recovered value
                            DISCONNECTED        - Server disconnected
                            ERRWRT              - Failed to write in socket
                            ERRRD               - key not found*/
int delete_value(char * key)
{
    int buf;

    buf=DEL;

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
        return DISCONNECTED;
    }

    if(buf==SUCCESS){
        return 1;
    }

    return buf;
}


/*register_callback()   This function tries to register a callback the will later be called. The function receives the keythat will be monitored and the pointer to the 
                        function that will be executed in the applications. When th value associated with the key is changed.
                        
                Arguments:      key                 - string that contains the key of the pair key-value we want to monitor
                                callback_function   - funtion to be called as a thread after a change in key
                        
                Returns:    1                   - Sucessfully recovered value
                            DISCONNECTED        - Server disconnected
                            ERRWRT              - Failed to write in socket
                            ERRRD               - key not found
                            ERRPTHR             - Eror creating the thread*/
int register_callback(char * key, void (*callback_function)(char *)){
    int answer;
    pthread_t thread_id;

    answer=CALL;
    
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
        return DISCONNECTED;
    }
    
    if(answer==SUCCESS){
        if(pthread_create(&thread_id,NULL,(void *)callback_function,(void *)key)<0)
        {
            perror("Error creating thread");
            return ERRPTHR;
        }
    }else{
        return answer;
    }
    return 1;
}

/*close_connection()   This function closes the connection previously opened.
                        
                Arguments:      
                        
                Returns:    1                   - Sucessfully recovered value
                            DISCONNECTED        - Server disconnected
                            ERRWRT              - Failed to write in socket
                            ERRCLS              - Error closing connection*/
int close_connection()
{
    int buf;

    buf=CLS;

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


