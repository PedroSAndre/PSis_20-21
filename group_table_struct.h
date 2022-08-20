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

//This function assigns an index to a certain key.
int hashCode_group_table(char* key);

//Returns a new inicialized table 
struct group_table * hashCreateInicialize_group_table();

//This function creates/updates an entry in the hash table for a pair group-table
int hashInsert_group_table(struct group_table * table, char * group);

//This function looks for the table of a certain group
struct key_value * hashGet_group_table(struct group_table * table, char * group);

//This function deletes an entry of the hash group-table table
int hashDelete_group_table(struct group_table * table, char * group);

//This function will dealocate the memory assigned to this group-table table
void hashFree_group_table(struct group_table * table);
#endif