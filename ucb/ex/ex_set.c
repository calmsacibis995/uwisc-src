/* Copyright (c) 1980 Regents of the University of California */
static char *sccsid = "@(#)ex_set.c	6.3 10/30/80";
#include "ex.h"
#include "ex_temp.h"
#include "ex_tty.h"

/*
 * Set command.
 */
char	optname[LBSIZE];

set()
{
	register char *cp, *t, *p;
	register struct option *op;
	register int c;
	int overflow = 0;
	char *index();
	char buf[20];
	bool no;
	short speed;
	extern short ospeed;

	setnoaddr();
	if (skipend()) {
		if (peekchar() != EOF)
			ignchar();
		propts();
		return;
	}
	do {
		cp = optname;
		do {
			if (cp < &optname[LBSIZE - 2])
				*cp++ = getchar();
			else {
				overflow++;
				getchar();
			}
			if( cp != optname && (*(cp-1) == '\\' && peekchar() != EOF))
				*(cp-1) = getchar();
		} while (!isspace(c=peekchar()) && c != EOF);
		if(overflow) {
			serror("Option too long.");
		}
		*cp = 0;
		cp = optname;
		if (eq("all", cp)) {
			if (inopen)
				pofix();
			prall();
			goto next;
		}
		if(*cp == '?') { /* a conditional set */
			if(!*++cp) {
				serror("%s: No such option@- 'set all' gives all option values", "?");
			}
			/* copy arg */
			for(t = buf, p = cp; ( (*++p != ':') && (*t++ = *p)) && (t < &buf[19]););
			*t = '\0';
			switch(*cp) {
			 case 'T':	/* set based on term type */
				if(strcmp(buf,ttytype)) {
					goto next;
				}
				break;

			 case 'S':	/* set based on term speed */
				speed = atoi(buf);
				if(	   (speed == 300  && ospeed != B300)
					|| (speed == 1200 && ospeed != B1200)
					|| (speed == 2400 && ospeed != B2400)
					|| (speed == 4800 && ospeed != B4800)
					|| (speed == 9600 && ospeed != B9600) )
					goto next;
				break;
			 default:
				serror("%c: No such condition@- 'set all' gives all option values", *cp);
			}
			cp = p;
			if(*cp == ':')
				cp++;
		}
		no = 0;
		if (cp[0] == 'n' && cp[1] == 'o') {
			cp += 2;
			no++;
		}
		/* separate out the option name, t now points to the [ = value] */
		c = 0;
		if( (t = index (cp,'=')) == NULL)
			t = "";
		else {
			c = '=';
			*t++ = '\0';
		}

		/* Implement w300, w1200, and w9600 specially */
		if (eq(cp, "w300")) {
			if (ospeed >= B1200) {
				goto next;
			}
			cp = "window";
		} else if (eq(cp, "w1200")) {
			if (ospeed < B1200 || ospeed >= B2400)
				goto next;
			cp = "window";
		} else if (eq(cp, "w9600")) {
			if (ospeed < B2400)
				goto next;
			cp = "window";
		}
		for (op = options; op < &options[NOPTS]; op++)
			if (eq(op->oname, cp) || op->oabbrev && eq(op->oabbrev, cp))
				break;
		if (op->oname == 0)
			serror("%s: No such option@- 'set all' gives all option values", cp);
		if (*t == '?') {
printone:
			propt(op);
			noonl();
			goto next;
		}
		if (op->otype == ONOFF) {
			op->ovalue = 1 - no;
			if (op == &options[PROMPT])
				oprompt = 1 - no;
			goto next;
		}
		if (no)
			serror("Option %s is not a toggle", op->oname);
		if (*t == '\0' )
			goto printone;
		if (c != '=')
			serror("Missing =@in assignment to option %s", op->oname);
		switch (op->otype) {

		case NUMERIC:
			if (!isdigit(*t))
				error("Digits required@after =");
			op->ovalue = atoi(t);
			if (value(TABSTOP) <= 0)
				value(TABSTOP) = TABS;
			if (op == &options[WINDOW]) {
				if (value(WINDOW) >= LINES)
					value(WINDOW) = LINES-1;
				vsetsiz(value(WINDOW));
			}
			break;

		case STRING:
		case OTERM:
			if (op->otype == OTERM) {
/*
 * At first glance it seems like we shouldn't care if the terminal type
 * is changed inside visual mode, as long as we assume the screen is
 * a mess and redraw it. However, it's a much harder problem than that.
 * If you happen to change from 1 crt to another that both have the same
 * size screen, it's OK. But if the screen size if different, the stuff
 * that gets initialized in vop() will be wrong. This could be overcome
 * by redoing the initialization, e.g. making the first 90% of vop into
 * a subroutine. However, the most useful case is where you forgot to do
 * a setenv before you went into the editor and it thinks you're on a dumb
 * terminal. Ex treats this like hardcopy and goes into HARDOPEN mode.
 * This loses because the first part of vop calls oop in this case.
 * The problem is so hard I gave up. I'm not saying it can't be done,
 * but I am saying it probably isn't worth the effort.
 */
				if (inopen)
error("Can't change type of terminal from within open/visual");
				setterm(t);
			} else {
				if(*t == '&') {
					*t = ' ';
					(void) strcat(op->osvalue,t);
				} else {
					CP(op->osvalue, t);
				}
				op->odefault = 1;
			}
			break;
		}
next:
		flush();
	} while (!skipend());
	eol();
}

setend()
{

	return (iswhite(peekchar()) || endcmd(peekchar()));
}

prall()
{
	register int incr = (NOPTS + 2) / 3;
	register int rows = incr;
	register struct option *op = options;

	for (; rows; rows--, op++) {
		propt(op);
		tab(24);
		propt(&op[incr]);
		if (&op[2*incr] < &options[NOPTS]) {
			tab(56);
			propt(&op[2 * incr]);
		}
		putNFL();
	}
}

propts()
{
	register struct option *op;

	for (op = options; op < &options[NOPTS]; op++) {
#ifdef V6
		if (op == &options[TERM])
#else
		if (op == &options[TTYTYPE])
#endif
			continue;
		switch (op->otype) {

		case ONOFF:
		case NUMERIC:
			if (op->ovalue == op->odefault)
				continue;
			break;

		case STRING:
			if (op->odefault == 0)
				continue;
			break;
		}
		propt(op);
		putchar(' ');
	}
	noonl();
	flush();
}

propt(op)
	register struct option *op;
{
	register char *name;
	
	name = op->oname;

	switch (op->otype) {

	case ONOFF:
		printf("%s%s", op->ovalue ? "" : "no", name);
		break;

	case NUMERIC:
		printf("%s=%d", name, op->ovalue);
		break;

	case STRING:
	case OTERM:
		printf("%s=%s", name, op->osvalue);
		break;
	}
}
