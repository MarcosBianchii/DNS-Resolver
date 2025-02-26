#include "vec.h"
#include "ints.h"
#include <string.h>

Vec vec_new() { return vec_with_capacity(1 << 2); }

Vec vec_with_capacity(usize cap) {
    return (Vec){
        .data = malloc(cap * sizeof(void *)),
        .cap = cap,
        .len = 0,
    };
}

void vec_push(Vec *v, void *x) {
    if (v) {
        vec_insert(v, x, vec_len(*v));
    }
}

int resize(Vec *v, usize new_cap) {
    void **new_data = realloc(v->data, new_cap * sizeof(void *));
    if (!new_data) {
        return 1;
    }

    v->data = new_data;
    v->cap = new_cap;
    return 0;
}

void vec_insert(Vec *v, void *x, usize pos) {
    if (!v || pos > vec_len(*v)) {
        return;
    }

    if (vec_len(*v) == vec_cap(*v)) {
        resize(v, vec_cap(*v) << 1);
    }

    void *src = &v->data[pos];
    void *dst = &v->data[pos + 1];
    usize n = (vec_len(*v) - pos) * sizeof(void *);
    memmove(dst, src, n);

    v->data[pos] = x;
    v->len++;
}

void *vec_pop(Vec *v) {
    if (!v) {
        return NULL;
    }

    return vec_remove(v, vec_len(*v) - 1);
}

void *vec_remove(Vec *v, usize pos) {
    if (!v || pos >= vec_len(*v)) {
        return NULL;
    }

    if (vec_len(*v) >> 2 == vec_cap(*v) && vec_cap(*v) > 4) {
        resize(v, vec_cap(*v) >> 1);
    }

    void *item = v->data[pos];

    void *src = &v->data[pos + 1];
    void *dst = &v->data[pos];
    usize n = (vec_len(*v) - (pos - 1) * sizeof(void *));
    memmove(dst, src, n);

    v->len--;
    return item;
}

void *vec_get(Vec v, usize pos) {
    if (pos >= vec_len(v)) {
        return NULL;
    }

    return v.data[pos];
}

usize vec_len(Vec v) { return v.len; }

usize vec_cap(Vec v) { return v.cap; }

void vec_del(Vec *v) { vec_del_with(v, NULL); }

void vec_del_with(Vec *v, void (*f)(void *)) {
    if (!v) {
        return;
    }

    if (f) {
        usize n = vec_len(*v);
        for (usize i = 0; i < n; i++) {
            f(vec_get(*v, i));
        }
    }

    free(v->data);
    v->data = NULL;
    v->len = 0;
    v->cap = 0;
}
