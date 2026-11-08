#include "char_utils.h"

bool is_ascii_digit(char c) {
    return c >= '0' && c <= '9';
}

bool is_ascii_alpha(char c) {
    return (c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z');
}

bool is_ascii_alnum(char c) {
    return is_ascii_digit(c) || is_ascii_alpha(c);
}

bool is_ascii_whitespace(char c) {
    return c == ' ' || c == '\t' || c == '\r' || c == '\n' || c == '\v' || c == '\f';
}

bool is_hex_digit(char c) {
    return is_ascii_digit(c) || (to_ascii_upper(c) >= 'A' && to_ascii_upper(c) <= 'F');
}

bool is_oct_digit(char c) {
    return c >= '0' && c <= '7';
}

bool is_bin_digit(char c) {
    return c == '0' || c == '1';
}

char to_ascii_lower(char c) {
    return c >= 'A' && c <= 'Z' ? c - 'A' + 'a' : c;
}

char to_ascii_upper(char c) {
    return c >= 'a' && c <= 'z' ? c - 'a' + 'A' : c;
}
