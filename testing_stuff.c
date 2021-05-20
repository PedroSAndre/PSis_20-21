#include "Basic.h"

int main(void)
{
    struct key_value * test_table;
    char * test_value;

    test_table = hashCreateInicialize_key_value();

    if(test_table == NULL)
    {
        perror("Deu merda");
        return -1;
    }

    if(hashInsert_key_value(test_table, "Testkey", "This is the value in here") != 0)
    {
        perror("Deu merda 2");
        return -1;
    }

    if(hashInsert_key_value(test_table, "Testkey2", "This is the value in here but different") != 0)
    {
        perror("Deu merda 2");
        return -1;
    }

    if(hashInsert_key_value(test_table, "Testkey3", "This is the value in here but different 3") != 0)
    {
        perror("Deu merda 2");
        return -1;
    }

    printf("%s\n%s\n%s\n",  hashGet_key_value(test_table, "Testkey"), hashGet_key_value(test_table, "Testkey2"), hashGet_key_value(test_table, "Testkey3"));
}