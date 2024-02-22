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
#define SERVER_PORT 7777
#define MAX_LISTEN 128
#define LOCAL_IPADDRESS "172.16.104.91"
#define BUFFER_SIZE 128
#define MINI_CAPACITY 5
#define MAX_CAPACITY 10
#define MAX_QUEUE_CA 50
#define EVENT_SIZE 1024
ThreadPool *pool = NULL;
int sockfd;
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

void *accept_handler(void *arg)
{
    Fdset *fdset = (Fdset *)arg;

    fdset->acceptfd = accept(fdset->sockfd, NULL, NULL);
    if (fdset->acceptfd == -1)
    {
        perror("accpet error");
        exit(-1);
    }
    /* 将通信句柄放到epoll的红黑树上 */
    struct epoll_event event;
    event.data.fd = fdset->acceptfd;
    event.events = EPOLLIN;
    epoll_ctl(fdset->epollfd, EPOLL_CTL_ADD, fdset->acceptfd, &event);
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
    int demand = 0;
    /*判断函数返回值*/
    int ret = 0;

    readBytes = read(fdset->acceptfd, (void *)&recvbuffer, sizeof(recvbuffer));
    if (readBytes < 0)
    {
        perror("read eror");
        /* 从epoll的红黑树上删除通信结点 */
        epoll_ctl(fdset->epollfd, EPOLL_CTL_DEL, fdset->acceptfd, NULL);
        close(fdset->acceptfd);
    }
    else if (readBytes == 0)
    {
        printf("客户端下线了...\n");
        /* 从epoll的红黑树上删除通信结点 */
        epoll_ctl(fdset->epollfd, EPOLL_CTL_DEL, fdset->acceptfd, NULL);
        close(fdset->acceptfd);
    }
    else
    {
        parseObj = json_tokener_parse(recvbuffer);
        /*接受消息*/
        if (json_object_get_int(json_object_object_get(parseObj, "choices")) == REGISTER)
        {
            printf("%s\n", recvbuffer);

            /*检查用户名有无重复*/
            ret = dataBaseDuplicateCheck(parseObj);
            if (ret != ON_SUCCESS)
            {
                /*有重复*/
                strncpy(sendBuffer, "美名尚存,另寻他名\n", sizeof(sendBuffer) - 1);
                int retwrite = write(fdset->acceptfd, sendBuffer, sizeof(sendBuffer) - 1);
                if (retwrite == -1)
                {
                    perror("write error");
                }
                sleep(2);
            }
            else
            {
                /*无重复*/
                ret = dataBaseUserInsert(parseObj);
                if (ret != ON_SUCCESS)
                {
                    printf("dataBaseUserInsert error\n");
                }
                strncpy(sendBuffer, "注册成功了，开始冲浪吧\n", sizeof(sendBuffer) - 1);
                int retwrite = write(fdset->acceptfd, sendBuffer, sizeof(sendBuffer) - 1);
                if (retwrite == -1)
                {
                    perror("write error");
                }
                sleep(2);
            }
        }
        else if (strncmp(recvbuffer, "778", strlen("778")) == 0)
        {
            
        }

        sleep(3);
    }

    pthread_exit(NULL);
}
void signal_handler(int sig)
{

    poolDestroy(pool);
    /*关闭文件描述符*/
    close(sockfd);
    printf("获取中断信号,退出服务器...\n");
    sleep(2);
    exit(-1);
}
int main()
{
    /*捕捉退出信号*/
    signal(SIGINT, signal_handler);
    dataBaseInit();
    /*初始化线程池*/
    poolInit(&pool, MINI_CAPACITY, MAX_CAPACITY, MAX_QUEUE_CA);

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