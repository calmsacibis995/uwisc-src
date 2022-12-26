/*	@(#)vfs.c 1.1 86/02/03 SMI	*/
/*      NFSSRC @(#)vfs.c	2.1 86/04/15 */

#include "param.h"
#include "user.h"
#include "uio.h"
#include "file.h"
#include "vfs.h"
#include "socket.h"
#undef NFS
#include "mount.h"
#include "pathname.h"
#include "vnode.h"
#include "../ufs/inode.h"

/*
 * vfs global data
 */
struct vnode *rootdir;			/* pointer to root vnode */

struct vfs *rootvfs;			/* pointer to root vfs. This is */
					/* also the head of the vfs list */

/*
 * System calls
 */

/*
 * mount system call
 */
mount()
{
	register struct a {
		int	type;
		char	*dir;
		int	flags;
		caddr_t	data;
	} *uap = (struct a *) u.u_ap;
	struct pathname pn;
	struct vnode *vp;
	struct vfs *vfsp;
	extern struct vnodeops ufs_vnodeops;

	/*
	 * Must be super user
	 */
	if (!suser())
		return;
	/*
	 * Get vnode to be covered
	 */
	u.u_error =
	    lookupname(uap->dir, UIO_USERSPACE, FOLLOW_LINK,
		(struct vnode **)0, &vp);
	if (u.u_error)
		return;
	dnlc_purge();
	if (vp->v_count != 1 || vp->v_type != VDIR) {
		VN_RELE(vp);
		u.u_error = EBUSY;
		return;
	}
	u.u_error = pn_get(uap->dir, UIO_USERSPACE, &pn);
	if (u.u_error) {
		VN_RELE(vp);
		return;
	}
	if (uap->type > MOUNT_MAXTYPE ||
	    vfssw[uap->type] == (struct vfsops *)0) {
		u.u_error = ENODEV;
		pn_free(&pn);		/* release pathname */
		VN_RELE(vp);
		return;
	}
	vfsp = (struct vfs *)kmem_alloc(sizeof (struct vfs));
	VFS_INIT(vfsp, vfssw[uap->type], (caddr_t)0);

	/*
	 * Mount the filesystem.
	 * Lock covered vnode (XXX this currently only works if it is type ufs)
	 */
	if (vp->v_op == &ufs_vnodeops)
		ilock(VTOI(vp));
	u.u_error = vfs_add(vp, vfsp, uap->flags);
	if (!u.u_error) {
		u.u_error = VFS_MOUNT(vfsp, pn.pn_path, uap->data);
	}
	pn_free(&pn);		/* release pathname */
	if (vp->v_op == &ufs_vnodeops)
		iunlock(VTOI(vp));
	if (!u.u_error) {
		vfs_unlock(vfsp);
	} else {
		vfs_remove(vfsp);
		kmem_free((caddr_t)vfsp, sizeof (struct vfs));
		VN_RELE(vp);
	}
	return;
}


/*
 * Sync system call. sync each vfs
 */
sync()
{
	register struct vfs *vfsp;

	for (vfsp = rootvfs; vfsp != (struct vfs *)0; vfsp = vfsp->vfs_next) {
		VFS_SYNC(vfsp);
	}
}

/*
 * get filesystem statistics
 */
statfs()
{
	struct a {
		char *path;
		struct statfs *buf;
	} *uap = (struct a *) u.u_ap;
	struct vnode *vp;

	u.u_error =
	    lookupname(uap->path, UIO_USERSPACE, FOLLOW_LINK,
		(struct vnode **)0, &vp);
	if (u.u_error)
		return;
	cstatfs(vp->v_vfsp, uap->buf);
	VN_RELE(vp);
}

fstatfs()
{
	struct a {
		int fd;
		struct statfs *buf;
	} *uap = (struct a *) u.u_ap;
	struct file *fp;

	u.u_error = getvnodefp(uap->fd, &fp);
	if (u.u_error == 0)
		cstatfs(((struct vnode *)fp->f_data)->v_vfsp, uap->buf);
}

cstatfs(vfsp, ubuf)
	struct vfs *vfsp;
	struct statfs *ubuf;
{
	struct statfs sb;

	u.u_error = VFS_STATFS(vfsp, &sb);
	if (u.u_error)
		return;
	u.u_error = copyout((caddr_t)&sb, (caddr_t)ubuf, sizeof(sb));
}

/*
 * Unmount system call.
 *
 * Note: unmount takes a path to the vnode mounted on as argument,
 * not special file (as before).
 */
unmount()
{
	struct a {
		char	*pathp;
	} *uap = (struct a *) u.u_ap;
	struct vnode *fsrootvp;
	register struct vnode *coveredvp;
	register struct vfs *vfsp;

	if (!suser()) {
		u.u_error = EPERM;
		return;
	}
	/*
	 * lookup root of fs
	 */
	u.u_error = lookupname(uap->pathp, UIO_USERSPACE, FOLLOW_LINK,
			(struct vnode **)0, &fsrootvp);
	if (u.u_error)
		return;
	/*
	 * make sure this is a root
	 */
	if ((fsrootvp->v_flag & VROOT) == 0) {
		u.u_error = EINVAL;
		VN_RELE(fsrootvp);
		return;
	}
	/*
	 * get vfs and covered vnode
	 */
	vfsp = fsrootvp->v_vfsp;
	VN_RELE(fsrootvp);
	coveredvp = vfsp->vfs_vnodecovered;
	/*
	 * lock vnode to maintain fs status quo during unmount
	 */
	u.u_error = vfs_lock(vfsp);
	if(u.u_error)
		return;

	xumount(vfsp);	/* remove unused sticky files from text table */
	dnlc_purge();	/* remove dnlc entries for this file sys */
	update();

	u.u_error = VFS_UNMOUNT(vfsp);
	if (u.u_error) {
		vfs_unlock(vfsp);
	} else {
		VN_RELE(coveredvp);
		vfs_remove(vfsp);
		kmem_free((caddr_t)vfsp, (u_int)sizeof(*vfsp));
	}
}

/*
 * External routines
 */

/*
 * vfs_mountroot is called by main (init_main.c) to
 * mount the root filesystem.
 */
void
vfs_mountroot()
{
	register int error;
	extern int (*rootfsmount)();	/* pointer to root mounting routine */
					/* set by (auto)configuration */

	/*
	 * Rootfsmount is a pointer to the routine which will mount a specific
	 * filesystem as the root. It is setup by autoconfiguration.
	 * If error panic.
	 */
	error = (*rootfsmount)();
	if (error) {
		panic("rootmount cannot mount root");
	}
	/*
	 * Get vnode for '/'.
	 * Setup rootdir, u.u_rdir and u.u_cdir to point to it.
	 * These are used by lookuppn so that it knows where
	 * to start from '/' or '.'.
	 */
	error = VFS_ROOT(rootvfs, &rootdir);
	if (error)
		panic("rootmount: cannot find root vnode");
	u.u_cdir = rootdir;
	VN_HOLD(u.u_cdir);
	u.u_rdir = NULL;
}

/*
 * vfs_add is called by a specific filesystem's mount routine to add
 * the new vfs into the vfs list and to cover the mounted on vnode.
 * The vfs is also locked so that lookuppn will not venture into the
 * covered vnodes subtree.
 * coveredvp is zero if this is the root.
 */
int
vfs_add(coveredvp, vfsp, mflag)
	register struct vnode *coveredvp;
	register struct vfs *vfsp;
	int mflag;
{
	register int error;

	error = vfs_lock(vfsp);
	if(error)
		return(error);
	if (coveredvp != (struct vnode *)0) {
		/*
		 * Return EBUSY if the covered vp is already mounted on.
		 */
		if (coveredvp->v_vfsmountedhere != (struct vfs *)0) {
			vfs_unlock(vfsp);
			return(EBUSY);
		}
		/*
		 * Put the new vfs on the vfs list after root.
		 * Point the covered vnode at the new vfs so lookuppn
		 * (vfs_lookup.c) can work its way into the new file system.
		 */
		vfsp->vfs_next = rootvfs->vfs_next;
		rootvfs->vfs_next = vfsp;
		coveredvp->v_vfsmountedhere = vfsp;
	} else {
		/*
		 * This is the root of the whole world.
		 */
		rootvfs = vfsp;
		vfsp->vfs_next = (struct vfs *)0;
	}
	vfsp->vfs_vnodecovered = coveredvp;
	if (mflag & M_RDONLY) {
		vfsp->vfs_flag |= VFS_RDONLY;
	} else {
		vfsp->vfs_flag &= ~VFS_RDONLY;
	}
	if (mflag & M_NOSUID) {
		vfsp->vfs_flag |= VFS_NOSUID;
	} else {
		vfsp->vfs_flag &= ~VFS_NOSUID;
	}
	return(0);
}

/*
 * Remove a vfs from the vfs list, and destory pointers to it.
 * Should be called by filesystem implementation after it determines
 * that an unmount is legal but before it destroys the vfs.
 */
void
vfs_remove(vfsp)
register struct vfs *vfsp;
{
	register struct vfs *tvfsp;
	register struct vnode *vp;

	/*
	 * can't unmount root. Should never happen, because fs will be busy.
	 */
	if (vfsp == rootvfs)
		panic("vfs_remove: unmounting root");
	for (tvfsp = rootvfs;
	    tvfsp != (struct vfs *)0; tvfsp = tvfsp->vfs_next) {
		if (tvfsp->vfs_next == vfsp) {
			/*
			 * remove vfs from list, unmount covered vp.
			 */
			tvfsp->vfs_next = vfsp->vfs_next;
			vp = vfsp->vfs_vnodecovered;
			vp->v_vfsmountedhere = (struct vfs *)0;
			/*
			 * release lock and wakeup anybody waiting
			 */
			vfs_unlock(vfsp);
			return;
		}
	}
	/*
	 * can't find vfs to remove
	 */
	panic("vfs_remove: vfs not found");
}

/*
 * Lock a filesystem to prevent access to it while mounting and unmounting.
 * Returns error if already locked.
 * XXX This totally inadequate for unmount right now - srk
 */
int
vfs_lock(vfsp)
	register struct vfs *vfsp;
{
	if (vfsp->vfs_flag & VFS_MLOCK)
		return(EBUSY);
	vfsp->vfs_flag |= VFS_MLOCK;
	return(0);
}

/*
 * Unlock a locked filesystem.
 * Panics if not locked
 */
void
vfs_unlock(vfsp)
	register struct vfs *vfsp;
{
	if ((vfsp->vfs_flag & VFS_MLOCK) == 0)
		panic("vfs_unlock");
	vfsp->vfs_flag &= ~VFS_MLOCK;
	/*
	 * Wake anybody waiting for the lock to clear
	 */
	if (vfsp->vfs_flag & VFS_MWAIT) {
		vfsp->vfs_flag &= ~VFS_MWAIT;
		wakeup((caddr_t)vfsp);
	}
}
