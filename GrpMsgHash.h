#ifndef _GRP_MSG_HASH_H_
#define _GRP_MSG_HASH_H_

#include "commonHash.h"

/* 消息结点 */
typedef struct GpMsgNode
{
    /* 消息数据 */
    char messge[MESSAGE_SIZE];
    char sender[ACCOUNT_SIZE];
    char receiver[ACCOUNT_SIZE]; /* 通过接收者账号给节点取数组索引 */
    char group[GROUP_SIZE];
    char time[TIME_SIZE];

    struct GpMsgNode *pre;
    struct GpMsgNode *next;
} GpMsgNode;

typedef struct GpMsgList
{
    GpMsgNode *head; // 虚拟头节点
    GpMsgNode *tail;
    int size;
} GpMsgList;

typedef struct GpHash
{
    /* 数组位置数 */
    int slotNums;
    /* 指向连续的消息结点双链表空间 */
    GpMsgList *GpKeyId;
} GpHash;

/* 群聊消息哈希表初始化 */
int GpHashInit(GpHash **Gp, int slotNum);

/* 群聊哈希表消息插入 */
int GpHashInsert(GpHash *Gp, char *Group, char *sender, char *receiver, char *Message);

/* 群聊哈希表消息取 */
int GpHashGet(GpHash *Gp, char *Group, char *sender, char *receiver, char *Message);

/* 群聊哈希表销毁 */
int GpHashDel(GpHash *Gp, sqlite3 *Data_Db);
#endif //_GRP_MSG_HASH_H_