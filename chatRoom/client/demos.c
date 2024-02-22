#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>
#include <errno.h>


#define BUF_SIZE 128


//定义客户端结构体
struct sockaddr_in client;

int sockfd;
int arr_fd[BUF_SIZE];	//客户端文件描述符
char arr_ip[2][20];	//客户端IP数组

/*TCP接收客户端2数据发送客户端1*/
void *show(void *arg){
    char buf[50];
    char buff[30] = "对方客户端已关闭！\n";
    while(1)
	{
	bzero(buf,sizeof(buf));
	int ret = recv(arr_fd[1],buf,sizeof(buf),0); //接收客户端2发来的数据 
	if(ret == 0 || ret < 0)//接收方向是客户端2的arr_fd
	{
	    sleep(1);
	    close(sockfd);
	    send(arr_fd[0],buff,sizeof(buff),0);
	    printf("客户端 %s 已下线\n", arr_ip[1]);
	    break;
	}else if(ret < 0 || errno == EINTR || errno == EWOULDBLOCK || errno == EAGAIN){
	    continue;
	}
	send(arr_fd[0],buf,sizeof(buf),0);//向客户端1的arr_fd发送数据
	printf("客户端%s：%s\n",arr_ip[1],buf);
    }
}

int main(void)
{
    //建立socket连接
    sockfd=socket(AF_INET,SOCK_STREAM,0);//IPV4   TCP   0:前两个参生效
    if(sockfd<0){
	perror("socket");
	return -1;
    }
    printf("sockfd:%d\n",sockfd);    
    
    //声明服务器结构体
    struct sockaddr_in server;
    server.sin_family=AF_INET;//地址族  代表选择IPv4
    server.sin_port=htons(10086);//端口号 //因为有大小端序之分，所以要转换端序
    server.sin_addr.s_addr=inet_addr("172.16.104.91");//IP地址 点分十进制转化网络二进制
    
    //绑定IP和端口号
    if(bind(sockfd,(struct sockaddr *)&server,sizeof(server))){
	perror("bind");
	return -1;
    }

    //监听			
    listen(sockfd,8);

    //等待客户端连接
    int len = sizeof(client), i = 0;
    for(int i = 0; i < 2; i++){
	arr_fd[i] = accept(sockfd,(struct sockaddr *)&client,&len);
	if(arr_fd[i] < 0){
	    perror("accept");
	    return -1;
	}
	printf("客户%d上线\n",arr_fd[i]);
	strcpy(arr_ip[i],inet_ntoa(client.sin_addr));
	printf("其IP:%s PORT:%d\n",arr_ip[i],ntohs(client.sin_port));
    }

    //植入线程
	pthread_t tid2;
	pthread_create(&tid2,NULL,show,NULL);
	pthread_detach(tid2);//线程收尸

    //TCP接收客户端1数据发送客户端2
    char buf[50];
    char buff[30] = "对方客户端已关闭！\n";
    while(1){
	bzero(buf,sizeof(buf));
	int ret = recv(arr_fd[0],buf,sizeof(buf),0);
	if(ret == 0)//接收方向是客户端1的arr_fd
	{
	    sleep(1);
	    close(sockfd);
	    send(arr_fd[1], buff, sizeof(buff), 0);
	    printf("客户端 %s 已关闭\n", arr_ip[0]);
	    break;
	}else if(ret < 0 || errno == EINTR || errno == EWOULDBLOCK || errno == EAGAIN){
	    continue;
	}
	
	send(arr_fd[1],buf,sizeof(buf),0);//向客户端2的arr_fd发送数据
	printf("客户端%s:%s\n",arr_ip[0],buf);
    }

    return 0;
}