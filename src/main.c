#include <stdio.h>

#include "file.h"

int main(void) {
    char* file = file_read("Hello, world.txt");
    printf("%s", file);
    file_unread(file);
    return 0;
}
