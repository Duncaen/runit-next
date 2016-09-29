#include "scan.h"
#include "ip4.h"

unsigned int
ip4_scan(const char *s, char ip[4])
{
	unsigned int i;
	unsigned int len;
	unsigned long u;

	len = 0;

	if (!(i = scan_ulong(s, &u))) return 0;
	ip[0] = u; len += i; s += i;
	if (*s != '.') return 0;

	++s; ++len;
	if (!(i = scan_ulong(s, &u))) return 0;
	ip[1] = u; len += i; s += i;
	if (*s != '.') return 0;

	++s; ++len;
	if (!(i = scan_ulong(s, &u))) return 0;
	ip[2] = u; len += i; s += i;
	if (*s != '.') return 0;

	++s; ++len;
	if (!(i = scan_ulong(s, &u))) return 0;
	ip[3] = u; len += i;

	return len;
}
