/*	@(#)ufs_vnodeops.c 1.1 86/02/03 SMI	*/
/*	@(#)ufs_vnodeops.c	2.1 86/04/14 NFSSRC */

#include "param.h"
#include "systm.h"
#include "user.h"
#include "buf.h"
#include "vfs.h"
#include "vnode.h"
#include "proc.h"
#include "file.h"
#include "uio.h"
#include "conf.h"
#include "kernel.h"
#include "cmap.h"
#include "../ufs/fs.h"
#include "../ufs/inode.h"
#include "../ufs/mount.h"
#include "../ufs/fsdir.h"
#ifdef QUOTA
#include "../ufs/quota.h"
#endif

extern int ufs_open();
extern int ufs_close();
extern int ufs_rdwr();
extern int ufs_ioctl();
extern int ufs_select();
extern int ufs_getattr();
extern int ufs_setattr();
extern int ufs_access();
extern int ufs_lookup();
extern int ufs_create();
extern int ufs_remove();
extern int ufs_link();
extern int ufs_rename();
extern int ufs_mkdir();
extern int ufs_rmdir();
extern int ufs_readdir();
extern int ufs_symlink();
extern int ufs_readlink();
extern int ufs_fsync();
extern int ufs_inactive();
extern int ufs_bmap();
extern int ufs_badop();
extern int ufs_bread();
extern int ufs_brelse();

struct vnodeops ufs_vnodeops = {
	ufs_open,
	ufs_close,
	ufs_rdwr,
	ufs_ioctl,
	ufs_select,
	ufs_getattr,
	ufs_setattr,
	ufs_access,
	ufs_lookup,
	ufs_create,
	ufs_remove,
	ufs_link,
	ufs_rename,
	ufs_mkdir,
	ufs_rmdir,
	ufs_readdir,
	ufs_symlink,
	ufs_readlink,
	ufs_fsync,
	ufs_inactive,
	ufs_bmap,
	ufs_badop,
	ufs_bread,
	ufs_brelse
};

/*ARGSUSED*/
int
ufs_open(vpp, flag, cred)
	struct vnode **vpp;
	int flag;
	struct ucred *cred;
{
	struct inode *ip;
	dev_t dev;
	register int majnum;
	int minnum;
	int error;

	/*
	 * Setjmp in case open is interrupted.
	 * If it is, close and return error.
	 */
	if (setjmp(&u.u_qsave)) {
		error = EINTR;
		(void) ufs_close(*vpp, flag & FMASK, cred);
		return (error);
	}
	ip = VTOI(*vpp);

	/*
	 * Do open protocol for inode type.
	 */
	dev = ip->i_rdev;
	majnum = major(dev);
	minnum = minor(dev);

	switch (ip->i_mode & IFMT) {

	case IFCHR:
		if ((u_int)majnum >= nchrdev)
			return (ENXIO);
		error = (*cdevsw[majnum].d_open)(dev, flag, &minnum);

		/*
		 * Test for new minor device inode allocation
		 */
		if ((error == 0) && (minnum != minor(dev))) {
			register struct inode *nip;

			/*
			 * Allocate new inode with new minor device
			 * Release old inode. Set vpp to point to new one.
			 * This inode will go away when the last reference
			 * to it goes away.
			 * Warning: if you stat this, and try to match it
			 * with a name in the filesystem you will fail,
			 * unless you had previously put names in that match.
			 */
			nip = ialloc(ip, dirpref(ip->i_fs), (int)ip->i_mode);
			if (nip == (struct inode *)0) {
				/*
				 * Must close the device we just opened,
				 * not the original.
				 */
				(void) (*cdevsw[majnum].d_close)
					(makedev(majnum, minnum), flag);
				return (ENXIO);
			}
			imark(nip, IACC|IUPD|ICHG);
			nip->i_mode = ip->i_mode;
			nip->i_vnode.v_type = ip->i_vnode.v_type;
			nip->i_nlink = 0;
			nip->i_uid = ip->i_uid;
			nip->i_gid = ip->i_gid;
			nip->i_vnode.v_rdev = nip->i_rdev =
			    makedev(majnum, minnum);
			irele(ip);
			ip = nip;
			iunlock(ip);
			*vpp = ITOV(ip);
		}
		break;

	case IFBLK:
		if ((u_int)majnum >= nblkdev)
			return (ENXIO);
		error = (*bdevsw[majnum].d_open)(dev, flag);
		break;

	case IFSOCK:
		error = EOPNOTSUPP;
		break;
	
	default:
		error = 0;
		break;
	}
	return (error);
}

/*ARGSUSED*/
int
ufs_close(vp, flag, cred)
	struct vnode *vp;
	int flag;
	struct ucred *cred;
{
	register struct inode *ip;
	register struct mount *mp;
	register int mode;
	struct vnode *dev_vp;
	int (*cfunc)();
	dev_t dev;

	/*
	 * setjmp in case close is interrupted
	 */
	if (setjmp(&u.u_qsave)) {
		return (EINTR);
	}
	ip = VTOI(vp);
	dev = ip->i_rdev;
	mode = ip->i_mode & IFMT;
	switch(mode) {

	case IFCHR:
		cfunc = cdevsw[major(dev)].d_close;
		break;

	case IFBLK:
		/*
		 * don't close device if it is mounted somewhere
		 */
		for (mp = mounttab; mp < &mounttab[NMOUNT]; mp++) {
			if (( mp->m_bufp != NULL) && ( mp->m_dev == dev))
				return (0);
		}
		cfunc = bdevsw[major(dev)].d_close;
		break;

	default:
		return (0);
	}
	if (mode == IFBLK) {
		/*
		 * On last close of a block device (that isn't mounted)
		 * we must invalidate any in core blocks, so that
		 * we can, for instance, change floppy disks.
		 */
		dev_vp = devtovp(dev);
		bflush(dev_vp);
		binval(dev_vp);
		VN_RELE(dev_vp);
	}
	/*
	 * Close the device.
	 */
	(*cfunc)(dev, flag);
	return (0);
}

/*
 * read or write a vnode
 */
/*ARGSUSED*/
int
ufs_rdwr(vp, uiop, rw, ioflag, cred)
	struct vnode *vp;
	struct uio *uiop;
	enum uio_rw rw;
	int ioflag;
	struct ucred *cred;
{
	register struct inode *ip;
	int error;

	ip = VTOI(vp);
	if ((ip->i_mode&IFMT) == IFREG) {
		ILOCK(ip);
		if ((ioflag & IO_APPEND) && (rw == UIO_WRITE)) {
			/*
			 * in append mode start at end of file.
			 */
			uiop->uio_offset = ip->i_size;
		}
		error = rwip(ip, uiop, rw, ioflag);
		IUNLOCK(ip);
	} else {
		error = rwip(ip, uiop, rw, ioflag);
	}
	return (error);
}

int
rwip(ip, uio, rw, ioflag)
	register struct inode *ip;
	register struct uio *uio;
	enum uio_rw rw;
	int ioflag;
{
	dev_t dev;
	struct vnode *devvp;
	struct buf *bp;
	struct fs *fs;
	daddr_t lbn, bn;
	register int n, on, type;
	int size;
	long bsize;
	extern int mem_no;
	int error = 0;

	dev = (dev_t)ip->i_rdev;
	if (rw != UIO_READ && rw != UIO_WRITE)
		panic("rwip");
	if (rw == UIO_READ && uio->uio_resid == 0)
		return (0);
	if ((uio->uio_offset < 0 || (uio->uio_offset + uio->uio_resid) < 0) &&
	    !((ip->i_mode&IFMT) == IFCHR && mem_no == major(dev)))
		return (EINVAL);
	if (rw == UIO_READ)
		imark(ip, IACC);
	type = ip->i_mode&IFMT;
	if (type == IFCHR) {
		if (rw == UIO_READ) {
			error = (*cdevsw[major(dev)].d_read)(dev, uio);
		} else {
			imark(ip, IUPD|ICHG);
			error = (*cdevsw[major(dev)].d_write)(dev, uio);
		}
		return (error);
	}
	if (uio->uio_resid == 0)
		return (0);
	if (rw == UIO_WRITE && type == IFREG &&
	    uio->uio_offset + uio->uio_resid >
	      u.u_rlimit[RLIMIT_FSIZE].rlim_cur) {
		psignal(u.u_procp, SIGXFSZ);
		return (EFBIG);
	}
	if (type != IFBLK) {
		devvp = ip->i_devvp;
		fs = ip->i_fs;
		bsize = fs->fs_bsize;
	} else {
		devvp = devtovp(dev);
		bsize = BLKDEV_IOSIZE;
	}
	u.u_error = 0;
	do {
		lbn = uio->uio_offset / bsize;
		on = uio->uio_offset % bsize;
		n = MIN((unsigned)(bsize - on), uio->uio_resid);
		if (type != IFBLK) {
			if (rw == UIO_READ) {
				int diff = ip->i_size - uio->uio_offset;
				if (diff <= 0)
					return (0);
				if (diff < n)
					n = diff;
			}
			bn =
			    fsbtodb(fs, bmap(ip, lbn,
				 rw == UIO_WRITE ? B_WRITE: B_READ,
				 (int)(on+n), ioflag & IO_SYNC));
			if (u.u_error || rw == UIO_WRITE && (long)bn<0)
				return (u.u_error);
			if (rw == UIO_WRITE &&
			   (uio->uio_offset + n > ip->i_size) &&
			   (type == IFDIR || type == IFREG || type == IFLNK))
				ip->i_size = uio->uio_offset + n;
			size = blksize(fs, ip, lbn);
		} else {
			bn = lbn * (BLKDEV_IOSIZE/DEV_BSIZE);
			rablock = bn + (BLKDEV_IOSIZE/DEV_BSIZE);
			rasize = size = bsize;
		}
		if (rw == UIO_READ) {
			if ((long)bn<0) {
				bp = geteblk(size);
				clrbuf(bp);
			} else if (ip->i_lastr + 1 == lbn)
				bp = breada(devvp, bn, size, rablock,
					rasize);
			else
				bp = bread(devvp, bn, size);
			ip->i_lastr = lbn;
		} else {
			int i, count;
			extern struct cmap *mfind();

			count = howmany(size, DEV_BSIZE);
			for (i = 0; i < count; i += CLBYTES/DEV_BSIZE)
				if (mfind(devvp, (daddr_t)(bn + i)))
					munhash(devvp, (daddr_t)(bn + i));
			if (n == bsize) 
				bp = getblk(devvp, bn, size);
			else
				bp = bread(devvp, bn, size);
		}
		n = MIN(n, bp->b_bcount - bp->b_resid);
		if (bp->b_flags & B_ERROR) {
			error = EIO;
			brelse(bp);
			goto bad;
		}
		u.u_error = uiomove(bp->b_un.b_addr+on, n, rw, uio);
		if (rw == UIO_READ) {
			if (n + on == bsize || uio->uio_offset == ip->i_size)
				bp->b_flags |= B_AGE;
			brelse(bp);
		} else {
			if ((ioflag & IO_SYNC) || (ip->i_mode&IFMT) == IFDIR)
				bwrite(bp);
			else if (n + on == bsize) {
				bp->b_flags |= B_AGE;
				bawrite(bp);
			} else
				bdwrite(bp);
			imark(ip, IUPD|ICHG);
			if (u.u_ruid != 0)
				ip->i_mode &= ~(ISUID|ISGID);
		}
	} while (u.u_error == 0 && uio->uio_resid > 0 && n != 0);
	if ((ioflag & IO_SYNC) && (rw == UIO_WRITE) &&
	    (ip->i_flag & (IUPD|ICHG))) {
		iupdat(ip, 1);
	}
	if (error == 0)				/* XXX */
		error = u.u_error;		/* XXX */
bad:
	if (type == IFBLK)
		VN_RELE(devvp);
	return (error);
}

/*ARGSUSED*/
int
ufs_ioctl(vp, com, data, flag, cred)
	struct vnode *vp;
	int com;
	caddr_t data;
	int flag;
	struct ucred *cred;
{
	register struct inode *ip;

	ip = VTOI(vp);
	if ((ip->i_mode & IFMT) != IFCHR)
		panic("ufs_ioctl");
	return ((*cdevsw[major(ip->i_rdev)].d_ioctl)
			(ip->i_rdev, com, data, flag));
}

/*ARGSUSED*/
int
ufs_select(vp, which, cred)
	struct vnode *vp;
	int which;
	struct ucred *cred;
{
	register struct inode *ip;

	ip = VTOI(vp);
	if ((ip->i_mode & IFMT) != IFCHR)
		panic("ufs_select");
	return ((*cdevsw[major(ip->i_rdev)].d_select)(ip->i_rdev, which));
}

/*ARGSUSED*/
int
ufs_getattr(vp, vap, cred)
	struct vnode *vp;
	register struct vattr *vap;
	struct ucred *cred;
{
	register struct inode *ip;

	ip = VTOI(vp);
	/*
	 * Copy from inode table.
	 */
	vap->va_type = IFTOVT(ip->i_mode);
	vap->va_mode = ip->i_mode;
	vap->va_uid = ip->i_uid;
	vap->va_gid = ip->i_gid;
	vap->va_fsid = ip->i_dev;
	vap->va_nodeid = ip->i_number;
	vap->va_nlink = ip->i_nlink;
	vap->va_size = ip->i_size;
	vap->va_atime = ip->i_atime;
	vap->va_mtime = ip->i_mtime;
	vap->va_ctime = ip->i_ctime;
	vap->va_rdev = ip->i_rdev;
	vap->va_blocks = ip->i_blocks;
	switch(ip->i_mode & IFMT) {

	case IFBLK:
		vap->va_blocksize = BLKDEV_IOSIZE;
		break;

	case IFCHR:
		vap->va_blocksize = MAXBSIZE;
		break;

	default:
		vap->va_blocksize = ip->i_fs->fs_bsize;
		break;
	}
	return (0);
}

int
ufs_setattr(vp, vap, cred)
	register struct vnode *vp;
	register struct vattr *vap;
	struct ucred *cred;
{
	register struct inode *ip;
	int chtime = 0;
	int error = 0;

	/*
	 * cannot set these attributes
	 */
	if ((vap->va_nlink != -1) || (vap->va_blocksize != -1) ||
	    (vap->va_rdev != -1) || (vap->va_blocks != -1) ||
	    (vap->va_fsid != -1) || (vap->va_nodeid != -1) ||
	    ((int)vap->va_type != -1)) {
		return (EINVAL);
	}

	ip = VTOI(vp);
	ilock(ip);
	/*
	 * Change file access modes. Must be owner or su.
	 */
	if (vap->va_mode != (u_short)-1) {
		error = OWNER(cred, ip);
		if (error)
			goto out;
		ip->i_mode &= IFMT;
		ip->i_mode |= vap->va_mode & ~IFMT;
		if (cred->cr_uid != 0) {
			ip->i_mode &= ~ISVTX;
			if (!groupmember(ip->i_gid))
				ip->i_mode &= ~ISGID;
		}
		imark(ip, ICHG);
		if ((ip->i_flag & ITEXT) && ((ip->i_mode & ISVTX) == 0)) {
			xrele(ITOV(ip));
		}
	}
	/*
	 * Change file ownership. Must be su.
	 */
	if ((vap->va_uid != -1) || (vap->va_gid != -1)) {
		if (!suser()) {
			error = EPERM;
			goto out;
		}
		error = chown1(ip, vap->va_uid, vap->va_gid);
		if (error)
			goto out;
	}
	/*
	 * Truncate file. Must have write permission and not be a directory.
	 */
	if (vap->va_size != (u_long)-1) {
		if ((ip->i_mode & IFMT) == IFDIR) {
			error = EISDIR;
			goto out;
		}
		if (iaccess(ip, IWRITE)) {
			error = u.u_error;
			goto out;
		}
		itrunc(ip, vap->va_size);
	}
	/*
	 * Change file access or modified times.
	 */
	if (vap->va_atime.tv_sec != -1) {
		error = OWNER(cred, ip);
		if (error)
			goto out;
		ip->i_atime = vap->va_atime;
		chtime++;
	}
	if (vap->va_mtime.tv_sec != -1) {
		error = OWNER(cred, ip);
		if (error)
			goto out;
		ip->i_mtime = vap->va_mtime;
		chtime++;
	}
	if (chtime) {
		ip->i_flag |= IACC|IUPD|ICHG;
		ip->i_ctime = time;
	}
out:
	iupdat(ip, 1);			/* XXX should be asyn for perf */
	iunlock(ip);
	return (error);
}

/*
 * Perform chown operation on inode ip;
 * inode must be locked prior to call.
 */
chown1(ip, uid, gid)
	register struct inode *ip;
	int uid, gid;
{
#ifdef QUOTA
	register long change;
#endif

	if (uid == -1)
		uid = ip->i_uid;
	if (gid == -1)
		gid = ip->i_gid;
#ifdef QUOTA
	if (ip->i_uid == uid)		/* this just speeds things a little */
		change = 0;
	else
		change = ip->i_blocks;
	(void) chkdq(ip, -change, 1);
	(void) chkiq(VFSTOM(ip->i_vnode.v_vfsp), ip, ip->i_uid, 1);
	dqrele(ip->i_dquot);
#endif
	ip->i_uid = uid;
	ip->i_gid = gid;
	imark(ip, ICHG);
	if (u.u_ruid != 0)
		ip->i_mode &= ~(ISUID|ISGID);
#ifdef QUOTA
	ip->i_dquot = getinoquota(ip);
	(void) chkdq(ip, change, 1);
	(void) chkiq(VFSTOM(ip->i_vnode.v_vfsp), (struct inode *)NULL, uid, 1);
	return (u.u_error);		/* should == 0 ALWAYS !! */
#else
	return (0);
#endif
}

/*ARGSUSED*/
int
ufs_access(vp, mode, cred)
	struct vnode *vp;
	int mode;
	struct ucred *cred;
{
	register struct inode *ip;
	int error;

	ip = VTOI(vp);
	ilock(ip);
	error = iaccess(ip, mode);
	iunlock(ip);
	return (error);
}

/*ARGSUSED*/
int
ufs_readlink(vp, uiop, cred)
	struct vnode *vp;
	struct uio *uiop;
	struct ucred *cred;
{
	register struct inode *ip;
	register int error;

	if (vp->v_type != VLNK)
		return (EINVAL);
	ip = VTOI(vp);
	ilock(ip);
	error = rwip(ip, uiop, UIO_READ, 0);
	iunlock(ip);
	return (error);
}

/*ARGSUSED*/
int
ufs_fsync(vp, cred)
	struct vnode *vp;
	struct ucred *cred;
{
	register struct inode *ip;

	ip = VTOI(vp);
	ilock(ip);
	syncip(ip);
	iunlock(ip);
	return (0);
}

/*ARGSUSED*/
int
ufs_inactive(vp, cred)
	struct vnode *vp;
	struct ucred *cred;
{

	iinactive(VTOI(vp));
	return (0);
}

/*
 * Unix file system operations having to do with directory manipulation.
 */
/*ARGSUSED*/
ufs_lookup(dvp, nm, vpp, cred)
	struct vnode *dvp;
	char *nm;
	struct vnode **vpp;
	struct ucred *cred;
{
	struct inode *ip;
	register int error;

	error = dirlook(VTOI(dvp), nm, &ip);
	if (error == 0) {
		*vpp = ITOV(ip);
		iunlock(ip);
	}
	return (error);
}

ufs_create(dvp, nm, vap, exclusive, mode, vpp, cred)
	struct vnode *dvp;
	char *nm;
	struct vattr *vap;
	enum vcexcl exclusive;
	int mode;
	struct vnode **vpp;
	struct ucred *cred;
{
	register int error;
	struct inode *ip;

	/*
	 * can't create directories. use ufs_mkdir.
	 */
	if (vap->va_type == VDIR)
		return (EISDIR);
	ip = (struct inode *) 0;
	error = direnter(VTOI(dvp), nm, DE_CREATE,
		(struct inode *)0, (struct inode *)0, vap, &ip);
	/*
	 * if file exists and this is a nonexclusive create,
	 * check for not directory and access permissions
	 */
	if (error == EEXIST) {
		if (exclusive == NONEXCL) {
			if (((ip->i_mode & IFMT) == IFDIR) && (mode & IWRITE)) {
				error = EISDIR;
			} else if (mode) {
				error = iaccess(ip, mode);
			} else {
				error = 0;
			}
		}
		if (error) {
			iput(ip);
		}
	} 
	if (error) {
		return (error);
	}
	/*
	 * truncate regular files, if required
	 */
	if (((ip->i_mode & IFMT) == IFREG) && (vap->va_size == 0)) {
		itrunc(ip, (u_long) 0);
	}
	*vpp = ITOV(ip);
	if (vap != (struct vattr *)0) {
		(void) ufs_getattr(*vpp, vap, cred);
	}
	iunlock(ip);
	return (error);
}

/*ARGSUSED*/
ufs_remove(vp, nm, cred)
	struct vnode *vp;
	char *nm;
	struct ucred *cred;
{
	register int error;

	error = dirremove(VTOI(vp), nm, (struct inode *)0, 0);
	return (error);
}

/*
 * link a file or a directory
 * If source is a directory, must be superuser
 */
/*ARGSUSED*/
ufs_link(vp, tdvp, tnm, cred)
	struct vnode *vp;
	struct vnode *tdvp;
	char *tnm;
	struct ucred *cred;
{
	register struct inode *sip;
	register int error;

	sip = VTOI(vp);
	if (((sip->i_mode & IFMT) == IFDIR) && !suser()) {
		return (EPERM);
	}
	error =
	    direnter(VTOI(tdvp), tnm, DE_LINK,
		(struct inode *)0, sip, (struct vattr *)0, (struct inode **)0);
	return (error);
}

/*
 * Rename a file or directory
 * We are given the vnode and entry string of the source and the
 * vnode and entry string of the place we want to move the source to
 * (the target). The essential operation is:
 *	unlink(target);
 *	link(source, target);
 *	unlink(source);
 * but "atomically". Can't do full commit without saving state in the inode
 * on disk, which isn't feasible at this time. Best we can do is always
 * guarantee that the TARGET exists.
 */
/*ARGSUSED*/
ufs_rename(sdvp, snm, tdvp, tnm, cred)
	struct vnode *sdvp;		/* old (source) parent vnode */
	char *snm;			/* old (source) entry name */
	struct vnode *tdvp;		/* new (target) parent vnode */
	char *tnm;			/* new (target) entry name */
	struct ucred *cred;
{
	struct inode *sip;		/* source inode */
	register struct inode *sdp;	/* old (source) parent inode */
	register struct inode *tdp;	/* new (target) parent inode */
	register int error;

	sdp = VTOI(sdvp);
	tdp = VTOI(tdvp);
	/*
	 * make sure we can delete the source entry
	 */
	error = iaccess(sdp, IWRITE);
	if (error) {
		return (error);
	}
	/*
	 * look up inode of file we're supposed to rename.
	 */
	error = dirlook(sdp, snm, &sip);
	if (error) {
		return (error);
	}

	iunlock(sip);			/* unlock inode (it's held) */
	/*
	 * check for renaming '.' or '..' or alias of '.'
	 */
	if ((strcmp(snm, ".") == 0) || (strcmp(snm, "..") == 0) ||
	    (sdp == sip)) {
		error = EINVAL;
		goto out;
	}
	/*
	 * link source to the target
	 */
	error =
	    direnter(tdp, tnm, DE_RENAME,
		sdp, sip, (struct vattr *)0, (struct inode **)0);
	if (error) {
		if (error == ESAME)	/* renaming linked files */
			error = 0;	/* not error just nop */
		goto out;
	}

	/*
	 * Unlink the source
	 * Remove the source entry. Dirremove checks that the entry
	 * still reflects sip, and returns an error if it doesn't.
	 * If the entry has changed just forget about it. 
	 * Release the source inode.
	 */
	error = dirremove(sdp, snm, sip, 0);
	if (error == ENOENT) {
		error = 0;
	} else if (error) {
		goto out;
	}

out:
	irele(sip);
	return (error);
}

/*ARGSUSED*/
ufs_mkdir(dvp, nm, vap, vpp, cred)
	struct vnode *dvp;
	char *nm;
	register struct vattr *vap;
	struct vnode **vpp;
	struct ucred *cred;
{
	struct inode *ip;
	register int error;

	error =
	    direnter(VTOI(dvp), nm, DE_CREATE,
		(struct inode *)0, (struct inode *)0, vap, &ip);
	if (error == 0) {
		*vpp = ITOV(ip);
		iunlock(ip);
	} else if (error == EEXIST) {
		iput(ip);
	}
	return (error);
}

/*ARGSUSED*/
ufs_rmdir(vp, nm, cred)
	struct vnode *vp;
	char *nm;
	struct ucred *cred;
{
	register int error;

	error = dirremove(VTOI(vp), nm, (struct inode *)0, 1);
	return (error);
}

/*ARGSUSED*/
ufs_readdir(vp, uiop, cred)
	struct vnode *vp;
	register struct uio *uiop;
	struct ucred *cred;
{
	register struct iovec *iovp;
	register unsigned count;

	iovp = uiop->uio_iov;
	count = iovp->iov_len;
	if ((uiop->uio_iovcnt != 1) || (count < DIRBLKSIZ) ||
	    (uiop->uio_offset & (DIRBLKSIZ -1)))
		return (EINVAL);
	count &= ~(DIRBLKSIZ - 1);
	uiop->uio_resid -= iovp->iov_len - count;
	iovp->iov_len = count;
	return (rwip(VTOI(vp), uiop, UIO_READ, 0));
}

/*ARGSUSED*/
ufs_symlink(dvp, lnm, vap, tnm, cred)
	struct vnode *dvp;
	char *lnm;
	struct vattr *vap;
	char *tnm;
	struct ucred *cred;
{
	struct inode *ip;
	register int error;

	ip = (struct inode *) 0;
	vap->va_type = VLNK;
	vap->va_rdev = 0;
	error =
	    direnter(VTOI(dvp), lnm, DE_CREATE,
		(struct inode *)0, (struct inode *)0, vap, &ip);
	if (error == 0) {
		error =
		    rdwri(UIO_WRITE, ip,
			tnm, strlen(tnm), 0, UIO_SYSSPACE, (int *)0);
		iput(ip);
	} else if (error == EEXIST) {
		iput(ip);
	}
	return (error);
}

rdwri(rw, ip, base, len, offset, seg, aresid)
	enum uio_rw rw;
	struct inode *ip;
	caddr_t base;
	int len;
	int offset;
	int seg;
	int *aresid;
{
	struct uio auio;
	struct iovec aiov;
	register int error;

	aiov.iov_base = base;
	aiov.iov_len = len;
	auio.uio_iov = &aiov;
	auio.uio_iovcnt = 1;
	auio.uio_offset = offset;
	auio.uio_segflg = seg;
	auio.uio_resid = len;
	error = ufs_rdwr(ITOV(ip), &auio, rw, 0, u.u_cred);
	if (aresid) {
		*aresid = auio.uio_resid;
	} else if (auio.uio_resid) {
		error = EIO;
	}
	return (error);
}

int
ufs_bmap(vp, lbn, vpp, bnp)
	struct vnode *vp;
	daddr_t lbn;
	struct vnode **vpp;
	daddr_t *bnp;
{
	register struct inode *ip;

	ip = VTOI(vp);
	if (vpp)
		*vpp = ip->i_devvp;
	if (bnp)
		*bnp = fsbtodb(ip->i_fs, bmap(ip, lbn, B_READ));
	return (0);
}

/*
 * read a logical block and return it in a buffer
 */
/*ARGSUSED*/
int
ufs_bread(vp, lbn, bpp, sizep)
	struct vnode *vp;
	daddr_t lbn;
	struct buf **bpp;
	long *sizep;
{
	register struct inode *ip;
	register struct buf *bp;
	register daddr_t bn;
	register int size;

	ip = VTOI(vp);
	size = blksize(ip->i_fs, ip, lbn);
	bn = fsbtodb(ip->i_fs, bmap(ip, lbn, B_READ));
	if ((long)bn < 0) {
		bp = geteblk(size);
		clrbuf(bp);
	} else if (ip->i_lastr + 1 == lbn) {
		bp = breada(ip->i_devvp, bn, size, rablock, rasize);
	} else {
		bp = bread(ip->i_devvp, bn, size);
	}
	ip->i_lastr = lbn;
	imark(ip, IACC);
	if (bp->b_flags & B_ERROR) {
		brelse(bp);
		return (EIO);
	} else {
		*bpp = bp;
		return (0);
	}
}

/*
 * release a block returned by ufs_bread
 */
/*ARGSUSED*/
ufs_brelse(vp, bp)
	struct vnode *vp;
	struct buf *bp;
{
	bp->b_flags |= B_AGE;
	bp->b_resid = 0;
	brelse(bp);
}

int
ufs_badop()
{
	panic("ufs_badop");
}
