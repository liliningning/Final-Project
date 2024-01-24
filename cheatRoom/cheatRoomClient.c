#include <stdio.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <error.h>
#include <netinet/in.h>
#include <strings.h>
#include <arpa/inet.h>

#define PORT 7777
#define SERVER_IP "172.16.104.91"

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
    int set = inet_pton(AF_INET, SERVER_IP, (void *)serveradder.sin_addr.s_addr);
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
    return 0;
}