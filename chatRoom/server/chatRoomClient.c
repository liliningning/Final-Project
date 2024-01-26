#include <stdio.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <error.h>
#include <netinet/in.h>
#include <strings.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <string.h>
#include <unistd.h>
#define PORT 7777
#define SERVER_IP "172.16.104.91"
#define BUF_SIZE 128

void *client_hander(void *arg)
{

    int sockfd = *(int *)arg;
    char buf[BUF_SIZE];
    while (1)
    {
        memset(buf, 0, sizeof(buf));
        read(sockfd, buf, sizeof(buf));
        printf("服务器 %s\n", buf);
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

    struct sockaddr_in serveradder;
    /* 清除脏数据 */
    bzero(&serveradder, sizeof(serveradder));

    serveradder.sin_family = AF_INET;
    serveradder.sin_port = htons(PORT);

    /* 地址转换 */
    int set = inet_pton(AF_INET, SERVER_IP, (void *)&serveradder.sin_addr.s_addr);
    if (set == -1)
    {
        perror("inet_pton error");
        exit(-1);
    }
    int enableOpt = 1;
    /* 端口复用 */
    int ret = setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &enableOpt, sizeof(enableOpt));
    if (ret == -1)
    {
        perror(" setsockopt error");
        exit(-1);
    }

    int rets = connect(sockfd, (struct sockaddr *)&serveradder, sizeof(serveradder));
    if (rets == -1)
    {
        perror("connect error");
        exit(-1);
    }

    /* 创建一个线程来接收服务器发来的数据 */
    pthread_t tid;
    ret = pthread_create(&tid, NULL, client_hander, (void *)&sockfd);
    if (ret == -1)
    {
        perror("ptread create error");
        exit(-1);
    }
    char bufF[BUF_SIZE];
    memset(bufF, 0, sizeof(bufF));

    while (1)
    {

        printf("input:");
        scanf("%s", bufF);
        write(sockfd, bufF, sizeof(bufF));
    }

    return 0;
}