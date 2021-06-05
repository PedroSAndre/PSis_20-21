#include "Basic.h"
#include "KVS-lib.h"


void f(char * key){
    printf("The key with name %s was changed\n", key);
    pthread_exit(0);
}



int main(void)
{
    char selector;
    char buf[input_string_max_size];
    char group[group_id_max_size];
    char secret[secret_max_size];
    char key[key_max_size];
    char * value;
    int aux=1;
    pthread_t register_callback_thread_id;



    while(aux!=0){
        printf("Do you want to connect to a group?(n will close the program)(y/n):");
        fgets(buf, input_string_max_size, stdin);
        sscanf(buf,"%c\n",&selector);
        if(selector=='y'){
            aux=-7;
            while(aux==-7){

                printf("\nInsert the group you want to access: ");
                fgets(group, group_id_max_size, stdin);
                group[strlen(group)-1]='\0';

                printf("Insert the secret of the group: ");
                fgets(secret, secret_max_size, stdin);
                secret[strlen(secret)-1]='\0';

                
                aux=establish_connection(group,secret);
                if(aux!=0 && aux!=DENIED){
                    printf("\nUnable to connect.\nPlease, try again or leave the program\n");
                    printf("Retry(r) or Leave(l): ");
                    fgets(buf, input_string_max_size, stdin);
                    sscanf(buf,"%c\n",&selector);
                    if(selector=='r'){
                        aux=-7;
                    }else{
                        break;
                    }
                }else if(aux==DENIED){
                    printf("\nWrong group ID or Secret.\nPlease, try again or leave the program\n");
                    printf("Retry(r) or Leave(l): ");
                    fgets(buf, input_string_max_size, stdin);
                    sscanf(buf,"%c\n",&selector);
                    if(selector=='r'){
                        aux=-7;
                    }else{
                        break;
                    }
                }else{
                    printf("\nSucessfully established conection with group %s\n\n",group);
                    while(selector !='5'){
                        printf("Choose what action do you want to take:\n1 - Put value\n2 - Get value\n3 - Delete value\n4 - Register Callback\n5 - Close connection\n");
                        printf("User: ");
                        fgets(buf, input_string_max_size, stdin);
                        sscanf(buf,"%c\n",&selector);
                        if(selector=='1'){//Put value
                            printf("Insert the key you want to access: ");
                            fgets(key, key_max_size, stdin);
                            key[strlen(key)-1]='\0';
                            value=malloc(100*sizeof(char));
                            printf("Insert the value of the key you inserted: ");
                            fgets(value,100, stdin);
                            value[strlen(value)-1] = '\0';
                            aux=put_value(key,value);
                            if(aux==1){
                                printf("\nSuccessfully inserted value\n");
                            }else if(aux==DISCONNECTED){
                                printf("\nLocal Server is disconnecting. Do you want to reconnect to a group?\n");
                                printf("Retry(r) or Leave(l): ");
                                fgets(buf, input_string_max_size, stdin);
                                sscanf(buf,"%c\n",&selector);
                                if(selector=='r'){
                                    aux=-7;
                                }
                                break;
                            }else
                        }else if(selector=='2'){//Get value
                            printf("Insert the key of the entry you want to access: ");
                            fgets(key, key_max_size, stdin);
                            key[strlen(key)-1]='\0';
                            aux=get_value(key,&value);
                            if(aux==1){
                                printf("Value of key %s\n",key);
                                printf("Value: %s\n",value);
                            }else if(aux==DISCONNECTED){
                                printf("\nLocal Server is disconnecting. Do you want to reconnect to a group?\n");
                                printf("Retry(r) or Leave(l): ");
                                fgets(buf, input_string_max_size, stdin);
                                sscanf(buf,"%c\n",&selector);
                                if(selector=='r'){
                                    aux=-7;
                                }
                                break;
                            }else if(aux==ERRMALLOC){
                                printf("Shutting down program due to lack of memory\n");
                                aux=0;
                                break;
                            }else if(aux==DENIED){
                                
                            }
                        }else if(selector=='3'){//Delete value
                            printf("Insert the key of the entry you want to delete: ");
                            fgets(key, key_max_size, stdin);
                            key[strlen(key)-1]='\0';
                            if(delete_value(key)==1){
                                printf("Successfully deleted entry of key %s",key);
                            }else{
                                printf("Something went wrong\n");
                            }
                        }else if(selector=='4'){//Register callback
                            printf("Insert the key of the entry you want to monitor: ");
                            fgets(key, key_max_size, stdin);
                            key[strlen(key)-1]='\0';
                            printf("(Only a simple callback function is used)\n");
                            if(register_callback(key,f)!=1)
                            {
                                printf("Something went wrong\n");
                            }
                        }else if(selector == '5'){//Close connection
                            printf("Are you sure you want to leave the connection to group %s?\n",group);
                            printf("User: (y/n)");
                            fgets(buf, input_string_max_size, stdin);
                            sscanf(buf,"%c\n",&selector);
                            if(selector != 'y'){
                                aux=6;
                            }else{
                                if(close_connection()==1){
                                    printf("Connection closed\n");
                                    selector='5';
                                    aux=1;
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

