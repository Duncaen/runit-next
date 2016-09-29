/* Copyright 2001 D. J. Bernstein */

#include <sys/types.h>
#include <sys/param.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include "byte.h"
#include "socket.h"
#include "uint16.h"

ssize_t
socket_send4(int s, const char *buf, int len, const char ip[4], uint16_t port)
{
	struct sockaddr_in sa;

	byte_zero(&sa, sizeof sa);
	sa.sin_family = AF_INET;
	uint16_pack_big((char *) &sa.sin_port, port);
	byte_copy((char *)&sa.sin_addr, 4, ip);

	return sendto(s, buf, len, 0, (struct sockaddr *)&sa, sizeof sa);
}
