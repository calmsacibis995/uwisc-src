/*
 *	This is an interactive adduser program suitable for adding single
 *	users to any of the UW Computer Science Department computers.  It
 *	asks for the users full name, login  name, group, and other  per-
 *	tinent  information.  For  most of the entries, a default will be
 *	given in "<default>" and if you just type  <return>,  this  value
 *	will  be  used.  If  a new group is being added, it looks for the
 *	file-system with  the most free-space   and   adds   the   person
 *	there.  Finally, it sends mail to ALIAS_PERSON telling of the new
 *	addition.  Of course, you have to be superuser  to  make  any  of
 *	this work.
 *
 *	-m option forces no mailing
 *	-h option means this login has no real home directory
 *	-u makes adduser ask for a uid rather than guessing
 *
 *	usage:	adduser
 *
 *	Author:	Dave Cohrs
 *
 *	To make: cc -O -o adduser adduser.c -ldbm [ -lnet ]
 *		the -lnet only needed for 4.1 research machines
 *
 *	Special defines:
 *		BSD4_2			for 4.2bsd systems
 *		BSD2_8			for 2.8bsd systems
 *		STAT			for UW Statistics system
 *		YP				for an YP machine
 */
#include <stdio.h>
#include <signal.h>
#include "adduser.h"

#ifdef YP
char	domain_name[128];
#endif YP

main(argc, argv)
	int argc;
	char *argv[];
{
	register char *carg;
	extern	abnquit();

	signal(SIGHUP, abnquit);	/* remove lockfile if fuck-up */
	signal(SIGINT, abnquit);
	signal(SIGQUIT, abnquit);
	signal(SIGTERM, abnquit);

#ifdef YP
	if ( getdomainname( domain_name, sizeof( domain_name))){
		perror("getdomainname");
	}
	(void) sprintf(Passwd,"/etc/yp/%s/passwd", domain_name);
	(void) sprintf(NewPasswd,"/etc/yp/%s/passwd.new", domain_name);
	(void) sprintf(OldPasswd,"/etc/yp/%s/passwd.old", domain_name);
	(void) sprintf(Lockfile,"/etc/yp/%s/ptmp", domain_name);
	(void) sprintf(Group,"/etc/yp/%s/group", domain_name);
	(void) sprintf(NewGroup,"/etc/yp/%s/group.new", domain_name);
	(void) sprintf(OldGroup,"/etc/yp/%s/group.old", domain_name);
#else
	(void) strcpy(Passwd,"/etc/passwd");
	(void) strcpy(NewPasswd,"/etc/passwd.new");
	(void) strcpy(OldPasswd,"/etc/passwd.old");
	(void)	strcpy(Lockfile,"/etc/ptmp");
	(void) strcpy(Group,"/etc/group");
	(void) strcpy(NewGroup,"/etc/group.new");
	(void) strcpy(OldGroup,"/etc/group.old");
#endif YP


	while(*++argv) {
		carg = *argv;
		if(*carg != '-')
			usage();
		while(*++carg)
			switch(*carg) {
			case 'm':
				nomail++;
				break;
			case 'h':
				nohome++;
				break;
			case 'u':
				askuid++;
				break;
			default:
				usage();
			}
	}

	if(link(Passwd, Lockfile)) {	/* make lock file */
		printf("Sorry, password file is busy.  Try later.\n");
		exit(1);
	}

	if(dbmerror = dbminit(Glob_names))	/* open alias database */
		fprintf(stderr,
			"Warning: %s not checked for conflict - can't access\n",
			Glob_names);

	/* set instructional if 'User' file exists */
	instructional = !access(User, 0);

	printf("UW Interactive %s ADDUSER\n\n",
		(instructional ? "Instructional" : "Research"));

	getdata();
	add();
}

/* Tell 'em how to use this program */
usage()
{
	fprintf(stderr, "usage: adduser [-hm]\n");
	exit(2);
}

/* Removes the lock file and any other stuff if we are exiting abnormally */
abnquit()
{
	unlink(Lockfile);
	exit(5);
}

quit(stat)	/* normal exit, kill lock and go away, status is stat */
	int	stat;
{
	unlink(Lockfile);
	exit(stat);
}
