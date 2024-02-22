#include <stdio.h>
#include <json-c/json.h>
#include <json-c/json_object.h>
#include <stdlib.h>
#include <error.h>
#include <sqlite3.h>
#include <string.h>
#include <unistd.h>

#define NAMESIZE 10
#define PWDSIZER 10

sqlite3 *db;
void re_hander()
{
    printf("------欢迎来到注册界面！------\n");

    /* 密码的输入 */
    char pwdVal[PWDSIZER];
    /* 用户名的输入 */
    char nameVal[NAMESIZE];
    printf("请输入用户名：\n");
    scanf("%s", nameVal);

    printf("请输入密码：\n");
    scanf("%s", pwdVal);

    /* 创建json定义信息 */
    struct json_object *jsonObj = json_object_new_object();
    if (jsonObj == NULL)
    {
        perror("jsonObj create error");
        exit(-1);
    }
    json_object_object_add(jsonObj, "name", json_object_new_string(nameVal));
    json_object_object_add(jsonObj, "pwd", json_object_new_string(pwdVal));

    /* 转换为字符串 */
    const char *str = json_object_to_json_string(jsonObj);
    write()


   

    

    /* 将其存储到数据库中 */
}
