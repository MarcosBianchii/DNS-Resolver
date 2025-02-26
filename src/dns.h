#ifndef __DNS__
#define __DNS__

#include "ints.h"
#include "vec.h"

#define HDR_SIZE (12 * sizeof(u8))

typedef struct DnsHeader {
    u16 id;
    u16 flags;
    u16 qdcount;
    u16 ancount;
    u16 nscount;
    u16 arcount;
} DnsHeader;

typedef struct DnsQuestion {
    u8 *name;
    u16 type;
    u16 class_;

    // Not serialized;
    usize namelen;
} DnsQuestion;

typedef enum Record {
    TYPE_A = 1,
    TYPE_NS,
} Record;

typedef enum Class_ {
    ClassIn = 1,
} Class;

typedef struct DnsRecord {
    u8 *name;
    u16 type;
    u16 class_;
    u32 ttl;
    u16 datalen;
    u8 *data;

    // Not serialized.
    usize namelen;
} DnsRecord;

typedef struct DnsPacket {
    DnsHeader header;
    Vec questions;
    Vec answers;
    Vec authorities;
    Vec additionals;
} DnsPacket;

u8 *header_to_bytes(DnsHeader h);
u8 *question_to_bytes(DnsQuestion q, usize *size);

DnsHeader parse_header(u8 *bytes, usize *read);
DnsQuestion parse_question(u8 *bytes, usize *read);
DnsRecord parse_record(u8 *bytes, usize *read);
DnsPacket parse_dns_packet(u8 *bytes);

void packet_del(DnsPacket p);

#endif // __DNS__
