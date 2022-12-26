#if defined(LIBC_RCS) && !defined(lint)
	.data
_rcsid: .asciz "$Header: syscall.c,v 1.2 86/09/08 16:00:29 tadl Exp $"
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
_sccsid:.asciz	"@(#)syscall.c	5.3 (Berkeley) 3/9/86"
#endif SYSLIBC_SCCS

#include "SYS.h"

ENTRY(syscall)
	movl	4(ap),r0	# syscall number
	subl3	$1,(ap)+,(ap)	# one fewer arguments
	chmk	r0
	jcs	1f
	ret
1:
	jmp	cerror
