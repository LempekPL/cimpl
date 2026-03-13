#ifndef DROPPER_H
#define DROPPER_H

#include <stdlib.h>
#include "vec.h"

typedef void (*DropFunc)(void**);

typedef struct {
    DropFunc func;
    void** arg;
} Drop;

#define drop_new(name) __attribute__((cleanup(drop_run))) Drop* (name) = NULL
#define drop_push(drops, func, value) do { vec_push((drops), ((Drop){(func), (void**)(value)})); } while (0)
#define drop_free(drops, ptr) do { vec_push((drops), ((Drop){drop_free_, (void**)(ptr)})); } while (0)
#define drop_vec(drops, value) do { vec_push((drops), ((Drop){drop_vec_, (void**)(value)})); } while (0)

void drop_run(Drop** drops);
void drop_vec_(void** vec);
void drop_free_(void** ptrV);

#ifdef DROP_IMPLEMENTATION

void drop_run(Drop** drops) {
    for (ssize_t i = vec_len(*drops) - 1; i >= 0; i--) {
        Drop d = (*drops)[i];
        if (d.func != NULL) {
            d.func(d.arg);
        }
    }
    vec_free(*drops);
}

void drop_free_(void** ptrV) {
    if (ptrV == NULL) return;
    void** ptr = (void**)ptrV;
    if (*ptr == NULL) return;
    free(*ptr);
}

void drop_vec_(void** vecPtr) {
    if (vecPtr == NULL) return;
    size_t** vec = (size_t**)vecPtr;
    vec_free(*vec);
}

#endif // DROP_IMPLEMENTATION

#endif // DROPPER_H