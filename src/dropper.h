#ifndef DROPPER_H
#define DROPPER_H

#include <stdlib.h>
#include "vec.h"

typedef void (*DropFunc)(void*);

typedef struct {
    DropFunc func;
    void* arg;
} Drop;



#define drop_new(name) __attribute__((cleanup(drop_run))) Drop* (name) = NULL
#define drop_free(drops, ptr) do { vec_push((drops), ((Drop){free, (ptr)})); } while (0)
#define drop_push(drops, func, value) do { vec_push((drops), ((Drop){(func), (value)})); } while (0)
#define drop_vec(drops, value) do { vec_push((drops), ((Drop){(drop_vec_), (value)})); } while (0)

void drop_run(Drop** drops);
void drop_vec_(void* vec); // void**

#endif // DROPPER_H