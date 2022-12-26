#if defined(LIBC_RCS) && !defined(lint)
	.data
_rcsid: .asciz "$Header: Ovadvise.c,v 1.2 86/09/08 15:54:20 tadl Exp $"
	.text
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

#ifdef SYSLIBC_SCCS
_sccsid:.asciz	"@(#)Ovadvise.c	5.3 (Berkeley) 3/9/86"
#endif SYSLIBC_SCCS

#include "SYS.h"

#define	SYS_vadvise	72

SYSCALL(vadvise)
	ret
