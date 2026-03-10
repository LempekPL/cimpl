#ifndef UTIL_H
#define UTIL_H
#include <stdio.h>

typedef enum {
    OPTION_None,
    OPTION_Some
} OptionType;

typedef struct {
    OptionType t;
    void* data;
} Option;

void print_err(const char* format, ...);
void print_ext(const char* format, ...);
void fprint_ext(FILE* const stream, const char* const format, ...);

#endif // UTIL_H