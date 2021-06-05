#ifndef _group_table_struct_
#define _group_table_struct_

#define group_id_max_size 1024 //copied from basic.h

//Struct to be used to store group_table pairs

struct group_table
{
    char group[group_id_max_size];
    struct key_value * key_value_table;
    struct group_table * next;
};

//Returns the hash code generated for a certain string (argument key)
int hashCode_group_table(char* key);

//Returns a new inicialized table (if it fails returns NULL)
struct group_table * hashCreateInicialize_group_table();

//Inserts or updates elements (returns 0 in sucess, -1 if failed to insert)
int hashInsert_group_table(struct group_table * table, char * group);

//Getting elements (returns element in sucess, NULL if failed)
struct key_value * hashGet_group_table(struct group_table * table, char * group);

//Deleting elements (returns 0 in sucess, -1 if failed to find)
int hashDelete_group_table(struct group_table * table, char * group);

//Frees the memory on the end of execution (returns 0 in sucess, -1 if failed to find)
void hashFree_group_table(struct group_table * table);
#endif