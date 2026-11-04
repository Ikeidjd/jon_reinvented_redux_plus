#include "string_view.h"

#include <limits.h>
#include <string.h>

#include "char_utils.h"

StringView sv_cstr(const char* str) {
    return sv_new(str, strlen(str));
}

StringView sv_new(const char* str, size_t length) {
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

StringView sv_first_line(StringView sv) {
    size_t i = 0;

    for (; i < sv.length; i++) {
        if (sv.data[i] == '\n') break;
    }

    return sv_slice_until(sv, i);
}

StringView sv_trim_left(StringView sv) {
    size_t start = 0;

    for (; start < sv.length; start++) {
        if (!is_ascii_whitespace(sv.data[start])) break;
    }

    return sv_slice_from(sv, start);
}

StringView sv_trim_right(StringView sv) {
    size_t length = sv.length;

    for (; length > 0; length--) {
        if (!is_ascii_whitespace(sv.data[length - 1])) break;
    }

    return sv_slice_until(sv, length);
}

StringView sv_trim(StringView sv) {
    return sv_trim_left(sv_trim_right(sv));
}

bool sv_eq_cstr(StringView sv, const char* cstr) {
    for (size_t i = 0; i < sv.length; i++) {
        if (cstr[i] == '\0' || sv.data[i] != cstr[i]) return false;
    }

    return cstr[sv.length] == '\0';
}

bool sv_to_int(StringView sv, int* out) {
    bool negative = sv.data[0] == '-';
    if (negative) sv = sv_slice_from(sv, 1);

    long long n = 0;

    for (size_t i = 0; i < sv.length; i++) {
        if (sv.data[i] < '0' || sv.data[i] > '9') return false;

        n *= 10;
        n += (negative ? -1 : 1) * (sv.data[i] - '0');

        if (n < INT_MIN || n > INT_MAX) return false;
    }

    *out = (int) n;
    return true;
}
