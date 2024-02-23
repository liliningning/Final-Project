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
#include "hashtable.h"
#include "../globalVary.h"
#define SERVER_PORT 8850
#define SERVER_IP "172.23.232.7"
#define BUFFER_SIZE 128
#define SUCCESS_LOGIN "登陆成功"
#define REGISTER "a"
#define LOGIN "b"
#define USER_SIZE 128
#define NO_APPLY_NOW "暂时没有好友申请"
#define SLOTNUMS 37
#define CONTINUE 0
#define STOP 1
typedef enum AFTER_LOGIN
{
    ADD_FRIEND = 1,
    HANDLE_APPLICATION,
    DELETE_FRIREND,
    SEND_MESSAGE,
    CREATE_GROUP,
    ADD_GROUP,
    GROUP_CHAT,
    EXIT,
} AFTER_LOGIN;
/* 客户端读写分离传参结构体 */
typedef struct PTH_CONNECT
{
    /* 服务端套接字 */
    int sockfd;
    /* 终止读标志 */
    int stop;
} PTH_CONNECT;
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

typedef struct ChatIndividual
{
    char sender[USER_SIZE];
    char recver[USER_SIZE];
    int sockfd;
} ChatIndividual;

typedef struct GroupMember
{
    char groupName[USER_SIZE];
    char signinedName[USER_SIZE];
    int sockfd;
} GroupMember;
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
static int clientLogIn(int sockfd, char **loginedName)
{

    char *demand = LOGIN;

    struct json_object *clientObj = json_object_new_object();

    /*账号*/
    char accountNumber[BUFFER_SIZE] = {0};
    char *remainStart = accountNumber;
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
    *loginedName = remainStart;
    if (passwordNumber != NULL)
    {
        free(passwordNumber);
        passwordNumber = NULL;
    }
    json_object_put(clientObj);
    return ON_SUCCESS;
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
    json_object_put(registerObj);
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
    json_object_put(clientObj);
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
    json_object_put(clientObj);
    return ON_SUCCESS;
}
int sendMessage(int sockfd, char *loginedName, char *chatFriendName, char *message)
{
    struct json_object *clientObj = json_object_new_object();
    json_object_object_add(clientObj, "options", json_object_new_int(SEND_MESSAGE));
    json_object_object_add(clientObj, "choices", json_object_new_string("c"));
    json_object_object_add(clientObj, "loginedName", json_object_new_string(loginedName));
    json_object_object_add(clientObj, "chatFriendName", json_object_new_string(chatFriendName));
    json_object_object_add(clientObj, "message", json_object_new_string(message));
    const char *sendStr = json_object_to_json_string(clientObj);
    char sendBuf[BUFFER_SIZE] = {0};
    strncpy(sendBuf, sendStr, sizeof(sendBuf) - 1);
    int ret = write(sockfd, (void *)sendBuf, sizeof(sendBuf));
    if (ret == -1)
    {
        perror("write error1");
    }
    json_object_put(clientObj);
    return ON_SUCCESS;
}
int compareHash(ELEMENTTYPE arg1, ELEMENTTYPE arg2)
{
    hashNode node1 = *(hashNode *)arg1;
    hashNode node2 = *(hashNode *)arg2;
    return strcmp(node1.real_key, node2.real_key);
}
void *readMsg_handler(void *arg)
{
    pthread_detach(pthread_self());
    ChatIndividual persons = *(ChatIndividual *)arg;
    char **message = NULL;
    int row = 0;
    char readBuffer[BUFFER_SIZE] = {0};
    while (1)
    {
        read(persons.sockfd, readBuffer, sizeof(readBuffer));
        // sleep(2);
        // hashTableGetAppointKeyValue(hash, persons.recver, &message, &row, persons.sender);
        // if (row == 0)
        // {
        //     continue;
        // }
        // else
        // {
        //     for (int idx = 0; idx < row; idx++)
        //     {
        printf("新消息：%s\n", readBuffer);
        //     }
        // }
    }
    pthread_exit(NULL);
}
/* 客户端私聊读线程函数 */
void *readThread(void *arg)
{

    /* 线程分离 */
    pthread_detach(pthread_self());

    char sendBuf[BUFFER_SIZE];
    char recvBuf[BUFFER_SIZE];

    int sockfd = ((PTH_CONNECT *)arg)->sockfd;
    memset(sendBuf, 0, sizeof(sendBuf));
    /* 告诉服务器取消息 */
    strncpy(sendBuf, "fetchMsg", strlen("fetchMsg"));
    while (1)
    {
        sleep(1);
        if (((PTH_CONNECT *)arg)->stop == CONTINUE)
        {
            /* ((PTH_CONNECT *)arg)->stop != STOP即传入线程函数的地址的值没有改变 */
            /* 不断通知服务端取消息 */
            write(sockfd, sendBuf, sizeof(sendBuf));
            memset(recvBuf, 0, sizeof(recvBuf));

            /* 读服务器传回的消息 */
            read(sockfd, recvBuf, sizeof(recvBuf));
            if (strncmp(recvBuf, "fetchMsg", strlen("fetchMsg")) != 0)
            {
                /*红色加粗下划线属性*/
                printf("\033[31;1;4m%s\033[0m\n", recvBuf);
            }
        }
        else
        {
            break;
        }
    }

    pthread_exit(NULL);
}
/* 发送和接收消息，读写分离 */
static int privateMsgChat(int sockfd)
{
    /* 需要读写分离 */
    char sendBuf[BUFFER_SIZE];
    char recvBuf[BUFFER_SIZE];

    pthread_t tid;
    PTH_CONNECT pth_Conect;

    pth_Conect.sockfd = sockfd;
    pth_Conect.stop = CONTINUE;

    /*读线程,读服务器发来的消息*/
    pthread_create(&tid, NULL, readThread, (void *)&pth_Conect);

    printf("聊天开启\n");
    while (1)
    {
        memset(sendBuf, 0, sizeof(sendBuf));
        // printf("输入(ESC退出):");
        scanf("%s", sendBuf);
        if (strlen(sendBuf) == 1 && sendBuf[0] == 27)
        {
            /* 输入了ESC键 */
            /* 停止读线程函数的read */
            pth_Conect.stop = STOP;
            printf("关闭读线程\n");
            memset(sendBuf, 0, sizeof(sendBuf));
            /* 告诉客户端可以停止读了 */
            strncpy(sendBuf, "stopRead", sizeof("stopRead"));
            write(sockfd, sendBuf, sizeof(sendBuf));
            break;
        }
        else
        {
            /* 要发的消息,传给服务端 */
            write(sockfd, sendBuf, sizeof(sendBuf));
        }
    }
    return 0;
}
/*persons中有user，friend和sockfd*/
int privateChat(ChatIndividual *persons)
{
    // SELECT INVITEE FROM FRIEND_DATA WHERE INVITER = '我';
    char sendBuf[BUFFER_SIZE];
    char recvBuf[BUFFER_SIZE];

    /* 打包行动 */
    struct json_object *friendObj = json_object_new_object();
    json_object_object_add(friendObj, "options", json_object_new_int(SEND_MESSAGE));
    json_object_object_add(friendObj, "loginedName", json_object_new_string(persons->recver));
    json_object_object_add(friendObj, "chatFriendName", json_object_new_string(persons->sender));
    json_object_object_add(friendObj, "choices", json_object_new_string("c"));
    const char *sendStr = json_object_to_json_string(friendObj);

    memset(sendBuf, 0, sizeof(sendBuf));
    strncpy(sendBuf, sendStr, sizeof(sendBuf) - 1);

    /*服务器第一次读*/
    /* 发送行动给服务端 */
    int ret = write(persons->sockfd, sendBuf, sizeof(sendBuf));
    if (ret == -1)
    {
        perror("write11 error");
    }
    printf("send options sendMessage\n");

    /* 读写分离发送消息和接收消息 */
    privateMsgChat(persons->sockfd);
    /* 释放json对象 */
    json_object_put(friendObj);
    return 0;
}
/* 客户端创建群聊申请 */
int createGroup(int sockfd)
{
    /* 群聊以群聊名作为唯一key */
    /* 告诉客户端创建群聊，并将要创建的群聊名告知服务端 */
    /* 服务端将进行查重 */
    /* 如果该群已存在，告知已存在，重新输入 */
    /* 不存在创建成功 */
    char groupName[BUFFER_SIZE];
    char sendBuf[BUFFER_SIZE];
    char recvBuf[BUFFER_SIZE];
    struct json_object *GroupNameCheck = json_object_new_object();

    while (1)
    {
        printf("请输入新建群聊的名称:\n");
        memset(groupName, 0, sizeof(groupName));
        scanf("%s", groupName);
        /* 打包 */
        json_object_object_add(GroupNameCheck, "options", json_object_new_int(CREATE_GROUP));
        json_object_object_add(GroupNameCheck, "groupName", json_object_new_string(groupName));
        json_object_object_add(GroupNameCheck, "choices", json_object_new_string("c"));
        /* 生成字符串 */
        const char *sendStr = json_object_to_json_string(GroupNameCheck);
        memset(sendBuf, 0, sizeof(sendBuf));
        memset(recvBuf, 0, sizeof(recvBuf));
        /* 放入发送缓存区 */
        strncpy(sendBuf, sendStr, sizeof(sendBuf) - 1);
        /* 发送给服务端 */
        write(sockfd, sendBuf, sizeof(sendBuf));
        /* 等待服务端回应 */
        read(sockfd, recvBuf, sizeof(recvBuf));
        /* 判断回应 */
        if (strncmp(recvBuf, "创建群聊成功", strlen("创建群聊成功")) == 0)
        {
            /* 可行，跳出循环，执行下一步操作 */
            printf("提示：%s\n", recvBuf);
            sleep(1);
            break;
        }
        else
        {
            /* 不可行，重新输入 */
            printf("提示：%s\n", recvBuf);
            sleep(1);
        }
    }
    /* 释放 */
    json_object_put(GroupNameCheck);
    return 0;
}
/* 添加群聊 */
int addGroup(int sockfd)
{
    /* 服务端先检查群聊是否存在 */
    /* 存在，查看是否已经是群成员 */

    char Group[BUFFER_SIZE];
    char sendBuf[BUFFER_SIZE];
    char recvBuf[BUFFER_SIZE];

    struct json_object *GroupObj = json_object_new_object();

    while (1)
    {
        printf("请输入要加入的群聊名称(esc退出):\n");
        memset(Group, 0, sizeof(Group));
        scanf("%s", Group);
        if (strlen(Group) == 1 && Group[0] == 27)
        {
            break;
        }
        /* 打包 */
        json_object_object_add(GroupObj, "options", json_object_new_int(ADD_GROUP));
        json_object_object_add(GroupObj, "groupName", json_object_new_string(Group));
        json_object_object_add(GroupObj, "choices", json_object_new_string("c"));
        /* 生成字符串 */
        const char *sendStr = json_object_to_json_string(GroupObj);
        memset(sendBuf, 0, sizeof(sendBuf));
        memset(recvBuf, 0, sizeof(recvBuf));
        /* 放入发送缓存区 */
        strncpy(sendBuf, sendStr, sizeof(sendBuf) - 1);
        /* 发送给服务端 */
        write(sockfd, sendBuf, sizeof(sendBuf));
        /* 等待服务端回应 */
        read(sockfd, recvBuf, sizeof(recvBuf));
        /* 判断回应 */
        if (strncmp(recvBuf, "ADDSUCCESS", strlen("ADDSUCCESS")) == 0)
        {
            /* 可行，跳出循环，执行下一步操作 */
            printf("成功加入群聊\n");
            sleep(1);
            break;
        }
        else if (strncmp(recvBuf, "GROUP_NOT_EXISTS", strlen("GROUP_NOT_EXISTS")) == 0)
        {
            /* 不可行，重新输入 */
            printf("群聊名称不存在,请重新输入\n");
            sleep(1);
        }
        else
        {
            /* 已经是群成员 */
            printf("您已是群成员\n");
            sleep(1);
        }
    }

    /* 释放 */
    json_object_put(GroupObj);
    return 0;
}
/* 读线程函数 */
static void *GrpReadThread(void *arg)
{
    /* 线程分离 */
    pthread_detach(pthread_self());

    char sendBuf[256];
    char recvBuf[256];

    int sockfd = ((PTH_CONNECT *)arg)->sockfd;
    memset(sendBuf, 0, sizeof(sendBuf));
    /* 告诉服务器取消息 */
    strncpy(sendBuf, "takeFetch", sizeof(sendBuf));

    while (1)
    {
        sleep(1);
        if (((PTH_CONNECT *)arg)->stop == CONTINUE)
        {
            /* ((PTH_CONNECT *)arg)->stop != STOP即传入线程函数的地址的值没有改变 */
            /* 不断通知服务端取消息 */
            write(sockfd, sendBuf, sizeof(sendBuf));
            memset(recvBuf, 0, sizeof(recvBuf));
            /* 读服务器传回的消息 */
            read(sockfd, recvBuf, sizeof(recvBuf));

            if (strncmp(recvBuf, "takeFetch", strlen("takeFetch")) != 0)
            {
                /*红色加粗下划线*/
                printf("\033[31;1;4m%s\033[0m\n", recvBuf);
            }
        }
        else
        {
            break;
        }
    }

    pthread_exit(NULL);
}

/* 读写分离 */
static int GrpMsgChat(int sockfd)
{
    /* 需要读写分离 */
    char sendBuf[256];
    char recvBuf[256];

    pthread_t tid;
    PTH_CONNECT pth_Conect;

    pth_Conect.sockfd = sockfd;
    pth_Conect.stop = CONTINUE;
    /* 创建读线程 */
    pthread_create(&tid, NULL, GrpReadThread, (void *)&pth_Conect);
    printf("群聊开启:\n");
    while (1)
    {
        memset(sendBuf, 0, sizeof(sendBuf));
        // printf("输入(ESC退出):");
        scanf("%s", sendBuf);
        if (strlen(sendBuf) == 1 && sendBuf[0] == 27)
        {
            /* 输入了ESC键 */
            /* 停止读线程函数的read */
            pth_Conect.stop = STOP;
            memset(sendBuf, 0, sizeof(sendBuf));
            /* 告诉服务端可以停止读了 */
            strncpy(sendBuf, "stopRead", sizeof(sendBuf));
            write(sockfd, sendBuf, sizeof(sendBuf));
            break;
        }
        else
        {
            /* 要发的消息,传给服务端 */
            write(sockfd, sendBuf, sizeof(sendBuf));
        }
    }
    return 0;
}

/*member中有sockfd,groupName,memberName*/
int groupChat(GroupMember *member)
{
    char sendBuf[256];
    char recvBuf[256];

    /* 行动 */
    struct json_object *GrChatObj = json_object_new_object();
    json_object_object_add(GrChatObj, "options", json_object_new_int(GROUP_CHAT));
    json_object_object_add(GrChatObj, "loginedName", json_object_new_string(member->signinedName));
    json_object_object_add(GrChatObj, "groupName", json_object_new_string(member->groupName));
    json_object_object_add(GrChatObj, "choices", json_object_new_string("c"));
    const char *str = json_object_to_json_string(GrChatObj);

    memset(sendBuf, 0, sizeof(sendBuf));
    strncpy(sendBuf, str, sizeof(sendBuf) - 1);
    write(member->sockfd, sendBuf, sizeof(sendBuf));

    /* 读写分离发送消息和接收消息 */
    GrpMsgChat(member->sockfd);

    /* 释放json对象 */
    json_object_put(GrChatObj);

    return 0;
}
int main()
{
    /*初始化树*/
    balanceBinarySearchTreeInit(&AVL, compareFunc, printStructData);
    /*将数据库中的信息存入内存*/
    dataBaseToMemory(mydb, AVL);
    /*初始化数据库*/
    dataBaseInit(&mydb);
    hashTableInit(&hash, SLOTNUMS, compareHash);
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

    char sendBuffer[BUFFER_SIZE];
    memset(sendBuffer, 0, sizeof(sendBuffer));
    char *choices = calloc(BUFFER_SIZE, sizeof(char));
    int options = 0;

    char recvAfter[BUFFER_SIZE] = {0};
    char chatFriendName[BUFFER_SIZE];
    memset(chatFriendName, 0, sizeof(chatFriendName));

    char chatGroupName[BUFFER_SIZE];
    memset(chatGroupName, 0, sizeof(chatGroupName));
    char message[BUFFER_SIZE] = {0};

    char *loginedName = calloc(BUFFER_SIZE, sizeof(char *));
    char signinedName[BUFFER_SIZE] = {0};
    while (1)
    {
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
            clientLogIn(sockfd, &loginedName);
            /*睡一会，等待函数执行*/
            sleep(2);
            ret = read(sockfd, recvBuffer, sizeof(recvBuffer) - 1);
            if (ret == -1)
            {
                perror("read error");
            }
            printf("提示:%s\n", recvBuffer);

            /*登录成功*/
            printf("loginedName=%s\n", loginedName);
            strncpy(signinedName, loginedName, sizeof(signinedName) - 1);
            options = 0;
            while (options != EXIT && (strcmp(recvBuffer, SUCCESS_LOGIN) == 0))
            {
                printf("请输入选项:\n");
                printf("1.添加好友\n2.好友请求\n3.删除好友\n4.发送消息\n5.创建群聊\n6.加入群聊\n7.群聊消息\n8.退出登录\n");
                scanf("%d", &options);
                switch (options)
                {
                case ADD_FRIEND:
                {
                    addfriend(sockfd);
                    ret = read(sockfd, recvAfter, sizeof(recvAfter));
                    if (ret <= 0)
                    {
                        perror("read error");
                    }
                    printf("提示:%s\n", recvAfter);
                    break;
                }
                case HANDLE_APPLICATION:
                {
                    /*向服务器发送HANDLE_APPLICATION请求*/
                    friendApplication(sockfd);
                    ret = read(sockfd, recvAfter, sizeof(recvAfter));
                    if (ret <= 0)
                    {
                        perror("read error");
                    }
                    printf("提示:%s\n", recvAfter);
                    sleep(2);
                    if (strcmp(recvAfter, NO_APPLY_NOW) != 0)
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
                    ret = read(sockfd, recvAfter, sizeof(recvAfter));
                    if (ret <= 0)
                    {
                        perror("read error");
                    }
                    printf("提示:%s\n", recvAfter);
                    break;
                }
                case SEND_MESSAGE:
                {
                    ret = dataBaseDisPlayFriend(signinedName);
                    if (ret != ON_SUCCESS)
                    {
                        printf("还没有好友,请添加好友\n");
                    }
                    else
                    {
                        /*输入你要聊天的对象，将登录用户,接收消息的人和消息打包成json发送给服务器处理*/
                        printf("输入你要聊天的对象:\n");
                        scanf("%s", chatFriendName);
                        /*读取消息*/
                        ChatIndividual persons;
                        persons.sockfd = sockfd;
                        strncpy(persons.recver, signinedName, sizeof(persons.recver) - 1);
                        strncpy(persons.sender, chatFriendName, sizeof(persons.sender) - 1);
                        privateChat(&persons);
                    }
                    break;
                }
                case CREATE_GROUP:
                {
                    createGroup(sockfd);
                    break;
                }
                case ADD_GROUP:
                {
                    addGroup(sockfd);
                    break;
                }
                case GROUP_CHAT:
                {
                    ret = dataBaseDisPlayGroup(signinedName);
                    if (ret != ON_SUCCESS)
                    {
                        printf("还没有群聊,请加入群聊\n");
                    }
                    else
                    {
                        /*输入你要聊天的对象，将登录用户,接收消息的人和消息打包成json发送给服务器处理*/
                        printf("输入你要聊天的群聊:\n");
                        scanf("%s", chatGroupName);
                        /*读取消息*/
                        GroupMember member;
                        member.sockfd = sockfd;
                        strncpy(member.signinedName, signinedName, sizeof(member.signinedName) - 1);
                        strncpy(member.groupName, chatGroupName, sizeof(member.groupName) - 1);
                        groupChat(&member);
                    }
                    break;
                }
                case EXIT:
                {
                    break;
                }
                default:
                    break;
                }
            }
        }
        sleep(1);
    }
    /*登录成功*/

    /* 休息5S */
    sleep(5);
    close(sockfd);
    balanceBinarySearchTreeDestroy(AVL);
    hashTableDestroy(hash);
    return 0;
}