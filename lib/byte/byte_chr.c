/* Public domain. */

#include "byte.h"

size_t
byte_chr(void *s, size_t n, uint8_t c)
{
	uint8_t ch;
	uint8_t *t;

	ch = c;
	t = s;

	for (;;) {
		if (!n) break;
		if (*t == ch) break;
		++t; --n;

		if (!n) break;
		if (*t == ch) break;
		++t; --n;

		if (!n) break;
		if (*t == ch) break;
		++t; --n;

		if (!n) break;
		if (*t == ch) break;
		++t; --n;
	}

	return t - (uint8_t *)s;
}
