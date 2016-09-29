#include "uint16.h"

void
uint16_unpack(const char s[2], uint16_t *u)
{
	uint16_t result;

	result = (unsigned char) s[1];
	result <<= 8;
	result += (unsigned char) s[0];

	*u = result;
}

void
uint16_unpack_big(const char s[2], uint16_t *u)
{
	uint16_t result;

	result = (unsigned char) s[0];
	result <<= 8;
	result += (unsigned char) s[1];

	*u = result;
}
