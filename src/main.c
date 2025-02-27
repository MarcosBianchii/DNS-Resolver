#include "dns.h"
#include "lib.h"
#include "lru.h"
#include <stdio.h>
#include <string.h>
#include <time.h>

int main(int argc, char *argv[]) {
    usize cache_size = 128;

    if (argc > 2) {
        int size = atoi(argv[1]);
        if (size < 2) {
            fprintf(stderr, "invalid cache size");
            return 1;
        }

        cache_size = (usize)size;
    }

    LruCache cache = lru_new(cache_size);
    char *line = malloc(64 * sizeof(char));
    if (!line) {
        lru_del(&cache);
        return 1;
    }

    srand(time(NULL));
    isize nread = 0;
    usize n = 128;
    for (;;) {
        printf("domain> ");
        nread = getline(&line, &n, stdin);
        line[nread - 1] = '\0';

        if (strcmp(line, "quit") == 0) {
            break;
        }

        char *ip = lru_get(&cache, line);
        if (!ip) {
            ip = resolve(line, TYPE_A, &cache);
            lru_push(&cache, line, ip);
        }

        if (ip) {
            printf("%s\n", ip);
        } else {
            fprintf(stderr, "couldn't resolve query for %s\n", line);
        }

        puts("");
    }

    free(line);
    lru_del(&cache);
    return 0;
}
