#ifndef lint
static char sccsid[] = "@(#)cmd2.c	3.33 4/24/85";
#endif

/*
 * Copyright (c) 1983 Regents of the University of California,
 * All rights reserved.  Redistribution permitted subject to
 * the terms of the Berkeley Software License Agreement.
 */

#include "defs.h"

char *help_shortcmd[] = {
	"#       Select window # and return to conversation mode",
	"%#      Select window # but stay in command mode",
	"escape  Return to conversation mode without changing window",
	"^^      Return to conversation mode and change to previous window",
	"c#      Close window #",
	"w       Open a new window",
	"m#      Move window #",
	"M#      Move window # to its previous position",
	"s#      Change the size of window #",
	"S#      Change window # to its previous size",
	"^Y      Scroll up one line",
	"^E      Scroll down one line",
	"^U      Scroll up half a window",
	"^D      Scroll down half a window",
	"^B      Scroll up a full window",
	"^F      Scroll down a full window",
	"h       Move cursor left",
	"j       Move cursor down",
	"k       Move cursor up",
	"l       Move cursor right",
	"^S      Stop output in current window",
	"^Q      Restart output in current window",
	"^L      Redraw screen",
	"^Z      Suspend",
	"q       Quit",
	":       Enter a long command",
	0
};
char *help_longcmd[] = {
	":alias name string ...  Make `name' an alias for `string ...'",
	":alias                  Show all aliases",
	":close # ...            Close windows",
	":close all              Close all windows",
	":cursor modes           Set the cursor modes",
	":echo # string ...      Print `string ...' in window #",
	":escape c               Set escape character to `c'",
	":foreground # flag      Make # a foreground window, if `flag' is true",
	":label # string         Set label of window # to `string'",
	":list                   List all open windows",
	":nline lines            Set default window buffer size to `lines'",
	":select #               Select window #",
	":shell string ...       Set default shell program to `string ...'",
	":source filename        Execute commands in `filename'",
	":terse flag             Set terse mode",
	":unalias name           Undefine `name' as an alias",
	":unset variable         Deallocate `variable'",
	":variable               List all variables",
	":window row col nrow ncol [nline label pty frame shell]",
	"                        Open a window at `row', `col' of size `nrow', `ncol',",
	"                        with `nline' lines in the buffer, and `label'",
	":write # string ...     Write `string ...' to window # as input",
	0
};

c_help()
{
	register struct ww *w;

	if ((w = openiwin(wwnrow - 3, "Help")) == 0) {
		error("Can't open help window: %s.", wwerror());
		return;
	}
	wwprintf(w, "The escape character is %c.\n", escapec);
	wwprintf(w, "(# represents one of the digits from 1 to 9.)\n\n");
	if (help_print(w, "Short commands", help_shortcmd) >= 0)
		(void) help_print(w, "Long commands", help_longcmd);
	closeiwin(w);
}

help_print(w, name, list)
register struct ww *w;
char *name;
register char **list;
{
	wwprintf(w, "%s:\n\n", name);
	while (*list)
		switch (more(w, 0)) {
		case 0:
			wwputs(*list++, w);
			wwputc('\n', w);
			break;
		case 1:
			wwprintf(w, "%s: (continued)\n\n", name);
			break;
		case 2:
			return -1;
		}
	return more(w, 1) == 2 ? -1 : 0;
}

c_quit()
{
	char oldterse = terse;

	setterse(0);
	wwputs("Really quit [yn]? ", cmdwin);
	wwcurtowin(cmdwin);
	while (wwpeekc() < 0)
		wwiomux();
	if (wwgetc() == 'y') {
		wwputs("Yes", cmdwin);
		quit++;
	} else
		wwputc('\n', cmdwin);
	setterse(!quit && oldterse);
}
