#pragma once

#include <stdbool.h>
#include <stddef.h>

typedef struct HMNode *HMList;

typedef struct {
    size_t len;
    HMList *buckets;
} HashMap;

HashMap *hm_init();
bool hm_insert(HashMap *hm, char *key, void *value);
void *hm_get(HashMap *hm, char *key);
