/*	@(#)ufs_vfsops.c 1.1 86/02/03 SMI; from UCB 4.1 83/05/27	*/
/*	@(#)ufs_vfsops.c	2.2 86/05/14 NFSSRC */

#include "param.h"
#include "systm.h"
#include "user.h"
#include "proc.h"
#include "buf.h"
#include "pathname.h"
#include "vfs.h"
#include "vnode.h"
#include "file.h"
#include "uio.h"
#include "conf.h"

#include "../ufs/fs.h"
#include "../ufs/mount.h"
#include "../ufs/inode.h"
#undef NFS
/*
 * must specify ../h/mount.h so as not to get ufs/mount.h again
 */
#include "../h/mount.h"

/*
 * ufs vfs operations.
 */
extern int ufs_mount();
extern int ufs_unmount();
extern int ufs_root();
extern int ufs_statfs();
extern int ufs_sync();

struct vfsops ufs_vfsops = {
	ufs_mount,
	ufs_unmount,
	ufs_root,
	ufs_statfs,
	ufs_sync,
};

/*
 * this is the default filesystem type.
 * this should be setup by the configurator
 */
extern int ufs_mountroot();
int (*rootfsmount)() = ufs_mountroot;

/*
 * Default device to mount on.
 */
extern dev_t rootdev;

/*
 * Mount table.
 */
struct mount	mounttab[NMOUNT];

/*
 * ufs_mount system call
 */
ufs_mount(vfsp, path, data)
	struct vfs *vfsp;
	char *path;
	caddr_t data;
{
	int error;
	dev_t dev;
	struct vnode *vp;
	struct ufs_args args;

	/*
	 * Get arguments
	 */
	error = copyin(data, (caddr_t)&args, sizeof (struct ufs_args));
	if (error) {
		return (error);
	}
	/*
	 * Get the device to be mounted
	 */
	error =
	    lookupname(args.fspec, UIO_USERSPACE, FOLLOW_LINK,
		(struct vnode **)0, &vp);
	if (error) {
		return (error);
	}
	if (vp->v_type != VBLK) {
		VN_RELE(vp);
		return (ENOTBLK);
	}
	dev = vp->v_rdev;
	VN_RELE(vp);
	if (major(dev) >= nblkdev) {
		return (ENXIO);
	}
	/*
	 * Mount the filesystem.
	 */
	error = mountfs(dev, path, vfsp);
	return (error);
}

/*
 * Called by vfs_mountroot when ufs is going to be mounted as root
 */
ufs_mountroot()
{
	struct vfs *vfsp;
	register struct fs *fsp;
	register int error;

	vfsp = (struct vfs *)kmem_alloc(sizeof (struct vfs));
	VFS_INIT(vfsp, &ufs_vfsops, (caddr_t)0);
	error = mountfs(rootdev, "/", vfsp);
	if (error) {
		kmem_free((caddr_t)vfsp, sizeof (struct vfs));
		return (error);
	}
	error = vfs_add((struct vnode *)0, vfsp, 0);
	if (error) {
		unmount1(vfsp, 0);
		kmem_free((caddr_t)vfsp, sizeof (struct vfs));
		return (error);
	}
	vfs_unlock(vfsp);
	fsp = ((struct mount *)(vfsp->vfs_data))->m_bufp->b_un.b_fs;
	inittodr(fsp->fs_time);
	return (0);
}

int
mountfs(dev, path, vfsp)
	dev_t dev;
	char *path;
	struct vfs *vfsp;
{
	register struct fs *fsp;
	register struct mount *mp = 0;
	register struct buf *bp = 0;
	struct buf *tp = 0;
	struct vnode *dev_vp;
	int error;
	int blks;
	caddr_t space;
	int i;
	int size;
	extern char *strncpy();

	/*
	 * Open block device mounted on.
	 * When bio is fixed for vnodes this can all be vnode operations
	 */
	error =
	    (*bdevsw[major(dev)].d_open)(
		dev, (vfsp->vfs_flag & VFS_RDONLY) ? FREAD : FREAD|FWRITE);
	if (error) {
		/* return (error); */
		return (EIO);
	}
	/*
	 * check for dev already mounted on
	 */
	for (mp = &mounttab[0]; mp < &mounttab[NMOUNT]; mp++) {
		if (mp->m_bufp != 0 && dev == mp->m_dev) {
			return (EBUSY);
		}
	}
	/*
	 * find empty mount table entry
	 */
	for (mp = &mounttab[0]; mp < &mounttab[NMOUNT]; mp++) {
		if (mp->m_bufp == 0)
			goto found;
	}
	return (EBUSY);
found:
	vfsp->vfs_data = (caddr_t)mp;
	mp->m_vfsp = vfsp;
	/*
	 * read in superblock
	 */
	dev_vp = devtovp(dev);
	tp = bread(dev_vp, SBLOCK, SBSIZE);
	if (tp->b_flags & B_ERROR)
		goto out;
	/*
	 * Copy the super block into a buffer in it's native size.
	 */
	mp->m_bufp = tp;	/* just to reserve this slot */
	mp->m_dev = NODEV;
	fsp = tp->b_un.b_fs;
	if (fsp->fs_magic != FS_MAGIC || fsp->fs_bsize > MAXBSIZE)
		goto out;
	bp = geteblk((int)fsp->fs_sbsize);
	mp->m_bufp = bp;
	bcopy((caddr_t)tp->b_un.b_addr, (caddr_t)bp->b_un.b_addr,
	   (u_int)fsp->fs_sbsize);
	brelse(tp);
	tp = 0;
	fsp = bp->b_un.b_fs;
	if (vfsp->vfs_flag & VFS_RDONLY) {
		fsp->fs_ronly = 1;
	} else {
		fsp->fs_fmod = 1;
		fsp->fs_ronly = 0;
	}
	vfsp->vfs_bsize = fsp->fs_bsize;
	/*
	 * Read in cyl group info
	 */
	blks = howmany(fsp->fs_cssize, fsp->fs_fsize);
	space = wmemall(vmemall, (int)fsp->fs_cssize);
	if (space == 0)
		goto out;
	for (i = 0; i < blks; i += fsp->fs_frag) {
		size = fsp->fs_bsize;
		if (i + fsp->fs_frag > blks)
			size = (blks - i) * fsp->fs_fsize;
		tp = bread(dev_vp, (daddr_t)fsbtodb(fsp, fsp->fs_csaddr + i),
		    size);
		if (tp->b_flags&B_ERROR) {
			wmemfree(space, (int)fsp->fs_cssize);
			goto out;
		}
		bcopy((caddr_t)tp->b_un.b_addr, space, (u_int)size);
		fsp->fs_csp[i / fsp->fs_frag] = (struct csum *)space;
		space += size;
		brelse(tp);
		tp = 0;
	}
	mp->m_dev = dev;
	(void) strncpy(fsp->fs_fsmnt, path, sizeof(fsp->fs_fsmnt));
	VN_RELE(dev_vp);
	return (0);
out:
	mp->m_bufp = 0;
	if (bp)
		brelse(bp);
	if (tp)
		brelse(tp);
	VN_RELE(dev_vp);
	return (EBUSY);
}

/*
 * vfs operations
 */

ufs_unmount(vfsp)
	struct vfs *vfsp;
{

	return (unmount1(vfsp, 0));
}

unmount1(vfsp, forcibly)
	register struct vfs *vfsp;
	int forcibly;
{
	dev_t dev;
	register struct mount *mp;
	register struct fs *fs;
	register int stillopen;
	int flag;

	mp = (struct mount *)vfsp->vfs_data;
	dev = mp->m_dev;
#ifdef QUOTA
	if ((stillopen = iflush(dev, mp->m_qinod)) < 0 && !forcibly)
#else
	if ((stillopen = iflush(dev)) < 0 && !forcibly)
#endif
		return (EBUSY);
	if (stillopen < 0)
		return (EBUSY);			/* XXX */
#ifdef QUOTA
	(void)closedq(mp);
	/*
	 * Here we have to iflush again to get rid of the quota inode.
	 * A drag, but it would be ugly to cheat, & this doesn't happen often
	 */
	(void)iflush(dev, (struct inode *)NULL);
#endif
	fs = mp->m_bufp->b_un.b_fs;
	wmemfree((caddr_t)fs->fs_csp[0], (int)fs->fs_cssize);
	flag = !fs->fs_ronly;
	brelse(mp->m_bufp);
	mp->m_bufp = 0;
	mp->m_dev = 0;
	if (!stillopen) {
		register struct vnode *dev_vp;

		(*bdevsw[major(dev)].d_close)(dev, flag);
		dev_vp = devtovp(dev);
		binval(dev_vp);
		VN_RELE(dev_vp);
	}
	return (0);
}

/*
 * find root of ufs
 */
int
ufs_root(vfsp, vpp)
	struct vfs *vfsp;
	struct vnode **vpp;
{
	register struct mount *mp;
	register struct inode *ip;

	mp = (struct mount *)vfsp->vfs_data;
	ip = iget(mp->m_dev, mp->m_bufp->b_un.b_fs, (ino_t)ROOTINO);
	if (ip == (struct inode *)0) {
		return (u.u_error);
	}
	iunlock(ip);
	*vpp = ITOV(ip);
	return (0);
}

/*
 * Get file system statistics.
 */
int
ufs_statfs(vfsp, sbp)
register struct vfs *vfsp;
struct statfs *sbp;
{
	register struct fs *fsp;

	fsp = ((struct mount *)vfsp->vfs_data)->m_bufp->b_un.b_fs;
	if (fsp->fs_magic != FS_MAGIC)
		panic("ufs_statfs");
	sbp->f_bsize = fsp->fs_fsize;
	sbp->f_blocks = fsp->fs_dsize;
	sbp->f_bfree =
	    fsp->fs_cstotal.cs_nbfree * fsp->fs_frag +
		fsp->fs_cstotal.cs_nffree;
	/*
	 * avail = MAX(max_avail - used, 0)
	 */
	sbp->f_bavail =
	    (fsp->fs_dsize * (100 - fsp->fs_minfree) / 100) -
		 (fsp->fs_dsize - sbp->f_bfree);
	/*
	 * inodes
	 */
	sbp->f_files =  fsp->fs_ncg * fsp->fs_ipg;
	sbp->f_ffree = fsp->fs_cstotal.cs_nifree;
	bcopy((caddr_t)fsp->fs_id, (caddr_t)sbp->f_fsid, sizeof (fsid_t));
	return (0);
}

/*
 * Flush any pending I/O.
 */
int
ufs_sync()
{
	update();
	return (0);
}

sbupdate(mp)
	struct mount *mp;
{
	register struct fs *fs = mp->m_bufp->b_un.b_fs;
	register struct buf *bp;
	int blks;
	caddr_t space;
	int i, size;
	register struct vnode *dev_vp;

	dev_vp = devtovp(mp->m_dev);
	bp = getblk(dev_vp, SBLOCK, (int)fs->fs_sbsize);
	bcopy((caddr_t)fs, bp->b_un.b_addr, (u_int)fs->fs_sbsize);
	bwrite(bp);
	blks = howmany(fs->fs_cssize, fs->fs_fsize);
	space = (caddr_t)fs->fs_csp[0];
	for (i = 0; i < blks; i += fs->fs_frag) {
		size = fs->fs_bsize;
		if (i + fs->fs_frag > blks)
			size = (blks - i) * fs->fs_fsize;
		bp = getblk(dev_vp, (daddr_t)fsbtodb(fs, fs->fs_csaddr + i),
		    size);
		bcopy(space, bp->b_un.b_addr, (u_int)size);
		space += size;
		bwrite(bp);
	}
	VN_RELE(dev_vp);
}
