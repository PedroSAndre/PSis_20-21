#include "Basic.h"
#include "group_table_struct.h"
#include "key_value_struct.h"

int hashCode_group_table(char* key)
{
    int i=0;
    int index = 0;

    while(key[i]!='\0'&& key[i]!='\n' && i<group_id_max_size){
        index=(key[i]+index)%n_groups_max;
        i++;
    }
    return index;
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


int hashInsert_group_table(struct group_table * table, char * group)
{
    int aux;
    struct group_table * aux2;
    aux = hashCode_group_table(group);
    if(strcmp(table[aux].group, "\0") == 0 || strcmp(table[aux].group, group) == 0) //Same key or no element
    {
        if(strcmp(table[aux].group, group) == 0)
            hashFree_key_value(table[aux].key_value_table);
        table[aux].key_value_table = hashCreateInicialize_key_value();
        if(table[aux].key_value_table == NULL)
            return ERRMALLOC;
        strcpy(table[aux].group, group);
    }
    else
    {
        aux2 = &(table[aux]);
        while(aux2->next != NULL && strcmp(aux2->group, group) != 0)
        {
            aux2 = aux2->next;
        }
        if(strcmp(aux2->group, group) == 0) //overwriting
        {
            hashFree_key_value(aux2->key_value_table);
            aux2->key_value_table = hashCreateInicialize_key_value();
            if(aux2->key_value_table == NULL)
                return ERRMALLOC;
        }
        else
        {
            aux2->next = malloc(sizeof(struct group_table));
            aux2 = aux2->next;
            if(aux2 == NULL)
                return ERRMALLOC;
            aux2->next=NULL;
            aux2->key_value_table = hashCreateInicialize_key_value();
            if(aux2->key_value_table == NULL)
                return ERRMALLOC;
            strcpy(aux2->group, group);
        }   
    }
    return SUCCESS;
}


struct key_value * hashGet_group_table(struct group_table * table, char * group)
{
    int aux;
    struct group_table * aux2;
    aux = hashCode_group_table(group);
    if(strcmp(table[aux].group, group) == 0) //Same group
    {
        return table[aux].key_value_table;
    }
    else
    {
        aux2 = &(table[aux]);
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

int hashDelete_group_table(struct group_table * table, char * group)
{
    int aux;
    struct group_table * aux2;
    struct group_table * aux3;
    aux = hashCode_group_table(group);
    if(strcmp(table[aux].group, group) == 0) //Same group
    {
        if(table[aux].next == NULL)
        {
            strcpy(table[aux].group, "\0");
            hashFree_key_value(table[aux].key_value_table);
            table[aux].key_value_table = NULL;
        }
        else
        {
            aux2 = table[aux].next;
            hashFree_key_value(table[aux].key_value_table);
            strcpy(table[aux].group, aux2->group);
            table[aux].key_value_table = aux2->key_value_table;
            table[aux].next = aux2->next;
            free(aux2);
        }
    }
    else
    {//needs to find it in the linked list
        if(strcmp(table[aux].group,"\0") == 0)
            return ERRRD;
        aux2 = &(table[aux]);
        while(aux2->next != NULL && strcmp(aux2->group, group) != 0)
        {
            aux3 = aux2;
            aux2 = aux2->next;
        }
        if(strcmp(aux2->group, group) == 0)
        {
            if(aux2->next == NULL)
            {
                hashFree_key_value(aux2->key_value_table);
                free(aux2);
                aux3->next = NULL;
            }
            else
            {
                aux3->next = aux2->next;
                hashFree_key_value(aux2->key_value_table);
                free(aux2);
            }
        }
        else
        {
            return ERRRD;
        }   
    }
    return SUCCESS;
}

void hashFree_group_table(struct group_table * table)
{
    struct group_table * aux;
    struct group_table * aux2;
    for(int i=n_groups_max-1;i>=0;i--)
    {
        if(table[i].next != NULL)
        {
            aux = table[i].next;
            while(aux->next != NULL)
            {
                aux2 = aux;
                aux = aux->next;
                hashFree_key_value(aux2->key_value_table);
                free(aux2);
            }
            hashFree_key_value(aux->key_value_table);
            free(aux);
        }
        if(table[i].key_value_table != NULL)
            hashFree_key_value(table[i].key_value_table);
    }
    free(table);
    return;
}