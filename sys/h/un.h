/*
 * Copyright (c) 1982, 1986 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 *
 *	@(#)un.h	7.1 (Berkeley) 6/4/86
 */
/*
 * RCS Info	
 *	$Header: un.h,v 3.1 86/10/22 13:29:49 tadl Exp $
 *	$Locker:  $
 */

/*
 * Definitions for UNIX IPC domain.
 */
struct	sockaddr_un {
	short	sun_family;		/* AF_UNIX */
	char	sun_path[108];		/* path name (gag) */
};

#ifdef KERNEL
int	unp_discard();
#endif
