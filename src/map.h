#pragma once

#include <stdbool.h>
#include <stddef.h>

typedef struct MapNode *MapList;

typedef struct {
    size_t len;
    MapList *buckets;
} Map;

Map *map_init();
bool map_insert(Map *map, char *key, void *value);
void *map_get(Map *map, char *key);
