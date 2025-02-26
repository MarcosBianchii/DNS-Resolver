#ifndef __VEC__
#define __VEC__

#include "ints.h"
#include <stdlib.h>

typedef struct Vec {
    void **data;
    usize len;
    usize cap;
} Vec;

Vec vec_new();
Vec vec_with_capacity(usize cap);

void vec_push(Vec *v, void *x);
void vec_insert(Vec *v, void *x, usize pos);
void *vec_pop(Vec *v);
void *vec_remove(Vec *v, usize pos);
void *vec_get(Vec v, usize pos);

usize vec_len(Vec v);
usize vec_cap(Vec v);

void vec_del(Vec *v);
void vec_del_with(Vec *v, void (*f)(void *));

#endif // __VEC__
