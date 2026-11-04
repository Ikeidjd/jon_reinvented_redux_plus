#pragma once

#define DYNA_INITIAL_CAPACITY 16

#include <stdlib.h>

#define DynamicArray(type) \
struct {\
    type* data; \
    size_t length; \
    size_t capacity; \
}

#define dyna_init_with_capacity(dyna, initial_capacity) \
do { \
    (dyna)->data = malloc((initial_capacity) * sizeof((dyna)->data[0])); \
    (dyna)->length = 0; \
    (dyna)->capacity = (initial_capacity); \
} while(0)

#define dyna_init(dyna) dyna_init_with_capacity(dyna, DYNA_INITIAL_CAPACITY)

#define dyna_destroy(dyna) \
do { \
    free((dyna).data); \
    (dyna).length = 0; \
    (dyna).capacity = 0; \
} while(0)

#define dyna_append(dyna, elem) \
do { \
    if ((dyna)->length >= (dyna)->capacity) { \
        (dyna)->capacity *= 2; \
        (dyna)->data = realloc((dyna)->data, (dyna)->capacity * sizeof((dyna)->data[0])); \
    } \
\
    (dyna)->data[(dyna)->length++] = elem; \
} while (0)
