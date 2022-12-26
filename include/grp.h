/*
 * RCS Info	
 *	$Header: grp.h,v 1.1 86/09/05 08:02:20 tadl Exp $
 *	$Locker: tadl $
 */
/*	grp.h	4.1	83/05/03	*/

struct	group { /* see getgrent(3) */
	char	*gr_name;
	char	*gr_passwd;
	int	gr_gid;
	char	**gr_mem;
};

struct group *getgrent(), *getgrgid(), *getgrnam();
