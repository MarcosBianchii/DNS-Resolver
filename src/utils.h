#ifndef __UTILS__
#define __UTILS__

#include "ints.h"
#include "vec.h"

Vec split(char *string, char *sep);
u8 *join(Vec v, char *sep, usize *size);
usize digits(int num);

#endif // __UTILS__
