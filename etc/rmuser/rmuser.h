#include <grp.h>
#include <pwd.h>
#ifndef pyr
#ifndef sun
#include <sys/types.h>
#include <dbm.h>
#endif sun
#endif pyr

#define SUCCESS		0
#define	FAILURE		-1

#define	ALIAS_CMD	"/usr/lib/sendmail aliases"	/* person who sets aliases */
extern char Glob_names[];
extern char Passwd[];
extern char Lockfile[];
extern char NewPasswd[];
extern char OldPasswd[];
extern char Group[];
extern char NewGroup[];
extern char OldGroup[];
extern char NewUser[];
extern char OldUser[];
extern int instructional;

extern char User[];

#if defined(pyr)|defined(sun)
typedef	struct {
	char *dptr;
	int	dsize;
} datum;
#endif pyr|sun

datum	fetch(), al, key;
struct	passwd	*passent, *getpwnam();

char	*rindex();
int	uid, gid, dbmerror, override;
int	nomail, nohome;
int	kid, waitstat, newgrp;
