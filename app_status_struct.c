#include "Basic.h"
#include "app_status_struct.h"


/*inicialize_app_status()  Returns a new inicialized dynamic vector
                        
                Returns:    dummy   - Correspond to the address of the first element of the vector.
                            NULL    - Alocation of memory unsuccessful*/
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

/*add_status()   This function adds an entry in the dynamic vector for the new client that connected
                        
                Arguments:  process_ptid, client_ptid, group    - Values to be inserted
                            clients_connected                   - Number of clients connected
                            deleting_group                      - To check if the group is being deleted
                            dummy                               - Corresponds to the address of the first element of the dynamic vector
                        
                Returns:    SUCCESS     - Added client
                            ERRMALLOC   - Error alocating memory
                            DENIED      - Cannot connect client because the group will be deleted*/
int add_status(struct app_status ** dummy, pthread_t process_ptid, int client_ptid, int * clients_connected, char * group, char * deleting_group)
{
    struct app_status *aux;
    if(strcmp(deleting_group,group)==0) {
        return DENIED;
    } 
    aux = realloc(*dummy,(*(clients_connected)+2)*sizeof(struct app_status));
    if(aux == NULL)
        return ERRMALLOC;
    *clients_connected = *clients_connected+1;
    aux[*clients_connected].client_ptid = client_ptid;
    aux[*clients_connected].process_ptid = process_ptid;
    aux[*clients_connected].connection_time = time(NULL);
    aux[*clients_connected].close_time = -1;
    strcpy(aux[*clients_connected].group,group);
    *dummy=aux;
    return SUCCESS;
}

/*close_status()   This function closes a connection on the dynamic vector
                        
                Arguments:  process_ptid, client_ptid       - Information of the client to close
                            clients_connected               - Number of clients connected
                            dummy                           - Correspond to the address of the first element of the dynamic vector
                        
                Returns:    SUCCESS     - Closed connection
                            ERRRD       - Connection not found*/
int close_status(struct app_status * dummy, pthread_t process_ptid, int client_ptid, int clients_connected)
{
    for(int i = 1;i<=clients_connected;i++)
    {
        if(dummy[i].client_ptid == client_ptid && dummy[i].process_ptid == process_ptid && dummy[i].close_time == -1)
        {
            dummy[i].close_time = time(NULL);
            return SUCCESS;
        }
    }
    return ERRRD;
}

/*print_status()   This function prints the information about all connections (past and present)
                        
                Arguments:  clients_connected               - Number of clients connected
                            dummy                           - Correspond to the address of the first element of the dynamic vector*/
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
    return;
}

/*wait_to_group_clients_to_disconect()   Waits for the connections of a certain group to disconnect
                        
                Arguments:  clients_connected               - Number of clients connected
                            dummy                           - Correspond to the address of the first element of the dynamic vector
                            group                           - Group to wait for*/
void wait_to_group_clients_to_disconect(struct app_status * dummy, int clients_connected, char * group)
{
    int aux=1;
    while(aux)
    {
        aux=0;
        for(int i=1;i<=clients_connected;i++)
        {
            if(strcmp(dummy[i].group,group)==0)
            {
                if(dummy[i].close_time==-1)
                {
                    aux=1;
                }
            }
        }
    }
}