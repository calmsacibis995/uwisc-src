/*
 * Copyright (c) 1980 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 *
 *	@(#)d_imag.c	5.1	6/7/85
 */

#include "complex"

double d_imag(z)
dcomplex *z;
{
return(z->dimag);
}
