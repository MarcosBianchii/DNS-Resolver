#include "queue.h"
#include <stdlib.h>
#include <string.h>

QueueNode *queuenode_new(char *key, char *ip) {
    QueueNode *node = calloc(1, sizeof(QueueNode));
    if (!node) {
        return NULL;
    }

    node->key = strdup(key);
    node->ip = strdup(ip);
    return node;
}

void queuenode_detach(QueueNode *node) {
    if (!node) {
        return;
    }

    QueueNode *prev = node->prev;
    QueueNode *next = node->next;

    if (prev) {
        prev->next = next;
    }

    if (next) {
        next->prev = prev;
    }

    node->prev = NULL;
    node->next = NULL;
}

void queuenode_del(QueueNode *node) {
    if (node) {
        queuenode_detach(node);
        free(node->key);
        free(node->ip);
        free(node);
    }
}
