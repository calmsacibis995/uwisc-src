/*	@(#)vfs_pathname.c 1.1 86/02/03 SMI	*/
/*      NFSSRC @(#)vfs_pathname.c	2.1 86/04/15 */

#include "param.h"
#include "systm.h"
#include "dir.h"
#include "uio.h"
#include "errno.h"
#include "pathname.h"

/*
 * Pathname utilities.
 *
 * In translating file names we copy each argument file
 * name into a pathname structure where we operate on it.
 * Each pathname structure can hold MAXPATHLEN characters
 * including a terminating null, and operations here support
 * allocating and freeing pathname structures, fetching
 * strings from user space, getting the next character from
 * a pathname, combining two pathnames (used in symbolic
 * link processing), and peeling off the first component
 * of a pathname.
 */

/*
 * Allocate contents of pathname structure.
 * Structure itself is typically automatic
 * variable in calling routine for convenience.
 */
pn_alloc(pnp)
	register struct pathname *pnp;
{
	pnp->pn_buf = (char *)kmem_alloc((u_int)MAXPATHLEN);
	pnp->pn_path = (char *)pnp->pn_buf;
	pnp->pn_pathlen = 0;
}

/*
 * Pull a pathname from user user or kernel space
 */
int
pn_get(str, seg, pnp)
	register struct pathname *pnp;
	int seg;
	register char *str;
{
	u_int clen;
	int error;

	pn_alloc(pnp);
	if (seg == UIO_SYSSPACE)
		error = copystr(str, pnp->pn_path, MAXPATHLEN, &clen);
	else
		error = copyinstr(str, pnp->pn_path, MAXPATHLEN, &clen);

	if (error) {
		pn_free(pnp);
		return (error);
	}

	pnp->pn_pathlen = clen;
	if (clen < MAXPATHLEN)
		return (0);

	pn_free(pnp);
	return (ENAMETOOLONG);
}

#ifdef notneeded
/*
 * Get next character from a path.
 * Return null at end forever.
 */
pn_getchar(pnp)
	register struct pathname *pnp;
{

	if (pnp->pn_pathlen == 0)
		return (0);
	pnp->pn_pathlen--;
	return (*pnp->pn_path++);
}
#endif notneeded

/*
 * Set pathname to argument string.
 */
pn_set(pnp, path)
	register struct pathname *pnp;
	register char *path;
{
	register char *cp;
	extern char *strncpy();

	pnp->pn_path = pnp->pn_buf;
	cp = strncpy(pnp->pn_path, path, MAXPATHLEN);
	pnp->pn_pathlen = cp - pnp->pn_path;
	if (pnp->pn_pathlen >= MAXPATHLEN)
		return (ENAMETOOLONG);
	return (0);
}

/*
 * Combine two argument pathnames by putting
 * second argument before first in first's buffer,
 * and freeing second argument.
 * This isn't very general: it is designed specifically
 * for symbolic link processing.
 */
pn_combine(pnp, sympnp)
	register struct pathname *pnp;
	register struct pathname *sympnp;
{

	if (pnp->pn_pathlen + sympnp->pn_pathlen >= MAXPATHLEN)
		return (ENAMETOOLONG);
	ovbcopy(pnp->pn_path, pnp->pn_buf + sympnp->pn_pathlen,
	    (u_int)pnp->pn_pathlen);
	bcopy(sympnp->pn_path, pnp->pn_buf, (u_int)sympnp->pn_pathlen);
	pnp->pn_pathlen += sympnp->pn_pathlen;
	pnp->pn_path = pnp->pn_buf;
	return (0);
}

/*
 * Strip next component off a pathname and leave in
 * buffer comoponent which should have room for
 * MAXNAMLEN bytes and a null terminator character.
 */
pn_getcomponent(pnp, component)
	register struct pathname *pnp;
	register char *component;
{
	register char *cp;
	register int l;
	register int n;

	cp = pnp->pn_path;
	l = pnp->pn_pathlen;
	n = MAXNAMLEN;
	while ((l > 0) && (*cp != '/')) {
		if (--n < 0)
			return(ENAMETOOLONG);
		*component++ = *cp++;
		--l;
	}
	pnp->pn_path = cp;
	pnp->pn_pathlen = l;
	*component = 0;
	return (0);
}

/*
 * skip over consecutive slashes in the pathname
 */
void
pn_skipslash(pnp)
	register struct pathname *pnp;
{
	while ((pnp->pn_pathlen > 0) && (*pnp->pn_path == '/')) {
		pnp->pn_path++;
		pnp->pn_pathlen--;
	}
}

/*
 * Free pathname resources.
 */
void
pn_free(pnp)
	register struct pathname *pnp;
{

	kmem_free((caddr_t)pnp->pn_buf, (u_int)MAXPATHLEN);
	pnp->pn_buf = 0;
}
