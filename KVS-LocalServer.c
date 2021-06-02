#include "Basic.h"
#include "key_value_struct.h"
#include "group_table_struct.h"
#include "app_status_struct.h"

/*struct message{
    int request;
    char * group;
    char * secret;
    struct message * next;
};*/

struct message
{
    int answer;
    char group[group_id_max_size];
    char secret[secret_max_size];
    int request;
    struct message * next;
};

struct message * Main=NULL;

//Declaration of functions writen in the end
int createAndBind_UNIX_stream_Socket(char * sock_addr);
void acceptConnections(void *arg);
void handleConnection(void *arg);
void handleAuthCom(void *arg);


void AuthServerCom(char * port_str,char * authaddr_str);

//Global variables (to be shared across all server threads)
int server_status = 1; //1 -> ON; 0 -> OFF 
int all_clients_connected = 0;
struct group_table * groups; //hash table with all groups
struct app_status * state; //struct with all clients and their time information
int auth_socket;
//        struct message * Main=NULL;

int main(void)
{
    int selector;
    int aux = 0;
    char input_string[input_string_max_size];
    char input_string2[input_string_max_size];
    pthread_t acepting_connections_thread_ptid;

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

            //Ask auth-server to create key

            if(hashInsert_group_table(groups, input_string) == 0)
                printf("Group created with sucess\n\n");
            else
                printf("Error creating selected group\n\n");
        }
        else if(selector==2)
        {
            // Delete also on auth-server
            printf("Insert the group ID to delete: ");
            fgets(input_string, group_id_max_size, stdin);
            if(hashDelete_group_table(groups, input_string) == 0)
                printf("Group deleted with sucess\n\n");
            else
                printf("Error deleting selected group\n\n");
        }
        else if(selector == 3)
        {
            //easy to implment
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

//Functions used to simplify code

//Returns binded socket (-1 if error)
int createAndBind_UNIX_stream_Socket(char * sock_addr)
{
    int sock;
    struct sockaddr_un struct_sock_addr;
    //To make sure the address is free
    remove(server_addr);
    //Creating socket
    
    sock = socket(AF_UNIX, SOCK_STREAM, 0);
    if(sock==-1){
        perror("Error creating socket\n");
        return -1;
    }

    //Binding address
    memset(&struct_sock_addr,0,sizeof(struct sockaddr_un));
    struct_sock_addr.sun_family=AF_UNIX;
    strcpy(struct_sock_addr.sun_path, sock_addr);
    if(bind(sock, &struct_sock_addr, sizeof(struct_sock_addr)) < 0)
    {
        perror("Error binding socket\n");
        return -1;
    }

    return sock;
}

//Accepts connection if timeout is not exeeded
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

    char * group_id;
    char * secret;
    char * key;
    char * value;
    group_id = malloc(group_id_max_size*sizeof(char));
    secret = malloc(secret_max_size*sizeof(char));
    key = malloc(key_max_size*sizeof(char));

    client_sock = *((int *)arg);

    local_PID = pthread_self();

    read(client_sock,&client_PID,sizeof(client_PID));
    read(client_sock,group_id,(group_id_max_size*sizeof(char)));
    read(client_sock,secret,(secret_max_size*sizeof(char)));

    local_key_value_table = hashGet_group_table(groups, group_id);
    if(local_key_value_table == NULL)
    {
        answer = 0;
        write(client_sock,&answer,sizeof(answer));
        close(client_sock);
        pthread_exit(NULL);
    }
    
    //AUTENTICATE HERE
    answer=1;
    write(client_sock,&answer,sizeof(answer));

    if(add_status(state, local_PID, client_PID, &all_clients_connected) == -1)
        printf("Error updating status");
    //Connection cycle
    while(server_status == 1 && cycle)
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
            read(client_sock,key,key_max_size*sizeof(char));
            answer = hashDelete_key_value(local_key_value_table,key);
            answer++; //Updating from one format to another
            write(client_sock,&answer,sizeof(answer));
        }
    }
    
    if(close(client_sock)<0)
    {
        perror("Error closing connection");
        pthread_exit(NULL);
    }


    pthread_exit(NULL);
}




void AuthServerCom(char * port_str,char * authaddr_str){
    int Authserver_sock;
    unsigned short port;
    struct sockaddr_in * Authserver_sock_addr=malloc(sizeof(struct sockaddr_in));
    socklen_t len = sizeof (struct sockaddr_in);
    char * buf=malloc(1050*sizeof(char));
    int answer;


    port = htons(atoi(port_str));

    //Creating socket
    Authserver_sock = socket(AF_INET, SOCK_DGRAM, 0);
    if(Authserver_sock==-1){
        perror("Error creating main local server socket\n");
        return -1;
    }

    //Binding address
    memset(Authserver_sock_addr,0,sizeof(struct sockaddr_in));
    Authserver_sock_addr->sin_family=AF_INET;
    Authserver_sock_addr->sin_port=port;
    Authserver_sock_addr->sin_addr.s_addr = inet_addr(authaddr_str);

    while(1){
        while(Main!=NULL){
            sprintf(buf,"%d:%s",Main->request,Main->group);
            sendto(Authserver_sock,buf,sizeof(buf),0, (struct sockaddr*)Authserver_sock_addr ,len);

            if(Main->request==GET){
                recvfrom(Authserver_sock,buf,sizeof(buf),0,(struct sockaddr*)Authserver_sock_addr ,&len);
                strcpy(Main->secret,buf);
            }else{
                recvfrom(Authserver_sock,&answer,sizeof(int),0,(struct sockaddr*)Authserver_sock_addr ,&len);
                if(Main->request==DEL || Main->request==PUT || Main->request==CMP){
                    strcpy(buf,Main->secret);
                    sendto(Authserver_sock,buf,sizeof(buf),0, (struct sockaddr*)Authserver_sock_addr ,len);
                    recvfrom(Authserver_sock,&answer,sizeof(int),0,(struct sockaddr*)Authserver_sock_addr ,&len);

                }
                Main->answer=answer;
            }
            Main->request=WAIT;
            Main=Main->next;
        }
    }

    close(Authserver_sock);

}