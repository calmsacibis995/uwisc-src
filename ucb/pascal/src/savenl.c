/*
 * Copyright (c) 1980 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 */

#ifndef lint
static char sccsid[] = "@(#)savenl.c	5.1 (Berkeley) 6/5/85";
#endif not lint

/*
 * savenl - routines for saving namelist and line number information
 *
 * This module contains the routines that make pi dump a namelist
 * at the end of the object file.  We do this by first creating
 * four temporary files in "startnlfile".  One temp file contains
 * the string table, one the symbol table, one the file name
 * information and one the line number information.
 *
 * Prior to generation of the code for a statement the "lineno"
 * routine is called to dump the line number and current object
 * address.  At the end of each block "savenl" is called to dump
 * the strings and symbol structures.
 *
 * At the end of execution "copynlfile" is called and it copies
 * the temp files onto the end of the obj file.
 *
 * In case of error, "removenlfile" is called to destroy the temp files.
 *
 * The only other changes to pi are in calling these routines from
 *
 * 	"main"		(main.c)
 *	"yymain"	(yymain.c)
 *	"funcend"	(fend.c)
 *	"yyget"		(yyget.c)
 *	"putline"	(stat.c)
 */

#include "whoami.h"
#ifdef OBJ
        /*
	 *	and the rest of the file
	 */
#include "0.h"
#include "objfmt.h"

#undef NIL

/*
 * pdx header files
 */

#include "../pdx/defs.h"
#include "../pdx/object.h"
#include "../pdx/object/objsym.rep"
#include "../pdx/mappings.h"
#include "../pdx/mappings/filetab.h"

LOCAL char *symname = "/tmp/obj.symXXXX";
LOCAL char *strname = "/tmp/obj.strXXXX";
LOCAL char *filesname = "/tmp/obj.filesXXXX";
LOCAL char *linesname = "/tmp/obj.linesXXXX";

LOCAL FILE *symfp;
LOCAL FILE *strfp;
LOCAL FILE *filesfp;
LOCAL FILE *linesfp;

LOCAL long nlsize;

extern FILE *fopen();

/*
 * create temporary files for the namelist info
 */

startnlfile()
{
	nlsize = 0;
	(void) mktemp(symname);
	(void) mktemp(strname);
	(void) mktemp(filesname);
	(void) mktemp(linesname);
	symfp = fopen(symname, "w");
	strfp = fopen(strname, "w");
	filesfp = fopen(filesname, "w");
	linesfp = fopen(linesname, "w");
	if (symfp==NULL || strfp==NULL || filesfp==NULL || linesfp==NULL) {
		fprintf(stderr, "can't create /tmp/obj");
		pexit(NOSTART);
	}
	newfile(filename, 1);
}

/*
 * now copy the temp files back to obj; strings, symbols, file names, and lines
 *
 * There's some efficiency garbage here that uses straight system
 * calls rather than standard I/O library calls.
 */

copynlfile()
{
	register int n;
	extern long lseek();
	int symfd, strfd, filesfd, linesfd;
	char buff[BUFSIZ];

	(void) fclose((FILE *) symfp);
	(void) fclose((FILE *) strfp);
	(void) fclose((FILE *) filesfp);
	(void) fclose((FILE *) linesfp);
	if (!opt('g')) {
		removenlfile();
		return;
	}
	symfd = open(symname, 0);
	strfd = open(strname, 0);
	filesfd = open(filesname, 0);
	linesfd = open(linesname, 0);
	if (symfd < 0 || strfd < 0 || filesfd < 0 || linesfd < 0) {
		fprintf(stderr, "sync error on /tmp/obj");
		pexit(ERRS);
	}
	(void) lseek(ofil, 0L, 2);
	write(ofil, (char *) (&nlhdr), sizeof(nlhdr));
	n = read(strfd, buff, BUFSIZ - sizeof(nlhdr));
	write(ofil, buff, n);
	cat(strfd);
	cat(symfd);
	cat(filesfd);
	cat(linesfd);
	removenlfile();
}

cat(fd)
int fd;
{
	register int n;
	char buff[BUFSIZ];

	while ((n = read(fd, buff, BUFSIZ)) > 0) {
		write(ofil, buff, n);
	}
	(void) close(fd);
}

removenlfile()
{
	unlink(symname);
	unlink(strname);
	unlink(filesname);
	unlink(linesname);
}

nlhdrsize()
{
	int r;

	if (!opt('g')) {
		r = 0;
	} else {
		r = nlsize + sizeof(nlhdr);
	}
	return r;
}

#define isblock(s)	(s->class == FUNC || s->class == PROC)
#define isbuiltin(s)	((s->nl_block&037) == 0 && isblock(s))
#define symno(p)	(p==NULL ? 0 : nloff(p))

struct nls {
	struct nl *nls_low;
	struct nl *nls_high;
};

struct nl nl[], *nlp, ntab[], *nlact;

/*VARARGS*/
savenl(to, rout)
struct nl *to;
{
	register struct nl *p;
	register OBJSYM *s;
	OBJSYM tmpsym;
	struct nls *nlsp;

	if (to != NIL) {
		putblock((char *) rout);
	} else {
		putblock("main program");
	}
	nlsp = (struct nls *) nlact;
	s = &tmpsym;
	for (p = nlp; p != to;) {
		if (p == nlsp->nls_low) {
			if (nlsp == ((struct nls *) &ntab[0]))
				break;
			nlsp--;
			p = nlsp->nls_high;
		}
		p--;
		if (isbuiltin(p) || symno(p) == 0) {
			continue;
		}
		nlhdr.nsyms++;
		nlsize += sizeof(OBJSYM) + sizeof(int);
		(void) putw(symno(p), symfp);
		if (p->symbol != NULL) {
			s->strindex = nlhdr.stringsize;
			putstring(p->symbol);
		} else {
			s->strindex = 0;
		}
		s->oclass = p->class;
		s->oblkno = (p->nl_block&037);
		s->typno = symno(p->type);
		s->chno = symno(p->chain);
		s->osymvalue.orangev.lower = p->range[0];
		s->osymvalue.orangev.upper = p->range[1];
		if (isblock(p)) {
			s->osymvalue.ofuncv.codeloc = p->value[NL_ENTLOC];
		} else if (p->class == RECORD || p->class == VARNT) {
			s->osymvalue.ovarnt.vtorecno = symno(p->ptr[2]);
			s->osymvalue.ovarnt.vtagno = symno(p->ptr[3]);
		}
		fwrite((char *) s, sizeof(*s), 1, symfp);
	}
}

/*
 * Dump a line number and the current object location counter.
 *
 * To save space the difference from the previous line number and offset
 * (one byte each) is dumped.
 */

LOCAL int oline = 0;
LOCAL int olc = HEADER_BYTES;

lineno(line)
int line;
{
	OBJLINE info;

	if (line != oline) {
		nlhdr.nlines++;
		nlsize += sizeof(OBJLINE);
		info.separate.lineincr = line - oline;
		info.separate.addrincr = ((unsigned short) (lc - olc));
		(void) putw((int) info.together, linesfp);
		oline = line;
		olc = (int) lc;
	}
}

/*
 * put out a file name entry, including:
 *
 *	the current line number for the new file
 *	the current location counter
 *	the string table address of the file name
 *	an index into the current line number information
 */

newfile(s, line)
char *s;
int line;
{
	FILETAB ft;

	nlhdr.nfiles++;
	nlsize += sizeof(FILETAB);
	ft.line = line;
	oline = line;
	if (lc == 0) {
		ft.addr = 0;
	} else {
		ft.addr = ((LINENO) lc - HEADER_BYTES );
	}
	ft.filename = (char *) nlhdr.stringsize;
	putstring(s);
	ft.lineindex = nlhdr.nlines;
	fwrite((char *) (&ft), sizeof(ft), 1, filesfp);
}

/*
 * put out a dummy symbol at the beginning of a block
 */

LOCAL putblock(s)
char *s;
{
	static OBJSYM zerosym;

	nlhdr.nsyms++;
	nlsize += sizeof(OBJSYM) + sizeof(int);
	(void) putw(0, symfp);
	zerosym.strindex = nlhdr.stringsize;
	putstring(s);
	fwrite((char *) (&zerosym), sizeof(zerosym), 1, symfp);
}

/*
 * put out a string to the string table file
 */

LOCAL putstring(s)
char *s;
{
	register char *p;

	for (p = s; *p != '\0'; p++) {
		putc(*p, strfp);
	}
	nlhdr.stringsize += (p - s + 1);
	nlsize += (p - s + 1);
	putc('\0', strfp);
}
#endif OBJ
