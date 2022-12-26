/* Copyright (c) 1979 Regents of the University of California */

static char sccsid[] = "@(#)SEEK.c 1.4 11/22/81";

#include "h00vars.h"

/*
 * Random access routine
 */
SEEK(curfile, loc)

	register struct iorec	*curfile;
	struct seekptr		*loc;
{
	curfile->funit |= SYNC;
	curfile->funit &= ~(EOFF | EOLN | SPEOLN);
	if (fseek(curfile->fbuf, loc->cnt, 0) == -1) {
		PERROR("Could not seek ", curfile->pfname);
		return;
	}
}
