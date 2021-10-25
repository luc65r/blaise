#include <assert.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#include "hashmap.h"

#define CAPACITY 1024

struct HMNode {
    char *key;
    void *value;
    HMList next;
};

static size_t hash(char *str) {
    size_t hash = 5381;
    for (; *str; str++)
        hash = hash * 33 ^ *str;
    return hash;
}

static void hml_push(HMList *l, char *key, void *value) {
    assert(l != NULL);

    HMList n = malloc(sizeof *n);
    assert(n != NULL);
    n->key = key;
    n->value = value;
    n->next = *l;
    *l = n;
}

static void *hml_find(HMList l, char *key) {
    for (; l; l = l->next)
        if (strcmp(key, l->key) == 0)
            return l->value;
    return NULL;
}

HashMap *hm_init() {
    HashMap *hm = calloc(1, sizeof *hm);
    assert(hm != NULL);

    hm->buckets = calloc(CAPACITY, sizeof *hm->buckets);
    assert(hm->buckets != NULL);

    return hm;
}

bool hm_insert(HashMap *hm, char *key, void *value) {
    assert(hm != NULL);

    size_t h = hash(key) % CAPACITY;
    HMList *l = &hm->buckets[h];
    if (hml_find(*l, key) != NULL)
        return false;
    hml_push(l, key, value);
    hm->len++;
    return true;
}

void *hm_get(HashMap *hm, char *key) {
    assert(hm != NULL);

    size_t h = hash(key) % CAPACITY;
    return hml_find(hm->buckets[h], key);
}
