#ifndef __LIB__
#define __LIB__

#include "dns.h"
#include "ints.h"
#include "lru.h"

u8 *build_query(char *domain_name, Record type, usize *size);
u8 *encode_dns_name(char *domain_name, usize *namlen);

DnsPacket send_query(char *ip_addr, char *domain_name, Record type);

char *resolve(char *domain_name, Record type, LruCache *cache);

#endif // __LIB__
