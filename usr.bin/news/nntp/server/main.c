#ifndef lint
static char	*sccsid = "@(#)main.c	1.3	(Berkeley) 5/17/86";
#endif

/*
 *	Network News Transfer Protocol server
 *
 *	Phil Lapsley
 *	College of Engineering
 *	University of California, Berkeley
 *	(ARPA: phil@berkeley.edu; UUCP: ...!ucbvax!phil)
 */

#include "common.h"
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <signal.h>

main()
{

#ifdef ALONE	/* If no inetd */

	int			sockt, client, length;
	struct sockaddr_in	from;
	extern int 		reaper();

	disassoc();

#ifdef FASTFORK
	num_groups = read_groups();	/* Read active file now */
					/* and then do it every */
	set_timer();			/* so often later */
#endif

	sockt = get_socket();

	signal(SIGCHLD, reaper);

	listen(sockt, SOMAXCONN);

	for (;;) {
		client = accept(sockt, &from, &length);
		if (client < 0) {
#ifdef SYSLOG
			if (errno != EINTR)
				syslog(LOG_ERR, "accept: %m\n");
#endif
			continue;
		}

		switch (fork()) {
		case	-1:
#ifdef SYSLOG
				syslog(LOG_ERR, "fork: %m\n");
#endif
				close(client);
				break;

		case	0:	close(sockt);
				make_stdio(client);
				serve();
				break;

		default:	close(client);
				break;
		}
	}

#else		/* We have inetd */

	serve();

#endif
}
