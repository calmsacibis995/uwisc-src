/*
 * Copyright (c) 1982, 1986 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 *
 *	@(#)cat.c	7.1 (Berkeley) 6/5/86
 */
#ifndef lint
static char rcs_id[] = {"$Header: cat.c,v 2.1 86/08/13 10:47:21 root Exp $"};
#endif not lint
/*
 * RCS Info
 *	$Locker: root $
 */

main()
{
	int c, i;
	char buf[50];

	do {
		printf("File: ");
		gets(buf);
		i = open(buf, 0);
	} while (i <= 0);

	while ((c = getc(i)) > 0)
		putchar(c);
	exit(0);
}
