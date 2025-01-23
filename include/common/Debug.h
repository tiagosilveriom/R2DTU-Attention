#pragma once

#include <stdio.h>
#include <stdlib.h>


#define ASSERTF(predicate, msg, ...)      \
    if (!(predicate)) {                   \
        printf(msg, __VA_ARGS__);         \
        std::abort();                     \
    }

#define ASSERT(predicate, msg)            \
    if (!(predicate)) {                   \
        printf(msg);                      \
        std::abort();                     \
    }


