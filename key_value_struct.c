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

struct group_table * hashCreateInicialize_group_table()
{
    struct group_table * table;

    table = malloc(n_groups_max*sizeof(struct group_table));
    if(table==NULL)
    {
        return NULL;
    }

    for(int i=0;i<n_groups_max;i++)
    {
        table[i].group[0] = '\0';
        table[i].key_value_table = NULL;
        table[i].next = NULL;
    }

    return table;
}

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


int hashInsert_group_table(struct group_table * table, char * group, struct key_value * key_value_table)
{
    int aux;
    struct group_table * aux2;
    aux = hashCode_group_table(group);
    if(strcmp(table[aux].group, "\0") == 0 || strcmp(table[aux].group, group) == 0) //Same key or no element
    {
        strcpy(table[aux].group, group);
        table[aux].key_value_table = key_value_table;
    }
    else
    {
        aux2 = table + aux*sizeof(struct group_table);
        while(aux2->next != NULL && strcmp(aux2->group, group) != 0)
        {
            aux2 = aux2->next;
        }
        if(strcmp(aux2->group, group) == 0) //overwriting
        {
            aux2->key_value_table = key_value_table;
        }
        else
        {
            aux2->next = malloc(sizeof(struct group_table));
            aux2 = aux2->next;
            if(aux2 == NULL)
            {
                return -1;
            }
            aux2->next=NULL;
            strcpy(aux2->group, group);
            aux2->key_value_table = key_value_table;
        }   
    }
    return 0;
}

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

struct key_value * hashGet_group_table(struct group_table * table, char * group)
{
    int aux;
    struct group_table * aux2;
    aux = hashCode_group_table(group);
    if(strcmp(table[aux].group, group) == 0) //Same key
    {
        return table[aux].key_value_table;
    }
    else
    {
        aux2 = table + aux*sizeof(struct group_table);
        while(aux2->next != NULL && strcmp(aux2->group, group) != 0)
        {
            aux2 = aux2->next;
        }
        if(strcmp(aux2->group, group) == 0)
        {
            return aux2->key_value_table;
        }
        else
        {
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
    if(strcmp(table[aux].key, key) == 0) //Same key
    {
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

int hashDelete_group_table(struct group_table * table, char * group)
{
    int aux;
    struct group_table * aux2;
    struct group_table * aux3;
    aux = hashCode_group_table(group);
    if(strcmp(table[aux].group, group) == 0) //Same key
    {
        if(table[aux].next == NULL)
        {
            strcpy(table[aux].group, "\0");
            //fazer free da tabela aqui e everywhere
            table[aux].key_value_table = NULL;
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
            strcpy(table[aux].group, aux2->group);
            table[aux].key_value_table = aux2->key_value_table;
            free(aux2);
            aux3->next = NULL;
        }
    }
    else
    {//needs to find it in the linked list
        if(strcmp(table[aux].group,"\0") == 0)
            return -1;
        aux2 = table + aux*sizeof(struct key_value);
        while(aux2->next != NULL && strcmp(aux2->group, group) != 0)
        {
            aux3 = aux2;
            aux2 = aux2->next;
        }
        if(strcmp(aux2->group, group) == 0)
        {
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
                strcpy(table->group, aux2->group);
                table->key_value_table = aux2->key_value_table;
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
int hashFree_group_table(struct key_value * table);