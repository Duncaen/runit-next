/* Public domain. */

#include "byte.h"

ssize_t
byte_diff(const void *s, size_t n, const void *t)
{
	const uint8_t *a = s;
	const uint8_t *b = t;

	for (;;) {
		if (!n) return 0;
		if (*a != *b) break;
		++a; ++b; --n;

		if (!n) return 0;
		if (*a != *b) break;
		++a; ++b; --n;

		if (!n) return 0;
		if (*a != *b) break;
		++a; ++b; --n;

		if (!n) return 0;
		if (*a != *b) break;
		++a; ++b; --n;
	}

	return ((int)(unsigned int)(unsigned char) * a)
	    - ((int)(unsigned int)(unsigned char) * b);
}
