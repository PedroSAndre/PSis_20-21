#ifndef _KVS_TEMP_
#define _KVS_TEMP_
#include <stdio.h>

int establish_connection (char * group_id, char * secret);
int put_value(char * key, char * value);
int get_value(char * key, char ** value);
int delete_value(char * key);
int register_callback(char * key, void (*callback_function)(char *));
int close_connection();


#endif