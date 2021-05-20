#include "Basic.h"
#include "key_value_struct.h"


int hashCode_key_value(char* key)
{
    return 1;
    int i=0;
    int index = 0;
    while(key[i]!='\0' && i<key_max_size)
    {
        index=(key[i]+index)%key_value_table_size;
        i++;
    }
    return index;
}

int hashCode_group_table(char* key)
{
    int i=0;
    int index = 0;

    while(key[i]!='\0' && i<group_id_max_size){
        index=(key[i]+index)%n_groups_max;
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

struct group_table * hashCreateInicialize_group_table();

int hashInsert_key_value(struct key_value * table, char * key, char * value)
{
    int aux;
    struct key_value * aux2;
    aux = hashCode_key_value(key);
    if(strcmp(table[aux].key, "\0") == 0 || strcmp(table[aux].key, key) == 0) //Same key or no element
    {
        if(strcmp(table[aux].key, key) == 0)
        {
            free(table[aux].value); //in case of overwriting
        }
        else
        {
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
            strcpy(aux2->value, value);
        }
        else
        {
            aux2->next = malloc(sizeof(struct key_value));
            aux2 = aux2->next;
            if(aux2 == NULL)
            {
                return -1;
            }
            aux2->next=NULL;
            strcpy(aux2->key, key);
            aux2->value = malloc(strlen(value)*sizeof(char));
            strcpy(aux2->value, value);
        }   
    }

    return 0;
}


int hashInsert_group_table(struct group_table * table, char * group, struct key_value * key_value_table);

//Getting elements (returns element in sucess, NULL if failed)
char * hashGet_key_value(struct key_value * table, char * key)
{
    int aux;
    struct key_value * aux2;
    aux = hashCode_key_value(key);
    if(strcmp(table[aux].key, key) == 0) //Same key
    {
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
            return aux2->value;
        }
        else
        {
            return NULL;
        }   
    }
}


struct group_table * hashGet_group_table(struct group_table * table, char * group);

//Deleting elements (returns 0 in sucess, -1 if failed to find, -2 if failed to delete)
int hashDelete_key_value(struct key_value * table, char * key)
{
    int aux;
    struct key_value * aux2;
    aux = hashCode_key_value(key);
    if(strcmp(table[aux].key, key) == 0) //Same key
    {
        if(table[aux].next == NULL)
        {
            strcpy(table[aux].key, "\0");
            free(table[aux].value);
            table[aux].value = NULL;
        }
        else
        {
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
            strcpy(aux2->value, value);
        }
        else
        {
            aux2->next = malloc(sizeof(struct key_value));
            aux2 = aux2->next;
            if(aux2 == NULL)
            {
                return -1;
            }
            aux2->next=NULL;
            strcpy(aux2->key, key);
            aux2->value = malloc(strlen(value)*sizeof(char));
            strcpy(aux2->value, value);
        }   
    }

    return 0;
}
int hashDelete_group_table(struct group_table * table, char * group);