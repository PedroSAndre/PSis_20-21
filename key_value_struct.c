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

    //Stop Write
    if(strcmp(table[aux].key, "\0") == 0 || strcmp(table[aux].key, key) == 0) //Same key or no element
    {
        if(strcmp(table[aux].key, key) == 0)
        {
            if(table[aux].signal==1 && strcmp(table[aux].value, value)!=0){
                pthread_mutex_trylock(&(table[aux].mutex));
                pthread_cond_signal(&(table[aux].cond));
                pthread_mutex_unlock(&(table[aux].mutex));
            }
            //Continue Write
            //Stop Read
            free(table[aux].value); //in case of overwriting
        }
        else
        {
            //Continue Write
            //Stop Read
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
            if(aux2->signal==1&& strcmp(table[aux].value, value)!=0){
                pthread_mutex_trylock(&(aux2->mutex));
                pthread_cond_signal(&(aux2->cond));
                pthread_mutex_unlock(&(aux2->mutex));
            }
            //Continue write
            //Stop Read
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
                return -1;
            }

            pthread_mutex_init(&(table[aux].mutex),NULL);
            pthread_cond_init(&(table[aux].cond),NULL);

            aux2->next=NULL;
            strcpy(aux2->key, key);
            aux2->value = malloc(strlen(value)*sizeof(char));
            strcpy(aux2->value, value);

        }   
    }
    //Continue Read
    

    return 0;
}

char * hashGet_key_value(struct key_value * table, char * key)
{
    int aux;
    struct key_value * aux2;
    aux = hashCode_key_value(key);
    //Stop Writing
    if(strcmp(table[aux].key, key) == 0) //Same key
    {
        //Continue 
        ///////////////////////////////NOT GOOD/////////////////////////////
        return table[aux].value;
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
            //Continue
            ///////////////////////////////NOT GOOD/////////////////////////////
            return aux2->value;
        }
        else
        {
            //Continue
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
    
    if(strcmp(table[aux].key, key) == 0) //Same key
    {
        if(table[aux].signal==1){
            pthread_mutex_trylock(&(table[aux].mutex));
            pthread_cond_signal(&(table[aux].cond));
            pthread_mutex_unlock(&(table[aux].mutex));
        }

        
        if(table[aux].next == NULL)
        {
            strcpy(table[aux].key, "\0");
            free(table[aux].value);
            table[aux].value = NULL;
        }
        else
        {//puts the last element first
            aux2 = table[aux].next;
            aux3 = &(table[aux]);
            while(aux2->next != NULL)
            {
                aux3 = aux2;
                aux2 = aux2->next;
            }
            strcpy(table[aux].key, aux2->key);
            strcpy(table[aux].value, aux2->value);
            free(aux2);
            aux3->next = NULL;
        }
    }
    else
    {//needs to find it in the linked list
        if(strcmp(table[aux].key,"\0") == 0)
            return -1;
        aux2 = table + aux*sizeof(struct key_value);
        while(aux2->next != NULL && strcmp(aux2->key, key) != 0)
        {
            aux3 = aux2;
            aux2 = aux2->next;
        }
        if(strcmp(aux2->key, key) == 0)
        {
            if(aux2->signal==1){
                pthread_mutex_trylock(&(table[aux]).mutex);
                pthread_cond_signal(&(table[aux]).cond);
                pthread_mutex_unlock(&(table[aux]).mutex);
            }

            if(aux2->next == NULL)
            {
                free(aux2);
                aux3->next = NULL;
            }
            else
            {
                table = aux2; //reusing provided variable
                aux2 = aux2->next;
                aux3 = table;
                while(aux2->next != NULL)
                {
                    aux3 = aux2;
                    aux2 = aux2->next;
                }
                strcpy(table->key, aux2->key);
                strcpy(table->value, aux2->value);
                free(aux2);
                aux3->next = NULL;
            }
        }
        else
        {
            return -1;
        }   
    }
    return 0;
}

int hashFree_key_value(struct key_value * table);



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
