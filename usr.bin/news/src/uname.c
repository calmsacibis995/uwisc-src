/*
 * This software is Copyright (c) 1986 by Rick Adams.
 *
 * Permission is hereby granted to copy, reproduce, redistribute or
 * otherwise use this software as long as: there is no monetary
 * profit gained specifically from the use or reproduction or this
 * software, it is not sold, rented, traded or otherwise marketed, and
 * this copyright notice is included prominently in any copy
 * made.
 *
 * The author make no claims as to the fitness or correctness of
 * this software for any use whatsoever, and it is provided as is. 
 * Any use of this software is at the user's own risk.
 *
 * This routine is compatible with the Unix T/S system call uname,
 * which figures out the name of the local system.
 * However, we do it by reading the file /usr/include/whoami.h.
 * This avoids having to recompile uucp for each site and hence
 * avoids having to distribute the source to uucp to people who
 * have only binary licenses.
 */

#ifdef SCCSID
static char	*SccsId = "@(#)uname.c	2.13	12/16/86";
#endif /* SCCSID */

#include "params.h"

#ifdef UNAME
# define DONE
#endif /* UNAME */

#ifdef UW
uname(uptr)
struct utsname *uptr;
{
	uptr->nodename[0] = 0;	/* null out name */

#ifdef	UUNAME
	if(uptr->nodename[0] == 0) {
		FILE *uucpf;
		register char *p;
		/* uucp name is stored UUNAME */

		if ((uucpf = fopen(UUNAME, "r")) != NULL) {
			fgets(uptr->nodename, sizeof (uptr->nodename), uucpf);
			p = index(uptr->nodename, '\n');
			if (p)
				*p = '\0';
			fclose(uucpf);
		}
	}
#endif /* UUNAME */

#ifdef GHNAME
	if(uptr->nodename[0] == 0) {
		char *cp;
		gethostname(uptr->nodename, sizeof (uptr->nodename));
		cp = MYDOMAIN;	/* Gould compiler can't handle ""[0] */
		if (*cp == '\0') /* get domain name from hostname */
			return;
		cp = index(uptr->nodename, '.');
		if (cp)
			*cp = '\0';
	}
#endif

	if(uptr->nodename[0] == 0) {
		fprintf(stderr, "no local sysname determined!\n");
	}
}
#define DONE
#else

#ifdef GHNAME
uname(uptr)
struct utsname *uptr;
{
	char *cp;
	gethostname(uptr->nodename, sizeof (uptr->nodename));
	cp = MYDOMAIN;	/* Gould compiler can't handle ""[0] */
	if (*cp == '\0') /* get domain name from hostname */
		return;
	cp = index(uptr->nodename, '.');
	if (cp)
		*cp = '\0';
}
# define DONE
#endif

#ifdef	UUNAME
uname(uptr)
struct utsname *uptr;
{
	FILE *uucpf;
	register char *p;
	/* uucp name is stored UUNAME */

#ifdef UW
	uptr->nodename[0] = 0;	/* null out name */
#endif UW
	if (((uucpf = fopen(UUNAME, "r")) == NULL) ||
		fgets(uptr->nodename, sizeof (uptr->nodename), uucpf) == NULL) {
			fprintf(stderr, "no sysname in %s\n", UUNAME);
			return;
	}
	p = index(uptr->nodename, '\n');
	if (p)
		*p = '\0';
	if (uucpf != NULL)
		fclose(uucpf);
}
#define DONE
#endif /* UUNAME */
#endif /* UW */

#ifndef DONE
#define	HDRFILE "/usr/include/whoami.h"

uname(uptr)
struct utsname *uptr;
{
	char buf[BUFSIZ];
	FILE *fd;
	
	fd = fopen(HDRFILE, "r");
	if (fd == NULL) {
		fprintf(stderr, "Cannot open %s\n", HDRFILE);
		exit(1);
	}
	
	for (;;) {	/* each line in the file */
		if (fgets(buf, sizeof buf, fd) == NULL) {
			fprintf(stderr, "no sysname in %s\n", HDRFILE);
			fclose(fd);
			exit(2);
		}
		if (sscanf(buf, "#define sysname \"%[^\"]\"", uptr->nodename) == 1) {
			fclose(fd);
			return;
		}
	}
}
#endif
