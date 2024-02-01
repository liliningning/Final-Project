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
#include <json-c/json.h>
#include <json-c/json_object.h>


#define SERVER_PORT 8888
#define SERVER_IP "172.16.104.91"
#define BUFFER_SIZE 128

typedef enum USER_OPTIONS
{
    REGISTER = 1,
    LOGIN = 2,
} USER_OPTIONS;



typedef enum USER_TWO
{
    ADD_FRIEND,
    FRIEND_APPLICATION,
    DELETE_FRIREND,
    SEND_MESSAGE,

} USER_TWO;

/* 主页面 */
void onemenu()
{
    printf("\t-------------------------------------\n");
    printf("\t|                                   |\n");
    printf("\t|    1:注册                         |\n");
    printf("\t|    2:登录                         |\n");
    printf("\t|    0:退出                         |\n");
    printf("\t|                                   |\n");
    printf("\t-------------------------------------\n");
}





/* 二级聊天室 登录成功后跳转 */
void  twomenu()
{
    printf("\t-------------------------------------\n");
    printf("\t|                                   |\n");
    printf("\t|    1:添加好友                         |\n");
    printf("\t|    2:好友请求                         |\n");
    printf("\t|    3:删除好友                         |\n");
    printf("\t|    4:给好友发送消息                         |\n");
    printf("\t|                                   |\n");
    printf("\t-------------------------------------\n");
}



/* 注册 */
static int clientRegister(int sockfd)
{
    int ret = 0;
    int demand = REGISTER;

    struct json_object *registerObj = json_object_new_object();

    char accountNumber[BUFFER_SIZE];
    memset(accountNumber, 0, sizeof(memset));
    char *passwordNumber = calloc(BUFFER_SIZE, sizeof(char));
    printf("请输入账号\n");
    scanf("%s", accountNumber);
    while (getchar() != '\n')
        ;
    /*账号查重todo...*/
    /*备份passwordNumber*/
    char *accord = passwordNumber;
    int checkout = 0;
    while (checkout == 0)
    {
        /*密码要求*/
        printf("请输入密码(密码不少于8位且必须包含字符和数字):\n");
        scanf("%s", passwordNumber);
        while (getchar() != '\n')
            ;
        int numCount = 0;
        int characterCount = 0;
        while (*passwordNumber != '\0')
        {
            if (*passwordNumber >= '0' && *passwordNumber <= '9')
            {
                numCount++;
            }
            else
            {
                characterCount++;
            }
            passwordNumber++;
        }
        if (numCount + characterCount < 8 || numCount == 0 || characterCount == 0)
        {
            printf("密码不符合要求，请重新输入\n");
        }
        else
        {
            checkout = 1;
        }
    }
    printf("check-----------------------\n");
    json_object_object_add(registerObj, "choices", json_object_new_int(demand));
    json_object_object_add(registerObj, "account", json_object_new_string(accountNumber));
    json_object_object_add(registerObj, "password", json_object_new_string(accord));
    const char *sendStr = json_object_to_json_string(registerObj);
    int len = strlen(sendStr);

    /*将json对象转化为字符串发给服务器*/
    int retWrite = write(sockfd, sendStr, len + 1);
    if (retWrite == -1)
    {
        perror("write error");
    }

    return ret;
}

 /* 登录 */
 int  clientLogin(int sockfd)
{
    int ret = 0;
    int demand = LOGIN;  
    struct json_object *registerObj = json_object_new_object();
    /* 存用户名 */
    char accountNumber[BUFFER_SIZE];
    memset(accountNumber, 0, sizeof(accountNumber));
    /* 存密码 */
    char *passwordNumber = calloc(BUFFER_SIZE, sizeof(char));  
    char *accord = passwordNumber;

    printf("登录\n");
    printf("\n");

    printf("请输入账户:");
    scanf("%s", accountNumber);
    printf("请输入密码：");
    scanf("%s", passwordNumber);

    json_object_object_add(registerObj, "choices", json_object_new_int(demand));
    json_object_object_add(registerObj, "account", json_object_new_string(accountNumber));
    json_object_object_add(registerObj, "password", json_object_new_string(accord));
    const char *sendStr = json_object_to_json_string(registerObj);

    int len = strlen(sendStr);
    /*将json对象转化为字符串发给服务器*/
    int retWrite = write(sockfd, sendStr, len + 1);
    if (retWrite == -1)
    {
        perror("write error");
    }
    return ret;
}


void addFrined(int sockfd)
{

}





int main()
{
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd == -1)
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

     /*设置端口复用*/
    int enableopt = 1;
    int set = setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, (void *)&enableopt, sizeof(enableopt));
    if (set == -1)
    {
        perror("setsockopt error");
        exit(-1);
    }

    /* ip地址 */
    ret = connect(sockfd, (struct sockaddr *)&serverAddress, sizeof(serverAddress));
    if (ret == -1)
    {
        perror("connect error");
        exit(-1);
    }
    /* 接收 */
    char recvBuffer[BUFFER_SIZE];
    memset(recvBuffer, 0, sizeof(recvBuffer));

    /*发送*/
    char sendBuffer[BUFFER_SIZE];
    memset(recvBuffer, 0, sizeof(recvBuffer));


    int choices;
    while (1)
    {
#if 1
        onemenu();
        printf("请输入选项:\n");
        scanf("%d", &choices);
        /*去除行缓存*/
        while (getchar() != '\n')
            ;
        switch (choices)
        {
        case REGISTER:
        {
            clientRegister(sockfd);
            sleep(2);
            ret = read(sockfd, recvBuffer, sizeof(recvBuffer) - 1);
            if (ret == -1)
            {
                perror("read error");
            }
            printf("提示:%s\n", recvBuffer);
            break;
        }
        case LOGIN:
        {
            clientLogin(sockfd);
            sleep(2);
            ret = read(sockfd, recvBuffer, sizeof(recvBuffer) - 1);
            if (ret == -1)
            {
                perror("read error");
            }
            printf("提示:%s\n", recvBuffer);
            break;
        }
        default:
        {
            break;
        }
            
        }

        // read(sockfd, recvBuffer, sizeof(recvBuffer) - 1);
        // printf("server massage:%s\n", recvBuffer);

        sleep(1);
#endif
    }

      /*登录成功*/
    while (1)
    {   twomenu();
        printf("请输入选项:\n");
        scanf("%d", &choices);
        /*去除行缓存*/
        while (getchar() != '\n')
            ;
        switch (choices)
        {
        case ADD_FRIEND:
        {
            /* 给服务器发送消息 然后服务器进行处理 */
            strncpy(sendBuffer, "添加好友", sizeof(sendBuffer));
            int retWrite = write(sockfd, sendBuffer, sizeof(sendBuffer) - 1);
            if(retWrite == -1)
            {
                perror("write error");
            }
            /* 服务器回的消息 */
            ret = read(sockfd, recvBuffer, sizeof(recvBuffer) - 1);
            if (ret == -1)
            {
                perror("read error");
            }
            printf("提示:%s\n", recvBuffer);
            break;
        }
        case FRIEND_APPLICATION:
        {
            strncpy(sendBuffer, "你好 可以加个好友吗", sizeof(sendBuffer) - 1);
            int  retWrite = write(sockfd, sendBuffer, sizeof(sendBuffer) - 1);
            if(retWrite == -1)
            {
                perror("write error");
            }
            /* 服务器回的消息 */
            ret = read(sockfd, recvBuffer, sizeof(recvBuffer) - 1);
            if (ret == -1)
            {
                perror("read error");
            }
            printf("提示:%s\n", recvBuffer);

            break;
        }
        case DELETE_FRIREND:
        {
            strncpy(sendBuffer, "我们的关系，就到此为止吧！", sizeof(sendBuffer) - 1);
            int  retWrite = write(sockfd, sendBuffer, sizeof(sendBuffer) - 1);
            if(retWrite == -1)
            {
                perror("write error");
            }
            /* 服务器回的消息 */
            ret = read(sockfd, recvBuffer, sizeof(recvBuffer) - 1);
            if (ret == -1)
            {
                perror("read error");
            }
            printf("提示:%s\n", recvBuffer);
            break;
        }
        case SEND_MESSAGE:
        {
            break;
        }
        default:
            break;
        }
    }

    /* 休息5S */
    sleep(5);

    close(sockfd);

    return 0;
}