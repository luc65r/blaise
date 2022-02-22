#include <assert.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#include "map.h"

#define CAPACITY 1024

struct MapNode {
    char *key;
    void *value;
    MapList next;
};

static size_t hash(char *str) {
    size_t hash = 5381;
    for (; *str; str++)
        hash = hash * 33 ^ *str;
    return hash;
}

static void mapl_push(MapList *l, char *key, void *value) {
    assert(l != NULL);

    MapList n = malloc(sizeof *n);
    assert(n != NULL);
    n->key = key;
    n->value = value;
    n->next = *l;
    *l = n;
}

static void *mapl_find(MapList l, char *key) {
    for (; l; l = l->next)
        if (strcmp(key, l->key) == 0)
            return l->value;
    return NULL;
}

Map *map_init() {
    Map *map = calloc(1, sizeof *map);
    assert(map != NULL);

    map->buckets = calloc(CAPACITY, sizeof *map->buckets);
    assert(map->buckets != NULL);

    return map;
}

bool map_insert(Map *map, char *key, void *value) {
    assert(map != NULL);

    size_t h = hash(key) % CAPACITY;
    MapList *l = &map->buckets[h];
    if (mapl_find(*l, key) != NULL)
        return false;
    mapl_push(l, key, value);
    map->len++;
    return true;
}

void *map_get(Map *map, char *key) {
    assert(map != NULL);

    size_t h = hash(key) % CAPACITY;
    return mapl_find(map->buckets[h], key);
}

MapIterator map_iter(Map *map) {
    return (MapIterator){
        .map = map,
        .h = 0,
        .node = map->buckets[0],
    };
}

bool map_iter_next(MapIterator *iter, char **key, void **value) {
    struct MapNode *node = iter->node;
    if (node != NULL)
        node = node->next;

    while (node == NULL && iter->h < CAPACITY)
        node = iter->map->buckets[++iter->h];

    iter->node = node;
    if (node == NULL)
        return false;

    if (key != NULL)
        *key = node->key;
    if (value != NULL)
        *value = node->value;

    return true;
}
