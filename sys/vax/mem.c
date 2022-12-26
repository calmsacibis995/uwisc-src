/*
 * Copyright (c) 1982, 1986 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 *
 *	@(#)mem.c	7.1 (Berkeley) 6/5/86
 */
#ifndef lint
static char rcs_id[] = {"$Header: mem.c,v 3.1 86/10/22 13:53:17 tadl Exp $"};
#endif not lint
/*
 * RCS Info
 *	$Locker:  $
 */

/*
 * Memory special file
 */

#include "pte.h"

#include "param.h"
#include "dir.h"
#include "user.h"
#include "conf.h"
#include "buf.h"
#include "systm.h"
#include "vm.h"
#include "cmap.h"
#include "uio.h"
#ifdef UW
#include "ioctl.h"
#include "kernel.h"
#endif UW

#include "mtpr.h"

mmread(dev, uio)
	dev_t dev;
	struct uio *uio;
{

	return (mmrw(dev, uio, UIO_READ));
}

mmwrite(dev, uio)
	dev_t dev;
	struct uio *uio;
{

	return (mmrw(dev, uio, UIO_WRITE));
}

#ifdef UW

mminit()
{
	char *timemem;
        register int *pte;
	extern struct pte Sysmap[6*NPTEPG];

	/*
	 * Map memory so time structure is user readable
	 */
	timemem = (char *)(((int) &time) & ~(0x80000000));
	pte = (int *)(Sysmap + btop( timemem ));

	*pte = (*pte & ~PG_PROT) | PG_URKW | PG_V;
	if(((int) timemem & PGOFSET)+sizeof(struct timeval) > NBPG) {
		/* have to map two pages */
		pte++;
		*pte = (*pte & ~PG_PROT) | PG_URKW | PG_V;
	}
}

/*ARGSUSED*/
mmioctl(dev, com, data, flag)
	register dev_t dev;
	caddr_t data;
{
	switch (com) {
	case MMGETLA: /* send current load avg to user */
		bcopy((caddr_t)avenrun, data, sizeof (struct _avenrun));
		break;
	case MMGETTIMEADR:
		* (caddr_t *) data = (caddr_t) &time;
		break;
	default:
		return (-1);
	}
	return (0);
}
#endif UW
mmrw(dev, uio, rw)
	dev_t dev;
	struct uio *uio;
	enum uio_rw rw;
{
	register int o;
	register u_int c, v;
	register struct iovec *iov;
	int error = 0;
	extern int umbabeg, umbaend;


	while (uio->uio_resid > 0 && error == 0) {
		iov = uio->uio_iov;
		if (iov->iov_len == 0) {
			uio->uio_iov++;
			uio->uio_iovcnt--;
			if (uio->uio_iovcnt < 0)
				panic("mmrw");
			continue;
		}
		switch (minor(dev)) {

/* minor device 0 is physical memory */
		case 0:
			v = btop(uio->uio_offset);
			if (v >= physmem)
				goto fault;
			*(int *)mmap = v | PG_V |
				(rw == UIO_READ ? PG_KR : PG_KW);
			mtpr(TBIS, vmmap);
			o = (int)uio->uio_offset & PGOFSET;
			c = (u_int)(NBPG - ((int)iov->iov_base & PGOFSET));
			c = MIN(c, (u_int)(NBPG - o));
			c = MIN(c, (u_int)iov->iov_len);
			error = uiomove((caddr_t)&vmmap[o], (int)c, rw, uio);
			continue;

/* minor device 1 is kernel memory */
		case 1:
			if ((caddr_t)uio->uio_offset < (caddr_t)&umbabeg &&
			    (caddr_t)uio->uio_offset + uio->uio_resid >= (caddr_t)&umbabeg)
				goto fault;
			if ((caddr_t)uio->uio_offset >= (caddr_t)&umbabeg &&
			    (caddr_t)uio->uio_offset < (caddr_t)&umbaend)
				goto fault;
			c = iov->iov_len;
			if (!kernacc((caddr_t)uio->uio_offset, c, rw == UIO_READ ? B_READ : B_WRITE))
				goto fault;
			error = uiomove((caddr_t)uio->uio_offset, (int)c, rw, uio);
			continue;

/* minor device 2 is EOF/RATHOLE */
		case 2:
			if (rw == UIO_READ)
				return (0);
			c = iov->iov_len;
			break;

/* minor device 3 is unibus memory (addressed by shorts) */
		case 3:
			c = iov->iov_len;
			if (!kernacc((caddr_t)uio->uio_offset, c, rw == UIO_READ ? B_READ : B_WRITE))
				goto fault;
			if (!useracc(iov->iov_base, c, rw == UIO_READ ? B_WRITE : B_READ))
				goto fault;
			error = UNIcpy((caddr_t)uio->uio_offset, iov->iov_base,
			    (int)c, rw);
			break;
		}
		if (error)
			break;
		iov->iov_base += c;
		iov->iov_len -= c;
		uio->uio_offset += c;
		uio->uio_resid -= c;
	}
	return (error);
fault:
	return (EFAULT);
}

/*
 * UNIBUS Address Space <--> User Space transfer
 */
UNIcpy(uniadd, usradd, n, rw)
	caddr_t uniadd, usradd;
	register int n;
	enum uio_rw rw;
{
	register short *from, *to;
 
	if (rw == UIO_READ) {
		from = (short *)uniadd;
		to = (short *)usradd;
	} else {
		from = (short *)usradd;
		to = (short *)uniadd;
	}
	for (n >>= 1; n > 0; n--)
		*to++ = *from++;
	return (0);
}
