#ifndef lint
static char rcs_id[] =
	{"$Header: nfs_subr.c,v 3.1 86/10/22 15:07:26 tadl Exp $"};
#endif not lint
/*
 * RCS Info
 *	$Locker:  $
 */
/* NFSSRC @(#)nfs_subr.c	2.1 86/04/14 */
/*	@(#)nfs_subr.c 1.1 86/02/03 SMI	*/

#include "../h/param.h"
#include "../h/systm.h"
#include "../h/user.h"
#include "../h/kernel.h"
#include "../h/buf.h"
#include "../h/vfs.h"
#include "../h/vnode.h"
#include "../h/proc.h"
#include "../h/socket.h"
#include "../h/socketvar.h"
#include "../ufs/inode.h"
#include "../ufs/fs.h"
#include "../h/uio.h"
#include "../net/if.h"
#include "../netinet/in.h"
#include "../rpc/types.h"
#include "../rpc/xdr.h"
#include "../rpc/auth.h"
#include "../rpc/clnt.h"
#include "../nfs/nfs.h"
#include "../nfs/nfs_clnt.h"
#include "../nfs/rnode.h"

#ifdef NFSDEBUG
int nfsdebug = 13;
#endif

extern struct vnodeops nfs_vnodeops;
struct inode *iget();
struct rnode *rfind();

/*
 * Client side utilities
 */

/*
 * client side statistics
 */
struct {
	int	nclsleeps;		/* client handle waits */
	int	nclgets;		/* client handle gets */
	int	ncalls;			/* client requests */
	int	nbadcalls;		/* rpc failures */
	int	reqs[32];		/* count of each request */
} clstat;


#define MAXCLIENTS	4
struct chtab {
	int	ch_timesused;
	bool_t	ch_inuse;
	CLIENT	*ch_client;
} chtable[MAXCLIENTS];

int	clwanted = 0;

CLIENT *
clget(mi, cred)
	struct mntinfo *mi;
	struct ucred *cred;
{
	register struct chtab *ch;

	for (;;) {
		clstat.nclgets++;
		for (ch = chtable; ch < &chtable[MAXCLIENTS]; ch++) {
			if (!ch->ch_inuse) {
				ch->ch_inuse = TRUE;
				if (ch->ch_client == NULL) {
					ch->ch_client =
					    clntkudp_create(&mi->mi_addr,
					    NFS_PROGRAM, NFS_VERSION,
					    mi->mi_retrans, cred);
				} else {
					clntkudp_init(ch->ch_client,
					    &mi->mi_addr, mi->mi_retrans, cred);
				}
				if (ch->ch_client == NULL) {
					panic("clget: null client");
				}
				ch->ch_timesused++;
				return (ch->ch_client);
			}
		}
		/*
		 * If we got here there are no available handles
		 */
		clwanted++;
		clstat.nclsleeps++;
		sleep((caddr_t)chtable, PRIBIO);
	}
}

clfree(cl)
	CLIENT *cl;
{
	register struct chtab *ch;

	for (ch = chtable; ch < &chtable[MAXCLIENTS]; ch++) {
		if (ch->ch_client == cl) {
			ch->ch_inuse = FALSE;
			break;
		}
	}
	if (clwanted) {
		clwanted = 0;
		wakeup((caddr_t)chtable);
	}
}

char *rpcstatnames[] = {
	"SUCCESS", "CANT ENCODE ARGS", "CANT DECODE RES", "CANT SEND",
	"CANT RECV", "TIMED OUT", "VERS MISMATCH", "AUTH ERROR", "PROG UNAVAIL",
	"PROG VERS MISMATCH", "PROC UNAVAIL", "CANT DECODE ARGS", "UNKNOWN HOST",
	"UNKNOWN", "PMAP FAILURE", "PROG NOT REGISTERED", "SYSTEM ERROR", "FAILED"
	};
char *rfsnames[] = {
	"null", "getattr", "setattr", "unused", "lookup", "readlink", "read",
	"unused", "write", "create", "remove", "rename", "link", "symlink",
	"mkdir", "rmdir", "readdir", "fsstat" };

/*
 * Back off for retransmission timeout, MAXTIMO is in 10ths of a sec
 */
#define MAXTIMO	300
#define backoff(tim)	((((tim) << 2) > MAXTIMO) ? MAXTIMO : ((tim) << 2))

int
rfscall(mi, which, xdrargs, argsp, xdrres, resp, cred)
	register struct mntinfo *mi;
	int	 which;
	xdrproc_t xdrargs;
	caddr_t	argsp;
	xdrproc_t xdrres;
	caddr_t	resp;
	struct ucred *cred;
{
	CLIENT *client;
	register enum clnt_stat status;
	struct rpc_err rpcerr;
	struct timeval wait;
	struct ucred *newcred;
	int timeo;
	bool_t tryagain;

#ifdef NFSDEBUG
	dprint(nfsdebug, 6, "rfscall: %x, %d, %x, %x, %x, %x\n",
	    mi, which, xdrargs, argsp, xdrres, resp);
#endif
	rpcerr.re_errno = 0;
	clstat.ncalls++;
	clstat.reqs[which]++;
	newcred = NULL;
	timeo = mi->mi_timeo;
retry:
	client = clget(mi, cred);

	/*
	 * If hard mounted fs, retry call forever unless hard error occurs
	 */
	do {
		wait.tv_sec = timeo / 10;
		wait.tv_usec = 100000 * (timeo % 10);
		status = CLNT_CALL(client, which, xdrargs, argsp,
		    xdrres, resp, wait);
		switch (status) {
		case RPC_SUCCESS:
			tryagain = FALSE;
			break;

		/*
		 * Unrecoverable errors: give up immediately
		 */
		case RPC_AUTHERROR:
		case RPC_CANTENCODEARGS:
		case RPC_CANTDECODERES:
		case RPC_VERSMISMATCH:
		case RPC_PROGVERSMISMATCH:
		case RPC_CANTDECODEARGS:
			tryagain = FALSE;
			break;

		default:
			if (!mi->mi_printed) {
				mi->mi_printed = 1;
				printf("NFS server %s not responding",
				    mi->mi_hostname);
				if (mi->mi_hard) {
					printf(", still trying\n");
				} else {
					printf(", giving up\n");
				}
			}
			timeo = backoff(timeo);
			tryagain = TRUE;
		}
	} while (tryagain && mi->mi_hard);

	if (status != RPC_SUCCESS) {
		CLNT_GETERR(client, &rpcerr);
		clstat.nbadcalls++;
		printf("NFS %s failed for server %s: %s\n", rfsnames[which],
		    mi->mi_hostname,rpcstatnames[(int)status]);
	} else if (resp && *(int *)resp == EACCES &&
	    newcred == NULL && cred->cr_uid == 0 && cred->cr_ruid != 0) {
		/*
		 * Boy is this a kludge!  If the reply status is EACCES
		 * it may be because we are root (no root net access).
		 * Check the real uid, if it isn't root make that
		 * the uid instead and retry the call.
		 */
		newcred = crdup(cred);
		cred = newcred;
		cred->cr_uid = cred->cr_ruid;
		clfree(client);
		goto retry;
	} else if (mi->mi_printed && mi->mi_hard) {
		printf("NFS server %s ok\n", mi->mi_hostname);
	}
	mi->mi_printed = 0;

	clfree(client);
#ifdef NFSDEBUG
	dprint(nfsdebug, 7, "rfscall: returning %d\n", rpcerr.re_errno);
#endif
	if (newcred) {
		crfree(newcred);
	}
	return (rpcerr.re_errno);
}

nattr_to_vattr(na, vap)
	register struct nfsfattr *na;
	register struct vattr *vap;
{

	vap->va_type = (enum vtype)na->na_type;
	vap->va_mode = na->na_mode;
	vap->va_uid = na->na_uid;
	vap->va_gid = na->na_gid;
	vap->va_fsid = na->na_fsid;
	vap->va_nodeid = na->na_nodeid;
	vap->va_nlink = na->na_nlink;
	vap->va_size = na->na_size;
	vap->va_atime.tv_sec  = na->na_atime.tv_sec;
	vap->va_atime.tv_usec = na->na_atime.tv_usec;
	vap->va_mtime.tv_sec  = na->na_mtime.tv_sec;
	vap->va_mtime.tv_usec = na->na_mtime.tv_usec;
	vap->va_ctime.tv_sec  = na->na_ctime.tv_sec;
	vap->va_ctime.tv_usec = na->na_ctime.tv_usec;
	vap->va_rdev = na->na_rdev;
	vap->va_blocks = na->na_blocks;
	switch(na->na_type) {

	case NFBLK:
		vap->va_blocksize = BLKDEV_IOSIZE;
		break;

	case NFCHR:
		vap->va_blocksize = MAXBSIZE;
		break;

	default:
		vap->va_blocksize = na->na_blocksize;
		break;
	}
}

vattr_to_sattr(vap, sa)
	register struct vattr *vap;
	register struct nfssattr *sa;
{
	sa->sa_mode = vap->va_mode;
	sa->sa_uid = vap->va_uid;
	sa->sa_gid = vap->va_gid;
	sa->sa_size = vap->va_size;
	sa->sa_atime.tv_sec  = vap->va_atime.tv_sec;
	sa->sa_atime.tv_usec = vap->va_atime.tv_usec;
	sa->sa_mtime.tv_sec  = vap->va_mtime.tv_sec;
	sa->sa_mtime.tv_usec = vap->va_mtime.tv_usec;
}

setdiropargs(da, nm, dvp)
	struct nfsdiropargs *da;
	char *nm;
	struct vnode *dvp;
{

	da->da_fhandle = *vtofh(dvp);
	da->da_name = nm;
}

/*
 * return a vnode for the given fhandle.
 * If no rnode exists for this fhandle create one and put it
 * in a table hashed by fh_fsid and fs_fid.  If the rnode for
 * this fhandle is already in the table return it (ref count is
 * incremented by rfind.  The rnode will be flushed from the
 * table when nfs_inactive calls runsave.
 */
struct vnode *
makenfsnode(fh, attr, vfsp)
	fhandle_t *fh;
	struct nfsfattr *attr;
	struct vfs *vfsp;
{
	register struct rnode *rp;
	char newnode = 0;

	if ((rp = rfind(fh, vfsp)) == NULL) {
		rp = (struct rnode *)kmem_alloc((u_int)sizeof(*rp));
		bzero((caddr_t)rp, sizeof(*rp));
		rp->r_fh = *fh;
		rtov(rp)->v_count = 1;
		rtov(rp)->v_op = &nfs_vnodeops;
		if (attr) {
			rtov(rp)->v_type = (enum vtype)attr->na_type;
			rtov(rp)->v_rdev = attr->na_rdev;
		}
		rtov(rp)->v_data = (caddr_t)rp;
		rtov(rp)->v_vfsp = vfsp;
		rsave(rp);
		((struct mntinfo *)(vfsp->vfs_data))->mi_refct++;
		newnode++;
	}
	if (attr) {
		nfs_attrcache(rtov(rp), attr, (newnode? NOFLUSH: SFLUSH));
	}
	return (rtov(rp));
}

/*
 * Rnode lookup stuff.
 * These routines maintain a table of rnodes hashed by fhandle so
 * that the rnode for an fhandle can be found if it already exists.
 * NOTE: RTABLESIZE must be a power of 2 for rtablehash to work!
 */

#define	RTABLESIZE	16
#define	rtablehash(fh)	(fh->fh_fno & (RTABLESIZE-1))

struct rnode *rtable[RTABLESIZE];

/*
 * Put a rnode in the table
 */
rsave(rp)
	struct rnode *rp;
{

	rp->r_next = rtable[rtablehash(rtofh(rp))];
	rtable[rtablehash(rtofh(rp))] = rp;
}

/*
 * Remove a rnode from the table
 */
runsave(rp)
	struct rnode *rp;
{
	struct rnode *rt;
	struct rnode *rtprev = NULL;

	rt = rtable[rtablehash(rtofh(rp))]; 
	while (rt != NULL) { 
		if (rt == rp) { 
			if (rtprev == NULL) {
				rtable[rtablehash(rtofh(rp))] = rt->r_next;
			} else {
				rtprev->r_next = rt->r_next;
			}
			return; 
		}	
		rtprev = rt;
		rt = rt->r_next;
	}	
}

/*
 * Lookup a rnode by fhandle
 */
struct rnode *
rfind(fh, vfsp)
	fhandle_t *fh;
	struct vfs *vfsp;
{
	register struct rnode *rt;

	rt = rtable[rtablehash(fh)]; 
	while (rt != NULL) { 
		if (bcmp((caddr_t)rtofh(rt), (caddr_t)fh, sizeof(*fh)) == 0 &&
		    vfsp == rtov(rt)->v_vfsp) { 
			rtov(rt)->v_count++; 
			return (rt); 
		}	
		rt = rt->r_next;
	}	
	return (NULL);
}

/*
 * remove buffers from the buffer cache that have this vfs.
 * NOTE: assumes buffers have been written already.
 */
rinval(vfsp)
	struct vfs *vfsp;
{
	int	i;
	register struct rnode *rp, *rptmp;

	for (i = 0; i < RTABLESIZE; i++) {
		for (rp = rtable[i]; rp; ) {
			if (rp->r_vnode.v_vfsp == vfsp) {
				/*
				 * Need to hold the vp because binval releases
				 * it which will call rfree if the ref count
				 * goes to zero and rfree will mess up rtable.
				 */
				VN_HOLD(rtov(rp));
				rptmp = rp;
				rp = rp->r_next;
				binval(rtov(rptmp));
				VN_RELE(rtov(rptmp));
			} else {
				rp = rp->r_next;
			}
		}
	}
}

/*
 * Flush dirty buffers for all vnodes in this vfs
 */
rflush(vfsp)
	struct vfs *vfsp;
{
	int	i;
	register struct rnode *rp;

	for (i = 0; i < RTABLESIZE; i++) {
		for (rp = rtable[i]; rp; rp = rp->r_next) {
			if (rp->r_vnode.v_vfsp == vfsp) {
				bflush(rtov(rp));
			}
		}
	}
}

#define	PREFIXLEN	4
static char prefix[PREFIXLEN+1] = ".nfs";

char *
newname(s)
	char *s;
{
	char *news;
	register char *s1, *s2;
	int id;

	id = time.tv_usec;
	news = (char *)kmem_alloc((u_int)NFS_MAXNAMLEN);
#ifdef UW
	/*
	 * 9/11/86  tadl
	 * the original code ignored the name passed in.
	 * changed to use the name passed as part of the 
	 * new filename
	 */
	for (s1 = news; *s; ) {
		*s1++ = *s++;
	}
	for ( s2 = prefix; s2 < &prefix[PREFIXLEN]; ) {
		*s1++ = *s2++;
	}
#else
	for (s1 = news, s2 = prefix; s2 < &prefix[PREFIXLEN]; ) {
		*s1++ = *s2++;
	}
#endif UW
	while (id) {
		*s1++ = "0123456789ABCDEF"[id & 0x0f];
		id = id >> 4;
	}
	*s1 = '\0';
	return (news);
}


/*
 * Server side utilities
 */

vattr_to_nattr(vap, na)
	register struct vattr *vap;
	register struct nfsfattr *na;
{
	na->na_type = (enum nfsftype)vap->va_type;
	na->na_mode = vap->va_mode;
	na->na_uid = vap->va_uid;
	na->na_gid = vap->va_gid;
	na->na_fsid = vap->va_fsid;
	na->na_nodeid = vap->va_nodeid;
	na->na_nlink = vap->va_nlink;
	na->na_size = vap->va_size;
	na->na_atime.tv_sec  = vap->va_atime.tv_sec;
	na->na_atime.tv_usec = vap->va_atime.tv_usec;
	na->na_mtime.tv_sec  = vap->va_mtime.tv_sec;
	na->na_mtime.tv_usec = vap->va_mtime.tv_usec;
	na->na_ctime.tv_sec  = vap->va_ctime.tv_sec;
	na->na_ctime.tv_usec = vap->va_ctime.tv_usec;
	na->na_rdev = vap->va_rdev;
	na->na_blocks = vap->va_blocks;
	na->na_blocksize = vap->va_blocksize;
}

sattr_to_vattr(sa, vap)
	register struct nfssattr *sa;
	register struct vattr *vap;
{
	vattr_null(vap);
	vap->va_mode = sa->sa_mode;
	vap->va_uid = sa->sa_uid;
	vap->va_gid = sa->sa_gid;
	vap->va_size = sa->sa_size;
	vap->va_atime.tv_sec  = sa->sa_atime.tv_sec;
	vap->va_atime.tv_usec = sa->sa_atime.tv_usec;
	vap->va_mtime.tv_sec  = sa->sa_mtime.tv_sec;
	vap->va_mtime.tv_usec = sa->sa_mtime.tv_usec;
}

/*
 * Make an fhandle from a ufs vnode
 */
makefh(fh, vp)
	register fhandle_t *fh;
	struct vnode *vp;
{
	struct inode *ip;

	if (vp->v_op != &ufs_vnodeops) {
		return (EREMOTE);
	}
	ip = VTOI(vp);
	bzero((caddr_t)fh, NFS_FHSIZE);
	fh->fh_fsid = ip->i_dev;
	fh->fh_fno = ip->i_number;
	fh->fh_fgen = ip->i_gen;
	return (0);
}

extern int nobody;

/*
 * Convert an fhandle into a vnode.
 * Uses the inode number in the fhandle (fh_fno) to get the locked inode.
 * The inode is unlocked and used to get the vnode.
 * WARNING: users of this routine must do a VN_RELE on the vnode when they
 * are done with it.
 */
struct vnode *
fhtovp(fh)
	fhandle_t *fh;
{
	register struct inode *ip;
	struct fs *fs, *trygetfs();

	fs = trygetfs(fh->fh_fsid);
	if (fs == NULL) {
		return (NULL);
	}
	ip = iget(fh->fh_fsid, fs, fh->fh_fno);
	if (ip == NULL) {
#ifdef NFSDEBUG
		dprint(nfsdebug, 1, "fhtovp(%x, %x) couldn't iget\n",
		    fh->fh_fsid, fh->fh_fno);
#endif
		return (NULL);
	}
	if (ip->i_gen != fh->fh_fgen) {
#ifdef NFSDEBUG
		dprint(nfsdebug, 2, "NFS stale fhandle %o %d\n",
		    fh->fh_fsid, fh->fh_fno);
#endif
		idrop(ip);
		return (NULL);
	}
	iunlock(ip);
	if (u.u_cred->cr_uid == nobody &&
	    (ITOV(ip)->v_vfsp->vfs_flag & VFS_EXPORTED)) {
		/*
		 * We assume here that the uid is already set to "nobody"
		 * if the uid in the credentials with the call was 0.
		 */
#ifdef NFSDEBUG
		dprint(nfsdebug, 3, "fhtovp: root -> %d\n",
		    ITOV(ip)->v_vfsp->vfs_exroot);
#endif
		u.u_cred->cr_uid = ITOV(ip)->v_vfsp->vfs_exroot;
	}
	return (ITOV(ip));
}

/*
 * Exportfs system call
 */
exportfs()
{
	register struct a {
		char *dname;
		int rootid;
		int flags;
	} *uap = (struct a *) u.u_ap;
	struct vnode *vp;
	register int error;

	error = lookupname(uap->dname, UIO_USERSPACE, FOLLOW_LINK,
	    (struct vnode **)0, &vp);
	if (error) {
		return (error);
	}
	vp->v_vfsp->vfs_exroot = (u_short)uap->rootid;
	vp->v_vfsp->vfs_exflags = (short)uap->flags;
	vp->v_vfsp->vfs_flag |= VFS_EXPORTED;
#ifdef NFSDEBUG
	dprint(nfsdebug, 3, "exportfs: root %d flags %x\n",
	    vp->v_vfsp->vfs_exroot, vp->v_vfsp->vfs_exflags);
#endif
	VN_RELE(vp);
	return (0);
}

/*
 * General utilities
 */

/*
 * Returns the prefered transfer size in bytes based on
 * what network interfaces are available.
 */
nfstsize()
{
	register struct ifnet *ifp;

	for (ifp = ifnet; ifp; ifp = ifp->if_next) {
		if (ifp->if_name[0] == 'e' && ifp->if_name[1] == 'c') {
#ifdef NFSDEBUG
			dprint(nfsdebug, 3, "nfstsize: %d\n", ECTSIZE);
#endif
			return (ECTSIZE);
		}
	}
#ifdef NFSDEBUG
	dprint(nfsdebug, 3, "nfstsize: %d\n", IETSIZE);
#endif
	return (IETSIZE);
}

#ifdef NFSDEBUG
/*
 * Utilities used by both client and server
 * Standard levels:
 * 0) no debugging
 * 1) hard failures
 * 2) soft failures
 * 3) current test software
 * 4) main procedure entry points
 * 5) main procedure exit points
 * 6) utility procedure entry points
 * 7) utility procedure exit points
 * 8) obscure procedure entry points
 * 9) obscure procedure exit points
 * 10) random stuff
 * 11) all <= 1
 * 12) all <= 2
 * 13) all <= 3
 * ...
 */

/*VARARGS2*/
dprint(var, level, str, a1, a2, a3, a4, a5, a6, a7, a8, a9)
	int var;
	int level;
	char *str;
	int a1, a2, a3, a4, a5, a6, a7, a8, a9;
{
	if (var == level || (var > 10 && (var - 10) >= level))
		printf(str, a1, a2, a3, a4, a5, a6, a7, a8, a9);
}
#endif
