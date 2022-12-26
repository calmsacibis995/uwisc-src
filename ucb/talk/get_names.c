/*
 * Copyright (c) 1983 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 */

#ifndef lint
static char sccsid[] = "@(#)get_names.c	5.2 (Berkeley) 3/13/86";
#endif not lint

#include "talk.h"
#include <sys/param.h>
#include <protocols/talkd.h>
#ifdef UW
#include <pwd.h>
#endif UW

char	*getlogin();
char	*ttyname();
char	*rindex();
static	any();
extern	CTL_MSG msg;

/*
 * Determine the local and remote user, tty, and machines
 */
get_names(argc, argv)
	int argc;
	char *argv[];
{
	char hostname[MAXHOSTNAMELEN];
	char *his_name, *my_name;
	char *my_machine_name, *his_machine_name;
	char *my_tty, *his_tty;
	register char *cp;

	if (argc < 2 ) {
		printf("Usage: talk user [ttyname]\n");
		exit(-1);
	}
	if (!isatty(0)) {
		printf("Standard input must be a tty, not a pipe or a file\n");
		exit(-1);
	}
	my_name = getlogin();
#ifdef UW
    if((my_name == NULL) || (*my_name == 0)) {
        struct passwd *pw;

        pw = getpwuid(getuid());
        if(pw != NULL)
            my_name = pw->pw_name;
    }
#endif UW
	if (my_name == NULL) {
		printf("You don't exist. Go away.\n");
		exit(-1);
	}
	gethostname(hostname, sizeof (hostname));
	my_machine_name = hostname;
	my_tty = rindex(ttyname(0), '/') + 1;
	/* check for, and strip out, the machine name of the target */
	for (cp = argv[1]; *cp && !any(*cp, "@:!."); cp++)
		;
	if (*cp == '\0') {
		/* this is a local to local talk */
		his_name = argv[1];
		his_machine_name = my_machine_name;
	} else {
		if (*cp++ == '@') {
			/* user@host */
			his_name = argv[1];
			his_machine_name = cp;
		} else {
			/* host.user or host!user or host:user */
			his_name = cp;
			his_machine_name = argv[1];
		}
		*--cp = '\0';
	}
	if (argc > 2)
		his_tty = argv[2];	/* tty name is arg 2 */
	else
		his_tty = "";
	get_addrs(my_machine_name, his_machine_name);
	/*
	 * Initialize the message template.
	 */
	msg.vers = TALK_VERSION;
	msg.addr.sa_family = htons(AF_INET);
	msg.ctl_addr.sa_family = htons(AF_INET);
	msg.id_num = htonl(0);
	strncpy(msg.l_name, my_name, NAME_SIZE);
	msg.l_name[NAME_SIZE - 1] = '\0';
	strncpy(msg.r_name, his_name, NAME_SIZE);
	msg.r_name[NAME_SIZE - 1] = '\0';
	strncpy(msg.r_tty, his_tty, TTY_SIZE);
	msg.r_tty[TTY_SIZE - 1] = '\0';
}

static
any(c, cp)
	register char c, *cp;
{

	while (*cp)
		if (c == *cp++)
			return (1);
	return (0);
}
