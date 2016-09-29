#include <stdio.h>
#include <stdlib.h>
#include <err.h>
#include <string.h>
#include <getopt.h>
#include <limits.h>
#include <errno.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>

#include <netinet/in.h>

#include "fd.h"

static char *argv0;

extern char **environ;

ssize_t
recvfds(int fd, int *fds, size_t nfds)
{
	struct msghdr m;
	struct iovec not;
	struct cmsghdr *cmsg;
	struct {
		struct cmsghdr h;
		int fd[1];
	} buf;
	char foo;
	int i;

	not.iov_base = &foo;
	not.iov_len = 1;

	memset(&m, 0, sizeof m);
	m.msg_name = 0;
	m.msg_namelen = 0;
	m.msg_iov = &not;
	m.msg_iovlen = 1;
	m.msg_control = &buf;
	m.msg_controllen = sizeof(struct cmsghdr) + sizeof(int) * nfds;
	cmsg = CMSG_FIRSTHDR(&m);
	cmsg->cmsg_len = m.msg_controllen;
	cmsg->cmsg_level = SOL_SOCKET;
	cmsg->cmsg_type = SCM_RIGHTS;
	for (i = 0; i < nfds; i++)
		((int *)CMSG_DATA(cmsg))[i] = -1;

	if (recvmsg(fd, &m, 0) == -1)
		err(1, "recvmsg");

	for(i = 0; i < nfds; i++)
		fds[i] = ((int *)CMSG_DATA(cmsg))[i];

	return (cmsg->cmsg_len - sizeof(struct cmsghdr)) / sizeof(int);
}

static void
usage()
{
	fprintf(stderr, "usage: %s path\n", argv0);
	exit(1);
}

int
main(int argc, char *argv[])
{
	char c, *path;
	int fd, cd;
	int vflag;
	struct sockaddr_un su;

	argv0 = argv[0];

	while ((c = getopt(argc, argv, "+v")) > 0) {
		switch (c) {
		case 'v': vflag++; break;
		default: usage(); /* NOT REACHED */
		}
	}
	argc -= optind;
	argv += optind;

	if (argc < 1)
		usage();

	path = argv[0];
	argc -= 1;
	argv += 1;

	if ((fd = socket(AF_UNIX, SOCK_STREAM, 0)) == -1)
		err(1, "socket");

	memset(&su, 0, sizeof su);
	su.sun_family = AF_UNIX;
	strncpy(su.sun_path, path, sizeof su.sun_path);

	if (connect(fd, (struct sockaddr *)&su, sizeof su) == -1)
		err(1, "connect");

	int rfd = -1;
	recvfds(fd, &rfd, 1);
	printf("rfd %d\n", rfd);
	close(0);
	fd_copy(0, rfd);
	execve(*argv, argv, environ);
	err(1, "execv");

	return 0;
}
