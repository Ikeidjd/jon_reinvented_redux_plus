#include "math_utils.h"

size_t log2sz(size_t n) {
    size_t out = 0;
    while (n >>= 1) out++;
    return out;
}
