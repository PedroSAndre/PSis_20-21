#include "Basic.h"
#include "Auth_group_secret.h"
#include <arpa/inet.h>



int main(int argc, char**argv)
{
    int u;
    int answer;
    int request, ident;
    int kvs_authserver_sock;
    unsigned short port;
    int kvs_localserver_sock;
    struct sockaddr_in kvs_authserver_sock_addr;
    struct sockaddr_in kvs_localserver_sock_addr;
    socklen_t len = sizeof(kvs_localserver_sock_addr);

    struct Message * Current, * Main=NULL;



    char * group=malloc(group_id_max_size*sizeof(char));
    char * secret;
    char * buf;

    if(argc !=3){
        perror("Wrong arguments");
        return 0;
    }

    port = htons(atoi(argv[2]));
    

    buf = malloc(1024*sizeof(char));

    //Creating socket
    kvs_authserver_sock = socket(AF_INET, SOCK_DGRAM, 0);
    if(kvs_authserver_sock==-1){
        perror("Error creating client socket\n");
        return -1;
    }

    //Binding address
    memset(&kvs_authserver_sock_addr,0,sizeof(struct sockaddr_in));
    kvs_authserver_sock_addr.sin_family=AF_INET;
    kvs_authserver_sock_addr.sin_port=port;
    kvs_authserver_sock_addr.sin_addr.s_addr=inet_addr(argv[1]);
    
    if(bind(kvs_authserver_sock,(struct sockaddr*) &kvs_authserver_sock_addr, sizeof(struct sockaddr_in)) < 0)
    {
        perror("Error binding socket\n");
        return -2;
    }

    //Waiting for connection cycle
    kvs_localserver_sock=0;
    memset(&kvs_localserver_sock_addr,0,sizeof(struct sockaddr_in));

    
    while(1){
        answer=0;
        

        if(recvfrom(kvs_authserver_sock,buf,sizeof(buf),0,(struct sockaddr *)&kvs_localserver_sock_addr,&len)<0)
        {
            perror("Error receving connection\n");
            return -3;
        }

        printf("Received: %s\n",buf);
        

        Current=recoverClientMessage(buf,kvs_localserver_sock_addr,&Main);

        if(Current==NULL){
            answer=-1;
        }else{
            if(strcmp(Current->group,"\0")==0){
                Current->request=WAIT;
                answer=-1;
            }
            if (Current->request==PUT){
                secret=generate_secret();
                if(secret==NULL){
                    answer=PUT;
                }
                strcpy(Current->secret,secret);
                if(CreateUpdateEntry(Current->group,Current->secret)==1){

                    sendto(kvs_authserver_sock,secret,sizeof(secret),0,(struct sockaddr * )&kvs_localserver_sock_addr,sizeof(struct sockaddr_in));

                    Main=deleteMessage(Current,Main);
                    answer=PUT;
                }else{
                    answer=PUT;
                }
            //Get secret
            }else if(Current->request==GET){
                secret=getGroupSecret(Current->group);
                
                sendto(kvs_authserver_sock,secret,sizeof(secret),0,(struct sockaddr * )&kvs_localserver_sock_addr,sizeof(struct sockaddr_in));

                free(Current->group);
                free(Current);
                answer=GET;
            //Delete entry for group
            }else if(Current->request==DEL){
                if(strcmp(Current->secret,"\0")!=0){
                    answer=DeleteEntry(Current->group,Current->secret);
                    Main=deleteMessage(Current,Main);
                }else{
                    answer=1;
                } 
            }else if(Current->request==CMP){
                if(strcmp(Current->secret,"\0")!=0){
                    answer=compareHashGroup(Current->group,Current->secret);
                    Main=deleteMessage(Current,Main);
                }else{
                    answer=1;
                }
            }else if(answer==0){
                answer=-1;
            }
        }

        
        if(answer!=GET && answer!=PUT){
            sendto(kvs_authserver_sock,&answer,sizeof(int),0,(struct sockaddr *)&kvs_localserver_sock_addr,sizeof(struct sockaddr_in));
            printf("Answer:%d\n",answer);
        }   
        
    }
    
    if(close(kvs_authserver_sock)<0)
        {
            perror("Error closing connection");
            return -5;
        }
    return 0;
}