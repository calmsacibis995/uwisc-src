#if defined(LIBC_RCS) && !defined(lint)
	.data
_rcsid: .asciz "$Header: sendmsg.c,v 1.2 86/09/08 15:58:57 tadl Exp $"
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
_sccsid:.asciz	"@(#)sendmsg.c	5.3 (Berkeley) 3/9/86"
#endif SYSLIBC_SCCS

#include "SYS.h"

SYSCALL(sendmsg)
	ret
