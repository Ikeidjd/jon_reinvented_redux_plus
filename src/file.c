#include "file.h"

#include <stdio.h>
#include <stdlib.h>

long file_length(FILE* file) {
    if (file == NULL) return -1;

    long cur = ftell(file);

    fseek(file, 0, SEEK_END);
    long length = ftell(file);

    fseek(file, cur, SEEK_SET);

    return length;
}

char* file_read(const char* file_path) {
    FILE* file = fopen(file_path, "rb");
    if (file == NULL) return NULL;

    long length = file_length(file);

    char* buf = malloc(length + 1);
    fread(buf, sizeof(buf[0]), length, file);
    buf[length] = '\0';

    fclose(file);
    return buf;
}

void file_unread(char* data_read) {
    free(data_read);
}
