#ifndef __DATABASE_H_
#define __DATABASE_H_
#include <sqlite3.h>
#include <json-c/json.h>
#include <json-c/json_object.h>

/* 数据库初始化 */
int dataBaseInit();
/* 查询  */
int dataBaseDuplicateCheck(struct json_object *parseObj);
/* 数据库的插入 */
int dataBaseUserInsert(struct json_object *parseObj);

/* 数据库的删除 */
int dataBaseDelete(struct json_object *parseObj);
#endif