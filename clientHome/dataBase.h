#ifndef __DATABASE_H_
#define __DATABASE_H_
#include <sqlite3.h>
#include <json-c/json.h>
#include <json-c/json_object.h>
/*初始化数据库*/
int dataBaseInit(sqlite3 **db);
/*检查用户名是否重复*/
int dataBaseDuplicateCheck(struct json_object *parseObj);
/*user插入用户信息*/
int dataBaseUserInsert(struct json_object *parseObj);
/*登录时，更新在线状态*/
int dataBaseFriendOnline(const char *name);
/*将上线状态变为下线*/
int dataBaseFriendOffline(struct json_object *parseObj);
/*给name发送好友申请*/
int dataBaseTakeApplyToName(struct json_object *parseObj, char *friendName);
/*name是否有好友申请*/
int dataBaseFindFriendApply(char *name);
/*找到有申请的好友名字*/
char *dataBaseFindApplyFriendName(char *name);
/*处理好友申请*/
int handleApply(int status, char *friendName);
/*好友表删除好友*/
int dataBaseDeleteFriend(struct json_object *parseObj, char *loginedName);
/*展示当前登录用户的好友名*/
int dataBaseDisPlayFriend(const char *loginedName);
/*创建群聊*/
int dataBaseCreateGroup(char *groupName, char *loginedName);
/*检查群聊名称是否存在*/
int dataBaseCheckGroupName(char *groupName, char *loginedName);
/*检查name是否存在于群聊中*/
int dataBaseCheckNameInGroup(char *groupName, char *loginedName);
/*插入群聊*/
int dataBaseInsertGroup(char *groupName, char *loginedName);
/*展示登录用户的所有群聊名称*/
int dataBaseDisPlayGroup(char *loginedName);
#endif