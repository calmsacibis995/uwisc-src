/* Header for the AED device.
 */

/* Structure for returning the AED status.
 */
struct aedstatus
{
	int	aed_flags;	/* Control block flags. */
	int	aed_user;	/* User id. */
	int	aed_lock;	/* Locking process id. */
	int	aed_tics;	/* Watchdog timer tics. */
	int	aed_b_flags;	/* Buf structure flags. */
	int	aed_b_error;	/* Buf structure error code. */
	int	aed_cosi;	/* AED command/status register. */
	int	aed_iset;	/* AED interface setup register. */
	int	aed_ba;		/* AED bus address register. */
	int	aed_wc;		/* AED word count register. */
};

/* Ioctl function codes.
 */
#define AEDPSEUDO	_IO(a,3)	/* Enable pseudo interrupts. */
#define AEDNOPSEUDO	_IO(a,4)	/* Disable pseudo interrupts. */
#define AEDRESET	_IO(a,5)	/* Reset AED device. */
#define AEDSTARTED	_IO(a,6)	/* Wait until io begun. */
#define AEDDEBUG	_IO(a,7)	/* Enable debugging */
#define AEDNODEBUG	_IO(a,8)	/* Disable debugging */
#define AEDLOCK		_IO(a,9)	/* Lock access to AED. */
#define AEDUNLOCK	_IO(a,10)	/* Unlock the AED. */
#define AEDSTATUS	_IOR(a,11,struct aedstatus)	/* Get AED status. */
#define AEDSWAPBYTES	_IO(a,12)	/* Swap bytes during transfer. */
#define AEDNOSWAPBYTES	_IO(a,13)	/* No swap during transfer. */

/* Device registers for the AED-512.
 */
struct aeddevice
{
	short		aed_cosi;	/* Command out/Status in register. */
	short		aed_iset;	/* Interface setup register. */
	unsigned short	aed_ba;		/* UNIBUS address register. */
	short		aed_wc;		/* Transfer word count register. */
};

/* AED cosi register flags.
 */
#define	AED_ALWAYS_ON	0x0200	/* Always on ? */
#define	AED_READ	0x2000	/* Read flag. */
#define	AED_WRITE	0x0000	/* Write flag. */
#define	AED_BUSY	0x8000	/* AED busy flag. */

/* AED iset register flags.
 */
#define	AED_IENABLE	0x0040	/* Interrupt enable. */
#define	AED_SWAP_BYTES	0x0800	/* Swap bytes flag. */
#define	AED_XBA		0x3000	/* Extended address bits. */
#define	AED_BUS_TIMEOUT	0x4000	/* Bus timeout flag, non-existant memory. */

/* Format strings for displaying the AED device registers.
 */
#define AED_COSI_BITS "\20\20BUSY\14READ\12ALWAYS"
#define AED_ISET_BITS "\20\17NEX\14SWAP\7IENABLE"

/* Amount to shift the AED_XBA bits from the UBA map descriptor to the
 * AED iset register.
 */
#define	AED_SHIFT_XBA	4
