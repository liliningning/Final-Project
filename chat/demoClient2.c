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
#include <ctype.h>
#include <fcntl.h>

#define SERVER_PORT 8236
#define SERVER_IP   "172.17.202.164"
#define BUFFER_SIZE 128

int main()
{
    int clntfd = socket(AF_INET, SOCK_STREAM, 0);
    if (clntfd == -1)
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
    ret = connect(clntfd, (struct sockaddr *)&serverAddress, sizeof(serverAddress));
    if (ret == -1)
    {
        perror("connect error");
        exit(-1);
    }

    /* 写缓冲区 */
    char writeBuffer[BUFFER_SIZE];
    memset(writeBuffer, 0, sizeof(writeBuffer));

    /* 读缓冲区 */
    char recvBuffer[BUFFER_SIZE];
    memset(recvBuffer, 0, sizeof(recvBuffer));

    int readBytes = 0;
    while (1)
    {
        printf("output:");
        scanf("%s", recvBuffer);

        // readBytes = read(clntfd, buffer, sizeof(buffer) - 1);

        int flag = fcntl(clntfd, F_GETFL);
        flag |= O_NONBLOCK;
        fcntl(clntfd, F_SETFL, flag);

        readBytes = read(clntfd, recvBuffer, sizeof(recvBuffer) - 1);
        if (readBytes < 0)
        {
            perror("read error");
            //exit(-1);
        }
        else if (readBytes == 0)
        {
            printf("readBytes == 0\n");
        }
        else
        {
            printf("recv:%s\n", recvBuffer);
        }
    }
    
    
    /* 休息5S */
    sleep(5);

    
    close(clntfd);

    return 0;
}