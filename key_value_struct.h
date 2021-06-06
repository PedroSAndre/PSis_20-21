#ifndef _key_value_struct_
#define _key_value_struct_

#define key_max_size 512 //copied from basic.h


//Struct to be used to store key_value pairs
struct key_value
{
    char key[key_max_size];
    char * value;
    int signal;
    pthread_cond_t cond; //condition variable to make callaback of a key possible
    pthread_mutex_t mutex;
    struct key_value * next;
};


//This function assigns an index to a certain key.
int hashCode_key_value(char* key);

//Returns a new inicialized table (if it fails returns NULL)
struct key_value * hashCreateInicialize_key_value();

//This function creates/updates an entry in the hash table for a pair key-value
int hashInsert_key_value(struct key_value * table, char * key, char * value);

//This function looks for the value of a certain key
char * hashGet_key_value(struct key_value * table, char * key);

//This function deletes an entry of the hash key-value table
int hashDelete_key_value(struct key_value * table, char * key);

//This function will dealocate the memory assigned to this key_value table
void hashFree_key_value(struct key_value * table);

//This function will wait for a condition variable associated to the key value is signaled, when key is changed
int hashWaitChange_key_value(struct key_value * table, char * key);

//signal_all_callback()   When deleting a table (deleting a group), it is necessary to signal all callbacks for them to not get stuck while the server is waiting
void signal_all_callback(struct key_value * table);
#endif