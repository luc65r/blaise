#pragma once

#include <stddef.h>

#define VEC_EMPTY (Vec){ 0, 0, 0}

typedef struct {
    size_t len;
    size_t capacity;
    void **elems;
} Vec;

void vec_push(Vec *v, void *elem);
