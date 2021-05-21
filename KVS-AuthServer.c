#include "Basic.h"
#include "Auth_group_secret.h"
#include <arpa/inet.h>


#define SIZE 101



struct Message{
    struct sockaddr_in clientaddr;
    int request;
    char * group;
    char * secret;
    struct Message * next;
};


struct HashGroup * Table[SIZE];


struct Message * recoverClientMessage(char * buf,struct sockaddr_in kvs_localserver_sock_addr,struct Message ** Main){
    int request=0,aux;
    char * group;
    char * secret;
    struct Message * Current, * Previous;


    Previous=NULL;
    Current=*Main;
    if(Current==NULL)
    {
        group=malloc(sizeof(char)*group_id_max_size);
        aux=sscanf(buf,"%d:%s",&request,group);
        if(aux!=2){
            return NULL;
        }
        Current=malloc(sizeof(struct Message));
        Current->clientaddr=kvs_localserver_sock_addr;
        Current->request=request;
        Current->group=malloc(1024*sizeof(char));
        strcpy(Current->group,group);
        if(request==PUT || request==CMP || request==DEL){
            *Main=Current;
            secret=malloc(sizeof(char)*secret_max_size);
            if(secret==NULL){
                perror("Error alocating memory");
                return NULL;
            }
            strcpy(secret,"\0");
            Current->secret=secret;
        }
    }else{
        

        while((Current->clientaddr.sin_addr.s_addr!=kvs_localserver_sock_addr.sin_addr.s_addr) && Current->next !=NULL){
            Previous=Current;
            Current=Current->next;
        }

        if(Current->clientaddr.sin_addr.s_addr!=kvs_localserver_sock_addr.sin_addr.s_addr){
            sscanf(buf,"%d:%s",&request,group);
            if(aux!=2){
                return NULL;
            }
            Current=malloc(sizeof(struct Message));
            Current->clientaddr=kvs_localserver_sock_addr;
            Current->group=malloc(1024*sizeof(char));
            strcpy(Current->group,group);
            Current->request=request;
                if(request==PUT || request==CMP || request==DEL){
                    secret=malloc(sizeof(char)*secret_max_size);
                if(secret==NULL){
                    perror("Error alocating memory");
                    return NULL;
                }
                strcpy(secret,"\0");
                Current->secret=secret;
                Previous=Current;
                Previous->next=Current;
            }
        }else{     
            strcpy(Current->secret,buf);
        }
    }
    return Current;
}

struct Message * deleteMessage(struct Message * Current, struct Message * Main){
    struct Message * Previous;
    if(Current==Main){
        free(Current->group);
        free(Current->secret);
        free(Current);
        return NULL;
    }
    Previous=Main;
    while(Previous->next!=Current){
        Previous=Previous->next;
    }
    free(Current->group);
    free(Current->secret);
    Previous->next=Current->next;
    free(Current);
    return Main;
}

int main(void)
{
    int u;
    int answer;
    int request, ident;
    int kvs_authserver_sock;
    int kvs_localserver_sock;
    struct sockaddr_in kvs_authserver_sock_addr;
    struct sockaddr_in kvs_localserver_sock_addr;

    struct Message * Current, * Main=NULL;



    char * group=malloc(1024*sizeof(char));
    char * secret;
    char * buf;
    

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
    kvs_authserver_sock_addr.sin_port=htons(8000);
    kvs_authserver_sock_addr.sin_addr.s_addr=INADDR_ANY;
    if(bind(kvs_authserver_sock,(struct sockaddr*) &kvs_authserver_sock_addr, sizeof(struct sockaddr_in)) < 0)
    {
        perror("Error binding socket\n");
        return -2;
    }

    //Waiting for connection cycle
    kvs_localserver_sock=0;
    memset(&kvs_localserver_sock_addr,0,sizeof(struct sockaddr_in));
    kvs_localserver_sock_addr.sin_addr.s_addr=inet_addr("127.0.0.1");

    
    while(1){
        answer=0;
        

        if(recvfrom(kvs_authserver_sock,buf,sizeof(buf),0,&kvs_localserver_sock_addr,sizeof(struct sockaddr_in))<0)
        {
            perror("Error receving connection\n");
            return -3;
        }
        

        Current=recoverClientMessage(buf,kvs_localserver_sock_addr,&Main);

        if(Current==NULL){
            answer=-1;
        }else{
            if(strcmp(Current->group,"\0")==0){
                Current->request=WAIT;
                answer=-1;
            }
            if (Current->request==PUT){
                if(strcmp(Current->secret,"\0")!=0){
                    if(CreateUpdateEntry(Current->group,Current->secret)==1){
                        Main=deleteMessage(Current,Main);
                        answer=1;
                    }else{
                        answer=-1;
                    }
                }else{
                    answer=1;
                }
            //Get secret
            }else if(Current->request==GET){
                secret=getGroupSecret(Current->group);
                
                sendto(kvs_authserver_sock,secret,sizeof(secret),0,&kvs_localserver_sock_addr,sizeof(struct sockaddr_in));
                printf("Secret:%s\n",secret);

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

        
        if(answer!=GET){
            sendto(kvs_authserver_sock,&answer,sizeof(int),0,&kvs_localserver_sock_addr,sizeof(struct sockaddr_in));
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