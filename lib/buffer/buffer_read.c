/* Public domain. */

#include <unistd.h>
#include "buffer.h"

int buffer_unixread(int fd, char *buf, size_t len)
{
	return read(fd, buf, len);
}
