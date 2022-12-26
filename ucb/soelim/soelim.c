/*
 * Copyright (c) 1980 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 */

#ifndef lint
char copyright[] =
"@(#) Copyright (c) 1980 Regents of the University of California.\n\
 All rights reserved.\n";
#endif not lint

#ifndef lint
static char sccsid[] = "@(#)soelim.c	5.1 (Berkeley) 5/31/85";
#endif not lint

#include <stdio.h>
#ifdef	UW
#include <pwd.h>
#endif	UW
/*
 * soelim - a filter to process n/troff input eliminating .so's
 *
 * Author: Bill Joy UCB July 8, 1977
 *
 * This program eliminates .so's from a n/troff input stream.
 * It can be used to prepare safe input for submission to the
 * phototypesetter since the software supporting the operator
 * doesn't let him do chdir.
 *
 * This is a kludge and the operator should be given the
 * ability to do chdir.
 *
 * This program is more generally useful, it turns out, because
 * the program tbl doesn't understand ".so" directives.
 */
#define	STDIN_NAME	"-"

main(argc, argv)
	int argc;
	char *argv[];
{

	argc--;
	argv++;
	if (argc == 0) {
		(void)process(STDIN_NAME);
		exit(0);
	}
	do {
		(void)process(argv[0]);
		argv++;
		argc--;
	} while (argc > 0);
	exit(0);
}

int process(file)
	char *file;
{
	register char *cp;
	register int c;
	char fname[BUFSIZ];
	FILE *soee;
	int isfile;
#ifdef UW
    char buf[256], *s, *index();
    struct passwd *pw;
#endif UW

	if (!strcmp(file, STDIN_NAME)) {
		soee = stdin;
	} else {
#ifdef	UW
		if((*file == '~') && (s = index(file, '/'))) {
			if(s-file-1)
				strncpy(buf, file+1, s-file-1);
			buf[s-file-1] = 0;
			if(pw = (*buf ? getpwnam(buf) : getpwuid(getuid()))) {
				(void)strcpy(buf, pw->pw_dir);
				(void)strcat(buf, s);
				(void)strcpy(file, buf);
			}
			(void)endpwent();
		}
#endif	UW
		soee = fopen(file, "r");
		if (soee == NULL) {
			perror(file);
			return(-1);
		}
	}
	for (;;) {
		c = getc(soee);
		if (c == EOF)
			break;
		if (c != '.')
			goto simple;
		c = getc(soee);
		if (c != 's') {
			putchar('.');
			goto simple;
		}
		c = getc(soee);
		if (c != 'o') {
			printf(".s");
			goto simple;
		}
		do
			c = getc(soee);
		while (c == ' ' || c == '\t');
		cp = fname;
		isfile = 0;
		for (;;) {
			switch (c) {

			case ' ':
			case '\t':
			case '\n':
			case EOF:
				goto donename;

			default:
				*cp++ = c;
				c = getc(soee);
				isfile++;
				continue;
			}
		}
donename:
		if (cp == fname) {
			printf(".so");
			goto simple;
		}
		*cp = 0;
		if (process(fname) < 0)
			if (isfile)
				printf(".so %s\n", fname);
		continue;
simple:
		if (c == EOF)
			break;
		putchar(c);
		if (c != '\n') {
			c = getc(soee);
			goto simple;
		}
	}
	if (soee != stdin) {
		fclose(soee);
	}
	return(0);
}
