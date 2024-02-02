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
static int openSql(sqlite3 **mydb)
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
int dataBaseInit(sqlite3 **db)
{
    sqlite3 *mydb = NULL;
    /*打开数据库*/
    int ret = sqlite3_open("chatBase.db", &mydb);
    if (ret != SQLITE_OK)
    {
        perror("sqlite3_open error");
        exit(-1);
    }
    /*创建用户信息表*/
    char *errormsg = NULL;
    const char *sql = " create table if not exists user (name text primary key not null ,password text)";
    ret = sqlite3_exec(mydb, sql, NULL, NULL, &errormsg);
    if (ret != SQLITE_OK)
    {
        printf("sqlite3_exec error1:%s\n", errormsg);
        exit(-1);
    }
    sql = "create table if not exists friend(name text not null,friendName text,acceptfd int default 0,whetherOnline int default 0,privateMessage text)";
    ret = sqlite3_exec(mydb, sql, NULL, NULL, &errormsg);
    if (ret != SQLITE_OK)
    {
        printf("sqlite3_exec error2:%s\n", errormsg);
        exit(-1);
    }
    sqlite3_close(mydb);
    /*解引用*/
    *db = mydb;
    return ON_SUCCESS;
}
int dataBaseDuplicateCheck(struct json_object *parseObj)
{
    sqlite3 *mydb = NULL;
    /*打开数据库*/
    int ret = sqlite3_open("chatBase.db", &mydb);
    if (ret != SQLITE_OK)
    {
        perror("sqlite3_open error");
        exit(-1);
    }
    /*从解析后的对象中获取账户和密码*/
    struct json_object *acountVal = json_object_object_get(parseObj, "account");
    struct json_object *passwordVal = json_object_object_get(parseObj, "password");
    /*查询用户民是否存在数据库当中*/
    char *errormsg = NULL;
    char sql[SQL_SIZE] = {0};
    sprintf(sql, "select name from user where name='%s'", json_object_get_string(acountVal));
    char **result = NULL;
    int row = 0;
    int column = 0;
    ret = sqlite3_get_table(mydb, sql, &result, &row, &column, &errormsg);
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
    int ret = sqlite3_open("chatBase.db", &mydb);
    if (ret != SQLITE_OK)
    {
        perror("sqlite3_open error");
        exit(-1);
    }
    /*插入用户信息*/
    char *errormsg = NULL;

    char sql[SQL_SIZE] = {0};
    sprintf(sql, "insert into user values('%s','%s')", json_object_get_string(acountVal), json_object_get_string(passwordVal));
    ret = sqlite3_exec(mydb, sql, NULL, NULL, &errormsg);
    if (ret != SQLITE_OK)
    {
        printf("sqlite3_exec error1:%s\n", errormsg);
        exit(-1);
    }
    sqlite3_close(mydb);
    return ON_SUCCESS;
}
/*用户登录时更新为在线状态*/
int dataBaseFriendOnline(const char *friendName)
{
    sqlite3 *mydb = NULL;
    /*打开数据库*/
    int ret = sqlite3_open("chatBase.db", &mydb);
    if (ret != SQLITE_OK)
    {
        perror("sqlite3_open error");
        exit(-1);
    }
    /*插入用户信息*/
    char *errormsg = NULL;

    char sql[SQL_SIZE] = {0};
    sprintf(sql, "UPDATE friend SET whetherOnline = 1 WHERE friendName = '%s'", friendName);
    ret = sqlite3_exec(mydb, sql, NULL, NULL, &errormsg);
    if (ret != SQLITE_OK)
    {
        printf("sqlite3_exec error1:%s\n", errormsg);
        exit(-1);
    }
    sqlite3_close(mydb);
    return ON_SUCCESS;
}

/*将客户端上线状态变为下线*/
int dataBaseFriendOffline(struct json_object *parseObj)
{
    sqlite3 *mydb = NULL;
    struct json_object *acountVal = json_object_object_get(parseObj, "account");
    /*打开数据库*/
    int ret = sqlite3_open("chatBase.db", &mydb);
    if (ret != SQLITE_OK)
    {
        perror("sqlite3_open error");
        exit(-1);
    }
    /*插入用户信息*/
    char *errormsg = NULL;
    char sql[SQL_SIZE] = {0};
    /*将下线的账户的在线状态置为0*/
    sprintf(sql, " update friend SET whetherOnline = 0 WHERE name = '%s'", json_object_get_string(acountVal));
    if (ret != SQLITE_OK)
    {
        printf("sqlite3_exec error1:%s\n", errormsg);
        exit(-1);
    }
    sqlite3_close(mydb);
    return ON_SUCCESS;
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
    int ret = sprintf(sql, "insert into friend values('%s', '%s', NULL, NULL,) ", name, friendName);
    if (ret != SQLITE_OK)
    {
        printf("inster into friend %s", errormsg);
        exit(-1);
    }
    sqlite3_close(mydb);
}
