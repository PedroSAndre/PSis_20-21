#include "Basic.h"
#include "Auth_group_secret.h"


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
            free(Current->group);
            free(Current->secret);
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
                    free(Current->group);
                    free(Current->secret);
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
