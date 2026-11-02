#include <stdio.h>

#include "file.h"
#include "token.h"
#include "lexer.h"

int main(int argc, char** argv) {
    if (argc != 2) {
        fprintf(stderr, "Usage: jon <file_path>");
        return -1;
    }

    const char* file_path = argv[1];

    size_t length;
    char* file = file_read(file_path, &length);

    if (file == NULL) {
        fprintf(stderr, "Could not read file '%s'.", file_path);
        return -1;
    }

    printf("FILE_START\n%s\nFILE_END\n\n", file);

    Lexer lexer;
    lexer_init(&lexer, sv_from_str_length(file, length));

    Token token;
    while (lexer_next(&lexer, &token)) {
        printf("'"SV_FMT"' (%s aka '%s')\n", SV_ARG(token.text), tt_repr(token.type), tt_str(token.type));
        if (token.type == TT_EOF) break;
    }

    file_unread(file);
    return 0;
}
