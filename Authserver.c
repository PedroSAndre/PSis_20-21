#include "Basic.h"
#include "Authserver.h"






int HashIndex(char * group){
    int i=0;
    int Index=0;
    while(group[i]!='\0' && group[i]!='\n' && i<group_id_max_size){
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
            strcpy(Novo->group,group);
            strcpy(Novo->secret,secret);
            Novo->next=NULL;
            Current->next=Novo;
            return 1;
        }
        Current=Current->next;

    }

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

char * getGroupSecret(char * group){
    int TableIndex=HashIndex(group);
    int result=0;
    struct HashGroup * Current,* Previous;
    char * noentry="\0";
    
    Current=Table[TableIndex];
    if(Current==NULL){
        perror("No entry for this group");
        return noentry;
    }
    
    while(Current!=NULL){
        if(strcmp(Current->group,group)==0){
            return Current->secret;
        }
        Current=Current->next;

    }
    perror("No entry for this group");
    return noentry;
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
            free(group);
            printf("request or group not translated\n");
            return NULL;
        }
        Current=malloc(sizeof(struct Message));
        Current->clientaddr=kvs_localserver_sock_addr;
        Current->request=request;
        Current->next=NULL;
        strcpy(Current->group,group);
        if(request==CMP){
            *Main=Current;
            secret=malloc(sizeof(char)*secret_max_size);
            if(secret==NULL){
                perror("Error alocating memory");
                return NULL;
            }
            strcpy(Current->secret,"\0");
        }
        free(group);
    }else{
        

        while((Current->clientaddr.sin_addr.s_addr!=kvs_localserver_sock_addr.sin_addr.s_addr || ntohs(kvs_localserver_sock_addr.sin_port)!=ntohs(Current->clientaddr.sin_port)) && Current->next !=NULL){
            Previous=Current;
            Current=Current->next;
        }

        if(Current->clientaddr.sin_addr.s_addr!=kvs_localserver_sock_addr.sin_addr.s_addr || ntohs(kvs_localserver_sock_addr.sin_port)!=ntohs(Current->clientaddr.sin_port)){
            group= malloc(sizeof(char)*group_id_max_size);
            aux=sscanf(buf,"%d:%s",&request,group);
            if(aux!=2){
                free(group);
                printf("request or group not translated\n");
                return NULL;
            }
            Current=malloc(sizeof(struct Message));
            if(Current==NULL){
                free(group);
                perror("Error alocating memory");
                return NULL;
            }
            Current->clientaddr=kvs_localserver_sock_addr;
            strcpy(Current->group,group);
            Current->request=request;
            if(request==CMP){
                strcpy(Current->secret,"\0");
                Previous=Current;
                Previous->next=Current;
                Current->next=NULL;
            }
            free(group);
        }else{     
            strcpy(Current->secret,buf);
        }
    }
    return Current;
}

struct Message * deleteMessage(struct Message * Current, struct Message * Main){
    struct Message * Previous;
    if(Current==Main){
        free(Current);
        return NULL;
    }
    Previous=Main;
    if(Previous==NULL){

    }
    while(Previous->next!=Current){
        Previous=Previous->next;
    }
    Previous->next=Current->next;
    free(Current);
    return Main;
}

void generate_secret(char * secret){


    for(int i=0;i<key_max_size-1;i++){
        secret[i]= 32 + rand() % nASCII;
    }
    secret[key_max_size-1]='\0';

    return NULL;

}

