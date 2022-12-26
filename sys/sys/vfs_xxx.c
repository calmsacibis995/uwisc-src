/*	@(#)vfs_xxx.c 1.1 85/05/30 SMI; from UCB 4.7 83/06/21	*/

#ifdef COMPAT
#include "param.h"
#include "systm.h"
#include "user.h"
#include "vnode.h"
#include "file.h"
#include "uio.h"


/*
 * Oh, how backwards compatibility is ugly!!!
 */
struct	ostat {
	dev_t	ost_dev;
	u_short	ost_ino;
	u_short ost_mode;
	short  	ost_nlink;
	short  	ost_uid;
	short  	ost_gid;
	dev_t	ost_rdev;
	int	ost_size;
	int	ost_atime;
	int	ost_mtime;
	int	ost_ctime;
};

/*
 * The old fstat system call.
 */
ofstat()
{
	register struct a {
		int	fd;
		struct ostat *sb;
	} *uap = (struct a *)u.u_ap;
	struct file *fp;
	extern struct file *getinode();

	u.u_error = getvnodefp(uap->fd, &fp);
	if (u.u_error)
		return;
	u.u_error = ostat1((struct vnode *)fp->f_data, uap->sb);
}

/*
 * Old stat system call.  This version follows links.
 */
ostat()
{
	struct vnode *vp;
	register struct a {
		char	*fname;
		struct ostat *sb;
	} *uap;

	uap = (struct a *)u.u_ap;
	u.u_error =
	    lookupname(uap->fname, UIO_USERSPACE, FOLLOW_LINK,
		(struct vnode **)0, &vp);
	if (u.u_error)
		return;
	u.u_error = ostat1(vp, uap->sb);
	VN_RELE(vp);
}

int
ostat1(vp, ub)
	register struct vnode *vp;
	struct ostat *ub;
{
	struct ostat ds;
	struct vattr vattr;
	register int error;

	error = VOP_GETATTR(vp, &vattr, u.u_cred);
	if (error)
		return(error);
	/*
	 * Copy from inode table
	 */
	ds.ost_dev = vattr.va_fsid;
	ds.ost_ino = (short)vattr.va_nodeid;
	ds.ost_mode = (u_short)vattr.va_mode;
	ds.ost_nlink = vattr.va_nlink;
	ds.ost_uid = (short)vattr.va_uid;
	ds.ost_gid = (short)vattr.va_gid;
	ds.ost_rdev = (dev_t)vattr.va_rdev;
	ds.ost_size = (int)vattr.va_size;
	ds.ost_atime = (int)vattr.va_atime.tv_sec;
	ds.ost_mtime = (int)vattr.va_mtime.tv_sec;
	ds.ost_ctime = (int)vattr.va_atime.tv_sec;
	return (copyout((caddr_t)&ds, (caddr_t)ub, sizeof(ds)));
}

/*
 * Set IUPD and IACC times on file.
 * Can't set ICHG.
 */
outime()
{
	register struct a {
		char	*fname;
		time_t	*tptr;
	} *uap = (struct a *)u.u_ap;
	struct vattr vattr;
	time_t tv[2];

	u.u_error = copyin((caddr_t)uap->tptr, (caddr_t)tv, sizeof (tv));
	if (u.u_error)
		return;
	vattr_null(&vattr);
	vattr.va_atime.tv_sec = tv[0];
	vattr.va_atime.tv_usec = 0;
	vattr.va_mtime.tv_sec = tv[1];
	vattr.va_mtime.tv_usec = 0;
	u.u_error = namesetattr(uap->fname, FOLLOW_LINK, &vattr);
}
#endif
