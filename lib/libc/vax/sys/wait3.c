#if defined(LIBC_RCS) && !defined(lint)
	.data
_rcsid: .asciz "$Header: wait3.c,v 1.2 86/09/08 16:00:56 tadl Exp $"
	.text
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

#ifdef SYSLIBC_SCCS
_sccsid:.asciz	"@(#)wait3.c	5.3 (Berkeley) 3/9/86"
#endif SYSLIBC_SCCS

/*	@(#)wait3.c	5.3	(Berkeley)	3/9/86	*/

/*
 * C library -- wait3
 *
 * pid = wait3(&status, flags, &rusage);
 *
 * pid == -1 if error
 * status indicates fate of process, if given
 * flags may indicate process is not to hang or
 * that untraced stopped children are to be reported.
 * rusage optionally returns detailed resource usage information
 */
#include "SYS.h"

#define	SYS_wait3	SYS_wait

ENTRY(wait3)
	movl	8(ap),r0	/* make it easy for system to get */
	movl	12(ap),r1	/* these extra arguments */
	bispsw	$0xf		/* flags wait3() */
	chmk	$SYS_wait3
	bcc 	noerror
	movl	r0,_errno
	mnegl	$1,r0
	ret
noerror:
	tstl	4(ap)		/* status desired? */
	beql	nostatus	/* no */
	movl	r1,*4(ap)	/* store child's status */
nostatus:
	ret
