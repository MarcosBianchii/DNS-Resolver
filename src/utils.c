#include "utils.h"
#include "ints.h"
#include <math.h>
#include <string.h>

Vec split(char *string, char *sep) {
    Vec v = vec_new();
    usize m = strlen(sep);

    char *found = NULL;
    while ((found = strstr(string, sep))) {
        usize len = (usize)(found - string);
        char *part = calloc(len + 1, sizeof(char));
        if (!part) {
            vec_del_with(&v, free);
            return v;
        }

        memcpy(part, string, len);
        vec_push(&v, part);
        string = found + m;
    }

    usize len = strlen(string);
    if (len > 0) {
        char *part = calloc(len + 1, sizeof(char));
        if (!part) {
            vec_del_with(&v, free);
            return v;
        }

        memcpy(part, string, len);
        vec_push(&v, part);
    }

    return v;
}

u8 *join(Vec v, char *sep, usize *size) {
    if (size) {
        *size = 0;
    }

    usize sep_len = strlen(sep);
    usize total_length = (vec_len(v) - 1) * sep_len;

    for (usize i = 0; i < vec_len(v); i++) {
        total_length += strlen((char *)vec_get(v, i));
    }

    u8 *joined = calloc(total_length + 1, sizeof(u8));
    if (!joined) {
        return NULL;
    }

    usize idx = 0;
    for (usize i = 0; i < vec_len(v); i++) {
        usize len = strlen((char *)vec_get(v, i));

        memcpy(&joined[idx], vec_get(v, i), len);
        idx += len;

        if (i < vec_len(v) - 1) {
            memcpy(&joined[idx], sep, sep_len);
            idx += sep_len;
        }
    }

    if (size) {
        *size = total_length;
    }

    return joined;
}

usize digits(int num) { return num == 0 ? 1 : floor(log10(abs(num))) + 1; }
