/*
 *  vf.c by tom christiansen, Friday, 18 Jul 86 -- 06:56:22 CDT
 *
 *  read in list of files with attributes, compare with
 *  existing files' attrs, correct them if -c flag given,
 *  print diagnostics explaining discrepancies and corrections.
 *  
 * 	modified by author Wednesday, 30 Jul 86 -- 05:20:02 CDT 
 * 		to send all msgs to stdout
 * 		have exit status better reflect error conditions
 */
#include <stdio.h>
#include <sysexits.h>
#include <pwd.h>
#include <grp.h>
#include <strings.h>
#include <sys/types.h>
#include <sys/file.h>
#include <sys/stat.h>

#ifndef MAXUSERS
#	define MAXUSERS	1500
#endif !MAXUSER

#define MODE(x) ((x) & ~S_IFMT) 
#define TYPE(x) ( ((x) & S_IFMT) == 0 ? S_IFREG : ((x) & S_IFMT))

#define FLAGS(A,B) (L_/**/A | L_/**/B)
#define L_USER		01
#define L_GROUP		02
#define L_NAME		10
#define L_ID		20

#define NOBODYS_UID	32767

#define STREQ(STR1,STR2) (strcmp(STR1,STR2) == 0)

extern char *malloc();
extern int   sys_nerr;
extern char *sys_errlist[];
extern int   errno;

char name[1025], owner[9], group[9];
char *users[MAXUSERS], *groups[MAXUSERS];
int errors=0, corrected=0, linecount=0;
struct stat stat_buf;
char *typename();

/*
 *  convenient lie:  assumes a (struct passwd *)
 *  and a (struct group *) both start like this.
 */
struct info {
	char *myname;
	char *mypasswd;
	int   myid;
};




main (ac,av) 
	char *av[];
{
	FILE *input;
	char *program = *av;
	int mode,cflag=0;
	register scan_count,uid,gid;
	register struct stat *sb = &stat_buf;

	if (STREQ(av[1],"-c")) {
		cflag++;
		av++;
		ac--;
	}

	if (cflag && getuid()) {
		fprintf ( stdout, "%s: Not superuser\n", program );
		exit (EX_NOPERM);
	}

	/*
	 *  process arguments
	 */
	switch (ac) {
		case 1:
			input = stdin;
			break;

		case 2:
			if (av[1][0] == '-')
				goto usage;
			if (!(input=fopen(av[1],"r"))) {
				fprintf ( stdout, "%s: ", program);
				perror (av[1]);
				exit (EX_NOINPUT);
			}
			break;

		default:
		usage:
			fprintf (stdout, "usage: %s [-c] [file]\n", program);
			exit (EX_USAGE);
	}

	/*
	 *  initialize all elts of the user and group arrays
	 *  to be "<unknown ....>".  we will check for 
	 *  whether (*users[uid] == '<') to see whether we
	 *  can chown it.
	 */
	init_array(users,&users[MAXUSERS],"<unknown user>");
	init_array(groups,&groups[MAXUSERS],"<unknown group>");

	/*
	 *  process input
	 */
	while(EOF!=(scan_count=fscanf(input,"%1024s%8s%8s%o",name,owner,group,&mode))) {
		linecount++;
		readln(input);	/* discard up to the next \n or EOF */

		if (scan_count != 4) {
			errors++;
			fprintf (stdout, "%s: format error at line %d in file \"%s\"\n", 
				program, linecount, (input == stdin) ? "<stdin>" : av[1] );
			fprintf (stdout,
						"\tscanned only %d items instead of 4\n",scan_count);
			exit(EX_DATAERR);
		}

		/*
		 *  is there such a file?
		 */
		if ( access (name, F_OK) != 0 || lstat (name, sb) < 0) {
			errors++;
			perror(name);
			continue;
		}

		/*
		 *  fix mode in case of directory specification
		 */
		{
			register char * s = name;
			while (s[1])
				s++;
			if (*s == '/')
				mode |= S_IFDIR;
		}

		/*
		 *  find numeric values of desired owner and group
		 */
		uid = getid ( owner, users, getpwnam, FLAGS(NAME,USER));
		gid = getid ( group, groups, getgrnam, FLAGS(NAME,GROUP));

		if (sb->st_uid != uid) {
			getid ( sb->st_uid, users, getpwuid, FLAGS(ID,USER));
			fprintf ( stdout,"%s: owner is \"%s\", expected \"%s\"\n", 
				name, users[sb->st_uid], owner);
		}
		if (sb->st_gid != gid)  {
			getid ( sb->st_gid, groups,  getgrgid, FLAGS(ID,GROUP));
			fprintf ( stdout, "%s: group is \"%s\", expected \"%s\"\n", 
				name, groups[sb->st_gid], group);
		}

		if (sb->st_uid != uid || sb->st_gid != gid)  {
			if (cflag) {
				if ( (uid != NOBODYS_UID && *users[uid] == '<') 
						|| *groups[gid] == '<' ) {
					errors++;	
					fprintf (stdout, "\tCANNOT CORRECT %s\n",name);
				}
				else {
					fprintf ( stdout, "   correcting owner.group to be \"%s.%s\"\n",
						(uid==NOBODYS_UID)?"nobody":users[uid], groups[gid] );
					if (chown ( name, uid, gid ) < 0) {
						errors++;
						perror("chown");
					} else
					corrected++;
				}
			} else
				errors++;
		}

		if (MODE(sb->st_mode) != MODE(mode))  {
			fprintf ( stdout, "%s: mode is %04o, expected %04o\n",
				name, MODE(sb->st_mode), MODE(mode));
			if (cflag) {
				fprintf ( stdout, "    correcting mode %04o to be %04o\n",
					MODE(sb->st_mode), MODE(mode));
				if (chmod ( name, MODE(mode)) < 0) {
					errors++;
					perror("chmod");
				} else
					corrected++;
			} else
				errors++;
		}
		if (TYPE(sb->st_mode) != TYPE(mode))  {
			errors++;
			fprintf ( stdout, "%s: type is \"%s\", expected \"%s\"\n",
				name, typename((int)sb->st_mode), typename(mode));
			fprintf (stdout, "\tCANNOT CORRECT %s\n",name);
		}

	}

	if (errors > 0)
		exit(EX_UNAVAILABLE);
	else if (corrected > 0)
		exit(EX_TEMPFAIL);
	else
		exit(EX_OK);
}



getid ( who, list, lookup, code )
	char *who;
	char **list;
	struct info * (*lookup)();  /* culpa mea: this is a convenient lie */
{
	register struct info *p;

	if ( ((code&L_NAME) && STREQ(who,"nobody")) || ((code&L_ID) && (int) who == NOBODYS_UID))
		return NOBODYS_UID;

	if (!(p = (*lookup)(who))) {
		errors++;
		fprintf (stdout, "%s: No such %s: ", name, (code&L_USER) ? "user" : "group");
		fprintf (stdout, (code&L_ID)? "%d\n"  : "%s\n", who );
		return -1;
	}

	if ( p->myid >= MAXUSERS ) {
		errors++;
		fprintf (stdout, "%s: TO STORE %s, RECOMPILE WITH MAXUSERS > %d\n",
			name,who,p->myid);
		exit (EX_SOFTWARE);
	}
	if ( list [p->myid] [0] == '<' )
		(void) strcpy ( list [ p->myid ] = 
							malloc ( (u_int) strlen (p->myname)+1), 
						p->myname );
	return p->myid ;
}

char *
typename(mode) {
	switch ( TYPE(mode) ) {
		case S_IFREG:
			return "plain file";
		case S_IFDIR: 
			return "directory";
		case S_IFCHR:
			return "character special";
		case S_IFBLK:
			return "block special";
		case S_IFLNK:
			return "symbolic link";
		case S_IFSOCK:
			return "socket";
		default:
			errors++;
			fprintf (stdout, "typename: can't match mode %08o (%08o)\n",
				TYPE(mode),mode);
			return "unknown file type";
	}
}

init_array ( start, end, string )
	register char **start,**end,*string;
{
	do {
		*start++ = string;
	} while (start <= end);
}



readln(file)
	FILE *file;
{
	int wasted;

	while ( (wasted = getc(file)) != EOF && wasted != '\n')
		/* VOID */;
}

/*
 *  perror() to stdout
 */
perror(s)
	char *s;
{
	int oerrno = errno;
	fprintf (	stdout, "%s%s%s\n", s,
				( s != NULL )
					? ": " 
					: "" , 
				(oerrno < sys_nerr)
					? sys_errlist[oerrno]
					: "Unknown error" 
			);
}

