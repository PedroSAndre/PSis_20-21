#ifndef _key_value_struct_
#define _key_value_struct_

#define key_max_size 512 //copied from basic.h


//Structs to be used to store key_value pairs and group_table pairs
struct key_value
{
    char key[key_max_size];
    char * value;
    int signal;
    pthread_cond_t cond;
    pthread_mutex_t mutex;
    struct key_value * next;
};


//Returns the hash code generated for a certain string (argument key)
int hashCode_key_value(char* key);

//Returns a new inicialized table (if it fails returns NULL)
struct key_value * hashCreateInicialize_key_value();

//Inserts or updates elements (returns 0 in sucess, -1 if failed to insert)
int hashInsert_key_value(struct key_value * table, char * key, char * value);

//Getting elements (returns element in sucess, NULL if failed)
char * hashGet_key_value(struct key_value * table, char * key);

//Deleting elements (returns 0 in sucess, -1 if failed to find)
int hashDelete_key_value(struct key_value * table, char * key);

//Frees the memory on the end of execution (returns 0 in sucess, -1 if failed to find)
void hashFree_key_value(struct key_value * table);

//This function will wait for a condition variable associated to the key value is signaled, when key is changed
int hashWaitChange_key_value(struct key_value * table, char * key);

//When deleting a table (deleting a group), it was necessary to signal all callbacks for them to not get stuck while the server is waiting
void signal_all_callback(struct key_value * table);
#endif