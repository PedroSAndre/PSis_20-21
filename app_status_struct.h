#ifndef _app_status_struct_
#define _app_status_struct_

#include <time.h> //included because of the struct
#include <pthread.h> //included because of the struct

struct app_status
{
    pthread_t process_ptid;
    int client_ptid;
    time_t connection_time;
    time_t close_time;
    char group[group_id_max_size];
};

//Returns a new inicialized dynamic vector
struct app_status * inicialize_app_status(void);

//This function adds an entry in the dynamic vector for the new client that connected
int add_status(struct app_status ** dummy, pthread_t process_ptid, int client_ptid, int * clients_connected, char * group, char * deleting_group);

//This function closes a connection on the dynamic vector
int close_status(struct app_status * dummy, pthread_t process_ptid, int client_ptid, int clients_connected);

//This function prints the information about all connections (past and present)
void print_status(struct app_status * dummy, int clients_connected);

//Waits for the connections of a certain group to disconnect
void wait_to_group_clients_to_disconect(struct app_status * dummy, int clients_connected, char * group);

#endif