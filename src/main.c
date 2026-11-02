#include <stdio.h>

#include "file.h"
#include "string_view.h"
#include "token.h"
#include "lexer.h"
#include "parser_.h"

int main(int argc, char** argv) {
    if (argc != 2) {
        fprintf(stderr, "Usage: jon <file_path>");
        return -1;
    }

    const char* file_path = argv[1];

    size_t length;
    char* src_cstr = file_read(file_path, &length);

    if (src_cstr == NULL) {
        fprintf(stderr, "Could not read file '%s'.", file_path);
        return -1;
    }

    printf("FILE_START\n%s\nFILE_END\n\n", src_cstr);

    StringView src = sv_from_str_length(src_cstr, length);

    Lexer lexer;
    lexer_init(&lexer, src);

    Token token;
    while (lexer_next(&lexer, &token)) {
        printf("'"SV_FMT"' (%s aka '%s')\n", SV_ARG(token.text), tt_repr(token.type), tt_str(token.type));
        if (token.type == TT_EOF) break;
    }

    printf("\n");

    Parser parser;
    parser_init(&parser, src);

    int result;
    if (!parser_parse(&parser, &result)) {
        fprintf(stderr, "Something went wrong while parsing.\n");
        return -1;
    }

    printf("%d\n", result);

    file_unread(src_cstr);
    return 0;
}
