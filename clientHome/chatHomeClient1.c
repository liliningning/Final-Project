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
#define SERVER_PORT 8878
#define SERVER_IP "172.18.167.133"
#define BUFFER_SIZE 128
#define SUCCESS_LOGIN "登陆成功"
typedef enum USER_OPTIONS
{
    REGISTER = 1,
    LOGIN = 2,
} USER_OPTIONS;

typedef enum AFTER_LOGIN
{
    ADD_FRIEND = 1,
    FRIEND_APPLICATION = 2,
    DELETE_FRIREND = 3,
    SEND_MESSAGE = 4,
} AFTER_LOGIN;
static int clientLogIn(int sockfd)
{
    int ret = 0;
    int demand = LOGIN;

    struct json_object *registerObj = json_object_new_object();

    /*账号*/
    char accountNumber[BUFFER_SIZE];
    /*密码*/
    char *passwordNumber = calloc(BUFFER_SIZE, sizeof(char));
    printf("请输入账号\n");
    scanf("%s", accountNumber);
    while (getchar() != '\n')
        ;
    /*密码要求*/
    printf("请输入密码:\n");
    scanf("%s", passwordNumber);
    while (getchar() != '\n')
        ;
    printf("check whether valid------------ing-----------\n");
    json_object_object_add(registerObj, "choices", json_object_new_int(demand));
    json_object_object_add(registerObj, "account", json_object_new_string(accountNumber));
    json_object_object_add(registerObj, "password", json_object_new_string(passwordNumber));
    const char *sendStr = json_object_to_json_string(registerObj);
    int len = strlen(sendStr);

    /*将json对象转化为字符串发给服务器*/
    int retWrite = write(sockfd, sendStr, len + 1);
    if (retWrite == -1)
    {
        perror("write error");
    }
    printf("client over\n");
    return ret;
}
static int clientRegister(int sockfd)
{
    int ret = 0;
    int demand = REGISTER;

    struct json_object *registerObj = json_object_new_object();

    char accountNumber[BUFFER_SIZE];
    char *passwordNumber = calloc(BUFFER_SIZE, sizeof(char));
    printf("请输入账号\n");
    scanf("%s", accountNumber);
    while (getchar() != '\n');
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
    printf("check valid------------ing-----------\n");
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

    /* ip地址 */
    ret = connect(sockfd, (struct sockaddr *)&serverAddress, sizeof(serverAddress));
    if (ret == -1)
    {
        perror("connect error");
        exit(-1);
    }

    char recvBuffer[BUFFER_SIZE];
    memset(recvBuffer, 0, sizeof(recvBuffer));

    int choices = 0;
    while (strcmp(recvBuffer, SUCCESS_LOGIN))
    {
#if 1
        printf("正在加载页面...\n");
        sleep(2);
        printf("请输入选项:\n");
        printf("1.注册\n2.登录\n");
        scanf("%d", &choices);
        /*去除行缓存*/
        while (getchar() != '\n')
            ;
        switch (choices)
        {
        case REGISTER:
        {
            clientRegister(sockfd);
            /*睡一会，等待函数执行*/
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
            clientLogIn(sockfd);
            /*睡一会，等待函数执行*/
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
            break;
        }
        sleep(1);
#endif
    }
    /*登录成功*/
    while (1)
    {
        printf("请输入选项:\n");
        printf("1.添加好友\n2.好友请求\n3.删除好友\n4.给好友发送消息\n");
        scanf("%d", &choices);
        /*去除行缓存*/
        while (getchar() != '\n')
            ;
        switch (choices)
        {
        case ADD_FRIEND:
        {
            break;
        }
        case FRIEND_APPLICATION:
        {
            break;
        }
        case DELETE_FRIREND:
        {
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