#include "Basic.h"
#include "group_table_struct.h"
#include "key_value_struct.h"

struct group_table * groups;

int main(void)
{
    struct key_value * test_table = NULL;
    int aux;

    groups = hashCreateInicialize_group_table();
    test_table = hashGet_group_table(groups, "Test_Group");
    aux = hashInsert_group_table(groups, "Test_Group");
    aux = hashInsert_group_table(groups, "Test_Group2");
    aux = hashInsert_group_table(groups, "Test_Group3");
    test_table = hashGet_group_table(groups, "Test_Group2");
    test_table = hashGet_group_table(groups, "Test_Group");
    test_table = hashGet_group_table(groups, "Test_Group3");
    return 1;
}