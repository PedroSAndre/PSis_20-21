#ifndef _key_value_struct_
#define _key_value_struct_

#define key_max_size 512
#define group_id_max_size 1024

//Structs to be used to store key_value pairs
struct key_value
{
    char key[key_max_size];
    char * value;
    struct key_value * next;
};

struct group_table
{
    char group[group_id_max_size];
    struct key_value * table;
    struct group_value * next;
};


//Returns the hash code generated for a certain string (argument key)
int hashCode_key_value(char* key);

//Returns the hash code generated for a certain string (argument key)
int hashCode_group_table(char* key);

//Returns a new inicialized table (if it fails returns NULL)
struct key_value * hashCreateInicialize_key_value();
struct group_table * hashCreateInicialize_group_table();

//Inserts or updates elements (returns 0 in sucess, -1 if failed to insert)
int hashInsert_key_value(struct key_value * table, char * key, char * value);
int hashInsert_group_table(struct group_table * table, char * group, struct key_value * key_value_table);

//Getting elements (returns element in sucess, NULL if failed)
char * hashGet_key_value(struct key_value * table, char * key);
char * hashGet_group_table(struct group_table * table, char * group);

//Deleting elements (returns 0 in sucess, -1 if failed to find, -2 if failed to insert)
int hashDelete_key_value(struct key_value * table, char * key);
int hashDelete_group_table(struct group_table * table, char * group);

#endif