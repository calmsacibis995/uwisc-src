/*
 *	Copyright 1984, 1985 by the Regents of the University of
 *	California and by Gregory Glenn Minshall.
 *
 *	Permission to use, copy, modify, and distribute these
 *	programs and their documentation for any purpose and
 *	without fee is hereby granted, provided that this
 *	copyright and permission appear on all copies and
 *	supporting documentation, the name of the Regents of
 *	the University of California not be used in advertising
 *	or publicity pertaining to distribution of the programs
 *	without specific prior permission, and notice be given in
 *	supporting documentation that copying and distribution is
 *	by permission of the Regents of the University of California
 *	and by Gregory Glenn Minshall.  Neither the Regents of the
 *	University of California nor Gregory Glenn Minshall make
 *	representations about the suitability of this software
 *	for any purpose.  It is provided "as is" without
 *	express or implied warranty.
 */


/* this takes characters from the keyboard, and produces 3270 keystroke
	codes
 */

#ifndef	lint
static	char	sccsid[] = "@(#)termin.c	2.1	4/11/85";
#endif	/* ndef lint */

#include <ctype.h>

#include "m4.out"		/* output of termcodes.m4 */
#include "state.h"

#define IsControl(c)	(!isprint(c) && ((c) != ' '))

#define NextState(x)	(x->next)

/* XXX temporary - hard code in the state table */

#define MATCH_ANY 0xff			/* actually, match any character */


static char
	ourBuffer[100],		/* where we store stuff */
	*ourPHead = ourBuffer,	/* first character in buffer */
	*ourPTail = ourBuffer;	/* where next character goes */

static state
	*headOfControl = 0;	/* where we enter code state table */

#define FullChar	(ourPTail == ourBuffer+sizeof ourBuffer)
#define EmptyChar	(ourPTail == ourPHead)


/* AddChar - put a character in our buffer */

static
AddChar(c)
int	c;
{
    if (!FullChar) {
	*ourPTail++ = (char) c;
    } else {
	RingBell();
    }
}

/* FlushChar - put everything where it belongs */

static
FlushChar()
{
    ourPTail = ourBuffer;
    ourPHead = ourBuffer;
}


int
TerminalIn()
{
    /* send data from us to next link in stream */
    int count;

    count = 0;

    if (!EmptyChar) {			/* send up the link */
	count += DataFrom3270(ourPHead, ourPTail-ourPHead);
	ourPHead += count;
	if (EmptyChar) {
	    FlushChar();
	}
    }
	/* return value answers question: "did we do anything useful?" */
    return(count? 1:0);
}

int
DataFromTerminal(buffer, count)
register char	*buffer;		/* the data read in */
register int	count;			/* how many bytes in this buffer */
{
    register state *regControlPointer;
    register char c;
    register int result;
    int origCount;

    static int InControl;
    static int WaitingForSynch = 0;
    static state *controlPointer;
    extern state *InitControl();

    if (!headOfControl) {
	/* need to initialize */
	headOfControl = InitControl();
	if (!headOfControl) {		/* should not occur */
	    quit();
	}
    }


    origCount = count;

    while (count) {
	c = *buffer++&0x7f;
	count--;

	if (!InControl && !IsControl(c)) {
	    AddChar(c);			/* add ascii character */
	} else {
	    if (!InControl) {		/* first character of sequence */
		InControl = 1;
		controlPointer = headOfControl;
	    }
	    /* control pointer points to current position in state table */
	    for (regControlPointer = controlPointer; ;
			regControlPointer = NextState(regControlPointer)) {
		if (!regControlPointer) {	/* ran off end */
		    RingBell();
		    regControlPointer = headOfControl;
		    InControl = 0;
		    break;
		}
		if ((regControlPointer->match == c) /* hit this character */
			|| (regControlPointer->match == MATCH_ANY)) {
		    result = regControlPointer->result;
		    if (result == TC_GOTO) {
			regControlPointer = regControlPointer->address;
			break;			/* go to next character */
		    }
		    if (WaitingForSynch) {
			if (result == TC_SYNCH) {
			    WaitingForSynch = 0;
			} else {
			    RingBell();
			}
		    }
		    else if (result == TC_FLINP) {
			FlushChar();		/* Don't add FLINP */
		    } else {
			if (result == TC_MASTER_RESET) {
			    FlushChar();
			}
			AddChar(result);		/* add this code */
		    }
		    InControl = 0;	/* out of control now */
		    break;
		}
	    }
	    controlPointer = regControlPointer;		/* save state */
	}
    }
    (void) TerminalIn();			/* try to send data */
    return(origCount-count);
}
