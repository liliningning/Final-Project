#include <stdio.h>
#include "hashtable.h"
#include <stdlib.h>
#include "doubleLinkList.h"
#include <error.h>
#include <string.h>

#define DEFAULT_SLOT_NUMS 37

/* 函数前置声明 */
static int calHashValue(HashTable *pHashtable, HASH_KEYTYPE key, int *slotKeyId);
static hashNode *createHashNode(HASH_KEYTYPE key, HASH_VALUETYPE value);

/* 哈希表的初始化 */
int hashTableInit(HashTable **pHashtable, int slotNums, int (*compareFunc)(ELEMENTTYPE, ELEMENTTYPE))
{
    /* 判空 */
    if (pHashtable == NULL)
    {
        return -1;
    }

    int ret = 0;

    HashTable *hash = (HashTable *)malloc(sizeof(HashTable) * 1);
    if (hash == NULL)
    {
        perror("malloc error");
        return MALLOC_ERROR;
    }
    /* 清除脏数据 */
    memset(hash, 0, sizeof(HashTable) * 1);

    /* 判断槽位号的合法性 */
    if (slotNums <= 0)
    {
        slotNums = DEFAULT_SLOT_NUMS;
    }
    hash->slotNums = slotNums;

    /* 动态数组分配空间 */
    hash->slotKeyId = (DoubleLinkList **)malloc(sizeof(DoubleLinkList *) * (hash->slotNums));
    if (hash->slotKeyId == NULL)
    {
        perror("malloc error");
        return MALLOC_ERROR;
    }
    /* 清除脏数据 */
    memset(hash->slotKeyId, 0, sizeof(DoubleLinkList *) * (hash->slotNums));

    /* 初始化 : 每一个槽位号内部维护一个链表. */
    for (int idx = 0; idx < hash->slotNums; idx++)
    {
        /* 为哈希表的value初始化。哈希表的value是链表的虚拟头结点 */
        DoubleLinkListInit(&(hash->slotKeyId[idx]));
    }

    /* 自定义比较函数 钩子🪝函数 */
    hash->compareFunc = compareFunc;

    /* 指针解引用 */
    *pHashtable = hash;
    return ret;
}

/* 计算外部传过来的key 转化为哈希表内部维护的slotKeyId. slotKeyIds是数组(动态数组)索引 */
static int calHashValue(HashTable *pHashtable, HASH_KEYTYPE key, int *slotKeyId)
{
    int ret = 0;
    int sum = 0;
    /*防止key执行完哈希函数后指向字符串末尾导致后续赋值出现错误*/
    char *temp = key;
    while (*temp != '\0')
    {
        sum = sum + *temp;
        temp++;
    }
    *slotKeyId = sum % (pHashtable->slotNums);
    return ret;
}

/* 新建结点 */
static hashNode *createHashNode(HASH_KEYTYPE key, HASH_VALUETYPE value)
{
    /* 封装结点 */
    hashNode *newNode = (hashNode *)malloc(sizeof(hashNode) * 1);
    if (newNode == NULL)
    {
        return NULL;
    }
    /* 清除脏数据 */
    memset(newNode, 0, sizeof(hashNode) * 1);

    newNode->real_key = key;
    newNode->value = value;

    /* 返回新结点 */
    return newNode;
}

/* 哈希表 插入<key, value> */
int hashTableInsert(HashTable *pHashtable, HASH_KEYTYPE key, HASH_VALUETYPE value)
{
    /* 判空 */
    if (pHashtable == NULL)
    {
        return NULL_PTR;
    }

    int ret = 0;

    /* 将外部传过来的key 转化为我哈希表对应的slotId */
    int KeyId = 0;
    calHashValue(pHashtable, key, &KeyId);

    /* todo: 去重... */
    /* 创建哈希node */
    hashNode *newNode = createHashNode(key, value);
    if (newNode == NULL)
    {
        perror("create hash node error");
        return MALLOC_ERROR;
    }
#if 0 /*去重，哈希表存储消息，不需要去重*/
    DoubleLinkNode *node = pHashtable->slotKeyId[KeyId]->head->next;
    while (node != NULL)
    {
        hashNode *travelHashNode = (hashNode *)node->data;
        if (pHashtable->compareFunc(travelHashNode, newNode) == 0)
        {
            /* 如果已经存在相同的键值对key-value,将哈希节点的value更新为最新value */
            travelHashNode->value = value;
            /*有重复的key,无需插入新节点，直接释放掉*/
            if (newNode != NULL)
            {
                free(newNode);
                newNode = NULL;
            }
            return ret;
        }
        node = node->next;
    }
#endif
    /* 将哈希结点插入到链表中. */
    DoubleLinkListTailInsert(pHashtable->slotKeyId[KeyId], newNode);

    return ret;
}

/* 哈希表 删除指定key. */
int hashTableDelAppointKey(HashTable *pHashtable, HASH_KEYTYPE key)
{
    /* 判空 */
    if (pHashtable == NULL)
    {
        return -1;
    }

    int ret = 0;
    /* 将外部传过来的key 转化为我哈希表对应的slotId */
    int KeyId = 0;
    calHashValue(pHashtable, key, &KeyId);

    hashNode tmpNode;
    memset(&tmpNode, 0, sizeof(hashNode));
    tmpNode.real_key = key;

#if 1
    /* todo... 删除哈希结点 */
    DoubleLinkNode *resNode = DoubleLinkListAppointKeyValGetNode((pHashtable->slotKeyId[KeyId]), &tmpNode, pHashtable->compareFunc);
    if (resNode == NULL)
    {
        return -1;
    }

    /* 备份哈希结点 */
    hashNode *delHashNode = resNode->data;
#endif
    DoubleLinkListDelAppointData(pHashtable->slotKeyId[KeyId], &tmpNode, pHashtable->compareFunc);

    if (delHashNode)
    {
        free(delHashNode);
        delHashNode = NULL;
    }
    return ret;
}
/*根据结点找到对应的值*/
char **DoubleLinkListAppointKeyValGetAllNodeValue(DoubleLinkList *pList, ELEMENTTYPE val, int (*compareFunc)(ELEMENTTYPE, ELEMENTTYPE), int *pRow, char *senderName)
{
    int row = 0;
    int pos = 0;
    char **result = NULL;
    int idx = 0;
    DoubleLinkNode *travelNode = pList->head->next;
    int cmp = 0;
    while (travelNode != NULL)
    {
        cmp = compareFunc(val, travelNode->data);
        if (cmp == 0)
        {

            hashNode *mapNode = (hashNode *)travelNode->data;
            if (strcmp(mapNode->value.sender, senderName) == 0)
            {
                result[idx] = mapNode->value.message;
                // sprintf(result[idx], "'%s'向你发送信息:'%s'", mapNode->value.sender, mapNode->value.message);
                idx++;
            }
            else
            {
                continue;
            }
        }
        /* 遍历 */
        travelNode = travelNode->next;
    }
    if (idx > 0)
    {
        *pRow = row;
        return result;
    }
    else
    {
        return NULL;
    }
}
/* 哈希表 获取接收方为key发送方为senderName的所有的message */
int hashTableGetAppointKeyValue(HashTable *pHashtable, HASH_KEYTYPE key, char ***messageList, int *pRow, char *senderName)
{

    /* 将外部传过来的key 转化为我哈希表对应的slotId */
    int KeyId = 0;
    calHashValue(pHashtable, key, &KeyId);
    int row = 0;
    hashNode tmpNode;
    memset(&tmpNode, 0, sizeof(hashNode));
    /*遍历keyid所属链表，检索messagePackage的发送者名字，将正确的发送者消息全部取出*/
    /*将key赋值给tmpNode*/
    tmpNode.real_key = key;
    /*给出哈希节点的key，找出key所对应的所有消息*/
    char **result = DoubleLinkListAppointKeyValGetAllNodeValue((pHashtable->slotKeyId[KeyId]), &tmpNode, pHashtable->compareFunc, &row, senderName);
    if (result == NULL)
    {
        return -1;
    }
    /*解引用*/
    *messageList = result;
    *pRow = row;
    return ON_SUCCESS;
}

/* 哈希表元素大小 */
int hashTableGetSize(HashTable *pHashtable)
{
    if (pHashtable == NULL)
    {
        return 0;
    }

    int size = 0;
    for (int idx = 0; idx < pHashtable->slotNums; idx++)
    {
        size += pHashtable->slotKeyId[idx]->len;
    }

    /* 哈希表的元素个数. */
    return size;
}

/* 哈希表的销毁 */
int hashTableDestroy(HashTable *pHashtable)
{
    /* 自己分配的内存自己释放 */
    if (pHashtable == NULL)
    {
        return 0;
    }

    /* 谁开辟空间, 谁释放空间. */

    /* 1. 先释放哈希表的结点 */
    for (int idx = 0; idx < pHashtable->slotNums; idx++)
    {
        DoubleLinkNode *travelLinkNode = pHashtable->slotKeyId[idx]->head->next;
        while (travelLinkNode != NULL)
        {
            /* 释放哈希结点 */
            free(travelLinkNode->data);
            travelLinkNode->data = NULL;

            /* 指针位置移动 */
            travelLinkNode = travelLinkNode->next;
        }
    }
    /* 2. 释放哈希表每个槽维的链表 */
    for (int idx = 0; idx < pHashtable->slotNums; idx++)
    {
        DoubleLinkListDestroy(pHashtable->slotKeyId[idx]);
    }

    /* 3. 释放槽位 */
    if (pHashtable->slotKeyId != NULL)
    {
        free(pHashtable->slotKeyId);
        pHashtable->slotKeyId = NULL;
    }

    /* 4. 释放哈希表 */
    if (pHashtable != NULL)
    {
        free(pHashtable);
        pHashtable = NULL;
    }
}