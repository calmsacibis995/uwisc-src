/*
 * Copyright (c) 1983 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 */

#ifndef lint
char copyright[] =
"@(#) Copyright (c) 1983 Regents of the University of California.\n\
 All rights reserved.\n";
#endif not lint

#ifndef lint
static char sccsid[] = "@(#)tunefs.c	5.3 (Berkeley) 9/11/85";
#endif not lint

/*
 * tunefs: change layout parameters to an existing file system.
 */

#include <sys/param.h>
#include <sys/stat.h>
#ifdef VFS
#include <sys/time.h>
#include <sys/vnode.h>
#include <ufs/fs.h>
#include <ufs/inode.h>
#else
#include <sys/fs.h>
#include <sys/inode.h>
#endif VFS

#include <stdio.h>
#include <fstab.h>

union {
	struct	fs sb;
	char pad[MAXBSIZE];
} sbun;
#define	sblock sbun.sb

int fi;

main(argc, argv)
	int argc;
	char *argv[];
{
	char *cp, *special, *name;
	struct stat st;
	int i;
	int Aflag = 0;
	struct fstab *fs;
	char *chg[2], device[MAXPATHLEN];
	extern char *sprintf();

	argc--, argv++; 
	if (argc < 2)
		goto usage;
	special = argv[argc - 1];
	fs = getfsfile(special);
	if (fs)
		special = fs->fs_spec;
again:
	if (stat(special, &st) < 0) {
		if (*special != '/') {
			if (*special == 'r')
				special++;
			special = sprintf(device, "/dev/%s", special);
			goto again;
		}
		fprintf(stderr, "tunefs: "); perror(special);
		exit(1);
	}
	if ((st.st_mode & S_IFMT) != S_IFBLK &&
	    (st.st_mode & S_IFMT) != S_IFCHR)
		fatal("%s: not a block or character device", special);
	getsb(&sblock, special);
	for (; argc > 0 && argv[0][0] == '-'; argc--, argv++) {
		for (cp = &argv[0][1]; *cp; cp++)
			switch (*cp) {

			case 'A':
				Aflag++;
				continue;

			case 'a':
				name = "maximum contiguous block count";
				if (argc < 1)
					fatal("-a: missing %s", name);
				argc--, argv++;
				i = atoi(*argv);
				if (i < 1)
					fatal("%s: %s must be >= 1",
						*argv, name);
				fprintf(stdout, "%s changes from %d to %d\n",
					name, sblock.fs_maxcontig, i);
				sblock.fs_maxcontig = i;
				continue;

			case 'd':
				name =
				   "rotational delay between contiguous blocks";
				if (argc < 1)
					fatal("-d: missing %s", name);
				argc--, argv++;
				i = atoi(*argv);
				if (i < 0)
					fatal("%s: bad %s", *argv, name);
				fprintf(stdout,
					"%s changes from %dms to %dms\n",
					name, sblock.fs_rotdelay, i);
				sblock.fs_rotdelay = i;
				continue;

			case 'e':
				name =
				  "maximum blocks per file in a cylinder group";
				if (argc < 1)
					fatal("-e: missing %s", name);
				argc--, argv++;
				i = atoi(*argv);
				if (i < 1)
					fatal("%s: %s must be >= 1",
						*argv, name);
				fprintf(stdout, "%s changes from %d to %d\n",
					name, sblock.fs_maxbpg, i);
				sblock.fs_maxbpg = i;
				continue;

			case 'm':
				name = "minimum percentage of free space";
				if (argc < 1)
					fatal("-m: missing %s", name);
				argc--, argv++;
				i = atoi(*argv);
				if (i < 0 || i > 99)
					fatal("%s: bad %s", *argv, name);
				fprintf(stdout,
					"%s changes from %d%% to %d%%\n",
					name, sblock.fs_minfree, i);
				sblock.fs_minfree = i;
				if (i >= 10 && sblock.fs_optim == FS_OPTSPACE)
					fprintf(stdout, "should optimize %s",
					    "for time with minfree >= 10%\n");
				if (i < 10 && sblock.fs_optim == FS_OPTTIME)
					fprintf(stdout, "should optimize %s",
					    "for space with minfree < 10%\n");
				continue;

			case 'o':
				name = "optimization preference";
				if (argc < 1)
					fatal("-o: missing %s", name);
				argc--, argv++;
				chg[FS_OPTSPACE] = "space";
				chg[FS_OPTTIME] = "time";
				if (strcmp(*argv, chg[FS_OPTSPACE]) == 0)
					i = FS_OPTSPACE;
				else if (strcmp(*argv, chg[FS_OPTTIME]) == 0)
					i = FS_OPTTIME;
				else
					fatal("%s: bad %s (options are `space' or `time')",
						*argv, name);
				if (sblock.fs_optim == i) {
					fprintf(stdout,
						"%s remains unchanged as %s\n",
						name, chg[i]);
					continue;
				}
				fprintf(stdout,
					"%s changes from %s to %s\n",
					name, chg[sblock.fs_optim], chg[i]);
				sblock.fs_optim = i;
				if (sblock.fs_minfree >= 10 && i == FS_OPTSPACE)
					fprintf(stdout, "should optimize %s",
					    "for time with minfree >= 10%\n");
				if (sblock.fs_minfree < 10 && i == FS_OPTTIME)
					fprintf(stdout, "should optimize %s",
					    "for space with minfree < 10%\n");
				continue;

			default:
				fatal("-%c: unknown flag", *cp);
			}
	}
	if (argc != 1)
		goto usage;
	bwrite(SBLOCK, (char *)&sblock, SBSIZE);
	if (Aflag)
		for (i = 0; i < sblock.fs_ncg; i++)
			bwrite(fsbtodb(&sblock, cgsblock(&sblock, i)),
			    (char *)&sblock, SBSIZE);
	close(fi);
	exit(0);
usage:
	fprintf(stderr, "Usage: tunefs tuneup-options special-device\n");
	fprintf(stderr, "where tuneup-options are:\n");
	fprintf(stderr, "\t-a maximum contiguous blocks\n");
	fprintf(stderr, "\t-d rotational delay between contiguous blocks\n");
	fprintf(stderr, "\t-e maximum blocks per file in a cylinder group\n");
	fprintf(stderr, "\t-m minimum percentage of free space\n");
	fprintf(stderr, "\t-o optimization preference (`space' or `time')\n");
	exit(2);
}

getsb(fs, file)
	register struct fs *fs;
	char *file;
{

	fi = open(file, 2);
	if (fi < 0) {
		fprintf(stderr, "cannot open");
		perror(file);
		exit(3);
	}
	if (bread(SBLOCK, (char *)fs, SBSIZE)) {
		fprintf(stderr, "bad super block");
		perror(file);
		exit(4);
	}
	if (fs->fs_magic != FS_MAGIC) {
		fprintf(stderr, "%s: bad magic number\n", file);
		exit(5);
	}
}

bwrite(blk, buf, size)
	char *buf;
	daddr_t blk;
	register size;
{
	if (lseek(fi, blk * DEV_BSIZE, 0) < 0) {
		perror("FS SEEK");
		exit(6);
	}
	if (write(fi, buf, size) != size) {
		perror("FS WRITE");
		exit(7);
	}
}

bread(bno, buf, cnt)
	daddr_t bno;
	char *buf;
{
	register i;

	if (lseek(fi, bno * DEV_BSIZE, 0) < 0)
		return(1);
	if ((i = read(fi, buf, cnt)) != cnt) {
		for(i=0; i<sblock.fs_bsize; i++)
			buf[i] = 0;
		return (1);
	}
	return (0);
}

/* VARARGS1 */
fatal(fmt, arg1, arg2)
	char *fmt, *arg1, *arg2;
{

	fprintf(stderr, "tunefs: ");
	fprintf(stderr, fmt, arg1, arg2);
	putc('\n', stderr);
	exit(10);
}
