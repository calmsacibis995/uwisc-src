/* Copyright (c) 1979 Regents of the University of California */

static char sccsid[] = "@(#)TEOF.c 1.4 8/16/82";

#include "h00vars.h"

bool
TEOF(filep)

	register struct iorec	*filep;
{
	if (filep->fblk >= MAXFILES || _actfile[filep->fblk] != filep ||
	    (filep->funit & FDEF)) {
		ERROR("Reference to an inactive file\n", 0);
		return;
	}
	if (filep->funit & (EOFF|FWRITE))
		return TRUE;
	IOSYNC(filep);
	if (filep->funit & EOFF)
		return TRUE;
	return FALSE;
}
