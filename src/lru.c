#include "lru.h"
#include "hashmap.h"
#include "queue.h"
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

LruCache lru_new(usize cap) {
    // invariant cap > 1
    return (LruCache){
        .map = hashmap_new(cap),
        .head_key = NULL,
        .tail_key = NULL,
        .cap = cap,
    };
}

usize lru_len(LruCache lru) { return hashmap_len(lru.map); }

void push_to_front(LruCache *lru, QueueNode *node) {
    if (!lru->head_key || !lru->tail_key) {
        lru->head_key = node->key;
        lru->tail_key = node->key;
        return;
    }

    if (strcmp(lru->head_key, node->key) == 0) {
        return;
    }

    if (strcmp(lru->tail_key, node->key) == 0) {
        lru->tail_key = node->prev->key;
    }

    queuenode_detach(node);
    if (lru->head_key) {
        QueueNode *newest = hashmap_get(lru->map, lru->head_key);
        if (newest) {
            node->next = newest;
            newest->prev = node;
        }
    } else {
        lru->tail_key = node->key;
    }

    lru->head_key = node->key;
}

void lru_push(LruCache *lru, char *key, char *value) {
    if (!lru || !key || !value) {
        return;
    }
    
    QueueNode *node = hashmap_get(lru->map, key);
    if (node) {
        push_to_front(lru, node);
        return;
    }

    if (lru_len(*lru) == lru->cap) {
        QueueNode *oldest = hashmap_remove(&lru->map, lru->tail_key);
        if (oldest) {
            lru->tail_key = oldest->prev ? oldest->prev->key : NULL;
            queuenode_del(oldest);

            if (!lru->tail_key) {
                lru->head_key = NULL;
            }
        }
    }

    node = queuenode_new(key, value);
    hashmap_insert(&lru->map, key, node);
    push_to_front(lru, node);
}

char *lru_get(LruCache *lru, char *key) {
    QueueNode *node = hashmap_get(lru->map, key);
    if (!node) {
        return NULL;
    }

    push_to_front(lru, node);
    return node->ip;
}

void lru_del(LruCache *lru) {
    if (lru) {
        hashmap_del(&lru->map);
    }
}
