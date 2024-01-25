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
#define SERVER_PORT 8888
#define MAX_LISTEN 128
#define LOCAL_IPADDRESS "172.23.232.7"
#define BUFFER_SIZE 128
#define MINI_CAPACITY 5
#define MAX_CAPACITY 10
#define MAX_QUEUE_CA 50
// void sigHander(int sigNum)
// {
//     int ret = 0;
//     /* 资源回收 */
//     /* todo... */
// }
typedef enum USER_OPTIONS
{
    REGISTER,
} USER_OPTIONS;
/*线程处理函数*/

void *threadHandle(void *arg)
{
    pthread_detach(pthread_self());
    /*通讯句柄*/
    int acceptfd = *(int *)arg;
    /*通信*/

    /*接收缓冲区*/
    char recvbuffer[BUFFER_SIZE];
    memset(recvbuffer, 0, sizeof(recvbuffer));

    /*发送缓冲区*/
    char sendBuffer[BUFFER_SIZE];
    memset(sendBuffer, 0, sizeof(sendBuffer));
    int readBytes = 0;
    struct json_object *parseObj = calloc(1, sizeof(parseObj));
    int demand = 0;
    while (1)
    {

        readBytes = read(acceptfd, (void *)recvbuffer, sizeof(recvbuffer));
        printf("ok11\n");
        if (readBytes <= 0)
        {
            perror("read error");
            close(acceptfd);
            break;
        }
        else
        {

            /*接受消息*/
            if (json_object_get_int(json_object_object_get(parseObj, "choices")) == REGISTER)
            {
                /* sqlite3 *db;
                int sqliteRet = sqlite3_open("./chatdb", &db); // 打开数据库

                // 构建sq语句

                // 执行sql

                // 如果执行成功，给客户端发送回馈*/
                /*注册函数*/
                printf("%s\n", recvbuffer);
                parseObj = json_tokener_parse(recvbuffer);
                struct json_object *acountVal = json_object_object_get(parseObj, "account");

                printf("client massage=%s\n", json_object_get_string(acountVal));
            }
            else if (strncmp(recvbuffer, "778", strlen("778")) == 0)
            {
                strncpy(sendBuffer, "889", sizeof(sendBuffer) - 1);
                sleep(1);
                write(acceptfd, sendBuffer, sizeof(sendBuffer));
            }

            sleep(3);
        }
    }

    pthread_exit(NULL);
}

int main()
{

    /*初始化线程池*/
    ThreadPool *pool = NULL;
    poolInit(&pool, MINI_CAPACITY, MAX_CAPACITY, MAX_QUEUE_CA);
    /* 信号注册 */
    // signal(SIGINT, sigHander);
    // signal(SIGQUIT, sigHander);
    // signal(SIGTSTP, sigHander);

    /* 创建socket套接字 */
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
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

    /* 客户的信息 */
    struct sockaddr_in clientAddress;
    memset(&clientAddress, 0, sizeof(clientAddress));
    socklen_t clientAddressLen = 0;
    int acceptfd = 0;
    /*循环去接收客服请求*/
    while (1)
    {

        acceptfd = accept(sockfd, (struct sockaddr *)&clientAddress, &clientAddressLen);
        if (acceptfd == -1)
        {
            perror("accpet error");
            exit(-1);
        }

#if 0
        /*开一个线程去服务一个acceptfd*/
        pthread_t tid;
        ret = pthread_create(&tid, NULL, threadHandle, (void *)&acceptfd);
        if (ret != 0)
        {
            perror("pthread_create error");
            exit(-1);
        }
#endif
        /*将任务添加到任务队列*/
        poolAdd(pool, threadHandle, (void *)&acceptfd);
    }

    poolDestroy(pool);
    /*关闭文件描述符*/
    close(sockfd);
    return 0;
}