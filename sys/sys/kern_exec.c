/*
 * Copyright (c) 1982, 1986 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 *
 *	@(#)kern_exec.c	7.1 (Berkeley) 6/5/86
 */
#ifndef lint
static char rcs_id[] = {"$Header: kern_exec.c,v 3.1 86/10/22 13:40:56 tadl Exp $"};
#endif not lint
/*
 * RCS Info
 *	$Locker:  $
 */

#include "../machine/reg.h"
#include "../machine/pte.h"
#include "../machine/psl.h"

#include "param.h"
#include "systm.h"
#include "map.h"
#include "user.h"
#include "kernel.h"
#include "proc.h"
#include "buf.h"
#include "socketvar.h"
#include "vnode.h"
#include "pathname.h"
#include "seg.h"
#include "vm.h"
#include "text.h"
#include "file.h"
#include "uio.h"
#include "acct.h"
#include "exec.h"
#include "vfs.h"

#ifdef vax
#include "../vax/mtpr.h"
#endif

/*
 * texts below this size will be read in (if there is enough free memory)
 * even though the file is ZMAGIC
 */
size_t pgthresh = clrnd(btoc(PGTHRESH));

/*
 * exec system call, with and without environments.
 */
struct execa {
	char	*fname;
	char	**argp;
	char	**envp;
};

execv()
{
	((struct execa *)u.u_ap)->envp = NULL;
	execve();
}

execve()
{
	register nc;
	register char *cp;
	register struct buf *bp;
	register struct execa *uap;
	int na, ne, ucp, ap, cc;
	unsigned len;
	int indir, uid, gid;
	char *sharg;
	char *execnamep;
	struct vnode *vp;
	struct vattr vattr;
	struct pathname pn, tpn;
	swblk_t bno;
	char cfname[MAXCOMLEN + 1];
#define	SHSIZE	32
	char cfarg[SHSIZE];
	union {
		char	ex_shell[SHSIZE];	/* #! and name of interpreter */
		struct	exec ex_exec;
	} exdata;
	int resid, error;

	uap = (struct execa *)u.u_ap;
	u.u_error = pn_get(uap->fname, UIO_USERSPACE, &pn);
	if (u.u_error) {
		return;
	}
	u.u_error = lookuppn(&pn, FOLLOW_LINK, (struct vnode **)0, &vp);
	if (u.u_error) {
		pn_free(&pn);
		return;
	}
	bno = 0;
	bp = 0;
	indir = 0;
	uid = u.u_uid;
	gid = u.u_gid;
	if (u.u_error = VOP_GETATTR(vp, &vattr, u.u_cred))
		goto bad;
	if ((vp->v_vfsp->vfs_flag & VFS_NOSUID) == 0) {
		if (vattr.va_mode & VSUID)
			uid = vattr.va_uid;
		if (vattr.va_mode & VSGID)
			gid = vattr.va_gid;
	} else {
		struct pathname tmppn;
		u.u_error = pn_get(uap->fname, UIO_USERSPACE, &tmppn);
		if (u.u_error)
			return;
		printf("%s: Setuid execution not allowed\n", tmppn.pn_buf);
		pn_free(&tmppn);
	}
again:
	/*
	 * XXX should change VOP_ACCESS to not let super user always have it
	 * for exec permission on regular files.
	 */
	if (u.u_error = VOP_ACCESS(vp, VEXEC, u.u_cred))
		goto bad;
	if ((u.u_procp->p_flag&STRC)
		&& (u.u_error = VOP_ACCESS(vp, VREAD, u.u_cred)))
			goto bad;
	if (vp->v_type != VREG ||
		(vattr.va_mode & (VEXEC|(VEXEC>>3)|(VEXEC>>6))) == 0) {
		u.u_error = EACCES;
		goto bad;
	}

	/*
	 * Read in first few bytes of file for segment sizes, magic number:
	 *	407 = plain executable
	 *	410 = RO text
	 *	413 = demand paged RO text
	 * Also an ASCII line beginning with #! is
	 * the file name of a ``shell'' and arguments may be prepended
	 * to the argument list if given here.
	 *
	 * SHELL NAMES ARE LIMITED IN LENGTH.
	 *
	 * ONLY ONE ARGUMENT MAY BE PASSED TO THE SHELL FROM
	 * THE ASCII LINE.
	 */
	exdata.ex_shell[0] = '\0';	/* for zero length files */
	u.u_error = vn_rdwr(UIO_READ, vp, (caddr_t)&exdata, sizeof (exdata),
	    0, UIO_SYSSPACE, IO_UNIT, &resid);
	if (u.u_error)
		goto bad;
#ifndef lint
	if (resid > sizeof(exdata) - sizeof(exdata.ex_exec) &&
	    exdata.ex_shell[0] != '#') {
		u.u_error = ENOEXEC;
		goto bad;
	}
#endif
	switch ((int)exdata.ex_exec.a_magic) {

	case 0407:
		exdata.ex_exec.a_data += exdata.ex_exec.a_text;
		exdata.ex_exec.a_text = 0;
		break;

	case 0413:
	case 0410:
		if (exdata.ex_exec.a_text == 0) {
			u.u_error = ENOEXEC;
			goto bad;
		}
		break;

	default:
		if (exdata.ex_shell[0] != '#' ||
		    exdata.ex_shell[1] != '!' ||
		    indir) {
			u.u_error = ENOEXEC;
			goto bad;
		}
		cp = &exdata.ex_shell[2];		/* skip "#!" */
		while ((u_int)cp < (u_int)&exdata.ex_shell[SHSIZE]) {
			if (*cp == '\t')
				*cp = ' ';
			else if (*cp == '\n') {
				*cp = '\0';
				break;
			}
			cp++;
		}
		if (*cp != '\0') {
			u.u_error = ENOEXEC;
			goto bad;
		}
		cp = &exdata.ex_shell[2];
		while (*cp == ' ')
			cp++;
		execnamep = cp;
		while (*cp && *cp != ' ')
			cp++;
		cfarg[0] = '\0';
		if (*cp) {
			*cp++ = '\0';
			while (*cp == ' ')
				cp++;
			if (*cp)
				bcopy((caddr_t)cp, (caddr_t)cfarg, SHSIZE);
		}
		indir = 1;
		VN_RELE(vp);
		u.u_error = pn_get(execnamep, UIO_SYSSPACE, &tpn);
		if (u.u_error)
			goto bad;
		u.u_error =
			lookuppn(&tpn, FOLLOW_LINK, (struct vnode **)0, &vp);
		if (u.u_error) {
			vp = (struct vnode *)0;
			pn_free(&tpn);
			goto bad;
		}
		if (u.u_error = VOP_GETATTR(vp, &vattr, u.u_cred))
			goto bad;
		goto again;
	}

	/*
	 * Collect arguments on "file" in swap space.
	 */
	na = 0;
	ne = 0;
	nc = 0;
	cc = 0;
	uap = (struct execa *)u.u_ap;
	bno = rmalloc(argmap, (long)ctod(clrnd((int)btoc(NCARGS))));
	if (bno == 0) {
		swkill(u.u_procp, "exec: no swap space");
		goto bad;
	}
	if (bno % CLSIZE)
		panic("execa rmalloc");
	/*
	 * Copy arguments into file in argdev area.
	 */
	if (uap->argp) for (;;) {
		ap = NULL;
		sharg = NULL;
		if (indir && na == 0) {
			sharg = cfname;
			ap = (int)sharg;
			uap->argp++;		/* ignore argv[0] */
		} else if (indir && (na == 1 && cfarg[0])) {
			sharg = cfarg;
			ap = (int)sharg;
		} else if (indir && (na == 1 || na == 2 && cfarg[0]))
			ap = (int)uap->fname;
		else if (uap->argp) {
			ap = fuword((caddr_t)uap->argp);
			uap->argp++;
		}
		if (ap == NULL && uap->envp) {
			uap->argp = NULL;
			if ((ap = fuword((caddr_t)uap->envp)) != NULL)
				uap->envp++, ne++;
		}
		if (ap == NULL)
			break;
		na++;
		if (ap == -1) {
			u.u_error = EFAULT;
			break;
		}
		do {
			if (cc <= 0) {
				/*
				 * We depend on NCARGS being a multiple of
				 * CLSIZE*NBPG.  This way we need only check
				 * overflow before each buffer allocation.
				 */
				if (nc >= NCARGS-1) {
					error = E2BIG;
					break;
				}
				if (bp)
					bdwrite(bp);
				cc = CLSIZE*NBPG;
				bp = getblk(argdev_vp, (daddr_t)(bno + ctod(nc/NBPG)), cc);
				cp = bp->b_un.b_addr;
			}
			if (sharg) {
				error = copystr(sharg, cp, (unsigned)cc, &len);
				sharg += len;
			} else {
				error = copyinstr((caddr_t)ap, cp, (unsigned)cc,
				    &len);
				ap += len;
			}
			cp += len;
			nc += len;
			cc -= len;
		} while (error == ENOENT);
		if (error) {
			u.u_error = error;
			if (bp)
				brelse(bp);
			bp = 0;
			goto badarg;
		}
	}
	if (bp)
		bdwrite(bp);
	bp = 0;
	nc = (nc + NBPW-1) & ~(NBPW-1);
	getxfile(vp, &exdata.ex_exec, nc + (na+4)*NBPW, uid, gid, &pn);
	if (u.u_error) {
badarg:
		for (cc = 0; cc < nc; cc += CLSIZE*NBPG) {
			bp = baddr(argdev_vp, (daddr_t)(bno + ctod(cc/NBPG)), CLSIZE*NBPG);
			if (bp) {
				bp->b_flags |= B_AGE;		/* throw away */
				bp->b_flags &= ~B_DELWRI;	/* cancel io */
				brelse(bp);
				bp = 0;
			}
		}
		goto bad;
	}

	/*
	 * Copy back arglist.
	 */
	ucp = USRSTACK - nc - NBPW;
	ap = ucp - na*NBPW - 3*NBPW;
	u.u_ar0[SP] = ap;
	(void) suword((caddr_t)ap, na-ne);
	nc = 0;
	cc = 0;
	for (;;) {
		ap += NBPW;
		if (na == ne) {
			(void) suword((caddr_t)ap, 0);
			ap += NBPW;
		}
		if (--na < 0)
			break;
		(void) suword((caddr_t)ap, ucp);
		do {
			if (cc <= 0) {
				if (bp)
					brelse(bp);
				cc = CLSIZE*NBPG;
				bp = bread(argdev_vp, (daddr_t)(bno + ctod(nc / NBPG)), cc);
				bp->b_flags |= B_AGE;		/* throw away */
				bp->b_flags &= ~B_DELWRI;	/* cancel io */
				cp = bp->b_un.b_addr;
			}
			error = copyoutstr(cp, (caddr_t)ucp, (unsigned)cc,
			    &len);
			ucp += len;
			cp += len;
			nc += len;
			cc -= len;
		} while (error == ENOENT);
		if (error == EFAULT)
			panic("exec: EFAULT");
	}
	(void) suword((caddr_t)ap, 0);

	/*
	 * Reset caught signals.  Held signals
	 * remain held through p_sigmask.
	 */
	while (u.u_procp->p_sigcatch) {
		nc = ffs((long)u.u_procp->p_sigcatch);
		u.u_procp->p_sigcatch &= ~sigmask(nc);
		u.u_signal[nc] = SIG_DFL;
	}
	/*
	 * Reset stack state to the user stack.
	 * Clear set of signals caught on the signal stack.
	 */
	u.u_onstack = 0;
	u.u_sigsp = 0;
	u.u_sigonstack = 0;

	for (nc = u.u_lastfile; nc >= 0; --nc) {
		if (u.u_pofile[nc] & UF_EXCLOSE) {
			closef(u.u_ofile[nc]);
			u.u_ofile[nc] = NULL;
			u.u_pofile[nc] = 0;
		}
		u.u_pofile[nc] &= ~UF_MAPPED;
	}
	while (u.u_lastfile >= 0 && u.u_ofile[u.u_lastfile] == NULL)
		u.u_lastfile--;
	setregs(exdata.ex_exec.a_entry);
	/*
	 * Remember file name for accounting.
	 */
	u.u_acflag &= ~AFORK;
	if (indir)
		bcopy((caddr_t)cfname, (caddr_t)u.u_comm, MAXCOMLEN);
	else {
		if (pn.pn_pathlen > MAXCOMLEN)
			pn.pn_pathlen = MAXCOMLEN;
		bcopy((caddr_t)pn.pn_buf, (caddr_t)u.u_comm,
			(unsigned)(pn.pn_pathlen + 1));
	}
bad:
	pn_free(&pn);
	if (bp)
		brelse(bp);
	if (bno)
		rmfree(argmap, (long)ctod(clrnd((int) btoc(NCARGS))), bno);
	if (vp)
		VN_RELE(vp);
}

/*
 * Read in and set up memory for executed file.
 */
int execwarn = 1;

getxfile(vp, ep, nargc, uid, gid, pn)
	register struct vnode *vp;
	register struct exec *ep;
	int nargc, uid, gid;
	struct pathname *pn;
{
	register size_t ts, ds, ids, uds, ss;
	int pagi;

	/*
	 * Check to make sure nobody is modifying the text right now
	 */
	if ((vp->v_flag & VTEXTMOD) != 0) {
		u.u_error = ETXTBSY;
		goto bad;
	}
	if ((ep->a_text != 0 && (vp->v_flag&VTEXT) ==0) &&
		(vp->v_count != 1)) {
		register struct file *fp;

		for (fp = file; fp < fileNFILE; fp++) {
			if (fp->f_type == DTYPE_VNODE &&
			    fp->f_count > 0 &&
			    (struct vnode *)fp->f_data == vp &&
			    (fp->f_flag&FWRITE)) {
				u.u_error = ETXTBSY;
				goto bad;
			}
		}
	}

	/*
	 * Compute text and data sizes and make sure not too large.
	 * NB - Check data and bss separately as they may overflow 
	 * when summed together.
	 */
	ts = clrnd(btoc(ep->a_text));
	ids = clrnd(btoc(ep->a_data));
	uds = clrnd(btoc(ep->a_bss));
	ds = clrnd(btoc(ep->a_data + ep->a_bss));
	ss = clrnd(SSIZE + btoc(nargc));
	if (chksize((unsigned)ts, (unsigned)ids, (unsigned)uds, (unsigned)ss))
		goto bad;

	if ((ep->a_magic == 0413) && ((ts + ids) > MIN(freemem, pgthresh)))
		if (vp->v_vfsp->vfs_bsize < CLBYTES) {
			if (execwarn) {
				uprintf("Warning: file system block size ");
				uprintf("too small, '%s' not demand paged\n", pn->pn_buf);
			}
			pagi = 0;
		} else {
			pagi = SPAGI;
		}
	else 
		pagi = 0;
	/*
	 * Make sure enough space to start process.
	 */
	u.u_cdmap = zdmap;
	u.u_csmap = zdmap;
	if (swpexpand(ds, ss, &u.u_cdmap, &u.u_csmap) == NULL)
		goto bad;

	/*
	 * At this point, committed to the new image!
	 * Release virtual memory resources of old process, and
	 * initialize the virtual memory of the new process.
	 * If we resulted from vfork(), instead wakeup our
	 * parent who will set SVFDONE when he has taken back
	 * our resources.
	 */
	if ((u.u_procp->p_flag & SVFORK) == 0)
		vrelvm();
	else {
		u.u_procp->p_flag &= ~SVFORK;
		u.u_procp->p_flag |= SKEEP;
		wakeup((caddr_t)u.u_procp);
		while ((u.u_procp->p_flag & SVFDONE) == 0)
			sleep((caddr_t)u.u_procp, PZERO - 1);
		u.u_procp->p_flag &= ~(SVFDONE|SKEEP);
	}
	u.u_procp->p_flag &= ~(SPAGI|SSEQL|SUANOM|SOUSIG);
	u.u_procp->p_flag |= pagi;
	u.u_dmap = u.u_cdmap;
	u.u_smap = u.u_csmap;
	vgetvm(ts, ds, ss);

	if (pagi == 0)
		u.u_error =
		    vn_rdwr(UIO_READ, vp,
			(char *)ctob(dptov(u.u_procp, 0)),
			(int)ep->a_data,
			(int)((ep->a_magic == 0413?
				CLBYTES:
				sizeof (struct exec)) + ep->a_text),
			UIO_SYSSPACE, IO_UNIT, (int *)0);
	xalloc(vp, ep, pagi);
	if (pagi && u.u_procp->p_textp)
		vinifod((struct fpte *)dptopte(u.u_procp, 0),
		    PG_FTEXT, u.u_procp->p_textp->x_vptr,
		    (long)(1 + ts/CLSIZE), (size_t)btoc(ep->a_data));

#ifdef vax
	/* THIS SHOULD BE DONE AT A LOWER LEVEL, IF AT ALL */
	mtpr(TBIA, 0);
#endif

	if (u.u_error)
		swkill(u.u_procp, "exec: I/O error mapping pages");
	/*
	 * set SUID/SGID protections, if no tracing
	 */
	if ((u.u_procp->p_flag&STRC)==0) {
		if (uid != u.u_uid || gid != u.u_gid)
			u.u_cred = crcopy(u.u_cred);
		u.u_uid = uid;
		u.u_procp->p_uid = uid;
		u.u_gid = gid;
	} else
		psignal(u.u_procp, SIGTRAP);
	u.u_tsize = ts;
	u.u_dsize = ds;
	u.u_ssize = ss;
	u.u_prof.pr_scale = 0;
bad:
	return;
}
