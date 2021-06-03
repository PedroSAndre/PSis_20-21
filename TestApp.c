#include "Basic.h"
#include "KVS-lib.h"


void f(char * key){
    printf("The key with name %s was changed\n", key);
    pthread_exit(0);
}


int main(void)
{
    char selector;
    char group[group_id_max_size];
    char secret[secret_max_size];
    char key[key_max_size];
    char value[100];//For this App value is capped but KVS is unlimited
    int aux=1;



    while(aux!=){
        printf("Do you want to connect to a group?(y/n)\n");
        printf("n will close the program\n")
        printf("User: ");
        selector=getchar();
        if(selector=='y'){
            aux=-7;
            while(aux==-7){

                printf("Insert the group you want to access: ");
                fgets(group, group_id_max_size, stdin);
                group[strlen(group)-1]='\0';
                printf("Insert the secret of the group you inserted: ");
                fgets(secret, secret_max_size, stdin);
                secret[strlen(secret)-1]='\0';

                
                aux=establish_connection(group,secret);
                if(aux!=0 && aux!=-7){
                    return -1;
                }else if(aux==-7){
                    printf("Wrong group ID or Secret.\nPlease, try again\n");
                }else{
                    printf("Sucessfully established conection with group %s\n",group);
                    while(selector !=CLS){
                        printf("Choose what action do you want to take:\n1 - Put value\n2 - Get value\n3 - Delete value\n4 - Register Callback\n5 - Close connection\n");
                        printf("User: ");
                        selector=getchar();
                        if(selector==PUT){
                            printf("Insert the key you want to access: ");
                            fgets(key, key_max_size, stdin);
                            printf("Insert the value of the key you inserted: ");
                            fgets(value,100, stdin);
                            if(put_value(key,value)==1){
                                printf("Successfully inserted value\n");
                            }else{
                                printf("Something went wrong\n");
                            }
                        }else if(selector==GET){
                            printf("Insert the key of the entry you want to access: ");
                            fgets(key, key_max_size, stdin);
                            if(get_value(key,&value)==1){
                                printf("Value of key %s",key);
                                printf("Value: %s",value);
                            }else{
                                printf("Something went wrong\n");
                            }
                        }else if(selector==DEL){
                            printf("Insert the key of the entry you want to delete: ");
                            fgets(key, key_max_size, stdin);
                            if(delete_value(key,&value)==1){
                                printf("Successfully deleted entry of key %s",key);
                            }else{
                                printf("Something went wrong\n");
                            }
                        }else if(selector==4){
                            printf("Insert the key of the entry you want to monitor: ");
                            fgets(key, key_max_size, stdin);
                            printf("(Only a simple callback function is used)\n");
                            if(delete_value(key)==1){
                                printf("Successfully deleted entry of key %s",key);
                            }else{
                                printf("Something went wrong\n");
                            }
                        }else if(selector == 5){
                            printf("Are you sure you want to leave the connection to group %s?\n",group);
                            printf("User: (y/n)");
                            selector=getchar();
                            if(selector != 'y'){
                                aux=6;
                            }else{
                                if(close_connection()==1){
                                    printf("Connection closed\n");
                                }else{
                                    printf("Something went wrong\n");
                                }
                            }
                        }
                    }
                }
            }
        }else{
            aux=0;
        }
    }
    printf("Closing program...\n");
    return 0;
}

