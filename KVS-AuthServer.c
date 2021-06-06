#include "Basic.h"
#include "Authserver.h"

#define SKIPWRTANSWER 2

//Thread functions
void quit_auth(void *arg);

int server_status=1;

/*KVS-AuthServer    This program will be reponsible for receiving requests from the local server, such as, to create, delete, get the secret of groups and even 
                    authenticate group-secret pairs.

                    Arguments: argv[1]      - Takes the port as an argument. This is the port the sockets will connect to.*/
int main(int argc, char**argv)
{
    int err=0;
    int answer;
    int kvs_authserver_sock;
    int kvs_localserver_sock;
    struct sockaddr_in kvs_authserver_sock_addr;
    struct sockaddr_in kvs_localserver_sock_addr;
    socklen_t len = sizeof(kvs_localserver_sock_addr);
    pthread_t quit_auth_thread_ptid;

    struct Message * Current, * Main=NULL;



    char group[group_id_max_size];
    char newsecret[secret_max_size];
    char * secret;
    char buf[group_id_max_size+2];

    srand(time(NULL));

    if(pthread_create(&quit_auth_thread_ptid,NULL,(void *)&quit_auth,NULL)<0)
    {
        perror("Error creating thread\n");
        return ERRPTHR;
    }

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

    kvs_localserver_sock=0;
    memset(&kvs_localserver_sock_addr,0,sizeof(struct sockaddr_in));


    //Connection cycle
    while(server_status){
        if(recvfrom_timeout(&kvs_authserver_sock,buf,(group_id_max_size+2)*sizeof(char),(struct sockaddr *)&kvs_localserver_sock_addr,&len)==ERRTIMEOUT)
        {
            //Timed out. Usefull when an order for shutdown has been registered
            continue;
        }
        printf("Received: %s\n",buf);
        answer=0;
        
        //If it is CMP we need to assign to a list so next time this local connects, we make sure to place the secret on the right place
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
            //Creates group - returns the secret or NULL. IF unable to alocate memory shutsdown server
            }else if (Current->request==PUT){
                generate_secret(newsecret);
                if(createUpdateEntry(Current->group,newsecret)==SUCCESS){
                    sendto(kvs_authserver_sock,newsecret,secret_max_size*sizeof(char),0,(struct sockaddr * )&kvs_localserver_sock_addr,sizeof(struct sockaddr_in));
                    printf("Secret:%s\n",newsecret);

                    free(Current);
                    answer=SKIPWRTANSWER;
                }else{
                    free(Current);
                    break;
                }
            //Get secret - returns secret if sucessful
            }else if(Current->request==GET){
                secret=getGroupSecret(Current->group);
                
                sendto(kvs_authserver_sock,secret,secret_max_size* sizeof(char),0,(struct sockaddr * )&kvs_localserver_sock_addr,sizeof(struct sockaddr_in));
                printf("Secret:%s\n",secret);

                free(Current);
                answer=SKIPWRTANSWER;
            //Delete entry for group - returns SUCCESS if successful
            }else if(Current->request==DEL){
                answer=deleteEntry(Current->group);
                free(Current);
            //Makes authentication - returns SUCCESS if successful
            }else if(Current->request==CMP){
                if(strcmp(Current->secret,"\0")!=0){
                    answer=compareHashGroup(Current->group,Current->secret);
                    Main=deleteMessage(Current,Main);
                }else{
                    answer=SUCCESS;
                }
            //Format wrong
            }else if(answer==0){
                answer=DENIED;
            }
        }

        
        if(answer!=SKIPWRTANSWER){
            sendto(kvs_authserver_sock,&answer,sizeof(int),0,(struct sockaddr *)&kvs_localserver_sock_addr,sizeof(struct sockaddr_in));
            printf("Answer:%d\n",answer);
        }   
    }

    pthread_join(quit_auth_thread_ptid, NULL);
    delete_All_messages(Main);
    delete_All_Entries();
    
    if(close(kvs_authserver_sock)<0)
        {
            perror("Error closing connection");
            return ERRCLS;
        }
        
    printf("Server terminated sucessfully\n");
    return SUCCESS;
}


//Thread functions
/*quit_auth     This thread will be called in the beginning and will make a control section for the user. In other words, by clicking enter changes 
                the value of server_status which will begin a shutdown order of the server*/
void quit_auth(void *arg)
{
    printf("*****Welcome to KVS-Auth-Server*****\nTo exit you just need to press enter\n");
    getchar();
    server_status = 0; //Closes server
    pthread_exit(NULL);
}