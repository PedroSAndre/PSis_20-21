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
int all_clients_connected = 0; //starts with no clients connected

struct group_table * groups; //hash table with all groups
struct app_status * state; //struct with all clients and their time information

int Authserver_sock;
struct sockaddr_in Authserver_sock_addr;

pthread_mutex_t acess_group; //to lock access to groups when critical operations are being executed
char deleting_group[group_id_max_size];

/*./KVS-LocalServer.out     Inicialization and control of the server

                            Arguments:  - argv[1] - IP of auth-server
                                        - argv[2] - PORT of auth-server*/
int main(int argc, char ** argv)
{
    int selector;
    int aux = 0;
    char input_string[input_string_max_size] = "\0";
    char secret[secret_max_size] = "\0";
    pthread_t acepting_connections_thread_ptid;

    if(argc !=3){
        perror("Incorrect number of arguments");
        return WRNGARG;
    }


    strcpy(deleting_group,"\0");

    if(createAuthServerSock(argv[2],argv[1],&Authserver_sock,&Authserver_sock_addr)==ERRSCKCREATION)
    {
        printf("Error creating auth-local server socket\n");
        return ERRSCKCREATION;
    }

    state = inicialize_app_status();
    if(state == NULL)
    {
        printf("Error inicializing app_status\n");
        return ERRMALLOC;
    }

    groups = hashCreateInicialize_group_table();
    if(groups == NULL)
    {
        printf("Error inicializing group_table\n");
        return ERRMALLOC;
    }

    if(pthread_create(&acepting_connections_thread_ptid,NULL,(void *)&acceptConnections,NULL)<0)
    {
        perror("Error creating thread\n");
        return ERRPTHR;
    }

    printf("*****Welcome to KVS Local Server*****\n");
    printf("Auth-Server IP address is: %s\n", inet_ntoa(Authserver_sock_addr.sin_addr));
    printf("Auth-Server Port is: %d\n\n", (int) ntohs(Authserver_sock_addr.sin_port));

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
            do
            {
                fgets(input_string, group_id_max_size, stdin); //asks until string is not empty
            } while(strlen(input_string)==1);
            input_string[strlen(input_string)-1]='\0'; //deleting \n
            aux = AuthServerCom(PUT,input_string,secret,Authserver_sock,Authserver_sock_addr);
            if(aux == ERRRD)
            {
                printf("Group not found on Auth server\n");
            }
            else if(aux<SUCCESS)
            {
                printf("No response from Auth server\n");
            }
            else if(aux==SUCCESS)
            {
                printf("Secret of group %s\n%s\n",input_string,secret);
                pthread_mutex_lock(&acess_group);
                aux=hashInsert_group_table(groups, input_string);
                if(aux == SUCCESS)
                    printf("Group %s created with sucess\n\n", input_string);
                else if(aux==DENIED)
                    printf("Group %s already created\n\n", input_string);
                else
                    printf("Error alocating memory for the creation of selected group\n\n");
                pthread_mutex_unlock(&acess_group);
            }
        }
        else if(selector==2)
        {
            printf("Delete the group with ID: ");
            do
            {
                fgets(input_string, group_id_max_size, stdin); //asks until string is not empty
            } while(strlen(input_string)==1);
            input_string[strlen(input_string)-1]='\0';
            aux=AuthServerCom(DEL,input_string,secret,Authserver_sock,Authserver_sock_addr);
            if(aux == ERRRD)
            {
                printf("Group not found on Auth server\n");
            }
            else if(aux<SUCCESS)
            {
                printf("No response from Auth server\n");
            }
            else if(aux==SUCCESS)
            {
                printf("Waiting for clients to be disconnected...\n");
                strcpy(deleting_group, input_string);
                signal_all_callback(hashGet_group_table(groups,input_string));
                wait_to_group_clients_to_disconect(state,all_clients_connected,input_string);
                printf("All clients disconected\n");
                pthread_mutex_lock(&acess_group);
                if(hashDelete_group_table(groups, input_string) == ERRRD){
                    pthread_mutex_unlock(&acess_group);
                    printf("Error deleting selected group\nIt was not found\n");
                }
                else
                {
                    pthread_mutex_unlock(&acess_group);
                    printf("Group %s deleted sucessfully\n\n", input_string);
                }
                strcpy(deleting_group, "\0");
            }
            else
            {
                printf("Something went wrong in the Auth Server\n");
            }
        }
        else if(selector == 3)
        {
            printf("Get info of group ID: ");
            do
            {
                fgets(input_string, group_id_max_size, stdin); //asks until string is not empty
            } while(strlen(input_string)==1);
            input_string[strlen(input_string)-1]='\0';
            aux=AuthServerCom(GET,input_string,secret,Authserver_sock,Authserver_sock_addr);
            if(aux == ERRRD)
            {
                printf("Group not found on Auth server\n");
            }
            else if(aux<SUCCESS)
            {
                printf("No response from Auth server\n");
            }
            else if(aux==SUCCESS)
            {
                printf("Group %s has secret\n%s\n\n", input_string, secret);
            }
            else
            {
                printf("Something went wrong with the Auth Server\n");
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

    hashFree_group_table(groups);
    free(state);



    printf("Server terminated sucessfully\n");

    return SUCCESS;
}



//acceptConnections()   Thread function that is constantly (until server_status=0) accepting connections and giving them to handleConnection() to handle
void acceptConnections(void *arg)
{
    int kvs_localserver_sock;
    int client_sock=0;
    pthread_t temp_PID;

    kvs_localserver_sock = createAndBind_UNIX_stream_Socket(server_addr);
    if(kvs_localserver_sock<0)
    {
        printf("Fatal error - could not create socket to accept connections\n");
        pthread_exit(NULL);
    }

    if(listen(kvs_localserver_sock,max_waiting_connections)<0)
    {
        perror("Fatal error - could not listen for connections\n");
        pthread_exit(NULL);
    }

    //Connection cycle
    while(server_status)
    {
        client_sock = accept_connection_timeout(&(kvs_localserver_sock));
        if(client_sock != ERRTIMEOUT)
        {
            if(pthread_create(&temp_PID,NULL,(void *)&handleConnection,(void *)&client_sock)<SUCCESS)
            {
                perror("Error creating new connection thread\n\n");
            }
        }
    }

    //waits for all connection threads to close
    for(int i=1;i<=all_clients_connected;i++)
    {
        pthread_join(state[i].process_ptid,NULL);
    }

    if(close(kvs_localserver_sock)<0)
    {
        perror("Error closing accepting connections socket\n");
    }
    pthread_exit(NULL);
}

/*acceptConnections()   Thread function that handles a connection to a client

                        Arguments:  arg[0]  - file descriptor of the client socket*/
void handleConnection(void *arg)
{
    int client_sock;
    int answer=0;
    int client_PID;
    int cycle = 1;
    long int value_size = 0;
    pthread_t local_PID;
    struct key_value * local_key_value_table;

    char group_id[group_id_max_size];
    char secret[secret_max_size];
    char key[key_max_size];
    char * value;


    client_sock = *((int *)arg);

    local_PID = pthread_self();


    read(client_sock,&client_PID,sizeof(client_PID));
    read(client_sock,group_id,(group_id_max_size*sizeof(char)));
    read(client_sock,secret,(secret_max_size*sizeof(char)));


    answer=AuthServerCom(CMP,group_id,secret,Authserver_sock,Authserver_sock_addr);

    pthread_mutex_lock(&acess_group);
    local_key_value_table = hashGet_group_table(groups, group_id);
    if(local_key_value_table == NULL)
    {
        answer = ERRRD;
    }

    if(answer==SUCCESS)
    {
        
        if(add_status(&state, local_PID, client_PID, &all_clients_connected,group_id,deleting_group) <SUCCESS)
        {
            printf("Error updating status\nNot allowing new connection\n\n");
            cycle=0;
            answer = DENIED;
        }
        pthread_mutex_unlock(&acess_group);

        write(client_sock,&answer,sizeof(answer));
        
        //Connection cycle
        while(server_status && cycle && strcmp(group_id,deleting_group)!=0)
        {
            answer = WAIT;
            key[0] = '\0';
            if(read_timeout(&client_sock,&answer,sizeof(answer))==ERRTIMEOUT)
            {
                continue;
            }
            if(answer == WAIT)
                continue;
            else if(answer == PUT)
            {
                read(client_sock,key,key_max_size*sizeof(char));
                read(client_sock,&value_size,sizeof(value_size));
                value = malloc(value_size*sizeof(char));
                read(client_sock,value,value_size*sizeof(char));
                answer = hashInsert_key_value(local_key_value_table,key,value);
                write(client_sock,&answer,sizeof(answer));
                free(value);
            }
            else if(answer == GET)
            {
                read(client_sock,key,key_max_size*sizeof(char));
                value = hashGet_key_value(local_key_value_table,key);
                if(value == NULL)
                    value_size = ERRRD;
                else
                    value_size = strlen(value);
                write(client_sock,&value_size,sizeof(value_size));
                if(value != NULL)
                {
                    write(client_sock,value,value_size*sizeof(char));
                    free(value);
                }
            }
            else if(answer == DEL)
            {
                read(client_sock,key,key_max_size*sizeof(char));
                answer = hashDelete_key_value(local_key_value_table,key);
                write(client_sock,&answer,sizeof(answer));
            }
            else if(answer == CLS)
            {
                cycle = 0;
            }
            else if(answer==CALL)
            {
                read(client_sock,key,key_max_size*sizeof(char));
                if(strcmp(group_id,deleting_group)!=0)
                {
                    answer=hashWaitChange_key_value(local_key_value_table,key);
                }
                else
                {
                    answer = DISCONNECTED;
                }
                write(client_sock,&answer,sizeof(answer));
            }
            else
            {
                answer=WRGREQ;
                write(client_sock,&answer,sizeof(answer));
            }
        }
        if(close_status(state, local_PID, client_PID, all_clients_connected) <SUCCESS)
            printf("Error updating status\n");
    }
    else
    {
        write(client_sock,&answer,sizeof(answer));
    }
    
    if(close(client_sock)<0)
    {
        perror("Error closing connection\n");
    }



    pthread_exit(NULL);
}