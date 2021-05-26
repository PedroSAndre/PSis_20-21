#include "Basic.h"
#include "KVS-lib.h"


void f(char * key){
    printf("The key with name %s was changed\n", key);
    pthread_exit(0);
}


int main()
{
    char * group="Emilia e a melhor";
    char * secret="Obviously";
    char * value;
    char * key="OK";
    int err;
    err=establish_connection(group,secret);
    
    if(err==0){
        err=close_connection();
    }
    return 0;
}

