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
#include <sqlite3.h>
#define SERVER_PORT 8888
#define SERVER_IP "172.16.104.91"
#define BUFFER_SIZE 128

typedef enum USER_ONE
{
    EXIT,
    REGISTER,
    LOGIN,

}USER_ONE;


typedef enum USER_TWO
{
    EXIT,
    PRIVECHAT,
    GROUPCHAT,
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



void  twomenu()
{
    printf("\t-------------------------------------\n");
    printf("\t|                                   |\n");
    printf("\t|    1:私聊                         |\n");
    printf("\t|    2:群聊                         |\n");
    printf("\t|    0:退出                         |\n");
    printf("\t|                                   |\n");
    printf("\t-------------------------------------\n");
}

/* 注册 */
static int clientRegister(int sockfd)
{
    int ret = 0;
    int demand = REGISTER;

    struct json_object *registerObj = json_object_new_object();

    /* 用数组去接账号 */
    char accountNumber[BUFFER_SIZE];
    char *passwordNumber = calloc(BUFFER_SIZE, sizeof(char));
    printf("请输入账号\n");
    scanf("%s", accountNumber);
    getchar();
    /*账号查重todo...*/
    /*备份passwordNumber*/
    char *accord = passwordNumber;
    int checkout = 0;
    while (checkout == 0)
    {
        /*密码要求*/
        printf("请输入密码(密码不少于8位且必须包含字符和数字):\n");
        scanf("%s", passwordNumber);
        getchar();
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
    write(sockfd, sendStr, len + 1);

    printf("注册成功\n");
    sleep(2);

    return ret;
}

/* 登录 */
void clientLogin(int sockfd)
{
    char accountNumber[BUFFER_SIZE];
    memset(accountNumber, 0, sizeof(accountNumber));
    char *passwordNumber = calloc(BUFFER_SIZE, sizeof(char));
    printf("登录\n");
    printf("\n");
    printf("请输入账户:");
    scanf("%s", accountNumber);

    printf("请输入密码：");
    scanf("%s", passwordNumber);
    char **result = NULL;
    const char *sql = "select pwd from  usr where name= 'xxx' ";
    /* 执行数据库语句 sqlite3_exec()*/
    /* 数据库查询 是否相等 */
    int num = strncmp(accountNumber, result[1], sizeof(accountNumber) - 1 );
    if (num == 0)
    {
        printf("登录成功！");
        /* 跳转到聊天界面 */
        twomenu();
    }
    else
    {
        /* 重新输入密码  */
        printf("请输入密码：");
        scanf("%s", passwordNumber);
    }
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

    int enableOpt = 1;
    /* 端口复用 */
    ret = setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &enableOpt, sizeof(enableOpt));
    if (ret == -1)
    {
        perror(" setsockopt error");
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

    int choices;
    while (1)
    {
#if 1
        menu();
        printf("请输入选项:\n");
        scanf("%d", &choices);
        switch (choices)
        {
        case REGISTER:
        {
            clientRegister(sockfd);
            break;
        }
        case LOGIN:
        {
            clientLogin(sockfd);
            break;
        }
        case EXIT:
        {
            break;
        }
        default:
            break;
        }

        // read(sockfd, recvBuffer, sizeof(recvBuffer) - 1);
        // printf("server massage:%s\n", recvBuffer);

        sleep(1);
#endif
    }

    /* 休息5S */
    sleep(5);

    close(sockfd);

    return 0;
}