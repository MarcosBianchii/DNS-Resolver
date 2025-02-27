#ifndef __LRU__
#define __LRU__

#include "hashmap.h"

typedef struct LruCache {
    HashMap map;
    char *head_key;
    char *tail_key;
    usize cap;
} LruCache;

LruCache lru_new(usize cap);
void lru_push(LruCache *lru, char *key, char *value);
char *lru_get(LruCache *lru, char *key);
void lru_del(LruCache *lru);

#endif // __LRU__
