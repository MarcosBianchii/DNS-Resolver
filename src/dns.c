#include "dns.h"
#include "utils.h"
#include <arpa/inet.h>
#include <stdbool.h>
#include <string.h>

u8 *header_to_bytes(DnsHeader h) {
    u8 *bytes = malloc(HDR_SIZE);
    if (!bytes) {
        return NULL;
    }

    u16 fields[] = {
        htons(h.id),      htons(h.flags),   htons(h.qdcount),
        htons(h.ancount), htons(h.nscount), htons(h.arcount),
    };

    memcpy(bytes, fields, HDR_SIZE);
    return bytes;
}

u8 *question_to_bytes(DnsQuestion q, usize *size) {
    if (size) {
        *size = 0;
    }

    usize q_size = q.namelen + 2 * sizeof(u16);
    u8 *bytes = malloc(q_size);
    if (!bytes) {
        return NULL;
    }

    u16 fields[] = {
        htons(q.class_),
        htons(q.type),
    };

    usize restsize = q_size - q.namelen;
    memcpy(bytes, q.name, q.namelen);
    memcpy(&bytes[q.namelen], fields, restsize);

    if (size) {
        *size = q_size;
    }

    return bytes;
}

DnsHeader parse_header(u8 *bytes, usize *read) {
    DnsHeader tmp = *(DnsHeader *)&bytes[*read];
    *read += HDR_SIZE;

    return (DnsHeader){
        .id = ntohs(tmp.id),
        .flags = ntohs(tmp.flags),
        .qdcount = ntohs(tmp.qdcount),
        .ancount = ntohs(tmp.ancount),
        .arcount = ntohs(tmp.arcount),
        .nscount = ntohs(tmp.nscount),
    };
}

u8 *parse_dns_name(u8 *bytes, usize *read, usize *namelen, usize jumps) {
    if (jumps > MAX_COMPRESSION_PTRS) {
        return NULL;
    }

    Vec parts = vec_new();
    bool jumped = false;

    while (!jumped && bytes[*read] != '\0') {
        u8 len = bytes[(*read)++];

        u8 *part = NULL;
        if (len & 0xC0) {
            usize ptr = ((len & 0x3F) << 8) | bytes[(*read)];
            part = parse_dns_name(bytes, &ptr, NULL, jumps + 1);
            jumped = true;
        } else {
            part = (u8 *)strndup((char *)&bytes[*read], len);
            *read += len;
        }

        if (!part) {
            vec_del_with(&parts, free);
            return NULL;
        }

        vec_push(&parts, part);
    }

    (*read)++;
    u8 *name = join(parts, ".", namelen);
    vec_del_with(&parts, free);
    return name;
}

DnsQuestion parse_question(u8 *bytes, usize *read) {
    usize namelen = 0;
    u8 *name = parse_dns_name(bytes, read, &namelen, 0);
    if (!name) {
        return (DnsQuestion){0};
    }

    u16 type;
    memcpy(&type, &bytes[*read], sizeof(type));
    *read += sizeof(type);
    type = ntohs(type);

    u16 class_;
    memcpy(&class_, &bytes[*read], sizeof(class_));
    *read += sizeof(class_);
    class_ = ntohs(class_);

    return (DnsQuestion){
        .name = name,
        .type = type,
        .class_ = class_,
        .namelen = namelen,
    };
}

DnsRecord parse_record(u8 *bytes, usize *read) {
    usize namelen = 0;
    u8 *name = parse_dns_name(bytes, read, &namelen, 0);
    if (!name) {
        return (DnsRecord){0};
    }

    u16 type;
    memcpy(&type, &bytes[*read], sizeof(type));
    *read += sizeof(type);
    type = ntohs(type);

    u16 class_;
    memcpy(&class_, &bytes[*read], sizeof(class_));
    *read += sizeof(class_);
    class_ = ntohs(class_);

    u32 ttl;
    memcpy(&ttl, &bytes[*read], sizeof(ttl));
    *read += sizeof(ttl);
    ttl = ntohl(ttl);

    u16 datalen;
    memcpy(&datalen, &bytes[*read], sizeof(datalen));
    *read += sizeof(datalen);
    datalen = ntohs(datalen);

    u8 *data = NULL;
    if (type == TYPE_NS || type == TYPE_CNAME) {
        data = parse_dns_name(bytes, read, NULL, 0);
    } else if (type == TYPE_A) {
        struct in_addr addr;
        memcpy(&addr, &bytes[*read], sizeof(addr));
        *read += sizeof(addr);
        data = (u8 *)strdup(inet_ntoa(addr));
    } else {
        data = calloc(datalen + 1, sizeof(u8));
        if (!data) {
            free(name);
            return (DnsRecord){0};
        }

        memcpy(data, &bytes[*read], datalen);
        *read += datalen;
    }

    if (!data) {
        return (DnsRecord){0};
    }

    return (DnsRecord){
        .name = name,
        .namelen = namelen,
        .data = data,
        .datalen = datalen,
        .type = type,
        .class_ = class_,
        .ttl = ttl,
    };
}

DnsPacket parse_dns_packet(u8 *bytes) {
    usize read = 0;
    DnsHeader header = parse_header(bytes, &read);

    Vec questions = vec_with_capacity(header.qdcount);
    for (usize i = 0; i < vec_cap(questions); i++) {
        DnsQuestion *question = malloc(sizeof(DnsQuestion));
        *question = parse_question(bytes, &read);
        vec_push(&questions, question);
    }

    Vec answers = vec_with_capacity(header.ancount);
    for (usize i = 0; i < vec_cap(answers); i++) {
        DnsRecord *record = malloc(sizeof(DnsRecord));
        *record = parse_record(bytes, &read);
        vec_push(&answers, record);
    }

    Vec authorities = vec_with_capacity(header.nscount);
    for (usize i = 0; i < vec_cap(authorities); i++) {
        DnsRecord *record = malloc(sizeof(DnsRecord));
        *record = parse_record(bytes, &read);
        vec_push(&authorities, record);
    }

    Vec additionals = vec_with_capacity(header.arcount);
    for (usize i = 0; i < vec_cap(additionals); i++) {
        DnsRecord *record = malloc(sizeof(DnsRecord));
        *record = parse_record(bytes, &read);
        vec_push(&additionals, record);
    }

    return (DnsPacket){
        .header = header,
        .questions = questions,
        .answers = answers,
        .authorities = authorities,
        .additionals = additionals,
    };
}

void question_del(void *q) {
    if (q) {
        free(((DnsQuestion *)q)->name);
        free(q);
    }
}

void record_del(void *r) {
    if (r) {
        DnsRecord *r_ = (DnsRecord *)r;
        free(r_->name);
        free(r_->data);
        free(r);
    }
}

void packet_del(DnsPacket p) {
    vec_del_with(&p.questions, question_del);
    vec_del_with(&p.additionals, record_del);
    vec_del_with(&p.answers, record_del);
    vec_del_with(&p.authorities, record_del);
}
