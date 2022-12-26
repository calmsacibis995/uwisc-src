#if defined(LIBC_RCS) && !defined(lint)
	.data
_rcsid: .asciz "$Header:$"
	.text
#endif
/*
 * RCS info
 *	$Locker:$
 */
/*
 * Copyright (c) 1983 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 */

#ifdef SYSLIBC_SCCS
_sccsid:.asciz	"@(#)umount.c	5.3 (Berkeley) 3/9/86"
#endif SYSLIBC_SCCS

#include "SYS.h"

SYSCALL(umount)
	ret
