/*
 * Copyright (c) 1982, 1986 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 *
 *	@(#)pdma.h	7.1 (Berkeley) 6/5/86
 */
/*
 * RCS Info	
 *	$Header: pdma.h,v 3.1 86/10/22 14:04:37 tadl Exp $
 *	$Locker:  $
 */

struct pdma {
	struct	dzdevice *p_addr;
	char	*p_mem;
	char	*p_end;
	int	p_arg;
	int	(*p_fcn)();
};
