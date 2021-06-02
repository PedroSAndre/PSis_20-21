#include "Basic.h"
#include "KVS-lib.h"


void f(char * key){
    printf("The key with name %s was changed\n", key);
    pthread_exit(0);
}


int main(void)
{
    char group[group_id_max_size];
    char secret[secret_max_size];
    char key[key_max_size];
    char * value;
    int aux;

    printf("Insert the group you want to access: ");
    fgets(group, group_id_max_size, stdin);
    printf("Insert the secret of the group you inserted: ");
    fgets(secret, secret_max_size, stdin);


    aux=establish_connection(group,secret);
    if(aux!=0){
        return -1;
    }
    return 0;
}

