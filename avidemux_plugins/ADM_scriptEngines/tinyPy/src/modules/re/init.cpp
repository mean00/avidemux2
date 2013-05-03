/*
 * regular expression module
 *
 * Important Note: do not support group name index
 *
 * $Id: init.c 144 2009-04-20 16:23:22Z ybc2084 $
 */

#include <stdio.h>
#include <assert.h>
#include <tinypy.h>
#include "regexpr.cpp"
 
/* tinypy API to be use in this unit */
extern tp_obj tp_data(TP,int magic,void *v);
extern tp_obj tp_object_new(TP);
extern tp_obj tp_object(TP);
extern tp_obj tp_method(TP,tp_obj self,tp_obj v(TP));
extern tp_obj tp_string_copy(TP, const char *s, int n);
extern tp_obj tp_list(TP);
extern tp_obj tp_copy(TP);

/* last error message */
static const char * LastError = NULL;

/* lower level regex object */
typedef struct {
	struct re_pattern_buffer re_patbuf;	/* The compiled expression */
	struct re_registers re_regs; 		/* The registers from the last match */
	char re_fastmap[256];				/* Storage for fastmap */
	unsigned char *re_translate;		/* String object for translate table */
	unsigned char *re_lastok;			/* String object last matched/searched */

	/* supplementary */
	int re_errno;						/* error num */
	int re_syntax;						/* syntax */
} regexobject;

/* local declarations */
static regexobject* getre(TP, tp_obj rmobj);
static tp_obj match_obj_group(TP);
static tp_obj match_obj_groups(TP);
static tp_obj match_obj_start(TP);
static tp_obj match_obj_end(TP);
static tp_obj match_obj_span(TP);

/*
 * helper function: return lower level regex object
 * rmobj	- regex or match object
 */
static regexobject * getre(TP, tp_obj rmobj)
{
	tp_obj reobj_data = tp_get(tp, rmobj, tp_string("__data__"));
	regexobject *re = NULL;

	/* validate magic */
	if (reobj_data.data.magic != sizeof(regexobject)) {
		LastError = "broken regex object";
		return (NULL);
	}
	re = (regexobject*)reobj_data.data.val;
	assert(re);

	return (re);
}

/*
 * derive match object from regex object
 */
static tp_obj match_object(TP, tp_obj reobj)
{
	tp_obj mo = tp_object(tp);	/* match object */
	tp_obj redata;				/* regex object data */
	tp_obj madata;				/* match object data */
	regexobject *re = NULL;		/* lower level regex object */

	redata = tp_get(tp, reobj, tp_string("__data__"));
	re = (regexobject *)redata.data.val;
	assert(re);
	madata = tp_data(tp, (int)sizeof(regexobject), re);

	tp_set(tp, mo, tp_string("group"),	tp_method(tp, mo, match_obj_group));
	tp_set(tp, mo, tp_string("groups"),	tp_method(tp, mo, match_obj_groups));
	tp_set(tp, mo, tp_string("start"),	tp_method(tp, mo, match_obj_start));
	tp_set(tp, mo, tp_string("end"),	tp_method(tp, mo, match_obj_end));
	tp_set(tp, mo, tp_string("span"),	tp_method(tp, mo, match_obj_span));
	tp_set(tp, mo, tp_string("__data__"), madata);

	return (mo);
}

/*
 * FUNC: regexobj.search(str[,pos=0])
 * self		- regex object
 * str		- string to be searched
 * pos		- optional starting offset
 *
 * RETURN:
 * match object	- when matched
 * None			- not matched
 */
static tp_obj regex_obj_search(TP)
{
	tp_obj self = TP_OBJ();		/* regex object */
	tp_obj str = TP_STR();
	tp_obj pos = TP_DEFAULT(tp_number(0));
	tp_obj maobj;				/* match object */
	regexobject *re = NULL;
	int r = -2;					/* -2 indicate exception */
	int range;

	if (pos.number.val < 0 || pos.number.val > str.string.len) {
		LastError = "search offset out of range";
		goto exception;
	}
	range = str.string.len - pos.number.val;

	re = getre(tp, self);
	re->re_lastok = NULL;
	r = re_search(&re->re_patbuf, (unsigned char *)str.string.val, 
			str.string.len, pos.number.val, range, &re->re_regs);

	/* cannot match pattern */
	if (r == -1)
		goto notfind;

	/* error occurred */
	if (r == -2)
		goto exception;

	/* matched */
	re->re_lastok = (unsigned char *)str.string.val;

	/* match obj */
	maobj = match_object(tp, self);

	return (maobj);	

notfind:
	re->re_lastok = NULL;
	return (tp_None);
exception:
	re->re_lastok = NULL;
	tp_raise(tp_None, tp_string("regex search error"));
}

/*
 * FUNC: regexobj.match(str[,pos=0])
 * self		- regex object
 * str		- string to be matched
 * pos		- optional starting position
 *
 * RETURN:
 * match object	- when matched
 * None			- not matched
 */
static tp_obj regex_obj_match(TP)
{
	tp_obj self = TP_OBJ();		/* regex object */
	tp_obj str = TP_STR();
	tp_obj pos = TP_DEFAULT(tp_number(0));
	tp_obj maobj;				/* match object */
	regexobject *re = NULL;
	int r = -2;					/* -2 indicate exception */

	re = getre(tp, self);
	re->re_lastok = NULL;
	r = re_match(&re->re_patbuf, (unsigned char *)str.string.val, 
			str.string.len, pos.number.val, &re->re_regs);

	/* cannot match pattern */
	if (r == -1)
		goto nomatch;

	/* error occurred */
	if (r == -2)
		goto exception;

	/* matched */
	re->re_lastok = (unsigned char *)str.string.val;

	/* match obj */
	maobj = match_object(tp, self);

	return (maobj);	

nomatch:
	re->re_lastok = NULL;
	return (tp_None);
exception:
	re->re_lastok = NULL;
	tp_raise(tp_None, tp_string("regex match error"));
}

/*
 * regex object split()
 * self		- regex object
 * restr	- regex string
 * maxsplit	- max split field, default 0, mean no limit
 */
static tp_obj regex_obj_split(TP)
{
	tp_obj self		= TP_OBJ();	/* regex object */
	tp_obj restr	= TP_OBJ();	/* string */
	tp_obj maxsplit = TP_DEFAULT(tp_number(0));
	tp_obj maobj;				/* match object */
	regexobject *re = NULL;		/* lower level regex object */
	tp_obj result	= tp_list(tp);
	tp_obj grpstr;				/* group string */
	int	slen;					/* string length */
	int srchloc;				/* search location */

	/* maxsplit == 0 means no limit */
	if ((int)maxsplit.number.val == 0)
		maxsplit.number.val = RE_NREGS;
	assert(maxsplit.number.val > 0);

	srchloc = 0;
	slen = strlen((char *)restr.string.val);

	do {
		/* generate a temp match object */
		tp_params_v(tp, 3, self, restr, tp_number(srchloc));
		maobj = regex_obj_search(tp);
		if (!tp_bool(tp, maobj)) 
			break;

		re = getre(tp, maobj);
		if (re->re_lastok == NULL) {
			tp_raise(tp_None, tp_string("no match for split()"));
		}

		/* extract fields */
		if ((int)maxsplit.number.val > 0) {
			int start = re->re_regs.start[0];
			int end   = re->re_regs.end[0];
			/*printf("%s:start(%d),end(%d)\n", __func__, start, end);*/
			if (start < 0 || end < 0)
				break;

			grpstr = tp_string_copy(tp, 
					(const char *)re->re_lastok + srchloc, start - srchloc);

			if (tp_bool(tp, grpstr)) {
				tp_set(tp, result, tp_None, grpstr);
				maxsplit.number.val--;
			}

			srchloc = end;
		}
	} while (srchloc < slen && (int)maxsplit.number.val > 0);

	/* collect remaining string, if necessary */
	if (srchloc < slen) {
		grpstr = tp_string_copy(tp, 
				(const char *)restr.string.val + srchloc, slen - srchloc);
		if (tp_bool(tp, grpstr))
			tp_set(tp, result, tp_None, grpstr);
	}

	return (result);
}

/*
 * regex object findall()
 * self		- regex object
 * restr	- regex string
 * pos		- starting position, default 0
 */
static tp_obj regex_obj_findall(TP)
{
	tp_obj self		= TP_OBJ();	/* regex object */
	tp_obj restr	= TP_OBJ();	/* string */
	tp_obj pos		= TP_DEFAULT(tp_number(0));
	tp_obj maobj;				/* match object */
	regexobject *re = NULL;		/* lower level regex object */
	tp_obj result	= tp_list(tp);
	tp_obj grpstr;				/* group string */
	int	slen;					/* string length */
	int srchloc;				/* search location */

	srchloc = (int)pos.number.val;
	slen	= strlen((char *)restr.string.val);
	if (srchloc < 0 || srchloc >= slen)
		tp_raise(tp_None, tp_string("starting position out of range"));

	do {
		/* generate a temp match object */
		tp_params_v(tp, 3, self, restr, tp_number(srchloc));
		maobj = regex_obj_search(tp);
		if (!tp_bool(tp, maobj)) 
			break;

		re = getre(tp, maobj);
		if (re->re_lastok == NULL) {
			tp_raise(tp_None, tp_string("no match for findall()"));
		}

		/* extract fields */
		if (srchloc < slen) {
			int start = re->re_regs.start[0];
			int end   = re->re_regs.end[0];
			/*printf("%s:start(%d),end(%d)\n", __func__, start, end);*/
			if (start < 0 || end < 0)
				break;

			grpstr = tp_string_copy(tp, 
					(const char *)re->re_lastok + start, end - start);

			if (tp_bool(tp, grpstr)) {
				tp_set(tp, result, tp_None, grpstr);
			}

			srchloc = end;
		}
	} while (srchloc < slen);

	return (result);
}

/*
 * FUNC: matchobj.group([group1, ...])
 * self		- match object
 * args		- optional group indices, default 0
 *
 * return specified group.
 */
static tp_obj match_obj_group(TP)
{
	tp_obj self = TP_OBJ();		/* match object */
	tp_obj grpidx;				/* a group index */
	regexobject *re = NULL;
	int indices[RE_NREGS];
	int start;
	int end;
	int i;
	int single = 0;				/* single group index? */
	tp_obj result;

	/* get lower level regex object representation */
	re = getre(tp, self);
	if (re->re_lastok == NULL)
		tp_raise(tp_None, 
				tp_string("group() only valid after successful match/search"));

	for (i = 0; i < RE_NREGS; i++)
		indices[i] = -1;

	/*
	 * if no group index provided, supply default group index 0; else
	 * fill in indices[] with provided group index list.
	 */
	if (tp->params.list.val->len == 0) {
		indices[0] = 0;
		single = 1;
	} else if (tp->params.list.val->len == 1) {
		indices[0] = (int)TP_NUM();
		single = 1;
	} else {
		i = 0;
		TP_LOOP(grpidx)
		if (grpidx.number.val < 0 || grpidx.number.val > RE_NREGS)
			tp_raise(tp_None, tp_string("group() grpidx out of range"));
		indices[i++] = (int)grpidx.number.val;
		TP_END
	}

	/* generate result string list */
	result = tp_list(tp);
	for (i = 0; i < RE_NREGS && indices[i] >= 0; i++) {
		tp_obj grpstr;
		start = re->re_regs.start[indices[i]];
		end   = re->re_regs.end[indices[i]];
		if (start < 0 || end < 0) {
			grpstr = tp_None;
		} else {
			grpstr = tp_string_copy(tp, (const char *)re->re_lastok + start, 
					end - start);
		}
		tp_set(tp, result, tp_None, grpstr);
	}
	return (single ? tp_get(tp, result, tp_number(0)) : result);
}

/*
 * FUNC: matchobj.groups()
 * self	- match object.
 * return all groups.
 * Note: CPython allow a 'default' argument, but we disallow it.
 */
static tp_obj match_obj_groups(TP)
{
	tp_obj self = TP_OBJ();		/* match object */
	regexobject *re = NULL;
	int start;
	int end;
	int i;
	tp_obj result = tp_list(tp);

	re = getre(tp, self);
	if (re->re_lastok == NULL) {
		tp_raise(tp_None, 
				tp_string("groups() only valid after successful match/search"));
	}

	for (i = 1; i < RE_NREGS; i++) {
		start = re->re_regs.start[i];
		end   = re->re_regs.end[i];
		if (start < 0 || end < 0)
			break;

		tp_obj grpstr = tp_string_copy(tp, 
				(const char *)re->re_lastok + start, end - start);

		if (tp_bool(tp, grpstr))
			tp_set(tp, result, tp_None, grpstr);
	}

	return (result);
}

/*
 * FUNC: matchobj.start([group])
 * self		- match object
 * group	- group index
 * return starting position of matched 'group' substring.
 */
static tp_obj match_obj_start(TP)
{
	tp_obj self = TP_OBJ();						/* match object */
	tp_obj group = TP_DEFAULT(tp_number(0));	/* group */
	regexobject *re = NULL;
	int start;

	re = getre(tp, self);
	if (re->re_lastok == NULL) {
		tp_raise(tp_None, 
				tp_string("start() only valid after successful match/search"));
	}

	if (group.number.val < 0 || group.number.val > RE_NREGS)
		tp_raise(tp_None, tp_string("IndexError: group index out of range"));

	start = re->re_regs.start[(int)group.number.val];

	return (tp_number(start));
}

/*
 * FUNC: matchobj.end([group])
 * self		- match object
 * group	- group index
 * return ending position of matched 'group' substring.
 */
static tp_obj match_obj_end(TP)
{
	tp_obj self = TP_OBJ();						/* match object */
	tp_obj group = TP_DEFAULT(tp_number(0));	/* group */
	regexobject *re = NULL;
	int end;

	re = getre(tp, self);
	if (re->re_lastok == NULL) {
		tp_raise(tp_None, 
				tp_string("end() only valid after successful match/search"));
	}

	if (group.number.val < 0 || group.number.val > RE_NREGS)
		tp_raise(tp_None, tp_string("IndexError: group index out of range"));

	end = re->re_regs.end[(int)group.number.val];

	return (tp_number(end));
}

/*
 * FUNC: matchobj.span([group])
 * self		- match object
 * group	- group index
 * return [start,end] position pair of matched 'group' substring.
 */
static tp_obj match_obj_span(TP)
{
	tp_obj self = TP_OBJ();						/* match object */
	tp_obj group = TP_DEFAULT(tp_number(0));	/* group */
	regexobject *re = NULL;
	int start;
	int end;
	tp_obj result;

	re = getre(tp, self);
	if (re->re_lastok == NULL) {
		tp_raise(tp_None, 
				tp_string("span() only valid after successful match/search"));
	}

	if (group.number.val < 0 || group.number.val > RE_NREGS)
		tp_raise(tp_None, tp_string("IndexError: group index out of range"));

	start = re->re_regs.start[(int)group.number.val];
	end   = re->re_regs.end[(int)group.number.val];

	result = tp_list(tp);
	tp_set(tp, result, tp_None, tp_number(start));
	tp_set(tp, result, tp_None, tp_number(end));

	return (result);
}

/*
 * compile out a re object
 * repat	- regex pattern
 * resyn	- regex syntax
 */
static tp_obj regex_compile(TP)
{
	const char *error = NULL;
	char const *pat = NULL;
	int size = 0;
	tp_obj reobj_data;
	tp_obj repat = TP_TYPE(TP_STRING);						/* pattern */
	tp_obj resyn = TP_DEFAULT(tp_number(RE_SYNTAX_EMACS));	/* syntax */
	tp_obj reobj;	/* regex object */
	regexobject *re;

	/*
	 * create regex object, its parent is builtin 'object'
	 */
	reobj = tp_object(tp);

	re = (regexobject *)malloc(sizeof(regexobject));
	if (!re) {
		error = "malloc lower level regex object failed";
		goto finally;
	}

	re->re_patbuf.buffer = NULL;
	re->re_patbuf.allocated = 0;
	re->re_patbuf.fastmap = (unsigned char *)re->re_fastmap;
	re->re_patbuf.translate = NULL;
	re->re_translate = NULL;
	re->re_lastok = NULL;

	re->re_errno = 0;
	re->re_syntax = (int)resyn.number.val;

	pat = repat.string.val;
	size = repat.string.len;
	error = re_compile_pattern((unsigned char *)pat, size, &re->re_patbuf);
	if (error != NULL) {
		LastError = error;
		goto finally;
	}

	/* regexobject's size as magic */
	reobj_data = tp_data(tp, (int)sizeof(regexobject), re);

	/*
	 * bind to regex object
	 */
	tp_set(tp, reobj, tp_string("search"), 
			tp_method(tp, reobj, regex_obj_search));
	tp_set(tp, reobj, tp_string("match"), 
			tp_method(tp, reobj, regex_obj_match));
	tp_set(tp, reobj, tp_string("split"),
			tp_method(tp, reobj, regex_obj_split));
	tp_set(tp, reobj, tp_string("findall"),
			tp_method(tp, reobj, regex_obj_findall));
	tp_set(tp, reobj, tp_string("__data__"), reobj_data);

	tp_set(tp, reobj, tp_string("__name__"), 
			tp_string("regular expression object"));
	tp_set(tp, reobj, tp_string("__doc__"), tp_string(
				"regular expression object, support methods:\n"
				"search(str[,pos=0])-search 'str' from 'pos'\n"
				"match(str[,pos=0])	-match 'str' from 'pos'\n"
				));

	return (reobj);

finally:
	tp_raise(tp_None, tp_string(error));
}

/*
 * module level search()
 */
static tp_obj regex_search(TP)
{
	tp_obj repat = TP_OBJ();	/* pattern */
	tp_obj restr = TP_OBJ();	/* string */
	tp_obj resyn = TP_DEFAULT(tp_number(RE_SYNTAX_EMACS));
	tp_obj reobj;				/* regex object */
	tp_obj maobj;				/* match object */

	/* compile out regex object */
	tp_params_v(tp, 2, repat, resyn);
	reobj = regex_compile(tp);
	
	/* call r.search() */
	tp_params_v(tp, 3, reobj, restr, tp_number(0));
	maobj = regex_obj_search(tp);

	return (maobj);
}

/*
 * module level match()
 */
static tp_obj regex_match(TP)
{
	tp_obj repat = TP_OBJ();	/* pattern */
	tp_obj restr = TP_OBJ();	/* string */
	tp_obj resyn = TP_DEFAULT(tp_number(RE_SYNTAX_EMACS));
	tp_obj reobj;				/* regex object */
	tp_obj maobj;				/* match object */

	/* compile out regex object */
	tp_params_v(tp, 2, repat, resyn);
	reobj = regex_compile(tp);
	
	/* call r.search() */
	tp_params_v(tp, 3, reobj, restr, tp_number(0));
	maobj = regex_obj_match(tp);

	return (maobj);
}

/*
 * module level split()
 * repat	- regex pattern
 * restr	- regex string
 * maxsplit	- max split field, default 0, mean no limit
 */
static tp_obj regex_split(TP)
{
	tp_obj repat = TP_OBJ();	/* pattern */
	tp_obj restr = TP_OBJ();	/* string */
	tp_obj maxsplit = TP_DEFAULT(tp_number(0));
	tp_obj reobj;				/* regex object */

	/* generate a temp regex object */
	tp_params_v(tp, 2, repat, tp_number(RE_SYNTAX_EMACS));
	reobj = regex_compile(tp);
	
	tp_params_v(tp, 3, reobj, restr, maxsplit);
	return regex_obj_split(tp);
}

/*
 * module level findall()
 * repat	- regex pattern
 * restr	- regex string
 * resyn	- regex syntax, optional, default RE_SYNTAX_EMAC
 */
static tp_obj regex_findall(TP)
{
	tp_obj repat = TP_OBJ();	/* pattern */
	tp_obj restr = TP_OBJ();	/* string */
	tp_obj resyn = TP_DEFAULT(tp_number(RE_SYNTAX_EMACS));
	tp_obj reobj;				/* regex object */

	/* generate a temp regex object */
	tp_params_v(tp, 2, repat, resyn);
	reobj = regex_compile(tp);
	
	tp_params_v(tp, 2, reobj, restr);
	return regex_obj_findall(tp);
}


/*
 * re mod can only support 'set_syntax', 'get_syntax', and 'compile' functions,
 * 'compile' function will return a 'reobj', and this 'reobj' will support
 * methods 'search', 'match', 'group', 'groupall', el al.
 */
void re_init(TP)
{
	/*
	 * module dict for re
	 */
	tp_obj re_mod = tp_dict(tp);

	/*
	 * bind to re module
	 */
	tp_set(tp, re_mod, tp_string("compile"),	  tp_fnc(tp, regex_compile));
	tp_set(tp, re_mod, tp_string("search"),		  tp_fnc(tp, regex_search));
	tp_set(tp, re_mod, tp_string("match"),		  tp_fnc(tp, regex_match));
	tp_set(tp, re_mod, tp_string("split"),		  tp_fnc(tp, regex_split));
	tp_set(tp, re_mod, tp_string("findall"),	  tp_fnc(tp, regex_findall));
	tp_set(tp, re_mod, tp_string("AWK_SYNTAX"),   tp_number(RE_SYNTAX_AWK));
	tp_set(tp, re_mod, tp_string("EGREP_SYNTAX"), tp_number(RE_SYNTAX_EGREP));
	tp_set(tp, re_mod, tp_string("GREP_SYNTAX"),  tp_number(RE_SYNTAX_GREP));
	tp_set(tp, re_mod, tp_string("EMACS_SYNTAX"), tp_number(RE_SYNTAX_EMACS));

	/*
	 * bind special attibutes to re module
	 */
	tp_set(tp, re_mod, tp_string("__name__"), 
			tp_string("regular expression module"));
	tp_set(tp, re_mod, tp_string("__file__"), tp_string(__FILE__));
	tp_set(tp, re_mod, tp_string("__doc__"), 
			tp_string("simple regular express implementation"));

	/*
	 * bind regex module to tinypy modules[]
	 */
	tp_set(tp, tp->modules, tp_string("re"), re_mod);
}

