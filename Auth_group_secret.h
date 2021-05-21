#ifndef _Auth_group_secret_
#define _Auth_group_secret_

#define SIZE 101

struct HashGroup {
    char group[group_id_max_size];
    char secret[secret_max_size];
    struct HashGroup * next;
};

struct HashGroup * Table[SIZE];

//Hash function for group hash table
int HashIndex(char * group);

//Creates an entry in hash function
int CreateUpdateEntry(char * group,char *secret);

//Deletes an entry in hash function. It requires the secret to make sure that the 
//entry can be deleted by the one requesting the delete
int DeleteEntry(char * group, char * secret);

//Request the group secret
char * getGroupSecret(char * group);

//Check if the secret matches the one saved in Auth server. Authentication function
int compareHashGroup(char * group, char * checksecret);

#endif