/*
 * Copyright (c) 1980 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 */

#ifndef lint
static char sccsid[] = "@(#)externs.c	5.1 (Berkeley) 5/30/85";
#endif not lint

# include	"trek.h"

/*
**	global variable definitions
*/

struct device	Device[NDEV] =
{
	"warp drive",		"Scotty",
	"S.R. scanners",	"Scotty",
	"L.R. scanners",	"Scotty",
	"phasers",		"Sulu",
	"photon tubes",		"Sulu",
	"impulse engines",	"Scotty",
	"shield control",	"Sulu",
	"computer",		"Spock",
	"subspace radio",	"Uhura",
	"life support",		"Scotty",
	"navigation system",	"Chekov",
	"cloaking device",	"Scotty",
	"transporter",		"Scotty",
	"shuttlecraft",		"Scotty",
	"*ERR 14*",		"Nobody",
	"*ERR 15*",		"Nobody"
};

char	*Systemname[NINHAB] =
{
	"ERROR",
	"Talos IV",
	"Rigel III",
	"Deneb VII",
	"Canopus V",
	"Icarus I",
	"Prometheus II",
	"Omega VII",
	"Elysium I",
	"Scalos IV",
	"Procyon IV",
	"Arachnid I",
	"Argo VIII",
	"Triad III",
	"Echo IV",
	"Nimrod III",
	"Nemisis IV",
	"Centarurus I",
	"Kronos III",
	"Spectros V",
	"Beta III",
	"Gamma Tranguli VI",
	"Pyris III",
	"Triachus",
	"Marcus XII",
	"Kaland",
	"Ardana",
	"Stratos",
	"Eden",
	"Arrikis",
	"Epsilon Eridani IV",
	"Exo III"
};
