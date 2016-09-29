#ifndef UINT16_H
#define UINT16_H

#include <stdint.h>

extern void uint16_pack(char *, uint16_t);
extern void uint16_pack_big(char *, uint16_t);
extern void uint16_unpack(const char *, uint16_t *);
extern void uint16_unpack_big(const char *, uint16_t *);

#endif
