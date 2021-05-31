#include "Basic.h"
#include "key_value_struct.h"
#include <arpa/inet.h>


struct message
{
    int answer;
    char group[group_id_max_size];
    char secret[secret_max_size];
    int request;
    struct message * next;
};

struct message * Main=NULL;

int main(int argc, char**argv){
    int Authserver_sock;
    unsigned short port;
    struct sockaddr_in * Authserver_sock_addr=malloc(sizeof(struct sockaddr_in));
    socklen_t len = sizeof (struct sockaddr_in);
    char * buf=malloc(1050*sizeof(char));
    int answer;

    if(argc !=3){
        perror("Wrong arguments");
        return 0;
    }

    port = htons(atoi(argv[2]));

    //Creating socket
    Authserver_sock = socket(AF_INET, SOCK_DGRAM, 0);
    if(Authserver_sock==-1){
        perror("Error creating main local server socket\n");
        return -1;
    }

    //Binding address
    memset(Authserver_sock_addr,0,sizeof(struct sockaddr_in));
    Authserver_sock_addr->sin_family=AF_INET;
    Authserver_sock_addr->sin_port=port;
    Authserver_sock_addr->sin_addr.s_addr = inet_addr(argv[1]);

    while(1){
        while(Main!=NULL){
            sprintf(buf,"%d:%s",Main->request,Main->group);
            sendto(Authserver_sock,buf,sizeof(buf),0, (struct sockaddr*)Authserver_sock_addr ,len);

            if(Main->request==GET){
                recvfrom(Authserver_sock,buf,sizeof(buf),0,(struct sockaddr*)Authserver_sock_addr ,&len);
                strcpy(Main->secret,buf);
            }else{
                recvfrom(Authserver_sock,&answer,sizeof(int),0,(struct sockaddr*)Authserver_sock_addr ,&len);
                if(Main->request==DEL || Main->request==PUT || Main->request==CMP){
                    strcpy(buf,Main->secret);
                    sendto(Authserver_sock,buf,sizeof(buf),0, (struct sockaddr*)Authserver_sock_addr ,len);
                    recvfrom(Authserver_sock,&answer,sizeof(int),0,(struct sockaddr*)Authserver_sock_addr ,&len);

                }
                Main->answer=answer;
            }
            Main->request=WAIT;
            Main=Main->next;
        }
    }


    

    close(Authserver_sock);

}