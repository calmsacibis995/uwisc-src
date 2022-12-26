#ifndef lint
static char rcs_id[] =
	{"$Header: nfs_vnodeops.c,v 3.1 86/10/22 15:07:54 tadl Exp $"};
#endif not lint
/*
 * RCS Info
 *	$Locker:  $
 */
/* NFSSRC @(#)nfs_vnodeops.c	2.2 86/05/14 */
/*      @(#)nfs_vnodeops.c 1.1 86/02/03 SMI      */

/*
 * Copyright (c) 1985 by Sun Microsystems, Inc.
 */

#include "../h/param.h"
#include "../h/systm.h"
#include "../h/user.h"
#include "../h/vnode.h"
#include "../h/vfs.h"
#include "../h/file.h"
#include "../h/uio.h"
#include "../h/buf.h"
#include "../h/kernel.h"
#include "../h/cmap.h"
#include "../netinet/in.h"
#include "../h/proc.h"
#include "../rpc/types.h"
#include "../rpc/auth.h"
#include "../rpc/clnt.h"
#include "../rpc/xdr.h"
#include "../nfs/nfs.h"
#include "../nfs/nfs_clnt.h"
#include "../nfs/rnode.h"

#ifdef NFSDEBUG
extern int nfsdebug;
#endif

struct vnode *makenfsnode();
struct vnode *dnlc_lookup();
char *newname();

/*
 * These are the vnode ops routines which implement the vnode interface to
 * the networked file system.  These routines just take their parameters,
 * make them look networkish by putting the right info into interface structs,
 * and then calling the appropriate remote routine(s) to do the work.
 */

int nfs_open();
int nfs_close();
int nfs_rdwr();
int nfs_ioctl();
int nfs_select();
int nfs_getattr();
int nfs_setattr();
int nfs_access();
int nfs_lookup();
int nfs_create();
int nfs_remove();
int nfs_link();
int nfs_rename();
int nfs_mkdir();
int nfs_rmdir();
int nfs_readdir();
int nfs_symlink();
int nfs_readlink();
int nfs_fsync();
int nfs_inactive();
int nfs_bmap();
int nfs_strategy();
int nfs_badop();

struct vnodeops nfs_vnodeops = {
	nfs_open,
	nfs_close,
	nfs_rdwr,
	nfs_ioctl,
	nfs_select,
	nfs_getattr,
	nfs_setattr,
	nfs_access,
	nfs_lookup,
	nfs_create,
	nfs_remove,
	nfs_link,
	nfs_rename,
	nfs_mkdir,
	nfs_rmdir,
	nfs_readdir,
	nfs_symlink,
	nfs_readlink,
	nfs_fsync,
	nfs_inactive,
	nfs_bmap,
	nfs_strategy,
	nfs_badop,
	nfs_badop,
};

/*ARGSUSED*/
int
nfs_open(vpp, flag, cred)
	register struct vnode **vpp;
	int flag;
	struct ucred *cred;
{
	struct vattr va;
	int error;

#ifdef NFSDEBUG
	dprint(nfsdebug, 4, "nfs_open %s %o %d flag %d\n",
	    vtomi(*vpp)->mi_hostname,
	    vtofh(*vpp)->fh_fsid, vtofh(*vpp)->fh_fno,
	    flag);
#endif
	/*
	 * validate cached data by getting the attributes
	 */
	error = nfs_getattr(*vpp, &va, cred);
	if (!error) {
		vtor(*vpp)->r_flags |= ROPEN;
	}
	return (error);
}

/*ARGSUSED*/
int
nfs_close(vp, flag, cred)
	struct vnode *vp;
	int flag;
	struct ucred *cred;
{
	register struct rnode *rp;

#ifdef NFSDEBUG
	dprint(nfsdebug, 4, "nfs_close %s %o %d flag %d\n",
	    vtomi(vp)->mi_hostname,
	    vtofh(vp)->fh_fsid, vtofh(vp)->fh_fno,
	    flag);
#endif
	rp = vtor(vp);
	rp->r_flags &= ~ROPEN;
	/*
	 * If this is a close of a file open for writing or an unlinked
	 * open file or a file that has had an asynchronous write error,
	 * flush synchronously. This allows us to invalidate the file's
	 * buffers if there was a write error or the file was unlinked.
	 * Invalidating the buffers kills their references to the vnode
	 * so that it will free up quickly.
	 */
	if (flag & FWRITE || rp->r_unldvp != NULL || rp->r_error) {
		(void) nfs_fsync(vp, cred);
	}
	if (rp->r_unldvp != NULL || rp->r_error) {
		binval(vp);
	}
	return (flag & FWRITE? rp->r_error: 0);
}

int
nfs_rdwr(vp, uiop, rw, ioflag, cred)
	register struct vnode *vp;
	struct uio *uiop;
	enum uio_rw rw;
	int ioflag;
	struct ucred *cred;
{
	int error = 0;
	struct vattr va;
	struct rnode *rp;

#ifdef NFSDEBUG
	dprint(nfsdebug, 4, "nfs_rdwr: %s %o %d %s %x %d\n",
	    vtomi(vp)->mi_hostname,
	    vtofh(vp)->fh_fsid, vtofh(vp)->fh_fno,
	    rw == UIO_READ ? "READ" : "WRITE",
	    uiop->uio_iov->iov_base, uiop->uio_iov->iov_len);
#endif
	if (vp->v_type != VREG) {
		return (EISDIR);
	}

	if (rw == UIO_WRITE || (rw == UIO_READ && vtor(vp)->r_cred == NULL)) {
		crhold(cred);
		if (vtor(vp)->r_cred) {
			crfree(vtor(vp)->r_cred);
		}
		vtor(vp)->r_cred = cred;
	}

#ifdef notdef
	if (ioflag & IO_UNIT) {
		rlock(rp);
	}
#endif
	if ((ioflag & IO_APPEND) && rw == UIO_WRITE) {
		rp = vtor(vp);
		rlock(rp);
		error = VOP_GETATTR(vp, &va, cred);
		if (!error) {
			uiop->uio_offset = rp->r_size;
		}
	}

	if (!error) {
		error = rwvp(vp, uiop, rw);
	}

	if ((ioflag & IO_APPEND) && rw == UIO_WRITE) {
		runlock(rp);
	}
#ifdef notdef
	if (ioflag & IO_UNIT) {
		runlock(rp);
	}
#endif
#ifdef NFSDEBUG
	dprint(nfsdebug, 5, "nfs_rdwr returning %d\n", error);
#endif
	return (error);
}

int
rwvp(vp, uio, rw)
	register struct vnode *vp;
	register struct uio *uio;
	enum uio_rw rw;
{
	struct buf *bp;
	struct rnode *rp;
	daddr_t bn;
	register int n, on;
	int size;
	int error = 0;
	struct vnode *mapped_vp;
	daddr_t mapped_bn, mapped_rabn;
	int eof = 0;

	if (uio->uio_resid == 0) {
		return (0);
	}
	if (uio->uio_offset < 0 || (uio->uio_offset + uio->uio_resid) < 0) {
		return(EINVAL);
	}
	if (rw == UIO_WRITE && vp->v_type == VREG &&
	    uio->uio_offset + uio->uio_resid >
	      u.u_rlimit[RLIMIT_FSIZE].rlim_cur) {
		psignal(u.u_procp, SIGXFSZ);
		return (EFBIG);
	}
	size = vtoblksz(vp);
	size &= ~(DEV_BSIZE - 1);
	if (size <= 0) {
		panic("rwvp: zero size");
	}
	rp = vtor(vp);
	do {
		bn = uio->uio_offset / size;
		on = uio->uio_offset % size;
		n = MIN((unsigned)(size - on), uio->uio_resid);
		VOP_BMAP(vp, bn, &mapped_vp, &mapped_bn);
		if (rw == UIO_READ) {
			int diff;

			if ((long) bn < 0) {
				bp = geteblk(size);
				clrbuf(bp);
			} else {
				if (incore(mapped_vp, mapped_bn)) {
					struct vattr va;

					/*
					 * get attributes to check whether in
					 * core data is stale
					 */
					(void) nfs_getattr(mapped_vp, &va,
					    u.u_cred);
				}
				if (rp->r_lastr + 1 == bn) {
					VOP_BMAP(vp, bn + 1,
					    &mapped_vp, &mapped_rabn);
					bp = breada(mapped_vp, mapped_bn, size,
						mapped_rabn, size);
				} else {
					bp = bread(mapped_vp, mapped_bn, size);
				}
			}
			rp->r_lastr = bn;
			diff = rp->r_size - uio->uio_offset;
			if (diff <= 0) {
				brelse(bp);
				return(0);
			}
			if (diff < n) {
				n = diff;
				eof = 1;
			}
		} else {
			int i, count;

			count = howmany(size, DEV_BSIZE);
			for (i = 0; i < count; i += CLBYTES/DEV_BSIZE)
				if (mfind(vp, (daddr_t)(bn + i)))
					munhash(vp, (daddr_t)(bn + i));
			if (n == size) {
				bp = getblk(mapped_vp, mapped_bn, size);
			} else {
				bp = bread(mapped_vp, mapped_bn, size);
			}
		}
		if (bp->b_flags & B_ERROR) {
			error = EIO;
			brelse(bp);
			goto bad;
		}
		u.u_error = uiomove(bp->b_un.b_addr+on, n, rw, uio);
		if (rw == UIO_READ) {
			brelse(bp);
		} else {
			/*
			 * r_size is the maximum number of bytes known
			 * to be in the file.
			 * Make sure it is at least as high as the last
			 * byte we just wrote into the buffer.
			 */
			if (rp->r_size < uio->uio_offset) {
				rp->r_size = uio->uio_offset;
			}
			rp->r_flags |= RDIRTY;
			if (n + on == size) {
				bp->b_flags |= B_AGE;
				bawrite(bp);
			} else {
				bdwrite(bp);
			}
		}
	} while (u.u_error == 0 && uio->uio_resid > 0 && !eof);
	if (rw == UIO_WRITE && uio->uio_resid && u.u_error == 0) {
		printf("rwvp: short write. resid %d vp %x bn %d\n",
		    uio->uio_resid, vp, bn);
	}
	if (error == 0)				/* XXX */
		error = u.u_error;		/* XXX */
bad:
	return (error);
}

/*
 * Write to file.
 * Writes to remote server in largest size chunks that the server can
 * handle.  Write is synchronous.
 */
nfswrite(vp, base, offset, count, cred)
	struct vnode *vp;
	caddr_t base;
	int offset;
	int count;
	struct ucred *cred;
{
	int error;
	struct nfswriteargs wa;
	struct nfsattrstat *ns;
	int tsize;

#ifdef NFSDEBUG
	dprint(nfsdebug, 4, "nfswrite %s %o %d offset = %d, count = %d\n",
	    vtomi(vp)->mi_hostname,
	    vtofh(vp)->fh_fsid, vtofh(vp)->fh_fno,
	    offset, count);
#endif
	ns = (struct nfsattrstat *)kmem_alloc((u_int)sizeof(*ns));
	do {
		tsize = min(vtomi(vp)->mi_stsize, count);
		wa.wa_data = base;
		wa.wa_fhandle = *vtofh(vp);
		wa.wa_begoff = offset;
		wa.wa_totcount = tsize;
		wa.wa_count = tsize;
		wa.wa_offset = offset;
		error = rfscall(vtomi(vp), RFS_WRITE, xdr_writeargs, (caddr_t)&wa,
			xdr_attrstat, (caddr_t)ns, cred);
		if (!error) {
			error = geterrno(ns->ns_status);
		}
#ifdef NFSDEBUG
		dprint(nfsdebug, 3, "nfswrite: sent %d of %d, error %d\n",
		    tsize, count, error);
#endif
		count -= tsize;
		base += tsize;
		offset += tsize;
	} while (!error && count);

	if (!error) {
		nfs_attrcache(vp, &ns->ns_attr, NOFLUSH);
	}
	kmem_free((caddr_t)ns, (u_int)sizeof(*ns));
	switch (error) {
	case 0:
	case EDQUOT:
		break;

	case ENOSPC:
		printf("NFS write error: on host %s remote file system full\n",
		   vtomi(vp)->mi_hostname );
		break;

	default:
		printf("NFS write error %d on host %s fs %o file %d\n",
		    error, vtomi(vp)->mi_hostname,
		    vtofh(vp)->fh_fsid, vtofh(vp)->fh_fno);
		break;
	}
#ifdef NFSDEBUG
	dprint(nfsdebug, 5, "nfswrite: returning %d\n", error);
#endif
	return (error);
}

/*
 * Read from a file.
 * Reads data in largest chunks our interface can handle
 */
nfsread(vp, base, offset, count, residp, cred)
	struct vnode *vp;
	caddr_t base;
	int offset;
	int count;
	int *residp;
	struct ucred *cred;
{
	int error;
	struct nfsreadargs ra;
	struct nfsrdresult rr;
	register int tsize;

#ifdef NFSDEBUG
	dprint(nfsdebug, 4, "nfsread %s %o %d offset = %d, totcount = %d\n",
	    vtomi(vp)->mi_hostname,
	    vtofh(vp)->fh_fsid, vtofh(vp)->fh_fno,
	    offset, count);
#endif
	do {
		tsize = min(vtomi(vp)->mi_tsize, count);
		rr.rr_data = base;
		ra.ra_fhandle = *vtofh(vp);
		ra.ra_offset = offset;
		ra.ra_totcount = tsize;
		ra.ra_count = tsize;
		error = rfscall(vtomi(vp), RFS_READ, xdr_readargs, (caddr_t)&ra,
			xdr_rdresult, (caddr_t)&rr, cred);
		if (!error) {
			error = geterrno(rr.rr_status);
		}
#ifdef NFSDEBUG
		dprint(nfsdebug, 3, "nfsread: got %d of %d, error %d\n",
		    tsize, count, error);
#endif
		count -= rr.rr_count;
		base += rr.rr_count;
		offset += rr.rr_count;
	} while (!error && count && rr.rr_count == tsize);

	*residp = count;

	if (!error) {
		nfs_attrcache(vp, &rr.rr_attr, SFLUSH);
	}
#ifdef NFSDEBUG
	dprint(nfsdebug, 5, "nfsread: returning %d, resid %d\n",
		error, *residp);
#endif
	return (error);
}

/*ARGSUSED*/
int
nfs_ioctl(vp, com, data, flag, cred)
	struct vnode *vp;
	int com;
	caddr_t data;
	int flag;
	struct ucred *cred;
{

	return (EOPNOTSUPP);
}

/*ARGSUSED*/
int
nfs_select(vp, which, cred)
	struct vnode *vp;
	int which;
	struct ucred *cred;
{

	return (EOPNOTSUPP);
}

/*
 * Timeout values for attributes for
 * regular files, and for directories.
 */
int nfsac_regtimeo = 3;
int nfsac_dirtimeo = 30;

nfs_attrcache(vp, na, fflag)
	struct vnode *vp;
	struct nfsfattr *na;
	enum staleflush fflag;
{
	register struct rnode *rp;

	rp = vtor(vp);
	/*
	 * check the new modify time against the old modify time
	 * to see if cached data is stale
	 */
	if (na->na_mtime.tv_sec != rp->r_nfsattr.na_mtime.tv_sec ||
	    na->na_mtime.tv_usec != rp->r_nfsattr.na_mtime.tv_usec) {
		/*
		 * The file has changed.
		 * If this was unexpected (fflag == SFLUSH),
		 * flush the delayed write blocks associated with this vnode
		 * from the buffer cache and mark the cached blocks on the
		 * free list as invalid. Also flush the page cache.
		 * If this is a text mark it invalid so that the next pagein
		 * from the file will fail.
		 * If the vnode is a directory, purge the directory name
		 * lookup cache.
		 */
		if (fflag == SFLUSH) {
			if ((vp->v_flag & VTEXT) == 0)
				mpurge(vp);
			binvalfree(vp);
		}
		if (vp->v_flag & VTEXT) {
			xinval(vp);
		}
		if (vp->v_type == VDIR) {
			dnlc_purge();
		}
	}
	rp->r_nfsattr = *na;
	rp->r_nfsattrtime = time;
	if (vp->v_type == VDIR) {
		rp->r_nfsattrtime.tv_sec += nfsac_dirtimeo;
	} else {
		rp->r_nfsattrtime.tv_sec += nfsac_regtimeo;
	}
}

int
nfs_getattr(vp, vap, cred)
	struct vnode *vp;
	struct vattr *vap;
	struct ucred *cred;
{
	int error;
	struct nfsattrstat *ns;
	struct rnode *rp;

#ifdef NFSDEBUG
	dprint(nfsdebug, 4, "nfs_getattr %s %d %o\n",
	    vtomi(vp)->mi_hostname,
	    vtofh(vp)->fh_fsid, vtofh(vp)->fh_fno);
#endif
	(void) nfs_fsync(vp, cred);    /* sync blocks so mod time is right */
	rp = vtor(vp);
	if (timercmp(&time, &rp->r_nfsattrtime, <)) {
		/*
		 * Use cached attributes.
		 */
		rp = vtor(vp);
		nattr_to_vattr(&rp->r_nfsattr, vap);
		vap->va_fsid = 0xff00 | vtomi(vp)->mi_mntno;
		if (rp->r_size < vap->va_size) {
			rp->r_size = vap->va_size;
		} else if (vap->va_size < rp->r_size) {
			vap->va_size = rp->r_size;
		}
		return (0);
	}
	ns = (struct nfsattrstat *)kmem_alloc((u_int)sizeof(*ns));
	error = rfscall(vtomi(vp), RFS_GETATTR, xdr_fhandle, (caddr_t)vtofh(vp),
	    xdr_attrstat, (caddr_t)ns, cred);
	if (!error) {
		error = geterrno(ns->ns_status);
		if (!error) {
			nattr_to_vattr(&ns->ns_attr, vap);
			/*
			 * this is a kludge to make programs that use dev from
			 * stat to tell file systems apart happy.  we kludge up
			 * an dev from the mount number and an arbitrary major
			 * number 255.
			*/
			vap->va_fsid = 0xff00 | vtomi(vp)->mi_mntno;
			if (rp->r_size < vap->va_size) {
				rp->r_size = vap->va_size;
			} else if (vap->va_size < rp->r_size) {
				vap->va_size = rp->r_size;
			}
			nfs_attrcache(vp, &ns->ns_attr, SFLUSH);
		}
	}
	kmem_free((caddr_t)ns, (u_int)sizeof(*ns));
#ifdef NFSDEBUG
	dprint(nfsdebug, 5, "nfs_getattr: returns %d\n", error);
#endif
	return (error);
}

int
nfs_setattr(vp, vap, cred)
	register struct vnode *vp;
	register struct vattr *vap;
	struct ucred *cred;
{
	int error;
	struct nfssaargs args;
	struct nfsattrstat *ns;
	extern int (*caller())();

#ifdef NFSDEBUG
	dprint(nfsdebug, 4, "nfs_setattr %s %o %d\n",
	    vtomi(vp)->mi_hostname,
	    vtofh(vp)->fh_fsid, vtofh(vp)->fh_fno);
#endif
	ns = (struct nfsattrstat *)kmem_alloc((u_int)sizeof(*ns));
	if ((vap->va_nlink != -1) || (vap->va_blocksize != -1) ||
	    (vap->va_rdev != -1) || (vap->va_blocks != -1) ||
	    (vap->va_ctime.tv_sec != -1) || (vap->va_ctime.tv_usec != -1)) {
		error = EINVAL;
	} else {
		(void) nfs_fsync(vp, cred);
		if (vap->va_size != -1) {
			(vtor(vp))->r_size = vap->va_size;
		}
		vattr_to_sattr(vap, &args.saa_sa);
		args.saa_fh = *vtofh(vp);
		error = rfscall(vtomi(vp), RFS_SETATTR, xdr_saargs,
		    (caddr_t)&args, xdr_attrstat, (caddr_t)ns, cred);
		if (!error) {
			error = geterrno(ns->ns_status);
			if (!error) {
				nfs_attrcache(vp, &ns->ns_attr, SFLUSH);
			}
		}
	}
	kmem_free((caddr_t)ns, (u_int)sizeof(*ns));
#ifdef NFSDEBUG
	dprint(nfsdebug, 5, "nfs_setattr: returning %d\n", error);
#endif
	return (error);
}

int
nfs_access(vp, mode, cred)
	struct vnode *vp;
	int mode;
	struct ucred *cred;
{
	struct vattr va;
	gid_t *gp;

#ifdef NFSDEBUG
	dprint(nfsdebug, 4, "nfs_access %s %o %d mode %d uid %d\n",
	    vtomi(vp)->mi_hostname,
	    vtofh(vp)->fh_fsid, vtofh(vp)->fh_fno,
	    mode, cred->cr_uid);
#endif
	u.u_error = nfs_getattr(vp, &va, cred);
	if (u.u_error) {
		return (u.u_error);
	}
	/*
	 * If you're the super-user,
	 * you always get access.
	 */
	if (cred->cr_uid == 0)
		return (0);
	/*
	 * Access check is based on only
	 * one of owner, group, public.
	 * If not owner, then check group.
	 * If not a member of the group, then
	 * check public access.
	 */
	if (cred->cr_uid != va.va_uid) {
		mode >>= 3;
		if (cred->cr_gid == va.va_gid)
			goto found;
		gp = cred->cr_groups;
		for (; gp < &cred->cr_groups[NGROUPS] && *gp != NOGROUP; gp++)
			if (va.va_gid == *gp)
				goto found;
		mode >>= 3;
	}
found:
	if ((va.va_mode & mode) == mode) {
		return (0);
	}
#ifdef NFSDEBUG
	dprint(nfsdebug, 5, "nfs_access: returning %d\n", u.u_error);
#endif
	u.u_error = EACCES;
	return (EACCES);
}

int
nfs_readlink(vp, uiop, cred)
	struct vnode *vp;
	struct uio *uiop;
	struct ucred *cred;
{
	int error;
	struct nfsrdlnres rl;

#ifdef NFSDEBUG
	dprint(nfsdebug, 4, "nfs_readlink %s %o %d\n",
	    vtomi(vp)->mi_hostname,
	    vtofh(vp)->fh_fsid, vtofh(vp)->fh_fno);
#endif
	if(vp->v_type != VLNK)
		return (ENXIO);
	rl.rl_data = (char *)kmem_alloc((u_int)NFS_MAXPATHLEN);
	error =
	    rfscall(vtomi(vp), RFS_READLINK, xdr_fhandle, (caddr_t)vtofh(vp),
	      xdr_rdlnres, (caddr_t)&rl, cred);
	if (!error) {
		error = geterrno(rl.rl_status);
		if (!error) {
			error = uiomove(rl.rl_data, (int)rl.rl_count,
			    UIO_READ, uiop);
		}
	}
	kmem_free((caddr_t)rl.rl_data, (u_int)NFS_MAXPATHLEN);
#ifdef NFSDEBUG
	dprint(nfsdebug, 5, "nfs_readlink: returning %d\n", error);
#endif
	return (error);
}

/*ARGSUSED*/
int
nfs_fsync(vp, cred)
	struct vnode *vp;
	struct ucred *cred;
{
	register struct rnode *rp;
	register int offset, blksize;

#ifdef NFSDEBUG
	dprint(nfsdebug, 4, "nfs_fsync %s %o %d\n",
	    vtomi(vp)->mi_hostname,
	    vtofh(vp)->fh_fsid, vtofh(vp)->fh_fno);
#endif
	rp = vtor(vp);
	if (rp->r_flags & RDIRTY) {
		rp->r_flags &= ~RDIRTY;
		bflush(vp);	/* start delayed writes */
		blksize = vtoblksz(vp);
		for (offset = 0; offset < rp->r_size; offset += blksize) {
			blkflush(
			    vp, (daddr_t)(offset / DEV_BSIZE), (long)blksize);
		}
	}
	return (rp->r_error);
}

/*
 * Weirdness: if the file was removed while it was open it got
 * renamed (by nfs_remove) instead.  Here we remove the renamed
 * file.
 */
/*ARGSUSED*/
int
nfs_inactive(vp, cred)
	struct vnode *vp;
	struct ucred *cred;
{
	int error;
	struct nfsdiropargs da;
	enum nfsstat status;

#ifdef NFSDEBUG
	dprint(nfsdebug, 4, "nfs_inactive %s %o %d\n",
	    vtomi(vp)->mi_hostname,
	    vtofh(vp)->fh_fsid, vtofh(vp)->fh_fno);
#endif
	runsave(vtor(vp));
	if (vtor(vp)->r_unlname != NULL) {
		setdiropargs(&da, vtor(vp)->r_unlname, vtor(vp)->r_unldvp);
		error = rfscall( vtomi(vtor(vp)->r_unldvp), RFS_REMOVE,
		    xdr_diropargs, (caddr_t)&da,
		    xdr_enum, (caddr_t)&status, vtor(vp)->r_unlcred);
		if (!error) {
			error = geterrno(status);
		}
		VN_RELE(vtor(vp)->r_unldvp);
		kmem_free((caddr_t)vtor(vp)->r_unlname, (u_int)NFS_MAXNAMLEN);
		crfree(vtor(vp)->r_unlcred);
	}
	((struct mntinfo *)vp->v_vfsp->vfs_data)->mi_refct--;
	if (vtor(vp)->r_cred) {
		crfree(vtor(vp)->r_cred);
	}
	kmem_free((caddr_t)vtor(vp), (u_int)sizeof(struct rnode));
#ifdef NFSDEBUG
	dprint(nfsdebug, 5, "nfs_inactive done\n");
#endif
	return (0);
}

/*
 * Remote file system operations having to do with directory manipulation.
 */

nfs_lookup(dvp, nm, vpp, cred)
	struct vnode *dvp;
	char *nm;
	struct vnode **vpp;
	struct ucred *cred;
{
	int error;
	struct nfsdiropargs da;
	struct  nfsdiropres *dr;
	struct vattr va;

#ifdef NFSDEBUG
	dprint(nfsdebug, 4, "nfs_lookup %s %o %d '%s'\n",
	    vtomi(dvp)->mi_hostname,
	    vtofh(dvp)->fh_fsid, vtofh(dvp)->fh_fno,
	    nm);
#endif
	/*
	 * Before checking dnlc, call getattr to be
	 * sure directory hasn't changed.  getattr
	 * will purge dnlc if a change has occurred.
	 */
	if (error = nfs_getattr(dvp, &va, cred)) {
		return (error);
	}
	*vpp = (struct vnode *) dnlc_lookup(dvp, nm, cred);
	if (*vpp) {
		VN_HOLD(*vpp);
		return (0);
	}
	dr = (struct  nfsdiropres *)kmem_alloc((u_int)sizeof(*dr));
	setdiropargs(&da, nm, dvp);
	error = rfscall(vtomi(dvp), RFS_LOOKUP, xdr_diropargs, (caddr_t)&da,
	    xdr_diropres, (caddr_t)dr, cred);
	if (!error) {
		error = geterrno(dr->dr_status);
	}
	if (!error) {
		*vpp = makenfsnode(&dr->dr_fhandle, &dr->dr_attr, dvp->v_vfsp);
		if ((enum vtype) dr->dr_attr.na_type == VDIR) {
			dnlc_enter(dvp, nm, *vpp, cred);
		}
	} else {
		*vpp = (struct vnode *)0;
	}
	kmem_free((caddr_t)dr, (u_int)sizeof(*dr));
#ifdef NFSDEBUG
	dprint(nfsdebug, 5, "nfs_lookup returning %d vp = %x\n", error, *vpp);
#endif
	return (error);
}

/*ARGSUSED*/
nfs_create(dvp, nm, va, exclusive, mode, vpp, cred)
	struct vnode *dvp;
	char *nm;
	struct vattr *va;
	enum vcexcl exclusive;
	int mode;
	struct vnode **vpp;
	struct ucred *cred;
{
	int error;
	struct nfscreatargs args;
	struct  nfsdiropres *dr;

#ifdef NFSDEBUG
	dprint(nfsdebug, 4, "nfs_create %s %o %d '%s' excl=%d, mode=%o\n",
	    vtomi(dvp)->mi_hostname,
	    vtofh(dvp)->fh_fsid, vtofh(dvp)->fh_fno,
	    nm, exclusive, mode);
#endif
	if (exclusive == EXCL) {
		/*
		 * This is buggy: there is a race between the lookup and the
		 * create.  We should send the exclusive flag over the wire.
		 */
		error = nfs_lookup(dvp, nm, vpp, cred);
		if (!error) {
			VN_RELE(*vpp);
			return (EEXIST);
		}
	}
	*vpp = (struct vnode *)0;

	/*
	 * Until the protocol change, disallow mknod on remote filesystems.
	 */
	if (va->va_type != VREG)
		return (EINVAL);

	dr = (struct  nfsdiropres *)kmem_alloc((u_int)sizeof(*dr));
	setdiropargs(&args.ca_da, nm, dvp);
	vattr_to_sattr(va, &args.ca_sa);
	error = rfscall(vtomi(dvp), RFS_CREATE, xdr_creatargs, (caddr_t)&args,
	    xdr_diropres, (caddr_t)dr, cred);
	if (!error) {
		error = geterrno(dr->dr_status);
		if (!error) {
			*vpp = makenfsnode(
			    &dr->dr_fhandle, &dr->dr_attr, dvp->v_vfsp);
			(vtor(*vpp))->r_size = 0;
			if (va != (struct vattr *)0) {
				nattr_to_vattr(&dr->dr_attr, va);
			}
		}
	}
	kmem_free((caddr_t)dr, (u_int)sizeof(*dr));
#ifdef NFSDEBUG
	dprint(nfsdebug, 5, "nfs_create returning %d\n", error);
#endif
	return (error);
}

/*
 * Weirdness: if the vnode to be removed is open
 * we rename it instead of removing it and nfs_inactive
 * will remove the new name.
 */
nfs_remove(dvp, nm, cred)
	struct vnode *dvp;
	char *nm;
	struct ucred *cred;
{
	int error;
	struct nfsdiropargs da;
	enum nfsstat status;
	struct vnode *vp;
	char *tmpname;

#ifdef NFSDEBUG
	dprint(nfsdebug, 4, "nfs_remove %s %o %d '%s'\n",
	    vtomi(dvp)->mi_hostname,
	    vtofh(dvp)->fh_fsid, vtofh(dvp)->fh_fno,
	    nm);
#endif
	status = NFS_OK;
	error = nfs_lookup(dvp, nm, &vp, cred);
	if (error == 0) {
		if ((vtor(vp)->r_flags & ROPEN) && vtor(vp)->r_unlname == NULL) {
			tmpname = newname(nm);
			error = nfs_rename(dvp, nm, dvp, tmpname, cred);
			if (error) {
				kmem_free((caddr_t)tmpname,
				    (u_int)NFS_MAXNAMLEN);
			} else {
				VN_HOLD(dvp);
				vtor(vp)->r_unldvp = dvp;
				vtor(vp)->r_unlname = tmpname;
				if (vtor(vp)->r_unlcred != NULL) {
					crfree(vtor(vp)->r_unlcred);
				}
				crhold(cred);
				vtor(vp)->r_unlcred = cred;
			}
		} else {
			setdiropargs(&da, nm, dvp);
			error = rfscall(
			    vtomi(dvp), RFS_REMOVE, xdr_diropargs, (caddr_t)&da,
			    xdr_enum, (caddr_t)&status, cred);
		}
		bflush(vp);
		VN_RELE(vp);
	}
	if (!error) {
		error = geterrno(status);
	}
#ifdef NFSDEBUG
	dprint(nfsdebug, 5, "nfs_remove: returning %d\n", error);
#endif
	return (error);
}

nfs_link(vp, tdvp, tnm, cred)
	struct vnode *vp;
	struct vnode *tdvp;
	char *tnm;
	struct ucred *cred;
{
	int error;
	struct nfslinkargs args;
	enum nfsstat status;

#ifdef NFSDEBUG
	dprint(nfsdebug, 4, "nfs_link from %s %o %d to %s %o %d '%s'\n",
	    vtomi(vp)->mi_hostname,
	    vtofh(vp)->fh_fsid, vtofh(vp)->fh_fno,
	    vtomi(tdvp)->mi_hostname,
	    vtofh(tdvp)->fh_fsid, vtofh(tdvp)->fh_fno,
	    tnm);
#endif
	args.la_from = *vtofh(vp);
	setdiropargs(&args.la_to, tnm, tdvp);
	error = rfscall(vtomi(vp), RFS_LINK, xdr_linkargs, (caddr_t)&args,
	    xdr_enum, (caddr_t)&status, cred);
	if (!error) {
		error = geterrno(status);
	}
#ifdef NFSDEBUG
	dprint(nfsdebug, 5, "nfs_link returning %d\n", error);
#endif
	return (error);
}

nfs_rename(odvp, onm, ndvp, nnm, cred)
	struct vnode *odvp;
	char *onm;
	struct vnode *ndvp;
	char *nnm;
	struct ucred *cred;
{
	int error;
	enum nfsstat status;
	struct nfsrnmargs args;

#ifdef NFSDEBUG
	dprint(nfsdebug, 4, "nfs_rename from %s %o %d '%s' to %s %o %d '%s'\n",
	    vtomi(odvp)->mi_hostname,
	    vtofh(odvp)->fh_fsid, vtofh(odvp)->fh_fno, onm,
	    vtomi(ndvp)->mi_hostname,
	    vtofh(ndvp)->fh_fsid, vtofh(ndvp)->fh_fno, nnm);
#endif
	if (!strcmp(onm, ".") || !strcmp(onm, "..") || !strcmp(nnm, ".")
	    || !strcmp (nnm, "..")) {
		error = EINVAL;
	} else {
		dnlc_purge();
		setdiropargs(&args.rna_from, onm, odvp);
		setdiropargs(&args.rna_to, nnm, ndvp);
		error = rfscall(vtomi(odvp), RFS_RENAME,
		    xdr_rnmargs, (caddr_t)&args,
		    xdr_enum, (caddr_t)&status, cred);
		if (!error) {
			error = geterrno(status);
		}
	}
#ifdef NFSDEBUG
	dprint(nfsdebug, 5, "nfs_rename returning %d\n", error);
#endif
	return (error);
}

nfs_mkdir(dvp, nm, va, vpp, cred)
	struct vnode *dvp;
	char *nm;
	register struct vattr *va;
	struct vnode **vpp;
	struct ucred *cred;
{
	int error;
	struct nfscreatargs args;
	struct  nfsdiropres *dr;

#ifdef NFSDEBUG
	dprint(nfsdebug, 4, "nfs_mkdir %s %o %d '%s'\n",
	    vtomi(dvp)->mi_hostname,
	    vtofh(dvp)->fh_fsid, vtofh(dvp)->fh_fno, nm);
#endif
	dr = (struct  nfsdiropres *)kmem_alloc((u_int)sizeof(*dr));
	setdiropargs(&args.ca_da, nm, dvp);
	vattr_to_sattr(va, &args.ca_sa);
	error = rfscall(vtomi(dvp), RFS_MKDIR, xdr_creatargs, (caddr_t)&args,
	    xdr_diropres, (caddr_t)dr, cred);
	if (!error) {
		error = geterrno(dr->dr_status);
	}
	if (!error) {
		*vpp = makenfsnode(&dr->dr_fhandle, &dr->dr_attr, dvp->v_vfsp);
	} else {
		*vpp = (struct vnode *)0;
	}
	kmem_free((caddr_t)dr, (u_int)sizeof(*dr));
#ifdef NFSDEBUG
	dprint(nfsdebug, 5, "nfs_mkdir returning %d\n", error);
#endif
	return (error);
}

nfs_rmdir(dvp, nm, cred)
	struct vnode *dvp;
	char *nm;
	struct ucred *cred;
{
	int error;
	enum nfsstat status;
	struct nfsdiropargs da;

#ifdef NFSDEBUG
	dprint(nfsdebug, 4, "nfs_rmdir %s %o %d '%s'\n",
	    vtomi(dvp)->mi_hostname,
	    vtofh(dvp)->fh_fsid, vtofh(dvp)->fh_fno, nm);
#endif
	setdiropargs(&da, nm, dvp);
	dnlc_purge();
	error = rfscall(vtomi(dvp), RFS_RMDIR, xdr_diropargs, (caddr_t)&da,
	    xdr_enum, (caddr_t)&status, cred);
	if (!error) {
		error = geterrno(status);
	}
#ifdef NFSDEBUG
	dprint(nfsdebug, 5, "nfs_rmdir returning %d\n", error);
#endif
	return (error);
}

nfs_symlink(dvp, lnm, tva, tnm, cred)
	struct vnode *dvp;
	char *lnm;
	struct vattr *tva;
	char *tnm;
	struct ucred *cred;
{
	int error;
	struct nfsslargs args;
	enum nfsstat status;

#ifdef NFSDEBUG
	dprint(nfsdebug, 4, "nfs_symlink %s %o %d '%s' to '%s'\n",
	    vtomi(dvp)->mi_hostname,
	    vtofh(dvp)->fh_fsid, vtofh(dvp)->fh_fno, lnm, tnm);
#endif
	setdiropargs(&args.sla_from, lnm, dvp);
	vattr_to_sattr(tva, &args.sla_sa);
	args.sla_tnm = tnm;
	error = rfscall(vtomi(dvp), RFS_SYMLINK, xdr_slargs, (caddr_t)&args,
	    xdr_enum, (caddr_t)&status, cred);
	if (!error) {
		error = geterrno(status);
	}
#ifdef NFSDEBUG
	dprint(nfsdebug, 5, "nfs_sysmlink: returning %d\n", error);
#endif
	return (error);
}

/*
 * Read directory entries.
 * There are some weird things to look out for here.  The uio_offset
 * field is either 0 or it is the offset returned from a previous
 * readdir.  It is an opaque value used by the server to find the
 * correct directory block to read.  The byte count must be at least
 * vtoblksz(vp) bytes.  The count field is the number of blocks to
 * read on the server.  This is advisory only, the server may return
 * only one block's worth of entries.  Entries may be compressed on
 * the server.
 */
nfs_readdir(vp, uiop, cred)
	struct vnode *vp;
	register struct uio *uiop;
	struct ucred *cred;
{
	int error = 0;
	struct iovec *iovp;
	unsigned count;
	struct nfsrddirargs rda;
	struct nfsrddirres  rd;
	struct rnode *rp;

	rp = vtor(vp);
	if ((rp->r_flags & REOF) && (rp->r_size == (u_long)uiop->uio_offset)) {
		return (0);
	}
	iovp = uiop->uio_iov;
	count = iovp->iov_len;
#ifdef NFSDEBUG
	dprint(nfsdebug, 4,
	    "nfs_readdir %s %o %d count %d offset %d\n",
	    vtomi(vp)->mi_hostname,
	    vtofh(vp)->fh_fsid, vtofh(vp)->fh_fno, count, uiop->uio_offset);
#endif
	/*
	 * XXX We should do some kind of test for count >= DEV_BSIZE
	 */
	if (uiop->uio_iovcnt != 1) {
		return (EINVAL);
	}
	count = MIN(count, vtomi(vp)->mi_tsize);
	rda.rda_count = count;
	rda.rda_offset = uiop->uio_offset;
	rda.rda_fh = *vtofh(vp);
	rd.rd_size = count;
	rd.rd_entries = (struct direct *)kmem_alloc((u_int)count);

	error = rfscall(vtomi(vp), RFS_READDIR, xdr_rddirargs, (caddr_t)&rda,
	    xdr_getrddirres, (caddr_t)&rd, cred);
	if (!error) {
		error = geterrno(rd.rd_status);
	}
	if (!error) {
		/*
		 * move dir entries to user land
		 */
		if (rd.rd_size) {
			error = uiomove((caddr_t)rd.rd_entries,
			    (int)rd.rd_size, UIO_READ, uiop);
			rda.rda_offset = rd.rd_offset;
			uiop->uio_offset = rd.rd_offset;
		}
		if (rd.rd_eof) {
			rp->r_flags |= REOF;
			rp->r_size = uiop->uio_offset;
		}
	}
	kmem_free((caddr_t)rd.rd_entries, (u_int)count);
#ifdef NFSDEBUG
	dprint(nfsdebug, 5, "nfs_readdir: returning %d resid %d, offset %d\n",
	    error, uiop->uio_resid, uiop->uio_offset);
#endif
	return (error);
}

/*
 * Convert from file system blocks to device blocks
 */
int
nfs_bmap(vp, bn, vpp, bnp)
	struct vnode *vp;	/* file's vnode */
	daddr_t bn;		/* fs block number */
	struct vnode **vpp;	/* RETURN vp of device */
	daddr_t *bnp;		/* RETURN device block number */
{
	int bsize;		/* server's block size in bytes */

#ifdef NFSDEBUG
	dprint(nfsdebug, 4, "nfs_bmap %s %o %d blk %d\n",
	    vtomi(vp)->mi_hostname,
	    vtofh(vp)->fh_fsid, vtofh(vp)->fh_fno, bn);
#endif
	if (vpp)
		*vpp = vp;
	if (bnp) {
		bsize = vtoblksz(vp);
		*bnp = (bn * bsize) / DEV_BSIZE;
	}
	return (0);
}

struct buf *async_bufhead;
int async_daemon_count;

#ifdef vax
#include "../h/vm.h"
#include "../h/map.h"
#include "../machine/pte.h"
#endif

int
nfs_strategy(bp)
	register struct buf *bp;
{
	register struct buf *bp1;

#ifdef NFSDEBUG
	dprint(nfsdebug, 4, "nfs_strategy bp %x\n", bp);
#endif
	if (bp->b_flags & (B_DIRTY|B_UAREA|B_PAGET))
		panic("nfs_strategy: swapping to nfs");
	/*
	 * If there was an asynchronous write error on this rnode
	 * then we just return the old error code. This continues
	 * until the rnode goes away (zero ref count). We do this because
	 * there can be many procs writing this rnode.
	 */
	if (vtor(bp->b_vp)->r_error) {
		bp->b_error = vtor(bp->b_vp)->r_error;
		bp->b_flags |= B_ERROR;
		iodone(bp);
		return;
	}
#ifdef vax
	if (bp->b_flags & B_PHYS) {
		register int npte;
		register int n;
		register long a;
		register struct pte *pte, *kpte;
		caddr_t va;
		int o;
		caddr_t saddr;

		/*
		 * Buffer's data is in userland, or in some other
		 * currently inaccessable place. We get a hunk of
		 * kernel address space and map it in.
		 */
		o = (int)bp->b_un.b_addr & PGOFSET;
		npte = btoc(bp->b_bcount + o);
		while ((a = rmalloc(kernelmap, (long)clrnd(npte))) == NULL) {
			kmapwnt++;
			sleep((caddr_t)kernelmap, PSWP+4);
		}
		kpte = &Usrptmap[a];
		pte = vtopte(bp->b_proc, btop(bp->b_un.b_addr));
		for (n = npte; n--; kpte++, pte++)
			*(int *)kpte = PG_NOACC | (*(int *)pte & PG_PFNUM);
		va = (caddr_t)kmxtob(a);
		vmaccess(&Usrptmap[a], va, npte);
		saddr = bp->b_un.b_addr;
		bp->b_un.b_addr = va + o;
		/*
		 * do the io
		 */
		do_bio(bp);
		/*
		 * Release kernel maps
		 */
		bp->b_un.b_addr = saddr;
		kpte = &Usrptmap[a];
		for (n = npte; n-- ; kpte++)
			*(int *)kpte = PG_NOACC;
		rmfree(kernelmap, (long)clrnd(npte), a);
	} else
#endif
	if (async_daemon_count && bp->b_flags & B_ASYNC) {
		if (async_bufhead) {
			bp1 = async_bufhead;
			while (bp1->b_actf) {
				bp1 = bp1->b_actf;
			}
			bp1->b_actf = bp;
		} else {
			async_bufhead = bp;
		}
		bp->b_actf = NULL;
		wakeup((caddr_t) &async_bufhead);
	} else {
		do_bio(bp);
	}
}

async_daemon()
{
	register struct buf *bp;

	/*
	 * First release resoruces.
	 */
	if ((u.u_procp->p_flag & SVFORK) == 0) {
		vrelvm();
	}
	if (setjmp(&u.u_qsave)) {
		async_daemon_count--;
		exit(0);
	}
	for (;;) {
		async_daemon_count++;
		while (async_bufhead == NULL) {
			sleep((caddr_t)&async_bufhead, PZERO + 1);
		}
		async_daemon_count--;
		bp = async_bufhead;
		async_bufhead = bp->b_actf;
		do_bio(bp);
	}
}

do_bio(bp)
	register struct buf *bp;
{
	register struct rnode *rp = vtor(bp->b_vp);
	int count, resid;

#ifdef NFSDEBUG
	dprint(nfsdebug, 4,
	    "do_bio: addr %x, blk %d, offset %d, size %d, B_READ %d\n",
	    bp->b_un.b_addr, bp->b_blkno, bp->b_blkno * DEV_BSIZE,
	    bp->b_bcount, bp->b_flags & B_READ);
#endif
	if ((bp->b_flags & B_READ) == B_READ) {
		bp->b_error = nfsread(bp->b_vp, bp->b_un.b_addr,
			bp->b_blkno * DEV_BSIZE, (int)bp->b_bcount,
			&resid, rp->r_cred);
		if (bp->b_error == 0) {
			if (resid) {
				bzero(bp->b_un.b_addr + bp->b_bcount - resid,
				      resid);
			}
			if (rp->r_size < rp->r_nfsattr.na_size) {
				rp->r_size = rp->r_nfsattr.na_size;
			}
		}
	} else {
		count = MIN(bp->b_bcount, rp->r_size - bp->b_blkno * DEV_BSIZE);
		if (count < 0) {	/* XXX */
			panic("do_bio: write count < 0");
		}
		bp->b_error = nfswrite(bp->b_vp, bp->b_un.b_addr,
			bp->b_blkno * DEV_BSIZE, count, rp->r_cred);
	}
	if (bp->b_error) {
		bp->b_flags |= B_ERROR;
	}
	iodone(bp);
}


int
nfs_badop()
{
	panic("nfs_badop");
}
