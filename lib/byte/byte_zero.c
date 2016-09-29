#include "byte.h"

void
byte_zero(void *dest, size_t n)
{
	uint8_t *s = dest;

	for (;;) {
		if (!n) break;
		*s++ = 0; --n;

		if (!n) break;
		*s++ = 0; --n;

		if (!n) break;
		*s++ = 0; --n;

		if (!n) break;
		*s++ = 0; --n;
	}
}
