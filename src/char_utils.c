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
