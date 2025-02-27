#ifndef __QUEUE__
#define __QUEUE__

#include "ints.h"

typedef struct QueueNode {
    struct QueueNode *prev;
    struct QueueNode *next;
    char *key;
    char *ip;
} QueueNode;

QueueNode *queuenode_new(char *key, char *ip);
void queuenode_detach(QueueNode *node);
void queuenode_del(QueueNode *node);

#endif // __QUEUE__
