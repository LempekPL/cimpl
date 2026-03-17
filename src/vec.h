#ifndef VEC_H
#define VEC_H

#include <stdlib.h>

typedef struct vec_header_t {
    size_t count;
    size_t capacity;
} VecHeader;

typedef struct vec_for_each_data_t {
    void* element;
    size_t i;
    void* data;
} VecFEData;

typedef void (*Vec_ForEach_Func)(VecFEData data);

#define VEC_INIT 8

#define vec_new(name, size) \
    name = NULL; \
    do { \
        VecHeader* __h = malloc(sizeof(VecHeader) + (size) * sizeof(*(name)));\
        __h->capacity = 2;\
        __h->count = 0;\
        (name) = (void*)(__h + 1);\
    } while(0)

#define vec_push(vec, x) \
    do {\
        if ((vec) == NULL) {\
            VecHeader* __h = malloc(sizeof(VecHeader) + VEC_INIT * sizeof(*(vec)));\
            __h->capacity = VEC_INIT;\
            __h->count = 0;\
            (vec) = (void*)(__h + 1);\
        }\
        VecHeader* __h = (VecHeader*)(vec) - 1;\
        if (__h->capacity <= __h->count) {\
            __h->capacity *= 2;\
            __h = realloc(__h, sizeof(VecHeader) + __h->capacity * sizeof(*(vec)));\
            (vec) = (void*)(__h + 1);\
        }\
        (vec)[__h->count++] = (x);\
    } while (0)

#define vec_pop(vec) (vec)[--((VecHeader*)(vec) - 1)->count]
#define vec_len(vec) (((vec) != NULL) ? (((VecHeader*)(vec)) - 1)->count : 0)
#define vec_first(vec) (vec)[0]
#define vec_last(vec) (vec)[ven_len(vec) - 1]
#define vec_free(vec) do { if ((VecHeader*)(vec) != NULL) {free(((VecHeader*)vec - 1)); (vec) = NULL;}} while(0)
#define vec_reverse(vec, type) \
    do { \
        for (size_t i = 0; i < vec_len(vec)/2; i++) { \
            type tmp = vec[i]; \
            vec[i] = vec[vec_len(vec)-1-i]; \
            vec[vec_len(vec)-1-i] = tmp; \
        } \
    } while(0)

#define vec_concat(dest, src) \
    do { \
        for (size_t i = 0; i < vec_len(src); i++) { \
            vec_push(dest, src[i]); \
        } \
        vec_free(src); \
    } while(0)

#define vec_arr_append(vec, arr, size) \
    do { \
        for (size_t i = 0; i < (size); i++) { \
            vec_push((vec), (arr)[i]); \
        } \
    } while(0)

void vec_for_each_fn_(void* vec, size_t typesize, Vec_ForEach_Func func, void* data);

#ifdef VEC_EXTRAS

#define vec_for_each_fn(vec, func, data) vec_for_each_fn_(vec, sizeof(*vec), func, data)

void vec_for_each_fn_(void* vec, size_t typesize, Vec_ForEach_Func func, void* data) {
    for (size_t i = 0; i < vec_len(vec); i++) { 
        func((VecFEData){.i = i, .element = &((vec)[i*typesize]), .data = data}); 
    } 
}

#define vec_for_each(vec, iname, type, name, code) \
    do { \
        for (size_t iname = 0; iname < vec_len(vec); iname++) { \
            type name = (vec)[iname]; \
            do { \
                code \
            } while(0); \
        } \
    } while(0)

#endif


#endif // VEC_H