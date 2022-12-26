#if defined(LIBC_RCS) && !defined(lint)
	.data
_rcsid: .asciz "$Header: statfs.c,v 1.2 86/09/08 16:00:16 tadl Exp $"
	.text
#endif
/*
 * RCS info
 *	$Locker:  $
 */

#include "SYS.h"

SYSCALL(statfs)
	ret
