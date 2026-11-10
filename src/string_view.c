#include "string_view.h"

#include <stdio.h>
#include <stdlib.h>
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

bool sv_eq(StringView a, StringView b) {
    if (a.length != b.length) return false;
    if (a.data == b.data) return true;

    for (size_t i = 0; i < a.length; i++) {
        if (a.data[i] != b.data[i]) return false;
    }

    return true;
}

bool sv_eq_cstr(StringView sv, const char* cstr) {
    for (size_t i = 0; i < sv.length; i++) {
        if (cstr[i] == '\0' || sv.data[i] != cstr[i]) return false;
    }

    return cstr[sv.length] == '\0';
}

SvToIntResult sv_to_u64(StringView sv, u64* out) {
    u64 n = 0;

    for (size_t i = 0; i < sv.length; i++) {
        if (!is_ascii_digit(sv.data[i])) return SV_TO_INT_INVALID_DIGIT;

        u64 prev = n;
        n *= 10;
        n += sv.data[i] - '0';

        // Overflow
        if (n < prev) return SV_TO_INT_TOO_LARGE;
    }

    *out = n;
    return SV_TO_INT_OK;
}

SvToIntResult sv_hex_to_u64(StringView sv, u64* out) {
    u64 n = 0;

    for (size_t i = 0; i < sv.length; i++) {
        if (!is_hex_digit(sv.data[i])) return SV_TO_INT_INVALID_DIGIT;

        u64 prev = n;
        n *= 16;
        n += is_ascii_digit(sv.data[i]) ? sv.data[i] - '0' : to_ascii_upper(sv.data[i]) + 10 - 'A';

        // Overflow
        if (n < prev) return SV_TO_INT_TOO_LARGE;
    }

    *out = n;
    return SV_TO_INT_OK;
}

SvToIntResult sv_oct_to_u64(StringView sv, u64* out) {
    u64 n = 0;

    for (size_t i = 0; i < sv.length; i++) {
        if (!is_oct_digit(sv.data[i])) return SV_TO_INT_INVALID_DIGIT;

        u64 prev = n;
        n *= 8;
        n += sv.data[i] - '0';

        // Overflow
        if (n < prev) return SV_TO_INT_TOO_LARGE;
    }

    *out = n;
    return SV_TO_INT_OK;
}

SvToIntResult sv_bin_to_u64(StringView sv, u64* out) {
    u64 n = 0;

    for (size_t i = 0; i < sv.length; i++) {
        if (!is_bin_digit(sv.data[i])) return SV_TO_INT_INVALID_DIGIT;

        u64 prev = n;
        n *= 2;
        n += sv.data[i] - '0';

        // Overflow
        if (n < prev) return SV_TO_INT_TOO_LARGE;
    }

    *out = n;
    return SV_TO_INT_OK;
}

bool sv_to_f64(StringView sv, f64* out) {
    char buf[sv.length + 1];
    memcpy(buf, sv.data, sv.length);
    buf[sv.length] = '\0';

    char* end_ptr;
    double d = strtod(buf, &end_ptr);

    if (end_ptr != buf + sv.length) return false;

    *out = d;
    return true;
}
