#include "Basic.h"
#include "Authserver.h"
#include <arpa/inet.h>



int main(int argc, char**argv)
{
    int u, aux;
    int answer;
    int request, ident;
    int kvs_authserver_sock;
    int kvs_localserver_sock;
    struct sockaddr_in kvs_authserver_sock_addr;
    struct sockaddr_in kvs_localserver_sock_addr;
    socklen_t len = sizeof(kvs_localserver_sock_addr);

    struct Message * Current, * Main=NULL;



    char * group=malloc(group_id_max_size*sizeof(char));
    char * secret;
    char * buf;

    srand(time(NULL));

    if(argc !=2){
        perror("Wrong arguments");
        return 0;
    }
    

    buf = malloc((group_id_max_size+2)*sizeof(char));

    //Creating socket
    kvs_authserver_sock = socket(AF_INET, SOCK_DGRAM, 0);
    if(kvs_authserver_sock==-1){
        perror("Error creating client socket\n");
        return -1;
    }

    //Binding address
    memset(&kvs_authserver_sock_addr,0,sizeof(struct sockaddr_in));
    kvs_authserver_sock_addr.sin_family=AF_INET;
    kvs_authserver_sock_addr.sin_port=htons(atoi(argv[1]));
    kvs_authserver_sock_addr.sin_addr.s_addr=htonl(INADDR_ANY);


    
    if(bind(kvs_authserver_sock,(struct sockaddr*) &kvs_authserver_sock_addr, sizeof(struct sockaddr_in)) < 0)
    {
        perror("Error binding socket\n");
        return -2;
    }

    printf("Sucessfully connected\n");
    printf("IP address is: %s\n", inet_ntoa(kvs_authserver_sock_addr.sin_addr));
    printf("Port is: %d\n", (int) ntohs(kvs_authserver_sock_addr.sin_port));

    //Waiting for connection cycle
    kvs_localserver_sock=0;
    memset(&kvs_localserver_sock_addr,0,sizeof(struct sockaddr_in));


    while(1){
        answer=0;

        if(recvfrom(kvs_authserver_sock,buf,(group_id_max_size+2)*sizeof(char),0,(struct sockaddr *)&kvs_localserver_sock_addr,&len)<0)
        {
            perror("Error receving connection\n");
            return -3;
        }
        printf("Received: %s\n",buf);
        

        Current=recoverClientMessage(buf,kvs_localserver_sock_addr,&Main);

        if(Current==NULL){
            printf("Current NULL\n");
            answer=-1;
        }else{
            if(strcmp(Current->group,"\0")==0){
                Current->request=WAIT;
                free(Current);
                answer=-1;
            }
            if (Current->request==PUT){
                secret=generate_secret();
                if(secret==NULL){
                    answer=PUT;
                }
                strcpy(Current->secret,secret);
                if(CreateUpdateEntry(Current->group,Current->secret)==1){

                    sendto(kvs_authserver_sock,secret,secret_max_size*sizeof(char),0,(struct sockaddr * )&kvs_localserver_sock_addr,sizeof(struct sockaddr_in));
                    printf("Secret:%s\n",secret);

                    free(Current);
                    answer=GET;
                }else{
                    answer=GET;
                }
            //Get secret
            }else if(Current->request==GET){
                secret=getGroupSecret(Current->group);
                
                sendto(kvs_authserver_sock,secret,secret_max_size* sizeof(char),0,(struct sockaddr * )&kvs_localserver_sock_addr,sizeof(struct sockaddr_in));
                printf("Secret:%s\n",secret);

                free(Current);
                answer=GET;
            //Delete entry for group
            }else if(Current->request==DEL){
                answer=DeleteEntry(Current->group);
                free(Current);
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