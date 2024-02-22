#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sqlite3.h>
#include <time.h>
#include "privateMsgHash.h"

/* 映射函数, 返回值为keyId */
static int HashKeyId(char *recvAccount, int slotNums);

/* 初始化消息结点双链表 */
static int MsgListInit(MsgList *List);

/* 映射函数 */
static int HashKeyId(char *recvAccount, int slotNums)
{
    /* 将字符串账号转化为数字 */
    long int number = strtol(recvAccount, NULL, 10);
    /* 转换成的数字对数组大小取余 */
    int keyId = (number % slotNums);
    return keyId;
}

/* 初始化消息结点双链表 */
static int MsgListInit(MsgList *List)
{
    if (List == NULL)
    {
        return NULL_PTRR;
    }

    List->head = (MsgNode *)malloc(sizeof(MsgNode) * 1);
    /* 虚拟头结点，不赋值 */
    List->head->pre = NULL;
    List->head->next = NULL;

    /* 此处头指针初始化应该指向head */
    List->tail = List->head;
    List->MsgSize = 0;

    return 0;
}

/* 初始化 */ /* 参数2为要定义的槽位数 */
int HashInit(MsgHash **msgHash, int slotNum)
{
    MsgHash *hash = (MsgHash *)malloc(sizeof(MsgHash) * 1);
    if (hash == NULL)
    {
        return MALLOC_ERRORR;
    }

    hash->slotNums = slotNum;
    hash->rcvKeyId = (MsgList *)malloc(sizeof(MsgList) * slotNum);

    /* 给每个链表初始化 */
    for (int idx = 0; idx < hash->slotNums; idx++)
    {
        MsgListInit(&(hash->rcvKeyId[idx]));
    }

    *msgHash = hash;

    return 0;
}

/* 读取数据库保存的未读消息到哈希表中 */

/* 消息链表结点插入 */ /* 插这里的发送者是我，接收者是好友 */
static int MsgListInsert(MsgList *List, char *Sender, char *Receiver, char *Message)
{
    if (List == NULL)
    {
        return NULL_PTRR;
    }

    MsgNode *Node = (MsgNode *)malloc(sizeof(MsgNode) * 1);
    memset(Node, 0, sizeof(MsgNode));
    /* 下面strncpy的字节数要约定好 */
    strncpy(Node->sender, Sender, sizeof(char) * ACCOUNT_LEN);
    strncpy(Node->receiver, Receiver, sizeof(char) * ACCOUNT_LEN);
    strncpy(Node->messge, Message, sizeof(char) * MESSAGE_SIZE);

    // 获取当前时间
    time_t rawtime;
    struct tm *timeinfo;

    time(&rawtime);
    timeinfo = localtime(&rawtime);

    /* 将时间字符串写入结点时间字符串数组中 */
    strftime(Node->time, TIME_SIZE, "%Y-%m-%d %H:%M:%S", timeinfo);

    /* 尾插头遍历 */
    Node->next = List->tail->next;
    Node->pre = List->tail;
    /* 这里记得判断 */
    if (List->tail->next != NULL)
    {
        List->tail->next->pre = Node;
    }
    List->tail->next = Node;
    /* 更新尾指针 */
    List->tail = Node;

    List->MsgSize++;

    return 0;
}

/* 插消息，Message为要插入的消息，获取插入时的时间并插入 */ /* 插这里的发送者是我，接收者是好友 */
int hashMsgInsert(MsgHash *msgHash, char *Sender, char *Receiver, char *Message)
{
    if (msgHash == NULL)
    {
        return NULL_PTRR;
    }
    /* 获取要插入的链表的索引 */
    int keyIdx = HashKeyId(Receiver, msgHash->slotNums);
    /* 头插尾取 */
    MsgListInsert(&(msgHash->rcvKeyId[keyIdx]), Sender, Receiver, Message);

    return 0;
}

/* 头遍历取 */ /* 返回值用于判断是否成功 */ /* 取这里发送者是好友，接收者是我 */
static int MsgListGet(MsgList *List, char *Sender, char *Receiver, char *Message)
{
    if (List == NULL)
    {
        return NULL_PTRR;
    }
    /* 头遍历取 */
    MsgNode *travelNode = List->head->next;
    while (travelNode != NULL)
    {
        int ret1 = strncmp(travelNode->sender, Sender, sizeof(char) * ACCOUNT_LEN);
        int ret2 = strncmp(travelNode->receiver, Receiver, sizeof(char) * ACCOUNT_LEN);
        if (ret1 == 0 && ret2 == 0)
        {
            /* 满足条件，是要找的消息 */
            /* 取 */
            if (travelNode == List->tail)
            {
                /* 维护好尾指针 */
                List->tail = travelNode->pre;
            }

            travelNode->pre->next = travelNode->next;
            if (travelNode->next != NULL)
            {
                travelNode->next->pre = travelNode->pre;
            }
            /* 取消息 */
            strncpy(Message, travelNode->messge, sizeof(char) * MESSAGE_SIZE);
            /* 存数据库todo....... */

            free(travelNode);
            return ON_SUCCESSS;
        }
        travelNode = travelNode->next;
    }

    return NOT_SEARCH;
}

/* 取消息，取完一个保存到数据库并释放，定义为已读 */ /* 取这里发送者是好友，接收者是我 */
int hashMsgGet(MsgHash *msgHash, char *Sender, char *Receiver, char *Message)
{
    if (msgHash == NULL)
    {
        return NULL_PTRR;
    }
    /* 获取要插入的链表的索引 */
    int keyIdx = HashKeyId(Receiver, msgHash->slotNums);
    /* Message用于取消息， 返回值用于判断是否成功找到 */
    int ret = MsgListGet(&(msgHash->rcvKeyId[keyIdx]), Sender, Receiver, Message);

    return ret;
}

/* 销毁消息哈希表，保存所有的节点到数据库，定义为未读，然后释放 */
int hashMsgDel(MsgHash *msgHash, sqlite3 *Data_Db)
{

    return 0;
}