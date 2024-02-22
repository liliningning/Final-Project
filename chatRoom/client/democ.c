#include <stdio.h>
#include <sys/types.h>          /* See NOTES */
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <string.h>
#include <pthread.h>

//实现双向通信	---线程
int fd;
void *show(void *arg){
    //接收
    int sockfd = *(int *)arg;
    char buf[50];
    while(1)
    {
	bzero(buf, sizeof(buf));
	recv(sockfd,buf,sizeof(buf),0);
	printf("服务器: %s \n",buf);
    }
}

int main(void)
{
    //1.建立socket连接
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);//IPv4
    if(sockfd < 0){
	perror("socket");
	return -1;
    }
    printf("sockfd = %d\n", sockfd);

    //2.绑定IP和端口号
    struct sockaddr_in server;
    server.sin_family = AF_INET;
    server.sin_port = htons(10086);
    server.sin_addr.s_addr = inet_addr("172.16.104.91");

    //3.主动连接
    if(connect(sockfd, (struct sockaddr *)&server, sizeof(server))){
	perror("socket");
	return -1;
    }

    //植入线程
    pthread_t tid;
    pthread_create(&tid, NULL, show, &sockfd);

    //5.TCP发送
    char buf[50];
    while(1)
    {
	bzero(buf, sizeof(buf));
	scanf("%s", buf);
	send(sockfd,buf,sizeof(buf),0);
    }

    return 0;
}