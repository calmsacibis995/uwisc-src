/*
 * @(#)var.h	3.7 4/24/85
 */

/*
 * Copyright (c) 1983 Regents of the University of California,
 * All rights reserved.  Redistribution permitted subject to
 * the terms of the Berkeley Software License Agreement.
 */

struct var {
	struct var *r_left;
	struct var *r_right;
	char *r_name;
	struct value r_val;
};

struct var *var_set1();
struct var *var_setstr1();
struct var *var_setnum1();
struct var **var_lookup1();

#define var_set(n, v)		var_set1(&var_head, n, v)
#define var_setstr(n, s)	var_setstr1(&var_head, n, s)
#define var_setnum(n, i)	var_setnum1(&var_head, n, i)
#define var_unset(n)		var_unset1(&var_head, n)
#define var_lookup(n)		(*var_lookup1(&var_head, n))
#define var_walk(f, a)		var_walk1(var_head, f, a)

struct var *var_head;		/* secret, shhh */
