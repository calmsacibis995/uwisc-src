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
#include <sys/stat.h>
#include <sys/dir.h>
#include <errno.h>

/*
 * open a directory.
 */
DIR *
opendir(name)
	char *name;
{
	register DIR *dirp;
	register int fd;
	struct stat sb;
	extern int errno;
	extern char *malloc();

	if ((fd = open(name, 0)) == -1)
		return (NULL);
	if (fstat(fd, &sb) == -1) {
		close(fd);
		return (NULL);
	}
	if ((sb.st_mode & S_IFMT) != S_IFDIR) {
		errno = ENOTDIR;
		close(fd);
		return (NULL);
	}
	if (((dirp = (DIR *)malloc(sizeof(DIR))) == NULL) ||
	    ((dirp->dd_buf = malloc((int)sb.st_blksize)) == NULL)) {
		if (dirp) {
			if (dirp->dd_buf) {
				free(dirp->dd_buf);
			}
			free(dirp);
		}
		close(fd);
		return (NULL);
	}
	dirp->dd_bsize = sb.st_blksize;
	dirp->dd_bbase = 0;
	dirp->dd_entno = 0;
	dirp->dd_fd = fd;
	dirp->dd_loc = 0;
	return (dirp);
}
