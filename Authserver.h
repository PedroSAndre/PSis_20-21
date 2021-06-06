#ifndef _Authserver_
#define _Authserver_



#define SIZE 101
#define nASCII 95


//This structure is a list of messages from the various customers
struct Message{
    struct sockaddr_in clientaddr;
    int request;
    char group[group_id_max_size];
    char secret[secret_max_size];
    struct Message * next;
};

//This structure will store all pairs of group-key values
struct HashGroup {
    char group[group_id_max_size];
    char secret[secret_max_size];
    struct HashGroup * next;
};

//This table will store all pairs of values group-key
struct HashGroup * Table[SIZE];

//Hash function for group hash table
int hashIndex(char * group);

//Creates an entry in hash function
int createUpdateEntry(char * group,char *secret);

//Deletes an entry in hash function. It requires the secret to make sure that the 
//entry can be deleted by the one requesting the delete
int deleteEntry(char * group);

//Request the group secret
char * getGroupSecret(char * group);

//Check if the secret matches the one saved in Auth server. Authentication function
int compareHashGroup(char * group, char * checksecret);

//Finds a message related to the Local server requesting, by comparing IP and port
struct Message * recoverClientMessage(char * buf,struct sockaddr_in kvs_localserver_sock_addr,struct Message ** Main, int * err);

//If the message has already been analysed, it can be deleted
struct Message * deleteMessage(struct Message * Current, struct Message * Main);

//Generates a random sting for the secret with key_max_size
void generate_secret(char * secret);

//When shutting down server, it is necessary to deallocate the memory for the list of messages
void delete_All_messages(struct Message * Main);

//When shutting down server, it is necessary to deallocate the memory for the hash table
void delete_All_Entries();

//Funtion to deal with timeout of the recv
int recvfrom_timeout(int * socket_af_stream, void * to_read, int size_to_read,struct sockaddr * server_sock_addr, socklen_t * len);

#endif