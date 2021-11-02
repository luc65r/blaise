#include <assert.h>
#include <stdlib.h>

#include "vec.h"

void vec_push(Vec *v, void *elem) {
    assert(v != NULL);

    if (v->elems == NULL) {
        v->elems = malloc(sizeof elem);
        assert(v->elems != NULL);
        v->elems[0] = elem;
        v->len = 1;
        v->capacity = 1;
    } else {
        assert(v->len <= v->capacity);
        if (v->len >= v->capacity) {
            v->capacity *= 2;
            assert(v->len < v->capacity);
            v->elems = realloc(v->elems, sizeof elem * v->capacity);
            assert(v->elems != NULL);
        }
        v->elems[v->len++] = elem;
    }
}
