/* Copyright 2001 D. J. Bernstein */

#ifndef SOCKET_H
#define SOCKET_H

#include <stdint.h>

extern int socket_tcp(void);
extern int socket_udp(void);

extern int socket_connect4(int, const char *, uint16_t);
extern int socket_connected(int);
extern int socket_bind4(int, char *, uint16_t);
extern int socket_bind4_reuse(int, char *, uint16_t);
extern int socket_listen(int, int);
extern int socket_accept4(int, char *, uint16_t *);
extern ssize_t socket_recv4(int, char *, int, char *, uint16_t *);
extern ssize_t socket_send4(int, const char *, int, const char *, uint16_t);
extern int socket_local4(int, char *, uint16_t *);
extern int socket_remote4(int, char *, uint16_t *);

extern void socket_tryreservein(int, int);

#endif
