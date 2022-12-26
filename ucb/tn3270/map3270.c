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


#ifndef	LINT
static char sccsid[] = "@(#)map3270.c	2.5";
#endif	/* LINT */

/*	This program reads a description file, somewhat like /etc/termcap,
    that describes the mapping between the current terminals keyboard and
    a 3270 keyboard.
 */
#ifdef DOCUMENTATION_ONLY
/* here is a sample (very small) entry...

	# this table is sensitive to position on a line.  In particular,
	# a terminal definition for a terminal is terminated whenever a
	# (non-comment) line beginning in column one is found.
	#
	# this is an entry to map tvi924 to 3270 keys...
	v8|tvi924|924|televideo model 924 {
		pfk1 =	'\E1';
		pfk2 =	'\E2';
		clear = '^z';		# clear the screen
	}
 */
#endif /* DOCUMENTATION_ONLY */

#include <stdio.h>
#include <ctype.h>
#include <curses.h>

#define IsPrint(c)	(isprint(c) || ((c) == ' '))

#define LETS_SEE_ASCII
#include "m4.out"

#include "state.h"

/* this is the list of types returned by the lex processor */
#define LEX_CHAR	TC_HIGHEST		/* plain unadorned character */
#define	LEX_ESCAPED	LEX_CHAR+1		/* escaped with \ */
#define LEX_CARETED	LEX_ESCAPED+1		/* escaped with ^ */
#define LEX_END_OF_FILE	LEX_CARETED+1		/* end of file encountered */
#define LEX_ILLEGAL	LEX_END_OF_FILE+1	/* trailing escape character */

/* the following is part of our character set dependancy... */
#define	ESCAPE		0x1b
#define	TAB		0x09
#define NEWLINE		0x0a
#define	CARRIAGE_RETURN	0x0d

typedef struct {
    int type;		/* LEX_* - type of character */
    int value;		/* character this was */
} lexicon;

typedef struct {
    int		length;		/* length of character string */
    char	array[500];	/* character string */
} stringWithLength;

#define panic(s)	{ fprintf(stderr, s); exit(1); }

static state firstentry = { 0, TC_NULL, 0, 0 };
static state *headOfQueue = &firstentry;

/* the following is a primitive adm3a table, to be used when nothing
 * else seems to be avaliable.
 */

#ifdef	DEBUG
static int debug = 0;		/* debug flag (for debuggin tables) */
#endif	/* DEBUG */

static int doPaste = 1;			/* should we have side effects */
static char usePointer;			/* use pointer, or file */
static FILE *ourFile;
static char *environPointer = 0;	/* if non-zero, point to input
					 * string in core.
					 */
static char keys3a[] =
#include "default.map3270"		/* Define the default default */
			;

static	int	Empty = 1,		/* is the unget lifo empty? */
		Full = 0;		/* is the unget lifo full? */
static	lexicon	lifo[200];		/* character stack for parser */
static	int	rp = 0,			/* read pointer into lifo */
		wp = 0;			/* write pointer into lifo */

static int
GetC()
{
    int character;

    if (usePointer) {
	if (*environPointer) {
	    character = 0xff&*environPointer++;
	} else {
	    character = EOF;
	}
    } else {
	character = getc(ourFile);
    }
    return(character);
}

static lexicon
Get()
{
    lexicon c;
    register lexicon *pC = &c;
    register int character;

    if (!Empty) {
	*pC = lifo[rp];
	rp++;
	if (rp == sizeof lifo/sizeof (lexicon)) {
	    rp = 0;
	}
	if (rp == wp) {
	    Empty = 1;
	}
	Full = 0;
    } else {
	character = GetC();
	switch (character) {
	case EOF:
	    pC->type = LEX_END_OF_FILE;
	    break;
	case '^':
	    character = GetC();
	    if (!IsPrint(character)) {
		pC->type = LEX_ILLEGAL;
	    } else {
		pC->type = LEX_CARETED;
		if (character == '?') {
		    character |= 0x40;	/* rubout */
		} else {
		    character &= 0x1f;
		}
	    }
	    break;
	case '\\':
	    character = GetC();
	    if (!IsPrint(character)) {
		pC->type = LEX_ILLEGAL;
	    } else {
		pC->type = LEX_ESCAPED;
		switch (character) {
		case 'E': case 'e':
		    character = ESCAPE;
		    break;
		case 't':
		    character = TAB;
		    break;
		case 'n':
		    character = NEWLINE;
		    break;
		case 'r':
		    character = CARRIAGE_RETURN;
		    break;
		default:
		    pC->type = LEX_ILLEGAL;
		    break;
		}
	    }
	    break;
	default:
	    if ((IsPrint(character)) || isspace(character)) {
		pC->type = LEX_CHAR;
	    } else {
		pC->type = LEX_ILLEGAL;
	    }
	    break;
	}
	pC->value = character;
    }
    return(*pC);
}

static
UnGet(c)
lexicon c;			/* character to unget */
{
    if (Full) {
	fprintf(stderr, "attempt to put too many characters in lifo\n");
	panic("map3270");
	/* NOTREACHED */
    } else {
	lifo[wp] = c;
	wp++;
	if (wp == sizeof lifo/sizeof (lexicon)) {
	    wp = 0;
	}
	if (wp == rp) {
	    Full = 1;
	}
	Empty = 0;
    }
}

/* compare two strings, ignoring case */

ustrcmp(string1, string2)
register char *string1;
register char *string2;
{
    register int c1, c2;

    while (c1 = (unsigned char) *string1++) {
	if (isupper(c1)) {
	    c1 = tolower(c1);
	}
	if (isupper(c2 = (unsigned char) *string2++)) {
	    c2 = tolower(c2);
	}
	if (c1 < c2) {
	    return(-1);
	} else if (c1 > c2) {
	    return(1);
	}
    }
    if (*string2) {
	return(-1);
    } else {
	return(0);
    }
}


static stringWithLength *
GetQuotedString()
{
    lexicon lex;
    static stringWithLength output;	/* where return value is held */
    char *pointer = output.array;

    lex = Get();
    if ((lex.type != LEX_CHAR) || (lex.value != '\'')) {
	UnGet(lex);
	return(0);
    }
    while (1) {
	lex = Get();
	if ((lex.type == LEX_CHAR) && (lex.value == '\'')) {
	    break;
	}
	if ((lex.type == LEX_CHAR) && !IsPrint(lex.value)) {
	    UnGet(lex);
	    return(0);		/* illegal character in quoted string */
	}
	if (pointer >= output.array+sizeof output.array) {
	    return(0);		/* too long */
	}
	*pointer++ = lex.value;
    }
    output.length = pointer-output.array;
    return(&output);
}

#ifdef	NOTUSED
static stringWithLength *
GetCharString()
{
    lexicon lex;
    static stringWithLength output;
    char *pointer = output.array;

    lex = Get();

    while ((lex.type == LEX_CHAR) &&
			!isspace(lex.value) && (lex.value != '=')) {
	*pointer++ = lex.value;
	lex = Get();
	if (pointer >= output.array + sizeof output.array) {
	    return(0);		/* too long */
	}
    }
    UnGet(lex);
    output.length = pointer-output.array;
    return(&output);
}
#endif	/* NOTUSED */

static
GetCharacter(character)
int	character;		/* desired character */
{
    lexicon lex;

    lex = Get();

    if ((lex.type != LEX_CHAR) || (lex.value != character)) {
	UnGet(lex);
	return(0);
    }
    return(1);
}

#ifdef	NOTUSED
static
GetString(string)
char	*string;		/* string to get */
{
    lexicon lex;

    while (*string) {
	lex = Get();
	if ((lex.type != LEX_CHAR) || (lex.value != *string&0xff)) {
	    UnGet(lex);
	    return(0);		/* XXX restore to state on entry */
	}
	string++;
    }
    return(1);
}
#endif	/* NOTUSED */


static stringWithLength *
GetAlphaMericString()
{
    lexicon lex;
    static stringWithLength output;
    char *pointer = output.array;
#   define	IsAlnum(c)	(isalnum(c) || (c == '_')|| (c == '-'))

    lex = Get();

    if ((lex.type != LEX_CHAR) || !IsAlnum(lex.value)) {
	UnGet(lex);
	return(0);
    }

    while ((lex.type == LEX_CHAR) && IsAlnum(lex.value)) {
	*pointer++ = lex.value;
	lex = Get();
    }
    UnGet(lex);
    *pointer = 0;
    output.length = pointer-output.array;
    return(&output);
}


/* eat up characters until a new line, or end of file.  returns terminating
	character.
 */

static lexicon
EatToNL()
{
    lexicon lex;

    lex = Get();

    while (!((lex.type != LEX_ESCAPED) && (lex.value == '\n')) &&
				(!(lex.type == LEX_END_OF_FILE))) {
	lex = Get();
    }
    if (lex.type != LEX_END_OF_FILE) {
	return(Get());
    } else {
	return(lex);
    }
}


static void
GetWS()
{
    lexicon lex;

    lex = Get();

    while ((lex.type == LEX_CHAR) &&
			(isspace(lex.value) || (lex.value == '#'))) {
	if (lex.value == '#') {
	    lex = EatToNL();
	} else {
	    lex = Get();
	}
    }
    UnGet(lex);
}

static void
FreeState(pState)
state *pState;
{
    free((char *)pState);
}


static state *
GetState()
{
    state *pState;
    char *malloc();

    pState = (state *) malloc(sizeof *pState);

    pState->result = TC_NULL;
    pState->next = 0;

    return(pState);
}


static state *
FindMatchAtThisLevel(pState, character)
state	*pState;
int	character;
{
    while (pState) {
	if (pState->match == character) {
	    return(pState);
	}
	pState = pState->next;
    }
    return(0);
}


static state *
PasteEntry(head, string, count, identifier)
state			*head;		/* points to who should point here... */
char			*string;	/* which characters to paste */
int			count;		/* number of character to do */
char			*identifier;	/* for error messages */
{
    state *pState, *other;

    if (!doPaste) {		/* flag to not have any side effects */
	return((state *)1);
    }
    if (!count) {
	return(head);	/* return pointer to the parent */
    }
    if ((head->result != TC_NULL) && (head->result != TC_GOTO)) {
	/* this means that a previously defined sequence is an initial
	 * part of this one.
	 */
	fprintf(stderr, "Conflicting entries found when scanning %s\n",
		identifier);
	return(0);
    }
#   ifdef	DEBUG
	if (debug) {
	    fprintf(stderr, "%s", unctrl(*string));
	}
#   endif	/* DEBUG */
    pState = GetState();
    pState->match = *string;
    if (head->result == TC_NULL) {
	head->result = TC_GOTO;
	head->address = pState;
	other = pState;
    } else {		/* search for same character */
	if (other = FindMatchAtThisLevel(head->address, *string)) {
	    FreeState(pState);
	} else {
	    pState->next = head->address;
	    head->address = pState;
	    other = pState;
	}
    }
    return(PasteEntry(other, string+1, count-1, identifier));
}

static
GetInput(tc, identifier)
int tc;
char *identifier;		/* entry being parsed (for error messages) */
{
    stringWithLength *outputString;
    state *head;
    state fakeQueue;

    if (doPaste) {
	head = headOfQueue;	/* always points to level above this one */
    } else {
	head = &fakeQueue;	/* don't have any side effects... */
    }

    if (!(outputString = GetQuotedString())) {
	return(0);
    } else if (IsPrint(outputString->array[0])) {
	fprintf(stderr,
	 "first character of sequence for %s is not a control type character\n",
		identifier);
	return(0);
    } else {
	if (!(head = PasteEntry(head, outputString->array,
				outputString->length, identifier))) {
	    return(0);
	}
	GetWS();
	while (outputString = GetQuotedString()) {
	    if (!(head = PasteEntry(head, outputString->array, outputString->length, identifier))) {
		return(0);
	    }
	    GetWS();
	}
    }
    if (!doPaste) {
	return(1);
    }
    if ((head->result != TC_NULL) && (head->result != tc)) {
	/* this means that this sequence is an initial part
	 * of a previously defined one.
	 */
	fprintf(stderr, "Conflicting entries found when scanning %s\n",
		identifier);
	return(0);
    } else {
	head->result = tc;
	return(1);		/* done */
    }
}

static
GetTc(string)
char *string;
{
    register TC_Ascii_t *Tc;

    for (Tc = TC_Ascii;
		Tc < TC_Ascii+sizeof TC_Ascii/sizeof (TC_Ascii_t); Tc++) {
	if (!ustrcmp(string, Tc->tc_name)) {
#	    ifdef	DEBUG
		if (debug) {
		    fprintf(stderr, "%s = ", Tc->tc_name);
		}
#	    endif	/* DEBUG */
	    return(Tc->tc_value&0xff);
	}
    }
    return(0);
}
static
GetDefinition()
{
    stringWithLength *string;
    int Tc;

    GetWS();
    if (!(string = GetAlphaMericString())) {
	return(0);
    }
    string->array[string->length] = 0;
    if (doPaste) {
	if (!(Tc = GetTc(string->array))) {
	    fprintf(stderr, "%s: unknown 3270 key identifier\n", string->array);
	    return(0);
	}
	if (Tc < TC_LOWEST_USER) {
	    fprintf(stderr, "%s is not allowed to be specified by a user.\n",
			string->array);
	    return(0);
	}
    } else {
	Tc = TC_LOWEST_USER;
    }
    GetWS();
    if (!GetCharacter('=')) {
	fprintf(stderr,
		"Required equal sign after 3270 key identifier %s missing\n",
			string->array);
	return(0);
    }
    GetWS();
    if (!GetInput(Tc, string->array)) {
	fprintf(stderr, "Missing definition part for 3270 key %s\n",
				string->array);
	return(0);
    } else {
	GetWS();
	while (GetCharacter('|')) {
#	    ifdef	DEBUG
		if (debug) {
		    fprintf(stderr, " or ");
		}
#	    endif	/* DEBUG */
	    GetWS();
	    if (!GetInput(Tc, string->array)) {
		fprintf(stderr, "Missing definition part for 3270 key %s\n",
					string->array);
		return(0);
	    }
	    GetWS();
	}
    }
    GetWS();
    if (!GetCharacter(';')) {
	fprintf(stderr, "Missing semi-colon for 3270 key %s\n", string->array);
	return(0);
    }
#   ifdef	DEBUG
	if (debug) {
	    fprintf(stderr, ";\n");
	}
#   endif	/* DEBUG */
    return(1);
}


static
GetDefinitions()
{
    if (!GetDefinition()) {
	return(0);
    } else {
	while (GetDefinition()) {
	    ;
	}
    }
    return(1);
}

static
GetBegin()
{
    GetWS();
    if (!GetCharacter('{')) {
	return(0);
    }
    return(1);
}

static
GetEnd()
{
    GetWS();
    if (!GetCharacter('}')) {
	return(0);
    }
    return(1);
}

static
GetName()
{
    if (!GetAlphaMericString()) {
	return(0);
    }
    GetWS();
    while (GetAlphaMericString()) {
	GetWS();
    }
    return(1);
}

static
GetNames()
{
    GetWS();
    if (!GetName()) {
	return(0);
    } else {
	GetWS();
        while (GetCharacter('|')) {
	    GetWS();
	    if (!GetName()) {
		return(0);
	    }
	}
    }
    return(1);
}

static
GetEntry0()
{
    if (!GetBegin()) {
	fprintf(stderr, "no '{'\n");
	return(0);
    } else if (!GetDefinitions()) {
	fprintf(stderr, "unable to parse the definitions\n");
	return(0);
    } else if (!GetEnd()) {
	fprintf(stderr, "no '}'\n");
	return(0);
    } else {
	/* done */
	return(1);
    }
}


static
GetEntry()
{
    if (!GetNames()) {
	fprintf(stderr, "illegal name field in entry\n");
	return(0);
    } else {
	return(GetEntry0());
    }
}

/* position ourselves within a given filename to the entry for the current
 *	TERM variable
 */

Position(filename, termPointer)
char *filename;
char *termPointer;
{
    lexicon lex;
    stringWithLength *name = 0;
    stringWithLength *oldName;
#   define	Return(x) {doPaste = 1; return(x);}

    doPaste = 0;

    if ((ourFile = fopen(filename, "r")) == NULL) {
	fprintf(stderr, "Unable to open file %s\n", filename);
	Return(0);
    }
    lex = Get();
    while (lex.type != LEX_END_OF_FILE) {
	UnGet(lex);
	/* now, find an entry that is our type. */
	GetWS();
	oldName = name;
	if (name = GetAlphaMericString()) {
	    if (!ustrcmp(name->array, termPointer)) {
		/* need to make sure there is a name here... */
		lex.type = LEX_CHAR;
		lex.value = 'a';
		UnGet(lex);
		Return(1);
	    }
	} else if (GetCharacter('|')) {
	    ;		/* more names coming */
	} else {
	    lex = Get();
	    UnGet(lex);
	    if (lex.type != LEX_END_OF_FILE) {
		    if (!GetEntry0()) {	/* start of an entry */
			fprintf(stderr, "error was in entry for %s in file %s\n",
			    (oldName)? oldName->array:"(unknown)", filename);
		    Return(0);
		}
	    }
	}
	lex = Get();
    }
    fprintf(stderr, "Unable to find entry for %s in file %s\n", termPointer,
		    filename);
    Return(0);
}
/* InitControl - our interface to the outside.  What we should
    do is figure out terminal type, set up file pointer (or string
    pointer), etc.
 */

state *
InitControl()
{
    char *getenv();
    int GotIt;
    char *termPointer;

    environPointer = getenv("MAP3270");

    if ((!environPointer) || (*environPointer == '/')) {
	usePointer = 0;
	GotIt = 0;

	termPointer = getenv("TERM");
	if (!termPointer) {
	    fprintf(stderr,
		        "TERM environment variable (that defines the kind of terminal you are using)\n");
	    fprintf(stderr,
			"is not set.  To set it, say 'setenv TERM <type>'\n");
	} else {
	    if (environPointer) {
		GotIt = Position(environPointer, termPointer);
	    }
	    if (!GotIt) {
		GotIt = Position("/etc/map3270", termPointer);
	    }
	}
	if (!GotIt) {
	    if (environPointer) {
		GotIt = Position(environPointer, "unknown");
	    }
	    if (!GotIt) {
		GotIt = Position("/etc/map3270", "unknown");
	    }
	}
	if (!GotIt) {
	    fprintf(stderr, "Using default key mappings.\n");
	    environPointer = keys3a;	/* use incore table */
	    usePointer = 1;		/* flag use of non-file */
	}
    } else {
	usePointer = 1;
    }
    (void) GetEntry();
    return(firstentry.address);
}
