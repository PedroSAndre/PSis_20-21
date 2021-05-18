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
    struct Message * next;
};

struct HashGroup *Table[SIZE];


int HashIndex(char * group){
    int i=0;
    int Index;
    while(group[i]!='\0' && i<1024){
        Index=(group[i]+Index)%SIZE;
        i++;
    }
    return Index;
}

int CreateUpdateEntry(char * group,char *secret){
    int TableIndex=HashIndex(group);
    struct HashGroup * Current,* Previous;
    
    struct HashGroup * Novo=malloc(sizeof(struct HashGroup));


    Current=Table[TableIndex];
    if(Current==NULL){
        Novo==malloc(sizeof(struct HashGroup));
        strcpy(Novo->group,group);
        strcpy(Novo->secret,secret);
        Table[TableIndex]=Novo;
        return 1;
    }

    while(Current!=NULL){

        if(strcmp(Current->group,group)==0){
            strcpy(Novo->secret,secret);
            return 1;
        }
        if(Current->next==NULL){
            Current->next=Novo;
            return 1;
        }
        Current=Current->next;

    }
    perror("Hash Error");
    return 0;

}

int DeleteEntry(char * group){
    int TableIndex=HashIndex(group);
    struct HashGroup * Current,* Previous;
    

    Current=Table[TableIndex];
    if(Current==NULL){
        perror("Entry to delete not found");
        return 0;
    }

    if(strcmp(Current->group,group)==0){
        Table[TableIndex]=Current->next;
        free(Current);
        return 1;
    }
    Previous=Current;
    Current=Current->next;

    while(Current!=NULL){

        if(strcmp(Current->group,group)==0){
            Previous->next=Current->next;
            free(Current);
            return 1;
        }
        Previous=Current;
        Current=Current->next;

    }
    perror("Entry to delete not found");
    return 0;

}

int compareHashGroup(char * group, char * secret){
    int TableIndex=HashIndex(group);
    int result=0;
    struct HashGroup * Current,* Previous;
    
    Current=Table[TableIndex];
    if(Current==NULL){
        perror("No entry for this group");
        return -1;
    }
    
    while(Current!=NULL){

        if(strcmp(Current->group,group)==0){
            if(strcmp(Current->secret,secret)==0){
                return 1;
            }
            return 0;
        }
        Current=Current->next;

    }
    perror("No entry for this group");
    return -1;
}





int main(void)
{
    int answer;
    int request, ident;
    int kvs_authserver_sock;
    int kvs_localserver_sock;
    struct sockaddr_in kvs_authserver_sock_addr;
    struct sockaddr_in kvs_localserver_sock_addr;

    struct Message * Current, * Main, *Previous;




    char * group_id;
    char * secret;
    char * buf;

    group_id = malloc(1024*sizeof(char));
    secret = malloc(1024*sizeof(char));
    buf = malloc(1030*sizeof(char));



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

        
        Current=Main;
        if(Current==NULL)
        {
            sscanf(buf,"%d",&request);
            Current=malloc(sizeof(struct Message));
            Main=Current;
            Current->clientaddr=kvs_localserver_sock_addr;
            Current->request=request;
        }

        while((Current->clientaddr.sin_addr.s_addr!=kvs_localserver_sock_addr.sin_addr.s_addr) && Current->next !=NULL){
            Previous=Current;
            Current=Current->next;
        }
        if(Current->clientaddr.sin_addr.s_addr!=kvs_localserver_sock_addr.sin_addr.s_addr){
            Previous=Current;
            sscanf(buf,"%d",&request);
            Current=malloc(sizeof(struct Message));
            Current->clientaddr=kvs_localserver_sock_addr;
            Current->request=request;
            Previous->next=Current;
        }else if(Current->request==WAIT){
            sscanf(buf,"%d",&request);
            Current->request=request;
        }else if (Current->request==PUT){
            if(Current->group=='\0'){
                strcpy(Current->group,buf);
            }else{
                if(CreateUpdateEntry(Current->group,buf)==1){
                    strcpy(Current->group,'\0');
                    Current->request=0;
                }else{
                    perror("Something went wrong");
                    return -5;
                }
            }
        }else if(Current->request==GET){
            if(Current->group=='\0'){
                strcpy(Current->group,buf);
            }else{
                answer=compareHashGroup(Current->group,buf);
                if(answer>=0){
                    strcpy(Current->group,'\0');
                    Current->request=0;
                }else{
                    perror("Something went wrong");
                    return -5;
                }
            }
        }else if(Current->request==DEL){
            if(Current->group=='\0'){
               ;
                if(DeleteEntry(buf)==1){
                    strcpy(Current->group,'\0');
                    Current->request=0;
                }else{
                    perror("Something went wrong");
                    return -5;
                }
                strcpy(Current->group,buf);
            }
        }

        
        if(bind(kvs_localserver_sock, &kvs_localserver_sock_addr, sizeof(kvs_localserver_sock_addr)) < 0)
        {
            perror("Error binding socket\n");
            return -2;
        }
        


        if(close(kvs_localserver_sock)<0)
        {
            perror("Error closing connection");
            return -5;
        }
    }
    
    if(close(kvs_authserver_sock)<0)
        {
            perror("Error closing connection");
            return -5;
        }
    return 0;
}