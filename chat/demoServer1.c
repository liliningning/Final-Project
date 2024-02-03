#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <string.h>
#include <strings.h>
#include <sys/select.h>
#include <sys/time.h>
#include <sys/epoll.h>
#include <unistd.h>
#include <time.h>
#include <ctype.h>
#include <pthread.h>
#if 1
#include "modelThreadPool.h"
#endif

#define SERVER_PORT 8099
#define MAX_LISTEN 128
#define LOCAL_IPADDRESS "127.0.0.1"
#define BUFFER_SIZE 128


/* 互斥锁 */
pthread_mutex_t mutex;

void *accept_func(void *arg)
{
    /* 设置线程分离 */
    pthread_detach(pthread_self());

    
}

void * comm_func(void *arg)
{
    /* 设置线程分离 */
    pthread_detach(pthread_self());

    /* 通信句柄. */
    int acceptfd = *(int *)arg;

    /* 通信 */

    /* 接收缓冲区 */
    char recvbuffer[BUFFER_SIZE];
    memset(recvbuffer, 0, sizeof(recvbuffer));

    /* 发送缓冲区 */
    char sendBuffer[BUFFER_SIZE];
    memset(sendBuffer, 0, sizeof(sendBuffer));

    /* 读取到的字节数 */
    int readBytes = 0;
    while (1)
    {
        readBytes = read(acceptfd, (void *)&recvbuffer, sizeof(recvbuffer));
        if (readBytes < 0)
        {
            // perror("read eror");
            printf("read error\n");
            close(acceptfd);
            break;
        }
        else if (readBytes == 0)
        {
            printf("read readBytes == 0\n");
            close(acceptfd);
            break;
        }
        else
        {
            /* 读到的字符串 */
            printf("buffer:%s\n", recvbuffer);

            write(acceptfd, sendBuffer, sizeof(sendBuffer));
        }    
    }

    /* 线程退出 */
    pthread_exit(NULL);
}




