#include "Basic.h"
#include "Localserver_aux.h"

//Functions used to simplify code

pthread_mutex_t acess_auth;

/*createAndBind_UNIX_stream_Socket()   Deals with UNIX Stream socket creation and bidding

                Arguments:  sock_addr   - string with socket path
                        
                Returns:    sock            - file descriptor of the socket
                            ERRSCKCREATION  - Error creating socket
                            ERRSCKBIND      - Error binding socket*/
int createAndBind_UNIX_stream_Socket(char * sock_addr)
{
    int sock;
    struct sockaddr_un struct_sock_addr;
    //To make sure the address is free
    remove(server_addr);
    //Creating socket
    
    sock = socket(AF_UNIX, SOCK_STREAM, 0);
    if(sock==-1){
        perror("Error creating socket\n");
        return ERRSCKCREATION;
    }

    //Binding address
    memset(&struct_sock_addr,0,sizeof(struct sockaddr_un));
    struct_sock_addr.sun_family=AF_UNIX;
    strcpy(struct_sock_addr.sun_path, sock_addr);
    if(bind(sock, (struct sockaddr *)&struct_sock_addr, sizeof(struct_sock_addr)) < 0)
    {
        perror("Error binding socket\n");
        return ERRSCKBIND;
    }

    return sock;
}

/*accept_connection_timeout()   Accepts connections if timeout is not reached

                Arguments:  socket_af_stream    - file descriptor of the socket to acept connectios
                        
                Returns:    client_sock         - file descriptor of the socket to comunicate with the client
                            ERRTIMEOUT          - attempt connection timed out*/
int accept_connection_timeout(int * socket_af_stream)
{
    struct timeval tmout;
    fd_set rfds;
    FD_ZERO(&rfds);
    FD_SET(*socket_af_stream, &rfds);
    int client_sock=-1;

    tmout.tv_sec = (long)timeout;
    tmout.tv_usec = 0;

    if(select(*socket_af_stream+1, &rfds, (fd_set *) 0, (fd_set *) 0, &tmout)>0)
        client_sock = accept(*socket_af_stream,NULL,NULL);
    
    if(client_sock<0)
    {
        return ERRTIMEOUT;
    }
    return client_sock;
}

/*read_timeout()   Timeout for read functions

                Arguments:  socket_af_stream    - file descriptor of the socket to comunicate with the client
                            to_read             - buffer that the data will be placed in after reading
                            size_to_read        - max size of the buffer
                        
                Returns:    SUCCESS         - Read something within timeout
                            ERRTIMEOUT      - Has not read anything until timetout*/
int read_timeout(int * socket_af_stream, void * to_read, int size_to_read)
{
    struct timeval tmout;
    fd_set rfds;
    FD_ZERO(&rfds);
    FD_SET(*socket_af_stream, &rfds);
    

    tmout.tv_sec = (long)timeout;
    tmout.tv_usec = 0;

    if(select(*socket_af_stream+1, &rfds, (fd_set *) 0, (fd_set *) 0, &tmout)>0)
        read(*socket_af_stream,to_read,size_to_read);
    else
    {
        return ERRTIMEOUT;
    }
    return SUCCESS;
}

/*recv_timeout()   Timeout for recv functions

                Arguments:  socket_af_stream    - file descriptor of the socket to comunicate with the client
                            to_read             - buffer that the data will be placed in after reading
                            size_to_read        - max size of the buffer
                        
                Returns:    SUCCESS         - Read something within timeout
                            ERRTIMEOUT      - Has not read anything until timeout*/
int recv_timeout(int * socket_af_stream, void * to_read, int size_to_read)
{
    struct timeval tmout;
    fd_set rfds;
    FD_ZERO(&rfds);
    FD_SET(*socket_af_stream, &rfds);
    
    tmout.tv_sec = (long)timeout;
    tmout.tv_usec = 0;

    if(select(*socket_af_stream+1, &rfds, (fd_set *) 0, (fd_set *) 0, &tmout)>0)
    {
        recv(*socket_af_stream,to_read,size_to_read,0);
    }
    else
    {
        return ERRTIMEOUT;
    }
    return SUCCESS; 
}



/*createAuthServerSock()   Deals with the creation of the Auth server sock. It also assigns the address (IP and port) of the Auth Server to the Authserver_sock_addr

                Arguments:  port_str                - string with the port of the server
                            authaddr_str            - string with the IP of the server
                            Authserver_sock         - address of the file descriptor
                            Authserver_sock_addr    - socket address
                        
                Returns:    SUCCESS         - Created socket successfully
                            ERRSCKCREATION  - Error creating socket*/
int createAuthServerSock(char * port_str,char * authaddr_str, int * Authserver_sock, struct sockaddr_in * Authserver_sock_addr){
    socklen_t len = sizeof(struct sockaddr_in);

    //Creating socket
    *Authserver_sock = socket(AF_INET, SOCK_DGRAM, 0);
    if(*Authserver_sock==-1){
        return ERRSCKCREATION;
    }

    //Binding address
    memset(Authserver_sock_addr,0,sizeof(struct sockaddr_in));
    Authserver_sock_addr->sin_family=AF_INET;
    Authserver_sock_addr->sin_port=htons(atoi(port_str));
    Authserver_sock_addr->sin_addr.s_addr = inet_addr(authaddr_str);

    return SUCCESS;
}


/*AuthServerCom()   Deals with the communication between the local server and the Auth server

                Arguments:  request                 - What type of action you request to the server (PUT,GET...)
                            group                   - string with the group for the request
                            secret                  - string for the secret of the request, will also be used as the destination whenever the server returns the secret
                            Authserver_sock         - address of the file descriptor
                            Authserver_sock_addr    - socket address
                        
                Returns:    SUCCESS         - Action performed successfully (string returned)
                            answer          - response from the Auth Server
                            ERRTIMEOUT      - Waited too long for the Auth server*/
int AuthServerCom(int request, char * group, char * secret, int Authserver_sock, struct sockaddr_in Authserver_sock_addr){
    char * buf = malloc((group_id_max_size+2)*sizeof(char));
    int answer=0;
    int aux;

    sprintf(buf,"%d:%s",request,group);


    pthread_mutex_lock(&acess_auth);
    sendto(Authserver_sock,buf,(group_id_max_size+2)*sizeof(char),0, (struct sockaddr*)&Authserver_sock_addr,sizeof(struct sockaddr_in));

    if(request==GET || request==PUT)
    {
        aux=recv_timeout(&Authserver_sock,buf,secret_max_size*sizeof(char));
        if(aux==ERRTIMEOUT)
        {
            return ERRTIMEOUT;
        }
        pthread_mutex_unlock(&acess_auth);
        if(strcmp(buf, "\0") == 0)
        {
            return ERRRD;
        }
        strcpy(secret,buf);
        answer=SUCCESS;
    }
    else
    {
        aux=recv_timeout(&Authserver_sock,&answer,sizeof(answer));
        if(aux == ERRTIMEOUT)
        {
            return ERRTIMEOUT;
        }
        pthread_mutex_unlock(&acess_auth);
        if(request==CMP){
            strcpy(buf,secret);
            sendto(Authserver_sock,buf,secret_max_size*sizeof(char),0, (struct sockaddr*)&Authserver_sock_addr ,sizeof(struct sockaddr_in));
            if(recv_timeout(&Authserver_sock,&answer,sizeof(answer)) == ERRTIMEOUT)
            {
                return ERRTIMEOUT;
            }
        }
    }
    return answer;
}