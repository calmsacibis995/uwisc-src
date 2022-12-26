#include <stdio.h>
#include "adduser.h"

char *strcpy();

char *
eat_shit(s)	/* kills all spaces and control characters in 's' */
	char *s;
{
	register char *i = (s-1), *o = s;

	while(*++i)
		if(*i > ' ')
			*o++ = *i;
	*o = 0;
	return(s);
}

/* Converts the string 's' to lower case */
char *
make_lower(s)
	char *s;
{
	register char *i = (s-1);

	while(*++i)
		if((*i >= 'A') && (*i <= 'Z'))
			*i += 32;
	return s;
}

/* check for legal ssn.  returns 1 if the ssn is legal, 0 otherwise */
int
ssnok(carr)
	char *carr;
{
	register i, j, ndig, ntot;

	ndig = strlen(carr);
	if(ndig == 0)
		return(1);	/* a null ssn is legal in this case */
	if (ndig != 10) {
		printf(" Too %s digits\n", (ndig < 10 ? "few" : "many"));
		return(0);
	}
	ntot = 0;
	for ( i = 0; i < 9; i++) {
		j = (int)carr[i];
		if (j <= '9' && j >= '0')
			j -= '0';
		else {
			printf(" contains non-numeric character\n");
			return(0);
		}
		if ( !( i & 1 ))
			j *= 2;
		ntot += ( j % 10 );
		if ( j > 9 )
			ntot++;
	}
	ntot %= 10;
	ntot = 10 - ntot;
	if (ntot > 9)
		ntot -= 10;
	if (ntot != (int)(carr[9] - '0')) {
		printf("check-digit mismatch.  retype\n");
		return(0);
	}
	return(1);
}

getdata()
{
	char readstr[256], change[100], *s;
	register i,j;

	newgrp = 0;
	printf("Full-name (last,first etc): "); fflush(stdout);
	if(gets(readstr) == NULL)
		quit(0);
	if(*(eat_shit(readstr)) == 0)
		quit(0);
	sscanf(readstr, "%[^,],%[^\n]",last, first);

/*
 * These are special things for the UW instructional machines,
 * which are a bit wierd.  
 */
	if(instructional)
		get_inst_data();
	else
		accttype = TA;		/* default to TA for research */

	make_lower(last);	/* make names pretty */
	if (accttype == GRAD) {
		strcpy(logname, "g-");
		strncpy(logname+2, last, LOG_MAX-2);	/* grads get g- in login */
	} else
		strncpy(logname, last, LOG_MAX);	/* save this first */

	logname[LOG_MAX] = 0;	/* insure null-terminator */
	make_lower(first);
	if((*last >= 'a') && (*last <= 'z'))	 /* cap 1st char of names */
		*last -= 32;
	if((*first >= 'a') && (*first <= 'z'))
		*first -= 32;
	j = check_unique(logname);	/* Now, check for a unique name */
	i = 0;

	for(;;) {
		printf("Login name <%s>: ", logname); fflush(stdout);
		if(gets(readstr) == NULL)
			quit(0);
		if(*(eat_shit(readstr)))
			strncpy(logname, make_lower(readstr), LOG_MAX);
		if(j = check_unique(logname)) {	/* make them retry if not unique */
			if((j == 2) && (++i == 3)) {
				printf("Override alias conflict? "); fflush(stdout);
				if(gets(change) == NULL)
					quit(0);
				if((*change|32) == 'y') {	/* ignore mod if yes */
					strncpy(logname, make_lower(readstr), LOG_MAX);
					override++;
					break;
				}
			} else
				printf("%s - try again\n",
					(j==2 ? "Alias conflict" : "Not unique"));
		} else break;
	}

	/* we got a login name.  Now, let's get the group */
	for(;;) {
		printf("Group <%s>: ", strcpy(grpname, logname)); fflush(stdout);
		if(accttype >= GRAD) {	/* GRAD and students have no choice */
			putchar('\n');
			newgrp++;
			break;
		}
		if(gets(readstr) == NULL)
			quit(0);
		if(*(eat_shit(readstr))) {
			strncpy(grpname, make_lower(readstr), 8);
			if((grpent = getgrnam(grpname)) == (struct group *) 0) {
				if(newgrp < 2)	{
					printf("%s is not currently a group\n", grpname);
					newgrp++;
					continue;
				}
			} else
				newgrp = 0;
		} else
			newgrp++;
		break;
	}

	/* ask them for a uid if they don't want us to guess */
	if(askuid)
		for(;;) {
			printf("Uid: "); fflush(stdout);
			if(gets(readstr) == NULL)
				quit(0);
			if(*(eat_shit(readstr))) {
				uid = atoi(readstr);
				printf("Adding login with uid %d\n", uid);
				break;
			}
		}

	/* Now, we'll get the "other pertinent information"... */
	/* Their shell */
	if(instructional)
		strcpy(shell, IShell);
	else
		strcpy(shell, RShell);
	for(;;) {
		printf("Shell <%s>: ", shell); fflush(stdout);
		if(gets(readstr) == NULL)
			quit(0);
		if(*(eat_shit(readstr)))
			strncpy(shell, readstr, 38);
		if(access(shell, 0) == 0)
			break;
		perror(shell);
	}
	
	/* where to put them */
	if(nohome)
		strcpy(homedir,"/");
	else {
		if(newgrp || (grpent->gr_mem[0] == 0))
			gethome();
		else {
			passent = getpwnam(grpent->gr_mem[0]);
			if (passent == NULL) {
				fprintf(stderr, "%s in group %s is not in the passwd file.\n", grpent->gr_name, grpent->gr_mem[0]);
				quit(0);
			}
			strcpy(homedir, passent->pw_dir);
			if(*homedir)
				strcpy(rindex(homedir, '/')+1, logname);
			else
				gethome();
		}

		for(;;) {
			printf("Home directory <%s>: ", homedir); fflush(stdout);
			if(gets(readstr) == NULL)
				quit(0);
			if(*(eat_shit(readstr))) {
				printf("Really put home directory in %s (y/n)? ", readstr); fflush(stdout);
				gets(change);
				if((*change|32) != 'y')
					continue;
				strncpy(homedir, readstr, 38);
			}
			break;
		}
	}
}

get_inst_data()
{
	char readstr[256], change[100], *s;
	register i,j;

	for(;;) {
		printf("U.W. Number <null>: "); fflush(stdout);
		if(gets(readstr) == NULL)
			quit(0);
		if(ssnok(readstr))
			break;
	}
	strncpy(ssn, eat_shit(readstr), 10);
	num_course = 1;

	for(;;) {
		printf("Account type: 1) staff, 2) TA, 3) COURSE, 4) GRAD, 5) UGRAD, 6) student <6>: "); fflush(stdout);
		if(gets(readstr) == NULL)
			quit(0);
		if(*(eat_shit(readstr)))
			accttype = atoi(readstr)-1;
		else
			accttype = STUDENT;
		if((accttype >= STAFF) && (accttype <= STUDENT))
			break;
	}
	if(accttype < STUDENT)	/* we already know contents of this field */
		strcpy(courses, acct_types[accttype].acct_name);
	else {
		printf("Courses (COURSE.SECTION, separated by comma's) <special>: "); fflush(stdout);
		if(gets(readstr) == NULL)
			quit(0);
		if(*(eat_shit(readstr)))
			strncpy(courses, readstr, 28);
		else
			strcpy(courses, "special");
		s = readstr-1;
		while(*++s)
			if(*s == ',')
				num_course++;
	}
}

/*
 *	This is where we ask for a place to put the home directory.
 *	Because every system is so different w.r.t. file-system organization,
 *	it's easier to ask for a place to put the home directory than guess.
 */
gethome()
{
	char readstr[256];
	if((kid = vfork()) == 0) {	/* we're a kid! let's turn into df */
		execl("/bin/df", "df", 0);
		_exit(1);
	}
	while(kid != wait(&waitstat));	/* wait for df to finish */

	for(;;) {
		printf("Home file-system: "); fflush(stdout);
		if(gets(readstr) == NULL)
			quit(0);
		if((*(eat_shit(readstr))==0) || (getfsfile(readstr) == 0))
			printf("That's not a filesystem name\n");
		else break;
	}
	if (strcmp(readstr, "/") == 0)
		*readstr = 0;
	if(instructional)
		sprintf(homedir, "%s/%s", readstr, logname);
	else
		sprintf(homedir, "%s/%s/%s", readstr, grpname, logname);
}

/*
 *	This function finds a unique version of the given login name.
 *	It first checks the current password file, then the global
 *	mail aliases for conflicts. It returns a 0 if the given name
 *	was unique, a 1 if the name was modified and a 2 if the mod
 *	was due to an alias conflict.
 */
int
check_unique(logname)
	char	*logname;
{
	register uniq_count = 0;
	register retval = 0;

	/* first, check passwd for any conflict */
	while(getpwnam(logname)) {
		if((strlen(logname) == LOG_MAX) || uniq_count)
			logname[strlen(logname)-1] = 0;
		logname[strlen(logname)+1] = '\0';
		logname[strlen(logname)] = (++uniq_count) + '0';
		retval = 1;
	}

	if(retval || dbmerror)	/* set if no dbm alias file */
		return retval;

	/* So far so good, now check global aliases.... */
	key.dptr = logname;
	for(;;) {
		key.dsize = strlen(logname)+1;
		al = fetch(key);
		if(al.dptr == NULL)
			break;
		if((strlen(logname) == LOG_MAX) || uniq_count)
			logname[strlen(logname)-1] = 0;
		logname[strlen(logname)+1] = '\0';
		logname[strlen(logname)] = (++uniq_count) + '0';
		retval = 2;
	}

	return retval;
}
