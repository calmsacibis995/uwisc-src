#ifdef LIBC_RCS
_rcsid: .asciz	"$Header: abs.s,v 1.2 86/09/08 15:51:49 tadl Exp $"
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
	.asciz	"@(#)abs.s	5.3 (Berkeley) 3/9/86"
#endif LIBC_SCCS


/* abs - int absolute value */

#include "DEFS.h"

ENTRY(abs, 0)
	movl	4(ap),r0
	bgeq	1f
	mnegl	r0,r0
1:
	ret
