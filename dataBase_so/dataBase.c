#include "dataBase.h"
#include <stdio.h>
#include <stdlib.h>
#include <json-c/json.h>
#include <json-c/json_object.h>
#include <string.h>
#include <unistd.h>
#define SQL_SIZE 666
#define BUFFER_SIZE 32
enum CODE_STATUS
{
    REPEATED_USER = -2,
    NO_APPLY,
    ON_SUCCESS,
};
#define ACCEPT 1
#define DENY 2
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
    int ret = sqlite3_open("../chatBase.db", &mydb);
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
        printf("sqlite3_exec 7:%s\n", errormsg);
        exit(-1);
    }
    sql = "create table if not exists friend(name text not null,friendName text,acceptfd int default 0,whetherOnline int default 0,privateMessage text,friendApply int default 0)";
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
        printf("sqlite3_exec 8:%s\n", errormsg);
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
        printf("sqlite3_exec 9:%s\n", errormsg);
        exit(-1);
    }
    sprintf(sql, "UPDATE friend SET whetherOnline = 1 WHERE name = '%s'", friendName);
    ret = sqlite3_exec(mydb, sql, NULL, NULL, &errormsg);
    if (ret != SQLITE_OK)
    {
        printf("sqlite3_exec error4:%s\n", errormsg);
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
    ret = sqlite3_exec(mydb, sql, NULL, NULL, &errormsg);
    if (ret != SQLITE_OK)
    {
        printf("sqlite3_exec error1:%s\n", errormsg);
        exit(-1);
    }
    sprintf(sql, " update friend SET whetherOnline = 0 WHERE friendName is not null and friendName = '%s'", json_object_get_string(acountVal));
    ret = sqlite3_exec(mydb, sql, NULL, NULL, &errormsg);
    if (ret != SQLITE_OK)
    {
        printf("sqlite3_exec error5:%s\n", errormsg);
        exit(-1);
    }
    sqlite3_close(mydb);
    return ON_SUCCESS;
}
/*好友申请*/
int dataBaseTakeApplyToName(struct json_object *parseObj, char *friendName)
{
    sqlite3 *mydb = NULL;
    struct json_object *nameVal = json_object_object_get(parseObj, "name");
    /*打开数据库*/
    int ret = sqlite3_open("chatBase.db", &mydb);
    if (ret != SQLITE_OK)
    {
        perror("sqlite3_open error");
        exit(-1);
    }
    char *errormsg = NULL;
    char sql[SQL_SIZE] = {0};
    /*将添加好友的对象的申请状态置为1*/
    sprintf(sql, " insert into friend ('name','friendName','friendApply') values('%s','%s',1)", json_object_get_string(nameVal), friendName);
    ret = sqlite3_exec(mydb, sql, NULL, NULL, &errormsg);
    if (ret != SQLITE_OK)
    {
        printf("sqlite3_exec 6:%s\n", errormsg);
        exit(-1);
    }
    sprintf(sql, " insert into friend ('name','friendName','friendApply') values('%s','%s',0)", friendName, json_object_get_string(nameVal));
    ret = sqlite3_exec(mydb, sql, NULL, NULL, &errormsg);
    if (ret != SQLITE_OK)
    {
        printf("sqlite3_exec 6:%s\n", errormsg);
        exit(-1);
    }
    sqlite3_close(mydb);
    return ON_SUCCESS;
}
/*name是否有好友申请*/
int dataBaseFindFriendApply(char *name)
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
    char sql[SQL_SIZE] = {0};
    char **result = NULL;
    int row = 0;
    int column = 0;
    sprintf(sql, " SELECT friendApply = 1  FROM friend WHERE name = '%s'", name);
    ret = sqlite3_get_table(mydb, sql, &result, &row, &column, &errormsg);
    if (ret != SQLITE_OK)
    {
        printf("sqlite3_get_table error9:%s\n", errormsg);
        exit(-1);
    }
    sqlite3_close(mydb);
    if (result[1] != NULL)
    {
        return ON_SUCCESS;
    }
    else
    {
        return NO_APPLY;
    }
}
char *dataBaseFindApplyFriendName(char *name)
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
    char sql[SQL_SIZE] = {0};
    char **result = NULL;
    int row = 0;
    int column = 0;
    sprintf(sql, " SELECT friendName FROM friend WHERE name = '%s' and friendApply=1 ", name);
    ret = sqlite3_get_table(mydb, sql, &result, &row, &column, &errormsg);
    if (ret != SQLITE_OK)
    {
        printf("sqlite3_get_table error10:%s\n", errormsg);
        exit(-1);
    }
    sqlite3_close(mydb);

    if (result[0] != NULL)
    {
        return result[1];
    }
    else
    {
        return NULL;
    }
}
static char *dataBaseAppointFriendNameFindName(const char *friendName)
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
    char sql[SQL_SIZE] = {0};
    char **result = NULL;
    int row = 0;
    int column = 0;
    sprintf(sql, " SELECT name FROM friend WHERE friendName = '%s' and friendApply=1", friendName);
    ret = sqlite3_get_table(mydb, sql, &result, &row, &column, &errormsg);
    if (ret != SQLITE_OK)
    {
        printf("sqlite3_get_table error11:%s\n", errormsg);
        exit(-1);
    }
    sqlite3_close(mydb);
    return result[1];
}
int handleApply(int status, char *name)
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
    char sql[SQL_SIZE] = {0};
    char *friendName = dataBaseFindApplyFriendName(name);
    /*将添加好友的对象的申请状态置为1*/
    if (status == ACCEPT)
    {
        sprintf(sql, " update friend SET whetherFriend = 1 WHERE name = '%s'and friendName='%s'", name, friendName);
        ret = sqlite3_exec(mydb, sql, NULL, NULL, &errormsg);
        if (ret != SQLITE_OK)
        {
            printf("sqlite3_exec 6:%s\n", errormsg);
            exit(-1);
        }
        sprintf(sql, " update friend SET whetherFriend = 1 WHERE name = '%s'and friendName='%s'", friendName, name);
        ret = sqlite3_exec(mydb, sql, NULL, NULL, &errormsg);
        if (ret != SQLITE_OK)
        {
            printf("sqlite3_exec 6:%s\n", errormsg);
            exit(-1);
        }
    }
    sprintf(sql, " update friend SET friendApply = 0 WHERE name = '%s'and friendName='%s'", name, friendName);
    ret = sqlite3_exec(mydb, sql, NULL, NULL, &errormsg);
    if (ret != SQLITE_OK)
    {
        printf("sqlite3_exec 6:%s\n", errormsg);
        exit(-1);
    }
    sprintf(sql, " update friend SET friendApply = 0 WHERE name = '%s'and friendName='%s'", friendName, name);
    ret = sqlite3_exec(mydb, sql, NULL, NULL, &errormsg);
    if (ret != SQLITE_OK)
    {
        printf("sqlite3_exec 6:%s\n", errormsg);
        exit(-1);
    }
    sqlite3_close(mydb);
    return ON_SUCCESS;
}
static char **dataBaseAppointNameFindFriendName(const char *name, int *resultRow)
{
    char pptmp[BUFFER_SIZE] = {0};
    strncpy(pptmp, name, sizeof(pptmp) - 1);
    sqlite3 *mydb = NULL;
    /*打开数据库*/
    int ret = sqlite3_open("/code/chatHome/Final-Project/chatBase.db", &mydb);
    if (ret != SQLITE_OK)
    {
        perror("sqlite3_open error");
        exit(-1);
    }
    char *errormsg = NULL;
    /*bug->todo...*/
    char sql[SQL_SIZE] = {0};
    char **result = NULL;
    int row = 0;
    int column = 0;
    sprintf(sql, " SELECT friendName FROM friend WHERE name = '%s' and whetherFriend = 1 ", pptmp);
    ret = sqlite3_get_table(mydb, sql, &result, &row, &column, &errormsg);
    if (ret != SQLITE_OK)
    {
        printf("sqlite3_get_table error8:%s\n", errormsg);
        exit(-1);
    }
    if (row != 0)
    {
        *resultRow = row;
        return result;
    }
    else
    {
        return NULL;
    }
}
int dataBaseDeleteFriend(struct json_object *parseObj, char *loginedName)
{
    const char *deleteName = json_object_get_string(json_object_object_get(parseObj, "deleteName"));
    int row;
    char **friendName = dataBaseAppointNameFindFriendName(deleteName, &row);
    int idx = 1;
    while (idx <= row)
    {
        printf("%s\n", friendName[idx]);
        if (strcmp(friendName[idx], loginedName) == 0)
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
            char sql[SQL_SIZE] = {0};
            sprintf(sql, "delete from friend where (name='%s' AND friendName='%s' and whetherFriend=1) OR (name='%s' AND friendName='%s' and whetherFriend=1)", loginedName, friendName[idx], friendName[idx], loginedName);
            ret = sqlite3_exec(mydb, sql, NULL, NULL, &errormsg);
            if (ret != SQLITE_OK)
            {
                printf("sqlite3_exec 13:%s\n", errormsg);
                exit(-1);
            }
            sqlite3_close(mydb);
            return ON_SUCCESS;
        }
        idx++;
    }
    return NO_APPLY;
}

int dataBaseDisPlayFriend(const char *loginedName)
{
    int row = 0;
    char **friendName = NULL;
    friendName = dataBaseAppointNameFindFriendName(loginedName, &row);
    sqlite3 *mydb = NULL;
    if (row == 0)
    {
        return NO_APPLY;
    }
    else
    {
        int idx = 1;
        while (idx <= row)
        {
            printf("%s\t", friendName[idx]);
            idx++;
        }
        printf("\n");
        sleep(1);
        return ON_SUCCESS;
    }
}