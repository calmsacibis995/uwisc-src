#ifdef LIBC_RCS
_rcsid: .asciz	"$Header: fabs.s,v 1.2 86/09/08 15:52:20 tadl Exp $"
#endif
/*
 * RCS info
 *	$Locker:  $
 */
#ifdef LIBC_SCCS
	.asciz	"@(#)fabs.s	5.2 (Berkeley) 3/9/86"
#endif LIBC_SCCS

/* fabs - floating absolute value */

#include "DEFS.h"

ENTRY(fabs, 0)
	movd	4(ap),r0
	bgeq	1f
	mnegd	r0,r0
1:
	ret
