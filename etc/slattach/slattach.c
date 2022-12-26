#ifndef lint
static char sccsid[] = "@(#)slattach.c	4.1 (Berkeley) 2/17/86";
#endif
#include <stdio.h>
#include <sys/param.h>
#include <sgtty.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <net/if.h>
#include <netdb.h>
#include <fcntl.h>

#ifndef lint
static char rcsid[] = "$Header: slattach.c,v 1.1 86/08/26 16:35:37 root Exp $";
#endif

#define DEFAULT_BAUD	9600
int	speed;
int	slipdisc = SLIPDISC;

char	devname[32];
char	hostname[MAXHOSTNAMELEN];

extern int errno;

main(argc, argv)
	int argc;
	char *argv[];
{
	register FILE *fp;
	register int fd;
	register char *dev = argv[1];
	struct sgttyb sgtty;
	int n;

	if (argc < 2 || argc > 3) {
		fprintf(stderr, "usage: %s ttyname [baudrate]\n", argv[0]);
		exit(1);
	}
	speed = argc == 3 ? findspeed(atoi(argv[2])) : findspeed(DEFAULT_BAUD);
	if (speed == 0) {
		fprintf(stderr, "unknown speed %s", argv[2]);
		exit(1);
	}
	if (strncmp("/dev/", dev, 5)) {
		sprintf(devname, "/dev/%s", dev);
		dev = devname;
	}
	if ((fd = open(dev, O_RDWR | O_NDELAY)) < 0) {
		perror(dev);
		exit(1);
	}
	sgtty.sg_flags = RAW | ANYP;
	sgtty.sg_ispeed = sgtty.sg_ospeed = speed;
	if (ioctl(fd, TIOCSETP, &sgtty) < 0) {
		perror("ioctl(TIOCSETP)");
		exit(1);
	}
	if (ioctl(fd, TIOCSETD, &slipdisc) < 0) {
		perror("ioctl(TIOCSETD)");
		exit(1);
	}

	if (fork() > 0)
		exit(0);
	for (;;)
		sigpause(0);
}

struct sg_spds {
	int sp_val, sp_name;
}       spds[] = {
#ifdef B50
	{ 50, B50 },
#endif
#ifdef B75
	{ 75, B75 },
#endif
#ifdef B110
	{ 110, B110 },
#endif
#ifdef B150
	{ 150, B150 },
#endif
#ifdef B200
	{ 200, B200 },
#endif
#ifdef B300
	{ 300, B300 },
#endif
#ifdef B600
	{ 600, B600 },
#endif
#ifdef B1200
	{ 1200, B1200 },
#endif
#ifdef B1800
	{ 1800, B1800 },
#endif
#ifdef B2000
	{ 2000, B2000 },
#endif
#ifdef B2400
	{ 2400, B2400 },
#endif
#ifdef B3600
	{ 3600, B3600 },
#endif
#ifdef B4800
	{ 4800, B4800 },
#endif
#ifdef B7200
	{ 7200, B7200 },
#endif
#ifdef B9600
	{ 9600, B9600 },
#endif
#ifdef EXTA
	{ 19200, EXTA },
#endif
#ifdef EXTB
	{ 38400, EXTB },
#endif
	{ 0, 0 }
};

findspeed(speed)
	register int speed;
{
	register struct sg_spds *sp;

	sp = spds;
	while (sp->sp_val && sp->sp_val != speed)
		sp++;
	return (sp->sp_name);
}
