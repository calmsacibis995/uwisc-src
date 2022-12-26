/*
 * Copyright (c) 1982, 1986 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 *
 *	@(#)ls.c	7.1 (Berkeley) 6/5/86
 */
#ifndef lint
static char rcs_id[] = {"$Header: ls.c,v 2.1 86/08/13 10:49:03 root Exp $"};
#endif not lint
/*
 * RCS Info
 *	$Locker: root $
 */

#include "../h/param.h"
#include "../h/time.h"
#include "../h/vfs.h"
#include "../h/vnode.h"
#include "../ufs/inode.h"
#include "../ufs/dir.h"
#include "../h/ino.h"
#include "saio.h"

char line[100];

main()
{
	int i;

	printf("ls\n");
	do  {
		printf(": "); gets(line);
		i = open(line, 0);
	} while (i < 0);

	ls(i);
}

ls(io)
register io;
{
	struct direct d;
	register i;

	while (read(io, (char *)&d, sizeof d) == sizeof d) {
		if (d.d_ino == 0)
			continue;
		printf("%d\t", d.d_ino);
		for (i=0; i<DIRSIZ; i++) {
			if (d.d_name[i] == 0)
				break;
			printf("%c", d.d_name[i]);
		}
		printf("\n");
	}
}
