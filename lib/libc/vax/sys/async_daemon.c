#if defined(LIBC_RCS) && !defined(lint)
	.data
_rcsid: .asciz "$Header: async_daemon.c,v 1.2 86/09/08 15:54:36 tadl Exp $"
	.text
#endif
/*
 * RCS info
 *	$Locker:  $
 */

#include "SYS.h"

SYSCALL(async_daemon)
	ret
