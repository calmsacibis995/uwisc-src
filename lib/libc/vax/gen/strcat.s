#ifdef LIBC_RCS
_rcsid: .asciz	"$Header: strcat.s,v 1.2 86/09/08 15:53:00 tadl Exp $"
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
	.asciz	"@(#)strcat.s	5.3 (Berkeley) 3/9/86"
#endif LIBC_SCCS

/*
 * Concatenate string s2 to the end of s1
 * and return the base of s1.
 *
 * char *
 * strcat(s1, s2)
 *	char *s1, *s2;
 */
#include "DEFS.h"

ENTRY(strcat, R6|R7)
	movq	4(ap), r6	# r6 = s1; r7 = s2
	movl	r6,r1
0:
	locc	$0,$65535,(r1)	# look for '\0'
	beql	0b
	movl	r1,r3		# save end of s1
1:
	locc	$0,$65535,(r7)	# find length of s2
	bneq	2f
	movc3	$65535,(r7),(r3)# copy full block
	movl	r1,r7
	jbr	1b
2:
	subl2	r7,r1		# calculate length
	incl	r1
	movc3	r1,(r7),(r3)	# copy remainder
	movl	r6,r0
	ret
