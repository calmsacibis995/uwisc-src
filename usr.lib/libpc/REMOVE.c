/* Copyright (c) 1979 Regents of the University of California */

static char sccsid[] = "@(#)REMOVE.c 1.4 5/19/84";

#include "h00vars.h"

REMOVE(name, namlim)

	char			*name;
	long			namlim;
{
	register int	cnt;
	register int	maxnamlen = namlim;
	char		namebuf[NAMSIZ];

	/*
	 * trim trailing blanks, and insure that the name 
	 * will fit into the file structure
	 */
	for (cnt = 0; cnt < maxnamlen; cnt++)
		if (name[cnt] == '\0' || name[cnt] == ' ')
			break;
	if (cnt >= NAMSIZ) {
		ERROR("%s: File name too long\n", name);
		return;
	}
	maxnamlen = cnt;
	/*
	 * put the name into the buffer with null termination
	 */
	for (cnt = 0; cnt < maxnamlen; cnt++)
		namebuf[cnt] = name[cnt];
	namebuf[cnt] = '\0';
	/*
	 * unlink the file
	 */
	if (unlink(namebuf)) {
		PERROR("Could not remove ", namebuf);
		return;
	}
}
