#ifndef __DATABASE_H_
#define __DATABASE_H_
#include <sqlite3.h>
#include <json-c/json.h>
#include <json-c/json_object.h>
int dataBaseInit(sqlite3 **db);
int dataBaseDuplicateCheck(struct json_object *parseObj);
int dataBaseUserInsert(struct json_object *parseObj);
int dataBaseFriendInsert(const char *name, const char *friendName, int acceptfd, int onlineStatus, const char *priviteMessage);
/*将上线状态变为下线*/
int dataBaseUpdateOnlineStatus(struct json_object *parseObj);
#endif