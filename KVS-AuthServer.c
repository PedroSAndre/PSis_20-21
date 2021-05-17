#include "Basic.h"

#define max_waiting_connections 10
#define SIZE 101

struct HashGroup {
    char * group;
    char * secret;
    struct HashGroup * next;
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
    Novo->group=group;
    Novo->secret=secret;

    Current=Table[TableIndex];
    if(Current==NULL){
        Table[TableIndex]=Novo;
        return 1;
    }

    free(Novo);

    while(Current!=NULL){

        if(strcmp(Current->group,group)==0){
            Current->secret=secret;
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
    /*int answer;
    int request;
    int kvs_authserver_sock;
    int kvs_localserver_sock;
    struct sockaddr_un kvs_authserver_sock_addr;
    struct sockaddr_un kvs_localserver_sock_addr;

    char * authserver_addr="";



    char * group_id;
    char * secret;

    group_id = malloc(1024*sizeof(char));
    secret = malloc(1024*sizeof(char));



    //Creating socket
    kvs_authserver_sock = socket(AF_INET, SOCK_DGRAM, 0);
    if(kvs_authserver_sock==-1){
        perror("Error creating client socket\n");
        return -1;
    }

    //Binding address
    memset(&kvs_authserver_sock_addr,0,sizeof(struct sockaddr_un));
    kvs_authserver_sock_addr.sun_family=AF_INET;
    strcpy(kvs_authserver_sock_addr.sun_path, authserver_addr); //adress defined in Basic.h
    if(bind(kvs_authserver_sock, &kvs_authserver_sock_addr, sizeof(kvs_authserver_sock_addr)) < 0)
    {
        perror("Error binding socket\n");
        return -2;
    }

    //Waiting for connection cycle
    kvs_localserver_sock=0;
    if(recvfrom(kvs_authserver_sock,&request,sizeof(int),0,&kvs_localserver_sock_addr,sizeof(struct sockaddr_un))<0)
    {
        perror("Error receving connection\n");
        return -3;
    }

    memset(&kvs_localserver_sock_addr,0,sizeof(struct sockaddr_un));
    if(bind(kvs_localserver_sock, &kvs_localserver_sock_addr, sizeof(kvs_localserver_sock_addr)) < 0)
    {
        perror("Error binding socket\n");
        return -2;
    }
    


    if(close(kvs_localserver_sock)<0)
    {
        perror("Error closing connection");
        return -5;
    }*/

    char group[10]="Group-A";
    char secret[10]="Secret-A";
    char secret2[10]="Secret-B";


    printf("Compare:%d\n",compareHashGroup(group,secret));
    printf("Delete:%d\n",DeleteEntry(group));
    printf("Create:%d\n",CreateUpdateEntry(group,secret));
    printf("Update:%d\n",CreateUpdateEntry(group,secret2));
    printf("Compare:%d\n",compareHashGroup(group,secret));
    printf("Compare:%d\n",compareHashGroup(group,secret2));
    printf("Delete:%d\n",DeleteEntry(group));
    return 0;

}