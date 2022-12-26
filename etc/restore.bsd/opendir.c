#if defined(LIBC_RCS) && !defined(lint)
static char rcs_id[] = 
	"$Header: opendir.c,v 1.3 86/09/08 14:44:11 tadl Exp $";
#endif
/*
 * RCS info
 *	$Locker:  $
 */

#ifndef lint
static	char sccsid[] = "@(#)opendir.c 1.1 86/02/03 SMI";
/* @(#)opendir.c	2.1 86/04/11 NFSSRC */
#endif

#include <sys/param.h>
#include <ufs/fsdir.h>

/*
 * open a directory.
 */
DIR *
opendir(name)
	char *name;
{
	register DIR *dirp;
	register int fd;
	extern char *malloc();

	if ((fd = open(name, 0)) == -1)
		return (NULL);
	if ((dirp = (DIR *)malloc(sizeof(DIR))) == NULL) {
		close(fd);
		return (NULL);
	}
	dirp->dd_fd = fd;
	dirp->dd_loc = 0;
	return (dirp);
}
