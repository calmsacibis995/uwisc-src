/*
 * Copyright (c) 1980 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 */

#ifndef lint
static char sccsid[] = "@(#)start.c	5.1 (Berkeley) 6/6/85";
#endif not lint
/*
 * Begin execution.
 *
 * For px, pstart does a traced exec to read in px and then stop.  But we
 * want control after px has read in the obj file and before it starts
 * executing.  The zeroth argument to px tells it to give us control
 * by sending itself a signal just prior to interpreting.
 *
 * We set a "END_BP" breakpoint at the end of the code so that the
 * process data doesn't disappear after the program terminates.
 */

#include "defs.h"
#include <signal.h>
#include "process.h"
#include "machine.h"
#include "main.h"
#include "breakpoint.h"
#include "source.h"
#include "object.h"
#include "mappings.h"
#include "sym.h"
#include "process.rep"

#   if (isvaxpx)
#       include "pxinfo.h"
#   endif

start(argv, infile, outfile)
char **argv;
char *infile, *outfile;
{
    char *cmd;

    setsigtrace();
#   if (isvaxpx)
	cmd = "px";
#   else
	cmd = argv[0];
#   endif
    pstart(process, cmd, argv, infile, outfile);
    if (process->status == STOPPED) {
#       if (isvaxpx)
	    TRAPARGS *ap, t;

	    pcont(process);
	    if (process->status != STOPPED) {
		if (option('t')) {
		    quit(process->exitval);
		} else {
		    panic("px exited with %d", process->exitval);
		}
	    }
	    dread(&ap, process->fp + 2*sizeof(int), sizeof(ap));
	    dread(&t, ap, sizeof(TRAPARGS));
	    if (t.nargs != 5) {
		if (option('t')) {
		    unsetsigtraces(process);
		    pcont(process);
		    quit(process->exitval);
		} else {
		    panic("start: args out of sync");
		}
	    }
	    DISPLAY = t.disp;
	    DP = t.dp;
	    ENDOFF = t.objstart;
	    PCADDRP = t.pcaddrp;
	    LOOPADDR = t.loopaddr;
#       endif
	pc = 0;
	curfunc = program;
	if (objsize != 0) {
	    addbp(lastaddr(), END_BP, NIL, NIL, NIL, 0);
	}
    }
}

/*
 * Note the termination of the program.  We do this so as to avoid
 * having the process exit, which would make the values of variables
 * inaccessible.
 *
 * Although the END_BP should really be deleted, it is taken
 * care of by fixbps the next time the program runs.
 */

endprogram()
{
    if (ss_variables) {
	prvarnews();
    }
    printf("\nexecution completed\n");
    curfunc = program;
    skimsource(srcfilename(pc));
    curline = lastlinenum;
    erecover();
}

/*
 * set up what signals we want to trace
 */

LOCAL setsigtrace()
{
    register int i;
    register PROCESS *p;

    p = process;
    psigtrace(p, SIGINT, TRUE);
    psigtrace(p, SIGTRAP, TRUE);
    psigtrace(p, SIGIOT, TRUE);
    psigtrace(p, SIGILL, TRUE);
    psigtrace(p, SIGBUS, TRUE);
}
