/*	dr.c	UW lsi-11 reset dr driver. NOT STANDARD */

#include "dr.h"
#if NDR > 0
/*
 *	
 *  DR11C driver used for break box (11/1/82)
 *	this is NOT a generic DR driver (it ignores the status word
 *	and it looks at input line for status)
 *
 */

#include "param.h"
#include "systm.h"
#include "ioctl.h"
#include "tty.h"
#include "../machine/pte.h"
#include "map.h"
#include "uio.h"
#include "buf.h"
#include "../vaxuba/ubareg.h"
#include "../vaxuba/ubavar.h"
#include "conf.h"
#include "user.h"
#include "proc.h"
#include "../ufs/fsdir.h"

#undef DEBUG

#define DRREADY	1
#define DRBUSY	0
#define TICK	1

int	drbusy = 0;	/* owners pid, set on open */

extern	drprobe(),drslave(),drattach(),dropen(),drclose(),
	drwrite(),drintr(); /* defined */
extern	wakeup(); /* used */
struct	uba_device *drdinfo[NDR];

struct drdevice {
	u_short	sr; /* ignored */
	u_short	obuf;
	u_short	ibuf;
};

#define drstd (u_short)(0167770)

struct	uba_driver drdriver = {
    drprobe,
    drslave, 
    drattach, 
    0, 
    drstd, /* device csr addresses */
    "dr", /* name of device */
    drdinfo, /* backpointer to ubdinit structs */
    0, /* name of controller */
    0, /* backpointer to ubminit structs */
    0  /*  exclusive use of bdp */
};

int drfound = 1;

drprobe()
{
	register	br;
	register	cvec;
	int		rv;

	/*
	 *	fake interrupt
	 *
	 *	Only allow 1 (one!) dr 
	 */

	cvec = 0360;			/* fake interrupt vector */
	br = 0x15;			/* fake priority level */

	rv = drfound;
	drfound = 0;
	return(rv);
}

drslave(ui, reg)
register struct uba_device *ui;
register caddr_t reg;
{
	if(ui->ui_slave) 
		return(0);
	return(1);
}

drattach()
{
}

#define NUMTRIES 60
drwait()
{
	register struct drdevice *draddr =
		(struct drdevice *)drdinfo[0]->ui_addr;
	register count;
	register oldpri;

	for(count = NUMTRIES; count; count--) {

#		ifdef DEBUG
		printf("drwait: count = %d; draddr=0x%x; draddr->ibuf= 0x%x\n",
				count, (int)draddr, draddr->ibuf);
#		endif DEBUG

		if (draddr->ibuf & DRREADY) {
			return(0);
		} else { 
			/* wait a bit & try again */ 
			oldpri = spl6(); 
			timeout(wakeup, &(drbusy), TICK); 
			sleep( &(drbusy), PRIBIO); splx(oldpri); 
		} 
	} 
	return(EIO); 
} 

dropen()
{
	register int oldpri;

	oldpri = spl6();
	if(drbusy) {
		splx(oldpri);
		return(EBUSY);
	}

	drbusy = u.u_procp->p_pid; /* need unique number for sleeping */
	splx(oldpri);
	return(0);
}

drclose()
{
	return(drbusy = 0);
}

drwrite(dev,uio)
dev_t		dev;
struct uio *uio;
{
	register struct iovec	*iov = uio->uio_iov;
	register struct drdevice *draddr =
		(struct drdevice *)drdinfo[minor(dev)]->ui_addr;
	char charbuf[2];
	register int err=0;

#	ifdef DEBUG
	printf("drwrite %d %x\n", iov->iov_len, draddr);
#	endif DEBUG

	/* Fill 16 bit buffer with 1 or 2 bytes */

#	ifdef DEBUG
	printf("drwrite: iov->iov_base is: %x; (%o)(%o)\n",
    	iov->iov_base,((char *)iov->iov_base)[0],((char *)iov->iov_base)[1]);
#	endif DEBUG

	charbuf[0] = *iov->iov_base++;
	if (--iov->iov_len) {
		charbuf[1] = *iov->iov_base++;
		iov->iov_len--;
	} else {
		charbuf[1] = 0;
	}
	if( !(err=drwait()) ) {
#		ifdef DEBUG
		printf("drwrite: write to address 0x%x\n",
			&(draddr->obuf));
#		endif DEBUG
		draddr->obuf =  *((short *)charbuf);
	}
	uio->uio_resid -= 2;
	return(err);
}

drintr()
{
#ifdef DEBUG
	printf("drintr: stray interrupt\n");
#endif DEBUG
}

#endif
