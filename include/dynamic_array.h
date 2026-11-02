#pragma once

#define DYNA_INITIAL_CAPACITY 16

#include <stdlib.h>

#define dyna_init(dyna) \
do { \
    (dyna)->data = malloc(DYNA_INITIAL_CAPACITY * sizeof((dyna)->data[0])); \
    (dyna)->length = 0; \
    (dyna)->capacity = DYNA_INITIAL_CAPACITY; \
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
