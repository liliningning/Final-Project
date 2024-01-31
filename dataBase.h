#ifndef __DATABASE_H_
#define __DATABASE_H_
#include <sqlite3.h>
#include <json-c/json.h>
#include <json-c/json_object.h>
int dataBaseInit(sqlite3 **db);
int dataBaseDuplicateCheck(struct json_object *parseObj);
int dataBaseUserInsert(struct json_object *parseObj);
#endif