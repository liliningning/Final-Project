#include <stdio.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <error.h>
#include <netinet/in.h>
#include <strings.h>
#include <sys/select.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>
#include <errno.h>

#define PORT 8888
#define LISTEN_MAX 128
#define BUFSIZE 128
#define GROUP_SIZE 8

/* 客户端套接字数组 */
int clent_td[GROUP_SIZE] = {0};

#if 0
/* 群发 */
void send_msg()
{
    char buf[BUFSIZE];
    bzero(&buf, sizeof(buf));

    for (int idx = 0; idx < GROUP_SIZE; idx++)
    {
        if (clent_td[idx] != -1)
        {
            printf(" buf %s\n", buf);
            write(clent_td[idx], buf, strlen(buf) - 1);
        }
    }
}
#endif
/* 接收客户端B 发送给客户端A */
void *client_hander(void *arg)
{
    pthread_detach(pthread_self());
    int num = *(int *)arg;
    /* 接收 */
    char recvbuf[BUFSIZE];
    memset(recvbuf, 0, sizeof(recvbuf));
    while (1)
    {
        /* 接收客户端B发送的数据 */
        int readByte = recv(clent_td[1], recvbuf, sizeof(recvbuf), 0);
        if (readByte == 0)
        {
            perror("read error");
            break;
        }
        /* 向客户端B发送数据 */
        send(clent_td[0], recvbuf, sizeof(recvbuf), 0);
    }
}


int main()
{

    /* 创建套接字 */
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd == -1)
    {
        perror("socket  create  error");
        exit(-1);
    }

    struct sockaddr_in localsockaddr;
    /* 清除脏数据 */
    bzero(&localsockaddr, sizeof(localsockaddr));

    localsockaddr.sin_family = AF_INET;
    localsockaddr.sin_port = htons(PORT);
    localsockaddr.sin_addr.s_addr = htonl(INADDR_ANY);

    int enableOpt = 1;
    /* 端口复用 */
    int ret = setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &enableOpt, sizeof(enableOpt));
    if (ret == -1)
    {
        perror(" setsockopt error");
        exit(-1);
    }

    /* 绑定 */
    socklen_t len = sizeof(localsockaddr);
    ret = bind(sockfd, (struct sockaddr *)&localsockaddr, len);
    if (ret == -1)
    {
        perror("bind  error ");
        exit(-1);
    }

    /* 监听 */
    ret = listen(sockfd, LISTEN_MAX);
    if (ret == -1)
    {
        perror("listen error ");
        exit(-1);
    }
    /* 创建客户端地址 */
    struct sockaddr_in clientaddr;
    bzero(&clientaddr, sizeof(clientaddr));
    socklen_t clientlen = sizeof(clientaddr);

    int idx = 0;
    for (idx = 0; idx < clientlen; idx++)
    {
        clent_td[idx] = accept(sockfd, (struct sockaddr *)&clientaddr, &clientlen);
        if (clent_td[idx] == -1)
        {
            perror("accept error");
            exit(-1);
        }
        printf("客户端 %d 上线了\n", clent_td[idx]);
    }
    int num  = 0;
    /* 创建线程 */
    pthread_t tid;
    pthread_create(&tid, NULL, client_hander, (void *)&num);
    pthread_detach(tid);

    /* 接收客户端A发送的数据 */
    char recvbuf[BUFSIZE];
    memset(recvbuf, 0, sizeof(recvbuf));

    int readByte = read(clent_td[0], recvbuf, sizeof(recvbuf));
    if (readByte < 0)
    {
        perror("read error");
    }
    else if(readByte == 0)
    {
        printf("客户端 %d 已下线\n",clent_td[idx]);
    }
    else
    {
        write(clent_td[1],recvbuf, sizeof(recvbuf));
        printf("recvbuf %s\n",recvbuf);
    }
   

    close(sockfd);

    return 0;
}