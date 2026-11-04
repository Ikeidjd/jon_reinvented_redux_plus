#pragma once

#define FPRINTLN_GEN(Type, type) \
static inline void type ## _fprintln(FILE* file, Type type) { \
    type ## _fprint(file, type); \
    fprintf(file, "\n"); \
}

#define PRINT_GEN(Type, type) \
static inline void type ## _print(Type type) { \
    type ## _fprint(stdout, type); \
}

#define PRINTLN_GEN(Type, type) \
static inline void type ## _println(Type type) { \
    type ## _fprintln(stdout, type); \
}

#define PRINT_GEN_ALL(Type, type) \
FPRINTLN_GEN(Type, type) \
PRINT_GEN(Type, type) \
PRINTLN_GEN(Type, type)
