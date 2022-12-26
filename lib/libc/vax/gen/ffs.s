#ifdef LIBC_RCS
_rcsid: .asciz	"$Header: ffs.s,v 1.2 86/09/08 15:52:23 tadl Exp $"
#endif
/*
 * RCS info
 *	$Locker:  $
 */
/*
 * Copyright (c) 1983 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 */

#ifdef LIBC_SCCS
	.asciz	"@(#)ffs.s	5.3 (Berkeley) 3/9/86"
#endif LIBC_SCCS

/* bit = ffs(value) */

#include "DEFS.h"

ENTRY(ffs, 0)
	ffs	$0,$32,4(ap),r0
	bneq	1f
	mnegl	$1,r0
1:
	incl	r0
	ret
