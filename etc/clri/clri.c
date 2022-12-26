static char sccsid[] = "@(#)clri.c 2.2 4/11/82";

/* static char *sccsid = "@(#)clri.c	4.1 (Berkeley) 10/1/80"; */

/*
 * clri filsys inumber ...
 */

#ifndef SIMFS
#include <sys/param.h>
#ifdef VFS
#include <sys/time.h>
#include <sys/vnode.h>
#include <ufs/inode.h>
#include <ufs/fs.h>
#else
#include <sys/inode.h>
#include <sys/fs.h>
#endif VFS
#else
#include "../h/param.h"
#include "../h/inode.h"
#include "../h/fs.h"
#endif

#define ISIZE	(sizeof(struct dinode))
#define	NI	(MAXBSIZE/ISIZE)

#ifdef VFS
struct	dinode	buf[NI];
#else
struct	ino {
	char	junk[ISIZE];
};
struct	ino	buf[NI];
#endif VFS

union {
	char		dummy[SBSIZE];
	struct fs	sblk;
} sb_un;
#define sblock sb_un.sblk

int	status;

main(argc, argv)
	int argc;
	char *argv[];
{
	register i, f;
	unsigned n;
	int j, k;
	long off;
#ifdef VFS
	long gen;
#endif VFS

	if (argc < 3) {
		printf("usage: clri filsys inumber ...\n");
		exit(4);
	}
	f = open(argv[1], 2);
	if (f < 0) {
		printf("cannot open %s\n", argv[1]);
		exit(4);
	}
	lseek(f, SBLOCK * DEV_BSIZE, 0);
	if (read(f, &sblock, SBSIZE) != SBSIZE) {
		printf("cannot read %s\n", argv[1]);
		exit(4);
	}
	if (sblock.fs_magic != FS_MAGIC) {
		printf("bad super block magic number\n");
		exit(4);
	}
	for (i = 2; i < argc; i++) {
		if (!isnumber(argv[i])) {
			printf("%s: is not a number\n", argv[i]);
			status = 1;
			continue;
		}
		n = atoi(argv[i]);
		if (n == 0) {
			printf("%s: is zero\n", argv[i]);
			status = 1;
			continue;
		}
		off = fsbtodb(&sblock, itod(&sblock, n)) * DEV_BSIZE;
		lseek(f, off, 0);
		if (read(f, (char *)buf, sblock.fs_bsize) != sblock.fs_bsize) {
			printf("%s: read error\n", argv[i]);
			status = 1;
		}
	}
	if (status)
		exit(status);
	for (i = 2; i < argc; i++) {
		n = atoi(argv[i]);
		printf("clearing %u\n", n);
		off = fsbtodb(&sblock, itod(&sblock, n)) * DEV_BSIZE;
		lseek(f, off, 0);
		read(f, (char *)buf, sblock.fs_bsize);
		j = itoo(&sblock, n);
#ifdef VFS
		gen = buf[j].di_gen;
		bzero((caddr_t)&buf[j], ISIZE);
		buf[j].di_gen = gen + 1;
#else
		for (k = 0; k < ISIZE; k++)
			buf[j].junk[k] = 0;
#endif VFS
		lseek(f, off, 0);
		write(f, (char *)buf, sblock.fs_bsize);
	}
	exit(status);
}

isnumber(s)
	char *s;
{
	register c;

	while(c = *s++)
		if (c < '0' || c > '9')
			return(0);
	return(1);
}
