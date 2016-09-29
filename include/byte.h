/* Public domain. */

#ifndef BYTE_H
#define BYTE_H

#include <sys/types.h>
#include <stdint.h>

extern size_t byte_chr(void *, size_t, uint8_t);
extern size_t byte_rchr(void *, size_t, uint8_t);
extern void byte_copy(void *, size_t, const void *);
extern void byte_copyr(void *, size_t, const void *);
extern ssize_t byte_diff(const void *, size_t, const void *);
extern void byte_zero(void *, size_t);

#define byte_equal(s,n,t) (!byte_diff((s),(n),(t)))

#endif
