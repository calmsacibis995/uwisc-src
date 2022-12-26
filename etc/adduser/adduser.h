#include <grp.h>
#include <pwd.h>
#ifndef pyr
#ifndef sun
#include <ndbm.h>
#endif sun
#endif pyr

#ifdef BSD2_8
/* vfork doesn't work on 2.8 systems */
#define	vfork	fork
#endif BSD2_8

#define	ALIAS_CMD	"/usr/lib/sendmail aliases"	/* person who sets aliases */
extern char Glob_names[];
extern char Passwd[];
extern char Lockfile[];
extern char NewPasswd[];
extern char OldPasswd[];
extern char Group[];
extern char NewGroup[];
extern char OldGroup[];
#ifdef pyr
extern char Universe[];
extern char defuniverse[];
#endif pyr
extern char Skel[];
extern int instructional;

extern char User[];
extern char IShell[];
extern char RShell[];
#define	START_ID	acct_types[accttype].start_id

#define	LOG_MAX		8			/* max. size of login name */
#define	MAX_USERS	32767		/* max-users in password file */

char 	last[40],
	first[50],
	logname[LOG_MAX+2],
	password[30],
	grpname[10],
	shell[40],
	homedir[40];

int	num_course, accttype;
char	ssn[12],
	courses[30];
extern struct 	{
	int	start_id;
	char lpquota[6];
	char resquota[4];
	char	acct_name[10];
	} acct_types[];

char	thepath[40];	/* where we put homedir for parsing */

#if defined(pyr)|defined(sun)
typedef	struct {
	char *dptr;
	int	dsize;
} datum;
#endif pyr|sun

datum	fetch(), al, key;
struct	group	*grpent, *getgrnam();
struct	passwd	*passent, *getpwnam();

char	*rindex();
int	uid, gid, dbmerror, override;
int	nomail, nohome, askuid;
int	kid, waitstat, newgrp;

#define STAFF 0
#define TA 1
#define COURSE 2
#define GRAD 3
#define UGRAD 4
#define STUDENT 5
