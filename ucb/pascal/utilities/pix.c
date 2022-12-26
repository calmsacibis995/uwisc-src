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
static char sccsid[] = "@(#)pix.c	5.1 (Berkeley) 6/5/85";
#endif not lint

/*
 * pix - pi then px
 *
 * Bill Joy UCB August 26, 1977
 */

#include "whoami.h"
#include "objfmt.h"
#include "config.h"
#define	ERRS	1

char	*name;

int	onintr();

#define	ETXTBSY	26

main(argc, argv)
	int argc;
	char *argv[];
{
	register char **av;
	register int ac;
	int i, io, pid, status;
	extern errno;

	do
		io = open("/dev/null", 0);
	while (io >= 0 && io < 3);
	for (io = 3; io < 15; io++)
		close(io);
	if ((signal(2, 1) & 01) == 0)
		signal(2, onintr);
	for (ac = 1; ac < argc; ac++)
		if (dotted(argv[ac], 'p')) {
			ac++;
			break;
		}
	name = "-o/tmp/pixaXXXXX" + 2;
	mktemp(name);
	for (;;) {
		io = creat(name, 0400);
		if (io > 0)
			break;
		if (name[8] == 'z') {
			perror(name);
			exit(1);
		}
		name[8]++;
	}
	pid = fork();
	if (pid == -1) {
		write(2, "No more processes\n", 18);
		onintr();
	}
	if (pid == 0) {
		if (io != 3) {
			write(2, "Impossible error in pix\n", 24);
			onintr();
		}
		argv[ac] = 0;
		argv[0] = name - 2;
		do
			execv(pi_comp, argv);
		while (errno == ETXTBSY);
		write(2, "Can't find pi\n", 14);
		onintr();
	}
	close(io);
	do
		i = wait(&status);
	while (i != pid && i != -1);
	if (i == -1 || (status & 0377))
		onintr();
	if (status != 0) {
		if ((status >> 8) == ERRS)
			write(2, "Execution suppressed due to compilation errors\n", 47);
		onintr();
	}
	ac--;
	argv[ac] = name;
	ac--;
	argv[ac] = "pix";
	argv[argc] = 0;
	do
		execv(px_debug, &argv[ac]);
	while (errno == ETXTBSY);
	write(2, "Can't find px\n", 14);
	onintr();
}

dotted(cp, ch)
	char *cp, ch;
{
	register int i;

	i = strlen(cp);
	return (i > 1 && cp[i - 2] == '.' && cp[i - 1] == ch);
}

onintr()
{

	signal(2, 1);
	unlink(name);
	exit(1);
}
