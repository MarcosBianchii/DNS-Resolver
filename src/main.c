#include "dns.h"
#include "lib.h"
#include <stdio.h>
#include <time.h>

int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "use: %s <domain_name>\n", argv[0]);
        return 1;
    }

    srand(time(NULL));
    char *domain_name = argv[1];
    char *ip = resolve(domain_name, TYPE_A);
    if (!ip) {
        fprintf(stderr, "couldn't resolve query\n");
        return 1;
    }

    puts(ip);
    free(ip);
    return 0;
}
