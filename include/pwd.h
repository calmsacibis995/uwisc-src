/*
 * RCS Info	
 *	$Header: pwd.h,v 1.1 86/09/05 08:02:44 tadl Exp $
 *	$Locker: tadl $
 */
/*	pwd.h	4.1	83/05/03	*/

struct	passwd { /* see getpwent(3) */
	char	*pw_name;
	char	*pw_passwd;
	int	pw_uid;
	int	pw_gid;
	int	pw_quota;
	char	*pw_comment;
	char	*pw_gecos;
	char	*pw_dir;
	char	*pw_shell;
};

struct passwd *getpwent(), *getpwuid(), *getpwnam();
