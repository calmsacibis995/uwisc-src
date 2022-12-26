#include <stdio.h>
#include <sys/file.h>
#include "rmuser.h"

/*
 * usage: rmuser [-c class] [-f file] [-h] user [user ...]
 * Options:
 *	c - remove the users from a particular class - just do accounting.
 *	f - remove all users in the specified file 
 *	h - remove user but leave home directory
 */
 /*  Want to fork off a find at the end to list out all the files that
 are owned by the person that is being redemoved. */

main (argc,argv)
char 	**argv;
int		argc;
{
	int cflag = 0;
	int fflag = 0;
	int hflag = 0;
	char *filename;
	char *class;
	FILE *fp;
	char buff[80];
	int n = 80;
	int count = 0;

	/* process arguments */
	argv++;
	while (**argv=='-') {
		(*argv)++;
		while(**argv) {
			if ((**argv != 'c') && (**argv != 'f') && (**argv != 'h')) {
			   printf("Usage: rmuser [-c class][-f file][-h] user [user...]\n");
			   exit(1);
			}
			if (**argv=='c') {
				cflag=1;
				printf("Flag is %c\n",**argv);
				class = *++argv;
				printf("Class is %s\n",*argv);
				break;
			}
			if (**argv=='f') {
				fflag=1;
				printf("Flag is %c\n",**argv);
				filename = *++argv;
				printf("Filename is %s\n",*argv);
				break;
			}
			if (**argv=='h') {
				hflag=1;
				printf("Flag is %c\n",**argv);
				(*argv)++;
			}
		}
		argv++;
	}

	if(link(Passwd, Lockfile)) {	/* make lock file */
		printf("Sorry, password file is busy.  Try later.\n");
		exit(1);
	}
	/* set instructional if 'User' file exists */
	instructional = !access(User, 0);
	if (cflag) {
		if (fflag){
			/* Open file .... */
			printf("Not implemented yet\n");
		}
		else {
			while (*argv)
				removeclass(class,*argv++);
		}
	}

	if (fflag) {
			if ((fp = fopen(filename,"r")) == NULL) {
				printf("File could not be opened\n");
				quit(1);
			}
			while (fgets(buff,n,fp) != NULL) {
				printf("About to remove %s\n",buff);
				count = strlen(buff);
				if (buff[count-1]=='\n')
					buff[count-1] = '\0';
				removeuser(hflag,buff);
			}
			fclose(fp);
	}
	else {
		while (*argv) {
			printf("About to remove %s\n",*argv);
			removeuser(hflag,*argv);
			argv++;
		}
	}


	(void)unlink(Lockfile);
}


removeclass(class,user)
char *user;
char *class;
{
	printf("Not implemented yet\n");
	quit(1);
}


removeuser(keep_home,user)
char *user;
int keep_home;
{
	int uid;
	int pid;
	int waitstat;
	struct passwd *getpwnam(), *pwent;
	char mailbuf[BUFSIZ];

	pwent = getpwnam(user);
	if(pwent == NULL) {
		printf("no user: %s\n", user);
		return;
	}
	uid = pwent->pw_uid;
	if (uid < 10) {
		printf("Cannot remove accounts with user-id < 10\n");
		quit(1); /* it is safe to quit -- no files are open */
	}

	/* Remove person's home directory..... */
	(void)sprintf(mailbuf,"/usr/spool/mail/%s",user);
	if((pid = vfork()) == 0) {	
		if (keep_home==0) {
			execl("/bin/rm", "rm", "-rf", pwent->pw_dir, mailbuf, 0);
			fprintf(stderr, "execl failed\n");
			_exit(1);
		}
		else {	/* Must make root the owner of the user's directories */
			execl("/etc/chown", "chown", "-R", "root.daemon", pwent->pw_dir, mailbuf, 0);
			fprintf(stderr, "execl failed\n");
			_exit(1);
		}
		while(pid != wait(&waitstat));	
		if(waitstat)	/* hey, that didn't work */
			fprintf(stderr,"Error: could not remove home directory or mail file.\n");
	} /* endif */


	/* Remove /etc/passwd entry... */
	if (rmpasswd(user) == FAILURE) {
		quit(4);
	}
	else {
		/* Remove /etc/group entry... */
		if (rmgroup(user) == FAILURE) {
			quit(4);
		}
		else {
			/* If a user file exists, remove /etc/user entry... */
			if (instructional)
				if (rmuserentry(user) == FAILURE)
					quit(4);
		}
	}

#ifndef pyr
#ifndef sun
	upd_hash(user,uid);		/* put them is the hashed file too */
#endif sun
#endif pyr

/* Mail aliases... */
	if (!instructional) mail_aliases(user);
}

int
rmpasswd(user) 
char *user;
{
	char readstr[BUFSIZ];
	char *readptr;
	char *fgets();
	FILE *oldpass, *newpass;

/* open /etc/passwd for reading */
/* open /etc/ptmp for writing */
	if (oldpass = fopen(Passwd, "r")) {
		if (newpass = fopen(NewPasswd, "w")) {

	/* Read in the password entry and put it in the form of getpwent. */
			while ((readptr = fgets(readstr,BUFSIZ,oldpass)) != NULL) {
				while (*readptr != ':') readptr++;
				*readptr = 0;
				if (strcmp(user,readstr)) {
					*readptr = ':';
					fputs(readstr,newpass);
				}
			}
			(void)fclose(newpass);
		}
	/* close ptmp */
	(void)fclose(oldpass);
	}
	if(rename(Passwd, OldPasswd))
	{
		printf("Error:cannot remove old password file. Please investigate\n");
		return FAILURE;
	}
	if(rename(NewPasswd, Passwd)) {
		printf("Error:cannot move new password file. Please investigate\n");
		return FAILURE;
	}
	return SUCCESS;

}

int
rmgroup(user) 
char *user;
{
	struct group *groupent;
	int i;
	FILE *fopen(), *oldgroup, *newgroup;

	if ((oldgroup = fopen(Group,"r"))==NULL) {
		printf("/etc/group could not be opened for reading.\n");
		return(FAILURE);
	}
	if ((newgroup = fopen(NewGroup, "w"))==NULL) {
		printf("/etc/group.new could not be opened for writing.\n");
		return(FAILURE);
	}

			(void)setgrent();

			while ((groupent = getgrent())!=NULL){ /*get one group entry*/
				i = 0;

				if((strcmp(groupent->gr_name,user) != 0)) {
					fprintf(newgroup,"%s:%s:%d:",groupent->gr_name,groupent->gr_passwd,groupent->gr_gid);
					/* delete name that is member of another group */

					while (groupent->gr_mem[i] != NULL) {
						if ((strcmp(groupent->gr_mem[i],user)!=0)) {
							if (i==0)
								fprintf(newgroup,"%s", groupent->gr_mem[i]);
							else 
								fprintf(newgroup,",%s", groupent->gr_mem[i]);
						}
						i++;
					}
					fprintf(newgroup,"\n");
				}
		
				/* If the group name matches the guy we are trying to delete,
					must make sure that there are no other people under them.
					If there are, print an error message and terminate. */
				else {
					/* count people in his group */
					while (groupent->gr_mem[i++] != NULL)
					if (i > 1) {
						printf("%s has people in their group.",user);
						printf("This person cannot be removed until the"); 
						printf(" people in their group are removed.\n");
						if(rename(Group, OldGroup))
						{
						 	printf("Error:cannot move old group file. Please investigate\n");
							return FAILURE;
						}
						if(rename(NewGroup, Group)) {
							printf("Error:cannot move new group file. Please investigate\n");
							return FAILURE;
						}
					}
					else {
						if ((strcmp(groupent->gr_mem[0],user) != 0)) {
							printf("Can't remove group;remove members first\n");
							if(rename(Group, OldGroup))
							{
						 		printf("Error:cannot move old group file. Please investigate\n");
								return FAILURE;
							}
							if(rename(NewGroup, Group)) {
						 		printf("Error:cannot move new group file. Please investigate\n");
								return FAILURE;
							}
						}
					}
				}
			}
			(void)fclose(newgroup);
	(void)fclose(oldgroup);
	(void)endgrent();
	if(rename(Group, OldGroup))
	{
		printf("Error:cannot move old group file. Please investigate\n");
		return FAILURE;
	}
	if(rename(NewGroup, Group)) {
		printf("Error:cannot move new group file. Please investigate\n");
		return FAILURE;
	}

	return SUCCESS;
}

int
rmuserentry(user) 
char *user;
{
	char readstr[BUFSIZ];
	char *readptr;
	char *fgets();
	FILE *fopen(), *olduser, *newuser;

	if (olduser = fopen(User,"r")) {
		if (newuser = fopen(NewUser, "w")) {
			while ((readptr = fgets(readstr,BUFSIZ,olduser)) != NULL) {
				/* strcmp needs the exact string to compare */
				while (*readptr != ':') readptr++;
				*readptr = 0;
				if (strcmp(user,readstr)) {
					*readptr = ':';
					fputs(readstr,newuser);
				}
			}
			(void)fclose(newuser);
		}
		(void)fclose(olduser);
	}
	if(rename(User, OldUser))
	{
		printf("Error: cannot move old user file. Directory there -- no login.\n");
		return FAILURE;
	}
	if(rename(NewUser, User)) {
		printf("Error: cannot move new user file. Move it yourself!\n");
		return FAILURE;
	}
	return SUCCESS;
}

/* o
main.c:
main.c(17): warning: argument argc unused in function main
main.c(107): warning: argument class unused in function removeclass
main.c(106): warning: argument user unused in function removeclass
wait, arg. 1 used inconsistently	llib-lc(138)  ::  main.c(143)
fclose returns value which is always ignored
*/
