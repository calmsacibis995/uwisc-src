/* aed.c 1.1		2/84
 *
 * Driver for the AED-512.  This driver uses raw block transfer.
 *
 * The driver has a kludge because we've been unable to get the AED board to
 * interrupt on completion of a DMA transfer.  Essentially the watch dog
 * routine will cause a pseudo interrupt if it detects that the current
 * operation has completed (i.e. word count = 0).  This kludge can be disabled
 * via the ioctl routine.
 *
 * Another kludge.  Because it is necessary to start the io on the AED board
 * before the device on the other side (eg. VICOM) begins the transfer a wait
 * for io initiated is available via aedioctl.  The proper sequence for an
 * read is:
 *
 *	if (fork())
 *	{
 *		read(aedfd, buffer, length);
 *		wait(0);
 *	}
 *	else
 *	{
 *		if (ioctl(aedfd, AEDSTARTED, NULL) == 0)
 *			start the VICOM transfer via serial line;
 *		_exit;
 *	}
 *
 * Notice that there is no protection from two processes trying to perform
 * simultaneous operations.  Things could become quite confused although
 * the system should not deadlock.  To protect against this use the AEDLOCK
 * control call (see below).
 *
 * To allow a process to make several tranfers without interference the AED
 * can be locked.  To lock the AED driver use:
 *		rc = ioctl(aedfd, AEDLOCK, NULL)
 * If the AED is already locked EBUSY will be returned otherwise the AED will
 * be locked so that only this process may perform transfers. To unlock it:
 *		ioctl(aedfd, AEDUNLOCK, NULL)
 * AEDRESET also forces the AED to unlock and permits any user to open it.
 *
 *****************************************************************************
 * Written by Dayton Clark (CIMS) September 1983.
 * Modified November 1983 for 4.2bsd by Lou Salkind.
 * Modified Feburary 1984 by Dayton Clark:
 *	Allow multiple processes to open the AED.
 *	Add AEDLOCK & AEDUNLOCK ioctl commands.
 *	Improve AEDRESET ioctl command.
 */

#ifdef lint
#define KERNEL 1
#define NAED 1
#else
#include 	"aed.h"
#endif
#if NAED > 0

#include "../machine/pte.h"

#include "param.h"
#include "user.h"
#include "proc.h"
#include "buf.h"
#include "systm.h"
#include "map.h"
#include "ioctl.h"
#include "uio.h"
#include "kernel.h"
#include "../ufs/fsdir.h"

#include "../vaxuba/aedreg.h"
#include "../vaxuba/ubavar.h"
#include "../vaxuba/ubareg.h"

/* Routines defined.
 */
int		aedprobe();
int		aedattach();
int		aedopen();
int		aedclose();
int		aedioctl();
int		aedread();
int		aedwrite();
int		aedio();
int		aedstrategy();
unsigned	aedmincnt();
int		aedintr();
int		aedwatch();
int		aedinprogress();

/* Miscellaneous constants.
 */
#define	MAXBLK		(2*65536)	/* Maximum transfer size, in bytes */
#define	AED_ADDRESS	0764040		/* AED device register address. */
#define AED_VECTOR	0170		/* AED interrupt vector location. */
#define WATCH_TIC	8		/* Watch dog interval. */
#define TICLIMIT	80		/* # tics til timeout. */
#define AED_PRIORITY	(PZERO+4)	/* Priority when waiting for IO. */

/* AEDUNIT(dev) extracts unit # from device descriptor.
 */
#define	AEDUNIT(dev) (minor(dev))

/* A few necessities for UNIBUS devices.
 */
unsigned short		aedstd []	/* Standard device address. */
				= {AED_ADDRESS};
struct uba_device	*aedinfo[NAED];	/* Device structures. */
struct uba_driver	aeddriver	/* Driver structure. */
				= {aedprobe, 0, aedattach, 0,
					aedstd, "aed", aedinfo};

/* Miscellaneous.
 */
static int	aed_debug = 0;		/* Debug print flag. */
static int	aedwatch_active = 0;	/* Watch dog routine active flag. */
static int	aed_priviledged_users [] = {90, 0};
				/* Users who can always open the AED. 90=tom@ai */

/* Device software control blocks for the AED-512's.
 */
struct aed_softc
{
	int			sc_flags;   /* NEW */
	int			sc_user;	/* User id. */
	int			sc_lock;	/* Access lock (process). */
	int			sc_ubinfo;	/* UNIBUS addr. user buf. */
	int			sc_tics;	/* Watch dog counter. */
	struct buf		sc_buf;		/* Buffer header. */
	struct aeddevice	sc_reg;		/* Copy of AED registers. */
};
static struct aed_softc aed_softc [NAED];

/* Control block flags.
 */
#define	SC_IN_PROGRESS	0x0001	/* Operation in progress. */
#define SC_OPENED	0x0002	/* Device opened. */
#define SC_BUSY		0x0008	/* Control block is in use. */
#define SC_STARTED	0x0020	/* Operation has been started. */
#define SC_CANCELLED	0x0040	/* Operation has been cancelled. */
#define SC_PSEUDO_INTR	0x0100	/* Non-interrupt operation kludge. */
#define SC_SWAP_BYTES	0x0200	/* Swap bytes on transfer. */
#define SC_NEX_MEMORY	0x2000	/* Non-existant memory. */
#define	SC_TIMED_OUT	0x4000	/* Operation timed out. */
#define	SC_ERROR	0x8000	/* Error in operation. */

/* Format string for the sc_flags.
 */
#define AED_SC_FLAGS_BITS "\20\20ERROR\17TIMEOUT\16NEX\12SWAP\11PSEINTR\7CANCELLED\6STARTED\4BUSY\2OPENED\1INPROGRESS"

/* Aedprobe(reg) sets the AED boards interrupt vector and returns 1
 * to indicate that the AED is available.  Reg points to the AED
 * device registers.
 */
/* ARGSUSED */
aedprobe(reg)
	caddr_t		reg;
{
	register int	br, cvec;	/* must be like this, don't touch. */

	if (aed_debug)
		printf("entered aedprobe()\n");

#ifdef lint
	br = 0; cvec = br; br = cvec;
#endif
	/* Assume the vector is at AED_VECTOR.
	 * This can perhaps be fixed, though I doubt it.
	 */
	cvec = AED_VECTOR;
	br = 0x15;

	if (aed_debug)
		printf("exited aedprobe()\n");
	return(1);
}


/* Aedattach(ui) performs some initialization.
 */
aedattach(ui)
	struct uba_device		*ui;
{
	register struct aed_softc	*sc;
	register struct aeddevice	*aedaddr;

	if (aed_debug)
		printf("entered aedattach()\n");

	sc = &aed_softc [ui->ui_unit];
	aedaddr = (struct aeddevice *) ui->ui_addr;

	/* Clear the control block fields.
	 * Clear the device registers and control block copies.
	 */
	sc->sc_flags = 0;
	sc->sc_user = 0;
	sc->sc_lock = 0;
	sc->sc_ubinfo = 0;
	sc->sc_tics = 0;
	aedaddr->aed_cosi = sc->sc_reg.aed_cosi = 0;
	aedaddr->aed_ba   = sc->sc_reg.aed_ba   = 0;
	aedaddr->aed_iset = sc->sc_reg.aed_iset = 0;
	aedaddr->aed_wc   = sc->sc_reg.aed_wc   = 0;

	if (aed_debug)
		printf("exited aedattach()\n");
	
}

/* Aedopen(dev, flag) opens the AED device. Dev indicates the device to be
 * opened, flag is the read/write flag and is currently ignored.  Aedopen
 * first checks that the unit number is ok then it checks that this user
 * can open the device (i.e. the open doesn't conflict with another user).
 * If all is ok the device is opened and the watchdog timer is started.
 */
/* ARGSUSED */
aedopen(dev, flag)
	dev_t	dev;
	int	flag;
{
	register int			unit;
	register struct aed_softc	*sc;
	register struct uba_device	*ui;
	int				uid;
	int				rc;

	/* If the unit # is out of range or there is no uba_device
	 * exit with ENXIO error.
	 * If it's not opened then open it.
	 * If it's already opened but ok for this user to reopen it, OK.
	 * Otherwise return EBUSY.
	 */
	if (aed_debug)
		printf("entered aedopen(), flag = %x\n",flag);

	unit = AEDUNIT(dev);
	if     (unit >= NAED ||
		(ui = aedinfo[unit]) == 0 ||
		ui->ui_alive == 0)

		rc = ENXIO;
	else
	{
		sc = &aed_softc[unit];
		uid = u.u_uid;
		if     ((sc->sc_flags & SC_OPENED) == 0)
		{
			sc->sc_user = uid;
			sc->sc_flags = SC_OPENED;
			if (aedwatch_active == 0) aedwatch((caddr_t) 0);
			rc = 0;
		}
		else if ((sc->sc_user == uid) || aed_priviledged_user(uid))

			rc = 0;
		else
			rc = EBUSY;
	}
	if (aed_debug)
		printf("exiting aedopen() with rc = %x\n",rc);

	return(rc);
}

/* Aed_priviledged_user(uid) returns 1 if the user indicated by uid is a
 * priviledged user for the AED.
 */
aed_priviledged_user(uid)
	int	uid;
{
	int	*puid;

	puid = aed_priviledged_users;
	do
		if (*puid == uid) return(1);
		while (*puid++ != 0);
	return(0);
}

/* Aedclose(dev, flags) closes the AED-512 unit specified by  dev.  Flags is
 * currently ignored.
 */
/* ARGSUSED */
aedclose(dev, flag)
	dev_t				dev;
	int				flag;
{
	register int			unit;
	register struct aed_softc	*sc;
	register struct uba_device	*ui;
	register struct aeddevice	*aedaddr;

	if (aed_debug)
		printf("entered aedclose() flag = %x\n",flag);

	unit = AEDUNIT(dev);
	sc = &aed_softc[unit];
	ui = aedinfo[unit];
	aedaddr = (struct aeddevice *) ui->ui_addr;

	/* Clear everything.
	 */
	sc->sc_flags = 0;
	sc->sc_user = 0;
	sc->sc_lock = 0;
	sc->sc_ubinfo = 0;
	sc->sc_tics = 0;
	aedaddr->aed_cosi = sc->sc_reg.aed_cosi = 0;
	aedaddr->aed_ba   = sc->sc_reg.aed_ba   = 0;
	aedaddr->aed_iset = sc->sc_reg.aed_iset = 0;
	aedaddr->aed_wc   = sc->sc_reg.aed_wc   = 0;

	if (aed_debug)
		printf("exiting aedclose()\n");
}

/* Aedioctl(dev, cmd, data, flag) performs the io control functions.  Dev is
 * the device code, cmd is the desired command code (from aedreg.h),
 * data points to the area to receive the status for the AEDSTATUS command.
 * Flag is not currently used.
 */
/*ARGSUSED*/
aedioctl(dev, cmd, data, flag)
	dev_t				dev;
	int				cmd;
	caddr_t				data;
	int 				flag;
{
	register struct aed_softc	*sc;
	register struct uba_device	*ui;
	register struct aeddevice	*aedaddr;
	struct buf			*bp;
	int				unit;
	struct aedstatus		*sp;
	int				s;
	int				rc;

	if (aed_debug)
		printf("Aedioctl(%x, %d, %x, %d)\n", dev, cmd, data, flag);

	unit = AEDUNIT(dev);
	sc = &aed_softc [unit];
	bp = &sc->sc_buf;
	ui = aedinfo [unit];
	aedaddr = (struct aeddevice *) ui->ui_addr;
	rc = 0;

	switch(cmd) {

	case AEDPSEUDO:

		/* Enable pseudo interrupts.
		 */
		sc->sc_flags |= SC_PSEUDO_INTR;
		break;

	case AEDNOPSEUDO:

		/* Disable pseudo interrupts.
		 */
		sc->sc_flags &= ~SC_PSEUDO_INTR;
		break;

	case AEDRESET:

		/* AEDRESET does the following:
		 * 	Clear the AED's device registers.
		 *	Wakeup processes that may be waiting anyplace
		 *		associated with this device.
		 *	Unlock the device.
		 *
		 * This is a pretty brutal reset and is intended for
		 * emergency use.  It may have to be repeated several
		 * times to clear everything out of the system.
		 * NOTE: once a reset has been done the device must
		 * be opened again before operations can be performed.
		 */
		aedaddr->aed_cosi = sc->sc_reg.aed_cosi = 0;
		aedaddr->aed_ba   = sc->sc_reg.aed_ba   = 0;
		aedaddr->aed_iset = sc->sc_reg.aed_iset = 0;
		aedaddr->aed_wc   = sc->sc_reg.aed_wc   = 0;
		sc->sc_user = 0;
		sc->sc_lock = 0;
		sc->sc_flags = SC_CANCELLED;
		wakeup((caddr_t) sc);
		wakeup((caddr_t) &(sc->sc_reg));
		break;

	case AEDSTARTED:

		/* If the AED is locked or not opened, error exit.
		 * Wait until SC_STARTED is set.
		 */
		if ((sc->sc_lock) && (sc->sc_lock != u.u_procp->p_pid))
			rc = EBUSY;
		else
		{
			s = spl5();		/* ********CRITICAL SECTION */
			while ((sc->sc_flags &
				(SC_STARTED | SC_CANCELLED)) == 0)

				sleep((caddr_t) &(sc->sc_reg), AED_PRIORITY);

			if (sc->sc_flags & SC_CANCELLED) rc = EIO;
			sc->sc_flags &= ~(SC_STARTED | SC_CANCELLED);
			splx(s);		/* ****END CRITICAL SECTION */
		}
		break;

	case AEDDEBUG:

		/* Turn on the debug flag.
		 */
		aed_debug = 1;
		break;

	case AEDNODEBUG:

		/* Turn off the debug flag.
		 */
		aed_debug = 0;
		break;

	case AEDLOCK:

		/* If the device is already locked return EBUSY.
		 * Otherwise lock the AED.
		 */
		if ((sc->sc_lock) && (sc->sc_lock != u.u_procp->p_pid))
			rc = EBUSY;
		else
			sc->sc_lock = u.u_procp->p_pid;
		break;

	case AEDUNLOCK:

		/* If this is the process that has the AED locked then
		 * unlock it otherwise return EBUSY.
		 */
		if ((sc->sc_lock) && (sc->sc_lock != u.u_procp->p_pid))
			rc = EBUSY;
		else
			sc->sc_lock = 0;
		break;

	case AEDSTATUS:

		/* Report the status of the AED.
		 */
		sp = (struct aedstatus *) data;
		sp->aed_flags = sc->sc_flags;
		sp->aed_user = sc->sc_user;
		sp->aed_lock = sc->sc_lock;
		sp->aed_tics = sc->sc_tics;
		sp->aed_b_flags = sc->sc_buf.b_flags;
		sp->aed_b_error = sc->sc_buf.b_error;
		sp->aed_cosi = sc->sc_reg.aed_cosi;
		sp->aed_iset = sc->sc_reg.aed_iset;
		sp->aed_ba = sc->sc_reg.aed_ba;
		sp->aed_wc = sc->sc_reg.aed_wc;
		break;

	case AEDSWAPBYTES:

		/* Set the swap bytes flag.
		 */
		sc->sc_flags |= SC_SWAP_BYTES;
		break;

	case AEDNOSWAPBYTES:

		/* Clear the swap bytes flag.
		 */
		sc->sc_flags &= ~SC_SWAP_BYTES;
		break;

	default:
		rc = ENOTTY;
	}
	if (aed_debug)
		printf("exiting aedioctl with rc = %x\n",rc);

	return(rc);
}

/* Aedread(dev) performs a DMA read using dev.
 */
aedread(dev, uio)
	dev_t		dev;
	struct uio	*uio;
{
	return(aedio(dev, uio, B_READ));
}

/* Aedwrite(dev) performs a DMA write using dev.
 */
aedwrite(dev, uio)
	dev_t		dev;
	struct uio	*uio;
{
	return(aedio(dev, uio, B_WRITE));
}

/* Aedio(dev, uio, rw) performs the read or write. Rw indicates the direction
 * of the transfer, either B_READ or B_WRITE.
 */
aedio(dev, uio, rw)
	dev_t		dev;
	struct	uio	*uio;
	int		rw;
{
	register struct buf		*bp;
	register struct aed_softc	*sc;
	int				unit;
	int				rc;
	int				s;

	if (aed_debug) printf ("aedio(%x, %x, %x)\n", dev, uio, rw);

	unit = AEDUNIT(dev);
	sc = &aed_softc [unit];
	bp = &sc->sc_buf;

	/* If the open flag is off, most likely someone has done AEDRESET.
	 * Skip all operations until this is reopened.
	 */
	if ((sc->sc_flags & SC_OPENED) == 0) return (EBADF);

	/* Wait until any previous IO is completed.
	 * Turn off the various completion flags.
	 * If the control block is locked by another process cancel
	 * the operation and return EBUSY.
	 * Otherwise, perform the IO, this won't return until IO is complete.
	 * If the IO was not successful, display message for the user.
	 * Release the control block and notify anyone waiting for it.
	 */
	s = spl5();				/* ********CRITICAL SECTION */
	while (sc->sc_flags & SC_BUSY) sleep((caddr_t) sc, AED_PRIORITY);
	sc->sc_flags |= SC_BUSY;
	splx(s);				/* ****END CRITICAL SECTION */

	sc->sc_flags &= ~(SC_NEX_MEMORY | SC_TIMED_OUT | SC_ERROR);
	if ((sc->sc_lock) && (sc->sc_lock != u.u_procp->p_pid))
	{
		sc->sc_flags |= SC_CANCELLED;
		wakeup((caddr_t) &(sc->sc_reg));
		rc = EBUSY;
	}
	else
		rc = physio(aedstrategy, bp, dev, rw, aedmincnt, uio);

	if (sc->sc_flags & SC_ERROR)
	{
		uprintf("AED unit %d error:\n   flags=%b\n",
			unit, sc->sc_flags, AED_SC_FLAGS_BITS);
		uprintf("  registers:\n    cosi=%b\n    iset=%b\n",
			sc->sc_reg.aed_cosi, AED_COSI_BITS,
			sc->sc_reg.aed_iset, AED_ISET_BITS);
		uprintf("    ba=%x wc=%x\n",
			sc->sc_reg.aed_ba, sc->sc_reg.aed_wc);
	}

	sc->sc_flags &= ~SC_BUSY;
	wakeup((caddr_t) sc);
	if (aed_debug)
		printf("exited aedio() with rc = %x\n",rc);
	return(rc);
}

/* Aedstrategy(bp) performs a DMA transfer using buffer pointed to by bp.
 */
aedstrategy(bp)
	register struct buf		*bp;
{
	register struct aed_softc	*sc;
	register struct uba_device	*ui;
	register struct aeddevice	*aedaddr;
	int				unit;

	if (aed_debug) printf("aedstrategy(%x)\n", bp);

	unit = AEDUNIT(bp->b_dev);
	ui = aedinfo[unit];
	sc = &aed_softc[unit];
	aedaddr = (struct aeddevice *) ui->ui_addr;

	/* Clear the watch dog counter.
	 * Set the control block pointer to the buffer.
	 * Get the necessary UNIBUS map registers.
	 */
	sc->sc_tics = 0;
	if (aed_debug)
		printf("going to do ubasetup()\n");
	sc->sc_flags |= SC_IN_PROGRESS;
#ifdef UW
	/* changed from UBA_NEEDBDP because there aren't enough
	 * of them and it will *never* come back if it just
	 * sits there and waits for it.
	 */
	sc->sc_ubinfo = ubasetup(ui->ui_ubanum, bp, UBA_LIKEBDP|UBA_CANTWAIT);
#else UW
	sc->sc_ubinfo = ubasetup(ui->ui_ubanum, bp, UBA_NEEDBDP);
#endif UW

	if (aed_debug)
		printf("done ubasetuo()\n");


	/* Load the device register and the sc's copies to start the
	 * transfer.
	 *	ba	UNIBUS address of the user's buffer.
	 *	cosi	AED_ALWAYS_ON plus the read/write flag from
	 *		bp->b_flags.
	 *	iset	AED_IENABLE (if SC_PSEUDO_INTR not set) and
	 *		the extended bus address bits shifted into the
	 *		proper position.
	 *	wc	The # of words to transfer, negated. (NOTE: loading
	 *		the wc register initiates the transfer.)
	 */
	aedaddr->aed_cosi = sc->sc_reg.aed_cosi = AED_ALWAYS_ON |
				((bp->b_flags & B_READ) ?
						AED_READ : AED_WRITE);
	aedaddr->aed_ba   = sc->sc_reg.aed_ba   =
					(unsigned short) sc->sc_ubinfo;
	aedaddr->aed_iset = sc->sc_reg.aed_iset =
		((sc->sc_flags & SC_PSEUDO_INTR) ? 0 : AED_IENABLE) |
		((sc->sc_flags & SC_SWAP_BYTES) ? AED_SWAP_BYTES : 0) |
		((sc->sc_ubinfo >> AED_SHIFT_XBA) & AED_XBA);
	aedaddr->aed_wc   = sc->sc_reg.aed_wc   =
					- (short) (bp->b_bcount >> 1);

	/* Signal that io has been initiated.  A co-process may be waiting
	 * on an AEDSTARTED command.
	 */
	sc->sc_flags |= SC_STARTED;
	wakeup((caddr_t) &(sc->sc_reg));

	if (aed_debug)
		printf("exited aedstrategy\n");

}

unsigned
aedmincnt(bp)
	struct buf *bp;
{
	if (bp->b_bcount > MAXBLK) bp->b_bcount = MAXBLK;
}

/* Aedintr(UNIT) is the interrupt service routine.  Unit is the number
 * of the unit causing the interrupt.
 */
aedintr(unit)
	int				unit;
{
	register struct aed_softc	*sc;
	register struct uba_device	*ui;
	register struct aeddevice	*aedaddr;
	struct buf			*bp;

	if (aed_debug) printf("aedintr(%d)\n", unit);

	sc = &aed_softc[unit];
	bp = &sc->sc_buf;
	ui = aedinfo[unit];
	aedaddr = (struct aeddevice *) ui->ui_addr;

	/* Get a copy of the device registers for safe keeping and for
	 * future reference.
	 * Then turn the device off.
	 * Clear the SC_IN_PROGRESS flag in the control block.
	 */
	sc->sc_reg.aed_cosi = aedaddr->aed_cosi;
	sc->sc_reg.aed_iset = aedaddr->aed_iset;
	sc->sc_reg.aed_ba   = aedaddr->aed_ba;
	sc->sc_reg.aed_wc   = aedaddr->aed_wc;
	aedaddr->aed_cosi = 0;
	aedaddr->aed_iset = 0;
	aedaddr->aed_ba   = 0;
	aedaddr->aed_wc   = 0;
	sc->sc_flags &= ~SC_IN_PROGRESS;

	/* Before we complete the io let's find out what has happened.
	 * Check for possible errors found in the iset register.
	 * If there's been an errors then set the buffer's error flag.
	 * NOTE that errors may have been detected elsewhere particularly
	 * by the watchdog routine.
	 * Release the UNIBUS map registers.
	 */
	if (sc->sc_reg.aed_iset & AED_BUS_TIMEOUT)
		sc->sc_flags |= SC_ERROR | SC_NEX_MEMORY;
	if (sc->sc_flags & SC_ERROR) bp->b_flags |= B_ERROR;

	ubarelse(ui->ui_ubanum, &sc->sc_ubinfo);
	biodone(bp);

	if (aed_debug) 
		printf("exited aedintr()\n");
}

/* Aedwatch(dum) is the watch dog routine.  It's original purpose is to
 * insure that an AED io request doesnt get hung up because of a lost
 * interrupt.  However, because we can't get the AED board to interrupt
 * on CSD1, aedwatch serves the additional function of watching for
 * completion and generating pseudo interrupts.
 */
/* ARGSUSED */
aedwatch(dum)
	caddr_t				dum;
{
	register struct aed_softc	*sc;
	int				unit;
	int				nopened = 0;	/* # of open AED's. */

	/* For each AED unit:
	 * 	If it's not open then ignore it.
	 *	Otherwise count it and check for io in progress.
	 *	If there is io in progress then handle it.
	 */

#ifdef notdef
	if (aed_debug)
		printf("entered aedwatch\n");
#endif

	for (unit=0; unit<NAED; unit++)
	{
		sc = &aed_softc[unit];
#ifdef notdef
		if (aed_debug)
		{
			printf("aedwatch: nopened = %d\n",nopened);
			printf("aedwatch: sc->sc_flags = 0x%x\n",sc->sc_flags);
		}
#endif
		if ((sc->sc_flags & SC_OPENED))
		{
			nopened++;
#ifdef notdef
			if (aed_debug)
			{
				printf("aedwatch: nopened = %d\n",nopened);
				printf("aedwatch: sc->sc_flags = 0x%x\n",sc->sc_flags);
			}
#endif
			if (sc->sc_flags & SC_IN_PROGRESS)
				aedinprogress(unit, sc);
		}
	}

	/* If there's any open devices, restart the watch dog
	 * otherwise shut it down.
	 */
	if (nopened)
	{
#ifdef notdef
		if (aed_debug)
			printf("aedwatch: calling timeout\n");
#endif
		timeout(aedwatch, (caddr_t) 0, WATCH_TIC);
		aedwatch_active = 1;
	}
	else
	{
		aedwatch_active = 0;
#ifdef notdef
		if (aed_debug)	
			printf("aedwatch: setting aedwatch_active to 0\n");
#endif
	}
	
#ifdef notdef
	if (aed_debug)
		printf("exited aedwatch()\n");
#endif
}

/* Aedinprogress(unit, sc) takes care of io in progress for the watchdog
 * routine.  Unit is the AED unit number and sc points to the software
 * control block.
 */
aedinprogress(unit, sc)
	int				unit;
	register struct aed_softc	*sc;
{
	register struct aeddevice	*aedaddr;
	struct uba_device		*ui;
	int s;				/* Hold processor priority. */

	/* Get the uba info for this device then grab a copy of the
	 * device registers for safe keeping & future reference.
	 */

	if (aed_debug)
		printf("entered aedinprogress()\n");

	ui = aedinfo[unit];
	aedaddr = (struct aeddevice *) ui->ui_addr;
	sc->sc_reg.aed_cosi = aedaddr->aed_cosi;
	sc->sc_reg.aed_iset = aedaddr->aed_iset;
	sc->sc_reg.aed_ba   = aedaddr->aed_ba;
	sc->sc_reg.aed_wc   = aedaddr->aed_wc;

	/* Check for a timeout.  If we've exceeded the TICLIMIT then
	 * set the appropiate flags.
	 */
	if (++(sc->sc_tics) > TICLIMIT)
		sc->sc_flags |= SC_ERROR | SC_TIMED_OUT;

	/* Now should we fake an interrupt?  There are two reasons to do so.
	 * One is that pseudo interrupts have been requested and the io
	 * operation has completed (a kludge to get this to work on CSD1).
	 * The other is that a timeout has been detected,  the fake interrupt
	 * should clean up the mess.
	 * The processor priority is raised for calling the interrupt so
	 * that it feels at home.
	 */
	if (((sc->sc_flags & SC_PSEUDO_INTR) && (sc->sc_reg.aed_wc == 0)) ||
		(sc->sc_flags & SC_TIMED_OUT))
	{
		s = spl5();			/* ********CRITICAL SECTION */
		aedintr(unit);
		splx(s);			/* ****END CRITICAL SECTION */
	}

	if (aed_debug)
		printf("exited aedinprogress()\n");
}
#endif	NAED
