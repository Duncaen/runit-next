/* Public domain. */

#include "byte.h"

size_t
byte_rchr(void *dest, size_t n, uint8_t c)
{
	uint8_t *s = dest;
	uint8_t ch = c;
	uint8_t *t = s;
	uint8_t *u = 0;

	for (;;) {
		if (!n) break;
		if (*t == ch) u = t;
		++t; --n;

		if (!n) break;
		if (*t == ch) u = t;
		++t; --n;

		if (!n) break;
		if (*t == ch) u = t;
		++t; --n;

		if (!n) break;
		if (*t == ch) u = t;
		++t; --n;
	}

	if (!u) u = t;
	return u - s;
}
