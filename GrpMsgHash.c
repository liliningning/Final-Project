#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sqlite3.h>
#include <time.h>
#include <pthread.h>
#include "GrpMsgHash.h"

/* 求字符串的ASC的绝对值 */
static int AscGet(char *GROUP);
/* 映射函数 */
static int GpHashKeyId(char *GROUP, int slotNums);
/* 双链表初始化 */
static int GpMsgListInit(GpMsgList *List);
/* 群聊消息节点插入 */
static int GpMsgListInsert(GpMsgList *list, char *Group, char *sender, char *receiver, char *Message);
/* 链表取消息 */
static int GpMsgListGet(GpMsgList *List, char *Group, char *sender, char *receiver, char *Message);

/* 求字符串的ASC */
static int AscGet(char *GROUP)
{
    int ASC_Nums = 0;
    int idx = 0;
    while (GROUP[idx] != '\0')
    {
        ASC_Nums += GROUP[idx];
        idx++;
    }

    if (ASC_Nums < 0)
    {
        ASC_Nums = ASC_Nums * -1;
    }

    return ASC_Nums;
}

/* 映射函数 */
static int GpHashKeyId(char *GROUP, int slotNums)
{
    int ASC_Nums = AscGet(GROUP);
    int keyId = (ASC_Nums % slotNums);

    return keyId;
}

/* 群聊消息链表初始化 */
static int GpMsgListInit(GpMsgList *List)
{
    if (List == NULL)
    {
        return NULL_PTRR;
    }

    List->head = (GpMsgNode *)malloc(sizeof(GpMsgNode) * 1);
    List->head->pre = NULL;
    List->head->next = NULL;
    List->tail = List->head;
    List->size = 0;
    return 0;
}

/* 群聊消息哈希表初始化 */
int GpHashInit(GpHash **Gp, int slotNum)
{
    GpHash *tmpGp = (GpHash *)malloc(sizeof(GpHash) * 1);
    tmpGp->slotNums = slotNum;
    tmpGp->GpKeyId = (GpMsgList *)malloc(sizeof(GpMsgList) * tmpGp->slotNums);

    for (int idx = 0; idx < tmpGp->slotNums; idx++)
    {
        GpMsgListInit(&(tmpGp->GpKeyId[idx]));
    }

    *Gp = tmpGp;
    return 0;
}

/* 群聊消息节点插入 */
static int GpMsgListInsert(GpMsgList *list, char *Group, char *sender, char *receiver, char *Message)
{
    if (list == NULL)
    {
        return NULL_PTRR;
    }

    GpMsgNode *tmpNode = (GpMsgNode *)malloc(sizeof(GpMsgNode) * 1);
    strncpy(tmpNode->messge, Message, sizeof(char) * MESSAGE_SIZE);
    strncpy(tmpNode->sender, sender, sizeof(char) * ACCOUNT_LEN);
    strncpy(tmpNode->receiver, receiver, sizeof(char) * ACCOUNT_LEN);
    strncpy(tmpNode->group, Group, sizeof(char) * GROUP_SIZE);
    // 获取当前时间
    time_t rawtime;
    struct tm *timeinfo;

    time(&rawtime);
    timeinfo = localtime(&rawtime);

    /* 将时间字符串写入结点时间字符串数组中 */
    strftime(tmpNode->time, TIME_SIZE, "%Y-%m-%d %H:%M:%S", timeinfo);

    /* 尾插头遍历 */
    tmpNode->next = list->tail->next;
    tmpNode->pre = list->tail;
    if (list->tail->next != NULL)
    {
        list->tail->next = tmpNode;
    }
    list->tail->next = tmpNode;
    list->tail = tmpNode;

    list->size++;
    return 0;
}

/* 群聊哈希表消息插入 */
int GpHashInsert(GpHash *Gp, char *Group, char *sender, char *receiver, char *Message)
{
    if (Gp == NULL)
    {
        return NULL_PTRR;
    }

    /* 获取索引，群名 */
    int keyIdx = GpHashKeyId(Group, Gp->slotNums);

    /* 插消息 */
    GpMsgListInsert(&(Gp->GpKeyId[keyIdx]), Group, sender, receiver, Message);

    return 0;
}

/* 取消息,此处已知的是群名和接收者，需要取消息和发送者 */
/*sender和消息是传出参数*/
static int GpMsgListGet(GpMsgList *List, char *Group, char *sender, char *receiver, char *Message)
{
    if (List == NULL)
    {
        return NULL_PTRR;
    }
    if (List->size == 0)
    {
        return NOT_SEARCH;
    }
    /* 头遍历取 */
    GpMsgNode *travelNode = List->head->next;
    // pthread_mutex_lock(Gp_Mutex);
    while (travelNode != NULL)
    {
        /* 接收者是我 && 群聊名称是指定的群聊名 */
        int ret1 = strncmp(travelNode->receiver, receiver, sizeof(char) * ACCOUNT_LEN);
        int ret2 = strncmp(travelNode->group, Group, strlen(Group));
        if (ret1 == 0 && ret2 == 0)
        {
            if (travelNode == List->tail)
            {
                printf("tail\n");
                List->tail = travelNode->pre;
            }

            travelNode->pre->next = travelNode->next;
            if (travelNode->next != NULL)
            {
                travelNode->next->pre = travelNode->pre;
            }
            /* 取消息 */
            strncpy(Message, travelNode->messge, sizeof(char) * MESSAGE_SIZE);
            /* 取发送者 */
            strncpy(sender, travelNode->sender, sizeof(char) * ACCOUNT_LEN);
            /* 存数据库todo */

            if (travelNode != NULL)
            {
                free(travelNode);
                printf("%s:take and free\n", receiver);
                travelNode = NULL;
                List->size--;
            }

            return ON_SUCCESSS;
        }
        travelNode = travelNode->next;
    }

    return NOT_SEARCH;
}

/* 群聊哈希表消息取 */
int GpHashGet(GpHash *Gp, char *Group, char *sender, char *receiver, char *Message)
{
    if (Gp == NULL)
    {
        return NULL_PTRR;
    }
    int keyIdx = GpHashKeyId(Group, Gp->slotNums);
    int ret = GpMsgListGet(&(Gp->GpKeyId[keyIdx]), Group, sender, receiver, Message);
    return ret;
}

/* 群聊哈希表销毁 */
int GpHashDel(GpHash *Gp, sqlite3 *Data_Db)
{

    return 0;
}