#ifdef LIBC_RCS
_rcsid: .asciz	"$Header: strcpy.s,v 1.2 86/09/08 15:53:06 tadl Exp $"
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
	.asciz	"@(#)strcpy.s	5.3 (Berkeley) 3/9/86"
#endif LIBC_SCCS

/*
 * Copy string s2 over top of s1.
 * Return base of s1.
 *
 * char *
 * strcpy(s1, s2)
 *	char *s1, *s2;
 */
#include "DEFS.h"

ENTRY(strcpy, R6)
	movl	4(ap), r3	# r3 = s1
	movl	8(ap), r6	# r6 = s2
1:
	locc	$0,$65535,(r6)	# find length of s2
	bneq	2f
	movc3	$65535,(r6),(r3)# copy full block
	movl	r1,r6
	jbr	1b
2:
	subl2	r6,r1		# calculate length
	incl	r1
	movc3	r1,(r6),(r3)	# copy remainder
	movl	4(ap),r0	# return base of s1
	ret
