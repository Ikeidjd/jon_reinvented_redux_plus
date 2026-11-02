#pragma once

#include <stddef.h>

typedef struct {
    const char* data;
    size_t length;
} StringView;

// Inspired by Tsoding
#define SV_FMT "%.*s"
#define SV_ARG(sv) (int) (sv).length, (sv).data

#define sv_from_literal(lit) sv_from_str_length(lit, sizeof(lit) / sizeof((lit)[0]))

StringView sv_from_cstr(const char* str);

StringView sv_from_str_length(const char* str, size_t length);

StringView sv_slice(StringView sv, size_t start, size_t length);
