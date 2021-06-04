#include "Basic.h"
#include "key_value_struct.h"
#include <pthread.h>




int hashCode_key_value(char* key)
{
    int i=0;
    int index = 0;
    while(key[i]!='\0' && i<key_max_size)
    {
        index=(key[i]+index)%key_value_table_size;
        i++;
    }
    return index;
}

struct key_value * hashCreateInicialize_key_value()
{
    struct key_value * table;

    table = malloc(key_value_table_size*sizeof(struct key_value));
    if(table==NULL)
    {
        return NULL;
    }

    for(int i=0;i<key_value_table_size;i++)
    {
        table[i].key[0] = '\0';
        table[i].value = NULL;
        table[i].next = NULL;
    }

    return table;
}

int hashInsert_key_value(struct key_value * table, char * key, char * value)
{
    int aux;
    struct key_value * aux2;
    aux = hashCode_key_value(key);

    pthread_mutex_lock(&(table[aux].mutex));
    if(strcmp(table[aux].key, "\0") == 0 || strcmp(table[aux].key, key) == 0) //Same key or no element
    {
        if(strcmp(table[aux].key, key) == 0)
        {
            
            if(table[aux].signal==1 && strcmp(table[aux].value, value)!=0){
                pthread_cond_signal(&(table[aux].cond));
            }
            free(table[aux].value); //in case of overwriting
        }
        else
        {
            pthread_mutex_init(&(table[aux].mutex),NULL);
            pthread_cond_init(&(table[aux].cond),NULL);

            strcpy(table[aux].key, key);
        }
        table[aux].value = malloc(strlen(value)*sizeof(char));
        strcpy(table[aux].value, value);
    }
    else
    {
        aux2 = table + aux*sizeof(struct key_value);
        while(aux2->next != NULL && strcmp(aux2->key, key) != 0)
        {
            aux2 = aux2->next;
        }
        if(strcmp(aux2->key, key) == 0)
        {
            pthread_mutex_trylock(&(aux2->mutex));
            if(aux2->signal==1&& strcmp(table[aux].value, value)!=0){
                pthread_cond_signal(&(aux2->cond));
            }
            pthread_mutex_unlock(&(aux2->mutex));
            strcpy(aux2->value, value);

        }
        else
        {
            //Continue write
            //Stop Read
            aux2->next = malloc(sizeof(struct key_value));
            aux2 = aux2->next;
            if(aux2 == NULL)
            {
                pthread_mutex_unlock(&(table[aux].mutex));
                return -1;
            }

            aux2->next=NULL;
            strcpy(aux2->key, key);
            aux2->value = malloc(strlen(value)*sizeof(char));
            strcpy(aux2->value, value);

        }   
    }
    //Continue Read
    
    pthread_mutex_unlock(&(table[aux].mutex));
    return 0;
}

char * hashGet_key_value(struct key_value * table, char * key)
{
    int aux;
    struct key_value * aux2;
    char * aux3;
    aux = hashCode_key_value(key);
    pthread_mutex_lock(&(table[aux].mutex));
    if(strcmp(table[aux].key, key) == 0) //Same key
    {
        aux3=malloc(strlen(table[aux].value)*sizeof(char));
        strcpy(aux3,table[aux].value);
        pthread_mutex_unlock(&(table[aux].mutex));
        return aux3;
    }
    else
    {
        aux2 = &(table[aux]);
        while(aux2->next != NULL && strcmp(aux2->key, key) != 0)
        {
            aux2 = aux2->next;
        }
        if(strcmp(aux2->key, key) == 0)
        {
            aux3=malloc(strlen(aux2->value)*sizeof(char));
            strcpy(aux3,aux2->value);
            pthread_mutex_unlock(&(table[aux].mutex));
            return aux3;
        }
        else
        {
            //Continue
            pthread_mutex_unlock(&(table[aux].mutex));
            return NULL;
        }   
    }
}

int hashDelete_key_value(struct key_value * table, char * key)
{
    int aux;
    struct key_value * aux2;
    struct key_value * aux3;
    aux = hashCode_key_value(key);
    //Stop Read
    
    pthread_mutex_lock(&(table[aux].mutex));
    if(strcmp(table[aux].key, key) == 0) //Same key
    {
        if(table[aux].signal==1){ 
            pthread_cond_signal(&(table[aux].cond));
        }
        

        
        if(table[aux].next == NULL)
        {
            strcpy(table[aux].key, "\0");
            free(table[aux].value);
            table[aux].value = NULL;
            
        }
        else
        {
            aux2 = table[aux].next;
            table[aux].next = aux2->next;
            strcpy(table[aux].key, aux2->key);
            table[aux].value = aux2->value;
            free(aux2->value);
            free(aux2);
        }
    }
    else
    {//needs to find it in the linked list
        if(strcmp(table[aux].key,"\0") == 0)
            pthread_mutex_unlock(&(table[aux].mutex));
            return -1;
        aux2 = table + aux*sizeof(struct key_value);
        if(aux2->next != NULL && strcmp(aux2->key, key) != 0){
            aux3 = aux2;
            aux2 = aux2->next;
        }
        while(aux2->next != NULL && strcmp(aux2->key, key) != 0)
        {
            aux3 = aux2;
            aux2 = aux2->next;
        }
        if(strcmp(aux2->key, key) == 0)
        {
            if(aux2->signal==1){
                pthread_cond_signal(&(aux2->cond));
            }

            if(aux2->next == NULL)
            {
                free(aux2);
                aux3->next = NULL;
            }
            else
            {
                aux3 = aux2->next;
                strcpy(aux3->key, aux2->key);
                aux3->value = aux2->value;
                aux3->next = aux2->next;
                free(aux2->value);
                free(aux2);
            }
        }
        else
        {
        pthread_mutex_unlock(&(table[aux].mutex));
        return -1;
        }   
    }
    pthread_mutex_unlock(&(table[aux].mutex));
    return 0;
}

void hashFree_key_value(struct key_value * table)
{
    struct key_value * aux;
    struct key_value * aux2;
    for(int i=key_value_table_size-1;i>=0;i--)
    {
        if(table[i].next != NULL)
        {
            aux = table[i].next;
            while(aux->next != NULL)
            {
                aux2 = aux;
                aux = aux->next;
                free(aux2);
            }
            free(aux);
        }
    }
    free(table);
    return;
}



int hashWaitChange_key_value(struct key_value * table, char * key)
{
    int aux;
    struct key_value * aux2;
    aux = hashCode_key_value(key);
    if(strcmp(table[aux].key, key) == 0) //Same key
    {
        pthread_mutex_trylock(&(table[aux].mutex));
        if(aux2->signal==0){
            aux2->signal=1;
        }
        pthread_cond_wait(&(aux2->cond),&(table[aux].mutex));
        pthread_mutex_unlock(&(aux2->mutex));
        return 1;
    }
    else
    {
        aux2 = table + aux*sizeof(struct key_value);
        while(aux2->next != NULL && strcmp(aux2->key, key) != 0)
        {
            aux2 = aux2->next;
        }
        if(strcmp(aux2->key, key) == 0)
        {
            pthread_mutex_trylock(&(table->mutex));
            if(aux2->signal==0){
                aux2->signal=1;
            }
            pthread_cond_wait(&(aux2->cond),&(aux2->mutex));
            pthread_mutex_unlock(&(aux2->mutex));
            return 1;
        }
        else
        {
            return 0;
        }   
    }
}
