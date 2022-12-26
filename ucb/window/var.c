#ifndef lint
static char sccsid[] = "@(#)var.c	3.8 4/24/85";
#endif

/*
 * Copyright (c) 1983 Regents of the University of California,
 * All rights reserved.  Redistribution permitted subject to
 * the terms of the Berkeley Software License Agreement.
 */

#include "value.h"
#include "var.h"
#include "string.h"

char *malloc();

struct var *
var_set1(head, name, v)
struct var **head;
char *name;
struct value *v;
{
	register struct var **p;
	register struct var *r;
	struct value val;

	/* do this first, easier to recover */
	val = *v;
	if (val.v_type == V_STR && val.v_str != 0 &&
	    (val.v_str = str_cpy(val.v_str)) == 0)
		return 0;
	if (*(p = var_lookup1(head, name)) == 0) {
		r = (struct var *) malloc(sizeof (struct var));
		if (r == 0) {
			val_free(val);
			return 0;
		}
		if ((r->r_name = str_cpy(name)) == 0) {
			val_free(val);
			free((char *) r);
			return 0;
		}
		r->r_left = r->r_right = 0;
		*p = r;
	} else {
		r = *p;
		val_free(r->r_val);
	}
	r->r_val = val;
	return r;
}

struct var *
var_setstr1(head, name, str)
struct var **head;
char *name;
char *str;
{
	struct value v;

	v.v_type = V_STR;
	v.v_str = str;
	return var_set1(head, name, &v);
}

struct var *
var_setnum1(head, name, num)
struct var **head;
char *name;
int num;
{
	struct value v;

	v.v_type = V_NUM;
	v.v_num = num;
	return var_set1(head, name, &v);
}

var_unset1(head, name)
struct var **head;
char *name;
{
	register struct var **p;
	register struct var *r;

	if (*(p = var_lookup1(head, name)) == 0)
		return -1;
	r = *p;
	*p = r->r_left;
	while (*p != 0)
		p = &(*p)->r_right;
	*p = r->r_right;
	val_free(r->r_val);
	str_free(r->r_name);
	free((char *) r);
	return 0;
}

struct var **
var_lookup1(p, name)
register struct var **p;
register char *name;
{
	register cmp;

	while (*p != 0) {
		if ((cmp = strcmp(name, (*p)->r_name)) < 0)
			p = &(*p)->r_left;
		else if (cmp > 0)
			p = &(*p)->r_right;
		else
			break;
	}
	return p;
}

var_walk1(r, func, a)
register struct var *r;
int (*func)();
{
	if (r == 0)
		return 0;
	if (var_walk1(r->r_left, func, a) < 0 || (*func)(a, r) < 0
	    || var_walk1(r->r_right, func, a) < 0)
		return -1;
	return 0;
}
