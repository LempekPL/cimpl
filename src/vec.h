#ifndef VEC_H
#define VEC_H

#include <stdlib.h>

typedef struct vec {
    size_t count;
    size_t capacity;
} Vec;

#define VEC_INIT 8

#define vec_push(arr, x) \
    do {\
        if ((arr) == NULL) {\
            Vec* header = malloc(sizeof(Vec) + VEC_INIT * sizeof(*(arr)));\
            header->capacity = VEC_INIT;\
            header->count = 0;\
            (arr) = (void*)(header + 1);\
        }\
        Vec* header = (Vec*)(arr) - 1;\
        if (header->capacity <= header->count) {\
            header->capacity *= 2;\
            header = realloc(header, sizeof(Vec) + header->capacity * sizeof(*(arr)));\
            (arr) = (void*)(header + 1);\
        }\
        (arr)[header->count++] = (x);\
    } while (0)

#define vec_pop(arr) (arr)[--((Vec*)(arr) - 1)->count]

#define vec_len(arr) ((Vec*)(arr) - 1)->count

#define vec_free(arr) free(((Vec*)arr - 1))

#endif // VEC_H