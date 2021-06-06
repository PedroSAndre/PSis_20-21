#include "Basic.h"
#include "Authserver.h"


/*HashIndex()   This function assigns an index to a certain group.
                        
                Arguments:  group   
                        
                Returns:    Index for the hash table*/
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
            return ERRMALLOC;
        }
        strcpy(Novo->group,group);
        strcpy(Novo->secret,secret);
        Novo->next=NULL;
        Table[TableIndex]=Novo;
        return SUCCESS;
    }

    while(Current!=NULL){

        if(strcmp(Current->group,group)==0){
            strcpy(secret,Current->secret);
            return SUCCESS;
        }
        if(Current->next==NULL){
            Novo=malloc(sizeof(struct HashGroup));
            if (Novo ==NULL){
                perror("Error alocating memory");
                return ERRMALLOC;
            }
            strcpy(Novo->group,group);
            strcpy(Novo->secret,secret);
            Novo->next=NULL;
            Current->next=Novo;
            return SUCCESS;
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
        return DENIED;
    }

    if(strcmp(Current->group,group)==0){
        Table[TableIndex]=Current->next;
        free(Current);
        return SUCCESS;
    }
    Previous=Current;
    Current=Current->next;

    while(Current!=NULL){

        if(strcmp(Current->group,group)==0){
            Previous->next=Current->next;
            free(Current);
            return SUCCESS; 
        }
        Previous=Current;
        Current=Current->next;
    }
    perror("Entry to delete not found");
    return DENIED;
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
            return SUCCESS;
        }else{
            return DENIED;
        }
    }else{
        return ERRRD;
    }
    
}

struct Message * recoverClientMessage(char * buf,struct sockaddr_in kvs_localserver_sock_addr,struct Message ** Main, int * err){
    int request=0,aux;
    char * group;
    char * secret;
    struct Message * Current, * Previous;


    Previous=NULL;
    Current=*Main;



    if(Current==NULL)
    {
        group=malloc(sizeof(char)*group_id_max_size);
        if(group==NULL){
            *err=ERRMALLOC;
            return NULL;
        }
        aux=sscanf(buf,"%d:%s",&request,group);
        if(aux!=2){
            free(group);
            printf("request or group not translated\n");
            return NULL;
        }
        Current=malloc(sizeof(struct Message));
        if(Current==NULL){
            free(group);
            *err=ERRMALLOC;
            return NULL;
        }
        Current->clientaddr=kvs_localserver_sock_addr;
        Current->request=request;
        Current->next=NULL;
        strcpy(Current->group,group);
        if(request==CMP){
            *Main=Current;
            secret=malloc(sizeof(char)*secret_max_size);
            if(secret==NULL){
                perror("Error alocating memory");
                free(group);
                free(Current);
                *err=ERRMALLOC;
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
            if(group==NULL){
                *err=ERRMALLOC;
                return NULL;
            }
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
                *err=ERRMALLOC;
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
        Main=Current->next;
        free(Current);
        return NULL;
    }
    Previous=Main;
    if(Previous==NULL){
        return NULL;
    }
    while(Previous->next!=Current || Previous->next ==NULL){
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

    return;

}

void delete_All_messages(struct Message * Main){
    struct Message * Current;
    while(Main!=NULL){
        Main=deleteMessage(Main,Main);
    }
}

void delete_All_Entries(){
    struct HashGroup * Current;
    for(int i=0;i<SIZE;i++){
        while(Table[i]!=NULL){
            Current=Table[i];
            DeleteEntry(Current->group);
        }
    }
}

int recvfrom_timeout(int * socket_af_stream, void * to_read, int size_to_read,struct sockaddr * server_sock_addr, socklen_t * len)
{
    struct timeval tmout;
    fd_set rfds;
    FD_ZERO(&rfds);
    FD_SET(*socket_af_stream, &rfds);
    
    tmout.tv_sec = (long)timeout;
    tmout.tv_usec = 0;

    if(select(*socket_af_stream+1, &rfds, (fd_set *) 0, (fd_set *) 0, &tmout)>0)
    {
        recvfrom(*socket_af_stream,to_read,size_to_read,0,server_sock_addr,len);
    }
    else
    {
        return ERRTIMEOUT;
    }
    return SUCCESS; 
}

