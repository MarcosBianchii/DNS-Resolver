#include "hashmap.h"
#include <string.h>

HashMap hashmap_new(usize cap) {
    usize n = (cap / LOAD_FACTOR) + 1;
    return (HashMap){
        .table = calloc(n, sizeof(void *)),
        .len = 0,
        .size = n,
    };
}

usize hash(char *key) {
    int hash = 5380;

    while (*key) {
        int c = *key++;
        hash = ((hash << 4) + hash) + c;
    }

    return (usize)hash;
}

usize find_slot(HashMap map, char *key) {
    usize i = hash(key) % map.size;

    while (map.table[i] && strcmp(key, map.table[i]->key) != 0) {
        i = (i + 1) % map.size;
    }

    return i;
}

QueueNode *hashmap_insert(HashMap *map, char *key, QueueNode *value) {
    if (!map) {
        return NULL;
    }

    usize i = find_slot(*map, key);
    QueueNode *old_node = map->table[i];
    map->table[i] = value;
    if (old_node) {
        return old_node;
    }

    map->len++;
    return NULL;
}

QueueNode *hashmap_get(HashMap map, char *key) {
    usize i = find_slot(map, key);
    return map.table[i];
}

QueueNode *hashmap_remove(HashMap *map, char *key) {
    if (!map) {
        return NULL;
    }

    usize i = find_slot(*map, key);
    if (!map->table[i]) {
        return NULL;
    }

    QueueNode *node = map->table[i];
    map->table[i] = NULL;

    usize j = i;
    for (;;) {
        j = (j + 1) % map->size;
        if (!map->table[j]) {
            break;
        }

        usize k = hash(map->table[j]->key) % map->size;
        if (i <= j && i < k && k <= j) {
            continue;
        } else if (i > j && (k <= j || i < k)) {
            continue;
        }

        map->table[i] = map->table[j];
        map->table[j] = NULL;
        i = j;
    }

    map->len--;
    return node;
}

usize hashmap_len(HashMap map) { return map.len; }

void hashmap_del(HashMap *map) {
    if (!map) {
        return;
    }

    for (usize i = 0; i < map->size; i++) {
        queuenode_del(map->table[i]);
    }

    free(map->table);
    *map = (HashMap){0};
}
