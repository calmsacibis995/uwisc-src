/* Copyright (c) 1979 Regents of the University of California */

static char sccsid[] = "@(#)LN.c 1.4 6/10/81";

#include <math.h>

double
LN(value)

	double	value;
{
	if (value <= 0) {
		ERROR("Non-positive argument of %e to ln\n", value);
		return;
	}
	return log(value);
}
