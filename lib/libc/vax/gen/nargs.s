#ifdef LIBC_RCS
_rcsid: .asciz	"$Header: nargs.s,v 1.2 86/09/08 15:52:43 tadl Exp $"
#endif
/*
 * RCS info
 *	$Locker:  $
 */
/*
 * Copyright (c) 1980 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 */

#ifdef LIBC_SCCS
	.asciz	"@(#)nargs.s	5.3 (Berkeley) 3/9/86"
#endif LIBC_SCCS

/* C library -- nargs */

#include "DEFS.h"

ENTRY(nargs, 0)
	movzbl	*8(fp),r0	/* 8(fp) is old ap */
	ret
