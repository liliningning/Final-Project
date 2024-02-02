#ifndef __DATABASE_H_
#define __DATABASE_H_
#include <sqlite3.h>
#include <json-c/json.h>
#include <json-c/json_object.h>
/*初始化数据库*/
int dataBaseInit(sqlite3 **db);
/*检查用户名是否重复*/
int dataBaseDuplicateCheck(struct json_object *parseObj);
/*插入用户信息*/
int dataBaseUserInsert(struct json_object *parseObj);
/*登录时，更新在线状态*/
int dataBaseFriendOnline(const char *friendName);
/*将上线状态变为下线*/
int dataBaseFriendOffline(struct json_object *parseObj);
#endif