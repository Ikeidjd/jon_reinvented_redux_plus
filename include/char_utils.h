#pragma once

#include <stdbool.h>

bool is_ascii_digit(char c);

bool is_ascii_alpha(char c);

bool is_ascii_alnum(char c);

bool is_ascii_whitespace(char c);

bool is_hex_digit(char c);

bool is_oct_digit(char c);

bool is_bin_digit(char c);

char to_ascii_lower(char c);

char to_ascii_upper(char c);
