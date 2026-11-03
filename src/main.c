#include <stdio.h>

#include <string.h>

#include "file.h"
#include "string_view.h"
#include "dynamic_array.h"
#include "segment_array.h"
#include "token.h"
#include "node.h"
#include "lexer.h"
#include "parser_.h"

static void dyna_test(void) {
    struct {
        int* data;
        size_t length;
        size_t capacity;
    } dyna;

    dyna_init(&dyna);

    for (size_t i = 0; i < 32; i++) {
        dyna_append(&dyna, i);
    }

    for (size_t i = 32; i--;) {
        printf("%d\n", dyna.data[i]);
    }
}

static void sega_test(void) {
    SegmentArray(int) sega;
    sega_init(&sega);

    for (size_t i = 0; i < 512; i++) {
        size_t index_of_segment, index_in_segment;
        sega_get_indices(i, &index_of_segment, &index_in_segment);
        printf("%zu %zu %zu\n", i, index_of_segment, index_in_segment);
    }

    printf("\n");

    for (int i = 0; i < 512; i++) {
        sega_append(&sega, i);
        printf("%p\n", (void*) &sega_get(&sega, i));
    }

    printf("\n");

    for (size_t i = 512; i--;) {
        printf("%d\n", sega_get(&sega, i));
        printf("%p\n", (void*) &sega_get(&sega, i));
    }
}

int main(int argc, char** argv) {
    if (argc == 3 && strcmp(argv[1], "test") == 0) {
        if (strcmp(argv[2], "dyna") == 0) {
            dyna_test();
        } else if (strcmp(argv[2], "sega") == 0) {
            sega_test();
        } else {
            fprintf(stderr, "bro respeta\n");
            return -1;
        }

        return 0;
    }

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

    Sega_Node sega_node;
    sega_init(&sega_node);

    Node* ast = parser_parse(&parser, &sega_node);

    if (!ast) {
        fprintf(stderr, "Something went wrong while parsing.\n");
        return -1;
    }

    node_println(ast);

    sega_destroy(sega_node);
    file_unread(src_cstr);
    return 0;
}
