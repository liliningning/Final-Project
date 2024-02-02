#ifndef __DATABASE_H_
#define __DATABASE_H_
#include <sqlite3.h>
#include <json-c/json.h>
#include <json-c/json_object.h>





/* 数据库建表  */
int dataBaseInit();
/* 查询  */
int dataBaseDuplicateCheck(struct json_object *parseObj);
/* 数据库的插入 */
int dataBaseUserInsert(struct json_object *parseObj);

/* 好有表数据库的删除 */
int dataBaseFriendDelete(const char * name , const  char * friendName);

/* 好友表的查询 */
int dataBaseFriendSelect(const char *friendName);

/* 好友表的插入 insert   into values */
int dataBaseFriendInsert(const char *name, const char *friendName);




#endif