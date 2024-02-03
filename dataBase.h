#ifndef __DATABASE_H_
#define __DATABASE_H_
#include <sqlite3.h>
#include <json-c/json.h>
#include <json-c/json_object.h>
/* 用于初始化数据库连接，并将连接对象存储在指向 sqlite3 指针的指针 db 中 */
int dataBaseInit(sqlite3 **db);
/* 用于检查数据库中是否存在重复的数据。参数 parseObj 可能包含需要检查的数据。 */
int dataBaseDuplicateCheck(struct json_object *parseObj);
/* 用于将用户信息插入数据库。参数 parseObj 可能包含要插入的用户信息。 */
int dataBaseUserInsert(struct json_object *parseObj);
/* 用于将朋友信息插入数据库。参数分别为用户名、朋友用户名、接受文件描述符、在线状态和私密消息。 */
int dataBaseFriendInsert(const char *name, const char *friendName, int acceptfd, int onlineStatus, const char *priviteMessage);
/*将上线状态变为下线*/
int dataBaseUpdateOnlineStatus(struct json_object *parseObj);
#endif