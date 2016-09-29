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

static char *argv0;

ssize_t
sendfds(int fd, const int *fds, size_t nfds)
{
	struct msghdr m;
	struct iovec not;
	struct cmsghdr *cmsg;
	struct {
		struct cmsghdr h;
		int fd[1];
	} buf;
	char foo = '!';
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
		((int *)CMSG_DATA(cmsg))[i] = fds[i];
	return sendmsg(fd, &m, 0);
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
	char c, *p, *path;
	int vflag, cflag;
	int sd, fd, cd;
	int domain, type;
	int enable = 1;
	int backlog = 500;
	union Addr {
		struct sockaddr_in in;
		struct sockaddr_un un;
	} addr;
	size_t sa_len;
	struct sockaddr *sa;
	struct sockaddr_un sa_broker;

	argv0 = argv[0];
	domain = AF_INET;
	type = SOCK_STREAM;

	while ((c = getopt(argc, argv, "46usdcb:v")) > 0) {
		switch (c) {
		case '4': domain = AF_INET; break;
		case '6': domain = AF_INET6; break;
		case 'u': domain = AF_UNIX; break;
		case 's': type = SOCK_STREAM; break;
		case 'd': type = SOCK_DGRAM; break;
		case 'c': cflag++; break;
		case 'b':
			if (!isdigit(optarg[0]))
				errx(1, "-b needs a positive integer");
			backlog = strtol(optarg, &p, 0);
			if ((p && p[0] != '\0') || backlog <= 0)
				errx(1, "-b needs a positive integer");
			break;
		case 'v': vflag++; break;
		default: usage(); /* NOT REACHED */
		}
	}
	argc -= optind;
	argv += optind;

	switch (domain) {
	case AF_INET:
		memset(&addr.in, 0, sizeof addr.in);
		addr.in.sin_family = PF_INET;
		addr.in.sin_addr.s_addr = INADDR_ANY;
		addr.in.sin_port = htons(8012);
		sa = (struct sockaddr *)&addr.in;
		sa_len = sizeof(struct sockaddr_in);
		break;
	case AF_INET6:
		break;
	case AF_UNIX:
		memset(&addr.un, 0, sizeof addr.un);
		addr.un.sun_family = AF_UNIX;
		strncpy(addr.un.sun_path, argv[0], sizeof addr.un.sun_path);
		printf("unix:%s %d\n", addr.un.sun_path, sizeof addr.un.sun_path);
		unlink(addr.un.sun_path);
		sa = (struct sockaddr *)&addr.un;
		sa_len = sizeof(struct sockaddr_un);
		argc -= 1;
		argv += 1;
		break;
	}

	if (argc < 1)
		usage();

	path = argv[0];
	printf("unix:%s\n", path);

	if (cflag)
		type |= SOCK_CLOEXEC;

	if ((sd = socket(domain, type, 0)) == -1)
		err(1, "socket");

	if (setsockopt(sd, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int)) == -1)
		err(1, "setsockopt");

	umask(0);
	if (bind(sd, sa, sa_len) == -1)
		err(1, "bind");

	if (domain == AF_UNIX) {
	}

	if (listen(sd, backlog) == -1)
		err(1, "listen");

	printf("fd %d\n", sd);

	/* broker */
	if ((fd = socket(AF_UNIX, SOCK_STREAM, 0)) == -1)
		err(1, "socket");

	memset(&sa_broker, 0, sizeof sa_broker);
	sa_broker.sun_family = AF_UNIX;
	strncpy(sa_broker.sun_path, path, sizeof sa_broker.sun_path);
	if (unlink(sa_broker.sun_path) == -1 && (errno != ENOENT))
		err(1, "unlink");

	if (bind(fd, (struct sockaddr *)&sa_broker, sizeof sa_broker) == -1)
		err(1, "bind broker");

	if (listen(fd, 1) == -1)
		err(1, "listen");

	for (;;) {
		printf("accept\n");
		if ((cd = accept(fd, 0, 0)) == -1)
			err(1, "accept");
		printf("sendfds %d\n", sendfds(cd, &sd, 1));
		close(cd);
	}

	return 0;
}
