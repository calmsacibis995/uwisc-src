#if defined(LIBC_RCS) && !defined(lint)
	.data
_rcsid: .asciz "$Header: quotactl.c,v 1.2 86/09/08 15:58:14 tadl Exp $"
	.text
#endif
/*
 * RCS info
 *	$Locker:  $
 */

#include "SYS.h"

SYSCALL(quotactl)
	ret
