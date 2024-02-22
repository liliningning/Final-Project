#ifndef __HASH_TABLE_H_
#define __HASH_TABLE_H_
#define SENDER_SIZE 32
#define Message_SIZE 128
#include "common.h"

typedef struct MessagePackage
{
    char sender[SENDER_SIZE];
    char message[Message_SIZE];
} MessagePackage;

#define HASH_KEYTYPE char *
#define HASH_VALUETYPE MessagePackage
typedef struct hashNode
{
    HASH_KEYTYPE real_key;
    HASH_VALUETYPE value;
} hashNode;
#define SLOT_CAPACITY 97

typedef struct hashTable
{
    /* 哈希表的槽位数 */
    int slotNums;

    /* 哈希表的槽位号 (分配一块连续的存储空间) 指针数组(链表头结点) */
    DoubleLinkList **slotKeyId;

    /* 自定义比较器 用于适配链表数据结构 */
    int (*compareFunc)(ELEMENTTYPE, ELEMENTTYPE);
} HashTable;

/* 哈希表的初始化 */
int hashTableInit(HashTable **pHashtable, int slotNums, int (*compareFunc)(ELEMENTTYPE, ELEMENTTYPE));

/* 哈希表 插入<key, value> */
int hashTableInsert(HashTable *pHashtable, HASH_KEYTYPE key, HASH_VALUETYPE value);

/* 哈希表 删除指定key. */
int hashTableDelAppointKey(HashTable *pHashtable, HASH_KEYTYPE key);

/* 哈希表 获取所有接收方为key发送方为senderName的message */
int hashTableGetAppointKeyValue(HashTable *pHashtable, HASH_KEYTYPE key, char ***messageList, int *pRow, char *senderName);

/* 哈希表元素大小 */
int hashTableGetSize(HashTable *pHashtable);

/* 哈希表的销毁 */
int hashTableDestroy(HashTable *pHashtable);

#endif //__HASH_TABLE_H_