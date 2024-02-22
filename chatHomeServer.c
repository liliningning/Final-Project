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
#include <pthread.h>
#include <sqlite3.h>
#include "modelThreadPool.h"
#include <json-c/json.h>
#include <json-c/json_object.h>
#include <sys/epoll.h>
#include "dataBase.h"
#include <signal.h>
#include <string.h>
#include <stddef.h>
#include "balanceBinarySearchTree.h"
#include <stdatomic.h>
#include <stdbool.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include "hashtable.h"
#include "globalVary.h"
#include "privateMsgHash.h"
#define SERVER_PORT 8850
#define MAX_LISTEN 128
#define LOCAL_IPADDRESS "172.23.232.7"
#define BUFFER_SIZE 128
#define MINI_CAPACITY 5
#define MAX_CAPACITY 10
#define MAX_QUEUE_CA 50
#define EVENT_SIZE 1024
#define USER_SIZE 128
#define REGISTER "a"
#define LOGIN "b"
#define SLOTNUMS 37
MsgHash *Hash = NULL;
typedef enum AFTER_LOGIN
{
    ADD_FRIEND = 1,
    HANDLE_APPLICATION = 2,
    DELETE_FRIREND = 3,
    SEND_MESSAGE = 4,
} AFTER_LOGIN;
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

typedef struct Fdset
{
    int acceptfd;
    int epollfd;
    int sockfd;
} Fdset;

/*线程处理函数*/

/* 二叉搜索树的比较函数 */
int compareFunc(void *arg1, void *arg2)
{
    USER val1 = *(USER *)arg1;
    USER val2 = *(USER *)arg2;
    size_t len1 = strlen(val1.name);
    size_t len2 = strlen(val2.name);
    return strcmp(val1.name, val2.name);
}

void *accept_handler(void *arg)
{
    pthread_detach(pthread_self());

    int ret = 0;
    Fdset *fdset = (Fdset *)arg;

    fdset->acceptfd = accept(fdset->sockfd, NULL, NULL);
    if (fdset->acceptfd == -1)
    {
        perror("accpet error");
        exit(-1);
    }
    /* 将通信句柄放到epoll的红黑树上 */
    struct epoll_event event;
    pthread_mutex_lock(&g_mutex);
    event.data.fd = fdset->acceptfd;
    event.events = EPOLLIN | EPOLLOUT;
    ret = epoll_ctl(fdset->epollfd, EPOLL_CTL_ADD, fdset->acceptfd, &event);
    pthread_mutex_unlock(&g_mutex);
    if (ret == -1)
    {
        perror("epoll_ctl error");
    }

    /* 释放堆空间 */
    if (fdset)
    {
        free(fdset);
        fdset = NULL;
    }
}
/* 服务端处理客户端私聊消息 */
int dealPrivateChat(int acceptfd, char *user, char *Friend, MsgHash *msgHash, pthread_mutex_t *Hash_Mutx)
{
    //"!@#$%^*&^%$#@!_^@%#$#!"  取消息交流
    //"!@%^&$@(#^)!@*+@$#$@"  停止读
    char sendBuf[BUFFER_SIZE];
    char recvBuf[BUFFER_SIZE];
    while (1)
    {
        sleep(1);
        memset(sendBuf, 0, sizeof(sendBuf));
        memset(recvBuf, 0, sizeof(recvBuf));
        read(acceptfd, recvBuf, sizeof(recvBuf));
        printf("read = %s\n", recvBuf);
        if (strncmp(recvBuf, "fetchMsg", strlen("fetchMsg")) == 0)
        {

            /* 去消息队列中取接收者是客户端和发送者是指定好友的消息，发回给客户端 */
            /* 上锁 */
            pthread_mutex_lock(Hash_Mutx);
            int ret = hashMsgGet(msgHash, Friend, user, sendBuf);
            pthread_mutex_unlock(Hash_Mutx);
            if (ret == ON_SUCCESS)
            {
                // 将取出的消息给客户端
                write(acceptfd, sendBuf, sizeof(sendBuf));
            }
            else
            {
                /* 告诉客户端无消息 */
                strncpy(sendBuf, "fetchMsg", strlen("fetchMsg")); /*无消息*/
                write(acceptfd, sendBuf, sizeof(sendBuf));
            }
        }
        else if (strncmp(recvBuf, "stopRead", strlen("stopRead")) == 0)
        {
            /* 停止读 */
            break;
        }
        else
        {
            printf("%s:%s\n", user, recvBuf);
            /* 聊天消息 */
            /* 往消息队列中存放消息，接收者为聊天对象 */
            /* 上锁 */
            pthread_mutex_lock(Hash_Mutx);
            hashMsgInsert(msgHash, user, Friend, recvBuf);
            pthread_mutex_unlock(Hash_Mutx);
        }
    }
    return 0;
}
void *communicate_handler(void *arg)
{ /*判断函数返回值*/
    int ret = 0;
    pthread_detach(pthread_self());

    printf("=======================communication=========================\n");
    Fdset *fdset = (Fdset *)arg;
    /*接收缓冲区*/
    char recvbuffer[BUFFER_SIZE];
    memset(recvbuffer, 0, sizeof(recvbuffer));

    /*发送缓冲区*/
    char sendBuffer[BUFFER_SIZE];
    memset(sendBuffer, 0, sizeof(sendBuffer));
    int readBytes = 0;

    /*命令变量*/
    char *demand = calloc(BUFFER_SIZE, sizeof(char));
    USER *currentUser = calloc(1, sizeof(USER));
    USER *nodeUser = NULL;

    while (1)
    {
        /*读取数据*/
        readBytes = read(fdset->acceptfd, (void *)&recvbuffer, sizeof(recvbuffer));
        printf("%s\n", recvbuffer);
        struct json_object *parseObj = calloc(1, sizeof(parseObj));
        /*解析json对象*/
        parseObj = json_tokener_parse(recvbuffer);
        if (readBytes < 0)
        {
            perror("read eror");
            /* 从epoll的红黑树上删除通信结点 */
            pthread_mutex_lock(&g_mutex);
            ret = epoll_ctl(fdset->epollfd, EPOLL_CTL_DEL, fdset->acceptfd, NULL);
            if (ret == -1)
            {
                perror("epoll_ctl2 error");
                continue;
            }
            pthread_mutex_unlock(&g_mutex);
            close(fdset->acceptfd);
        }
        else if (readBytes == 0)
        {
            /* 从epoll的红黑树上删除通信结点 */
            pthread_mutex_lock(&g_mutex);
            ret = epoll_ctl(fdset->epollfd, EPOLL_CTL_DEL, fdset->acceptfd, NULL);
            if (ret == -1)
            {
                perror("epoll_ctl3 error");
            }
            pthread_mutex_unlock(&g_mutex);
            /*将数据库的上线状态改为下线状态*/
            pthread_mutex_lock(&g_mutex);
            ret = dataBaseFriendOffline(parseObj);
            pthread_mutex_unlock(&g_mutex);
            if (ret != ON_SUCCESS)
            {
                printf("dataBaseUpdateOnlineStatus error\n");
            }
            printf("客户端下线了...\n");
            close(fdset->acceptfd);
        }
        else
        {
            printf("go body\n");
            /*接受消息*/
            if (!strcmp(json_object_get_string(json_object_object_get(parseObj, "choices")), REGISTER))
            {
                /*将解析对象的name和password放入currentUser中，方便后续调用树的查找接口*/
                strncpy(currentUser->name, json_object_get_string(json_object_object_get(parseObj, "account")), sizeof(currentUser->name) - 1);
                strncpy(currentUser->password, json_object_get_string(json_object_object_get(parseObj, "password")), sizeof(currentUser->password) - 1);
                /*检查用户名有无重复*/
                ret = balanceBinarySearchTreeIsContainAppointVal(AVL, (void *)currentUser);
                if (ret == ON_SUCCESS)
                {
                    /*有重复*/
                    strncpy(sendBuffer, "美名尚存,另寻他名", sizeof(sendBuffer) - 1);
                    int retwrite = write(fdset->acceptfd, sendBuffer, sizeof(sendBuffer) - 1);
                    if (retwrite == -1)
                    {
                        perror("write error");
                    }
                    sleep(1);
                }
                else
                {
                    /*无重复*/
                    /*插入数据树,上锁*/
                    pthread_mutex_lock(&g_mutex);
                    balanceBinarySearchTreeInsert(AVL, (void *)currentUser);
                    pthread_mutex_unlock(&g_mutex);
                    /*插入数据库,上锁*/
                    pthread_mutex_lock(&g_mutex);
                    ret = dataBaseUserInsert(parseObj);
                    pthread_mutex_unlock(&g_mutex);
                    if (ret != ON_SUCCESS)
                    {
                        printf("dataBaseUserInsert error\n");
                    }
                    strncpy(sendBuffer, "注册成功了，开始冲浪吧", sizeof(sendBuffer) - 1);
                    int retwrite = write(fdset->acceptfd, sendBuffer, sizeof(sendBuffer) - 1);
                    if (retwrite == -1)
                    {
                        perror("write error");
                    }
                    sleep(2);
                }
            }
            /*功能在if中加*/
            else if (!strcmp(json_object_get_string(json_object_object_get(parseObj, "choices")), LOGIN))
            {
                printf("go login\n");
                /*将解析对象的name和password放入currentUser中，方便后续调用树的查找接口*/
                strncpy(currentUser->name, json_object_get_string(json_object_object_get(parseObj, "account")), sizeof(currentUser->name) - 1);
                strncpy(currentUser->password, json_object_get_string(json_object_object_get(parseObj, "password")), sizeof(currentUser->password) - 1);
                // /*查找该用户的密码是否与对应密码匹配*/
                // ret = balanceBinarySearchTreeIsContainAppointVal(AVL, (void *)currentUser);
                AVLTreeNode *matchNode = baseAppointValGetAVLTreeNode(AVL, (void *)currentUser);
                if (matchNode != NULL)
                {
                    /*若匹配结点不为空说明账号正确，此时确认密码是否正确*/
                    /*将matchNode中的data强转为USR型，否则strncmp会调用错误*/
                    nodeUser = (USER *)matchNode->data;
                    size_t len1 = strlen(nodeUser->password);
                    size_t len2 = strlen(currentUser->password);
                    if (strncmp(nodeUser->password, currentUser->password, sizeof(nodeUser->password) - 1) == 0)
                    {
                        /*如果匹配*/
                        strncpy(sendBuffer, "登陆成功", sizeof(sendBuffer) - 1);
                        int retwrite = write(fdset->acceptfd, sendBuffer, sizeof(sendBuffer) - 1);
                        if (retwrite == -1)
                        {
                            perror("write error");
                        }

                        /*登录成功的人要加入数据库,上锁*/
                        pthread_mutex_lock(&g_mutex);
                        ret = dataBaseFriendOnline(nodeUser->name);
                        pthread_mutex_unlock(&g_mutex);

                        if (ret != ON_SUCCESS)
                        {
                            printf("dataBaseFriendInsert error\n");
                        }
                    }
                    else
                    {
                        /*密码不匹配，重新登录*/
                        strncpy(sendBuffer, "密码不正确,请重新登陆", sizeof(sendBuffer) - 1);
                        int retwrite = write(fdset->acceptfd, sendBuffer, sizeof(sendBuffer) - 1);
                        if (retwrite == -1)
                        {
                            perror("write error");
                        }
                        sleep(2);
                    }
                }
                else
                {
                    /*如果匹配结点为空则说明输入的账户不存在*/
                    strncpy(sendBuffer, "用户不存在,请重新输入或者注册", sizeof(sendBuffer) - 1);
                    int retwrite = write(fdset->acceptfd, sendBuffer, sizeof(sendBuffer) - 1);
                    if (retwrite == -1)
                    {
                        perror("write error");
                    }
                    sleep(1);
                }
                printf("login over\n");
            }
            else if (json_object_get_int(json_object_object_get(parseObj, "options")) == ADD_FRIEND)
            {
                /*给传入进来的name发送好友请求消息*/
                ret = dataBaseTakeApplyToName(parseObj, nodeUser->name);
                if (ret != ON_SUCCESS)
                {
                    printf("dataBaseTakeApplyToName error\n");
                }
                strncpy(sendBuffer, "已发送好友申请", sizeof(sendBuffer) - 1);
                write(fdset->acceptfd, sendBuffer, sizeof(sendBuffer));
            }
            else if (json_object_get_int(json_object_object_get(parseObj, "options")) == HANDLE_APPLICATION)
            {

                char *friendName = dataBaseFindApplyFriendName(nodeUser->name);

                if (friendName != NULL)
                {
                    printf("%s\n", friendName);
                    sprintf(sendBuffer, "'%s'向你发送好友申请", friendName);
                    /*发给qwe,让qwe的数据库private上有接受或者拒绝的信息*/
                    write(fdset->acceptfd, sendBuffer, sizeof(sendBuffer));
                    int status = 0;
                    ret = read(fdset->acceptfd, &status, sizeof(status));
                    if (ret < 0)
                    {
                        perror("read error");
                    }
                    else if (ret == 0)
                    {
                        printf("客户端下线\n");
                    }
                    else
                    {
                        handleApply(status, nodeUser->name);
                    }
                }
                else
                {
                    strncpy(sendBuffer, "暂时没有好友申请", sizeof(sendBuffer) - 1);
                    /*发给qwe,让qwe的数据库private上有接受或者拒绝的信息*/
                    write(fdset->acceptfd, sendBuffer, sizeof(sendBuffer));
                }
            }
            else if (json_object_get_int(json_object_object_get(parseObj, "options")) == DELETE_FRIREND)
            {
                /* 将数据库中的name和friendName的好友状态全部设置为0*/
                ret = dataBaseDeleteFriend(parseObj, nodeUser->name);
                if (ret != ON_SUCCESS)
                {
                    strncpy(sendBuffer, "该用户不是你的好友", sizeof(sendBuffer) - 1);
                    write(fdset->acceptfd, sendBuffer, sizeof(sendBuffer));
                }
                else
                {
                    strncpy(sendBuffer, "删除成功", sizeof(sendBuffer) - 1);
                    write(fdset->acceptfd, sendBuffer, sizeof(sendBuffer));
                }
            }
            else if (json_object_get_int(json_object_object_get(parseObj, "options")) == SEND_MESSAGE)
            {
                printf("go sendMessage\n");
#if 0
                /*将哈希表中的取信息传入客户端*/
                while (1)
                {
                    read;
                    /*判断客户端发过来的消息如果不是*/
                    write;
                }

                /*将客户端传入过来的信息存入哈希表*/
                struct json_object *loginedNameVal = json_object_object_get(parseObj, "loginedName");
                struct json_object *chatFriendNameVal = json_object_object_get(parseObj, "chatFriendName");
                struct json_object *messageVal = json_object_object_get(parseObj, "message");
                MessagePackage messageSet;
                memset(&messageSet, 0, sizeof(messageSet));
                strncpy(messageSet.message, json_object_get_string(messageVal), sizeof(messageSet.message) - 1);
                strncpy(messageSet.sender, json_object_get_string(loginedNameVal), sizeof(messageSet.sender) - 1);
                char chatFriendName[BUFFER_SIZE] = {0};
                strncpy(chatFriendName, json_object_get_string(chatFriendNameVal), sizeof(chatFriendNameVal) - 1);
                hashTableInsert(hash, chatFriendName, messageSet);
            }

#endif
                /*处理消息*/
                struct json_object *loginedNameVal = json_object_object_get(parseObj, "loginedName");
                struct json_object *chatFriendNameVal = json_object_object_get(parseObj, "chatFriendName");
                // struct json_object *messageVal = json_object_object_get(parseObj, "message");
                pthread_mutex_t hash_mutex;
                char loginedName[BUFFER_SIZE] = {0};
                strncpy(loginedName, json_object_get_string(loginedNameVal), sizeof(loginedName) - 1);
                char chatFriendName[BUFFER_SIZE] = {0};
                strncpy(chatFriendName, json_object_get_string(chatFriendNameVal), sizeof(chatFriendName) - 1);
                dealPrivateChat(fdset->acceptfd, loginedName, chatFriendName, Hash, &hash_mutex);
            }
        }
        json_object_put(parseObj);
    }

    /* 释放堆空间 */
    if (fdset)
    {
        free(fdset);
        fdset = NULL;
    }
    if (demand != NULL)
    {
        free(demand);
        demand = NULL;
    }
    if (currentUser != NULL)
    {
        free(currentUser);
        currentUser = NULL;
    }
    pthread_exit(NULL);
}
void signal_handler(int sig)
{
    hashTableDestroy(hash);
    /*销毁线程池*/
    threadPoolDestroy(pool);
    /*关闭文件描述符*/
    close(sockfd);
    /*销毁AVL*/
    balanceBinarySearchTreeDestroy(AVL);
    /*释放锁资源*/
    pthread_mutex_destroy(&g_mutex);
    printf("获取中断信号,释放资源退出服务器...\n");
    sleep(2);
    exit(-1);
}
int dataBaseToMemory(sqlite3 *db, BalanceBinarySearchTree *avl)
{

    sqlite3_open("chatBase.db", &db);
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
/* 打印数据 */
int printStructData(void *arg)
{
    int ret = 0;
    USER val = *(USER *)arg;
    printf("val.name:%s\tval.password:%s\t", val.name, val.password);
    return ret;
}
int compareHash(ELEMENTTYPE arg1, ELEMENTTYPE arg2)
{
    hashNode node1 = *(hashNode *)arg1;
    hashNode node2 = *(hashNode *)arg2;
    return strcmp(node1.real_key, node2.real_key);
}

int main()
{

    HashInit(&Hash, 37);
    hashTableInit(&hash, SLOTNUMS, compareHash);
    /*初始化树，数据库，线程池，锁*/
    /*初始化树*/
    balanceBinarySearchTreeInit(&AVL, compareFunc, printStructData);
    /*将数据库中的信息存入内存*/
    dataBaseToMemory(mydb, AVL);
    /*初始化数据库*/
    dataBaseInit(&mydb);
    /*初始化线程池*/
    pool = threadPoolCreate(MINI_CAPACITY, MAX_CAPACITY, MAX_QUEUE_CA);
    /*初始化锁资源*/
    pthread_mutex_init(&g_mutex, NULL);
    /*层序遍历树，打印信息*/
    balanceBinarySearchTreeLevelOrderTravel(AVL);
    /*捕捉退出信号*/
    signal(SIGINT, signal_handler);

    /* 创建socket套接字 */
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd == -1)
    {
        perror("socket error");
        exit(-1);
    }

    /* 设置端口复用 */
    int enableOpt = 1;
    int ret = setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, (void *)&enableOpt, sizeof(enableOpt));
    if (ret == -1)
    {
        perror("setsockopt error");
        exit(-1);
    }

    /* 绑定 */
#if 0
    /* 这个结构体不好用 */
    struct sockaddr localAddress;
#else
    struct sockaddr_in localAddress;
#endif
    /* 清除脏数据 */
    memset(&localAddress, 0, sizeof(localAddress));

    /* 地址族 */
    localAddress.sin_family = AF_INET;
    /* 端口需要转成大端 */
    localAddress.sin_port = htons(SERVER_PORT);
    /* ip地址需要转成大端 */

    /* Address to accept any incoming messages.  */
    /* INADDR_ANY = 0x00000000 */
    localAddress.sin_addr.s_addr = htonl(INADDR_ANY);

    int localAddressLen = sizeof(localAddress);
    ret = bind(sockfd, (struct sockaddr *)&localAddress, localAddressLen);
    if (ret == -1)
    {
        perror("bind error");
        exit(-1);
    }

    /* 监听 */
    ret = listen(sockfd, MAX_LISTEN);
    if (ret == -1)
    {
        perror("listen error");
        exit(-1);
    }

    /* 创建epoll 红黑树 */
    int epfd = epoll_create(1);
    if (epfd == -1)
    {
        perror("epoll create error");
        exit(-1);
    }

    /* 将sockfd 添加到监听事件中 */
    struct epoll_event event;
    memset(&event, 0, sizeof(event));
    event.data.fd = sockfd;
    event.events = EPOLLIN;

    ret = epoll_ctl(epfd, EPOLL_CTL_ADD, sockfd, &event);
    if (ret == -1)
    {
        perror("epoll_ctl error");
        exit(-1);
    }

    struct epoll_event events[EVENT_SIZE];
    /* 清除脏数据 */
    memset(events, 0, sizeof(events));
    int maxEventSize = sizeof(events) / sizeof(events[0]);
    while (1)
    {
        int num = epoll_wait(epfd, events, maxEventSize, -1);
        if (num == -1)
        {
            perror("epoll wait error");
            continue;
        }
        /* 程序到这里一定有通信 */
        // printf("num = %d\n", num);

        for (int idx = 0; idx < num; idx++)
        {
            int fd = events[idx].data.fd;
            if (fd == sockfd)
            {
#if 1
                printf("监听到客户端请求连接事件\n");
                int acceptfd = accept(sockfd, NULL, NULL);
                if (acceptfd == -1)
                {
                    perror("accpet error");
                    exit(-1);
                }
                /* 将通信句柄放到epoll的红黑树上 */
                // struct epoll_event event;
                // event.data.fd = acceptfd;
                // event.events = EPOLLIN | EPOLLOUT;
                // epoll_ctl(epfd, EPOLL_CTL_ADD, acceptfd, &event);
                Fdset *set = calloc(1, sizeof(Fdset));
                if (set == NULL)
                {
                    continue;
                }
                set->epollfd = epfd;
                set->acceptfd = acceptfd;
                printf("监听到读事件，添加一次服务端服务任务\n");
                threadPoolAdd(pool, communicate_handler, (void *)set);
#else

                Fdset *set = calloc(1, sizeof(Fdset));
                if (set == NULL)
                {
                    continue;
                }
                set->epollfd = epfd;
                set->sockfd = fd;
                threadPoolAdd(pool, accept_handler, (void *)set);
#endif
            }
            else
            {
                printf("监听到客户端读写事件\n");
            }
        }
    }

    return 0;
}
