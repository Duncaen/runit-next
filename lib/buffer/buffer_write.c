/* Public domain. */

#include <unistd.h>
#include "buffer.h"

int buffer_unixwrite(int fd, const char *buf, size_t len)
{
	return write(fd, buf, len);
}
