#include "dataBase.h"
#include <stdio.h>
#include <stdlib.h>
enum CODE_STATUS
{
    ON_SUCCESS,
};
/*初始化数据库*/
int dataBaseInit()
{
    sqlite3 *mydb = NULL;
    int ret = sqlite3_open("chatBase.db", &mydb);
    if (ret != SQLITE_OK)
    {
        perror("sqlite3_open error");
        exit(-1);
    }
    char *errormsg = NULL;
    const char *sql = " create table if not exists user (name text primary key not null not null,password text)";
    ret = sqlite3_exec(mydb, sql, NULL, NULL, &errormsg);
    if (ret != SQLITE_OK)
    {
        printf("sqlite3_exec error1:%s\n", errormsg);
        exit(-1);
    }
    sqlite3_close(mydb);
    return ON_SUCCESS;
}
