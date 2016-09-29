#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/un.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include "byte.h"
#include "buffer.h"
#include "error.h"
#include "strerr.h"
#include "scan.h"
#include "env.h"
#include "prot.h"
#include "sig.h"
#include "open.h"
#include "sgetopt.h"

#define SYSLOG_NAMES
#include <syslog.h>

#if defined(__sun__) && defined(__sparc__) && defined(__unix__) && defined(__svr4__)
#define SOLARIS
# include <stropts.h>
# include <sys/strlog.h>
# include <fcntl.h>
# include "syslognames.h"
#endif

#define FATAL "socklog: fatal: "

#define USAGE " [-rRi] [args]"

#define VERSION "$Id: socklog.c,v 1.18 2004/06/26 09:36:25 pape Exp $"
#define DEFAULTUNIX "/dev/log"

static const char *progname;

#ifndef LINEC
#define LINEC 1024
#endif

char line[LINEC];
uint8_t lograw =0;

uint8_t flag_exitasap = 0;

void
sig_term_handler(void) {
	flag_exitasap = 1;
}

void
usage()
{
	strerr_die4x(1, "usage: ", progname, USAGE, "\n");
}

void
out(const char *s1, const char *s2)
{
	if (s1) buffer_puts(buffer_1, s1);
	if (s2) buffer_puts(buffer_1, s2);
}

void
err(const char *s1, const char *s2, const char *s3)
{
	if (s1) buffer_puts(buffer_2, s1);
	if (s2) buffer_puts(buffer_2, s2);
	if (s3) buffer_puts(buffer_2, s3);
}

int
print_syslog_names(int fpr, buffer *buf)
{
	CODE *p;
	int fp, rc = 1;

	fp = LOG_FAC(fpr) << 3;
	for (p = facilitynames; p->c_name; p++) {
		if (p->c_val != fp)
			continue;
		buffer_puts(buf, p->c_name);
		buffer_puts(buf, ".");
		break;
	}
	if (!p->c_name) {
		buffer_puts(buf, "unknown.");
		rc = 0;
	}

	fp =LOG_PRI(fpr);
	for (p =prioritynames; p->c_name; p++) {
		if (p->c_val != fp)
			continue;
		buffer_puts(buf, p->c_name);
		buffer_puts(buf, ": ");
		break;
	}
	if (!p->c_name) {
		buffer_puts(buf, "unknown: ");
		rc = 0;
	}

	return rc;
}

int
scan_syslog_names (char *line, ssize_t len, buffer *buf)
{
	int i, ok, fpr;
	char *p;

	if (*line != '<')
		return 0;

	p = line + 1;
  for (i = 1; (i < 5) && (i < len); i++ && p++) {
		if ((ok = *p == '>'))
			break;

    if (('0' <= *p) && (*p <= '9'))
			fpr = 10 * fpr + *p - '0';
		else
			return 0;
  }

  if (!ok || !fpr)
		return 0;

	return print_syslog_names(fpr, buf) ? ++i : 0;
}

void
remote_info(struct sockaddr_in *sa)
{
	char *host;

	host = inet_ntoa(sa->sin_addr);
	out(host, ": ");
}

int
read_socket(int fd)
{
	int type;

	sig_catch(sig_term, sig_term_handler);
	sig_catch(sig_int, sig_term_handler);

	buffer_putsflush(buffer_2, "starting.\n");

	for(;;) {
		struct sockaddr_in saf;
		socklen_t fromlen;
		ssize_t linec;
		int os;

		fromlen = sizeof saf;
		linec = recvfrom(fd, line, LINEC, 0, (struct sockaddr *)&saf, &fromlen);
		if (linec == -1) {
			if (errno != error_intr)
				strerr_die2sys(111, FATAL, "recvfrom(): ");
			linec = 0;
		}

		/* rexeived sigterm, exit */
		if (flag_exitasap) break;

		/* skip zero */
		while (linec && (line[linec - 1] == 0)) linec--;
		if (linec == 0) continue;

		if (lograw) {
			buffer_put(buffer_1, line, linec);
			if (line[linec - 1] != '\n') {
				if (linec == LINEC)
					out("...", 0);
				out("\n", 0);
			}
			if (lograw == 2) {
				buffer_flush(buffer_1);
				continue;
			}
		}

		// if (mode == MODE_INET) remote_info(&saf);
		os = scan_syslog_names(line, linec, buffer_1);

		buffer_put(buffer_1, line + os, linec - os);
		if (line[linec -1] != '\n') {
			if (linec == LINEC) out("...", 0);
			out("\n", 0);
		}
		buffer_flush(buffer_1);
	}

	return 0;
}

int
read_stdin(int fd, const char **vars)
{
	char *envs[9];
	int i, flageol = 1;

	for (i = 0; *vars && (i < 8); vars++) {
		if ((envs[i] = env_get(*vars)) != NULL)
			i++;
	}
	envs[i] =NULL;

	for(;;) {
		int linec;
		char *l, *p;

		linec = buffer_get(buffer_0, line, LINEC);
		if (linec == -1)
			strerr_die2sys(111, FATAL, "read(): ");

		if (linec == 0) {
			if (!flageol)
				err("\n", 0, 0);
			buffer_flush(buffer_2);
			return 0;
		}

		for (l = p = line; l - line < linec; l++) {
			if (flageol) {
				if (!*l || (*l == '\n'))
					continue;
				for (i = 0; envs[i]; i++) {
					err(envs[i], ": ", 0);
				}
				/* could fail on eg <13\0>user.notice: ... */
				l += scan_syslog_names(l, line -l +linec, buffer_2);
				p = l;
				flageol = 0;
			}
			if (!*l || (*l == '\n')) {
				buffer_put(buffer_2, p, l - p);
				buffer_putsflush(buffer_2, "\n");
				flageol = 1;
			}
		}
		if (!flageol) buffer_putflush(buffer_2, p, l -p);
  }

	return 0;
}

int main(int argc, const char **argv, const char *const *envp) {
	char opt;
	int iflag;

	progname = *argv;

	while ((opt = getopt(argc, argv, "rRi")) != -1) {
		switch(opt) {
		case 'r': lograw = 1; break;
		case 'R': lograw = 2; break;
		case 'i': iflag++; break;
		}
	}
	argv += optind;

	if (iflag)
		return read_stdin(0, argv);

	return read_socket(0);

	/* NOT REACHED */
	return 1;
}
