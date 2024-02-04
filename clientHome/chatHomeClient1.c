#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <signal.h>
#include <error.h>
#include <json-c/json.h>
#include <json-c/json_object.h>
#include "balanceBinarySearchTree.h"
#include <sqlite3.h>
#include "dataBase.h"
#define SERVER_PORT 8850
#define SERVER_IP "172.23.232.7"
#define BUFFER_SIZE 128
#define SUCCESS_LOGIN "登陆成功"
#define REGISTER "a"
#define LOGIN "b"
#define USER_SIZE 128
#define NO_APPLY_NOW "暂时没有好友申请"
typedef enum AFTER_LOGIN
{
    ADD_FRIEND = 1,
    HANDLE_APPLICATION = 2,
    DELETE_FRIREND = 3,
    SEND_MESSAGE = 4,
} AFTER_LOGIN;
/*全局变量*/
BalanceBinarySearchTree *AVL = NULL;
sqlite3 *mydb = NULL;
typedef struct USER
{
    char name[USER_SIZE];
    char password[USER_SIZE];
} USER;
enum CODE_STATUS
{
    REPEATED_USER = -1,
    ON_SUCCESS,
};

int compareFunc(void *arg1, void *arg2)
{
    USER val1 = *(USER *)arg1;
    USER val2 = *(USER *)arg2;
    size_t len1 = strlen(val1.name);
    size_t len2 = strlen(val2.name);
    return strncmp(val1.name, val2.name, len1 < len2 ? len1 : len2);
}
int printStructData(void *arg)
{
    int ret = 0;
    USER val = *(USER *)arg;
    printf("val.name:%s\tval.password:%s\t", val.name, val.password);
    return ret;
}
static int clientLogIn(int sockfd)
{
    int ret = 0;
    char *demand = LOGIN;

    struct json_object *clientObj = json_object_new_object();

    /*账号*/
    char accountNumber[BUFFER_SIZE];
    /*密码*/
    char *passwordNumber = calloc(BUFFER_SIZE, sizeof(char));
    printf("请输入账号\n");
    scanf("%s", accountNumber);
    while (getchar() != '\n')
        ;
    /*密码要求*/
    printf("请输入密码:\n");
    scanf("%s", passwordNumber);
    while (getchar() != '\n')
        ;

    json_object_object_add(clientObj, "choices", json_object_new_string(demand));
    json_object_object_add(clientObj, "account", json_object_new_string(accountNumber));
    json_object_object_add(clientObj, "password", json_object_new_string(passwordNumber));
    const char *sendStr = json_object_to_json_string(clientObj);
    int len = strlen(sendStr);
    char sendBuf[BUFFER_SIZE] = {0};
    strncpy(sendBuf, sendStr, sizeof(sendBuf) - 1);
    /*将json对象转化为字符串发给服务器*/
    int retWrite = write(sockfd, sendBuf, BUFFER_SIZE);
    if (retWrite == -1)
    {
        perror("write error3");
    }
    printf("check whether valid------------ing-----------\n");
    return ret;
}
static int clientRegister(int sockfd)
{
    int ret = 0;
    char *demand = REGISTER;

    struct json_object *registerObj = json_object_new_object();

    char accountNumber[BUFFER_SIZE];
    char *passwordNumber = calloc(BUFFER_SIZE, sizeof(char));
    printf("请输入账号\n");
    scanf("%s", accountNumber);
    while (getchar() != '\n')
        ;
    /*账号查重todo...*/
    /*备份passwordNumber*/
    char *accord = passwordNumber;
    int checkout = 0;
    while (checkout == 0)
    {
        /*密码要求*/
        printf("请输入密码(密码不少于8位且必须包含字符和数字):\n");
        scanf("%s", passwordNumber);
        while (getchar() != '\n')
            ;
        int numCount = 0;
        int characterCount = 0;
        while (*passwordNumber != '\0')
        {
            if (*passwordNumber >= '0' && *passwordNumber <= '9')
            {
                numCount++;
            }
            else
            {
                characterCount++;
            }
            passwordNumber++;
        }
        if (numCount + characterCount < 8 || numCount == 0 || characterCount == 0)
        {
            printf("密码不符合要求，请重新输入\n");
        }
        else
        {
            checkout = 1;
        }
    }

    json_object_object_add(registerObj, "choices", json_object_new_string(demand));
    json_object_object_add(registerObj, "account", json_object_new_string(accountNumber));
    json_object_object_add(registerObj, "password", json_object_new_string(accord));
    const char *sendStr = json_object_to_json_string(registerObj);
    char sendBuf[BUFFER_SIZE] = {0};
    strncpy(sendBuf, sendStr, sizeof(sendBuf) - 1);
    ret = write(sockfd, (void *)sendBuf, sizeof(sendBuf));
    if (ret == -1)
    {
        perror("write error2");
    }
    printf("check valid------------ing-----------\n");
    return ON_SUCCESS;
}
int addfriend(int sockfd)
{
    char *name = calloc(BUFFER_SIZE, sizeof(char));
    while (1)
    {
        printf("请输入对方名称:\n");
        scanf("%s", name);
        USER *friendUser = calloc(1, sizeof(USER));
        if (friendUser == NULL)
        {
            printf("friendUser calloc error\n");
        }
        strncpy(friendUser->name, name, sizeof(friendUser->name) - 1);
        if (balanceBinarySearchTreeIsContainAppointVal(AVL, friendUser) != ON_SUCCESS)
        {
            printf("该用户不存在,请重新输入\n");
        }
        else
        {
            break;
        }
    }
    int options = ADD_FRIEND;
    struct json_object *applyObj = json_object_new_object();
    json_object_object_add(applyObj, "options", json_object_new_int(options));
    json_object_object_add(applyObj, "name", json_object_new_string(name));
    json_object_object_add(applyObj, "choices", json_object_new_string("c"));
    const char *sendStr = json_object_to_json_string(applyObj);
    int len = strlen(sendStr);
    /*将json对象转化为字符串发给服务器*/
    int retWrite = write(sockfd, sendStr, len + 1);
    if (retWrite == -1)
    {
        perror("write error");
    }
    printf("发送中\n");
    sleep(2);

    return ON_SUCCESS;
}
int friendApplication(int sockfd)
{
    struct json_object *clientObj = json_object_new_object();
    json_object_object_add(clientObj, "options", json_object_new_int(HANDLE_APPLICATION));
    json_object_object_add(clientObj, "choices", json_object_new_string("c"));
    const char *sendStr = json_object_to_json_string(clientObj);
    char sendBuf[BUFFER_SIZE] = {0};
    strncpy(sendBuf, sendStr, sizeof(sendBuf) - 1);
    int ret = write(sockfd, (void *)sendBuf, sizeof(sendBuf));
    if (ret == -1)
    {
        perror("write error1");
    }
    return ON_SUCCESS;
}
int dataBaseToMemory(sqlite3 *db, BalanceBinarySearchTree *avl)
{

    sqlite3_open("../chatBase.db", &db);
    char *errormsg = NULL;
    const char *sql = "select * from user";
    char **result = NULL;
    int row = 0;
    int column = 0;
    int ret = sqlite3_get_table(db, sql, &result, &row, &column, &errormsg);
    if (ret != SQLITE_OK)
    {
        printf("sqlite3_exec error2:%s\n", errormsg);
        exit(-1);
    }
    /*略过表头，从第二行开始插入*/
    for (int idx = column; idx < (row + 1) * column; idx = idx + 2)
    {
        USER *dataBaseUser = calloc(1, sizeof(USER));
        strncpy(dataBaseUser->name, result[idx], sizeof(dataBaseUser->name) - 1);
        strncpy(dataBaseUser->password, result[idx + 1], sizeof(dataBaseUser->password) - 1);
        balanceBinarySearchTreeInsert(avl, (void *)dataBaseUser);
    }

    sqlite3_close(db);
    return ON_SUCCESS;
}
int deleteFriend(int sockfd)
{
    char deleteName[BUFFER_SIZE] = {0};
    printf("请输入需要删除好友的名称\n");
    scanf("%s", deleteName);
    struct json_object *clientObj = json_object_new_object();
    json_object_object_add(clientObj, "options", json_object_new_int(DELETE_FRIREND));
    json_object_object_add(clientObj, "deleteName", json_object_new_string(deleteName));
    json_object_object_add(clientObj, "choices", json_object_new_string("c"));
    const char *sendStr = json_object_to_json_string(clientObj);
    char sendBuf[BUFFER_SIZE] = {0};
    strncpy(sendBuf, sendStr, sizeof(sendBuf) - 1);
    int ret = write(sockfd, (void *)sendBuf, sizeof(sendBuf));
    if (ret == -1)
    {
        perror("write error1");
    }
    return ON_SUCCESS;
}
int main()
{
    /*初始化树*/
    balanceBinarySearchTreeInit(&AVL, compareFunc, printStructData);
    /*将数据库中的信息存入内存*/
    dataBaseToMemory(mydb, AVL);
    /*初始化数据库*/
    dataBaseInit(&mydb);
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd == -1)
    {
        perror("socket error");
        exit(-1);
    }

    struct sockaddr_in serverAddress;
    memset(&serverAddress, 0, sizeof(serverAddress));
    /* 端口 */
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(SERVER_PORT);
    /* IP地址 */
    int ret = inet_pton(AF_INET, SERVER_IP, (void *)&(serverAddress.sin_addr.s_addr));
    if (ret != 1)
    {
        perror("inet_pton error");
        exit(-1);
    }

    /* ip地址 */
    ret = connect(sockfd, (struct sockaddr *)&serverAddress, sizeof(serverAddress));
    if (ret == -1)
    {
        perror("connect error");
        exit(-1);
    }

    char recvBuffer[BUFFER_SIZE];
    memset(recvBuffer, 0, sizeof(recvBuffer));

    char *choices = calloc(BUFFER_SIZE, sizeof(char));
    int options = 0;
    while (strcmp(recvBuffer, SUCCESS_LOGIN))
    {
        printf("正在加载页面...\n");
        sleep(1);
        printf("a.注册\nb.登录\n");
        printf("请输入选项:\n");
        scanf("%s", choices);
        /*去除行缓存*/
        while (getchar() != '\n')
            ;

        if (!strcmp(choices, REGISTER))
        {
            clientRegister(sockfd);
            /*睡一会，等待函数执行*/
            sleep(2);
            ret = read(sockfd, recvBuffer, sizeof(recvBuffer) - 1);
            if (ret == -1)
            {
                perror("read error");
            }
            printf("提示:%s\n", recvBuffer);
        }
        else if (!strcmp(choices, LOGIN))
        {
            clientLogIn(sockfd);
            /*睡一会，等待函数执行*/
            sleep(2);
            ret = read(sockfd, recvBuffer, sizeof(recvBuffer) - 1);
            if (ret == -1)
            {
                perror("read error");
            }
            printf("提示:%s\n", recvBuffer);
        }
        sleep(1);
    }
    /*登录成功*/
    while (1)
    {
        printf("请输入选项:\n");
        printf("1.添加好友\n2.好友请求\n3.删除好友\n4.给好友发送消息\n");
        scanf("%d", &options);
        switch (options)
        {
        case ADD_FRIEND:
        {
            addfriend(sockfd);
            ret = read(sockfd, recvBuffer, sizeof(recvBuffer));
            if (ret == -1)
            {
                perror("read error");
            }
            printf("提示:%s\n", recvBuffer);
            break;
        }
        case HANDLE_APPLICATION:
        {
            /*向服务器发送HANDLE_APPLICATION请求*/
            friendApplication(sockfd);
            ret = read(sockfd, recvBuffer, sizeof(recvBuffer));
            if (ret == -1)
            {
                perror("read error");
            }
            printf("提示:%s\n", recvBuffer);
            sleep(2);
            if (strcmp(recvBuffer, NO_APPLY_NOW) != 0)
            {
                printf("请输入选项:\n");
                printf("1.接受\n2.拒绝\n");
                int status = 0;
                scanf("%d", &status);
                printf("读取成功\n");
                write(sockfd, (void *)&status, sizeof(status));
                printf("--------write ok-----\n");
            }
            break;
        }
        case DELETE_FRIREND:
        {

            deleteFriend(sockfd);
            break;
        }
        case SEND_MESSAGE:
        {
            break;
        }
        default:
            break;
        }
    }

    /* 休息5S */
    sleep(5);

    close(sockfd);

    return 0;
}