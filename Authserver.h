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
int HashIndex(char * group);

//Creates an entry in hash function
int CreateUpdateEntry(char * group,char *secret);

//Deletes an entry in hash function. It requires the secret to make sure that the 
//entry can be deleted by the one requesting the delete
int DeleteEntry(char * group);

//Request the group secret
char * getGroupSecret(char * group);

//Check if the secret matches the one saved in Auth server. Authentication function
int compareHashGroup(char * group, char * checksecret);

//Finds a message related to the Local server requesting, by comparing IP and port
struct Message * recoverClientMessage(char * buf,struct sockaddr_in kvs_localserver_sock_addr,struct Message ** Main);

//If the message has already been analysed, it can be deleted
struct Message * deleteMessage(struct Message * Current, struct Message * Main);

char * generate_secret(void);

#endif