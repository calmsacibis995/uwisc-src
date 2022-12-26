/*
 * Copyright (c) 1982, 1986 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 *
 *	@(#)kern_acct.c	7.1 (Berkeley) 6/5/86
 */
#ifndef lint
static char rcs_id[] = {"$Header: kern_acct.c,v 3.1 86/10/22 13:40:29 tadl Exp $"};
#endif not lint
/*
 * RCS Info
 *	$Locker:  $
 */

#include "param.h"
#include "systm.h"
#include "user.h"
#include "vnode.h"
#include "vfs.h"
#include "kernel.h"
#include "acct.h"
#include "uio.h"

/*
 * SHOULD REPLACE THIS WITH A DRIVER THAT CAN BE READ TO SIMPLIFY.
 */
struct	vnode *acctp;
struct	vnode *savacctp;

/*
 * Perform process accounting functions.
 */
sysacct()
{
	struct vnode *vp;
	register struct a {
		char	*fname;
	} *uap = (struct a *)u.u_ap;

	if (suser()) {
		if (savacctp) {
			acctp = savacctp;
			savacctp = NULL;
		}
		if (uap->fname==NULL) {
			if (vp = acctp) {
				VN_RELE(vp);
				acctp = NULL;
			}
			return;
		}
		u.u_error = 
			lookupname(uap->fname, UIO_USERSPACE, FOLLOW_LINK,
				(struct vnode **)0, &vp);
		if (u.u_error)
			return;
		if (vp->v_type != VREG) {
			u.u_error = EACCES;
			return;
		}
		if (acctp)
			VN_RELE(acctp);
		acctp = vp;
	}
}

int	acctsuspend = 2;	/* stop accounting when < 2% free space left */
int	acctresume = 4;		/* resume when free space risen to > 4% */

struct	acct acctbuf;
/*
 * On exit, write a record on the accounting file.
 */
acct()
{
	register int i;
	register struct vnode *vp;
	register struct rusage *ru;
	struct timeval t;
	register struct acct *ap = &acctbuf;
	struct statfs sb;

	if (savacctp) {
		(void)VFS_STATFS(savacctp->v_vfsp, &sb);
		if (sb.f_bavail > (acctresume * sb.f_blocks / 100)) {
			acctp = savacctp;
			savacctp = NULL;
			printf("Accounting resumed\n");
		}
	}
	if ((vp = acctp) == NULL)
		return;
	(void)VFS_STATFS(acctp->v_vfsp, &sb);
	if (sb.f_bavail <= (acctsuspend * sb.f_blocks / 100)) {
		savacctp = acctp;
		acctp = NULL;
		printf("Accounting suspended\n");
		return;
	}
	for (i = 0; i < sizeof (ap->ac_comm); i++)
		ap->ac_comm[i] = u.u_comm[i];
	ru = &u.u_ru;
	ap->ac_utime = compress(ru->ru_utime.tv_sec, ru->ru_utime.tv_usec);
	ap->ac_stime = compress(ru->ru_stime.tv_sec, ru->ru_stime.tv_usec);
	t = time;
	timevalsub(&t, &u.u_start);
	ap->ac_etime = compress(t.tv_sec, t.tv_usec);
	ap->ac_btime = u.u_start.tv_sec;
	ap->ac_uid = u.u_ruid;
	ap->ac_gid = u.u_rgid;
	t = ru->ru_stime;
	timevaladd(&t, &ru->ru_utime);
	if (i = t.tv_sec * hz + t.tv_usec / tick)
		ap->ac_mem = (ru->ru_ixrss+ru->ru_idrss+ru->ru_isrss) / i;
	else
		ap->ac_mem = 0;
	ap->ac_mem >>= CLSIZELOG2;
	ap->ac_io = compress(ru->ru_inblock + ru->ru_oublock, (long)0);
	if (u.u_ttyp)
		ap->ac_tty = u.u_ttyd;
	else
		ap->ac_tty = NODEV;
	ap->ac_flag = u.u_acflag;
	u.u_error =
		vn_rdwr(UIO_WRITE, vp, (caddr_t)ap, sizeof(acctbuf), 0,
			UIO_SYSSPACE, IO_UNIT|IO_APPEND, (int *)0);
}

/*
 * Produce a pseudo-floating point representation
 * with 3 bits base-8 exponent, 13 bits fraction.
 */
compress(t, ut)
	register long t;
	long ut;
{
	register exp = 0, round = 0;

	t = t * AHZ;  /* compiler will convert only this format to a shift */
	if (ut)
		t += ut / (1000000 / AHZ);
	while (t >= 8192) {
		exp++;
		round = t&04;
		t >>= 3;
	}
	if (round) {
		t++;
		if (t >= 8192) {
			t >>= 3;
			exp++;
		}
	}
	return ((exp<<13) + t);
}
