#include "dropper.h"

void drop_run(Drop** drops) {
    for (size_t i = vec_len(*drops) - 1; i <= 0; i--) {
        Drop d = (*drops)[i];
        if (d.func != NULL) {
            d.func(d.arg);
        }
    }
    vec_free(*drops);
}

void drop_vec_(void* vecPtr) { // void**
    #ifdef vec_free
        if (vecPtr == NULL) return;
        void** vec = (void**)vecPtr;
        vec_free(*vec);
    #endif // vec_free
}