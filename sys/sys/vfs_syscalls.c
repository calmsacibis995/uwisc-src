/*	@(#)vfs_syscalls.c 1.1 86/02/03 SMI     */
/*      NFSSRC @(#)vfs_syscalls.c	2.1 86/04/15 */

#include "param.h"
#include "user.h"
#include "file.h"
#include "stat.h"
#include "uio.h"
#include "ioctl.h"
#include "tty.h"
#include "vfs.h"
#include "pathname.h"
#include "vnode.h"
#include "../ufs/inode.h"		/* to make mknod compatable */

extern	struct fileops vnodefops;

/*
 * System call routines for operations on files other
 * than read, write and ioctl.  These calls manipulate
 * the per-process file table which references the
 * networkable version of normal UNIX inodes, called vnodes.
 *
 * Many operations take a pathname, which is read
 * into a kernel buffer by pn_get (see vfs_pathname.c).
 * After preparing arguments for an operation, a simple
 * operation proceeds:
 *
 *	error = lookupname(pname, seg, followlink, &dvp, &vp)
 *
 * where pname is the pathname operated on, seg is the segment that the
#ifdef BSD4_3
 * pathname is in (UIO_USERSPACE or UIO_SYSSPACE), followlink specifies
#else BSD4_3
 * pathname is in (UIOSEG_USER or UIOSEG_KERNEL), followlink specifies
#endif BSD4_3
 * whether to follow symbolic links, dvp is a pointer to the vnode that
 * represents the parent directory of vp, the pointer to the vnode
 * referenced by the pathname. The lookupname routine fetches the
 * pathname string into an internal buffer using pn_get (vfs_pathname.c),
 * and iteratively running down each component of the path until the
 * the final vnode and/or it's parent are found. If either of the addresses
 * for dvp or vp are NULL, then it assumes that the caller is not interested
 * in that vnode.  Once the vnode or its parent is found, then a vnode
 * operation (e.g. VOP_OPEN) may be applied to it.
 *
 * One important point is that the operations on vnode's are atomic, so that
 * vnode's are never locked at this level.  Vnode locking occurs
 * at lower levels either on this or a remote machine. Also permission
 * checking is generally done by the specific filesystem. The only
 * checks done by the vnode layer is checks involving file types
 * (e.g. VREG, VDIR etc.), since this is static over the life of the vnode.
 *
 */

/*
 * Change current working directory (".").
 */
chdir()
{
	register struct a {
		char *dirnamep;
	} *uap = (struct a *) u.u_ap;
	struct vnode *vp;

	u.u_error = chdirec(uap->dirnamep, &vp);
	if (u.u_error == 0) {
		VN_RELE(u.u_cdir);
		u.u_cdir = vp;
	}
}

/*
 * Change notion of root ("/") directory.
 */
chroot()
{
	register struct a {
		char *dirnamep;
	} *uap = (struct a *) u.u_ap;
	struct vnode *vp;

	if (!suser())
		return;

	u.u_error = chdirec(uap->dirnamep, &vp);
	if (u.u_error == 0) {
		if (u.u_rdir != (struct vnode *)0)
			VN_RELE(u.u_rdir);
		u.u_rdir = vp;
	}
}

/*
 * Common code for chdir and chroot.
 * Translate the pathname and insist that it
 * is a directory to which we have execute access.
 * If it is replace u.u_[cr]dir with new vnode.
 */
chdirec(dirnamep, vpp)
	char *dirnamep;
	struct vnode **vpp;
{
	struct vnode *vp;		/* new directory vnode */
	register int error;

        error =
	    lookupname(dirnamep, UIO_USERSPACE, FOLLOW_LINK,
		(struct vnode **)0, &vp);
	if (error)
		return (error);
	if (vp->v_type != VDIR) {
		error = ENOTDIR;
	} else {
		error = VOP_ACCESS(vp, VEXEC, u.u_cred);
	}
	if (error) {
		VN_RELE(vp);
	} else {
		*vpp = vp;
	}
	return (error);
}

/*
 * Open system call.
 */
open()
{
	register struct a {
		char *fnamep;
		int fmode;
		int cmode;
	} *uap = (struct a *) u.u_ap;

	u.u_error = copen(uap->fnamep, uap->fmode - FOPEN, uap->cmode);
}

/*
 * Creat system call.
 */
creat()
{
	register struct a {
		char *fnamep;
		int cmode;
	} *uap = (struct a *) u.u_ap;

	u.u_error = copen(uap->fnamep, FWRITE|FCREAT|FTRUNC, uap->cmode);
}

/*
 * Common code for open, creat.
 */
copen(pnamep, filemode, createmode)
	char *pnamep;
	int filemode;
	int createmode;
{
	register struct file *fp;
	struct vnode *vp;
	register int error;
	register int i;

	/*
	 * allocate a user file descriptor and file table entry.
	 */
	fp = falloc();
	if (fp == NULL)
		return(u.u_error);
	i = u.u_r.r_val1;		/* this is bullshit */
	/*
	 * open the vnode.
	 */
	error =
	    vn_open(pnamep, UIO_USERSPACE,
		filemode, ((createmode & 07777) & ~u.u_cmask), &vp);

	/*
	 * If there was an error, deallocate the file descriptor.
	 * Otherwise fill in the file table entry to point to the vnode.
	 */
	if (error) {
		u.u_ofile[i] = NULL;
		crfree(fp->f_cred);
		fp->f_count = 0;
	} else {
		fp->f_flag = filemode & FMASK;
		fp->f_type = DTYPE_VNODE;
		fp->f_data = (caddr_t)vp;
		fp->f_ops = &vnodefops;
	}
	return(error);
}

/*
 * Create a special (or regular) file.
 */
mknod()
{
	register struct a {
		char		*pnamep;
		int		fmode;
		int		dev;
	} *uap = (struct a *) u.u_ap;
	struct vnode *vp;
	struct vattr vattr;

	/*
	 * Must be super user
	 */
	if (!suser())
		return;

	/*
	 * Setup desired attributes and vn_create the file.
	 */
	vattr_null(&vattr);
	vattr.va_type = IFTOVT(uap->fmode);
	vattr.va_mode = (uap->fmode & 07777) & ~u.u_cmask;
	/*
	 * Can't mknod directories. Use mkdir.
	 */
	if (vattr.va_type == VDIR) {
		u.u_error = EISDIR;
		return;	
	}
	switch (vattr.va_type) {
	case VBAD:
	case VCHR:
	case VBLK:
		vattr.va_rdev = uap->dev;
		break;

	case VNON:
		u.u_error = EINVAL;
		return;

	default:
		break;
	}

	u.u_error = vn_create(uap->pnamep, UIO_USERSPACE, &vattr, EXCL, 0, &vp);
	if (u.u_error == 0)
		VN_RELE(vp);
}

/*
 * Make a directory.
 */
mkdir()
{
	struct a {
		char	*dirnamep;
		int	dmode;
	} *uap = (struct a *) u.u_ap;
	struct vnode *vp;
	struct vattr vattr;

	vattr_null(&vattr);
	vattr.va_type = VDIR;
	vattr.va_mode = (uap->dmode & 0777) & ~u.u_cmask;

	u.u_error = vn_create(uap->dirnamep, UIO_USERSPACE, &vattr, EXCL, 0, &vp);
	if (u.u_error == 0)
		VN_RELE(vp);
}

/*
 * make a hard link
 */
link()
{
	register struct a {
		char	*from;
		char	*to;
	} *uap = (struct a *) u.u_ap;

	u.u_error = vn_link(uap->from, uap->to, UIO_USERSPACE);
}

/*
 * rename or move an existing file
 */
rename()
{
	register struct a {
		char	*from;
		char	*to;
	} *uap = (struct a *) u.u_ap;

	u.u_error = vn_rename(uap->from, uap->to, UIO_USERSPACE);
}

/*
 * Create a symbolic link.
 * Similar to link or rename except target
 * name is passed as string argument, not
 * converted to vnode reference.
 */
symlink()
{
	register struct a {
		char	*target;
		char	*linkname;
	} *uap = (struct a *) u.u_ap;
	struct vnode *dvp;
	struct vattr vattr;
	struct pathname tpn;
	struct pathname lpn;

	u.u_error = pn_get(uap->linkname, UIO_USERSPACE, &lpn);
	if (u.u_error)
		return;
	u.u_error = lookuppn(&lpn, NO_FOLLOW, &dvp, (struct vnode **)0);
	if (u.u_error) {
		pn_free(&lpn);
		return;
	}
	if (dvp->v_vfsp->vfs_flag & VFS_RDONLY) {
		u.u_error = EROFS;
		goto out;
	}
	u.u_error = pn_get(uap->target, UIO_USERSPACE, &tpn);
	vattr_null(&vattr);
	vattr.va_mode = 0777;
	if (u.u_error == 0) {
		u.u_error =
		   VOP_SYMLINK(dvp, lpn.pn_path, &vattr, tpn.pn_path, u.u_cred);
		pn_free(&tpn);
	}
out:
	pn_free(&lpn);
	VN_RELE(dvp);
}

/*
 * Unlink (i.e. delete) a file.
 */
unlink()
{
	struct a {
		char	*pnamep;
	} *uap = (struct a *) u.u_ap;

	u.u_error = vn_remove(uap->pnamep, UIO_USERSPACE, 0);
}

/*
 * Remove a directory.
 */
rmdir()
{
	struct a {
		char	*dnamep;
	} *uap = (struct a *) u.u_ap;

	u.u_error = vn_remove(uap->dnamep, UIO_USERSPACE, 1);
}

/*
 * get directory entries in a file system independent format
 */
getdirentries()
{
	register struct a {
		int	fd;
		char	*buf;
		unsigned count;
		long	*basep;
	} *uap = (struct a *) u.u_ap;
	struct file *fp;
	struct uio auio;
	struct iovec aiov;

	u.u_error = getvnodefp(uap->fd, &fp);
	if (u.u_error)
		return;
	if ((fp->f_flag & FREAD) == 0) {
		u.u_error = EBADF;
		return;
	}
	aiov.iov_base = uap->buf;
	aiov.iov_len = uap->count;
	auio.uio_iov = &aiov;
	auio.uio_iovcnt = 1;
	auio.uio_offset = fp->f_offset;
	auio.uio_segflg = UIO_USERSPACE;
	auio.uio_resid = uap->count;
	u.u_error = VOP_READDIR((struct vnode *)fp->f_data, &auio, fp->f_cred);
	if (u.u_error)
		return;
	u.u_error =
	    copyout((caddr_t)&fp->f_offset, (caddr_t)uap->basep, sizeof(long));
	u.u_r.r_val1 = uap->count - auio.uio_resid;
	fp->f_offset = auio.uio_offset;
}

/*
 * Seek on file.  Only hard operation
 * is seek relative to end which must
 * apply to vnode for current file size.
 * 
 * Note: lseek(0, 0, L_XTND) costs much more than it did before.
 */
lseek()
{
	register struct a {
		int	fd;
		off_t	off;
		int	sbase;
	} *uap = (struct a *) u.u_ap;
	struct file *fp;

	u.u_error = getvnodefp(uap->fd, &fp);
	if (u.u_error)
		return;
	switch (uap->sbase) {

	case L_INCR:
		fp->f_offset += uap->off;
		break;

	case L_XTND: {
		struct vattr vattr;

		u.u_error =
		    VOP_GETATTR((struct vnode *)fp->f_data, &vattr, u.u_cred);
		if (u.u_error)
			return;
		fp->f_offset = uap->off + vattr.va_size;
		break;
	}

	case L_SET:
		fp->f_offset = uap->off;
		break;

	default:
		u.u_error = EINVAL;
	}
	u.u_r.r_off = fp->f_offset;
}

/*
 * Determine accessibility of file, by
 * reading its attributes and then checking
 * against our protection policy.
 */
access()
{
	register struct a {
		char	*fname;
		int	fmode;
	} *uap = (struct a *) u.u_ap;
	struct vnode *vp;
	register u_short mode;
	register int svuid;
	register int svgid;

	/*
	 * Lookup file
	 */
	u.u_error =
	    lookupname(uap->fname, UIO_USERSPACE, FOLLOW_LINK,
		(struct vnode **)0, &vp);
	if (u.u_error)
		return;

	/*
	 * Use the real uid and gid and check access
	 */
	svuid = u.u_uid;
	svgid = u.u_gid;
	u.u_uid = u.u_ruid;
	u.u_gid = u.u_rgid;

	mode = 0;
	/*
	 * fmode == 0 means only check for exist
	 */
	if (uap->fmode) {
		if (uap->fmode & R_OK)
			mode |= VREAD;
		if (uap->fmode & W_OK) {
			if(vp->v_vfsp->vfs_flag & VFS_RDONLY) {
				u.u_error = EROFS;
				goto out;
			}
			mode |= VWRITE;
		}
		if (uap->fmode & X_OK)
			mode |= VEXEC;
		u.u_error = VOP_ACCESS(vp, mode, u.u_cred);
	}

	/*
	 * release the vnode and restore the uid and gid
	 */
out:
	VN_RELE(vp);
	u.u_uid = svuid;
	u.u_gid = svgid;
}

/*
 * Get attributes from file or file descriptor.
 * Argument says whether to follow links, and is
 * passed through in flags.
 */
stat()
{
	struct a {
		char	*fname;
		struct	stat *ub;
	} *uap = (struct a *) u.u_ap;

	u.u_error = stat1(uap, FOLLOW_LINK);
}

lstat()
{
	struct a {
		char	*fname;
		struct	stat *ub;
	} *uap = (struct a *) u.u_ap;

	u.u_error = stat1(uap, NO_FOLLOW);
}

stat1(uap, follow)
	register struct a {
		char	*fname;
		struct	stat *ub;
	} *uap;
	enum symfollow follow;
{
	struct vnode *vp;
	struct stat sb;
	register int error;

	error =
	    lookupname(uap->fname, UIO_USERSPACE, follow,
		(struct vnode **)0, &vp);
	if (error)
		return (error);
	error = vno_stat(vp, &sb);
	VN_RELE(vp);
	if (error)
		return (error);
	return (copyout((caddr_t)&sb, (caddr_t)uap->ub, sizeof (sb)));
}

/*
 * Read contents of symbolic link.
 */
readlink()
{
	register struct a {
		char	*name;
		char	*buf;
		int	count;
	} *uap = (struct a *) u.u_ap;
	struct vnode *vp;
	struct iovec aiov;
	struct uio auio;

	u.u_error =
	    lookupname(uap->name, UIO_USERSPACE, NO_FOLLOW,
		(struct vnode **)0, &vp);
	if (u.u_error)
		return;
	if (vp->v_type != VLNK) {
		u.u_error = EINVAL;
		goto out;
	}
	aiov.iov_base = uap->buf;
	aiov.iov_len = uap->count;
	auio.uio_iov = &aiov;
	auio.uio_iovcnt = 1;
	auio.uio_offset = 0;
	auio.uio_segflg = UIO_USERSPACE;
	auio.uio_resid = uap->count;
	u.u_error = VOP_READLINK(vp, &auio, u.u_cred);
out:
	VN_RELE(vp);
	u.u_r.r_val1 = uap->count - auio.uio_resid;
}

/*
 * Change mode of file given path name.
 */
chmod()
{
	register struct a {
		char	*fname;
		int	fmode;
	} *uap = (struct a *) u.u_ap;
	struct vattr vattr;

	vattr_null(&vattr);
	vattr.va_mode = uap->fmode & 07777;
	u.u_error = namesetattr(uap->fname, FOLLOW_LINK, &vattr);
}

/*
 * Change mode of file given file descriptor.
 */
fchmod()
{
	register struct a {
		int	fd;
		int	fmode;
	} *uap = (struct a *) u.u_ap;
	struct vattr vattr;

	vattr_null(&vattr);
	vattr.va_mode = uap->fmode & 07777;
	u.u_error = fdsetattr(uap->fd, &vattr);
}

/*
 * Change ownership of file given file name.
 */
chown()
{
	register struct a {
		char	*fname;
		int	uid;
		int	gid;
	} *uap = (struct a *) u.u_ap;
	struct vattr vattr;

	if (!suser())
		return;
	vattr_null(&vattr);
	vattr.va_uid = uap->uid;
	vattr.va_gid = uap->gid;
	u.u_error = namesetattr(uap->fname, NO_FOLLOW,  &vattr);
}

/*
 * Change ownership of file given file descriptor.
 */
fchown()
{
	register struct a {
		int	fd;
		int	uid;
		int	gid;
	} *uap = (struct a *) u.u_ap;
	struct vattr vattr;

	if (!suser())
		return;
	vattr_null(&vattr);
	vattr.va_uid = uap->uid;
	vattr.va_gid = uap->gid;
	u.u_error = fdsetattr(uap->fd, &vattr);
}

/*
 * Set access/modify times on named file.
 */
utimes()
{
	register struct a {
		char	*fname;
		struct	timeval *tptr;
	} *uap = (struct a *) u.u_ap;
	struct timeval tv[2];
	struct vattr vattr;

	u.u_error = copyin((caddr_t)uap->tptr, (caddr_t)tv, sizeof (tv));
	if (u.u_error)
		return;
	vattr_null(&vattr);
	vattr.va_atime = tv[0];
	vattr.va_mtime = tv[1];
	u.u_error = namesetattr(uap->fname, FOLLOW_LINK, &vattr);
}

/*
 * Truncate a file given its path name.
 */
truncate()
{
	register struct a {
		char	*fname;
		int	length;
	} *uap = (struct a *) u.u_ap;
	struct vattr vattr;

	vattr_null(&vattr);
	vattr.va_size = uap->length;
	u.u_error = namesetattr(uap->fname, FOLLOW_LINK, &vattr);
}

/*
 * Truncate a file given a file descriptor.
 */
ftruncate()
{
	register struct a {
		int	fd;
		int	length;
	} *uap = (struct a *) u.u_ap;
	register struct vnode *vp;
	struct file *fp;

	u.u_error = getvnodefp(uap->fd, &fp);
	if (u.u_error)
		return;
	vp = (struct vnode *)fp->f_data;
	if ((fp->f_flag & FWRITE) == 0) {
		u.u_error = EINVAL;
	} else if (vp->v_vfsp->vfs_flag & VFS_RDONLY) {
		u.u_error = EROFS;
	} else {
		struct vattr vattr;

		vattr_null(&vattr);
		vattr.va_size = uap->length;
		u.u_error = VOP_SETATTR(vp, &vattr, fp->f_cred);
	}
}

/*
 * Common routine for modifying attributes
 * of named files.
 */
namesetattr(fnamep, followlink, vap)
	char *fnamep;
	enum symfollow followlink;
	struct vattr *vap;
{
	struct vnode *vp;
	register int error;

	error =
	    lookupname(fnamep, UIO_USERSPACE, followlink,
		 (struct vnode **)0, &vp);
	if (error)
		return(error);	
	if(vp->v_vfsp->vfs_flag & VFS_RDONLY)
		error = EROFS;
	else
		error = VOP_SETATTR(vp, vap, u.u_cred);
	VN_RELE(vp);
	return(error);
}

/*
 * Common routine for modifying attributes
 * of file referenced by descriptor.
 */
fdsetattr(fd, vap)
	int fd;
	struct vattr *vap;
{
	struct file *fp;
	register struct vnode *vp;
	register int error;

	error = getvnodefp(fd, &fp);
	if (error == 0) {
		vp = (struct vnode *)fp->f_data;
		if(vp->v_vfsp->vfs_flag & VFS_RDONLY)
			return(EROFS);
		error = VOP_SETATTR(vp, vap, fp->f_cred);
	}
	return(error);
}

/*
 * Flush output pending for file.
 */
fsync()
{
	struct a {
		int	fd;
	} *uap = (struct a *) u.u_ap;
	struct file *fp;

	u.u_error = getvnodefp(uap->fd, &fp);
	if (u.u_error == 0)
		u.u_error = VOP_FSYNC((struct vnode *)fp->f_data, fp->f_cred);
}

/*
 * Set file creation mask.
 */
umask()
{
	register struct a {
		int mask;
	} *uap = (struct a *) u.u_ap;

	u.u_r.r_val1 = u.u_cmask;
	u.u_cmask = uap->mask & 07777;
}

/*
 * Revoke access the current tty by all processes.
 * Used only by the super-user in init
 * to give ``clean'' terminals at login.
 */
vhangup()
{

	if (!suser())
		return;
	if (u.u_ttyp == NULL)
		return;
	forceclose(u.u_ttyd);
	if ((u.u_ttyp->t_state) & TS_ISOPEN)
		gsignal(u.u_ttyp->t_pgrp, SIGHUP);
}

forceclose(dev)
	dev_t dev;
{
	register struct file *fp;
	register struct vnode *vp;

	for (fp = file; fp < fileNFILE; fp++) {
		if (fp->f_count == 0)
			continue;
		if (fp->f_type != DTYPE_VNODE)
			continue;
		vp = (struct vnode *)fp->f_data;
		if (vp == 0)
			continue;
		if (vp->v_type != VCHR)
			continue;
		if (vp->v_rdev != dev)
			continue;
		fp->f_flag &= ~(FREAD|FWRITE);
	}
}

/*
 * Get the file structure entry for the file descrpitor, but make sure
 * its a vnode.
 */
int
getvnodefp(fd, fpp)
	int fd;
	struct file **fpp;
{
	register struct file *fp;

	fp = getf(fd);
	if (fp == (struct file *)0)
		return(EBADF);
	if (fp->f_type != DTYPE_VNODE)
		return(EINVAL);
	*fpp = fp;
	return(0);
}
