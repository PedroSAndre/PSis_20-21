#include "Basic.h"
#include "KVS-lib.h"





int main()
{
    char * group="Emilia e a melhor";
    char * secret="Obviously";
    int err;
    err=establish_connection(group,secret);

    if(err==0){
        err=close_connection();
    }
    return 0;
}

