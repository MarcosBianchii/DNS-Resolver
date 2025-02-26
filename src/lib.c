#include "lib.h"
#include "utils.h"
#include <arpa/inet.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

u8 *build_query(char *domain_name, Record type, usize *size) {
    if (size) {
        *size = 0;
    }

    u8 *header = header_to_bytes((DnsHeader){
        .id = rand() % (1 << (8 * sizeof(u16) - 1)),
        .flags = 0,
        .qdcount = 1,
        .ancount = 0,
        .arcount = 0,
        .nscount = 0,
    });

    if (!header) {
        return NULL;
    }

    usize qnamelen = 0;
    u8 *name = encode_dns_name(domain_name, &qnamelen);
    DnsQuestion q = (DnsQuestion){
        .name = name,
        .namelen = qnamelen,
        .type = type,
        .class_ = ClassIn,
    };

    usize q_size = 0;
    u8 *question = question_to_bytes(q, &q_size);
    free(name);

    if (!question) {
        free(header);
        return NULL;
    }

    u8 *encoding = realloc(header, HDR_SIZE + q_size);
    if (!encoding) {
        free(question);
        return NULL;
    }

    memcpy(&encoding[HDR_SIZE], question, q_size);
    free(question);

    if (size) {
        *size = HDR_SIZE + q_size;
    }

    return encoding;
}

u8 *encode_dns_name(char *domain_name, usize *namelen) {
    if (namelen) {
        *namelen = 0;
    }

    Vec parts = split(domain_name, ".");
    Vec lengths = vec_with_capacity(vec_len(parts));
    usize total_size = 1;

    for (usize i = 0; i < vec_len(parts); i++) {
        usize len = strlen((char *)vec_get(parts, i));
        vec_push(&lengths, (void *)len);
        total_size += len + 1;
    }

    u8 *bytes = calloc(total_size, sizeof(u8));
    if (!bytes) {
        vec_del_with(&parts, free);
        vec_del(&lengths);
        return NULL;
    }

    usize idx = 0;
    for (usize i = 0; i < vec_len(parts); i++) {
        char *part = (char *)vec_get(parts, i);
        usize len = (usize)vec_get(lengths, i);

        bytes[idx++] = (u8)len;
        memcpy(&bytes[idx], part, len);
        idx += len;
    }

    vec_del_with(&parts, free);
    vec_del(&lengths);

    if (namelen) {
        *namelen = total_size;
    }

    return bytes;
}

DnsPacket send_query(char *ip_addr, char *domain_name, Record type) {
    DnsPacket invalid = (DnsPacket){0};
    int skt = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (skt < 0) {
        fprintf(stderr, "couldn't open a socket\n");
        return invalid;
    }

    usize size = 0;
    u8 *query = build_query(domain_name, TYPE_A, &size);
    if (!query || size == 0) {
        fprintf(stderr, "error while creating dns query\n");
        close(skt);
        return invalid;
    }

    struct sockaddr_in servaddr = (struct sockaddr_in){
        .sin_addr.s_addr = inet_addr(ip_addr),
        .sin_port = htons(53),
        .sin_family = AF_INET,
    };

    isize sent = sendto(skt, query, size, 0, (struct sockaddr *)&servaddr,
                        sizeof(servaddr));
    free(query);
    if (sent < 0) {
        fprintf(stderr, "unable to send query\n");
        close(skt);
        return invalid;
    }

    u8 response[1028] = {0};
    isize read = recv(skt, response, sizeof(response), 0);
    close(skt);

    if (read < 0) {
        fprintf(stderr, "error while receiving server's response");
        return invalid;
    }

    return parse_dns_packet(response);
}

char *get_answer(DnsPacket packet) {
    Vec answers = packet.answers;
    for (usize i = 0; i < vec_len(answers); i++) {
        DnsRecord *record = vec_get(answers, i);
        if (record->type == TYPE_A) {
            return (char *)record->data;
        }
    }

    return NULL;
}

char *get_nameserver_ip(DnsPacket packet) {
    Vec additionals = packet.additionals;
    for (usize i = 0; i < vec_len(additionals); i++) {
        DnsRecord *record = vec_get(additionals, i);
        if (record->type == TYPE_A) {
            return (char *)record->data;
        }
    }

    return NULL;
}

char *get_nameserver(DnsPacket packet) {
    Vec authorities = packet.authorities;
    for (usize i = 0; i < vec_len(authorities); i++) {
        DnsRecord *record = vec_get(authorities, i);
        if (record->type == TYPE_NS) {
            return (char *)record->data;
        }
    }

    return NULL;
}

char *resolve(char *domain_name, Record type) {
    char *nameserver = strdup("198.41.0.4");

    for (;;) {
        printf("Querying %s for %s\n", nameserver, domain_name);
        DnsPacket response = send_query(nameserver, domain_name, type);
        char *ip = get_answer(response);
        char *nsip = get_nameserver_ip(response);
        char *nsdomain = get_nameserver(response);
        free(nameserver);

        if (ip) {
            ip = strdup(ip);
            packet_del(response);
            return ip;
        } else if (nsip) {
            nameserver = strdup(nsip);
        } else if (nsdomain) {
            nameserver = resolve(nsdomain, TYPE_A);
        } else {
            break;
        }

        packet_del(response);
    }

    return NULL;
}
