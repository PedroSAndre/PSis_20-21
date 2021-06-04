#include "Basic.h"
#include "key_value_struct.h"
#include "group_table_struct.h"
#include "app_status_struct.h"
#include "Localserver_aux.h"

//Thread functions
void acceptConnections(void *arg);
void handleConnection(void *arg);
void handleAuthCom(void *arg);


//Global variables (to be shared across all server threads)
int server_status = 1; //1 -> ON; 0 -> OFF 
int all_clients_connected = 0;

struct group_table * groups; //hash table with all groups
struct app_status * state; //struct with all clients and their time information

int Authserver_sock;
struct sockaddr_in Authserver_sock_addr;


int main(int argc, char ** argv)
{
    unsigned short port;
    socklen_t len = sizeof (struct sockaddr_in);
    char * port_str;
    char * authaddr_str;
    char * secret = malloc(secret_max_size*sizeof(char));


    int selector;
    int aux = 0;
    char input_string[input_string_max_size];
    pthread_t acepting_connections_thread_ptid;

    if(argc !=3){
        perror("Incorrect number of arguments");
        return 0;
    }


    CreateAuthServerSock(argv[2],argv[1],&Authserver_sock,&Authserver_sock_addr);

    printf("Sucessfully connected\n");
    printf("IP address is: %s\n", inet_ntoa(Authserver_sock_addr.sin_addr));
    printf("Port is: %d\n", (int) ntohs(Authserver_sock_addr.sin_port));

    state = inicialize_app_status();
    if(state == NULL)
    {
        printf("Error inicializing app_status");
        return -1;
    }

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
    while(server_status)
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
        }
        else if(selector==1)
        {
            printf("Insert the new group ID: ");
            fgets(input_string, group_id_max_size, stdin);
            input_string[strlen(input_string)-1]='\0';

            if(AuthServerCom(PUT,input_string,secret,Authserver_sock,Authserver_sock_addr)==0){
                printf("No response from Auth server\n");
            }else{
                printf("Group %s Created\n",input_string);

                printf("Secret of group %s%s\n",input_string,secret);
                //CONFLICT HERE
                if(hashInsert_group_table(groups, input_string) == 0)
                    printf("Group created with sucess\n\n");
                else
                    printf("Error creating selected group\n\n");
            }
        }
        else if(selector==2)
        {
            printf("Delete the group with ID: ");
            fgets(input_string, group_id_max_size, stdin);
            input_string[strlen(input_string)-1]='\0';
            aux=AuthServerCom(DEL,input_string,secret,Authserver_sock,Authserver_sock_addr);
            if(aux==0 ){
                printf("No response from Auth server\n");
            }else if(aux==1){
                    //CONFLICT HERE
                    kick_out_clients(state,all_clients_connected,input_string);
                    if(hashDelete_group_table(groups, input_string) == 0)
                        printf("Group deleted with sucess\n\n");
                    else
                        printf("Error deleting selected group\n\n");
                
            }else{
                printf("Something went wrong in the Auth Server\n");
            }
        }
        else if(selector == 3)
        {
            printf("Get info of group ID: ");
            fgets(input_string, group_id_max_size, stdin);
            aux=AuthServerCom(GET,input_string,secret,Authserver_sock,Authserver_sock_addr);
            if(aux==0){
                printf("No response from Auth server\n");
            }else if(aux==1){
                
                
            }else{
                printf("Something went wrong in the Auth Server\n");
            }
        }
        else if(selector == 4)
        {
            if(all_clients_connected>0)
            {
                print_status(state,all_clients_connected);
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






//Thread functions
void acceptConnections(void *arg)
{
    int kvs_localserver_sock;
    int client_sock=0;
    struct sockaddr_un kvs_localserver_sock_addr;
    pthread_t temp_PID;

    kvs_localserver_sock = createAndBind_UNIX_stream_Socket(server_addr);
    if(kvs_localserver_sock<0)
    {
        pthread_exit(NULL);
    }

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
                perror("Error creating new connection thread");
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
    int cycle = 1;
    long int value_size = 0;
    pthread_t local_PID;
    struct key_value * local_key_value_table;


    pthread_cond_t cond = PTHREAD_COND_INITIALIZER;
    pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

    pthread_cond_init(&cond,NULL); 
    pthread_mutex_init(&mutex,NULL);

    

    char * group_id;
    char * secret;
    char * key;
    char * value;
    int ison=1;
    group_id = malloc(group_id_max_size*sizeof(char));
    secret = malloc(secret_max_size*sizeof(char));
    key = malloc(key_max_size*sizeof(char));

    client_sock = *((int *)arg);

    local_PID = pthread_self();

    read(client_sock,&client_PID,sizeof(client_PID));
    read(client_sock,group_id,(group_id_max_size*sizeof(char)));
    read(client_sock,secret,(secret_max_size*sizeof(char)));


    //CONFLICT HERE
    local_key_value_table = hashGet_group_table(groups, group_id);
    if(local_key_value_table == NULL)
    {
        answer = -1;
        write(client_sock,&answer,sizeof(answer));
        close(client_sock);
        pthread_exit(NULL);
    }
    

    answer=AuthServerCom(CMP,group_id,secret,Authserver_sock,Authserver_sock_addr);
    answer--;
    write(client_sock,&answer,sizeof(answer));

    if (answer==0){

        if(add_status(state, local_PID, client_PID, &all_clients_connected,group_id,&ison) == -1)
            printf("Error updating status");
        //Connection cycle
        while(server_status == 1 && cycle && ison)
        {
            answer = WAIT;
            key[0] = '\0';
            read(client_sock,&answer,sizeof(answer));
            if(answer == WAIT)
                continue;
            else if(answer == PUT)
            {
                read(client_sock,key,key_max_size*sizeof(char));
                read(client_sock,&value_size,sizeof(value_size));
                value = malloc(value_size*sizeof(char));
                read(client_sock,value,value_size*sizeof(char));
                answer = hashInsert_key_value(local_key_value_table,key,value);
                answer++;
                write(client_sock,&answer,sizeof(answer));
                free(value);
            }
            else if(answer == GET)
            {
                //CONFLICT HERE
                read(client_sock,key,key_max_size*sizeof(char));
                value = hashGet_key_value(local_key_value_table,key);
                if(value == NULL)
                    value_size = 0;
                else
                    value_size = strlen(value);
                write(client_sock,&value_size,sizeof(value_size));
                write(client_sock,value,value_size*sizeof(char));
            }
            else if(answer == DEL)
            {
                //CONFLICT HERE
                read(client_sock,key,key_max_size*sizeof(char));
                answer = hashDelete_key_value(local_key_value_table,key);
                answer++; //Updating from one format to another
                write(client_sock,&answer,sizeof(answer));
            }
            else if(answer == CLS)
            {
                cycle = 0;
            }else if(answer==CALL){
                read(client_sock,key,key_max_size*sizeof(char));
                answer=hashWaitChange_key_value(local_key_value_table,key);
                
                write(client_sock,&answer,sizeof(answer));
            }else{
                answer=-3;
                write(client_sock,&answer,sizeof(answer));
            }
        }
    }

    //Delete Status
    
    if(close(client_sock)<0)
    {
        perror("Error closing connection");
    }

    if(close_status(state, local_PID, client_PID, all_clients_connected) == -1)
        printf("Error updating status");

    pthread_exit(NULL);
}