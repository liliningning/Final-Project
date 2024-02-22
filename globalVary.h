#ifndef _GLOBALVARY_H__
#define _GLOBALVARY_H__
#include <stdio.h>
#include "hashtable.h"
#include "modelThreadPool.h"
#include "balanceBinarySearchTree.h"
#include "sqlite3.h"
#include <pthread.h>
/*全局变量，方便捕捉信号后释放资源*/
HashTable *hash = NULL;
ThreadPool *pool = NULL;
int sockfd;
BalanceBinarySearchTree *AVL = NULL;
sqlite3 *mydb = NULL;
pthread_mutex_t g_mutex;
#endif //_GLOBALVARY_H__