#include <stdio.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <error.h>
#include <netinet/in.h>
#include <strings.h>

#define PORT 7777
#define LISTEN_MAX 128


/* 选择*/
enum
{
    EXIT,
    REGIST,
    LOGIN,
    ADD_FRIENDS,
    PRIVATE_CHAT,
    GROUP_CHAT,
};




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
    struct sockaddr_in clientaddr;
    bzero(&clientaddr, sizeof(clientaddr));
    socklen_t clientlen = sizeof(clientaddr);

    /* 连接  连接客户端的地址 */
    int acceptfd = accept(sockfd, (struct sockaddr *)&clientaddr, &clientlen);
    if (acceptfd == -1)
    {
        perror("accept error");
    }

    return 0;
}
