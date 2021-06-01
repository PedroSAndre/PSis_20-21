#include "Basic.h"
#include "key_value_struct.h"
#include "group_table_struct.h"

struct app_status
{
    pthread_t process_ptid;
    int client_ptid;
    time_t connection_time;
    time_t close_time;
};

struct message{
    int request;
    char * group;
    char * secret;
    struct message * next;
};

//Declaration of functions writen in the end
int createAndBindServerSocket(int * localserver_sock, struct sockaddr_un * localserver_sock_addr);
void acceptConnections(void *arg);
void handleConnection(void *arg);
void handleAuthCom(void *arg);
struct app_status * inicialize_app_status(void);
int add_status(struct app_status * dummy, pthread_t process_ptid, int client_ptid);
int close_status(struct app_status * dummy, pthread_t process_ptid, int client_ptid);
void print_status(struct app_status * dummy);

//Global variables (to be shared across all server threads)
int server_status = 1; //1 -> ON; 0 -> OFF 
int clients_connected;
struct group_table * groups; //hash table with all groups
struct app_status * state; //struct with all clients and their time information
int auth_socket;
struct message * Main=NULL;

int main(void)
{
    int cycle_status = 1;
    int selector;
    int aux = 0;
    char input_string[input_string_max_size];
    char input_string2[input_string_max_size];
    pthread_t acepting_connections_thread_ptid;
    struct key_value * aux_key_value;

    state = inicialize_app_status();
    if(state == NULL)
    {
        printf("Error inicializing app_status");
        return -1;
    }
    clients_connected = 0;

    groups = hashCreateInicialize_group_table();

    if(groups == NULL)
    {
        printf("Error inicializing group_table");
        return -1;
    }

    if(pthread_create(&acepting_connections_thread_ptid,NULL,(void *)&acceptConnections,NULL)<0)
    {
        perror("Error creating thread");
        return -1;
    }

    printf("*****Welcome to KVS Local Server*****\n");

    //Main control cycle
    while(cycle_status)
    {
        selector = 5; //To make sure the server does nothing in case of a bad read
        printf("Select the desired option:\n0) Shutdown server\n1) Create a group\n2) Delete a group\n3) Show group info\n4) Show app status\n\n");
        fgets(input_string, input_string_max_size, stdin);
        if(sscanf(input_string, "%d", &selector) < 1)
        {
            printf("The option you inserted is not valid\nPlease enter a positive integer between 0-4\n\n");
            selector = 5;
        }
        if(selector==0)
        {
            server_status=0;
            cycle_status=0;
        }
        else if(selector==1)
        {
            printf("Insert the new group ID: ");
            fgets(input_string, group_id_max_size, stdin);
            printf("Insert the secret: ");
            fgets(input_string2, secret_max_size, stdin);

            //Send key to auth-server

            aux_key_value = hashCreateInicialize_key_value();
            if(aux_key_value != NULL){
                if(hashInsert_group_table(groups, input_string, aux_key_value) == 0){
                    printf("Group created with sucess\n\n");}
                else{
                    printf("Error creating group\n\n");}}
            else{
                printf("Error creating group\n\n");}
        }
        else if(selector==2)
        {
            // Delete also on auth-server
            printf("Insert the group ID to delete: ");
            fgets(input_string, group_id_max_size, stdin);
            if(hashDelete_group_table(groups, input_string) == 0){
                printf("Group deleted with sucess\n\n");}
            else{
                printf("Error deleting group\n\n");}
        }
        else if(selector == 3)
        {
            //easy to implment
        }
        else if(selector == 4)
        {
            if(clients_connected>0)
            {
                print_status(state);
            }
            else
            {
                printf("No clients connected yet\n\n");
            }
        }
    }

    pthread_join(acepting_connections_thread_ptid,NULL);

    printf("Server terminated sucessfully\n");

    return 0;
}

//Functions used to simplify code
int createAndBindServerSocket(int * localserver_sock, struct sockaddr_un * localserver_sock_addr)
{
    //To make sure the address is free
    remove(server_addr);
    //Creating socket
    *localserver_sock = socket(AF_UNIX, SOCK_STREAM, 0);
    if(*localserver_sock==-1){
        perror("Error creating main local server socket\n");
        return -1;
    }

    //Binding address
    memset(localserver_sock_addr,0,sizeof(struct sockaddr_un));
    localserver_sock_addr->sun_family=AF_UNIX;
    strcpy(localserver_sock_addr->sun_path, server_addr); //address defined in Basic.h
    if(bind(*localserver_sock, localserver_sock_addr, sizeof(*localserver_sock_addr)) < 0)
    {
        perror("Error binding socket\n");
        return -1;
    }

    return 0;
}


int accept_connection_timeout(int * socket_af_stream)
{
    struct timeval tmout;
    fd_set rfds;
    FD_ZERO(&rfds);
    FD_SET(*socket_af_stream, &rfds);
    int client_sock=-1;

    tmout.tv_sec = (long)timeout;
    tmout.tv_usec = 0;

    if(select(*socket_af_stream+1, &rfds, (fd_set *) 0, (fd_set *) 0, &tmout)>0)
        client_sock = accept(*socket_af_stream,NULL,NULL);
    
    if(client_sock<0)
    {
        return -1;
    }
    return client_sock;
}

//struct app_status functions

struct app_status * inicialize_app_status(void)
{
    //Creates an instance with the information from main
    struct app_status * dummy;
    dummy = malloc(sizeof(struct app_status));
    if(dummy == NULL)
        return NULL;
    dummy[0].process_ptid = getpid();
    dummy[0].client_ptid = -1;
    dummy[0].connection_time = time(NULL);
    dummy[0].close_time = -1;
    return dummy;
}

int add_status(struct app_status * dummy, pthread_t process_ptid, int client_ptid)
{
    dummy = realloc(dummy,(clients_connected+2)*sizeof(struct app_status));
    if(dummy == NULL)
        return -1;
    clients_connected = clients_connected+1;
    dummy[clients_connected].client_ptid = client_ptid;
    dummy[clients_connected].process_ptid = process_ptid;
    dummy[clients_connected].close_time = -1;
    return 0;
}

int close_status(struct app_status * dummy, pthread_t process_ptid, int client_ptid)
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

void print_status(struct app_status * dummy)
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

//Thread functions
void acceptConnections(void *arg)
{
    int aux;
    int kvs_localserver_sock;
    int client_sock;
    struct sockaddr_un kvs_localserver_sock_addr;
    pthread_t temp_PID;

    aux = createAndBindServerSocket(&kvs_localserver_sock, &kvs_localserver_sock_addr);
    if(aux<0)
    {
        pthread_exit(NULL);
    }

    
    client_sock=0;
    if(listen(kvs_localserver_sock,max_waiting_connections)<0)
    {
        perror("Error listening for connections\n");
        pthread_exit(NULL);
    }

    //Connection cycle
    while(server_status)
    {
        client_sock = accept_connection_timeout(&(kvs_localserver_sock));
        if(client_sock != -1)
        {
            if(pthread_create(&temp_PID,NULL,(void *)&handleConnection,(void *)&client_sock)<0)
            {
                perror("Error creating thread");
                pthread_exit(NULL);
            }
        }
    }

    //pthread_join(ptid,NULL);
    pthread_exit(NULL);
}

void handleConnection(void *arg)
{
    int client_sock;
    int answer;
    int client_PID;
    pthread_t local_PID;

    char * group_id;
    char * secret;
    group_id = malloc(1024*sizeof(char));
    secret = malloc(1024*sizeof(char));

    client_sock = *((int *)arg);

    local_PID = pthread_self();

    //Ask authentication

    read(client_sock,&client_PID,sizeof(client_PID));
    read(client_sock,group_id,(1024*sizeof(char)));
    read(client_sock,secret,(1024*sizeof(char)));
    printf("Client_PID: %d\n", client_PID);
    printf("Group_ID: %s\n", group_id);
    printf("Secret: %s\n", secret);

    add_status(state, local_PID, client_PID);

    answer=1;
    write(client_sock,&answer,sizeof(answer));
    read(client_sock,&answer,sizeof(answer));
    if(close(client_sock)<0)
    {
        perror("Error closing connection");
        pthread_exit((void *)-5);
    }

    pthread_exit((void *)0);
}


//Functions used to simplify code
int createAndBindAuthSocket(int * localserver_sock, struct sockaddr_un * localserver_sock_addr)
{
    //To make sure the address is free
    remove(server_addr);
    //Creating socket
    *localserver_sock = socket(AF_UNIX, SOCK_STREAM, 0);
    if(*localserver_sock==-1){
        perror("Error creating main local server socket\n");
        return -1;
    }

    //Binding address
    memset(localserver_sock_addr,0,sizeof(struct sockaddr_un));
    localserver_sock_addr->sun_family=AF_UNIX;
    strcpy(localserver_sock_addr->sun_path, server_addr); //address defined in Basic.h
    if(bind(*localserver_sock, localserver_sock_addr, sizeof(*localserver_sock_addr)) < 0)
    {
        perror("Error binding socket\n");
        return -1;
    }

    return 0;
}

void handleAuthCom(void *arg){


    

}