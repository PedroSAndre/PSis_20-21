#include "Basic.h"
#include "Localserver_aux.h"

//Functions used to simplify code

//Returns binded socket (-1 if error)
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
        return -1;
    }

    //Binding address
    memset(&struct_sock_addr,0,sizeof(struct sockaddr_un));
    struct_sock_addr.sun_family=AF_UNIX;
    strcpy(struct_sock_addr.sun_path, sock_addr);
    if(bind(sock, (struct sockaddr *)&struct_sock_addr, sizeof(struct_sock_addr)) < 0)
    {
        perror("Error binding socket\n");
        return -1;
    }

    return sock;
}

//Accepts connection if timeout is not exeeded
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
        return -1;
    }
    return client_sock;
}




int CreateAuthServerSock(char * port_str,char * authaddr_str, int * Authserver_sock, struct sockaddr_in * Authserver_sock_addr){
    socklen_t len = sizeof(struct sockaddr_in);

    //Creating socket
    *Authserver_sock = socket(AF_INET, SOCK_DGRAM, 0);
    if(*Authserver_sock==-1){
        perror("Error creating main local server socket\n");
        return 0;
    }

    //Binding address
    memset(Authserver_sock_addr,0,sizeof(struct sockaddr_in));
    Authserver_sock_addr->sin_family=AF_INET;
    Authserver_sock_addr->sin_port=htons(atoi(port_str));
    Authserver_sock_addr->sin_addr.s_addr = inet_addr(authaddr_str);

    return 1;

}



int AuthServerCom(int request, char * group, char * secret, int Authserver_sock, struct sockaddr_in Authserver_sock_addr ){
    char * buf = malloc((group_id_max_size+2)*sizeof(char));
    int answer=0;
    socklen_t len = sizeof(struct sockaddr_in);



    sprintf(buf,"%d:%s",request,group);


    //From here until AuthCrit the function is critical and no other thread must comunicate with Auth server
    sendto(Authserver_sock,buf,(group_id_max_size+2)*sizeof(char),0, (struct sockaddr*)&Authserver_sock_addr ,len);

    if(request==GET || request == PUT){
        recvfrom(Authserver_sock,buf,(group_id_max_size+2)*sizeof(char),0,(struct sockaddr*)&Authserver_sock_addr ,&len);
        strcpy(secret,buf);
        answer=1;
    }else{
        recvfrom(Authserver_sock,&answer,sizeof(int),0,(struct sockaddr*)&Authserver_sock_addr ,&len);
        if(request==CMP){


            strcpy(buf,secret);
            sendto(Authserver_sock,buf,secret_max_size*sizeof(char),0, (struct sockaddr*)&Authserver_sock_addr ,len);


            recvfrom(Authserver_sock,&answer,sizeof(int),0,(struct sockaddr*)&Authserver_sock_addr ,&len);

    
        }
    }

    //AuthCrit
    return answer;

}