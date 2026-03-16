#ifndef UTIL_H
#define UTIL_H
#include <stdio.h>
#include <stdlib.h>

#define OptionNone (Option){.t = OPTION_None}
#define OptionSome(x) (Option){.t = OPTION_Some, .data = (x)}
#define OptionUnwrap(dest, opt) \
    do { \
        Option __unwrap_option = (opt); \
        if (__unwrap_option.t == OPTION_None) { \
            printf("Panic! Unwrapped OptionNone\n"); \
            exit(1); \
        } \
        (dest) = __unwrap_option.data; \
    } while(0)
    

typedef enum {
    OPTION_None,
    OPTION_Some
} OptionType;

typedef struct {
    OptionType t;
    void* data;
} Option;

#define box_value(boxName, x) boxName = malloc(sizeof(void*)); *(boxName) = (x)

void print_err(const char* format, ...);
void print_ext(const char* format, ...);
void fprint_ext(FILE* const stream, const char* const format, ...);

#endif // UTIL_H