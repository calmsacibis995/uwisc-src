#ifndef lint
static char	*RcsId = "$Header: usr2400.c,v 1.1 86/08/31 15:27:55 root Exp $";
/* From:  @(#)USR.c	4.4 (Berkeley) 10/10/85 */
#endif !lint

#include "../condevs.h"

#ifdef USR2400
#define DROPDTR

/*
 *	USRpopn(telno, flds, dev) connect to USR2400 smartmodem (pulse call)
 *	USRtopn(telno, flds, dev) connect to USR2400 smartmodem (tone call)
 *	char *flds[], *dev[];
 *
 *	return codes:
 *		>0  -  file number  -  ok
 *		CF_DIAL,CF_DEVICE  -  failed
 */

USRpopn(telno, flds, dev)
char *telno, *flds[];
struct Devices *dev;
{
	return USRopn(telno, flds, dev, 0);
}

USRtopn(telno, flds, dev)
char *telno, *flds[];
struct Devices *dev;
{
	return USRopn(telno, flds, dev, 1);
}

/* ARGSUSED */
USRopn(telno, flds, dev, toneflag)
char *telno;
char *flds[];
struct Devices *dev;
int toneflag;
{
	extern errno;
	char dcname[20];
	char cbuf[MAXPH];
	register char *cp;
	register int i;
	int dh = -1, nrings = 0;

	sprintf(dcname, "/dev/%s", dev->D_line);
	DEBUG(4, "dc - %s\n", dcname);
	if (setjmp(Sjbuf)) {
		logent(dcname, "TIMEOUT");
		if (dh >= 0)
			close(dh);
		return CF_DIAL;
	}
	signal(SIGALRM, alarmtr);
	getnextfd();
	alarm(10);
	dh = open(dcname, 2); /* read/write */
	alarm(0);

	/* modem is open */
	next_fd = -1;
	if (dh >= 0) {
		fixline(dh, dev->D_speed);
		if (dochat(dev, flds, dh)) {
			logent(dcname, "CHAT FAILED");
			close(dh);
			return CF_DIAL;
		}
		write(dh, "ATV1E0H\r", 8);
		if (expect("OK\r\n", dh) != 0) {
			logent(dcname, "HSM seems dead");
			USRcls(dh);
			return CF_DIAL;
		}
		write(dh, "ATX6S7=44\r", 10);
		if (expect("OK\r\n", dh) != 0) {
			logent(dcname, "HSM seems dead");
			USRcls(dh);
			return CF_DIAL;
		}
		if (toneflag)
			write(dh, "\rATDT", 5);
		else
			write(dh, "\rATD", 4);
		write(dh, telno, strlen(telno));
		write(dh, "\r", 1);

		if (setjmp(Sjbuf)) {
			logent(dcname, "TIMEOUT");
			strcpy(devSel, dev->D_line);
			USRcls(dh);
			return CF_DIAL;
		}
		signal(SIGALRM, alarmtr);
		alarm(2*MAXMSGTIME);
		do {
			cp = cbuf;
			while (read(dh, cp ,1) == 1)
				if (*cp >= ' ')
					break;
			while (++cp < &cbuf[MAXPH] && read(dh, cp, 1) == 1 && *cp != '\n')
				;
			alarm(0);
			*cp-- = '\0';
			if (*cp == '\r')
				*cp = '\0';
			DEBUG(4,"\nGOT: %s", cbuf);
			alarm(MAXMSGTIME);
		} while (strncmp(cbuf, "RING", 4) == 0 && nrings++ < 5);
		if (strncmp(cbuf, "CONNECT", 7) != 0) {
			logent(cbuf, _FAILED);
			strcpy(devSel, dev->D_line);
			USRcls(dh);
			return CF_DIAL;
		}
		i = atoi(&cbuf[8]);
		if (i > 0 && i != dev->D_speed) {	
			DEBUG(4,"Baudrate reset to %d\n", i);
			fixline(dh, i);
		}

	}
	if (dh < 0) {
		logent(dcname, "CAN'T OPEN");
		return dh;
	}
	DEBUG(4, "USR2400 ok\n", CNULL);
	return dh;
}

USRcls(fd)
int fd;
{
	char dcname[20];
#ifdef DROPDTR
	struct sgttyb hup, sav;
#endif

	if (fd > 0) {
		sprintf(dcname, "/dev/%s", devSel);
		DEBUG(4, "Hanging up fd = %d\n", fd);
#ifdef DROPDTR
		/*
		 * code to drop DTR -- change to 0 baud then back to default.
		 */
		gtty(fd, &hup);
		gtty(fd, &sav);
		hup.sg_ispeed = B0;
		hup.sg_ospeed = B0;
		stty(fd, &hup);
		sleep(2);
		stty(fd, &sav);
		/*
		 * now raise DTR -- close the device & open it again.
		 */
		sleep(2);
		close(fd);
		sleep(2);
		fd = open(dcname, 2);
		stty(fd, &sav);
#else
		sleep(3);
		write(fd, "+++", 3);
#endif
		sleep(3);
		write(fd, "ATZ\r", 4);
		if (expect("OK",fd) != 0)
			logent(devSel, "HSM did not respond to ATZ");
		write(fd, "ATH\r", 4);
		sleep(1);
		close(fd);
		delock(devSel);
	}
}
#endif USR2400
