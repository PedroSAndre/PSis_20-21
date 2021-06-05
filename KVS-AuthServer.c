#include "Basic.h"
#include "Authserver.h"
#include <arpa/inet.h>

#define SKIPWRTANSWER 2

int main(int argc, char**argv)
{
    int * err;
    int answer;
    int kvs_authserver_sock;
    int kvs_localserver_sock;
    struct sockaddr_in kvs_authserver_sock_addr;
    struct sockaddr_in kvs_localserver_sock_addr;
    socklen_t len = sizeof(kvs_localserver_sock_addr);

    struct Message * Current, * Main=NULL;



    char group[group_id_max_size];
    char newsecret[secret_max_size];
    char * secret;
    char buf[group_id_max_size+2];

    srand(time(NULL));

    if(argc !=2){
        perror("Wrong arguments");
        return WRNGARG;
    }

    //Creating socket
    kvs_authserver_sock = socket(AF_INET, SOCK_DGRAM, 0);
    if(kvs_authserver_sock==-1){
        perror("Error creating client socket\n");
        return ERRSCKCREATION;
    }

    //Binding address
    memset(&kvs_authserver_sock_addr,0,sizeof(struct sockaddr_in));
    kvs_authserver_sock_addr.sin_family=AF_INET;
    kvs_authserver_sock_addr.sin_port=htons(atoi(argv[1]));
    kvs_authserver_sock_addr.sin_addr.s_addr=htonl(INADDR_ANY);


    
    if(bind(kvs_authserver_sock,(struct sockaddr*) &kvs_authserver_sock_addr, sizeof(struct sockaddr_in)) < 0)
    {
        perror("Error binding socket\n");
        return ERRSCKBIND;
    }

    printf("Sucessfully connected\n");
    printf("IP address is: %s\n", inet_ntoa(kvs_authserver_sock_addr.sin_addr));
    printf("Port is: %d\n", (int) ntohs(kvs_authserver_sock_addr.sin_port));

    //Waiting for connection cycle
    kvs_localserver_sock=0;
    memset(&kvs_localserver_sock_addr,0,sizeof(struct sockaddr_in));


    while(1){
        answer=0;

        if(recvfrom(kvs_authserver_sock,buf,(group_id_max_size+2)*sizeof(char),0,(struct sockaddr *)&kvs_localserver_sock_addr,&len)==-1)
        {
            perror("Error receving connection\n");
            break;
        }
        printf("Received: %s\n",buf);
        

        Current=recoverClientMessage(buf,kvs_localserver_sock_addr,&Main,&err);

        if(Current==NULL){
            if(err==ERRMALLOC){
                break;
            }
            printf("Current NULL\n");
            answer=DENIED;
        }else{
            if(strcmp(Current->group,"\0")==0){
                Current->request=WAIT;
                free(Current);
                answer=DENIED;
            }else if (Current->request==PUT){
                generate_secret(newsecret);
                strcpy(Current->secret,newsecret);
                if(CreateUpdateEntry(Current->group,Current->secret)==1){

                    sendto(kvs_authserver_sock,newsecret,secret_max_size*sizeof(char),0,(struct sockaddr * )&kvs_localserver_sock_addr,sizeof(struct sockaddr_in));
                    printf("Secret:%s\n",newsecret);

                    free(Current);
                    answer=SKIPWRTANSWER;
                }else{
                    free(Current);
                    break;
                }
            //Get secret
            }else if(Current->request==GET){
                secret=getGroupSecret(Current->group);
                
                sendto(kvs_authserver_sock,secret,secret_max_size* sizeof(char),0,(struct sockaddr * )&kvs_localserver_sock_addr,sizeof(struct sockaddr_in));
                printf("Secret:%s\n",secret);

                free(Current);
                answer=SKIPWRTANSWER;
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
                answer=DENIED;
            }
        }

        
        if(answer!=SKIPWRTANSWER){
            sendto(kvs_authserver_sock,&answer,sizeof(int),0,(struct sockaddr *)&kvs_localserver_sock_addr,sizeof(struct sockaddr_in));
            printf("Answer:%d\n",answer);
        }   
    }

    delete_All_messages(Main);
    delete_ALL_Entries();
    
    if(close(kvs_authserver_sock)<0)
        {
            perror("Error closing connection");
            return ERRCLS;
        }
    return 0;
}