#ifndef _PRIVATE_MSG_HASH_H_
#define _PRIVATE_MSG_HASH_H_

#include <sqlite3.h>
#include "commonHash.h"
/* 消息结点 */
typedef struct MsgNode
{
    /* 消息数据 */
    char messge[MESSAGE_SIZE];
    char sender[ACCOUNT_SIZE];
    char receiver[ACCOUNT_SIZE]; /* 通过接收者账号给节点取数组索引 */
    char time[TIME_SIZE];

    struct MsgNode *pre;
    struct MsgNode *next;
} MsgNode;

/* 消息结点双链表 */
typedef struct MsgList
{
    MsgNode *head; // 虚拟头节点，分配空间
    MsgNode *tail; // 标记，不分配
    /* 链表结点个数 */
    int MsgSize;
} MsgList;

typedef struct MsgHash
{
    /* 数组位置数 */
    int slotNums;
    /* 指向连续的消息结点双链表空间 */
    MsgList *rcvKeyId;
} MsgHash;

/* 初始化 */ /* 参数2为要定义的槽位数 */
int HashInit(MsgHash **msgHash, int slotNum);

/* 插消息，Message为要插入的消息，获取插入时的时间并插入 */ /* 插这里的发送者是我，接收者是好友 */
int hashMsgInsert(MsgHash *msgHash, char *Sender, char *Receiver, char *Message);

/* 取消息，取完一个保存到数据库并释放，定义为已读 */ /* 取这里发送者是好友，接收者是我 */
int hashMsgGet(MsgHash *msgHash, char *Sender, char *Receiver, char *Message);

#endif //_PRIVATE_MSG_HASH_H_