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
    int * ison;
};

struct app_status * inicialize_app_status(void);
int add_status(struct app_status * dummy, pthread_t process_ptid, int client_ptid, int * clients_connected);
int close_status(struct app_status * dummy, pthread_t process_ptid, int client_ptid, int clients_connected);
void print_status(struct app_status * dummy, int clients_connected);
void Kick_out_clients(struct app_status * dummy, int clients_connected,char * group);

#endif