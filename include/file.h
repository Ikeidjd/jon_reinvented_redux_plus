#pragma once

#include "stdio.h"

// Returns the length of a file
// This length does not include a null terminator
// Returns -1 if file == NULL
long file_length(FILE* file);

// Reads an entire file to a null-terminated dynamically allocated string
// Remember to call file_unread on it!
// May return NULL to signal an error
char* file_read(const char* file_path, size_t* bytes_read);

// Deallocates a string returned by file_read
// Really just a wrapper for free
void file_unread(char* data_read);
