#include "Basic.h"
#include "app_status_struct.h"


//struct app_status functions
struct app_status * inicialize_app_status(void)
{
    //Creates an instance with the information from main
    struct app_status * dummy;
    dummy = malloc(sizeof(struct app_status));
    if(dummy == NULL)
        return NULL;
    dummy[0].process_ptid = -1;
    dummy[0].client_ptid = -1;
    dummy[0].connection_time = time(NULL); //start of the server
    dummy[0].close_time = -1;
    return dummy;
}

//Returns 0 in sucess, -1 in failure
int add_status(struct app_status * dummy, pthread_t process_ptid, int client_ptid, int * clients_connected, char * group,int*ison, char * deleting_group)
{
    if(strcmp(deleting_group,group)==0) {
        return -1;
    } 
    dummy = realloc(dummy,sizeof(struct app_status));
    if(dummy == NULL)
        return -1;
    *clients_connected = *clients_connected+1;
    dummy[*clients_connected].client_ptid = client_ptid;
    dummy[*clients_connected].process_ptid = process_ptid;
    dummy[*clients_connected].connection_time = time(NULL);
    dummy[*clients_connected].close_time = -1;
    strcpy(dummy[*clients_connected].group,group);
    dummy[*clients_connected].ison = ison;
    return 0;
}

//Returns 0 in sucess, -1 in failure
int close_status(struct app_status * dummy, pthread_t process_ptid, int client_ptid, int clients_connected)
{
    for(int i = 1;i<=clients_connected;i++)
    {
        if(dummy[i].client_ptid == client_ptid && dummy[i].process_ptid == process_ptid && dummy[i].close_time == -1)
        {
            dummy[i].close_time = time(NULL);
            return 0;
        }
    }
    return -1;
}

void print_status(struct app_status * dummy, int clients_connected)
{
    struct tm* tm_info;
    char buffer[26];
    for(int i = 1;i<=clients_connected;i++)
    {
        tm_info = localtime(&(dummy[i].connection_time));
        strftime(buffer, 26, "%Y-%m-%d %H:%M:%S", tm_info);
        printf("Client pid: %d; Connection time: %s;", dummy[i].client_ptid, buffer);
        if(dummy[i].close_time != -1)
        {
            tm_info = localtime(&(dummy[i].close_time));
            strftime(buffer, 26, "%Y-%m-%d %H:%M:%S", tm_info);
            printf(" Close time: %s;", buffer);
        }
        printf("\n");
    }
    printf("\n");
}

void send_kick_out_order(struct app_status * dummy, int clients_connected,char * group)
{
    int j[clients_connected];
    int a=0;
    int aux=1;
    for(int i = 0;i<clients_connected;i++)
    {
        if(strcmp(dummy[i].group,group)==0){
            *(dummy[i].ison)=0;
            j[a]=i;
            a++;
        }
    }
    return;

}