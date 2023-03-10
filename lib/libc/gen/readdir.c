#if defined(LIBC_RCS) && !defined(lint)
static char rcs_id[] = 
	"$Header: readdir.c,v 1.3 86/09/08 14:44:36 tadl Exp $";
#endif
/*
 * RCS info
 *	$Locker:  $
 */

#ifndef lint
static	char sccsid[] = "@(#)readdir.c 1.1 85/05/30 SMI";
#endif

#include <sys/param.h>
#include <sys/dir.h>

/*
 * get next entry in a directory.
 */
struct direct *
readdir(dirp)
	register DIR *dirp;
{
	register struct direct *dp;

	for (;;) {
		if (dirp->dd_loc == 0) {
			dirp->dd_size = getdirentries(dirp->dd_fd,
			    dirp->dd_buf, dirp->dd_bsize, &dirp->dd_bbase);
			if (dirp->dd_size <= 0)
				return (NULL);
			dirp->dd_entno = 0;
		}
		if (dirp->dd_loc >= dirp->dd_size) {
			dirp->dd_loc = 0;
			continue;
		}
		dp = (struct direct *)(dirp->dd_buf + dirp->dd_loc);
		if (dp->d_reclen <= 0)
			return (NULL);
		dirp->dd_loc += dp->d_reclen;
		dirp->dd_entno++;
		if (dp->d_fileno == 0)
			continue;
		return (dp);
	}
}
