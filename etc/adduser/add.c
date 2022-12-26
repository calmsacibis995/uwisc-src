#include <stdio.h>
#include <sys/file.h>
#include "adduser.h"

add()	/* Now we've got all the info we need.  Let's add 'em! */
{
	if(!askuid)	/* if we asked, uid is already set */
		uid = finduid();
	gid = (newgrp ? uid : grpent->gr_gid);
	endgrent();	/* no more looking in group either */

	addpasswd();	/* put in password file */
	addgroup();	/* put in group file */
	if(instructional)
		adduser();	/* put in user file */
#ifdef pyr
	adduniverse();	/* put in universe file */
#endif pyr
	mkhomedir();	/* make the new home directory tree & copy /usr/skel */

	if(rename(Passwd, OldPasswd))
	{
		printf("Error: cannot remove old password file. Directory there -- no login.\n");
		quit(4);
	}
	if(rename(Group, OldGroup))
	{
		printf("Error: cannot move old group file. Directory there -- no login.\n");
		quit(4);
	}

	if(rename(NewPasswd, Passwd)) {
		printf("Error: cannot move new password file. Move it and new group file yourself!\n");
		quit(4);
	}

	if(rename(NewGroup, Group)) {
		printf("Error: cannot move new group file. Move it yourself!\n");
		quit(4);
	}

#ifdef YP
	upd_yp("passwd");
#else
#ifndef pyr
#ifndef sun
	upd_hash();		/* put them is the hashed file too */
#endif sun
#endif pyr
#endif YP

	/* well, that's done with.  Now lets set the password... */
	unlink(Lockfile);	/* first get rid of lock file */

	/* now, change file ownership */
	if(!nohome) {
		char buf[1024];
		sprintf(buf, "/etc/chown -R %s.%s %s",logname,grpname,homedir);
		system(buf);
	}

	if(instructional) {
		lpset();
		resset();
		setdiskq();
		if(accttype >= GRAD) {	/* student and GRAD */
			chmod(homedir, 0750);	/* fix their protection */
			exit(0);
		}
	}

	if(instructional || strcmp(shell, RShell)) {
		if((kid = vfork()) == 0) {	/* we're a kid! let's get the passwd */
			execl("/bin/passwd", "passwd", logname, 0);
			_exit(1);
		}
		while(kid != wait(&waitstat));	/* wait for passwd to finish */
		if(waitstat)	/* hey, that didn't work */
			printf("Error: could not set password.\n");
	} /* endif */

	if(!instructional)
		mail_aliases();		/* tell alias maintainer to add the person */
}

/*
**	finduid -- find the first unused uid in /etc/passwd.
*/
int
finduid()
{
#define BITS_PER_CHAR	8
	register i;
	char	users[MAX_USERS/BITS_PER_CHAR];	/* bitmask -- 8 bits per char */
	struct	passwd *getpwent();

	bzero(users, MAX_USERS/BITS_PER_CHAR);	/* zero out bitmask */

	while (passent = getpwent())
		users[passent->pw_uid/BITS_PER_CHAR] |=
			1<<(passent->pw_uid%BITS_PER_CHAR);
	for (i=START_ID; i<MAX_USERS; i++)
		if((users[i/BITS_PER_CHAR]&(1<<(i%BITS_PER_CHAR))) == 0)
			break;
	endpwent();
	return(i);
}

addpasswd()
{
	char	passline[200],readstr[202];	/* holds password file entry */
	register FILE	*oldpass, *newpass;
	int	curruid, ruid = uid;

	strcpy(password, "nopass");	/* all get this in case password set fails */
	sprintf(passline, "%s:%s:%d:%d:%s %s:%s:%s\n", logname, password, uid,
		gid, first, last, homedir, shell);

	oldpass = fopen(Passwd, "r");	/* open the password files */
	newpass = fopen(NewPasswd, "w");
	fgets(readstr, 200, oldpass);
	do {
		sscanf(readstr, "%*[^:]:%*[^:]:%d", &curruid);
		if(curruid >= ruid) {
			fputs(passline, newpass);
			ruid = MAX_USERS+1;	/* a magic, unused uid */
		}
		fputs(readstr, newpass);
	} while(fgets(readstr, 200, oldpass) != NULL);

	if(ruid != MAX_USERS+1)
		fputs(passline, newpass);

	fclose(oldpass);	/* all done, close 'em */
	fclose(newpass);
}

addgroup()
{
	char	grpline[1024], readstr[1024];	/* holds group file entry */
	register FILE	*oldgroup, *newgroup;
	int	currgid, rgid = gid;

	oldgroup = fopen(Group, "r");	/* open the files */
	newgroup = fopen(NewGroup, "w");

	while(fgets(readstr, 1000, oldgroup) != NULL) {
		sscanf(readstr, "%*[^:]:%*[^:]:%d", &currgid);
		if(currgid == rgid) {	/* append new person */
			if(readstr[strlen(readstr)-2] == ':')	/* blank group */
				readstr[strlen(readstr)-1] = '\0';
			else
				readstr[strlen(readstr)-1] = ',';
			strcat(readstr, logname);
			strcat(readstr, "\n");
			rgid = MAX_USERS+1;	/* a magic, unused gid */
		} else if(currgid > rgid) {
			fprintf(newgroup, "%s:*:%d:%s\n",
				grpname, rgid, logname);
			rgid = MAX_USERS+1;	/* a magic, never used gid */
		}
		fputs(readstr, newgroup);	/* write entry into new file */
	}

	if(rgid != MAX_USERS+1)
		fprintf(newgroup, "%s:*:%d:%s\n", grpname, rgid, logname);

	fclose(oldgroup);	/* all done here too, close up and leave */
	fclose(newgroup);
}

adduser()
{
	register FILE	*olduser;

	olduser = fopen(User, "a");	/* open the user file */

	fprintf(olduser, "%s:%s:%s %s:%d:%s\n", logname, ssn, first, last,
		num_course, courses);
	fclose(olduser);
}

#ifdef pyr
adduniverse()
{
	register FILE *olduniverse;

	olduniverse = fopen(Universe, "a");		/* open the universe file */

	fprintf(olduniverse, "%s:%s\n", logname, defuniverse);
	fclose(olduniverse);
}
#endif pyr

#ifdef YP

upd_yp(file_name)
	char *file_name;
{
	extern	char	*domain_name;
	char buff[512];

	register i;

	sprintf(buff, "/etc/yp/%s/make %s\n", domain_name, file_name);
	i = system(buff);
	if((i >> 8) != 0) {
		printf("upd_yp (make &s) failed\n", file_name);
		abnquit();
	}
}

#endif YP

#ifndef pyr
#ifndef sun
upd_hash()
{
	int dsize = 0, msize = 0;
	register oumask, *nap;
	char buf[1024];
	register char *cp, *tp;
	datum key, content;
	DBM *dp;

	oumask = umask(0);
	dp = dbm_open(Passwd, O_RDWR, 0644, &dsize, &msize, NULL);
	if (dp == NULL) {
		fprintf(stderr, "dbminit failed: ");
		fflush(stderr);
		perror(Passwd);
		fprintf(stderr, "Please 'rmuser %s' and restart\n", logname);
		abnquit();
	}
	cp = buf;
	tp = logname;
	while (*cp++ = *tp++)
		;
	tp = password;
	while (*cp++ = *tp++)
		;
	nap = (int *)cp;
	*nap = uid;
	cp += sizeof (int);
	nap = (int *)cp;
	*nap = gid;
	cp += sizeof (int);
	nap = (int *)cp;
	*nap = 0;	/* quota */
	cp += sizeof (int);
	*cp++ = 0;	/* comment */
	sprintf(cp, "%s %s", first, last);
	cp += strlen(cp)+1;
	tp = homedir;
	while (*cp++ = *tp++)
		;
	tp = shell;
	while (*cp++ = *tp++)
		;
	content.dptr = buf;
	content.dsize = cp - buf;
	key.dptr = logname;
	key.dsize = strlen(logname);
	if (dbm_store(dp, key, content, DBM_INSERT) < 0) {
		fprintf(stderr, "mkpasswd: ");
		perror("dbm_store failed");
		exit(1);
	}
	key.dptr = (char *) &uid;
	key.dsize = sizeof(int);
	if (dbm_store(dp, key, content, DBM_INSERT) < 0) {
		fprintf(stderr, "mkpasswd: ");
		perror("dbm_store failed");
		exit(1);
	}
	dbm_close(dp);
	(void) umask(oumask);
}
#endif sun
#endif pyr

mkhomedir()
{
	char	*pathlist[15],	/* holds the parsed up home-directory path */
			 readstr[256];
	register char *prev;
	register char *i;
	register curr = 0, j;
	int	madedir = 0;

	if(nohome)
		return;
	/* parse up the path to get individual directories */
	strncpy(thepath, homedir, 40);
	prev = i = thepath - 1;
	pathlist[curr++] = "/";
	while(*(++i)) {
		if(*i == '/') {
			if(prev+1 != i)
				pathlist[curr++] = prev+1;
			prev = i;
			*i = 0;
		}
	}
	pathlist[curr] = prev+1;

	/* now, make directory hierarchy */
	for(j=0; j<=curr; j++)
		if(chdir(pathlist[j])) {	/* non-zero means not found */
			if((kid = vfork()) == 0) {	/* turn into mkdir */
				execl("/bin/mkdir", "mkdir", pathlist[j], 0);
				_exit(1);
			}
			while(kid != wait(&waitstat));	/* wait for mkdir */
			if(waitstat != 0) {
				printf("Error: cannot 'mkdir %s'. Aborting\n",
					pathlist[j]);
				quit(2);
			}
			madedir++;
			if(chdir(pathlist[j])) { /* we've got problems */
				printf("Error: cannot get to %s. Aborting\n",
					pathlist[j]);
				quit(3);
			}
		}

	/* now, copy /usr/skel stuff into this directory */
	if(!madedir) {
		printf("Directory exists.  Copy dot files <n>? ");
		if(gets(readstr) == NULL)
			quit(0);
		if(*readstr|32 != 'y') {
			nohome++;	/* use this flag to not change prots */
			return;
		}
	}
	chdir(Skel);
	sprintf(readstr, "/bin/cp .??* %s/", homedir);
	waitstat = system(readstr);
	if(waitstat)
		printf("Error: could not cp dot files from %s!\n", Skel);
}

mail_aliases()
{
	FILE	*pipe, *popen();
#ifdef BSD4_2
#define HOSTLEN 20
	char	hostname[HOSTLEN];
#endif BSD4_2

	if(override || nomail)	/* no need to mail if already set up */
		return;
	if((pipe = popen(ALIAS_CMD, "w")) == NULL) {
		printf("Error could not 'mail aliases'. Do it yourself!\n");
		quit(5);
	}
	fprintf(pipe, "To: aliases\n");
	fprintf(pipe, "Subject: Mail alias for %s %s\n\n", first, last);
	fprintf(pipe, "Please make a mail alias for %s %s.\n", first, last);
#ifdef BSD4_2
	gethostname(hostname, HOSTLEN);
	fprintf(pipe, "Login name is %s@%s.\n", logname, hostname);
#else
	fprintf(pipe, "Login name is %s@%s.\n", logname, thisname());
#endif BSD4_2
	fprintf(pipe, "\t\t\tThe adduser program\n.\n");
	pclose(pipe);
}

lpset()
{
	if((kid = vfork()) == 0) {
		execlp("lpset", "lpset", acct_types[accttype].lpquota, logname, 0);
		_exit(1);
	}
	while(kid != wait(&waitstat));
	if(waitstat != 0)
		printf("Error: paper quota not set\n");
}

resset()
{
	if((kid = vfork()) == 0) {
		execlp("resset", "resset", acct_types[accttype].resquota, logname, 0);
		_exit(1);
	}
	while(kid != wait(&waitstat));
	if(waitstat != 0)
		printf("Error: reservations not set\n");
}

setdiskq()
{
#ifdef BSD4_2
#ifndef NFS
	if((kid = vfork()) == 0) {
		execlp("setdiskq", "setdiskq", "-n", courses, logname, 0);
		_exit(1);
	}
	while(kid != wait(&waitstat));
	if(waitstat != 0)
		printf("Error: disk quota not set\n");
#endif !NFS
#endif BSD4_2
}
