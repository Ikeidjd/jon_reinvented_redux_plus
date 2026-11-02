#include "string_view.h"

#include <limits.h>
#include <string.h>

StringView sv_from_cstr(const char* str) {
    return sv_from_str_length(str, strlen(str));
}

StringView sv_from_str_length(const char* str, size_t length) {
    return (StringView) {
        .data = str,
        .length = length,
    };
}

StringView sv_slice(StringView sv, size_t start, size_t length) {
    return (StringView) {
        .data = sv.data + start,
        .length = length,
    };
}

StringView sv_slice_from(StringView sv, size_t start) {
    return sv_slice(sv, start, sv.length - start);
}

StringView sv_slice_until(StringView sv, size_t length) {
    return sv_slice(sv, 0, length);
}

bool sv_to_int(StringView sv, int* out) {
    bool negative = sv.data[0] == '-';
    if (negative) sv = sv_slice_from(sv, 1);

    long long n = 0;

    for (size_t i = 0; i < sv.length; i++) {
        if (sv.data[i] < '0' || sv.data[i] > '9') return false;

        n *= 10;
        n += sv.data[i] - '0';

        if (n < INT_MIN || n > INT_MAX) return false;
    }

    *out = (int) n;
    return true;
}
