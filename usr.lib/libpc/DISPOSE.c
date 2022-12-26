/* Copyright (c) 1979 Regents of the University of California */

static char sccsid[] = "@(#)DISPOSE.c 1.3 6/10/81";

#include	"h00vars.h"

DISPOSE(var, siz)
	register char	**var;	/* pointer to pointer being deallocated */
	long		siz;	/* sizeof(bletch) */
{
	register int size = siz;

	if (*var == 0 || *var + size > _maxptr || *var < _minptr) {
		ERROR("Pointer value out of legal range\n", 0);
		return;
	}
	free(*var);
	if (*var == _minptr)
		_minptr += size;
	if (*var + size == _maxptr)
		_maxptr -= size;
	*var = (char *)(0);
}
