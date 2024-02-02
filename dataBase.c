#include "dataBase.h"
#include <stdio.h>
#include <stdlib.h>
#include <json-c/json.h>
#include <json-c/json_object.h>
#include <string.h>
#define SQL_SIZE 64
enum CODE_STATUS
{
    REPEATED_USER = -1,
    ON_SUCCESS,
};

/* 打开数据库的函数 */
int openSql(sqlite3 **mydb)
{
    int ret = sqlite3_open("chatBase.db", mydb);
    if (ret != SQLITE_OK)
    {
        perror("sqlite3_open error");
        exit(-1);
    }
    return ret;
}
/*初始化数据库*/
int dataBaseInit()
{
    sqlite3 *mydb = NULL;
    /*打开数据库*/
    openSql(&mydb);
    /*创建用户信息表*/
    char *errormsg = NULL;
    const char *sql = " create table if not exists user (name text primary key  not null, password text not null)";
   int  ret = sqlite3_exec(mydb, sql, NULL, NULL, &errormsg);
    if (ret != SQLITE_OK)
    {
        printf("sqlite3_exec error1:%s\n", errormsg);
        exit(-1);
    }

    sqlite3_close(mydb);
    return ON_SUCCESS;
}
int dataBaseDuplicateCheck(struct json_object *parseObj)
{
    sqlite3 *mydb = NULL;
    /*打开数据库*/
    openSql(&mydb);
    /*从解析后的对象中获取账户和密码*/
    struct json_object *acountVal = json_object_object_get(parseObj, "account");
    struct json_object *passwordVal = json_object_object_get(parseObj, "password");
    /*查询用户名是否存在数据库当中*/
    char *errormsg = NULL;
    char sql[SQL_SIZE] = {0};
    sprintf(sql, "select name from user where name='%s'", json_object_get_string(acountVal));
    char **result = NULL;
    int row = 0;
    int column = 0;
    int ret = sqlite3_get_table(mydb, sql, &result, &row, &column, &errormsg);
    if (ret != SQLITE_OK)
    {
        printf("sqlite3_exec error2:%s\n", errormsg);
        exit(-1);
    }
    /*如果重复，返回REPEATED_USER*/
    if (strcmp(json_object_get_string(acountVal), result[1]) == 0)
    {
        return REPEATED_USER;
    }
    sqlite3_close(mydb);
    /*如果不重复，返回ON_SUCCESS*/
    return ON_SUCCESS;
}

int dataBaseUserInsert(struct json_object *parseObj)
{
    /*解析json对象*/
    // struct json_object *parseObj = calloc(1, sizeof(parseObj));
    // parseObj = json_tokener_parse(jsonStr);
    struct json_object *acountVal = json_object_object_get(parseObj, "account");
    struct json_object *passwordVal = json_object_object_get(parseObj, "password");
    sqlite3 *mydb = NULL;
    /*打开数据库*/
      openSql(&mydb);
    /*插入用户信息*/
    char *errormsg = NULL;

    char sql[SQL_SIZE] = {0};
    sprintf(sql, "insert into user values('%s','%s')", json_object_get_string(acountVal), json_object_get_string(passwordVal));
   int  ret = sqlite3_exec(mydb, sql, NULL, NULL, &errormsg);
    if (ret != SQLITE_OK)
    {
        printf("sqlite3_exec error1:%s\n", errormsg);
        exit(-1);
    }
    sqlite3_close(mydb);
    return ON_SUCCESS;
}

/* 好友表数据库的删除 */
int dataBaseFriendDelete(const char *name, const char *friendName)
{
    sqlite3 *mydb = NULL;
     openSql(&mydb);
    char *errormsg = NULL;
    char sql[SQL_SIZE] = {0};
    sprintf(sql, "delete from friend where name = '%s'  and friendName= '%s'  ", name, friendName);

   int  ret = sqlite3_exec(mydb, sql, NULL, NULL, &errormsg);
    if (ret == SQLITE_OK)
    {
        printf("sqlite3 drop error %s", errormsg);
        exit(-1);
    }
    sqlite3_close(mydb);
}

/* 好友表的插入 insert   into values */
int dataBaseFriendInsert(const char *name, const char *friendName)
{
    /*打开数据库*/
    sqlite3 *mydb = NULL;
    openSql(&mydb);
    /*插入用户信息*/
    char *errormsg = NULL;
    char sql[SQL_SIZE] = {0};
}

#if 0
/* 好友表的查询 */
int dataBaseFriendSelect(const char *friendName)
{
     sqlite3 *mydb = NULL;
    /*打开数据库*/
    int ret = sqlite3_open("chatBase.db", &mydb);
    if (ret != SQLITE_OK)
    {
        perror("sqlite3_open error");
        exit(-1);
    }
    char *errormsg = NULL;
    char sql[SQL_SIZE] = { 0 };
    sprintf("select * from friend where friendName = '%s'", friendName);
    char **result = NULL;
    int row = 0;
    int column = 0;
    ret = sqlite3_get_table(mydb, sql, &result, &row, &column, &errormsg);
    if (ret != SQLITE_OK)
    {
        printf("sqlite3_exec error2:%s\n", errormsg);
        exit(-1);
    }
}
#endif