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
#define SERVER_PORT 8878
#define MAX_LISTEN 128
#define LOCAL_IPADDRESS "172.23.232.7"
#define BUFFER_SIZE 128
#define MINI_CAPACITY 5
#define MAX_CAPACITY 10
#define MAX_QUEUE_CA 50
#define EVENT_SIZE 1024
#define USER_SIZE 128
/*全局变量，方便捕捉信号后释放资源*/

ThreadPool *pool = NULL;
int sockfd;
BalanceBinarySearchTree *AVL = NULL;
sqlite3 *mydb = NULL;
pthread_mutex_t g_mutex;
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
typedef enum USER_OPTIONS
{
    REGISTER = 1,
    LOGIN = 2,
} USER_OPTIONS;
/*线程处理函数*/

/* 二叉搜索树的比较函数 */
int compareFunc(void *arg1, void *arg2)
{
    USER val1 = *(USER *)arg1;
    USER val2 = *(USER *)arg2;
    printf("val1:%s = val2:%s\n", val1.name, val2.name);
    size_t len1 = strlen(val1.name);
    size_t len2 = strlen(val2.name);
    return strncmp(val1.name, val2.name, len1 < len2 ? len1 : len2);
}

void *accept_handler(void *arg)
{
    Fdset *fdset = (Fdset *)arg;
    pthread_detach(pthread_self());
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
    event.events = EPOLLIN;
    epoll_ctl(fdset->epollfd, EPOLL_CTL_ADD, fdset->acceptfd, &event);
    pthread_mutex_unlock(&g_mutex);
}

void *communicate_handler(void *arg)
{
    pthread_detach(pthread_self());
    Fdset *fdset = (Fdset *)arg;
    /*接收缓冲区*/
    char recvbuffer[BUFFER_SIZE];
    memset(recvbuffer, 0, sizeof(recvbuffer));

    /*发送缓冲区*/
    char sendBuffer[BUFFER_SIZE];
    memset(sendBuffer, 0, sizeof(sendBuffer));
    int readBytes = 0;
    /*解析json对象*/
    struct json_object *parseObj = calloc(1, sizeof(parseObj));
    parseObj = json_tokener_parse(recvbuffer);
    /*命令变量*/
    int demand = 0;
    /*判断函数返回值*/
    int ret = 0;
    USER *currentUser = calloc(1, sizeof(USER));
    readBytes = read(fdset->acceptfd, (void *)&recvbuffer, sizeof(recvbuffer));
    if (readBytes < 0)
    {
        perror("read eror");
        /* 从epoll的红黑树上删除通信结点 */
        pthread_mutex_lock(&g_mutex);
        epoll_ctl(fdset->epollfd, EPOLL_CTL_DEL, fdset->acceptfd, NULL);
        pthread_mutex_unlock(&g_mutex);
        close(fdset->acceptfd);
    }
    else if (readBytes == 0)
    {
        printf("客户端下线了...\n");
        /* 从epoll的红黑树上删除通信结点 */
        pthread_mutex_lock(&g_mutex);
        epoll_ctl(fdset->epollfd, EPOLL_CTL_DEL, fdset->acceptfd, NULL);
        pthread_mutex_unlock(&g_mutex);
        /*将数据库的上线状态改为下线状态*/
        pthread_mutex_lock(&g_mutex);
        ret = dataBaseUpdateOnlineStatus(parseObj);
        pthread_mutex_unlock(&g_mutex);
        if (ret != ON_SUCCESS)
        {
            printf("dataBaseUpdateOnlineStatus error\n");
        }
        sleep(2);
        close(fdset->acceptfd);
    }
    else
    {

        /*接受消息*/
        if (json_object_get_int(json_object_object_get(parseObj, "choices")) == REGISTER)
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
                json_object_put(parseObj); // 释放 parseObj
                sleep(2);
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
                json_object_put(parseObj); // 释放 parseObj
                sleep(2);
            }
        }
        /*功能在if中加*/
        else if (json_object_get_int(json_object_object_get(parseObj, "choices")) == LOGIN)
        {
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
                USER *nodeUser = (USER *)matchNode->data;
                size_t len1 = strlen(nodeUser->password);
                size_t len2 = strlen(currentUser->password);
                if (strncmp(nodeUser->password, currentUser->password, len1 < len2 ? len1 : len2) == 0)
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
                    ret = dataBaseFriendInsert(nodeUser->name, NULL, fdset->acceptfd, 1, NULL);
                    pthread_mutex_unlock(&g_mutex);
                    if (ret != ON_SUCCESS)
                    {
                        printf("dataBaseFriendInsert error\n");
                    }
                    json_object_put(parseObj); // 释放 parseObj
                    sleep(2);
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
                    json_object_put(parseObj); // 释放 parseObj
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
                json_object_put(parseObj); // 释放 parseObj
                sleep(2);
            }
        }
        else if (1)
        {
        }
    }

    pthread_exit(NULL);
}
void signal_handler(int sig)
{
    /*销毁线程成*/
    poolDestroy(pool);
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
int main()
{
    /*初始化树，数据库，线程池，锁*/
    balanceBinarySearchTreeInit(&AVL, compareFunc, printStructData);
    /*将数据库中的信息存入内存*/
    dataBaseToMemory(mydb, AVL);
    /*初始化数据库*/
    dataBaseInit(&mydb);
    /*初始化线程池*/
    poolInit(&pool, MINI_CAPACITY, MAX_CAPACITY, MAX_QUEUE_CA);
    /*初始化锁资源*/
    pthread_mutex_init(&g_mutex, NULL);
    /*层序遍历树，打印信息*/
    balanceBinarySearchTreeLevelOrderTravel(AVL);
    sleep(2);
    /*捕捉退出信号*/
    signal(SIGINT, signal_handler);

    /* 创建socket套接字 */
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd == -1)
    {
        perror("socket error");
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

    /*设置端口复用*/
    int enableopt = 1;
    int ret = setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, (void *)&enableopt, sizeof(enableopt));
    if (ret == -1)
    {
        perror("setsockopt error");
        exit(-1);
    }
    ret = bind(sockfd, (struct sockaddr *)&localAddress, sizeof(localAddress));
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

    /* 客户的信息 */
    struct sockaddr_in clientAddress;
    memset(&clientAddress, 0, sizeof(clientAddress));
    socklen_t clientAddressLen = 0;
    int acceptfd = 0;
    /*循环去接收客服请求*/
    while (1)
    {

        int num = epoll_wait(epfd, events, maxEventSize, -1);
        if (num == -1)
        {
            perror("epoll wait error");
            exit(-1);
        }
        /* 程序到这里一定有通信 */
        printf("num = %d\n", num);

        for (int idx = 0; idx < num; idx++)
        {

            int fd = events[idx].data.fd;
            if (fd == sockfd)
            {
                Fdset *fdset = calloc(1, sizeof(Fdset));
                fdset->sockfd = fd;
                fdset->epollfd = epfd;
                poolAdd(pool, accept_handler, (void *)fdset);
            }
            else
            {
                Fdset *fdset = calloc(1, sizeof(Fdset));
                fdset->acceptfd = fd;
                fdset->sockfd = sockfd;
                fdset->epollfd = epfd;
                poolAdd(pool, communicate_handler, (void *)fdset);
            }
        }
    }

    return 0;
}