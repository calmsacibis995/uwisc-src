/*
 * RCS Info	
 *	$Header: mtab.h,v 1.1 86/09/05 08:02:31 tadl Exp $
 *	$Locker: tadl $
 */
/*
 * Copyright (c) 1980 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 *
 *	@(#)mtab.h	5.1 (Berkeley) 5/30/85
 */

/*
 * Mounted device accounting file.
 */
struct mtab {
	char	m_path[32];		/* mounted on pathname */
	char	m_dname[32];		/* block device pathname */
	char	m_type[4];		/* read-only, quotas */
};
