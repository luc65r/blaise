#pragma once

#include <stdio.h>
#include <stdnoreturn.h>

#define _PARENS ()

#define _EXPAND(...) _EXPAND4(_EXPAND4(_EXPAND4(_EXPAND4(__VA_ARGS__))))
#define _EXPAND4(...) _EXPAND3(_EXPAND3(_EXPAND3(_EXPAND3(__VA_ARGS__))))
#define _EXPAND3(...) _EXPAND2(_EXPAND2(_EXPAND2(_EXPAND2(__VA_ARGS__))))
#define _EXPAND2(...) _EXPAND1(_EXPAND1(_EXPAND1(_EXPAND1(__VA_ARGS__))))
#define _EXPAND1(...) __VA_ARGS__

#define FOR_EACH(M, ...)                                    \
    __VA_OPT__(_EXPAND(_FOR_EACH_HELPER(M, __VA_ARGS__)))
#define _FOR_EACH_HELPER(M, A, ...)                             \
    M(A)                                                        \
    __VA_OPT__(_FOR_EACH_AGAIN _PARENS (M, __VA_ARGS__))
#define _FOR_EACH_AGAIN() _FOR_EACH_HELPER

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
