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
#include "semantic_analyzer.h"

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

    StringView src = sv_new(src_cstr, length);

    Tokens tokens;
    dyna_init(&tokens);

    if (!lex_program(src, &tokens)) return -1;

    printf("LEXER_START\n");

    for (size_t i = 0; i < tokens.length; i++) {
        Token token = tokens.data[i];
        printf("'"SV_FMT"' (%s aka %s)\n", SV_ARG(token.text), tt_repr(token.type), tt_error_message_name(token.type));
    }

    printf("LEXER_END\n\n");

    Sega_Node nodes;
    sega_init(&nodes);

    Program program;
    dyna_init(&program);

    bool b = parse_program(tokens, &nodes, &program);
    dyna_destroy(tokens);

    if (!b) return -1;

    printf("PARSER START\n");
    program_println(program);
    printf("PARSER END\n\n");

    if (!analyze_program(&program)) return -1;

    printf("SEMANTIC ANALYZER START\n");
    program_println(program);
    printf("SEMANTIC ANALYZER END\n\n");

    dyna_destroy(program);
    sega_destroy(nodes);
    file_unread(src_cstr);
    return 0;
}
