#pragma once

#include <stdio.h>
#include <stdnoreturn.h>

#define TODO do {                               \
        fprintf(stderr, "%s:%d: %s: TODO\n",    \
                __FILE__, __LINE__, __func__);  \
        abort();                                \
    } while (0)

static inline noreturn void unreachable(void) {
    fprintf(stderr, "%s:%d: %s: reached unreachable code\n",
            __FILE__, __LINE__, __func__);
    abort();
}
