#ifndef __HASHMAP__
#define __HASHMAP__

#include "ints.h"
#include "queue.h"
#include <stdlib.h>
#include <stdbool.h>

#define LOAD_FACTOR 0.75

// A fixed size implementation of a HashMap
typedef struct HashMap {
    QueueNode **table;
    usize size;
    usize len;
} HashMap;

HashMap hashmap_new(usize cap);
QueueNode *hashmap_insert(HashMap *map, char *key, QueueNode *value);
QueueNode *hashmap_get(HashMap map, char *key);
QueueNode *hashmap_remove(HashMap *map, char *key);
usize hashmap_len(HashMap map);
void hashmap_del(HashMap *map);

#endif // __HASHMAP__
