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
            Vec* __h = malloc(sizeof(Vec) + VEC_INIT * sizeof(*(arr)));\
            __h->capacity = VEC_INIT;\
            __h->count = 0;\
            (arr) = (void*)(__h + 1);\
        }\
        Vec* __h = (Vec*)(arr) - 1;\
        if (__h->capacity <= __h->count) {\
            __h->capacity *= 2;\
            __h = realloc(__h, sizeof(Vec) + __h->capacity * sizeof(*(arr)));\
            (arr) = (void*)(__h + 1);\
        }\
        (arr)[__h->count++] = (x);\
    } while (0)

#define vec_pop(arr) (arr)[--((Vec*)(arr) - 1)->count]

#define vec_len(arr) (((arr) != NULL) ? (((Vec*)(arr)) - 1)->count : 0)

#define vec_free(arr) do { if ((Vec*)(arr) != NULL) {free(((Vec*)arr - 1)); (arr) = NULL;}} while(0)

#endif // VEC_H