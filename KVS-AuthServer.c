#include "Basic.h"


#define max_waiting_connections 10
#define SIZE 101

struct HashGroup {
    char * group;
    char * secret;
    struct HashGroup * next;
};

struct Message{
    struct sockaddr_in clientaddr;
    int request;
    char * group;
    char * secret;
    struct Message * next;
};

struct HashGroup * Table[SIZE];


int HashIndex(char * group){
    int i=0;
    int Index=0;
    while(group[i]!='\0' && i<group_id_max_size){
        Index=(group[i]+Index)%SIZE;
        i++;
    }
    return Index;
}

int CreateUpdateEntry(char * group,char *secret){
    int TableIndex=HashIndex(group);
    struct HashGroup * Current,* Previous;
    
    struct HashGroup * Novo;



    Current=Table[TableIndex];
    if(Current==NULL){
        Novo=malloc(sizeof(struct HashGroup));
        if (Novo ==NULL){
            perror("Error alocating memory");
            return -1;
        }
        Novo->group=malloc(group_id_max_size*sizeof(char));
        Novo->secret=malloc(secret_max_size*sizeof(char));
        strcpy(Novo->group,group);
        strcpy(Novo->secret,secret);
        Novo->next=NULL;
        Table[TableIndex]=Novo;
        return 1;
    }

    while(Current!=NULL){

        if(strcmp(Current->group,group)==0){
            strcpy(Current->secret,secret);
            return 1;
        }
        if(Current->next==NULL){
            Novo=malloc(sizeof(struct HashGroup));
            if (Novo ==NULL){
                perror("Error alocating memory");
                return -1;
            }
            Novo->group=malloc(group_id_max_size*sizeof(char));
            Novo->secret=malloc(secret_max_size*sizeof(char));
            strcpy(Novo->group,group);
            strcpy(Novo->secret,secret);
            Novo->next=NULL;
            Current->next=Novo;
            return 1;
        }
        Current=Current->next;

    }

}

int DeleteEntry(char * group, char * secret){
    int TableIndex=HashIndex(group);
    struct HashGroup * Current,* Previous;
    
    Current=Table[TableIndex];
    if(Current==NULL){
        perror("Entry to delete not found");
        return 0;
    }

    if(strcmp(Current->group,group)==0){
        if(strcmp(Current->secret,secret)==0){
            Table[TableIndex]=Current->next;
            free(group);
            free(secret);
            free(Current);
            return 1;
        }else{
            perror("Delete request denied");
            return -1;
        }
    }
    Previous=Current;
    Current=Current->next;

    while(Current!=NULL){

        if(strcmp(Current->group,group)==0){
            if(strcmp(Current->secret,secret)==0){
                if(Current->secret==secret){
                    Previous->next=Current->next;
                    free(Current);
                    return 1; 
                }else{
                    perror("Delete request denied");
                    return -1;
                }
            }
        }
        Previous=Current;
        Current=Current->next;
    }
    perror("Entry to delete not found");
    return 0;

}

int compareHashGroup(char * group, char * checksecret){
    char * secret;

    secret=getGroupSecret(group);
    if(secret!=NULL){
        if(strcmp(secret,checksecret)==0){
            return 1;
        }else{
            return 0;
        }
    }else{
        return -1;
    }
    
}

char * getGroupSecret(char * group){
    int TableIndex=HashIndex(group);
    int result=0;
    struct HashGroup * Current,* Previous;
    
    Current=Table[TableIndex];
    if(Current==NULL){
        perror("No entry for this group");
        return NULL;
    }
    
    while(Current!=NULL){
        if(strcmp(Current->group,group)==0){
            return Current->secret;
        }
        Current=Current->next;

    }
    perror("No entry for this group");
    return NULL;
}


struct Message * recoverClientMessage(char * buf,struct sockaddr_in kvs_localserver_sock_addr,struct Message * Main){
    int request;
    char * group=malloc(sizeof(char)*group_id_max_size);
    struct Message * Current, * Previous;


    Previous=NULL;
    Current=Main;
    if(Current==NULL)
    {
        sscanf(buf,"%d:%s",&request,group);
        Current=malloc(sizeof(struct Message));
        Current->clientaddr=kvs_localserver_sock_addr;
        Current->request=request;
        Current->group=malloc(1024*sizeof(char));
        strcpy(Current->group,group);
        if(request==PUT || request==CMP){
            Main=Current;
            Current->secret=malloc(secret_max_size*sizeof(char));
            Current->secret="\0";
        }
    }

    while((Current->clientaddr.sin_addr.s_addr!=kvs_localserver_sock_addr.sin_addr.s_addr) && Current->next !=NULL){
        Previous=Current;
        Current=Current->next;
    }
    if(Current->clientaddr.sin_addr.s_addr!=kvs_localserver_sock_addr.sin_addr.s_addr){
        
        sscanf(buf,"%d:%s",&request,group);
        Current=malloc(sizeof(struct Message));
        Current->clientaddr=kvs_localserver_sock_addr;
        Current->group=malloc(1024*sizeof(char));
        strcpy(Current->group,group);
        Current->request=request;
        if(request==PUT || request==CMP || request==DEL){
            Current->secret="\0";
            Previous=Current;
            Previous->next=Current;
        }
    }else{
        sscanf(buf,"%s",Current->secret);
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
    if(bind(kvs_authserver_sock, &kvs_authserver_sock_addr, sizeof(kvs_authserver_sock_addr)) < 0)
    {
        perror("Error binding socket\n");
        return -2;
    }

    //Waiting for connection cycle
    kvs_localserver_sock=0;
    memset(&kvs_localserver_sock_addr,0,sizeof(struct sockaddr_in));


    while(1){

        if(recvfrom(kvs_authserver_sock,buf,sizeof(buf),0,&kvs_localserver_sock_addr,sizeof(struct sockaddr_in))<0)
        {
            perror("Error receving connection\n");
            return -3;
        }


        Current=recoverClientMessage(buf,kvs_authserver_sock_addr,Main);


        if (Current->request==PUT){
            if(Current->secret!="\0"){
                if(CreateUpdateEntry(Current->group,Current->secret)==1){
                    Main=deleteMessage(Current,Main);
                    answer=1;
                }else{
                    answer=-1;
                }
            }
        //Get secret
        }else if(Current->request==GET){
            secret=getGroupSecret(Current->group);
            sendto(kvs_authserver_sock,secret,sizeof(secret),0,&kvs_localserver_sock_addr,sizeof(struct sockaddr_in));

            free(Current->group);
            free(Current);
            answer=GET;
        //Delete entry for group
        }else if(Current->request==DEL){
            if(Current->secret!="\0"){
                answer=DeleteEntry(Current->group,Current->secret);
                Main=deleteMessage(Current,Main);
            } 
        }else if(Current->request==CMP){
            if(Current->secret!="\0"){
                answer=compareHashGroup(Current->group,Current->secret);
                Main=deleteMessage(Current,Main);
            }         
        }

        if(answer!=GET){
            sendto(kvs_authserver_sock,&answer,sizeof(int),0,&kvs_localserver_sock_addr,sizeof(struct sockaddr_in));
        }   
    }
    
    if(close(kvs_authserver_sock)<0)
        {
            perror("Error closing connection");
            return -5;
        }
    return 0;
}