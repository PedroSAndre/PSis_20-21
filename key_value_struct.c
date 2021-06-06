#include "Basic.h"
#include "key_value_struct.h"

//This file will store all functions related to storing key-value pairs


/*hashCode_key_value()   This function assigns an index to a certain key.
                        
                Arguments:  key   
                        
                Returns:    Index for the hash key_value table*/
int hashCode_key_value(char* key)
{
    int i=0;
    int index=0;
    while(key[i]!='\0' && i<key_max_size)
    {
        index=(key[i]+index)%key_value_table_size;
        i++;
    }
    return index;
}

/*hashCreateInicialize_key_value()  Returns a new inicialized table 
                        
                Returns:    table   - Correspond to the address of the first element of the table for the considered group. Table inicialized successfully
                            NULL    - Alocation of memory unsuccessful*/
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

/*hashInsert_key_value()   This function creates/updates an entry in the hash table for a pair key-value
                        
                Arguments:  key, value  - Values to be inserted
                            table       - Correspond to the address of the first element of the table for the considered group
                        
                Returns:    SUCCESS     - Created group
                            ERRMALLOC   - Error alocating memory*/
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
            strcpy(table[aux].key, key);
        }
        table[aux].value = malloc(strlen(value)*sizeof(char));
        if(table[aux].value == NULL)
            return ERRMALLOC;
        strcpy(table[aux].value, value);
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
            if(aux2->signal==1 && strcmp(table[aux].value, value)!=0){
                pthread_cond_signal(&(aux2->cond));
            }
        free(aux2->value);
        aux2->value = malloc(strlen(value)*sizeof(char));
        if(aux2->value == NULL)
            return ERRMALLOC;
        strcpy(aux2->value, value);
        }
        else
        {
            aux2->next = malloc(sizeof(struct key_value));
            aux2 = aux2->next;
            if(aux2 == NULL)
                return ERRMALLOC;

            aux2->next=NULL;
            strcpy(aux2->key, key);
            aux2->value = malloc(strlen(value)*sizeof(char));
            if(aux2->value == NULL)
                return ERRMALLOC;
            strcpy(aux2->value, value);
        }   
    }
    pthread_mutex_unlock(&(table[aux].mutex));
    return SUCCESS;
}

/*hashGet_key_value()   This function looks for the value of a certain key
                        
                Arguments:  key     - Key to search for value
                            table   - Correspond to the address of the first element of the table for the considered group
                        
                Returns:    aux3        - Found the value of the group
                            NULL        - Group not found*/
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
        }
        else
        {
            pthread_mutex_unlock(&(table[aux].mutex));
            return NULL;
        }   
    }
    pthread_mutex_unlock(&(table[aux].mutex));
    return aux3;
}

/*hashDelete_key_value()   This function deletes an entry of the hash key-value table
                        
                Arguments:  key         - Key to delete
                            table       - Correspond to the address of the first element of the table for the considered group
                        
                Returns:    SUCCESS     - Deleted group
                            DENIED      - key not found*/
int hashDelete_key_value(struct key_value * table, char * key)
{
    int aux;
    struct key_value * aux2;
    struct key_value * aux3;
    aux = hashCode_key_value(key);
    
    pthread_mutex_lock(&(table[aux].mutex));
    if(strcmp(table[aux].key, key) == 0) //Same key
    {
        if(table[aux].signal==1)
            pthread_cond_signal(&(table[aux].cond));

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
        {
            pthread_mutex_unlock(&(table[aux].mutex));
            return ERRRD;
        }
        aux2 = &(table[aux]);
        while(aux2->next != NULL && strcmp(aux2->key, key) != 0)
        {
            aux3 = aux2;
            aux2 = aux2->next;
        }
        if(strcmp(aux2->key, key) == 0)
        {
            if(aux2->signal==1)
                pthread_cond_signal(&(aux2->cond));

            if(aux2->next == NULL)
            {
                free(aux2);
                aux3->next = NULL;
            }
            else
            {
                aux3->next = aux2->next;
                free(aux2->value);
                free(aux2);
            }
        }
        else
        {
        pthread_mutex_unlock(&(table[aux].mutex));
        return ERRRD;
        }   
    }
    pthread_mutex_unlock(&(table[aux].mutex));
    return SUCCESS;
}


/*hashFree_key_value()   This function will dealocate the memory assigned to this key_value table
                        
                Arguments:  table - Correspond to the address of the first element of the table for the considered group*/
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


/*hashWaitChange_key_value()   This function will wait for a condition variable associated to the key value is signaled, when key is changed
                        
                Arguments:  key         - Key to wait for change
                            table       - Correspond to the address of the first element of the table for the considered group
                        
                Returns:    SUCCESS     - Waited for signal
                            ERRRD       - key not found*/
int hashWaitChange_key_value(struct key_value * table, char * key)
{
    int aux;
    struct key_value * aux2;
    aux = hashCode_key_value(key);
    pthread_mutex_lock(&(table[aux].mutex));
    if(strcmp(table[aux].key, key) == 0) //Same key
    {  
        if(table[aux].signal==0){
            table[aux].signal=1;
        }
        pthread_cond_wait(&(table[aux].cond),&(table[aux].mutex));
        pthread_mutex_unlock(&(table[aux].mutex));
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
            if(aux2->signal==0){
                aux2->signal=1;
            }
            pthread_cond_wait(&(aux2->cond),&(table[aux].mutex));
            pthread_mutex_unlock(&(table[aux].mutex));
        }
        else
        {
            pthread_mutex_unlock(&(table[aux].mutex));
            return ERRRD;
        }
    }
    return SUCCESS;
}

/*signal_all_callback()   When deleting a table (deleting a group), it is necessary to signal all callbacks for them to not get stuck while the server is waiting
                        
                Arguments:  table       - Correspond to the address of the first element of the table for the considered group*/
void signal_all_callback(struct key_value * table)
{
    struct key_value * aux;
    if(table==NULL)
    {
        return;
    }
    for(int i=0;i<key_value_table_size;i++)
    {
        if(strcmp(table[i].key, "\0") != 0)
        {
            pthread_mutex_lock(&(table[i].mutex));
            if(table[i].signal==1)
            {
                pthread_cond_signal(&(table[i].cond));
            }
            aux = &(table[i]);
            while(aux->next != NULL)
            {
                aux = aux->next;
                if(aux->signal==1){
                    pthread_cond_signal(&(aux->cond));
                }
            }
            pthread_mutex_unlock(&(table[i].mutex));
        }

    }
    
}
