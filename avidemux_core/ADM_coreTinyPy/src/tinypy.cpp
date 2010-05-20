/*
================================================================================

tinypy contains tinypy code licensed in a MIT format license.  It also
contains some goodies grabbed from Python, so that license is included
as well.

================================================================================

The tinypy License

Copyright (c) 2008 Phil Hassey

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.

================================================================================

PYTHON SOFTWARE FOUNDATION LICENSE VERSION 2
--------------------------------------------

1. This LICENSE AGREEMENT is between the Python Software Foundation
("PSF"), and the Individual or Organization ("Licensee") accessing and
otherwise using this software ("Python") in source or binary form and
its associated documentation.

2. Subject to the terms and conditions of this License Agreement, PSF
hereby grants Licensee a nonexclusive, royalty-free, world-wide
license to reproduce, analyze, test, perform and/or display publicly,
prepare derivative works, distribute, and otherwise use Python
alone or in any derivative version, provided, however, that PSF's
License Agreement and PSF's notice of copyright, i.e., "Copyright (c)
2001, 2002, 2003, 2004, 2005, 2006, 2007 Python Software Foundation;
All Rights Reserved" are retained in Python alone or in any derivative
version prepared by Licensee.

3. In the event Licensee prepares a derivative work that is based on
or incorporates Python or any part thereof, and wants to make
the derivative work available to others as provided herein, then
Licensee hereby agrees to include in any such work a brief summary of
the changes made to Python.

4. PSF is making Python available to Licensee on an "AS IS"
basis.  PSF MAKES NO REPRESENTATIONS OR WARRANTIES, EXPRESS OR
IMPLIED.  BY WAY OF EXAMPLE, BUT NOT LIMITATION, PSF MAKES NO AND
DISCLAIMS ANY REPRESENTATION OR WARRANTY OF MERCHANTABILITY OR FITNESS
FOR ANY PARTICULAR PURPOSE OR THAT THE USE OF PYTHON WILL NOT
INFRINGE ANY THIRD PARTY RIGHTS.

5. PSF SHALL NOT BE LIABLE TO LICENSEE OR ANY OTHER USERS OF PYTHON
FOR ANY INCIDENTAL, SPECIAL, OR CONSEQUENTIAL DAMAGES OR LOSS AS
A RESULT OF MODIFYING, DISTRIBUTING, OR OTHERWISE USING PYTHON,
OR ANY DERIVATIVE THEREOF, EVEN IF ADVISED OF THE POSSIBILITY THEREOF.

6. This License Agreement will automatically terminate upon a material
breach of its terms and conditions.

7. Nothing in this License Agreement shall be deemed to create any
relationship of agency, partnership, or joint venture between PSF and
Licensee.  This License Agreement does not grant permission to use PSF
trademarks or trade name in a trademark sense to endorse or promote
products or services of Licensee, or any third party.

8. By copying, installing or otherwise using Python, Licensee
agrees to be bound by the terms and conditions of this License
Agreement.

================================================================================
*/

#ifndef TINYPY_H
#define TINYPY_H
/* File: General
 * Things defined in tp.h.
 */
#ifndef TP_H
#define TP_H

#include <setjmp.h>
#include <sys/stat.h>
#ifndef __USE_ISOC99
#define __USE_ISOC99
#endif
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <math.h>
#include <time.h>

#ifdef __GNUC__
#define tp_inline __inline__
#endif

#ifdef _MSC_VER
#ifdef NDEBUG
#define tp_inline __inline
#else
/* don't inline in debug builds (for easier debugging) */
#define tp_inline
#endif
#endif

#ifndef tp_inline
#error "Unsuported compiler"
#endif
// MEANX : Redirect printf
bool pyPrintf(const char *fmt,...);
#define printf pyPrintf
// ************************

/*  #define tp_malloc(x) calloc((x),1)
    #define tp_realloc(x,y) realloc(x,y)
    #define tp_free(x) free(x) */

/* #include <gc/gc.h>
   #define tp_malloc(x) GC_MALLOC(x)
   #define tp_realloc(x,y) GC_REALLOC(x,y)
   #define tp_free(x)*/

enum {
    TP_NONE,TP_NUMBER,TP_STRING,TP_DICT,
    TP_LIST,TP_FNC,TP_DATA,
};

typedef double tp_num;

typedef struct tp_number_ {
    int type;
    tp_num val;
} tp_number_;
typedef struct tp_string_ {
    int type;
    struct _tp_string *info;
    char const *val;
    int len;
} tp_string_;
typedef struct tp_list_ {
    int type;
    struct _tp_list *val;
} tp_list_;
typedef struct tp_dict_ {
    int type;
    struct _tp_dict *val;
    int dtype;
} tp_dict_;
typedef struct tp_fnc_ {
    int type;
    struct _tp_fnc *info;
    int ftype;
    void *cfnc;
} tp_fnc_;
typedef struct tp_data_ {
    int type;
    struct _tp_data *info;
    void *val;
    int magic;
} tp_data_;

/* Type: tp_obj
 * Tinypy's object representation.
 *
 * Every object in tinypy is of this type in the C API.
 *
 * Fields:
 * type - This determines what kind of objects it is. It is either TP_NONE, in
 *        which case this is the none type and no other fields can be accessed.
 *        Or it has one of the values listed below, and the corresponding
 *        fields can be accessed.
 * number - TP_NUMBER
 * number.val - A double value with the numeric value.
 * string - TP_STRING
 * string.val - A pointer to the string data.
 * string.len - Length in bytes of the string data.
 * dict - TP_DICT
 * list - TP_LIST
 * fnc - TP_FNC
 * data - TP_DATA
 * data.val - The user-provided data pointer.
 * data.magic - The user-provided magic number for identifying the data type.
 */
typedef union tp_obj {
    int type;
    tp_number_ number;
    struct { int type; int *data; } gci;
    tp_string_ string;
    tp_dict_ dict;
    tp_list_ list;
    tp_fnc_ fnc;
    tp_data_ data;
} tp_obj;

typedef struct _tp_string {
    int gci;
    int len;
    char s[1];
} _tp_string;
typedef struct _tp_list {
    int gci;
    tp_obj *items;
    int len;
    int alloc;
} _tp_list;
typedef struct tp_item {
    int used;
    int hash;
    tp_obj key;
    tp_obj val;
} tp_item;
typedef struct _tp_dict {
    int gci;
    tp_item *items;
    int len;
    int alloc;
    int cur;
    int mask;
    int used;
    tp_obj meta;
} _tp_dict;
typedef struct _tp_fnc {
    int gci;
    tp_obj self;
    tp_obj globals;
    tp_obj code;
} _tp_fnc;


typedef union tp_code {
    unsigned char i;
    struct { unsigned char i,a,b,c; } regs;
    struct { char val[4]; } string;
    struct { float val; } number;
} tp_code;

typedef struct tp_frame_ {
/*    tp_code *codes; */
    tp_obj code;
    tp_code *cur;
    tp_code *jmp;
    tp_obj *regs;
    tp_obj *ret_dest;
    tp_obj fname;
    tp_obj name;
    tp_obj line;
    tp_obj globals;
    int lineno;
    int cregs;
} tp_frame_;

#define TP_GCMAX 4096
#define TP_FRAMES 256
#define TP_REGS_EXTRA 2
/* #define TP_REGS_PER_FRAME 256*/
#define TP_REGS 16384

/* Type: tp_vm
 * Representation of a tinypy virtual machine instance.
 *
 * A new tp_vm struct is created with <tp_init>, and will be passed to most
 * tinypy functions as first parameter. It contains all the data associated
 * with an instance of a tinypy virtual machine - so it is easy to have
 * multiple instances running at the same time. When you want to free up all
 * memory used by an instance, call <tp_deinit>.
 *
 * Fields:
 * These fields are currently documented:
 *
 * builtins - A dictionary containing all builtin objects.
 * modules - A dictionary with all loaded modules.
 * params - A list of parameters for the current function call.
 * frames - A list of all call frames.
 * cur - The index of the currently executing call frame.
 * frames[n].globals - A dictionary of global sybmols in callframe n.
 */
typedef struct tp_vm {
    tp_obj builtins;
    tp_obj modules;
    tp_frame_ frames[TP_FRAMES];
    tp_obj _params;
    tp_obj params;
    tp_obj _regs;
    tp_obj *regs;
    tp_obj root;
    jmp_buf buf;
#ifdef CPYTHON_MOD
    jmp_buf nextexpr;
#endif
    int jmp;
    tp_obj ex;
    char chars[256][2];
    int cur;
    /* gc */
    _tp_list *white;
    _tp_list *grey;
    _tp_list *black;
    int steps;
    /* sandbox */
    clock_t clocks;
    double time_elapsed;
    double time_limit;
    unsigned long mem_limit;
    unsigned long mem_used;
    int mem_exceeded;
} tp_vm;

#define TP tp_vm *tp
typedef struct _tp_data {
    int gci;
    void (*free)(TP,tp_obj);
} _tp_data;

#define tp_True tp_number(1)
#define tp_False tp_number(0)

extern tp_obj tp_None;

#ifdef TP_SANDBOX
void *tp_malloc(TP, unsigned long);
void *tp_realloc(TP, void *, unsigned long);
void tp_free(TP, void *);
#else
#define tp_malloc(TP,x) calloc((x),1)
#define tp_realloc(TP,x,y) realloc(x,y)
#define tp_free(TP,x) free(x)
#endif

void tp_sandbox(TP, double, unsigned long);
void tp_time_update(TP);
void tp_mem_update(TP);

void tp_run(TP,int cur);
void tp_set(TP,tp_obj,tp_obj,tp_obj);
tp_obj tp_get(TP,tp_obj,tp_obj);
tp_obj tp_has(TP,tp_obj self, tp_obj k);
tp_obj tp_len(TP,tp_obj);
void tp_del(TP,tp_obj,tp_obj);
tp_obj tp_str(TP,tp_obj);
int tp_bool(TP,tp_obj);
int tp_cmp(TP,tp_obj,tp_obj);
void _tp_raise(TP,tp_obj);
tp_obj tp_printf(TP,char const *fmt,...);
tp_obj tp_track(TP,tp_obj);
void tp_grey(TP,tp_obj);
tp_obj tp_call(TP, tp_obj fnc, tp_obj params);
tp_obj tp_add(TP,tp_obj a, tp_obj b) ;

/* __func__ __VA_ARGS__ __FILE__ __LINE__ */

/* Function: tp_raise
 * Macro to raise an exception.
 *
 * This macro will return from the current function returning "r". The
 * remaining parameters are used to format the exception message.
 */
/*
#define tp_raise(r,fmt,...) { \
    _tp_raise(tp,tp_printf(tp,fmt,__VA_ARGS__)); \
    return r; \
}
*/
#define tp_raise(r,v) { \
    _tp_raise(tp,v); \
    return r; \
}

/* Function: tp_string
 * Creates a new string object from a C string.
 *
 * Given a pointer to a C string, creates a tinypy object representing the
 * same string.
 *
 * *Note* Only a reference to the string will be kept by tinypy, so make sure
 * it does not go out of scope, and don't de-allocate it. Also be aware that
 * tinypy will not delete the string for you. In many cases, it is best to
 * use <tp_string_t> or <tp_string_slice> to create a string where tinypy
 * manages storage for you.
 */
tp_inline static tp_obj tp_string(char const *v) {
    tp_obj val;
    tp_string_ s = {TP_STRING, 0, v, 0};
    s.len = strlen(v);
    val.string = s;
    return val;
}

#define TP_CSTR_LEN 256

tp_inline static void tp_cstr(TP,tp_obj v, char *s, int l) {
    if (v.type != TP_STRING) {
        tp_raise(,tp_string("(tp_cstr) TypeError: value not a string"));
    }
    if (v.string.len >= l) {
        tp_raise(,tp_string("(tp_cstr) TypeError: value too long"));
    }
    memset(s,0,l);
    memcpy(s,v.string.val,v.string.len);
}


#define TP_OBJ() (tp_get(tp,tp->params,tp_None))
tp_inline static tp_obj tp_type(TP,int t,tp_obj v) {
    if (v.type != t) { tp_raise(tp_None,tp_string("(tp_type) TypeError: unexpected type")); }
    return v;
}



#define TP_NO_LIMIT 0
#define TP_TYPE(t) tp_type(tp,t,TP_OBJ())
#define TP_NUM() (TP_TYPE(TP_NUMBER).number.val)
/* #define TP_STR() (TP_CSTR(TP_TYPE(TP_STRING))) */
#define TP_STR() (TP_TYPE(TP_STRING))
#define TP_DEFAULT(d) (tp->params.list.val->len?tp_get(tp,tp->params,tp_None):(d))

/* Macro: TP_LOOP
 * Macro to iterate over all remaining arguments.
 *
 * If you have a function which takes a variable number of arguments, you can
 * iterate through all remaining arguments for example like this:
 *
 * > tp_obj *my_func(tp_vm *tp)
 * > {
 * >     // We retrieve the first argument like normal.
 * >     tp_obj first = TP_OBJ();
 * >     // Then we iterate over the remaining arguments.
 * >     tp_obj arg;
 * >     TP_LOOP(arg)
 * >         // do something with arg
 * >     TP_END
 * > }
 */
#define TP_LOOP(e) \
    int __l = tp->params.list.val->len; \
    int __i; for (__i=0; __i<__l; __i++) { \
    (e) = _tp_list_get(tp,tp->params.list.val,__i,"TP_LOOP");
#define TP_END \
    }

tp_inline static int _tp_min(int a, int b) { return (a<b?a:b); }
tp_inline static int _tp_max(int a, int b) { return (a>b?a:b); }
tp_inline static int _tp_sign(tp_num v) { return (v<0?-1:(v>0?1:0)); }

/* Function: tp_number
 * Creates a new numeric object.
 */
tp_inline static tp_obj tp_number(tp_num v) {
    tp_obj val = {TP_NUMBER};
    val.number.val = v;
    return val;
}

tp_inline static void tp_echo(TP,tp_obj e) {
    e = tp_str(tp,e);
    //fwrite(e.string.val,1,e.string.len,stdout); // MEANX
    printf("%s",e.string.val);
}

/* Function: tp_string_n
 * Creates a new string object from a partial C string.
 *
 * Like <tp_string>, but you specify how many bytes of the given C string to
 * use for the string object. The *note* also applies for this function, as the
 * string reference and length are kept, but no actual substring is stored.
 */
tp_inline static tp_obj tp_string_n(char const *v,int n) {
    tp_obj val;
    tp_string_ s = {TP_STRING, 0,v,n};
    val.string = s;
    return val;
}

#endif
void _tp_list_realloc(TP, _tp_list *self,int len) ;
void _tp_list_set(TP,_tp_list *self,int k, tp_obj v, const char *error) ;
void _tp_list_free(TP, _tp_list *self) ;
tp_obj _tp_list_get(TP,_tp_list *self,int k,const char *error) ;
void _tp_list_insertx(TP,_tp_list *self, int n, tp_obj v) ;
void _tp_list_appendx(TP,_tp_list *self, tp_obj v) ;
void _tp_list_insert(TP,_tp_list *self, int n, tp_obj v) ;
void _tp_list_append(TP,_tp_list *self, tp_obj v) ;
tp_obj _tp_list_pop(TP,_tp_list *self, int n, const char *error) ;
int _tp_list_find(TP,_tp_list *self, tp_obj v) ;
tp_obj tp_index(TP) ;
_tp_list *_tp_list_new(TP) ;
tp_obj _tp_list_copy(TP, tp_obj rr) ;
tp_obj tp_append(TP) ;
tp_obj tp_pop(TP) ;
tp_obj tp_insert(TP) ;
tp_obj tp_extend(TP) ;
tp_obj tp_list_nt(TP) ;
tp_obj tp_list(TP) ;
tp_obj tp_list_n(TP,int n,tp_obj *argv) ;
int _tp_sort_cmp(tp_obj *a,tp_obj *b) ;
tp_obj tp_sort(TP) ;
int tp_lua_hash(void const *v,int l) ;
void _tp_dict_free(TP, _tp_dict *self) ;
int tp_hash(TP,tp_obj v) ;
void _tp_dict_hash_set(TP,_tp_dict *self, int hash, tp_obj k, tp_obj v) ;
void _tp_dict_tp_realloc(TP,_tp_dict *self,int len) ;
int _tp_dict_hash_find(TP,_tp_dict *self, int hash, tp_obj k) ;
int _tp_dict_find(TP,_tp_dict *self,tp_obj k) ;
void _tp_dict_setx(TP,_tp_dict *self,tp_obj k, tp_obj v) ;
void _tp_dict_set(TP,_tp_dict *self,tp_obj k, tp_obj v) ;
tp_obj _tp_dict_get(TP,_tp_dict *self,tp_obj k, const char *error) ;
void _tp_dict_del(TP,_tp_dict *self,tp_obj k, const char *error) ;
_tp_dict *_tp_dict_new(TP) ;
tp_obj _tp_dict_copy(TP,tp_obj rr) ;
int _tp_dict_next(TP,_tp_dict *self) ;
tp_obj tp_merge(TP) ;
tp_obj tp_dict(TP) ;
tp_obj tp_dict_n(TP,int n, tp_obj* argv) ;
tp_obj _tp_dcall(TP,tp_obj fnc(TP)) ;
tp_obj _tp_tcall(TP,tp_obj fnc) ;
tp_obj tp_fnc_new(TP,int t, void *v, tp_obj c,tp_obj s, tp_obj g) ;
tp_obj tp_def(TP,tp_obj code, tp_obj g) ;
tp_obj tp_fnc(TP,tp_obj v(TP)) ;
tp_obj tp_method(TP,tp_obj self,tp_obj v(TP)) ;
tp_obj tp_data(TP,int magic,void *v) ;
tp_obj tp_params(TP) ;
tp_obj tp_params_n(TP,int n, tp_obj argv[]) ;
tp_obj tp_params_v(TP,int n,...) ;
tp_obj tp_string_t(TP, int n) ;
tp_obj tp_string_copy(TP, const char *s, int n) ;
tp_obj tp_string_sub(TP, tp_obj s, int a, int b) ;
int _tp_str_index(tp_obj s, tp_obj k) ;
tp_obj tp_join(TP) ;
tp_obj tp_split(TP) ;
tp_obj tp_find(TP) ;
tp_obj tp_str_index(TP) ;
tp_obj tp_str2(TP) ;
tp_obj tp_chr(TP) ;
tp_obj tp_ord(TP) ;
tp_obj tp_strip(TP) ;
tp_obj tp_replace(TP) ;
tp_obj tp_print(TP) ;
tp_obj tp_bind(TP) ;
tp_obj tp_min(TP) ;
tp_obj tp_max(TP) ;
tp_obj tp_copy(TP) ;
tp_obj tp_len_(TP) ;
tp_obj tp_assert(TP) ;
tp_obj tp_range(TP) ;
tp_obj tp_system(TP) ;
tp_obj tp_istype(TP) ;
tp_obj tp_float(TP) ;
tp_obj tp_save(TP) ;
tp_obj tp_load(TP) ;
tp_obj tp_fpack(TP) ;
tp_obj tp_abs(TP) ;
tp_obj tp_int(TP) ;
tp_num _roundf(tp_num v) ;
tp_obj tp_round(TP) ;
tp_obj tp_exists(TP) ;
tp_obj tp_mtime(TP) ;
int _tp_lookup_(TP,tp_obj self, tp_obj k, tp_obj *meta, int depth) ;
int _tp_lookup(TP,tp_obj self, tp_obj k, tp_obj *meta) ;
tp_obj tp_setmeta(TP) ;
tp_obj tp_getmeta(TP) ;
tp_obj tp_object(TP) ;
tp_obj tp_object_new(TP) ;
tp_obj tp_object_call(TP) ;
tp_obj tp_getraw(TP) ;
tp_obj tp_class(TP) ;
tp_obj tp_builtins_bool(TP) ;
void tp_follow(TP,tp_obj v) ;
void tp_reset(TP) ;
void tp_gc_init(TP) ;
void tp_gc_deinit(TP) ;
void tp_delete(TP,tp_obj v) ;
void tp_collect(TP) ;
void _tp_gcinc(TP) ;
void tp_full(TP) ;
void tp_gcinc(TP) ;
tp_obj tp_iter(TP,tp_obj self, tp_obj k) ;
int tp_iget(TP,tp_obj *r, tp_obj self, tp_obj k) ;
tp_obj tp_mul(TP,tp_obj a, tp_obj b) ;
tp_obj tp_bitwise_not(TP, tp_obj a) ;
tp_vm *_tp_init(void) ;
void tp_deinit(TP) ;
void tp_frame(TP,tp_obj globals,tp_obj code,tp_obj *ret_dest) ;
void tp_print_stack(TP) ;
void tp_handle(TP) ;
void tp_return(TP, tp_obj v) ;
int tp_step(TP) ;
void _tp_run(TP,int cur) ;
tp_obj tp_ez_call(TP, const char *mod, const char *fnc, tp_obj params) ;
tp_obj _tp_import(TP, tp_obj fname, tp_obj name, tp_obj code) ;
tp_obj tp_import(TP, const char * fname, const char * name, void *codes, int len) ;
tp_obj tp_exec_(TP) ;
tp_obj tp_import_(TP) ;
void tp_builtins(TP) ;
void tp_args(TP,int argc, char *argv[]) ;
tp_obj tp_main(TP,char *fname, void *code, int len) ;
tp_obj tp_compile(TP, tp_obj text, tp_obj fname) ;
tp_obj tp_exec(TP, tp_obj code, tp_obj globals) ;
tp_obj tp_eval(TP, const char *text, tp_obj globals) ;
tp_vm *tp_init(int argc, char *argv[]) ;
void tp_compiler(TP) ;
tp_obj tp_sandbox_(TP) ;
void tp_bounds(TP, tp_code *cur, int n) ;
#endif

void _tp_list_realloc(TP, _tp_list *self,int len) {
    if (!len) { len=1; }
    self->items = (tp_obj*)tp_realloc(tp, self->items,len*sizeof(tp_obj));
    self->alloc = len;
}

void _tp_list_set(TP,_tp_list *self,int k, tp_obj v, const char *error) {
    if (k >= self->len) {
        tp_raise(,tp_string("(_tp_list_set) KeyError"));
    }
    self->items[k] = v;
    tp_grey(tp,v);
}
void _tp_list_free(TP, _tp_list *self) {
    tp_free(tp, self->items);
    tp_free(tp, self);
}

tp_obj _tp_list_get(TP,_tp_list *self,int k,const char *error) {
    if (k >= self->len) {
        tp_raise(tp_None,tp_string("(_tp_list_set) KeyError"));
    }
    return self->items[k];
}
void _tp_list_insertx(TP,_tp_list *self, int n, tp_obj v) {
    if (self->len >= self->alloc) {
        _tp_list_realloc(tp, self,self->alloc*2);
    }
    if (n < self->len) { memmove(&self->items[n+1],&self->items[n],sizeof(tp_obj)*(self->len-n)); }
    self->items[n] = v;
    self->len += 1;
}
void _tp_list_appendx(TP,_tp_list *self, tp_obj v) {
    _tp_list_insertx(tp,self,self->len,v);
}
void _tp_list_insert(TP,_tp_list *self, int n, tp_obj v) {
    _tp_list_insertx(tp,self,n,v);
    tp_grey(tp,v);
}
void _tp_list_append(TP,_tp_list *self, tp_obj v) {
    _tp_list_insert(tp,self,self->len,v);
}
tp_obj _tp_list_pop(TP,_tp_list *self, int n, const char *error) {
    tp_obj r = _tp_list_get(tp,self,n,error);
    if (n != self->len-1) { memmove(&self->items[n],&self->items[n+1],sizeof(tp_obj)*(self->len-(n+1))); }
    self->len -= 1;
    return r;
}

int _tp_list_find(TP,_tp_list *self, tp_obj v) {
    int n;
    for (n=0; n<self->len; n++) {
        if (tp_cmp(tp,v,self->items[n]) == 0) {
            return n;
        }
    }
    return -1;
}

tp_obj tp_index(TP) {
    tp_obj self = TP_OBJ();
    tp_obj v = TP_OBJ();
    int i = _tp_list_find(tp,self.list.val,v);
    if (i < 0) {
        tp_raise(tp_None,tp_string("(tp_index) ValueError: list.index(x): x not in list"));
    }
    return tp_number(i);
}

_tp_list *_tp_list_new(TP) {
    return (_tp_list*)tp_malloc(tp, sizeof(_tp_list));
}

tp_obj _tp_list_copy(TP, tp_obj rr) {
    tp_obj val = {TP_LIST};
    _tp_list *o = rr.list.val;
    _tp_list *r = _tp_list_new(tp);
    *r = *o; r->gci = 0;
    r->items = (tp_obj*)tp_malloc(tp, sizeof(tp_obj)*o->len);
    memcpy(r->items,o->items,sizeof(tp_obj)*o->len);
    val.list.val = r;
    return tp_track(tp,val);
}

tp_obj tp_append(TP) {
    tp_obj self = TP_OBJ();
    tp_obj v = TP_OBJ();
    _tp_list_append(tp,self.list.val,v);
    return tp_None;
}

tp_obj tp_pop(TP) {
    tp_obj self = TP_OBJ();
    return _tp_list_pop(tp,self.list.val,self.list.val->len-1,"pop");
}

tp_obj tp_insert(TP) {
    tp_obj self = TP_OBJ();
    int n = TP_NUM();
    tp_obj v = TP_OBJ();
    _tp_list_insert(tp,self.list.val,n,v);
    return tp_None;
}

tp_obj tp_extend(TP) {
    tp_obj self = TP_OBJ();
    tp_obj v = TP_OBJ();
    int i;
    for (i=0; i<v.list.val->len; i++) {
        _tp_list_append(tp,self.list.val,v.list.val->items[i]);
    }
    return tp_None;
}

tp_obj tp_list_nt(TP) {
    tp_obj r = {TP_LIST};
    r.list.val = _tp_list_new(tp);
    return r;
}

tp_obj tp_list(TP) {
    tp_obj r = {TP_LIST};
    r.list.val = _tp_list_new(tp);
    return tp_track(tp,r);
}

tp_obj tp_list_n(TP,int n,tp_obj *argv) {
    int i;
    tp_obj r = tp_list(tp); _tp_list_realloc(tp, r.list.val,n);
    for (i=0; i<n; i++) {
        _tp_list_append(tp,r.list.val,argv[i]);
    }
    return r;
}

int _tp_sort_cmp(tp_obj *a,tp_obj *b) {
    return tp_cmp(0,*a,*b);
}

tp_obj tp_sort(TP) {
    tp_obj self = TP_OBJ();
    qsort(self.list.val->items, self.list.val->len, sizeof(tp_obj), (int(*)(const void*,const void*))_tp_sort_cmp);
    return tp_None;
}

/* File: Dict
 * Functions for dealing with dictionaries.
 */
int tp_lua_hash(void const *v,int l) {
    int i,step = (l>>5)+1;
    int h = l + (l >= 4?*(int*)v:0);
    for (i=l; i>=step; i-=step) {
        h = h^((h<<5)+(h>>2)+((unsigned char *)v)[i-1]);
    }
    return h;
}
void _tp_dict_free(TP, _tp_dict *self) {
    tp_free(tp, self->items);
    tp_free(tp, self);
}

/* void _tp_dict_reset(_tp_dict *self) {
       memset(self->items,0,self->alloc*sizeof(tp_item));
       self->len = 0;
       self->used = 0;
       self->cur = 0;
   }*/

int tp_hash(TP,tp_obj v) {
    switch (v.type) {
        case TP_NONE: return 0;
        case TP_NUMBER: return tp_lua_hash(&v.number.val,sizeof(tp_num));
        case TP_STRING: return tp_lua_hash(v.string.val,v.string.len);
        case TP_DICT: return tp_lua_hash(&v.dict.val,sizeof(void*));
        case TP_LIST: {
            int r = v.list.val->len; int n; for(n=0; n<v.list.val->len; n++) {
            tp_obj vv = v.list.val->items[n]; r += vv.type != TP_LIST?tp_hash(tp,v.list.val->items[n]):tp_lua_hash(&vv.list.val,sizeof(void*)); } return r;
        }
        case TP_FNC: return tp_lua_hash(&v.fnc.info,sizeof(void*));
        case TP_DATA: return tp_lua_hash(&v.data.val,sizeof(void*));
    }
    tp_raise(0,tp_string("(tp_hash) TypeError: value unhashable"));
}

void _tp_dict_hash_set(TP,_tp_dict *self, int hash, tp_obj k, tp_obj v) {
    tp_item item;
    int i,idx = hash&self->mask;
    for (i=idx; i<idx+self->alloc; i++) {
        int n = i&self->mask;
        if (self->items[n].used > 0) { continue; }
        if (self->items[n].used == 0) { self->used += 1; }
        item.used = 1;
        item.hash = hash;
        item.key = k;
        item.val = v;
        self->items[n] = item;
        self->len += 1;
        return;
    }
    tp_raise(,tp_string("(_tp_dict_hash_set) RuntimeError: ?"));
}

void _tp_dict_tp_realloc(TP,_tp_dict *self,int len) {
    tp_item *items = self->items;
    int i,alloc = self->alloc;
    len = _tp_max(8,len);

    self->items = (tp_item*)tp_malloc(tp, len*sizeof(tp_item));
    self->alloc = len; self->mask = len-1;
    self->len = 0; self->used = 0;

    for (i=0; i<alloc; i++) {
        if (items[i].used != 1) { continue; }
        _tp_dict_hash_set(tp,self,items[i].hash,items[i].key,items[i].val);
    }
    tp_free(tp, items);
}

int _tp_dict_hash_find(TP,_tp_dict *self, int hash, tp_obj k) {
    int i,idx = hash&self->mask;
    for (i=idx; i<idx+self->alloc; i++) {
        int n = i&self->mask;
        if (self->items[n].used == 0) { break; }
        if (self->items[n].used < 0) { continue; }
        if (self->items[n].hash != hash) { continue; }
        if (tp_cmp(tp,self->items[n].key,k) != 0) { continue; }
        return n;
    }
    return -1;
}
int _tp_dict_find(TP,_tp_dict *self,tp_obj k) {
    return _tp_dict_hash_find(tp,self,tp_hash(tp,k),k);
}

void _tp_dict_setx(TP,_tp_dict *self,tp_obj k, tp_obj v) {
    int hash = tp_hash(tp,k); int n = _tp_dict_hash_find(tp,self,hash,k);
    if (n == -1) {
        if (self->len >= (self->alloc/2)) {
            _tp_dict_tp_realloc(tp,self,self->alloc*2);
        } else if (self->used >= (self->alloc*3/4)) {
            _tp_dict_tp_realloc(tp,self,self->alloc);
        }
        _tp_dict_hash_set(tp,self,hash,k,v);
    } else {
        self->items[n].val = v;
    }
}

void _tp_dict_set(TP,_tp_dict *self,tp_obj k, tp_obj v) {
    _tp_dict_setx(tp,self,k,v);
    tp_grey(tp,k); tp_grey(tp,v);
}

tp_obj _tp_dict_get(TP,_tp_dict *self,tp_obj k, const char *error) {
    int n = _tp_dict_find(tp,self,k);
    if (n < 0) {
        tp_raise(tp_None,tp_add(tp,tp_string("(_tp_dict_get) KeyError: "),tp_str(tp,k)));
    }
    return self->items[n].val;
}

void _tp_dict_del(TP,_tp_dict *self,tp_obj k, const char *error) {
    int n = _tp_dict_find(tp,self,k);
    if (n < 0) {
        tp_raise(,tp_add(tp,tp_string("(_tp_dict_del) KeyError: "),tp_str(tp,k)));
    }
    self->items[n].used = -1;
    self->len -= 1;
}

_tp_dict *_tp_dict_new(TP) {
    _tp_dict *self = (_tp_dict*)tp_malloc(tp, sizeof(_tp_dict));
    return self;
}
tp_obj _tp_dict_copy(TP,tp_obj rr) {
    tp_obj obj = {TP_DICT};
    _tp_dict *o = rr.dict.val;
    _tp_dict *r = _tp_dict_new(tp);
    *r = *o; r->gci = 0;
    r->items = (tp_item*)tp_malloc(tp, sizeof(tp_item)*o->alloc);
    memcpy(r->items,o->items,sizeof(tp_item)*o->alloc);
    obj.dict.val = r;
    obj.dict.dtype = 1;
    return tp_track(tp,obj);
}

int _tp_dict_next(TP,_tp_dict *self) {
    if (!self->len) {
        tp_raise(0,tp_string("(_tp_dict_next) RuntimeError"));
    }
    while (1) {
        self->cur = ((self->cur + 1) & self->mask);
        if (self->items[self->cur].used > 0) {
            return self->cur;
        }
    }
}

tp_obj tp_merge(TP) {
    tp_obj self = TP_OBJ();
    tp_obj v = TP_OBJ();
    int i; for (i=0; i<v.dict.val->len; i++) {
        int n = _tp_dict_next(tp,v.dict.val);
        _tp_dict_set(tp,self.dict.val,
            v.dict.val->items[n].key,v.dict.val->items[n].val);
    }
    return tp_None;
}

/* Function: tp_dict
 *
 * Creates a new dictionary object.
 *
 * *Note* If you use <tp_setmeta> on the dictionary, you have to use <tp_getraw> to
 * access the "raw" dictionary again.
 *
 * Returns:
 * The newly created dictionary.
 */
tp_obj tp_dict(TP) {
    tp_obj r = {TP_DICT};
    r.dict.val = _tp_dict_new(tp);
    r.dict.dtype = 1;
    return tp ? tp_track(tp,r) : r;
}

tp_obj tp_dict_n(TP,int n, tp_obj* argv) {
    tp_obj r = tp_dict(tp);
    int i; for (i=0; i<n; i++) { tp_set(tp,r,argv[i*2],argv[i*2+1]); }
    return r;
}


/* File: Miscellaneous
 * Various functions to help interface tinypy.
 */

tp_obj _tp_dcall(TP,tp_obj fnc(TP)) {
    return fnc(tp);
}
tp_obj _tp_tcall(TP,tp_obj fnc) {
    if (fnc.fnc.ftype&2) {
        _tp_list_insert(tp,tp->params.list.val,0,fnc.fnc.info->self);
    }
    return _tp_dcall(tp,(tp_obj (*)(tp_vm *))fnc.fnc.cfnc);
}

tp_obj tp_fnc_new(TP,int t, void *v, tp_obj c,tp_obj s, tp_obj g) {
    tp_obj r = {TP_FNC};
    _tp_fnc *info = (_tp_fnc*)tp_malloc(tp, sizeof(_tp_fnc));
    info->code = c;
    info->self = s;
    info->globals = g;
    r.fnc.ftype = t;
    r.fnc.info = info;
    r.fnc.cfnc = v;
    return tp_track(tp,r);
}

tp_obj tp_def(TP,tp_obj code, tp_obj g) {
    tp_obj r = tp_fnc_new(tp,1,0,code,tp_None,g);
    return r;
}

/* Function: tp_fnc
 * Creates a new tinypy function object.
 *
 * This is how you can create a tinypy function object which, when called in
 * the script, calls the provided C function.
 */

//tp_obj tp_fnc_new(TP,int t, void *v, tp_obj c,tp_obj s, tp_obj g) ;
tp_obj tp_fnc(TP,tp_obj v(TP)) {
    return tp_fnc_new(tp,0,(void *)v,tp_None,tp_None, tp_None);
}

tp_obj tp_method(TP,tp_obj self,tp_obj v(TP)) {
    return tp_fnc_new(tp,2,(void*)v,tp_None,self,tp_None);
}

/* Function: tp_data
 * Creates a new data object.
 *
 * Parameters:
 * magic - An integer number associated with the data type. This can be used
 *         to check the type of data objects.
 * v     - A pointer to user data. Only the pointer is stored in the object,
 *         you keep all responsibility for the data it points to.
 *
 *
 * Returns:
 * The new data object.
 *
 * Public fields:
 * The following fields can be access in a data object:
 *
 * magic      - An integer number stored in the object.
 * val        - The data pointer of the object.
 * info->free - If not NULL, a callback function called when the object gets
 *              destroyed.
 *
 * Example:
 * > void *__free__(TP, tp_obj self)
 * > {
 * >     free(self.data.val);
 * > }
 * >
 * > tp_obj my_obj = tp_data(TP, 0, my_ptr);
 * > my_obj.data.info->free = __free__;
 */
tp_obj tp_data(TP,int magic,void *v) {
    tp_obj r = {TP_DATA};
    r.data.info = (_tp_data*)tp_malloc(tp, sizeof(_tp_data));
    r.data.val = v;
    r.data.magic = magic;
    return tp_track(tp,r);
}

/* Function: tp_params
 * Initialize the tinypy parameters.
 *
 * When you are calling a tinypy function, you can use this to initialize the
 * list of parameters getting passed to it. Usually, you may want to use
 * <tp_params_n> or <tp_params_v>.
 */
tp_obj tp_params(TP) {
    tp_obj r;
    tp->params = tp->_params.list.val->items[tp->cur];
    r = tp->_params.list.val->items[tp->cur];
    r.list.val->len = 0;
    return r;
}

/* Function: tp_params_n
 * Specify a list of objects as function call parameters.
 *
 * See also: <tp_params>, <tp_params_v>
 *
 * Parameters:
 * n - The number of parameters.
 * argv - A list of n tinypy objects, which will be passed as parameters.
 *
 * Returns:
 * The parameters list. You may modify it before performing the function call.
 */
tp_obj tp_params_n(TP,int n, tp_obj argv[]) {
    tp_obj r = tp_params(tp);
    int i; for (i=0; i<n; i++) { _tp_list_append(tp,r.list.val,argv[i]); }
    return r;
}

/* Function: tp_params_v
 * Pass parameters for a tinypy function call.
 *
 * When you want to call a tinypy method, then you use this to pass parameters
 * to it.
 *
 * Parameters:
 * n   - The number of variable arguments following.
 * ... - Pass n tinypy objects, which a subsequently called tinypy method will
 *       receive as parameters.
 *
 * Returns:
 * A tinypy list object representing the current call parameters. You can modify
 * the list before doing the function call.
 */
tp_obj tp_params_v(TP,int n,...) {
    int i;
    tp_obj r = tp_params(tp);
    va_list a; va_start(a,n);
    for (i=0; i<n; i++) {
        _tp_list_append(tp,r.list.val,va_arg(a,tp_obj));
    }
    va_end(a);
    return r;
}
/* File: String
 * String handling functions.
 */

/*
 * Create a new empty string of a certain size.
 * Does not put it in for GC tracking, since contents should be
 * filled after returning.
 */
tp_obj tp_string_t(TP, int n) {
    tp_obj r = tp_string_n(0,n);
    r.string.info = (_tp_string*)tp_malloc(tp, sizeof(_tp_string)+n);
    r.string.info->len = n;
    r.string.val = r.string.info->s;
    return r;
}

/*
 * Create a new string which is a copy of some memory.
 * This is put into GC tracking for you.
 */
tp_obj tp_string_copy(TP, const char *s, int n) {
    tp_obj r = tp_string_t(tp,n);
    memcpy(r.string.info->s,s,n);
    return tp_track(tp,r);
}

/*
 * Create a new string which is a substring slice of another STRING.
 * Does not need to be put into GC tracking, as its parent is
 * already being tracked (supposedly).
 */
tp_obj tp_string_sub(TP, tp_obj s, int a, int b) {
    int l = s.string.len;
    a = _tp_max(0,(a<0?l+a:a)); b = _tp_min(l,(b<0?l+b:b));
    tp_obj r = s;
    r.string.val += a;
    r.string.len = b-a;
    return r;
}

tp_obj tp_printf(TP, char const *fmt,...) {
    int l;
    tp_obj r;
    char *s;
    va_list arg;
    va_start(arg, fmt);
    l = vsnprintf(NULL, 0, fmt,arg);
    r = tp_string_t(tp,l);
    s = r.string.info->s;
    va_end(arg);
    va_start(arg, fmt);
    vsprintf(s,fmt,arg);
    va_end(arg);
    return tp_track(tp,r);
}

int _tp_str_index(tp_obj s, tp_obj k) {
    int i=0;
    while ((s.string.len - i) >= k.string.len) {
        if (memcmp(s.string.val+i,k.string.val,k.string.len) == 0) {
            return i;
        }
        i += 1;
    }
    return -1;
}

tp_obj tp_join(TP) {
    tp_obj delim = TP_OBJ();
    tp_obj val = TP_OBJ();
    int l=0,i;
    tp_obj r;
    char *s;
    for (i=0; i<val.list.val->len; i++) {
        if (i!=0) { l += delim.string.len; }
        l += tp_str(tp,val.list.val->items[i]).string.len;
    }
    r = tp_string_t(tp,l);
    s = r.string.info->s;
    l = 0;
    for (i=0; i<val.list.val->len; i++) {
        tp_obj e;
        if (i!=0) {
            memcpy(s+l,delim.string.val,delim.string.len); l += delim.string.len;
        }
        e = tp_str(tp,val.list.val->items[i]);
        memcpy(s+l,e.string.val,e.string.len); l += e.string.len;
    }
    return tp_track(tp,r);
}

tp_obj tp_split(TP) {
    tp_obj v = TP_OBJ();
    tp_obj d = TP_OBJ();
    tp_obj r = tp_list(tp);

    int i;
    while ((i=_tp_str_index(v,d))!=-1) {
        _tp_list_append(tp,r.list.val,tp_string_sub(tp,v,0,i));
        v.string.val += i + d.string.len; v.string.len -= i + d.string.len;
    }
    _tp_list_append(tp,r.list.val,tp_string_sub(tp,v,0,v.string.len));
    return r;
}


tp_obj tp_find(TP) {
    tp_obj s = TP_OBJ();
    tp_obj v = TP_OBJ();
    return tp_number(_tp_str_index(s,v));
}

tp_obj tp_str_index(TP) {
    tp_obj s = TP_OBJ();
    tp_obj v = TP_OBJ();
    int n = _tp_str_index(s,v);
    if (n >= 0) { return tp_number(n); }
    tp_raise(tp_None,tp_string("(tp_str_index) ValueError: substring not found"));
}

tp_obj tp_str2(TP) {
    tp_obj v = TP_OBJ();
    return tp_str(tp,v);
}

tp_obj tp_chr(TP) {
    int v = TP_NUM();
    return tp_string_n(tp->chars[(unsigned char)v],1);
}
tp_obj tp_ord(TP) {
    tp_obj s = TP_STR();
    if (s.string.len != 1) {
        tp_raise(tp_None,tp_string("(tp_ord) TypeError: ord() expected a character"));
    }
    return tp_number((unsigned char)s.string.val[0]);
}

tp_obj tp_strip(TP) {
    tp_obj o = TP_TYPE(TP_STRING);
    char const *v = o.string.val; int l = o.string.len;
    int i; int a = l, b = 0;
    tp_obj r;
    char *s;
    for (i=0; i<l; i++) {
        if (v[i] != ' ' && v[i] != '\n' && v[i] != '\t' && v[i] != '\r') {
            a = _tp_min(a,i); b = _tp_max(b,i+1);
        }
    }
    if ((b-a) < 0) { return tp_string(""); }
    r = tp_string_t(tp,b-a);
    s = r.string.info->s;
    memcpy(s,v+a,b-a);
    return tp_track(tp,r);
}

tp_obj tp_replace(TP) {
    tp_obj s = TP_OBJ();
    tp_obj k = TP_OBJ();
    tp_obj v = TP_OBJ();
    tp_obj p = s;
    int i,n = 0;
    int c;
    int l;
    tp_obj rr;
    char *r;
    char *d;
    tp_obj z;
    while ((i = _tp_str_index(p,k)) != -1) {
        n += 1;
        p.string.val += i + k.string.len; p.string.len -= i + k.string.len;
    }
/*     fprintf(stderr,"ns: %d\n",n); */
    l = s.string.len + n * (v.string.len-k.string.len);
    rr = tp_string_t(tp,l);
    r = rr.string.info->s;
    d = r;
    z = p = s;
    while ((i = _tp_str_index(p,k)) != -1) {
        p.string.val += i; p.string.len -= i;
        memcpy(d,z.string.val,c=(p.string.val-z.string.val)); d += c;
        p.string.val += k.string.len; p.string.len -= k.string.len;
        memcpy(d,v.string.val,v.string.len); d += v.string.len;
        z = p;
    }
    memcpy(d,z.string.val,(s.string.val + s.string.len) - z.string.val);

    return tp_track(tp,rr);
}

/* File: Builtins
 * Builtin tinypy functions.
 */

tp_obj tp_print(TP) {
    int n = 0;
    tp_obj e;
    TP_LOOP(e)
        if (n) { printf(" "); }
        tp_echo(tp,e);
        n += 1;
    TP_END;
    printf("\n");
    return tp_None;
}

tp_obj tp_bind(TP) {
    tp_obj r = TP_TYPE(TP_FNC);
    tp_obj self = TP_OBJ();
    return tp_fnc_new(tp,
        r.fnc.ftype|2,r.fnc.cfnc,r.fnc.info->code,
        self,r.fnc.info->globals);
}

tp_obj tp_min(TP) {
    tp_obj r = TP_OBJ();
    tp_obj e;
    TP_LOOP(e)
        if (tp_cmp(tp,r,e) > 0) { r = e; }
    TP_END;
    return r;
}

tp_obj tp_max(TP) {
    tp_obj r = TP_OBJ();
    tp_obj e;
    TP_LOOP(e)
        if (tp_cmp(tp,r,e) < 0) { r = e; }
    TP_END;
    return r;
}

tp_obj tp_copy(TP) {
    tp_obj r = TP_OBJ();
    int type = r.type;
    if (type == TP_LIST) {
        return _tp_list_copy(tp,r);
    } else if (type == TP_DICT) {
        return _tp_dict_copy(tp,r);
    }
    tp_raise(tp_None,tp_string("(tp_copy) TypeError: ?"));
}


tp_obj tp_len_(TP) {
    tp_obj e = TP_OBJ();
    return tp_len(tp,e);
}

tp_obj tp_assert(TP) {
    int a = TP_NUM();
    if (a) { return tp_None; }
    tp_raise(tp_None,tp_string("(tp_assert) AssertionError"));
}

tp_obj tp_range(TP) {
    int a,b,c,i;
    tp_obj r = tp_list(tp);
    switch (tp->params.list.val->len) {
        case 1: a = 0; b = TP_NUM(); c = 1; break;
        case 2:
        case 3: a = TP_NUM(); b = TP_NUM(); c = TP_DEFAULT(tp_number(1)).number.val; break;
        default: return r;
    }
    if (c != 0) {
        for (i=a; (c>0) ? i<b : i>b; i+=c) {
            _tp_list_append(tp,r.list.val,tp_number(i));
        }
    }
    return r;
}

/* Function: tp_system
 *
 * The system builtin. A grave security flaw. If your version of tinypy
 * enables this, you better remove it before deploying your app :P
 */
tp_obj tp_system(TP) {
    char s[TP_CSTR_LEN]; tp_cstr(tp,TP_STR(),s,TP_CSTR_LEN);
    int r = system(s);
    return tp_number(r);
}

tp_obj tp_istype(TP) {
    tp_obj v = TP_OBJ();
    tp_obj t = TP_STR();
    if (tp_cmp(tp,t,tp_string("string")) == 0) { return tp_number(v.type == TP_STRING); }
    if (tp_cmp(tp,t,tp_string("list")) == 0) { return tp_number(v.type == TP_LIST); }
    if (tp_cmp(tp,t,tp_string("dict")) == 0) { return tp_number(v.type == TP_DICT); }
    if (tp_cmp(tp,t,tp_string("number")) == 0) { return tp_number(v.type == TP_NUMBER); }
    if (tp_cmp(tp,t,tp_string("fnc")) == 0) { return tp_number(v.type == TP_FNC && (v.fnc.ftype&2) == 0); }
    if (tp_cmp(tp,t,tp_string("method")) == 0) { return tp_number(v.type == TP_FNC && (v.fnc.ftype&2) != 0); }
    tp_raise(tp_None,tp_string("(is_type) TypeError: ?"));
}


tp_obj tp_float(TP) {
    tp_obj v = TP_OBJ();
    int ord = TP_DEFAULT(tp_number(0)).number.val;
    int type = v.type;
    if (type == TP_NUMBER) { return v; }
    if (type == TP_STRING && v.string.len < 32) {
        char s[32]; memset(s,0,v.string.len+1);
        memcpy(s,v.string.val,v.string.len);
        if (strchr(s,'.')) { return tp_number(atof(s)); }
        return(tp_number(strtol(s,0,ord)));
    }
    tp_raise(tp_None,tp_string("(tp_float) TypeError: ?"));
}


tp_obj tp_save(TP) {
    char fname[256]; tp_cstr(tp,TP_STR(),fname,256);
    tp_obj v = TP_OBJ();
    FILE *f;
    f = fopen(fname,"wb");
    if (!f) { tp_raise(tp_None,tp_string("(tp_save) IOError: ?")); }
    fwrite(v.string.val,v.string.len,1,f);
    fclose(f);
    return tp_None;
}

tp_obj tp_load(TP) {
    FILE *f;
    long l;
    tp_obj r;
    char *s;
    char fname[256]; tp_cstr(tp,TP_STR(),fname,256);
    struct stat stbuf;
    stat(fname, &stbuf);
    l = stbuf.st_size;
    f = fopen(fname,"rb");
    if (!f) {
        tp_raise(tp_None,tp_string("(tp_load) IOError: ?"));
    }
    r = tp_string_t(tp,l);
    s = r.string.info->s;
    fread(s,1,l,f);
/*    if (rr !=l) { printf("hmmn: %d %d\n",rr,(int)l); }*/
    fclose(f);
    return tp_track(tp,r);
}


tp_obj tp_fpack(TP) {
    tp_num v = TP_NUM();
    tp_obj r = tp_string_t(tp,sizeof(tp_num));
    *(tp_num*)r.string.val = v;
    return tp_track(tp,r);
}

tp_obj tp_abs(TP) {
    return tp_number(fabs(tp_float(tp).number.val));
}
tp_obj tp_int(TP) {
    return tp_number((long)tp_float(tp).number.val);
}
tp_num _roundf(tp_num v) {
    tp_num av = fabs(v); tp_num iv = (long)av;
    av = (av-iv < 0.5?iv:iv+1);
    return (v<0?-av:av);
}
tp_obj tp_round(TP) {
    return tp_number(_roundf(tp_float(tp).number.val));
}

tp_obj tp_exists(TP) {
    char fname[TP_CSTR_LEN]; tp_cstr(tp,TP_STR(),fname,TP_CSTR_LEN);
    struct stat stbuf;
    return tp_number(!stat(fname,&stbuf));
}
tp_obj tp_mtime(TP) {
    char fname[TP_CSTR_LEN]; tp_cstr(tp,TP_STR(),fname,TP_CSTR_LEN);
    struct stat stbuf;
    if (!stat(fname,&stbuf)) { return tp_number(stbuf.st_mtime); }
    tp_raise(tp_None,tp_string("(tp_mtime) IOError: ?"));
}

int _tp_lookup_(TP,tp_obj self, tp_obj k, tp_obj *meta, int depth) {
    int n = _tp_dict_find(tp,self.dict.val,k);
    if (n != -1) {
        *meta = self.dict.val->items[n].val;
        return 1;
    }
    depth--; if (!depth) { tp_raise(0,tp_string("(tp_lookup) RuntimeError: maximum lookup depth exceeded")); }
    if (self.dict.dtype && self.dict.val->meta.type == TP_DICT && _tp_lookup_(tp,self.dict.val->meta,k,meta,depth)) {
        if (self.dict.dtype == 2 && meta->type == TP_FNC) {
            *meta = tp_fnc_new(tp,meta->fnc.ftype|2,
                meta->fnc.cfnc,meta->fnc.info->code,
                self,meta->fnc.info->globals);
        }
        return 1;
    }
    return 0;
}

int _tp_lookup(TP,tp_obj self, tp_obj k, tp_obj *meta) {
    return _tp_lookup_(tp,self,k,meta,8);
}

#define TP_META_BEGIN(self,name) \
    if (self.dict.dtype == 2) { \
        tp_obj meta; if (_tp_lookup(tp,self,tp_string(name),&meta)) {

#define TP_META_END \
        } \
    }

/* Function: tp_setmeta
 * Set a "dict's meta".
 *
 * This is a builtin function, so you need to use <tp_params> to provide the
 * parameters.
 *
 * In tinypy, each dictionary can have a so-called "meta" dictionary attached
 * to it. When dictionary attributes are accessed, but not present in the
 * dictionary, they instead are looked up in the meta dictionary. To get the
 * raw dictionary, you can use <tp_getraw>.
 *
 * This function is particulary useful for objects and classes, which are just
 * special dictionaries created with <tp_object> and <tp_class>. There you can
 * use tp_setmeta to change the class of the object or parent class of a class.
 *
 * Parameters:
 * self - The dictionary for which to set a meta.
 * meta - The meta dictionary.
 *
 * Returns:
 * None
 */
tp_obj tp_setmeta(TP) {
    tp_obj self = TP_TYPE(TP_DICT);
    tp_obj meta = TP_TYPE(TP_DICT);
    self.dict.val->meta = meta;
    return tp_None;
}

tp_obj tp_getmeta(TP) {
    tp_obj self = TP_TYPE(TP_DICT);
    return self.dict.val->meta;
}

/* Function: tp_object
 * Creates a new object.
 *
 * Returns:
 * The newly created object. The object initially has no parent class, use
 * <tp_setmeta> to set a class. Also see <tp_object_new>.
 */
tp_obj tp_object(TP) {
    tp_obj self = tp_dict(tp);
    self.dict.dtype = 2;
    return self;
}

tp_obj tp_object_new(TP) {
    tp_obj klass = TP_TYPE(TP_DICT);
    tp_obj self = tp_object(tp);
    self.dict.val->meta = klass;
    TP_META_BEGIN(self,"__init__");
        tp_call(tp,meta,tp->params);
    TP_META_END;
    return self;
}

tp_obj tp_object_call(TP) {
    tp_obj self;
    if (tp->params.list.val->len) {
        self = TP_TYPE(TP_DICT);
        self.dict.dtype = 2;
    } else {
        self = tp_object(tp);
    }
    return self;
}

/* Function: tp_getraw
 * Retrieve the raw dict of a dict.
 *
 * This builtin retrieves one dict parameter from tinypy, and returns its raw
 * dict. This is very useful when implementing your own __get__ and __set__
 * functions, as it allows you to directly access the attributes stored in the
 * dict.
 */
tp_obj tp_getraw(TP) {
    tp_obj self = TP_TYPE(TP_DICT);
    self.dict.dtype = 0;
    return self;
}

/* Function: tp_class
 * Creates a new base class.
 *
 * Parameters:
 * none
 *
 * Returns:
 * A new, empty class (derived from tinypy's builtin "object" class).
 */
tp_obj tp_class(TP) {
    tp_obj klass = tp_dict(tp);
    klass.dict.val->meta = tp_get(tp,tp->builtins,tp_string("object"));
    return klass;
}

/* Function: tp_builtins_bool
 * Coerces any value to a boolean.
 */
tp_obj tp_builtins_bool(TP) {
    tp_obj v = TP_OBJ();
    return (tp_number(tp_bool(tp, v)));
}
/* tp_obj tp_track(TP,tp_obj v) { return v; }
   void tp_grey(TP,tp_obj v) { }
   void tp_full(TP) { }
   void tp_gc_init(TP) { }
   void tp_gc_deinit(TP) { }
   void tp_delete(TP,tp_obj v) { }*/

void tp_grey(TP,tp_obj v) {
    if (v.type < TP_STRING || (!v.gci.data) || *v.gci.data) { return; }
    *v.gci.data = 1;
    if (v.type == TP_STRING || v.type == TP_DATA) {
        _tp_list_appendx(tp,tp->black,v);
        return;
    }
    _tp_list_appendx(tp,tp->grey,v);
}

void tp_follow(TP,tp_obj v) {
    int type = v.type;
    if (type == TP_LIST) {
        int n;
        for (n=0; n<v.list.val->len; n++) {
            tp_grey(tp,v.list.val->items[n]);
        }
    }
    if (type == TP_DICT) {
        int i;
        for (i=0; i<v.dict.val->len; i++) {
            int n = _tp_dict_next(tp,v.dict.val);
            tp_grey(tp,v.dict.val->items[n].key);
            tp_grey(tp,v.dict.val->items[n].val);
        }
        tp_grey(tp,v.dict.val->meta);
    }
    if (type == TP_FNC) {
        tp_grey(tp,v.fnc.info->self);
        tp_grey(tp,v.fnc.info->globals);
        tp_grey(tp,v.fnc.info->code);
    }
}

void tp_reset(TP) {
    int n;
    _tp_list *tmp;
    for (n=0; n<tp->black->len; n++) {
        *tp->black->items[n].gci.data = 0;
    }
    tmp = tp->white;
    tp->white = tp->black;
    tp->black = tmp;
}

void tp_gc_init(TP) {
    tp->white = _tp_list_new(tp);
    tp->grey = _tp_list_new(tp);
    tp->black = _tp_list_new(tp);
    tp->steps = 0;
}

void tp_gc_deinit(TP) {
    _tp_list_free(tp, tp->white);
    _tp_list_free(tp, tp->grey);
    _tp_list_free(tp, tp->black);
}

void tp_delete(TP,tp_obj v) {
    int type = v.type;
    if (type == TP_LIST) {
        _tp_list_free(tp, v.list.val);
        return;
    } else if (type == TP_DICT) {
        _tp_dict_free(tp, v.dict.val);
        return;
    } else if (type == TP_STRING) {
        tp_free(tp, v.string.info);
        return;
    } else if (type == TP_DATA) {
        if (v.data.info->free) {
            v.data.info->free(tp,v);
        }
        tp_free(tp, v.data.info);
        return;
    } else if (type == TP_FNC) {
        tp_free(tp, v.fnc.info);
        return;
    }
    tp_raise(,tp_string("(tp_delete) TypeError: ?"));
}

void tp_collect(TP) {
    int n;
    for (n=0; n<tp->white->len; n++) {
        tp_obj r = tp->white->items[n];
        if (*r.gci.data) { continue; }
        tp_delete(tp,r);
    }
    tp->white->len = 0;
    tp_reset(tp);
}

void _tp_gcinc(TP) {
    tp_obj v;
    if (!tp->grey->len) {
        return;
    }
    v = _tp_list_pop(tp,tp->grey,tp->grey->len-1,"_tp_gcinc");
    tp_follow(tp,v);
    _tp_list_appendx(tp,tp->black,v);
}

void tp_full(TP) {
    while (tp->grey->len) {
        _tp_gcinc(tp);
    }
    tp_collect(tp);
    tp_follow(tp,tp->root);
}

void tp_gcinc(TP) {
    tp->steps += 1;
    if (tp->steps < TP_GCMAX || tp->grey->len > 0) {
        _tp_gcinc(tp); _tp_gcinc(tp);
    }
    if (tp->steps < TP_GCMAX || tp->grey->len > 0) { return; }
    tp->steps = 0;
    tp_full(tp);
    return;
}

tp_obj tp_track(TP,tp_obj v) {
    tp_gcinc(tp);
    tp_grey(tp,v);
    return v;
}

/**/

/* File: Operations
 * Various tinypy operations.
 */

/* Function: tp_str
 * String representation of an object.
 *
 * Returns a string object representating self.
 */
tp_obj tp_str(TP,tp_obj self) {
    int type = self.type;
    if (type == TP_STRING) { return self; }
    if (type == TP_NUMBER) {
        tp_num v = self.number.val;
        if ((fabs(v)-fabs((long)v)) < 0.000001) { return tp_printf(tp,"%ld",(long)v); }
        return tp_printf(tp,"%f",v);
    } else if(type == TP_DICT) {
        return tp_printf(tp,"<dict 0x%x>",self.dict.val);
    } else if(type == TP_LIST) {
        return tp_printf(tp,"<list 0x%x>",self.list.val);
    } else if (type == TP_NONE) {
        return tp_string("None");
    } else if (type == TP_DATA) {
        return tp_printf(tp,"<data 0x%x>",self.data.val);
    } else if (type == TP_FNC) {
        return tp_printf(tp,"<fnc 0x%x>",self.fnc.info);
    }
    return tp_string("<?>");
}

/* Function: tp_bool
 * Check the truth value of an object
 *
 * Returns false if v is a numeric object with a value of exactly 0, v is of
 * type None or v is a string list or dictionary with a length of 0. Else true
 * is returned.
 */
int tp_bool(TP,tp_obj v) {
    switch(v.type) {
        case TP_NUMBER: return v.number.val != 0;
        case TP_NONE: return 0;
        case TP_STRING: return v.string.len != 0;
        case TP_LIST: return v.list.val->len != 0;
        case TP_DICT: return v.dict.val->len != 0;
    }
    return 1;
}


/* Function: tp_has
 * Checks if an object contains a key.
 *
 * Returns tp_True if self[k] exists, tp_False otherwise.
 */
tp_obj tp_has(TP,tp_obj self, tp_obj k) {
    int type = self.type;
    if (type == TP_DICT) {
        if (_tp_dict_find(tp,self.dict.val,k) != -1) { return tp_True; }
        return tp_False;
    } else if (type == TP_STRING && k.type == TP_STRING) {
        return tp_number(_tp_str_index(self,k)!=-1);
    } else if (type == TP_LIST) {
        return tp_number(_tp_list_find(tp,self.list.val,k)!=-1);
    }
    tp_raise(tp_None,tp_string("(tp_has) TypeError: iterable argument required"));
}

/* Function: tp_del
 * Remove a dictionary entry.
 *
 * Removes the key k from self. Also works on classes and objects.
 *
 * Note that unlike with Python, you cannot use this to remove list items.
 */
void tp_del(TP,tp_obj self, tp_obj k) {
    int type = self.type;
    if (type == TP_DICT) {
        _tp_dict_del(tp,self.dict.val,k,"tp_del");
        return;
    }
    tp_raise(,tp_string("(tp_del) TypeError: object does not support item deletion"));
}


/* Function: tp_iter
 * Iterate through a list or dict.
 *
 * If self is a list/string/dictionary, this will iterate over the
 * elements/characters/keys respectively, if k is an increasing index
 * starting with 0 up to the length of the object-1.
 *
 * In the case of a list of string, the returned items will correspond to the
 * item at index k. For a dictionary, no guarantees are made about the order.
 * You also cannot call the function with a specific k to get a specific
 * item -- it is only meant for iterating through all items, calling this
 * function len(self) times. Use <tp_get> to retrieve a specific item, and
 * <tp_len> to get the length.
 *
 * Parameters:
 * self - The object over which to iterate.
 * k - You must pass 0 on the first call, then increase it by 1 after each call,
 *     and don't call the function with k >= len(self).
 *
 * Returns:
 * The first (k = 0) or next (k = 1 .. len(self)-1) item in the iteration.
 */
tp_obj tp_iter(TP,tp_obj self, tp_obj k) {
    int type = self.type;
    if (type == TP_LIST || type == TP_STRING) { return tp_get(tp,self,k); }
    if (type == TP_DICT && k.type == TP_NUMBER) {
        return self.dict.val->items[_tp_dict_next(tp,self.dict.val)].key;
    }
    tp_raise(tp_None,tp_string("(tp_iter) TypeError: iteration over non-sequence"));
}


/* Function: tp_get
 * Attribute lookup.
 *
 * This returns the result of using self[k] in actual code. It works for
 * dictionaries (including classes and instantiated objects), lists and strings.
 *
 * As a special case, if self is a list, self[None] will return the first
 * element in the list and subsequently remove it from the list.
 */
tp_obj tp_get(TP,tp_obj self, tp_obj k) {
    int type = self.type;
    tp_obj r;
    if (type == TP_DICT) {
        TP_META_BEGIN(self,"__get__");
            return tp_call(tp,meta,tp_params_v(tp,1,k));
        TP_META_END;
        if (self.dict.dtype && _tp_lookup(tp,self,k,&r)) { return r; }
        return _tp_dict_get(tp,self.dict.val,k,"tp_get");
    } else if (type == TP_LIST) {
        if (k.type == TP_NUMBER) {
            int l = tp_len(tp,self).number.val;
            int n = k.number.val;
            n = (n<0?l+n:n);
            return _tp_list_get(tp,self.list.val,n,"tp_get");
        } else if (k.type == TP_STRING) {
            if (tp_cmp(tp,tp_string("append"),k) == 0) {
                return tp_method(tp,self,tp_append);
            } else if (tp_cmp(tp,tp_string("pop"),k) == 0) {
                return tp_method(tp,self,tp_pop);
            } else if (tp_cmp(tp,tp_string("index"),k) == 0) {
                return tp_method(tp,self,tp_index);
            } else if (tp_cmp(tp,tp_string("sort"),k) == 0) {
                return tp_method(tp,self,tp_sort);
            } else if (tp_cmp(tp,tp_string("extend"),k) == 0) {
                return tp_method(tp,self,tp_extend);
            } else if (tp_cmp(tp,tp_string("*"),k) == 0) {
                tp_params_v(tp,1,self);
                r = tp_copy(tp);
                self.list.val->len=0;
                return r;
            }
        } else if (k.type == TP_NONE) {
            return _tp_list_pop(tp,self.list.val,0,"tp_get");
        }
    } else if (type == TP_STRING) {
        if (k.type == TP_NUMBER) {
            int l = self.string.len;
            int n = k.number.val;
            n = (n<0?l+n:n);
            if (n >= 0 && n < l) { return tp_string_n(tp->chars[(unsigned char)self.string.val[n]],1); }
        } else if (k.type == TP_STRING) {
            if (tp_cmp(tp,tp_string("join"),k) == 0) {
                return tp_method(tp,self,tp_join);
            } else if (tp_cmp(tp,tp_string("split"),k) == 0) {
                return tp_method(tp,self,tp_split);
            } else if (tp_cmp(tp,tp_string("index"),k) == 0) {
                return tp_method(tp,self,tp_str_index);
            } else if (tp_cmp(tp,tp_string("strip"),k) == 0) {
                return tp_method(tp,self,tp_strip);
            } else if (tp_cmp(tp,tp_string("replace"),k) == 0) {
                return tp_method(tp,self,tp_replace);
            }
        }
    }

    if (k.type == TP_LIST) {
        int a,b,l;
        tp_obj tmp;
        l = tp_len(tp,self).number.val;
        tmp = tp_get(tp,k,tp_number(0));
        if (tmp.type == TP_NUMBER) { a = tmp.number.val; }
        else if(tmp.type == TP_NONE) { a = 0; }
        else { tp_raise(tp_None,tp_string("(tp_get) TypeError: indices must be numbers")); }
        tmp = tp_get(tp,k,tp_number(1));
        if (tmp.type == TP_NUMBER) { b = tmp.number.val; }
        else if(tmp.type == TP_NONE) { b = l; }
        else { tp_raise(tp_None,tp_string("(tp_get) TypeError: indices must be numbers")); }
        a = _tp_max(0,(a<0?l+a:a)); b = _tp_min(l,(b<0?l+b:b));
        if (type == TP_LIST) {
            return tp_list_n(tp,b-a,&self.list.val->items[a]);
        } else if (type == TP_STRING) {
            return tp_string_sub(tp,self,a,b);
        }
    }

    tp_raise(tp_None,tp_string("(tp_get) TypeError: ?"));
}

/* Function: tp_iget
 * Failsafe attribute lookup.
 *
 * This is like <tp_get>, except it will return false if the attribute lookup
 * failed. Otherwise, it will return true, and the object will be returned
 * over the reference parameter r.
 */
int tp_iget(TP,tp_obj *r, tp_obj self, tp_obj k) {
    if (self.type == TP_DICT) {
        int n = _tp_dict_find(tp,self.dict.val,k);
        if (n == -1) { return 0; }
        *r = self.dict.val->items[n].val;
        tp_grey(tp,*r);
        return 1;
    }
    if (self.type == TP_LIST && !self.list.val->len) { return 0; }
    *r = tp_get(tp,self,k); tp_grey(tp,*r);
    return 1;
}

/* Function: tp_set
 * Attribute modification.
 *
 * This is the counterpart of tp_get, it does the same as self[k] = v would do
 * in actual tinypy code.
 */
void tp_set(TP,tp_obj self, tp_obj k, tp_obj v) {
    int type = self.type;

    if (type == TP_DICT) {
        TP_META_BEGIN(self,"__set__");
            tp_call(tp,meta,tp_params_v(tp,2,k,v));
            return;
        TP_META_END;
        _tp_dict_set(tp,self.dict.val,k,v);
        return;
    } else if (type == TP_LIST) {
        if (k.type == TP_NUMBER) {
            _tp_list_set(tp,self.list.val,k.number.val,v,"tp_set");
            return;
        } else if (k.type == TP_NONE) {
            _tp_list_append(tp,self.list.val,v);
            return;
        } else if (k.type == TP_STRING) {
            if (tp_cmp(tp,tp_string("*"),k) == 0) {
                tp_params_v(tp,2,self,v); tp_extend(tp);
                return;
            }
        }
    }
    tp_raise(,tp_string("(tp_set) TypeError: object does not support item assignment"));
}

tp_obj tp_add(TP,tp_obj a, tp_obj b) {
    if (a.type == TP_NUMBER && a.type == b.type) {
        return tp_number(a.number.val+b.number.val);
    } else if (a.type == TP_STRING && a.type == b.type) {
        int al = a.string.len, bl = b.string.len;
        tp_obj r = tp_string_t(tp,al+bl);
        char *s = r.string.info->s;
        memcpy(s,a.string.val,al); memcpy(s+al,b.string.val,bl);
        return tp_track(tp,r);
    } else if (a.type == TP_LIST && a.type == b.type) {
        tp_obj r;
        tp_params_v(tp,1,a);
        r = tp_copy(tp);
        tp_params_v(tp,2,r,b);
        tp_extend(tp);
        return r;
    }
    tp_raise(tp_None,tp_string("(tp_add) TypeError: ?"));
}

tp_obj tp_mul(TP,tp_obj a, tp_obj b) {
    if (a.type == TP_NUMBER && a.type == b.type) {
        return tp_number(a.number.val*b.number.val);
    } else if ((a.type == TP_STRING && b.type == TP_NUMBER) ||
               (a.type == TP_NUMBER && b.type == TP_STRING)) {
        if(a.type == TP_NUMBER) {
            tp_obj c = a; a = b; b = c;
        }
        int al = a.string.len; int n = b.number.val;
        if(n <= 0) {
            tp_obj r = tp_string_t(tp,0);
            return tp_track(tp,r);
        }
        tp_obj r = tp_string_t(tp,al*n);
        char *s = r.string.info->s;
        int i; for (i=0; i<n; i++) { memcpy(s+al*i,a.string.val,al); }
        return tp_track(tp,r);
    }
    tp_raise(tp_None,tp_string("(tp_mul) TypeError: ?"));
}

/* Function: tp_len
 * Returns the length of an object.
 *
 * Returns the number of items in a list or dict, or the length of a string.
 */
tp_obj tp_len(TP,tp_obj self) {
    int type = self.type;
    if (type == TP_STRING) {
        return tp_number(self.string.len);
    } else if (type == TP_DICT) {
        return tp_number(self.dict.val->len);
    } else if (type == TP_LIST) {
        return tp_number(self.list.val->len);
    }

    tp_raise(tp_None,tp_string("(tp_len) TypeError: len() of unsized object"));
}

int tp_cmp(TP,tp_obj a, tp_obj b) {
    if (a.type != b.type) { return a.type-b.type; }
    switch(a.type) {
        case TP_NONE: return 0;
        case TP_NUMBER: return _tp_sign(a.number.val-b.number.val);
        case TP_STRING: {
            int l = _tp_min(a.string.len,b.string.len);
            int v = memcmp(a.string.val,b.string.val,l);
            if (v == 0) {
                v = a.string.len-b.string.len;
            }
            return v;
        }
        case TP_LIST: {
            int n,v; for(n=0;n<_tp_min(a.list.val->len,b.list.val->len);n++) {
        tp_obj aa = a.list.val->items[n]; tp_obj bb = b.list.val->items[n];
            if (aa.type == TP_LIST && bb.type == TP_LIST) { v = aa.list.val-bb.list.val; } else { v = tp_cmp(tp,aa,bb); }
            if (v) { return v; } }
            return a.list.val->len-b.list.val->len;
        }
        case TP_DICT: return a.dict.val - b.dict.val;
        case TP_FNC: return a.fnc.info - b.fnc.info;
        case TP_DATA: return (char*)a.data.val - (char*)b.data.val;
    }
    tp_raise(0,tp_string("(tp_cmp) TypeError: ?"));
}

#define TP_OP(name,expr) \
    tp_obj name(TP,tp_obj _a,tp_obj _b) { \
    if (_a.type == TP_NUMBER && _a.type == _b.type) { \
        tp_num a = _a.number.val; tp_num b = _b.number.val; \
        return tp_number(expr); \
    } \
    tp_raise(tp_None,tp_string("(" #name ") TypeError: unsupported operand type(s)")); \
}

TP_OP(tp_bitwise_and,((long)a)&((long)b));
TP_OP(tp_bitwise_or,((long)a)|((long)b));
TP_OP(tp_bitwise_xor,((long)a)^((long)b));
TP_OP(tp_mod,((long)a)%((long)b));
TP_OP(tp_lsh,((long)a)<<((long)b));
TP_OP(tp_rsh,((long)a)>>((long)b));
TP_OP(tp_sub,a-b);
TP_OP(tp_div,a/b);
TP_OP(tp_pow,pow(a,b));

tp_obj tp_bitwise_not(TP, tp_obj a) {
    if (a.type == TP_NUMBER) {
        return tp_number(~(long)a.number.val);
    }
    tp_raise(tp_None,tp_string("(tp_bitwise_not) TypeError: unsupported operand type"));
}

/**/
/* File: VM
 * Functionality pertaining to the virtual machine.
 */

tp_vm *_tp_init(void) {
    int i;
    tp_vm *tp = (tp_vm*)calloc(sizeof(tp_vm),1);
    tp->time_limit = TP_NO_LIMIT;
    tp->clocks = clock();
    tp->time_elapsed = 0.0;
    tp->mem_limit = TP_NO_LIMIT;
    tp->mem_exceeded = 0;
    tp->mem_used = sizeof(tp_vm);
    tp->cur = 0;
    tp->jmp = 0;
    tp->ex = tp_None;
    tp->root = tp_list_nt(tp);
    for (i=0; i<256; i++) { tp->chars[i][0]=i; }
    tp_gc_init(tp);
    tp->_regs = tp_list(tp);
    for (i=0; i<TP_REGS; i++) { tp_set(tp,tp->_regs,tp_None,tp_None); }
    tp->builtins = tp_dict(tp);
    tp->modules = tp_dict(tp);
    tp->_params = tp_list(tp);
    for (i=0; i<TP_FRAMES; i++) { tp_set(tp,tp->_params,tp_None,tp_list(tp)); }
    tp_set(tp,tp->root,tp_None,tp->builtins);
    tp_set(tp,tp->root,tp_None,tp->modules);
    tp_set(tp,tp->root,tp_None,tp->_regs);
    tp_set(tp,tp->root,tp_None,tp->_params);
    tp_set(tp,tp->builtins,tp_string("MODULES"),tp->modules);
    tp_set(tp,tp->modules,tp_string("BUILTINS"),tp->builtins);
    tp_set(tp,tp->builtins,tp_string("BUILTINS"),tp->builtins);
    tp_obj sys = tp_dict(tp);
    tp_set(tp, sys, tp_string("version"), tp_string("tinypy 1.2+SVN"));
    tp_set(tp,tp->modules, tp_string("sys"), sys);
    tp->regs = tp->_regs.list.val->items;
    tp_full(tp);
    return tp;
}


/* Function: tp_deinit
 * Destroys a VM instance.
 *
 * When you no longer need an instance of tinypy, you can use this to free all
 * memory used by it. Even when you are using only a single tinypy instance, it
 * may be good practice to call this function on shutdown.
 */
void tp_deinit(TP) {
    while (tp->root.list.val->len) {
        _tp_list_pop(tp,tp->root.list.val,0,"tp_deinit");
    }
    tp_full(tp); tp_full(tp);
    tp_delete(tp,tp->root);
    tp_gc_deinit(tp);
    tp->mem_used -= sizeof(tp_vm);
    free(tp);
}

/* tp_frame_*/
void tp_frame(TP,tp_obj globals,tp_obj code,tp_obj *ret_dest) {
    tp_frame_ f;
    f.globals = globals;
    f.code = code;
    f.cur = (tp_code*)f.code.string.val;
    f.jmp = 0;
/*     fprintf(stderr,"tp->cur: %d\n",tp->cur);*/
    f.regs = (tp->cur <= 0?tp->regs:tp->frames[tp->cur].regs+tp->frames[tp->cur].cregs);

    f.regs[0] = f.globals;
    f.regs[1] = f.code;
    f.regs += TP_REGS_EXTRA;

    f.ret_dest = ret_dest;
    f.lineno = 0;
    f.line = tp_string("");
    f.name = tp_string("?");
    f.fname = tp_string("?");
    f.cregs = 0;
/*     return f;*/
    if (f.regs+(256+TP_REGS_EXTRA) >= tp->regs+TP_REGS || tp->cur >= TP_FRAMES-1) {
        tp_raise(,tp_string("(tp_frame) RuntimeError: stack overflow"));
    }
    tp->cur += 1;
    tp->frames[tp->cur] = f;
}

void _tp_raise(TP,tp_obj e) {
    /*char *x = 0; x[0]=0;*/
    if (!tp || !tp->jmp) {
#ifndef CPYTHON_MOD
        printf("\nException:\n"); tp_echo(tp,e); printf("\n");
        exit(-1);
#else
        tp->ex = e;
        printf("\nException:\n"); tp_echo(tp,e); printf("\n");
        longjmp(tp->nextexpr,1);
#endif
    }
    if (e.type != TP_NONE) { tp->ex = e; }
    tp_grey(tp,e);
    longjmp(tp->buf,1);
}

void tp_print_stack(TP) {
    int i;
    printf("\n");
    for (i=0; i<=tp->cur; i++) {
        if (!tp->frames[i].lineno) { continue; }
        printf("File \""); tp_echo(tp,tp->frames[i].fname); printf("\", ");
        printf("line %d, in ",tp->frames[i].lineno);
        tp_echo(tp,tp->frames[i].name); printf("\n ");
        tp_echo(tp,tp->frames[i].line); printf("\n");
    }
    printf("\nException:\n"); tp_echo(tp,tp->ex); printf("\n");
}

void tp_handle(TP) {
    int i;
    for (i=tp->cur; i>=0; i--) {
        if (tp->frames[i].jmp) { break; }
    }
    if (i >= 0) {
        tp->cur = i;
        tp->frames[i].cur = tp->frames[i].jmp;
        tp->frames[i].jmp = 0;
        return;
    }
#ifndef CPYTHON_MOD
    tp_print_stack(tp);
    exit(-1);
#else
    tp_print_stack(tp);
    longjmp(tp->nextexpr,1);
#endif
}

/* Function: tp_call
 * Calls a tinypy function.
 *
 * Use this to call a tinypy function.
 *
 * Parameters:
 * tp - The VM instance.
 * self - The object to call.
 * params - Parameters to pass.
 *
 * Example:
 * > tp_call(tp,
 * >     tp_get(tp, tp->builtins, tp_string("foo")),
 * >     tp_params_v(tp, tp_string("hello")))
 * This will look for a global function named "foo", then call it with a single
 * positional parameter containing the string "hello".
 */
tp_obj tp_call(TP,tp_obj self, tp_obj params) {
    /* I'm not sure we should have to do this, but
    just for giggles we will. */
    tp->params = params;

    if (self.type == TP_DICT) {
        if (self.dict.dtype == 1) {
            tp_obj meta; if (_tp_lookup(tp,self,tp_string("__new__"),&meta)) {
                _tp_list_insert(tp,params.list.val,0,self);
                return tp_call(tp,meta,params);
            }
        } else if (self.dict.dtype == 2) {
            TP_META_BEGIN(self,"__call__");
                return tp_call(tp,meta,params);
            TP_META_END;
        }
    }
    if (self.type == TP_FNC && !(self.fnc.ftype&1)) {
        tp_obj r = _tp_tcall(tp,self);
        tp_grey(tp,r);
        return r;
    }
    if (self.type == TP_FNC) {
        tp_obj dest = tp_None;
        tp_frame(tp,self.fnc.info->globals,self.fnc.info->code,&dest);
        if ((self.fnc.ftype&2)) {
            tp->frames[tp->cur].regs[0] = params;
            _tp_list_insert(tp,params.list.val,0,self.fnc.info->self);
        } else {
            tp->frames[tp->cur].regs[0] = params;
        }
        tp_run(tp,tp->cur);
        return dest;
    }
    tp_params_v(tp,1,self); tp_print(tp);
    tp_raise(tp_None,tp_string("(tp_call) TypeError: object is not callable"));
}


void tp_return(TP, tp_obj v) {
    tp_obj *dest = tp->frames[tp->cur].ret_dest;
    if (dest) { *dest = v; tp_grey(tp,v); }
/*     memset(tp->frames[tp->cur].regs,0,TP_REGS_PER_FRAME*sizeof(tp_obj));
       fprintf(stderr,"regs:%d\n",(tp->frames[tp->cur].cregs+1));*/
    memset(tp->frames[tp->cur].regs-TP_REGS_EXTRA,0,(TP_REGS_EXTRA+tp->frames[tp->cur].cregs)*sizeof(tp_obj));
    tp->cur -= 1;
}

enum {
    TP_IEOF,TP_IADD,TP_ISUB,TP_IMUL,TP_IDIV,TP_IPOW,TP_IBITAND,TP_IBITOR,TP_ICMP,TP_IGET,TP_ISET,
    TP_INUMBER,TP_ISTRING,TP_IGGET,TP_IGSET,TP_IMOVE,TP_IDEF,TP_IPASS,TP_IJUMP,TP_ICALL,
    TP_IRETURN,TP_IIF,TP_IDEBUG,TP_IEQ,TP_ILE,TP_ILT,TP_IDICT,TP_ILIST,TP_INONE,TP_ILEN,
    TP_ILINE,TP_IPARAMS,TP_IIGET,TP_IFILE,TP_INAME,TP_INE,TP_IHAS,TP_IRAISE,TP_ISETJMP,
    TP_IMOD,TP_ILSH,TP_IRSH,TP_IITER,TP_IDEL,TP_IREGS,TP_IBITXOR, TP_IIFN,
    TP_INOT, TP_IBITNOT,
    TP_ITOTAL
};

/* char *tp_strings[TP_ITOTAL] = {
       "EOF","ADD","SUB","MUL","DIV","POW","BITAND","BITOR","CMP","GET","SET","NUM",
       "STR","GGET","GSET","MOVE","DEF","PASS","JUMP","CALL","RETURN","IF","DEBUG",
       "EQ","LE","LT","DICT","LIST","NONE","LEN","LINE","PARAMS","IGET","FILE",
       "NAME","NE","HAS","RAISE","SETJMP","MOD","LSH","RSH","ITER","DEL","REGS",
       "BITXOR", "IFN", "NOT", "BITNOT",
   };*/

#define VA ((int)e.regs.a)
#define VB ((int)e.regs.b)
#define VC ((int)e.regs.c)
#define RA regs[e.regs.a]
#define RB regs[e.regs.b]
#define RC regs[e.regs.c]
#define UVBC (unsigned short)(((VB<<8)+VC))
#define SVBC (short)(((VB<<8)+VC))
#define GA tp_grey(tp,RA)
#define SR(v) f->cur = cur; return(v);


int tp_step(TP) {
    tp_frame_ *f = &tp->frames[tp->cur];
    tp_obj *regs = f->regs;
    tp_code *cur = f->cur;
    while(1) {
    #ifdef TP_SANDBOX
    tp_bounds(tp,cur,1);
    #endif
    tp_code e = *cur;
    /*
     fprintf(stderr,"%2d.%4d: %-6s %3d %3d %3d\n",tp->cur,cur - (tp_code*)f->code.string.val,tp_strings[e.i],VA,VB,VC);
       int i; for(i=0;i<16;i++) { fprintf(stderr,"%d: %s\n",i,TP_xSTR(regs[i])); }
    */
    switch (e.i) {
        case TP_IEOF: tp_return(tp,tp_None); SR(0); break;
        case TP_IADD: RA = tp_add(tp,RB,RC); break;
        case TP_ISUB: RA = tp_sub(tp,RB,RC); break;
        case TP_IMUL: RA = tp_mul(tp,RB,RC); break;
        case TP_IDIV: RA = tp_div(tp,RB,RC); break;
        case TP_IPOW: RA = tp_pow(tp,RB,RC); break;
        case TP_IBITAND: RA = tp_bitwise_and(tp,RB,RC); break;
        case TP_IBITOR:  RA = tp_bitwise_or(tp,RB,RC); break;
        case TP_IBITXOR:  RA = tp_bitwise_xor(tp,RB,RC); break;
        case TP_IMOD:  RA = tp_mod(tp,RB,RC); break;
        case TP_ILSH:  RA = tp_lsh(tp,RB,RC); break;
        case TP_IRSH:  RA = tp_rsh(tp,RB,RC); break;
        case TP_ICMP: RA = tp_number(tp_cmp(tp,RB,RC)); break;
        case TP_INE: RA = tp_number(tp_cmp(tp,RB,RC)!=0); break;
        case TP_IEQ: RA = tp_number(tp_cmp(tp,RB,RC)==0); break;
        case TP_ILE: RA = tp_number(tp_cmp(tp,RB,RC)<=0); break;
        case TP_ILT: RA = tp_number(tp_cmp(tp,RB,RC)<0); break;
        case TP_IBITNOT:  RA = tp_bitwise_not(tp,RB); break;
        case TP_INOT: RA = tp_number(!tp_bool(tp,RB)); break;
        case TP_IPASS: break;
        case TP_IIF: if (tp_bool(tp,RA)) { cur += 1; } break;
        case TP_IIFN: if (!tp_bool(tp,RA)) { cur += 1; } break;
        case TP_IGET: RA = tp_get(tp,RB,RC); GA; break;
        case TP_IITER:
            if (RC.number.val < tp_len(tp,RB).number.val) {
                RA = tp_iter(tp,RB,RC); GA;
                RC.number.val += 1;
                #ifdef TP_SANDBOX
                tp_bounds(tp,cur,1);
                #endif
                cur += 1;
            }
            break;
        case TP_IHAS: RA = tp_has(tp,RB,RC); break;
        case TP_IIGET: tp_iget(tp,&RA,RB,RC); break;
        case TP_ISET: tp_set(tp,RA,RB,RC); break;
        case TP_IDEL: tp_del(tp,RA,RB); break;
        case TP_IMOVE: RA = RB; break;
        case TP_INUMBER:
            #ifdef TP_SANDBOX
            tp_bounds(tp,cur,sizeof(tp_num)/4);
            #endif
            RA = tp_number(*(tp_num*)(*++cur).string.val);
            cur += sizeof(tp_num)/4;
            continue;
        case TP_ISTRING: {
            #ifdef TP_SANDBOX
            tp_bounds(tp,cur,(UVBC/4)+1);
            #endif
            /* RA = tp_string_n((*(cur+1)).string.val,UVBC); */
            int a = (*(cur+1)).string.val-f->code.string.val;
            RA = tp_string_sub(tp,f->code,a,a+UVBC),
            cur += (UVBC/4)+1;
            }
            break;
        case TP_IDICT: RA = tp_dict_n(tp,VC/2,&RB); break;
        case TP_ILIST: RA = tp_list_n(tp,VC,&RB); break;
        case TP_IPARAMS: RA = tp_params_n(tp,VC,&RB); break;
        case TP_ILEN: RA = tp_len(tp,RB); break;
        case TP_IJUMP: cur += SVBC; continue; break;
        case TP_ISETJMP: f->jmp = SVBC?cur+SVBC:0; break;
        case TP_ICALL:
            #ifdef TP_SANDBOX
            tp_bounds(tp,cur,1);
            #endif
            f->cur = cur + 1;  RA = tp_call(tp,RB,RC); GA;
            return 0; break;
        case TP_IGGET:
            if (!tp_iget(tp,&RA,f->globals,RB)) {
                RA = tp_get(tp,tp->builtins,RB); GA;
            }
            break;
        case TP_IGSET: tp_set(tp,f->globals,RA,RB); break;
        case TP_IDEF: {
/*            RA = tp_def(tp,(*(cur+1)).string.val,f->globals);*/
            #ifdef TP_SANDBOX
            tp_bounds(tp,cur,SVBC);
            #endif
            {
            int a = (*(cur+1)).string.val-f->code.string.val;
            RA = tp_def(tp,
                /*tp_string_n((*(cur+1)).string.val,(SVBC-1)*4),*/
                tp_string_sub(tp,f->code,a,a+(SVBC-1)*4),
                f->globals);
            cur += SVBC; continue;
            }
            }
            break;

        case TP_IRETURN: tp_return(tp,RA); SR(0); break;
        case TP_IRAISE: _tp_raise(tp,RA); SR(0); break;
        case TP_IDEBUG:
            tp_params_v(tp,3,tp_string("DEBUG:"),tp_number(VA),RA); tp_print(tp);
            break;
        case TP_INONE: RA = tp_None; break;
        case TP_ILINE:
            #ifdef TP_SANDBOX
            tp_bounds(tp,cur,VA);
            #endif
            ;
            {
            int a = (*(cur+1)).string.val-f->code.string.val;
/*            f->line = tp_string_n((*(cur+1)).string.val,VA*4-1);*/
            f->line = tp_string_sub(tp,f->code,a,a+VA*4-1);
/*             fprintf(stderr,"%7d: %s\n",UVBC,f->line.string.val);*/
            cur += VA; f->lineno = UVBC;
            }
            break;
        case TP_IFILE: f->fname = RA; break;
        case TP_INAME: f->name = RA; break;
        case TP_IREGS: f->cregs = VA; break;
        default:
            tp_raise(0,tp_string("(tp_step) RuntimeError: invalid instruction"));
            break;
    }
    #ifdef TP_SANDBOX
    tp_time_update(tp);
    tp_mem_update(tp);
    tp_bounds(tp,cur,1);
    #endif
    cur += 1;
    }
    SR(0);
}

void _tp_run(TP,int cur) {
    tp->jmp += 1; if (setjmp(tp->buf)) { tp_handle(tp); }
    while (tp->cur >= cur && tp_step(tp) != -1);
    tp->jmp -= 1;
}

void tp_run(TP,int cur) {
    jmp_buf tmp;
    memcpy(tmp,tp->buf,sizeof(jmp_buf));
    _tp_run(tp,cur);
    memcpy(tp->buf,tmp,sizeof(jmp_buf));
}


tp_obj tp_ez_call(TP, const char *mod, const char *fnc, tp_obj params) {
    tp_obj tmp;
    tmp = tp_get(tp,tp->modules,tp_string(mod));
    tmp = tp_get(tp,tmp,tp_string(fnc));
    return tp_call(tp,tmp,params);
}

tp_obj _tp_import(TP, tp_obj fname, tp_obj name, tp_obj code) {
    tp_obj g;

    if (!((fname.type != TP_NONE && _tp_str_index(fname,tp_string(".tpc"))!=-1) || code.type != TP_NONE)) {
        return tp_ez_call(tp,"py2bc","import_fname",tp_params_v(tp,2,fname,name));
    }

    if (code.type == TP_NONE) {
        tp_params_v(tp,1,fname);
        code = tp_load(tp);
    }

    g = tp_dict(tp);
    tp_set(tp,g,tp_string("__name__"),name);
    tp_set(tp,g,tp_string("__code__"),code);
    tp_set(tp,g,tp_string("__dict__"),g);
    tp_frame(tp,g,code,0);
    tp_set(tp,tp->modules,name,g);

    if (!tp->jmp) { tp_run(tp,tp->cur); }

    return g;
}


/* Function: tp_import
 * Imports a module.
 *
 * Parameters:
 * fname - The filename of a file containing the module's code.
 * name - The name of the module.
 * codes - The module's code.  If this is given, fname is ignored.
 * len - The length of the bytecode.
 *
 * Returns:
 * The module object.
 */
tp_obj tp_import(TP, const char * fname, const char * name, void *codes, int len) {
    tp_obj f = fname?tp_string(fname):tp_None;
    tp_obj bc = codes?tp_string_n((const char*)codes,len):tp_None;
    return _tp_import(tp,f,tp_string(name),bc);
}



tp_obj tp_exec_(TP) {
    tp_obj code = TP_OBJ();
    tp_obj globals = TP_OBJ();
    tp_obj r = tp_None;
    tp_frame(tp,globals,code,&r);
    tp_run(tp,tp->cur);
    return r;
}


tp_obj tp_import_(TP) {
    tp_obj mod = TP_OBJ();
    tp_obj r;

    if (tp_has(tp,tp->modules,mod).number.val) {
        return tp_get(tp,tp->modules,mod);
    }

    r = _tp_import(tp,tp_add(tp,mod,tp_string(".tpc")),mod,tp_None);
    return r;
}

void tp_builtins(TP) {
    tp_obj o;
    struct {const char *s;tp_obj (*f)(TP);} b[] = {
    {"print",tp_print}, {"range",tp_range}, {"min",tp_min},
    {"max",tp_max}, {"bind",tp_bind}, {"copy",tp_copy},
    {"import",tp_import_}, {"len",tp_len_}, {"assert",tp_assert},
    {"str",tp_str2}, {"float",tp_float}, {"system",tp_system},
    {"istype",tp_istype}, {"chr",tp_chr}, {"save",tp_save},
    {"load",tp_load}, {"fpack",tp_fpack}, {"abs",tp_abs},
    {"int",tp_int}, {"exec",tp_exec_}, {"exists",tp_exists},
    {"mtime",tp_mtime}, {"number",tp_float}, {"round",tp_round},
    {"ord",tp_ord}, {"merge",tp_merge}, {"getraw",tp_getraw},
    {"setmeta",tp_setmeta}, {"getmeta",tp_getmeta},
    {"bool", tp_builtins_bool},
    #ifdef TP_SANDBOX
    {"sandbox",tp_sandbox_},
    #endif
    {0,0},
    };
    int i; for(i=0; b[i].s; i++) {
        tp_set(tp,tp->builtins,tp_string(b[i].s),tp_fnc(tp,(tp_obj (*)(tp_vm *))b[i].f));
    }

    o = tp_object(tp);
    tp_set(tp,o,tp_string("__call__"),tp_fnc(tp,tp_object_call));
    tp_set(tp,o,tp_string("__new__"),tp_fnc(tp,tp_object_new));
    tp_set(tp,tp->builtins,tp_string("object"),o);
}


void tp_args(TP,int argc, char *argv[]) {
    tp_obj self = tp_list(tp);
    int i;
    for (i=1; i<argc; i++) { _tp_list_append(tp,self.list.val,tp_string(argv[i])); }
    tp_set(tp,tp->builtins,tp_string("ARGV"),self);
}

tp_obj tp_main(TP,char *fname, void *code, int len) {
    return tp_import(tp,fname,"__main__",code, len);
}

/* Function: tp_compile
 * Compile some tinypy code.
 *
 */
tp_obj tp_compile(TP, tp_obj text, tp_obj fname) {
    return tp_ez_call(tp,"BUILTINS","compile",tp_params_v(tp,2,text,fname));
}

/* Function: tp_exec
 * Execute VM code.
 */
tp_obj tp_exec(TP, tp_obj code, tp_obj globals) {
    tp_obj r=tp_None;
    tp_frame(tp,globals,code,&r);
    tp_run(tp,tp->cur);
    return r;
}

tp_obj tp_eval(TP, const char *text, tp_obj globals) {
    tp_obj code = tp_compile(tp,tp_string(text),tp_string("<eval>"));
    return tp_exec(tp,code,globals);
}

/* Function: tp_init
 * Initializes a new virtual machine.
 *
 * The given parameters have the same format as the parameters to main, and
 * allow passing arguments to your tinypy scripts.
 *
 * Returns:
 * The newly created tinypy instance.
 */
tp_vm *tp_init(int argc, char *argv[]) {
    tp_vm *tp = _tp_init();
    tp_builtins(tp);
    tp_args(tp,argc,argv);
    tp_compiler(tp);
    return tp;
}

/**/
unsigned char tp_tokenize[] = {
44,67,0,0,30,4,0,1,99,108,97,115,115,32,84,111,
107,101,110,58,0,0,0,0,12,0,0,11,116,111,107,101,
110,105,122,101,46,112,121,0,33,0,0,0,12,0,0,1,
63,0,0,0,34,0,0,0,26,0,0,0,12,1,0,5,
84,111,107,101,110,0,0,0,14,1,0,0,12,3,0,7,
115,101,116,109,101,116,97,0,13,2,3,0,15,3,0,0,
12,5,0,6,111,98,106,101,99,116,0,0,13,4,5,0,
31,1,3,2,19,1,2,1,16,1,0,89,44,11,0,0,
30,17,0,2,32,32,32,32,100,101,102,32,95,95,105,110,
105,116,95,95,40,115,101,108,102,44,112,111,115,61,40,48,
44,48,41,44,116,121,112,101,61,39,115,121,109,98,111,108,
39,44,118,97,108,61,78,111,110,101,44,105,116,101,109,115,
61,78,111,110,101,41,58,0,12,1,0,11,116,111,107,101,
110,105,122,101,46,112,121,0,33,1,0,0,12,1,0,8,
95,95,105,110,105,116,95,95,0,0,0,0,34,1,0,0,
28,2,0,0,9,1,0,2,11,3,0,0,0,0,0,0,
0,0,0,0,11,4,0,0,0,0,0,0,0,0,0,0,
27,2,3,2,28,3,0,0,32,2,0,3,12,3,0,6,
115,121,109,98,111,108,0,0,28,4,0,0,32,3,0,4,
28,4,0,0,28,5,0,0,32,4,0,5,28,5,0,0,
28,6,0,0,32,5,0,6,30,17,0,3,32,32,32,32,
32,32,32,32,115,101,108,102,46,112,111,115,44,115,101,108,
102,46,116,121,112,101,44,115,101,108,102,46,118,97,108,44,
115,101,108,102,46,105,116,101,109,115,61,112,111,115,44,116,
121,112,101,44,118,97,108,44,105,116,101,109,115,0,0,0,
15,6,2,0,15,7,3,0,15,8,4,0,15,9,5,0,
12,10,0,3,112,111,115,0,10,1,10,6,12,6,0,4,
116,121,112,101,0,0,0,0,10,1,6,7,12,6,0,3,
118,97,108,0,10,1,6,8,12,6,0,5,105,116,101,109,
115,0,0,0,10,1,6,9,0,0,0,0,12,2,0,8,
95,95,105,110,105,116,95,95,0,0,0,0,10,0,2,1,
30,6,0,5,100,101,102,32,117,95,101,114,114,111,114,40,
99,116,120,44,115,44,105,41,58,0,0,0,16,0,0,175,
44,12,0,0,30,6,0,5,100,101,102,32,117,95,101,114,
114,111,114,40,99,116,120,44,115,44,105,41,58,0,0,0,
12,1,0,11,116,111,107,101,110,105,122,101,46,112,121,0,
33,1,0,0,12,1,0,7,117,95,101,114,114,111,114,0,
34,1,0,0,28,2,0,0,9,1,0,2,28,3,0,0,
9,2,0,3,28,4,0,0,9,3,0,4,30,3,0,6,
32,32,32,32,121,44,120,32,61,32,105,0,11,6,0,0,
0,0,0,0,0,0,0,0,9,5,3,6,15,4,5,0,
11,7,0,0,0,0,0,0,0,0,240,63,9,6,3,7,
15,5,6,0,30,8,0,7,32,32,32,32,108,105,110,101,
32,61,32,115,46,115,112,108,105,116,40,39,92,110,39,41,
91,121,45,49,93,0,0,0,12,8,0,5,115,112,108,105,
116,0,0,0,9,7,2,8,12,8,0,1,10,0,0,0,
31,6,8,1,19,6,7,6,11,8,0,0,0,0,0,0,
0,0,240,63,2,7,4,8,9,6,6,7,15,3,6,0,
30,3,0,8,32,32,32,32,112,32,61,32,39,39,0,0,
12,7,0,0,0,0,0,0,15,6,7,0,30,6,0,9,
32,32,32,32,105,102,32,121,32,60,32,49,48,58,32,112,
32,43,61,32,39,32,39,0,11,8,0,0,0,0,0,0,
0,0,36,64,25,7,4,8,21,7,0,0,18,0,0,6,
12,8,0,1,32,0,0,0,1,7,6,8,15,6,7,0,
18,0,0,1,30,7,0,10,32,32,32,32,105,102,32,121,
32,60,32,49,48,48,58,32,112,32,43,61,32,39,32,32,
39,0,0,0,11,8,0,0,0,0,0,0,0,0,89,64,
25,7,4,8,21,7,0,0,18,0,0,6,12,8,0,2,
32,32,0,0,1,7,6,8,15,6,7,0,18,0,0,1,
30,10,0,11,32,32,32,32,114,32,61,32,112,32,43,32,
115,116,114,40,121,41,32,43,32,34,58,32,34,32,43,32,
108,105,110,101,32,43,32,34,92,110,34,0,12,11,0,3,
115,116,114,0,13,10,11,0,15,11,4,0,31,9,11,1,
19,9,10,9,1,8,6,9,12,9,0,2,58,32,0,0,
1,8,8,9,1,8,8,3,12,9,0,1,10,0,0,0,
1,8,8,9,15,7,8,0,30,9,0,12,32,32,32,32,
114,32,43,61,32,34,32,32,32,32,32,34,43,34,32,34,
42,120,43,34,94,34,32,43,39,92,110,39,0,0,0,0,
12,9,0,5,32,32,32,32,32,0,0,0,12,10,0,1,
32,0,0,0,3,10,10,5,1,9,9,10,12,10,0,1,
94,0,0,0,1,9,9,10,12,10,0,1,10,0,0,0,
1,9,9,10,1,8,7,9,15,7,8,0,30,8,0,13,
32,32,32,32,114,97,105,115,101,32,39,101,114,114,111,114,
58,32,39,43,99,116,120,43,39,92,110,39,43,114,0,0,
12,8,0,7,101,114,114,111,114,58,32,0,1,8,8,1,
12,9,0,1,10,0,0,0,1,8,8,9,1,8,8,7,
37,8,0,0,0,0,0,0,12,2,0,7,117,95,101,114,
114,111,114,0,14,2,0,0,30,11,0,15,73,83,89,77,
66,79,76,83,32,61,32,39,96,45,61,91,93,59,44,46,
47,126,33,64,36,37,94,38,42,40,41,43,123,125,58,60,
62,63,124,39,0,0,0,0,12,2,0,8,73,83,89,77,
66,79,76,83,0,0,0,0,12,3,0,27,96,45,61,91,
93,59,44,46,47,126,33,64,36,37,94,38,42,40,41,43,
123,125,58,60,62,63,124,0,14,2,3,0,30,3,0,16,
83,89,77,66,79,76,83,32,61,32,91,0,12,2,0,7,
83,89,77,66,79,76,83,0,30,19,0,17,32,32,32,32,
39,100,101,102,39,44,39,99,108,97,115,115,39,44,39,121,
105,101,108,100,39,44,39,114,101,116,117,114,110,39,44,39,
112,97,115,115,39,44,39,97,110,100,39,44,39,111,114,39,
44,39,110,111,116,39,44,39,105,110,39,44,39,105,109,112,
111,114,116,39,44,0,0,0,12,4,0,3,100,101,102,0,
12,5,0,5,99,108,97,115,115,0,0,0,12,6,0,5,
121,105,101,108,100,0,0,0,12,7,0,6,114,101,116,117,
114,110,0,0,12,8,0,4,112,97,115,115,0,0,0,0,
12,9,0,3,97,110,100,0,12,10,0,2,111,114,0,0,
12,11,0,3,110,111,116,0,12,12,0,2,105,110,0,0,
12,13,0,6,105,109,112,111,114,116,0,0,30,17,0,18,
32,32,32,32,39,105,115,39,44,39,119,104,105,108,101,39,
44,39,98,114,101,97,107,39,44,39,102,111,114,39,44,39,
99,111,110,116,105,110,117,101,39,44,39,105,102,39,44,39,
101,108,115,101,39,44,39,101,108,105,102,39,44,39,116,114,
121,39,44,0,12,14,0,2,105,115,0,0,12,15,0,5,
119,104,105,108,101,0,0,0,12,16,0,5,98,114,101,97,
107,0,0,0,12,17,0,3,102,111,114,0,12,18,0,8,
99,111,110,116,105,110,117,101,0,0,0,0,12,19,0,2,
105,102,0,0,12,20,0,4,101,108,115,101,0,0,0,0,
12,21,0,4,101,108,105,102,0,0,0,0,12,22,0,3,
116,114,121,0,30,17,0,19,32,32,32,32,39,101,120,99,
101,112,116,39,44,39,114,97,105,115,101,39,44,39,84,114,
117,101,39,44,39,70,97,108,115,101,39,44,39,78,111,110,
101,39,44,39,103,108,111,98,97,108,39,44,39,100,101,108,
39,44,39,102,114,111,109,39,44,0,0,0,12,23,0,6,
101,120,99,101,112,116,0,0,12,24,0,5,114,97,105,115,
101,0,0,0,12,25,0,4,84,114,117,101,0,0,0,0,
12,26,0,5,70,97,108,115,101,0,0,0,12,27,0,4,
78,111,110,101,0,0,0,0,12,28,0,6,103,108,111,98,
97,108,0,0,12,29,0,3,100,101,108,0,12,30,0,4,
102,114,111,109,0,0,0,0,30,10,0,20,32,32,32,32,
39,45,39,44,39,43,39,44,39,42,39,44,39,42,42,39,
44,39,47,39,44,39,37,39,44,39,60,60,39,44,39,62,
62,39,44,0,12,31,0,1,45,0,0,0,12,32,0,1,
43,0,0,0,12,33,0,1,42,0,0,0,12,34,0,2,
42,42,0,0,12,35,0,1,47,0,0,0,12,36,0,1,
37,0,0,0,12,37,0,2,60,60,0,0,12,38,0,2,
62,62,0,0,30,17,0,21,32,32,32,32,39,45,61,39,
44,39,43,61,39,44,39,42,61,39,44,39,47,61,39,44,
39,61,39,44,39,61,61,39,44,39,33,61,39,44,39,60,
39,44,39,62,39,44,32,39,124,61,39,44,32,39,38,61,
39,44,32,39,94,61,39,44,0,0,0,0,12,39,0,2,
45,61,0,0,12,40,0,2,43,61,0,0,12,41,0,2,
42,61,0,0,12,42,0,2,47,61,0,0,12,43,0,1,
61,0,0,0,12,44,0,2,61,61,0,0,12,45,0,2,
33,61,0,0,12,46,0,1,60,0,0,0,12,47,0,1,
62,0,0,0,12,48,0,2,124,61,0,0,12,49,0,2,
38,61,0,0,12,50,0,2,94,61,0,0,30,18,0,22,
32,32,32,32,39,60,61,39,44,39,62,61,39,44,39,91,
39,44,39,93,39,44,39,123,39,44,39,125,39,44,39,40,
39,44,39,41,39,44,39,46,39,44,39,58,39,44,39,44,
39,44,39,59,39,44,39,38,39,44,39,124,39,44,39,33,
39,44,32,39,94,39,0,0,12,51,0,2,60,61,0,0,
12,52,0,2,62,61,0,0,12,53,0,1,91,0,0,0,
12,54,0,1,93,0,0,0,12,55,0,1,123,0,0,0,
12,56,0,1,125,0,0,0,12,57,0,1,40,0,0,0,
12,58,0,1,41,0,0,0,12,59,0,1,46,0,0,0,
12,60,0,1,58,0,0,0,12,61,0,1,44,0,0,0,
12,62,0,1,59,0,0,0,12,63,0,1,38,0,0,0,
12,64,0,1,124,0,0,0,12,65,0,1,33,0,0,0,
12,66,0,1,94,0,0,0,27,3,4,63,14,2,3,0,
30,11,0,24,66,95,66,69,71,73,78,44,66,95,69,78,
68,32,61,32,91,39,91,39,44,39,40,39,44,39,123,39,
93,44,91,39,93,39,44,39,41,39,44,39,125,39,93,0,
12,4,0,1,91,0,0,0,12,5,0,1,40,0,0,0,
12,6,0,1,123,0,0,0,27,3,4,3,15,2,3,0,
12,5,0,1,93,0,0,0,12,6,0,1,41,0,0,0,
12,7,0,1,125,0,0,0,27,4,5,3,15,3,4,0,
12,4,0,7,66,95,66,69,71,73,78,0,14,4,2,0,
12,2,0,5,66,95,69,78,68,0,0,0,14,2,3,0,
30,4,0,26,99,108,97,115,115,32,84,68,97,116,97,58,
0,0,0,0,26,2,0,0,12,3,0,5,84,68,97,116,
97,0,0,0,14,3,2,0,12,5,0,7,115,101,116,109,
101,116,97,0,13,4,5,0,15,5,2,0,12,7,0,6,
111,98,106,101,99,116,0,0,13,6,7,0,31,3,5,2,
19,3,4,3,16,3,0,91,44,6,0,0,30,6,0,27,
32,32,32,32,100,101,102,32,95,95,105,110,105,116,95,95,
40,115,101,108,102,41,58,0,12,1,0,11,116,111,107,101,
110,105,122,101,46,112,121,0,33,1,0,0,12,1,0,8,
95,95,105,110,105,116,95,95,0,0,0,0,34,1,0,0,
28,2,0,0,9,1,0,2,30,11,0,28,32,32,32,32,
32,32,32,32,115,101,108,102,46,121,44,115,101,108,102,46,
121,105,44,115,101,108,102,46,110,108,32,61,32,49,44,48,
44,84,114,117,101,0,0,0,11,3,0,0,0,0,0,0,
0,0,240,63,15,2,3,0,11,4,0,0,0,0,0,0,
0,0,0,0,15,3,4,0,11,5,0,0,0,0,0,0,
0,0,240,63,15,4,5,0,12,5,0,1,121,0,0,0,
10,1,5,2,12,2,0,2,121,105,0,0,10,1,2,3,
12,2,0,2,110,108,0,0,10,1,2,4,30,13,0,29,
32,32,32,32,32,32,32,32,115,101,108,102,46,114,101,115,
44,115,101,108,102,46,105,110,100,101,110,116,44,115,101,108,
102,46,98,114,97,99,101,115,32,61,32,91,93,44,91,48,
93,44,48,0,27,3,0,0,15,2,3,0,11,5,0,0,
0,0,0,0,0,0,0,0,27,4,5,1,15,3,4,0,
11,5,0,0,0,0,0,0,0,0,0,0,15,4,5,0,
12,5,0,3,114,101,115,0,10,1,5,2,12,2,0,6,
105,110,100,101,110,116,0,0,10,1,2,3,12,2,0,6,
98,114,97,99,101,115,0,0,10,1,2,4,0,0,0,0,
12,4,0,8,95,95,105,110,105,116,95,95,0,0,0,0,
10,2,4,3,16,4,0,53,44,12,0,0,30,15,0,30,
32,32,32,32,100,101,102,32,97,100,100,40,115,101,108,102,
44,116,44,118,41,58,32,115,101,108,102,46,114,101,115,46,
97,112,112,101,110,100,40,84,111,107,101,110,40,115,101,108,
102,46,102,44,116,44,118,41,41,0,0,0,12,1,0,11,
116,111,107,101,110,105,122,101,46,112,121,0,33,1,0,0,
12,1,0,3,97,100,100,0,34,1,0,0,28,2,0,0,
9,1,0,2,28,3,0,0,9,2,0,3,28,4,0,0,
9,3,0,4,12,6,0,3,114,101,115,0,9,5,1,6,
12,6,0,6,97,112,112,101,110,100,0,0,9,5,5,6,
12,8,0,5,84,111,107,101,110,0,0,0,13,7,8,0,
12,11,0,1,102,0,0,0,9,8,1,11,15,9,2,0,
15,10,3,0,31,6,8,3,19,6,7,6,31,4,6,1,
19,4,5,4,0,0,0,0,12,5,0,3,97,100,100,0,
10,2,5,4,30,4,0,32,100,101,102,32,99,108,101,97,
110,40,115,41,58,0,0,0,16,2,0,65,44,6,0,0,
30,4,0,32,100,101,102,32,99,108,101,97,110,40,115,41,
58,0,0,0,12,1,0,11,116,111,107,101,110,105,122,101,
46,112,121,0,33,1,0,0,12,1,0,5,99,108,101,97,
110,0,0,0,34,1,0,0,28,2,0,0,9,1,0,2,
30,8,0,33,32,32,32,32,115,32,61,32,115,46,114,101,
112,108,97,99,101,40,39,92,114,92,110,39,44,39,92,110,
39,41,0,0,12,4,0,7,114,101,112,108,97,99,101,0,
9,3,1,4,12,4,0,2,13,10,0,0,12,5,0,1,
10,0,0,0,31,2,4,2,19,2,3,2,15,1,2,0,
30,8,0,34,32,32,32,32,115,32,61,32,115,46,114,101,
112,108,97,99,101,40,39,92,114,39,44,39,92,110,39,41,
0,0,0,0,12,4,0,7,114,101,112,108,97,99,101,0,
9,3,1,4,12,4,0,1,13,0,0,0,12,5,0,1,
10,0,0,0,31,2,4,2,19,2,3,2,15,1,2,0,
30,4,0,35,32,32,32,32,114,101,116,117,114,110,32,115,
0,0,0,0,20,1,0,0,0,0,0,0,12,5,0,5,
99,108,101,97,110,0,0,0,14,5,2,0,30,5,0,37,
100,101,102,32,116,111,107,101,110,105,122,101,40,115,41,58,
0,0,0,0,16,5,0,124,44,10,0,0,30,5,0,37,
100,101,102,32,116,111,107,101,110,105,122,101,40,115,41,58,
0,0,0,0,12,1,0,11,116,111,107,101,110,105,122,101,
46,112,121,0,33,1,0,0,12,1,0,8,116,111,107,101,
110,105,122,101,0,0,0,0,34,1,0,0,28,2,0,0,
9,1,0,2,30,4,0,38,32,32,32,32,103,108,111,98,
97,108,32,84,0,0,0,0,30,5,0,39,32,32,32,32,
115,32,61,32,99,108,101,97,110,40,115,41,0,0,0,0,
12,4,0,5,99,108,101,97,110,0,0,0,13,3,4,0,
15,4,1,0,31,2,4,1,19,2,3,2,15,1,2,0,
30,8,0,40,32,32,32,32,84,44,105,44,108,32,61,32,
84,68,97,116,97,40,41,44,48,44,108,101,110,40,115,41,
0,0,0,0,12,5,0,5,84,68,97,116,97,0,0,0,
13,4,5,0,31,3,0,0,19,3,4,3,15,2,3,0,
11,4,0,0,0,0,0,0,0,0,0,0,15,3,4,0,
12,7,0,3,108,101,110,0,13,6,7,0,15,7,1,0,
31,5,7,1,19,5,6,5,15,4,5,0,12,5,0,1,
84,0,0,0,14,5,2,0,15,2,3,0,15,3,4,0,
30,9,0,41,32,32,32,32,116,114,121,58,32,114,101,116,
117,114,110,32,100,111,95,116,111,107,101,110,105,122,101,40,
115,44,105,44,108,41,0,0,38,0,0,14,12,6,0,11,
100,111,95,116,111,107,101,110,105,122,101,0,13,5,6,0,
15,6,1,0,15,7,2,0,15,8,3,0,31,4,6,3,
19,4,5,4,20,4,0,0,38,0,0,0,18,0,0,29,
30,10,0,42,32,32,32,32,101,120,99,101,112,116,58,32,
117,95,101,114,114,111,114,40,39,116,111,107,101,110,105,122,
101,39,44,115,44,84,46,102,41,0,0,0,12,6,0,7,
117,95,101,114,114,111,114,0,13,5,6,0,12,6,0,8,
116,111,107,101,110,105,122,101,0,0,0,0,15,7,1,0,
12,9,0,1,84,0,0,0,13,8,9,0,12,9,0,1,
102,0,0,0,9,8,8,9,31,4,6,3,19,4,5,4,
0,0,0,0,12,6,0,8,116,111,107,101,110,105,122,101,
0,0,0,0,14,6,5,0,30,6,0,44,100,101,102,32,
100,111,95,116,111,107,101,110,105,122,101,40,115,44,105,44,
108,41,58,0,16,6,2,55,44,11,0,0,30,6,0,44,
100,101,102,32,100,111,95,116,111,107,101,110,105,122,101,40,
115,44,105,44,108,41,58,0,12,1,0,11,116,111,107,101,
110,105,122,101,46,112,121,0,33,1,0,0,12,1,0,11,
100,111,95,116,111,107,101,110,105,122,101,0,34,1,0,0,
28,2,0,0,9,1,0,2,28,3,0,0,9,2,0,3,
28,4,0,0,9,3,0,4,30,4,0,45,32,32,32,32,
103,108,111,98,97,108,32,84,0,0,0,0,30,7,0,46,
32,32,32,32,84,46,102,32,61,32,40,84,46,121,44,105,
45,84,46,121,105,43,49,41,0,0,0,0,12,5,0,1,
84,0,0,0,13,4,5,0,12,8,0,1,84,0,0,0,
13,6,8,0,12,8,0,1,121,0,0,0,9,6,6,8,
12,9,0,1,84,0,0,0,13,8,9,0,12,9,0,2,
121,105,0,0,9,8,8,9,2,7,2,8,11,8,0,0,
0,0,0,0,0,0,240,63,1,7,7,8,27,5,6,2,
12,6,0,1,102,0,0,0,10,4,6,5,30,5,0,47,
32,32,32,32,119,104,105,108,101,32,105,32,60,32,108,58,
0,0,0,0,25,4,2,3,21,4,0,0,18,0,1,202,
30,10,0,48,32,32,32,32,32,32,32,32,99,32,61,32,
115,91,105,93,59,32,84,46,102,32,61,32,40,84,46,121,
44,105,45,84,46,121,105,43,49,41,0,0,9,5,1,2,
15,4,5,0,12,6,0,1,84,0,0,0,13,5,6,0,
12,9,0,1,84,0,0,0,13,7,9,0,12,9,0,1,
121,0,0,0,9,7,7,9,12,10,0,1,84,0,0,0,
13,9,10,0,12,10,0,2,121,105,0,0,9,9,9,10,
2,8,2,9,11,9,0,0,0,0,0,0,0,0,240,63,
1,8,8,9,27,6,7,2,12,7,0,1,102,0,0,0,
10,5,7,6,30,13,0,49,32,32,32,32,32,32,32,32,
105,102,32,84,46,110,108,58,32,84,46,110,108,32,61,32,
70,97,108,115,101,59,32,105,32,61,32,100,111,95,105,110,
100,101,110,116,40,115,44,105,44,108,41,0,12,6,0,1,
84,0,0,0,13,5,6,0,12,6,0,2,110,108,0,0,
9,5,5,6,21,5,0,0,18,0,0,22,12,6,0,1,
84,0,0,0,13,5,6,0,11,6,0,0,0,0,0,0,
0,0,0,0,12,7,0,2,110,108,0,0,10,5,7,6,
12,7,0,9,100,111,95,105,110,100,101,110,116,0,0,0,
13,6,7,0,15,7,1,0,15,8,2,0,15,9,3,0,
31,5,7,3,19,5,6,5,15,2,5,0,18,0,1,121,
30,11,0,50,32,32,32,32,32,32,32,32,101,108,105,102,
32,99,32,61,61,32,39,92,110,39,58,32,105,32,61,32,
100,111,95,110,108,40,115,44,105,44,108,41,0,0,0,0,
12,6,0,1,10,0,0,0,23,5,4,6,21,5,0,0,
18,0,0,12,12,7,0,5,100,111,95,110,108,0,0,0,
13,6,7,0,15,7,1,0,15,8,2,0,15,9,3,0,
31,5,7,3,19,5,6,5,15,2,5,0,18,0,1,93,
30,13,0,51,32,32,32,32,32,32,32,32,101,108,105,102,
32,99,32,105,110,32,73,83,89,77,66,79,76,83,58,32,
105,32,61,32,100,111,95,115,121,109,98,111,108,40,115,44,
105,44,108,41,0,0,0,0,12,6,0,8,73,83,89,77,
66,79,76,83,0,0,0,0,13,5,6,0,36,5,5,4,
21,5,0,0,18,0,0,13,12,7,0,9,100,111,95,115,
121,109,98,111,108,0,0,0,13,6,7,0,15,7,1,0,
15,8,2,0,15,9,3,0,31,5,7,3,19,5,6,5,
15,2,5,0,18,0,1,59,30,15,0,52,32,32,32,32,
32,32,32,32,101,108,105,102,32,99,32,62,61,32,39,48,
39,32,97,110,100,32,99,32,60,61,32,39,57,39,58,32,
105,32,61,32,100,111,95,110,117,109,98,101,114,40,115,44,
105,44,108,41,0,0,0,0,12,5,0,1,48,0,0,0,
24,5,5,4,21,5,0,0,18,0,0,4,12,6,0,1,
57,0,0,0,24,5,4,6,21,5,0,0,18,0,0,13,
12,7,0,9,100,111,95,110,117,109,98,101,114,0,0,0,
13,6,7,0,15,7,1,0,15,8,2,0,15,9,3,0,
31,5,7,3,19,5,6,5,15,2,5,0,18,0,1,21,
30,18,0,54,32,32,32,32,32,32,32,32,32,32,32,32,
40,99,32,62,61,32,39,65,39,32,97,110,100,32,99,32,
60,61,32,39,90,39,41,32,111,114,32,99,32,61,61,32,
39,95,39,58,32,32,105,32,61,32,100,111,95,110,97,109,
101,40,115,44,105,44,108,41,0,0,0,0,30,11,0,53,
32,32,32,32,32,32,32,32,101,108,105,102,32,40,99,32,
62,61,32,39,97,39,32,97,110,100,32,99,32,60,61,32,
39,122,39,41,32,111,114,32,92,0,0,0,12,5,0,1,
97,0,0,0,24,5,5,4,21,5,0,0,18,0,0,4,
12,6,0,1,122,0,0,0,24,5,4,6,46,5,0,0,
18,0,0,28,30,18,0,54,32,32,32,32,32,32,32,32,
32,32,32,32,40,99,32,62,61,32,39,65,39,32,97,110,
100,32,99,32,60,61,32,39,90,39,41,32,111,114,32,99,
32,61,61,32,39,95,39,58,32,32,105,32,61,32,100,111,
95,110,97,109,101,40,115,44,105,44,108,41,0,0,0,0,
12,5,0,1,65,0,0,0,24,5,5,4,21,5,0,0,
18,0,0,4,12,6,0,1,90,0,0,0,24,5,4,6,
46,5,0,0,18,0,0,4,12,6,0,1,95,0,0,0,
23,5,4,6,21,5,0,0,18,0,0,12,12,7,0,7,
100,111,95,110,97,109,101,0,13,6,7,0,15,7,1,0,
15,8,2,0,15,9,3,0,31,5,7,3,19,5,6,5,
15,2,5,0,18,0,0,191,30,13,0,55,32,32,32,32,
32,32,32,32,101,108,105,102,32,99,61,61,39,34,39,32,
111,114,32,99,61,61,34,39,34,58,32,105,32,61,32,100,
111,95,115,116,114,105,110,103,40,115,44,105,44,108,41,0,
12,6,0,1,34,0,0,0,23,5,4,6,46,5,0,0,
18,0,0,4,12,6,0,1,39,0,0,0,23,5,4,6,
21,5,0,0,18,0,0,13,12,7,0,9,100,111,95,115,
116,114,105,110,103,0,0,0,13,6,7,0,15,7,1,0,
15,8,2,0,15,9,3,0,31,5,7,3,19,5,6,5,
15,2,5,0,18,0,0,155,30,11,0,56,32,32,32,32,
32,32,32,32,101,108,105,102,32,99,61,61,39,35,39,58,
32,105,32,61,32,100,111,95,99,111,109,109,101,110,116,40,
115,44,105,44,108,41,0,0,12,6,0,1,35,0,0,0,
23,5,4,6,21,5,0,0,18,0,0,13,12,7,0,10,
100,111,95,99,111,109,109,101,110,116,0,0,13,6,7,0,
15,7,1,0,15,8,2,0,15,9,3,0,31,5,7,3,
19,5,6,5,15,2,5,0,18,0,0,126,30,11,0,57,
32,32,32,32,32,32,32,32,101,108,105,102,32,99,32,61,
61,32,39,92,92,39,32,97,110,100,32,115,91,105,43,49,
93,32,61,61,32,39,92,110,39,58,0,0,12,6,0,1,
92,0,0,0,23,5,4,6,21,5,0,0,18,0,0,9,
11,7,0,0,0,0,0,0,0,0,240,63,1,6,2,7,
9,5,1,6,12,6,0,1,10,0,0,0,23,5,5,6,
21,5,0,0,18,0,0,42,30,10,0,58,32,32,32,32,
32,32,32,32,32,32,32,32,105,32,43,61,32,50,59,32,
84,46,121,44,84,46,121,105,32,61,32,84,46,121,43,49,
44,105,0,0,11,6,0,0,0,0,0,0,0,0,0,64,
1,5,2,6,15,2,5,0,12,7,0,1,84,0,0,0,
13,6,7,0,12,7,0,1,121,0,0,0,9,6,6,7,
11,7,0,0,0,0,0,0,0,0,240,63,1,6,6,7,
15,5,6,0,15,6,2,0,12,8,0,1,84,0,0,0,
13,7,8,0,12,8,0,1,121,0,0,0,10,7,8,5,
12,7,0,1,84,0,0,0,13,5,7,0,12,7,0,2,
121,105,0,0,10,5,7,6,18,0,0,58,30,11,0,59,
32,32,32,32,32,32,32,32,101,108,105,102,32,99,32,61,
61,32,39,32,39,32,111,114,32,99,32,61,61,32,39,92,
116,39,58,32,105,32,43,61,32,49,0,0,12,6,0,1,
32,0,0,0,23,5,4,6,46,5,0,0,18,0,0,4,
12,6,0,1,9,0,0,0,23,5,4,6,21,5,0,0,
18,0,0,7,11,6,0,0,0,0,0,0,0,0,240,63,
1,5,2,6,15,2,5,0,18,0,0,30,30,10,0,60,
32,32,32,32,32,32,32,32,101,108,115,101,58,32,117,95,
101,114,114,111,114,40,39,116,111,107,101,110,105,122,101,39,
44,115,44,84,46,102,41,0,12,7,0,7,117,95,101,114,
114,111,114,0,13,6,7,0,12,7,0,8,116,111,107,101,
110,105,122,101,0,0,0,0,15,8,1,0,12,10,0,1,
84,0,0,0,13,9,10,0,12,10,0,1,102,0,0,0,
9,9,9,10,31,5,7,3,19,5,6,5,18,0,0,1,
18,0,254,53,30,4,0,61,32,32,32,32,105,110,100,101,
110,116,40,48,41,0,0,0,12,7,0,6,105,110,100,101,
110,116,0,0,13,6,7,0,11,7,0,0,0,0,0,0,
0,0,0,0,31,5,7,1,19,5,6,5,30,6,0,62,
32,32,32,32,114,32,61,32,84,46,114,101,115,59,32,84,
32,61,32,78,111,110,101,0,12,7,0,1,84,0,0,0,
13,6,7,0,12,7,0,3,114,101,115,0,9,6,6,7,
15,5,6,0,12,6,0,1,84,0,0,0,28,7,0,0,
14,6,7,0,30,4,0,65,32,32,32,32,114,101,116,117,
114,110,32,114,0,0,0,0,20,5,0,0,0,0,0,0,
12,7,0,11,100,111,95,116,111,107,101,110,105,122,101,0,
14,7,6,0,30,5,0,67,100,101,102,32,100,111,95,110,
108,40,115,44,105,44,108,41,58,0,0,0,16,7,0,121,
44,8,0,0,30,5,0,67,100,101,102,32,100,111,95,110,
108,40,115,44,105,44,108,41,58,0,0,0,12,1,0,11,
116,111,107,101,110,105,122,101,46,112,121,0,33,1,0,0,
12,1,0,5,100,111,95,110,108,0,0,0,34,1,0,0,
28,2,0,0,9,1,0,2,28,3,0,0,9,2,0,3,
28,4,0,0,9,3,0,4,30,6,0,68,32,32,32,32,
105,102,32,110,111,116,32,84,46,98,114,97,99,101,115,58,
0,0,0,0,12,6,0,1,84,0,0,0,13,5,6,0,
12,6,0,6,98,114,97,99,101,115,0,0,9,5,5,6,
47,4,5,0,21,4,0,0,18,0,0,21,30,7,0,69,
32,32,32,32,32,32,32,32,84,46,97,100,100,40,39,110,
108,39,44,78,111,110,101,41,0,0,0,0,12,6,0,1,
84,0,0,0,13,5,6,0,12,6,0,3,97,100,100,0,
9,5,5,6,12,6,0,2,110,108,0,0,28,7,0,0,
31,4,6,2,19,4,5,4,18,0,0,1,30,6,0,70,
32,32,32,32,105,44,84,46,110,108,32,61,32,105,43,49,
44,84,114,117,101,0,0,0,11,6,0,0,0,0,0,0,
0,0,240,63,1,5,2,6,15,4,5,0,11,6,0,0,
0,0,0,0,0,0,240,63,15,5,6,0,15,2,4,0,
12,6,0,1,84,0,0,0,13,4,6,0,12,6,0,2,
110,108,0,0,10,4,6,5,30,6,0,71,32,32,32,32,
84,46,121,44,84,46,121,105,32,61,32,84,46,121,43,49,
44,105,0,0,12,6,0,1,84,0,0,0,13,5,6,0,
12,6,0,1,121,0,0,0,9,5,5,6,11,6,0,0,
0,0,0,0,0,0,240,63,1,5,5,6,15,4,5,0,
15,5,2,0,12,7,0,1,84,0,0,0,13,6,7,0,
12,7,0,1,121,0,0,0,10,6,7,4,12,6,0,1,
84,0,0,0,13,4,6,0,12,6,0,2,121,105,0,0,
10,4,6,5,30,4,0,72,32,32,32,32,114,101,116,117,
114,110,32,105,0,0,0,0,20,2,0,0,0,0,0,0,
12,8,0,5,100,111,95,110,108,0,0,0,14,8,7,0,
30,6,0,74,100,101,102,32,100,111,95,105,110,100,101,110,
116,40,115,44,105,44,108,41,58,0,0,0,16,8,0,144,
44,10,0,0,30,6,0,74,100,101,102,32,100,111,95,105,
110,100,101,110,116,40,115,44,105,44,108,41,58,0,0,0,
12,1,0,11,116,111,107,101,110,105,122,101,46,112,121,0,
33,1,0,0,12,1,0,9,100,111,95,105,110,100,101,110,
116,0,0,0,34,1,0,0,28,2,0,0,9,1,0,2,
28,3,0,0,9,2,0,3,28,4,0,0,9,3,0,4,
30,3,0,75,32,32,32,32,118,32,61,32,48,0,0,0,
11,5,0,0,0,0,0,0,0,0,0,0,15,4,5,0,
30,4,0,76,32,32,32,32,119,104,105,108,101,32,105,60,
108,58,0,0,25,5,2,3,21,5,0,0,18,0,0,53,
30,5,0,77,32,32,32,32,32,32,32,32,99,32,61,32,
115,91,105,93,0,0,0,0,9,6,1,2,15,5,6,0,
30,11,0,78,32,32,32,32,32,32,32,32,105,102,32,99,
32,33,61,32,39,32,39,32,97,110,100,32,99,32,33,61,
32,39,92,116,39,58,32,98,114,101,97,107,0,0,0,0,
12,7,0,1,32,0,0,0,35,6,5,7,21,6,0,0,
18,0,0,4,12,7,0,1,9,0,0,0,35,6,5,7,
21,6,0,0,18,0,0,3,18,0,0,22,18,0,0,1,
30,6,0,79,32,32,32,32,32,32,32,32,105,44,118,32,
61,32,105,43,49,44,118,43,49,0,0,0,11,8,0,0,
0,0,0,0,0,0,240,63,1,7,2,8,15,6,7,0,
11,9,0,0,0,0,0,0,0,0,240,63,1,8,4,9,
15,7,8,0,15,2,6,0,15,4,7,0,18,0,255,202,
30,15,0,80,32,32,32,32,105,102,32,99,32,33,61,32,
39,92,110,39,32,97,110,100,32,99,32,33,61,32,39,35,
39,32,97,110,100,32,110,111,116,32,84,46,98,114,97,99,
101,115,58,32,105,110,100,101,110,116,40,118,41,0,0,0,
12,7,0,1,10,0,0,0,35,6,5,7,21,6,0,0,
18,0,0,4,12,7,0,1,35,0,0,0,35,6,5,7,
21,6,0,0,18,0,0,9,12,8,0,1,84,0,0,0,
13,7,8,0,12,8,0,6,98,114,97,99,101,115,0,0,
9,7,7,8,47,6,7,0,21,6,0,0,18,0,0,9,
12,8,0,6,105,110,100,101,110,116,0,0,13,7,8,0,
15,8,4,0,31,6,8,1,19,6,7,6,18,0,0,1,
30,4,0,81,32,32,32,32,114,101,116,117,114,110,32,105,
0,0,0,0,20,2,0,0,0,0,0,0,12,9,0,9,
100,111,95,105,110,100,101,110,116,0,0,0,14,9,8,0,
30,4,0,83,100,101,102,32,105,110,100,101,110,116,40,118,
41,58,0,0,16,9,0,229,44,8,0,0,30,4,0,83,
100,101,102,32,105,110,100,101,110,116,40,118,41,58,0,0,
12,1,0,11,116,111,107,101,110,105,122,101,46,112,121,0,
33,1,0,0,12,1,0,6,105,110,100,101,110,116,0,0,
34,1,0,0,28,2,0,0,9,1,0,2,30,8,0,84,
32,32,32,32,105,102,32,118,32,61,61,32,84,46,105,110,
100,101,110,116,91,45,49,93,58,32,112,97,115,115,0,0,
12,4,0,1,84,0,0,0,13,3,4,0,12,4,0,6,
105,110,100,101,110,116,0,0,9,3,3,4,11,4,0,0,
0,0,0,0,0,0,240,191,9,3,3,4,23,2,1,3,
21,2,0,0,18,0,0,3,17,0,0,0,18,0,0,186,
30,7,0,85,32,32,32,32,101,108,105,102,32,118,32,62,
32,84,46,105,110,100,101,110,116,91,45,49,93,58,0,0,
12,3,0,1,84,0,0,0,13,2,3,0,12,3,0,6,
105,110,100,101,110,116,0,0,9,2,2,3,11,3,0,0,
0,0,0,0,0,0,240,191,9,2,2,3,25,2,2,1,
21,2,0,0,18,0,0,44,30,7,0,86,32,32,32,32,
32,32,32,32,84,46,105,110,100,101,110,116,46,97,112,112,
101,110,100,40,118,41,0,0,12,4,0,1,84,0,0,0,
13,3,4,0,12,4,0,6,105,110,100,101,110,116,0,0,
9,3,3,4,12,4,0,6,97,112,112,101,110,100,0,0,
9,3,3,4,15,4,1,0,31,2,4,1,19,2,3,2,
30,7,0,87,32,32,32,32,32,32,32,32,84,46,97,100,
100,40,39,105,110,100,101,110,116,39,44,118,41,0,0,0,
12,4,0,1,84,0,0,0,13,3,4,0,12,4,0,3,
97,100,100,0,9,3,3,4,12,4,0,6,105,110,100,101,
110,116,0,0,15,5,1,0,31,2,4,2,19,2,3,2,
18,0,0,121,30,7,0,88,32,32,32,32,101,108,105,102,
32,118,32,60,32,84,46,105,110,100,101,110,116,91,45,49,
93,58,0,0,12,4,0,1,84,0,0,0,13,3,4,0,
12,4,0,6,105,110,100,101,110,116,0,0,9,3,3,4,
11,4,0,0,0,0,0,0,0,0,240,191,9,3,3,4,
25,2,1,3,21,2,0,0,18,0,0,99,30,8,0,89,
32,32,32,32,32,32,32,32,110,32,61,32,84,46,105,110,
100,101,110,116,46,105,110,100,101,120,40,118,41,0,0,0,
12,5,0,1,84,0,0,0,13,4,5,0,12,5,0,6,
105,110,100,101,110,116,0,0,9,4,4,5,12,5,0,5,
105,110,100,101,120,0,0,0,9,4,4,5,15,5,1,0,
31,3,5,1,19,3,4,3,15,2,3,0,30,9,0,90,
32,32,32,32,32,32,32,32,119,104,105,108,101,32,108,101,
110,40,84,46,105,110,100,101,110,116,41,32,62,32,110,43,
49,58,0,0,11,4,0,0,0,0,0,0,0,0,240,63,
1,3,2,4,12,6,0,3,108,101,110,0,13,5,6,0,
12,7,0,1,84,0,0,0,13,6,7,0,12,7,0,6,
105,110,100,101,110,116,0,0,9,6,6,7,31,4,6,1,
19,4,5,4,25,3,3,4,21,3,0,0,18,0,0,45,
30,8,0,91,32,32,32,32,32,32,32,32,32,32,32,32,
118,32,61,32,84,46,105,110,100,101,110,116,46,112,111,112,
40,41,0,0,12,5,0,1,84,0,0,0,13,4,5,0,
12,5,0,6,105,110,100,101,110,116,0,0,9,4,4,5,
12,5,0,3,112,111,112,0,9,4,4,5,31,3,0,0,
19,3,4,3,15,1,3,0,30,8,0,92,32,32,32,32,
32,32,32,32,32,32,32,32,84,46,97,100,100,40,39,100,
101,100,101,110,116,39,44,118,41,0,0,0,12,5,0,1,
84,0,0,0,13,4,5,0,12,5,0,3,97,100,100,0,
9,4,4,5,12,5,0,6,100,101,100,101,110,116,0,0,
15,6,1,0,31,3,5,2,19,3,4,3,18,0,255,194,
18,0,0,1,0,0,0,0,12,10,0,6,105,110,100,101,
110,116,0,0,14,10,9,0,30,6,0,94,100,101,102,32,
100,111,95,115,121,109,98,111,108,40,115,44,105,44,108,41,
58,0,0,0,16,10,1,27,44,13,0,0,30,6,0,94,
100,101,102,32,100,111,95,115,121,109,98,111,108,40,115,44,
105,44,108,41,58,0,0,0,12,1,0,11,116,111,107,101,
110,105,122,101,46,112,121,0,33,1,0,0,12,1,0,9,
100,111,95,115,121,109,98,111,108,0,0,0,34,1,0,0,
28,2,0,0,9,1,0,2,28,3,0,0,9,2,0,3,
28,4,0,0,9,3,0,4,30,5,0,95,32,32,32,32,
115,121,109,98,111,108,115,32,61,32,91,93,0,0,0,0,
27,5,0,0,15,4,5,0,30,6,0,96,32,32,32,32,
118,44,102,44,105,32,61,32,115,91,105,93,44,105,44,105,
43,49,0,0,9,6,1,2,15,5,6,0,15,6,2,0,
11,9,0,0,0,0,0,0,0,0,240,63,1,8,2,9,
15,7,8,0,15,8,5,0,15,5,6,0,15,2,7,0,
30,10,0,97,32,32,32,32,105,102,32,118,32,105,110,32,
83,89,77,66,79,76,83,58,32,115,121,109,98,111,108,115,
46,97,112,112,101,110,100,40,118,41,0,0,12,7,0,7,
83,89,77,66,79,76,83,0,13,6,7,0,36,6,6,8,
21,6,0,0,18,0,0,9,12,9,0,6,97,112,112,101,
110,100,0,0,9,7,4,9,15,9,8,0,31,6,9,1,
19,6,7,6,18,0,0,1,30,4,0,98,32,32,32,32,
119,104,105,108,101,32,105,60,108,58,0,0,25,6,2,3,
21,6,0,0,18,0,0,74,30,5,0,99,32,32,32,32,
32,32,32,32,99,32,61,32,115,91,105,93,0,0,0,0,
9,7,1,2,15,6,7,0,30,9,0,100,32,32,32,32,
32,32,32,32,105,102,32,110,111,116,32,99,32,105,110,32,
73,83,89,77,66,79,76,83,58,32,98,114,101,97,107,0,
12,10,0,8,73,83,89,77,66,79,76,83,0,0,0,0,
13,9,10,0,36,9,9,6,47,7,9,0,21,7,0,0,
18,0,0,3,18,0,0,46,18,0,0,1,30,6,0,101,
32,32,32,32,32,32,32,32,118,44,105,32,61,32,118,43,
99,44,105,43,49,0,0,0,1,9,8,6,15,7,9,0,
11,11,0,0,0,0,0,0,0,0,240,63,1,10,2,11,
15,9,10,0,15,8,7,0,15,2,9,0,30,11,0,102,
32,32,32,32,32,32,32,32,105,102,32,118,32,105,110,32,
83,89,77,66,79,76,83,58,32,115,121,109,98,111,108,115,
46,97,112,112,101,110,100,40,118,41,0,0,12,9,0,7,
83,89,77,66,79,76,83,0,13,7,9,0,36,7,7,8,
21,7,0,0,18,0,0,9,12,10,0,6,97,112,112,101,
110,100,0,0,9,9,4,10,15,10,8,0,31,7,10,1,
19,7,9,7,18,0,0,1,18,0,255,181,30,11,0,103,
32,32,32,32,118,32,61,32,115,121,109,98,111,108,115,46,
112,111,112,40,41,59,32,110,32,61,32,108,101,110,40,118,
41,59,32,105,32,61,32,102,43,110,0,0,12,10,0,3,
112,111,112,0,9,9,4,10,31,7,0,0,19,7,9,7,
15,8,7,0,12,11,0,3,108,101,110,0,13,10,11,0,
15,11,8,0,31,9,11,1,19,9,10,9,15,7,9,0,
1,9,5,7,15,2,9,0,30,6,0,104,32,32,32,32,
84,46,97,100,100,40,39,115,121,109,98,111,108,39,44,118,
41,0,0,0,12,11,0,1,84,0,0,0,13,10,11,0,
12,11,0,3,97,100,100,0,9,10,10,11,12,11,0,6,
115,121,109,98,111,108,0,0,15,12,8,0,31,9,11,2,
19,9,10,9,30,9,0,105,32,32,32,32,105,102,32,118,
32,105,110,32,66,95,66,69,71,73,78,58,32,84,46,98,
114,97,99,101,115,32,43,61,32,49,0,0,12,10,0,7,
66,95,66,69,71,73,78,0,13,9,10,0,36,9,9,8,
21,9,0,0,18,0,0,20,12,10,0,1,84,0,0,0,
13,9,10,0,12,11,0,1,84,0,0,0,13,10,11,0,
12,11,0,6,98,114,97,99,101,115,0,0,9,10,10,11,
11,11,0,0,0,0,0,0,0,0,240,63,1,10,10,11,
12,11,0,6,98,114,97,99,101,115,0,0,10,9,11,10,
18,0,0,1,30,9,0,106,32,32,32,32,105,102,32,118,
32,105,110,32,66,95,69,78,68,58,32,84,46,98,114,97,
99,101,115,32,45,61,32,49,0,0,0,0,12,10,0,5,
66,95,69,78,68,0,0,0,13,9,10,0,36,9,9,8,
21,9,0,0,18,0,0,20,12,10,0,1,84,0,0,0,
13,9,10,0,12,11,0,1,84,0,0,0,13,10,11,0,
12,11,0,6,98,114,97,99,101,115,0,0,9,10,10,11,
11,11,0,0,0,0,0,0,0,0,240,63,2,10,10,11,
12,11,0,6,98,114,97,99,101,115,0,0,10,9,11,10,
18,0,0,1,30,4,0,107,32,32,32,32,114,101,116,117,
114,110,32,105,0,0,0,0,20,2,0,0,0,0,0,0,
12,11,0,9,100,111,95,115,121,109,98,111,108,0,0,0,
14,11,10,0,30,6,0,109,100,101,102,32,100,111,95,110,
117,109,98,101,114,40,115,44,105,44,108,41,58,0,0,0,
16,11,0,240,44,10,0,0,30,6,0,109,100,101,102,32,
100,111,95,110,117,109,98,101,114,40,115,44,105,44,108,41,
58,0,0,0,12,1,0,11,116,111,107,101,110,105,122,101,
46,112,121,0,33,1,0,0,12,1,0,9,100,111,95,110,
117,109,98,101,114,0,0,0,34,1,0,0,28,2,0,0,
9,1,0,2,28,3,0,0,9,2,0,3,28,4,0,0,
9,3,0,4,30,7,0,110,32,32,32,32,118,44,105,44,
99,32,61,115,91,105,93,44,105,43,49,44,115,91,105,93,
0,0,0,0,9,5,1,2,15,4,5,0,11,7,0,0,
0,0,0,0,0,0,240,63,1,6,2,7,15,5,6,0,
9,7,1,2,15,6,7,0,15,7,4,0,15,2,5,0,
15,4,6,0,30,4,0,111,32,32,32,32,119,104,105,108,
101,32,105,60,108,58,0,0,25,5,2,3,21,5,0,0,
18,0,0,74,30,5,0,112,32,32,32,32,32,32,32,32,
99,32,61,32,115,91,105,93,0,0,0,0,9,5,1,2,
15,4,5,0,30,20,0,113,32,32,32,32,32,32,32,32,
105,102,32,40,99,32,60,32,39,48,39,32,111,114,32,99,
32,62,32,39,57,39,41,32,97,110,100,32,40,99,32,60,
32,39,97,39,32,111,114,32,99,32,62,32,39,102,39,41,
32,97,110,100,32,99,32,33,61,32,39,120,39,58,32,98,
114,101,97,107,0,0,0,0,12,6,0,1,48,0,0,0,
25,5,4,6,46,5,0,0,18,0,0,4,12,5,0,1,
57,0,0,0,25,5,5,4,21,5,0,0,18,0,0,9,
12,6,0,1,97,0,0,0,25,5,4,6,46,5,0,0,
18,0,0,4,12,5,0,1,102,0,0,0,25,5,5,4,
21,5,0,0,18,0,0,4,12,6,0,1,120,0,0,0,
35,5,4,6,21,5,0,0,18,0,0,3,18,0,0,19,
18,0,0,1,30,6,0,114,32,32,32,32,32,32,32,32,
118,44,105,32,61,32,118,43,99,44,105,43,49,0,0,0,
1,6,7,4,15,5,6,0,11,9,0,0,0,0,0,0,
0,0,240,63,1,8,2,9,15,6,8,0,15,7,5,0,
15,2,6,0,18,0,255,181,30,5,0,115,32,32,32,32,
105,102,32,99,32,61,61,32,39,46,39,58,0,0,0,0,
12,6,0,1,46,0,0,0,23,5,4,6,21,5,0,0,
18,0,0,78,30,6,0,116,32,32,32,32,32,32,32,32,
118,44,105,32,61,32,118,43,99,44,105,43,49,0,0,0,
1,6,7,4,15,5,6,0,11,9,0,0,0,0,0,0,
0,0,240,63,1,8,2,9,15,6,8,0,15,7,5,0,
15,2,6,0,30,5,0,117,32,32,32,32,32,32,32,32,
119,104,105,108,101,32,105,60,108,58,0,0,25,5,2,3,
21,5,0,0,18,0,0,52,30,6,0,118,32,32,32,32,
32,32,32,32,32,32,32,32,99,32,61,32,115,91,105,93,
0,0,0,0,9,5,1,2,15,4,5,0,30,11,0,119,
32,32,32,32,32,32,32,32,32,32,32,32,105,102,32,99,
32,60,32,39,48,39,32,111,114,32,99,32,62,32,39,57,
39,58,32,98,114,101,97,107,0,0,0,0,12,6,0,1,
48,0,0,0,25,5,4,6,46,5,0,0,18,0,0,4,
12,5,0,1,57,0,0,0,25,5,5,4,21,5,0,0,
18,0,0,3,18,0,0,20,18,0,0,1,30,7,0,120,
32,32,32,32,32,32,32,32,32,32,32,32,118,44,105,32,
61,32,118,43,99,44,105,43,49,0,0,0,1,6,7,4,
15,5,6,0,11,9,0,0,0,0,0,0,0,0,240,63,
1,8,2,9,15,6,8,0,15,7,5,0,15,2,6,0,
18,0,255,203,18,0,0,1,30,6,0,121,32,32,32,32,
84,46,97,100,100,40,39,110,117,109,98,101,114,39,44,118,
41,0,0,0,12,8,0,1,84,0,0,0,13,6,8,0,
12,8,0,3,97,100,100,0,9,6,6,8,12,8,0,6,
110,117,109,98,101,114,0,0,15,9,7,0,31,5,8,2,
19,5,6,5,30,4,0,122,32,32,32,32,114,101,116,117,
114,110,32,105,0,0,0,0,20,2,0,0,0,0,0,0,
12,12,0,9,100,111,95,110,117,109,98,101,114,0,0,0,
14,12,11,0,30,5,0,124,100,101,102,32,100,111,95,110,
97,109,101,40,115,44,105,44,108,41,58,0,16,12,0,194,
44,10,0,0,30,5,0,124,100,101,102,32,100,111,95,110,
97,109,101,40,115,44,105,44,108,41,58,0,12,1,0,11,
116,111,107,101,110,105,122,101,46,112,121,0,33,1,0,0,
12,1,0,7,100,111,95,110,97,109,101,0,34,1,0,0,
28,2,0,0,9,1,0,2,28,3,0,0,9,2,0,3,
28,4,0,0,9,3,0,4,30,5,0,125,32,32,32,32,
118,44,105,32,61,115,91,105,93,44,105,43,49,0,0,0,
9,5,1,2,15,4,5,0,11,7,0,0,0,0,0,0,
0,0,240,63,1,6,2,7,15,5,6,0,15,6,4,0,
15,2,5,0,30,4,0,126,32,32,32,32,119,104,105,108,
101,32,105,60,108,58,0,0,25,4,2,3,21,4,0,0,
18,0,0,90,30,5,0,127,32,32,32,32,32,32,32,32,
99,32,61,32,115,91,105,93,0,0,0,0,9,5,1,2,
15,4,5,0,30,26,0,128,32,32,32,32,32,32,32,32,
105,102,32,40,99,32,60,32,39,97,39,32,111,114,32,99,
32,62,32,39,122,39,41,32,97,110,100,32,40,99,32,60,
32,39,65,39,32,111,114,32,99,32,62,32,39,90,39,41,
32,97,110,100,32,40,99,32,60,32,39,48,39,32,111,114,
32,99,32,62,32,39,57,39,41,32,97,110,100,32,99,32,
33,61,32,39,95,39,58,32,98,114,101,97,107,0,0,0,
12,7,0,1,97,0,0,0,25,5,4,7,46,5,0,0,
18,0,0,4,12,5,0,1,122,0,0,0,25,5,5,4,
21,5,0,0,18,0,0,9,12,7,0,1,65,0,0,0,
25,5,4,7,46,5,0,0,18,0,0,4,12,5,0,1,
90,0,0,0,25,5,5,4,21,5,0,0,18,0,0,9,
12,7,0,1,48,0,0,0,25,5,4,7,46,5,0,0,
18,0,0,4,12,5,0,1,57,0,0,0,25,5,5,4,
21,5,0,0,18,0,0,4,12,7,0,1,95,0,0,0,
35,5,4,7,21,5,0,0,18,0,0,3,18,0,0,19,
18,0,0,1,30,6,0,129,32,32,32,32,32,32,32,32,
118,44,105,32,61,32,118,43,99,44,105,43,49,0,0,0,
1,7,6,4,15,5,7,0,11,9,0,0,0,0,0,0,
0,0,240,63,1,8,2,9,15,7,8,0,15,6,5,0,
15,2,7,0,18,0,255,165,30,10,0,130,32,32,32,32,
105,102,32,118,32,105,110,32,83,89,77,66,79,76,83,58,
32,84,46,97,100,100,40,39,115,121,109,98,111,108,39,44,
118,41,0,0,12,7,0,7,83,89,77,66,79,76,83,0,
13,5,7,0,36,5,5,6,21,5,0,0,18,0,0,14,
12,8,0,1,84,0,0,0,13,7,8,0,12,8,0,3,
97,100,100,0,9,7,7,8,12,8,0,6,115,121,109,98,
111,108,0,0,15,9,6,0,31,5,8,2,19,5,7,5,
18,0,0,22,30,7,0,131,32,32,32,32,101,108,115,101,
58,32,84,46,97,100,100,40,39,110,97,109,101,39,44,118,
41,0,0,0,12,8,0,1,84,0,0,0,13,7,8,0,
12,8,0,3,97,100,100,0,9,7,7,8,12,8,0,4,
110,97,109,101,0,0,0,0,15,9,6,0,31,5,8,2,
19,5,7,5,18,0,0,1,30,4,0,132,32,32,32,32,
114,101,116,117,114,110,32,105,0,0,0,0,20,2,0,0,
0,0,0,0,12,13,0,7,100,111,95,110,97,109,101,0,
14,13,12,0,30,6,0,134,100,101,102,32,100,111,95,115,
116,114,105,110,103,40,115,44,105,44,108,41,58,0,0,0,
16,13,1,240,44,11,0,0,30,6,0,134,100,101,102,32,
100,111,95,115,116,114,105,110,103,40,115,44,105,44,108,41,
58,0,0,0,12,1,0,11,116,111,107,101,110,105,122,101,
46,112,121,0,33,1,0,0,12,1,0,9,100,111,95,115,
116,114,105,110,103,0,0,0,34,1,0,0,28,2,0,0,
9,1,0,2,28,3,0,0,9,2,0,3,28,4,0,0,
9,3,0,4,30,6,0,135,32,32,32,32,118,44,113,44,
105,32,61,32,39,39,44,115,91,105,93,44,105,43,49,0,
12,5,0,0,0,0,0,0,15,4,5,0,9,6,1,2,
15,5,6,0,11,8,0,0,0,0,0,0,0,0,240,63,
1,7,2,8,15,6,7,0,15,7,4,0,15,4,5,0,
15,2,6,0,30,14,0,136,32,32,32,32,105,102,32,40,
108,45,105,41,32,62,61,32,53,32,97,110,100,32,115,91,
105,93,32,61,61,32,113,32,97,110,100,32,115,91,105,43,
49,93,32,61,61,32,113,58,32,35,32,34,34,34,0,0,
11,5,0,0,0,0,0,0,0,0,20,64,2,6,3,2,
24,5,5,6,21,5,0,0,18,0,0,3,9,5,1,2,
23,5,5,4,21,5,0,0,18,0,0,7,11,8,0,0,
0,0,0,0,0,0,240,63,1,6,2,8,9,5,1,6,
23,5,5,4,21,5,0,0,18,0,0,182,30,4,0,137,
32,32,32,32,32,32,32,32,105,32,43,61,32,50,0,0,
11,6,0,0,0,0,0,0,0,0,0,64,1,5,2,6,
15,2,5,0,30,6,0,138,32,32,32,32,32,32,32,32,
119,104,105,108,101,32,105,60,108,45,50,58,0,0,0,0,
11,8,0,0,0,0,0,0,0,0,0,64,2,6,3,8,
25,5,2,6,21,5,0,0,18,0,0,157,30,6,0,139,
32,32,32,32,32,32,32,32,32,32,32,32,99,32,61,32,
115,91,105,93,0,0,0,0,9,6,1,2,15,5,6,0,
30,14,0,140,32,32,32,32,32,32,32,32,32,32,32,32,
105,102,32,99,32,61,61,32,113,32,97,110,100,32,115,91,
105,43,49,93,32,61,61,32,113,32,97,110,100,32,115,91,
105,43,50,93,32,61,61,32,113,58,0,0,23,6,5,4,
21,6,0,0,18,0,0,7,11,9,0,0,0,0,0,0,
0,0,240,63,1,8,2,9,9,6,1,8,23,6,6,4,
21,6,0,0,18,0,0,7,11,9,0,0,0,0,0,0,
0,0,0,64,1,8,2,9,9,6,1,8,23,6,6,4,
21,6,0,0,18,0,0,44,30,6,0,141,32,32,32,32,
32,32,32,32,32,32,32,32,32,32,32,32,105,32,43,61,
32,51,0,0,11,8,0,0,0,0,0,0,0,0,8,64,
1,6,2,8,15,2,6,0,30,9,0,142,32,32,32,32,
32,32,32,32,32,32,32,32,32,32,32,32,84,46,97,100,
100,40,39,115,116,114,105,110,103,39,44,118,41,0,0,0,
12,9,0,1,84,0,0,0,13,8,9,0,12,9,0,3,
97,100,100,0,9,8,8,9,12,9,0,6,115,116,114,105,
110,103,0,0,15,10,7,0,31,6,9,2,19,6,8,6,
30,6,0,143,32,32,32,32,32,32,32,32,32,32,32,32,
32,32,32,32,98,114,101,97,107,0,0,0,18,0,0,72,
18,0,0,70,30,5,0,144,32,32,32,32,32,32,32,32,
32,32,32,32,101,108,115,101,58,0,0,0,30,8,0,145,
32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,
118,44,105,32,61,32,118,43,99,44,105,43,49,0,0,0,
1,8,7,5,15,6,8,0,11,10,0,0,0,0,0,0,
0,0,240,63,1,9,2,10,15,8,9,0,15,7,6,0,
15,2,8,0,30,13,0,146,32,32,32,32,32,32,32,32,
32,32,32,32,32,32,32,32,105,102,32,99,32,61,61,32,
39,92,110,39,58,32,84,46,121,44,84,46,121,105,32,61,
32,84,46,121,43,49,44,105,0,0,0,0,12,8,0,1,
10,0,0,0,23,6,5,8,21,6,0,0,18,0,0,26,
12,9,0,1,84,0,0,0,13,8,9,0,12,9,0,1,
121,0,0,0,9,8,8,9,11,9,0,0,0,0,0,0,
0,0,240,63,1,8,8,9,15,6,8,0,15,8,2,0,
12,10,0,1,84,0,0,0,13,9,10,0,12,10,0,1,
121,0,0,0,10,9,10,6,12,9,0,1,84,0,0,0,
13,6,9,0,12,9,0,2,121,105,0,0,10,6,9,8,
18,0,0,1,18,0,0,1,18,0,255,94,18,0,0,230,
30,5,0,148,32,32,32,32,32,32,32,32,119,104,105,108,
101,32,105,60,108,58,0,0,25,6,2,3,21,6,0,0,
18,0,0,220,30,6,0,149,32,32,32,32,32,32,32,32,
32,32,32,32,99,32,61,32,115,91,105,93,0,0,0,0,
9,6,1,2,15,5,6,0,30,7,0,150,32,32,32,32,
32,32,32,32,32,32,32,32,105,102,32,99,32,61,61,32,
34,92,92,34,58,0,0,0,12,8,0,1,92,0,0,0,
23,6,5,8,21,6,0,0,18,0,0,124,30,9,0,151,
32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,
105,32,61,32,105,43,49,59,32,99,32,61,32,115,91,105,
93,0,0,0,11,8,0,0,0,0,0,0,0,0,240,63,
1,6,2,8,15,2,6,0,9,6,1,2,15,5,6,0,
30,10,0,152,32,32,32,32,32,32,32,32,32,32,32,32,
32,32,32,32,105,102,32,99,32,61,61,32,34,110,34,58,
32,99,32,61,32,39,92,110,39,0,0,0,12,8,0,1,
110,0,0,0,23,6,5,8,21,6,0,0,18,0,0,5,
12,6,0,1,10,0,0,0,15,5,6,0,18,0,0,1,
30,11,0,153,32,32,32,32,32,32,32,32,32,32,32,32,
32,32,32,32,105,102,32,99,32,61,61,32,34,114,34,58,
32,99,32,61,32,99,104,114,40,49,51,41,0,0,0,0,
12,8,0,1,114,0,0,0,23,6,5,8,21,6,0,0,
18,0,0,11,12,9,0,3,99,104,114,0,13,8,9,0,
11,9,0,0,0,0,0,0,0,0,42,64,31,6,9,1,
19,6,8,6,15,5,6,0,18,0,0,1,30,10,0,154,
32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,
105,102,32,99,32,61,61,32,34,116,34,58,32,99,32,61,
32,34,92,116,34,0,0,0,12,8,0,1,116,0,0,0,
23,6,5,8,21,6,0,0,18,0,0,5,12,6,0,1,
9,0,0,0,15,5,6,0,18,0,0,1,30,10,0,155,
32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,
105,102,32,99,32,61,61,32,34,48,34,58,32,99,32,61,
32,34,92,48,34,0,0,0,12,8,0,1,48,0,0,0,
23,6,5,8,21,6,0,0,18,0,0,5,12,6,0,1,
0,0,0,0,15,5,6,0,18,0,0,1,30,8,0,156,
32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,
118,44,105,32,61,32,118,43,99,44,105,43,49,0,0,0,
1,8,7,5,15,6,8,0,11,10,0,0,0,0,0,0,
0,0,240,63,1,9,2,10,15,8,9,0,15,7,6,0,
15,2,8,0,18,0,0,74,30,7,0,157,32,32,32,32,
32,32,32,32,32,32,32,32,101,108,105,102,32,99,32,61,
61,32,113,58,0,0,0,0,23,6,5,4,21,6,0,0,
18,0,0,44,30,6,0,158,32,32,32,32,32,32,32,32,
32,32,32,32,32,32,32,32,105,32,43,61,32,49,0,0,
11,8,0,0,0,0,0,0,0,0,240,63,1,6,2,8,
15,2,6,0,30,9,0,159,32,32,32,32,32,32,32,32,
32,32,32,32,32,32,32,32,84,46,97,100,100,40,39,115,
116,114,105,110,103,39,44,118,41,0,0,0,12,9,0,1,
84,0,0,0,13,8,9,0,12,9,0,3,97,100,100,0,
9,8,8,9,12,9,0,6,115,116,114,105,110,103,0,0,
15,10,7,0,31,6,9,2,19,6,8,6,30,6,0,160,
32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,
98,114,101,97,107,0,0,0,18,0,0,22,18,0,0,20,
30,8,0,162,32,32,32,32,32,32,32,32,32,32,32,32,
32,32,32,32,118,44,105,32,61,32,118,43,99,44,105,43,
49,0,0,0,1,8,7,5,15,6,8,0,11,10,0,0,
0,0,0,0,0,0,240,63,1,9,2,10,15,8,9,0,
15,7,6,0,15,2,8,0,18,0,0,1,18,0,255,35,
18,0,0,1,30,4,0,163,32,32,32,32,114,101,116,117,
114,110,32,105,0,0,0,0,20,2,0,0,0,0,0,0,
12,14,0,9,100,111,95,115,116,114,105,110,103,0,0,0,
14,14,13,0,30,6,0,165,100,101,102,32,100,111,95,99,
111,109,109,101,110,116,40,115,44,105,44,108,41,58,0,0,
16,14,0,83,44,7,0,0,30,6,0,165,100,101,102,32,
100,111,95,99,111,109,109,101,110,116,40,115,44,105,44,108,
41,58,0,0,12,1,0,11,116,111,107,101,110,105,122,101,
46,112,121,0,33,1,0,0,12,1,0,10,100,111,95,99,
111,109,109,101,110,116,0,0,34,1,0,0,28,2,0,0,
9,1,0,2,28,3,0,0,9,2,0,3,28,4,0,0,
9,3,0,4,30,3,0,166,32,32,32,32,105,32,43,61,
32,49,0,0,11,5,0,0,0,0,0,0,0,0,240,63,
1,4,2,5,15,2,4,0,30,4,0,167,32,32,32,32,
119,104,105,108,101,32,105,60,108,58,0,0,25,4,2,3,
21,4,0,0,18,0,0,35,30,5,0,168,32,32,32,32,
32,32,32,32,99,32,61,32,115,91,105,93,0,0,0,0,
9,5,1,2,15,4,5,0,30,7,0,169,32,32,32,32,
32,32,32,32,105,102,32,99,32,61,61,32,39,92,110,39,
58,32,98,114,101,97,107,0,12,6,0,1,10,0,0,0,
23,5,4,6,21,5,0,0,18,0,0,3,18,0,0,13,
18,0,0,1,30,4,0,170,32,32,32,32,32,32,32,32,
105,32,43,61,32,49,0,0,11,6,0,0,0,0,0,0,
0,0,240,63,1,5,2,6,15,2,5,0,18,0,255,220,
30,4,0,171,32,32,32,32,114,101,116,117,114,110,32,105,
0,0,0,0,20,2,0,0,0,0,0,0,12,15,0,10,
100,111,95,99,111,109,109,101,110,116,0,0,14,15,14,0,
0,0,0,0,
};
unsigned char tp_parse[] = {
44,114,0,0,30,6,0,1,105,109,112,111,114,116,32,116,
111,107,101,110,105,122,101,44,32,115,121,115,0,0,0,0,
12,0,0,8,112,97,114,115,101,46,112,121,0,0,0,0,
33,0,0,0,12,0,0,1,63,0,0,0,34,0,0,0,
12,2,0,6,105,109,112,111,114,116,0,0,13,1,2,0,
12,2,0,8,116,111,107,101,110,105,122,101,0,0,0,0,
31,0,2,1,19,0,1,0,12,1,0,8,116,111,107,101,
110,105,122,101,0,0,0,0,14,1,0,0,12,2,0,6,
105,109,112,111,114,116,0,0,13,1,2,0,12,2,0,3,
115,121,115,0,31,0,2,1,19,0,1,0,12,1,0,3,
115,121,115,0,14,1,0,0,30,7,0,2,102,114,111,109,
32,116,111,107,101,110,105,122,101,32,105,109,112,111,114,116,
32,84,111,107,101,110,0,0,12,2,0,6,105,109,112,111,
114,116,0,0,13,1,2,0,12,2,0,8,116,111,107,101,
110,105,122,101,0,0,0,0,31,0,2,1,19,0,1,0,
12,2,0,8,95,95,100,105,99,116,95,95,0,0,0,0,
13,1,2,0,12,3,0,5,84,111,107,101,110,0,0,0,
9,2,0,3,12,0,0,5,84,111,107,101,110,0,0,0,
10,1,0,2,30,8,0,3,105,102,32,110,111,116,32,34,
116,105,110,121,112,121,34,32,105,110,32,115,121,115,46,118,
101,114,115,105,111,110,58,0,12,2,0,3,115,121,115,0,
13,1,2,0,12,2,0,7,118,101,114,115,105,111,110,0,
9,1,1,2,12,2,0,6,116,105,110,121,112,121,0,0,
36,1,1,2,47,0,1,0,21,0,0,0,18,0,0,30,
30,6,0,4,32,32,32,32,102,114,111,109,32,98,111,111,
116,32,105,109,112,111,114,116,32,42,0,0,12,2,0,6,
105,109,112,111,114,116,0,0,13,1,2,0,12,2,0,4,
98,111,111,116,0,0,0,0,31,0,2,1,19,0,1,0,
12,3,0,5,109,101,114,103,101,0,0,0,13,2,3,0,
12,5,0,8,95,95,100,105,99,116,95,95,0,0,0,0,
13,3,5,0,15,4,0,0,31,1,3,2,19,1,2,1,
18,0,0,1,30,5,0,6,100,101,102,32,99,104,101,99,
107,40,116,44,42,118,115,41,58,0,0,0,16,0,0,114,
44,6,0,0,30,5,0,6,100,101,102,32,99,104,101,99,
107,40,116,44,42,118,115,41,58,0,0,0,12,1,0,8,
112,97,114,115,101,46,112,121,0,0,0,0,33,1,0,0,
12,1,0,5,99,104,101,99,107,0,0,0,34,1,0,0,
28,2,0,0,9,1,0,2,12,3,0,1,42,0,0,0,
9,2,0,3,30,9,0,7,32,32,32,32,105,102,32,118,
115,91,48,93,32,61,61,32,78,111,110,101,58,32,114,101,
116,117,114,110,32,84,114,117,101,0,0,0,11,4,0,0,
0,0,0,0,0,0,0,0,9,3,2,4,28,4,0,0,
23,3,3,4,21,3,0,0,18,0,0,6,11,3,0,0,
0,0,0,0,0,0,240,63,20,3,0,0,18,0,0,1,
30,9,0,8,32,32,32,32,105,102,32,116,46,116,121,112,
101,32,105,110,32,118,115,58,32,114,101,116,117,114,110,32,
84,114,117,101,0,0,0,0,12,5,0,4,116,121,112,101,
0,0,0,0,9,4,1,5,36,3,2,4,21,3,0,0,
18,0,0,6,11,3,0,0,0,0,0,0,0,0,240,63,
20,3,0,0,18,0,0,1,30,14,0,9,32,32,32,32,
105,102,32,116,46,116,121,112,101,32,61,61,32,39,115,121,
109,98,111,108,39,32,97,110,100,32,116,46,118,97,108,32,
105,110,32,118,115,58,32,114,101,116,117,114,110,32,84,114,
117,101,0,0,12,4,0,4,116,121,112,101,0,0,0,0,
9,3,1,4,12,4,0,6,115,121,109,98,111,108,0,0,
23,3,3,4,21,3,0,0,18,0,0,5,12,5,0,3,
118,97,108,0,9,4,1,5,36,3,2,4,21,3,0,0,
18,0,0,6,11,3,0,0,0,0,0,0,0,0,240,63,
20,3,0,0,18,0,0,1,30,5,0,10,32,32,32,32,
114,101,116,117,114,110,32,70,97,108,115,101,0,0,0,0,
11,3,0,0,0,0,0,0,0,0,0,0,20,3,0,0,
0,0,0,0,12,1,0,5,99,104,101,99,107,0,0,0,
14,1,0,0,30,4,0,12,100,101,102,32,116,119,101,97,
107,40,107,44,118,41,58,0,16,1,0,101,44,10,0,0,
30,4,0,12,100,101,102,32,116,119,101,97,107,40,107,44,
118,41,58,0,12,1,0,8,112,97,114,115,101,46,112,121,
0,0,0,0,33,1,0,0,12,1,0,5,116,119,101,97,
107,0,0,0,34,1,0,0,28,2,0,0,9,1,0,2,
28,3,0,0,9,2,0,3,30,8,0,13,32,32,32,32,
80,46,115,116,97,99,107,46,97,112,112,101,110,100,40,40,
107,44,100,109,97,112,91,107,93,41,41,0,12,5,0,1,
80,0,0,0,13,4,5,0,12,5,0,5,115,116,97,99,
107,0,0,0,9,4,4,5,12,5,0,6,97,112,112,101,
110,100,0,0,9,4,4,5,15,6,1,0,12,8,0,4,
100,109,97,112,0,0,0,0,13,7,8,0,9,7,7,1,
27,5,6,2,31,3,5,1,19,3,4,3,30,7,0,14,
32,32,32,32,105,102,32,118,58,32,100,109,97,112,91,107,
93,32,61,32,111,109,97,112,91,107,93,0,21,2,0,0,
18,0,0,12,12,4,0,4,100,109,97,112,0,0,0,0,
13,3,4,0,12,5,0,4,111,109,97,112,0,0,0,0,
13,4,5,0,9,4,4,1,10,3,1,4,18,0,0,31,
30,11,0,15,32,32,32,32,101,108,115,101,58,32,100,109,
97,112,91,107,93,32,61,32,123,39,108,98,112,39,58,48,
44,39,110,117,100,39,58,105,116,115,101,108,102,125,0,0,
12,4,0,4,100,109,97,112,0,0,0,0,13,3,4,0,
12,5,0,3,108,98,112,0,11,6,0,0,0,0,0,0,
0,0,0,0,12,7,0,3,110,117,100,0,12,9,0,6,
105,116,115,101,108,102,0,0,13,8,9,0,26,4,5,4,
10,3,1,4,18,0,0,1,0,0,0,0,12,2,0,5,
116,119,101,97,107,0,0,0,14,2,1,0,30,4,0,16,
100,101,102,32,114,101,115,116,111,114,101,40,41,58,0,0,
16,2,0,56,44,6,0,0,30,4,0,16,100,101,102,32,
114,101,115,116,111,114,101,40,41,58,0,0,12,1,0,8,
112,97,114,115,101,46,112,121,0,0,0,0,33,1,0,0,
12,1,0,7,114,101,115,116,111,114,101,0,34,1,0,0,
30,6,0,17,32,32,32,32,107,44,118,32,61,32,80,46,
115,116,97,99,107,46,112,111,112,40,41,0,12,3,0,1,
80,0,0,0,13,2,3,0,12,3,0,5,115,116,97,99,
107,0,0,0,9,2,2,3,12,3,0,3,112,111,112,0,
9,2,2,3,31,1,0,0,19,1,2,1,11,4,0,0,
0,0,0,0,0,0,0,0,9,3,1,4,15,2,3,0,
11,5,0,0,0,0,0,0,0,0,240,63,9,4,1,5,
15,3,4,0,30,4,0,18,32,32,32,32,100,109,97,112,
91,107,93,32,61,32,118,0,12,4,0,4,100,109,97,112,
0,0,0,0,13,1,4,0,10,1,2,3,0,0,0,0,
12,3,0,7,114,101,115,116,111,114,101,0,14,3,2,0,
30,3,0,20,100,101,102,32,99,112,121,40,100,41,58,0,
16,3,0,45,44,6,0,0,30,3,0,20,100,101,102,32,
99,112,121,40,100,41,58,0,12,1,0,8,112,97,114,115,
101,46,112,121,0,0,0,0,33,1,0,0,12,1,0,3,
99,112,121,0,34,1,0,0,28,2,0,0,9,1,0,2,
30,3,0,21,32,32,32,32,114,32,61,32,123,125,0,0,
26,3,0,0,15,2,3,0,30,7,0,22,32,32,32,32,
102,111,114,32,107,32,105,110,32,100,58,32,114,91,107,93,
32,61,32,100,91,107,93,0,11,4,0,0,0,0,0,0,
0,0,0,0,42,3,1,4,18,0,0,4,9,5,1,3,
10,2,3,5,18,0,255,252,30,4,0,23,32,32,32,32,
114,101,116,117,114,110,32,114,0,0,0,0,20,2,0,0,
0,0,0,0,12,4,0,3,99,112,121,0,14,4,3,0,
30,4,0,25,99,108,97,115,115,32,80,68,97,116,97,58,
0,0,0,0,26,4,0,0,12,5,0,5,80,68,97,116,
97,0,0,0,14,5,4,0,12,7,0,7,115,101,116,109,
101,116,97,0,13,6,7,0,15,7,4,0,12,9,0,6,
111,98,106,101,99,116,0,0,13,8,9,0,31,5,7,2,
19,5,6,5,16,5,0,105,44,6,0,0,30,9,0,26,
32,32,32,32,100,101,102,32,95,95,105,110,105,116,95,95,
40,115,101,108,102,44,115,44,116,111,107,101,110,115,41,58,
0,0,0,0,12,1,0,8,112,97,114,115,101,46,112,121,
0,0,0,0,33,1,0,0,12,1,0,8,95,95,105,110,
105,116,95,95,0,0,0,0,34,1,0,0,28,2,0,0,
9,1,0,2,28,3,0,0,9,2,0,3,28,4,0,0,
9,3,0,4,30,5,0,27,32,32,32,32,32,32,32,32,
115,101,108,102,46,115,32,61,32,115,0,0,12,4,0,1,
115,0,0,0,10,1,4,2,30,8,0,28,32,32,32,32,
32,32,32,32,115,101,108,102,46,116,111,107,101,110,115,32,
61,32,116,111,107,101,110,115,0,0,0,0,12,4,0,6,
116,111,107,101,110,115,0,0,10,1,4,3,30,6,0,29,
32,32,32,32,32,32,32,32,115,101,108,102,46,112,111,115,
32,61,32,48,0,0,0,0,11,4,0,0,0,0,0,0,
0,0,0,0,12,5,0,3,112,111,115,0,10,1,5,4,
30,7,0,30,32,32,32,32,32,32,32,32,115,101,108,102,
46,116,111,107,101,110,32,61,32,78,111,110,101,0,0,0,
28,4,0,0,12,5,0,5,116,111,107,101,110,0,0,0,
10,1,5,4,30,6,0,31,32,32,32,32,32,32,32,32,
115,101,108,102,46,115,116,97,99,107,32,61,32,91,93,0,
27,4,0,0,12,5,0,5,115,116,97,99,107,0,0,0,
10,1,5,4,30,7,0,32,32,32,32,32,32,32,32,32,
115,101,108,102,46,95,116,101,114,109,105,110,97,108,32,61,
32,48,0,0,11,4,0,0,0,0,0,0,0,0,0,0,
12,5,0,9,95,116,101,114,109,105,110,97,108,0,0,0,
10,1,5,4,0,0,0,0,12,6,0,8,95,95,105,110,
105,116,95,95,0,0,0,0,10,4,6,5,16,6,0,87,
44,7,0,0,30,5,0,33,32,32,32,32,100,101,102,32,
105,110,105,116,40,115,101,108,102,41,58,0,12,1,0,8,
112,97,114,115,101,46,112,121,0,0,0,0,33,1,0,0,
12,1,0,4,105,110,105,116,0,0,0,0,34,1,0,0,
28,2,0,0,9,1,0,2,30,7,0,34,32,32,32,32,
32,32,32,32,103,108,111,98,97,108,32,111,109,97,112,44,
100,109,97,112,0,0,0,0,30,8,0,35,32,32,32,32,
32,32,32,32,111,109,97,112,32,61,32,99,112,121,40,98,
97,115,101,95,100,109,97,112,41,0,0,0,12,2,0,4,
111,109,97,112,0,0,0,0,12,5,0,3,99,112,121,0,
13,4,5,0,12,6,0,9,98,97,115,101,95,100,109,97,
112,0,0,0,13,5,6,0,31,3,5,1,19,3,4,3,
14,2,3,0,30,8,0,36,32,32,32,32,32,32,32,32,
100,109,97,112,32,61,32,99,112,121,40,98,97,115,101,95,
100,109,97,112,41,0,0,0,12,2,0,4,100,109,97,112,
0,0,0,0,12,5,0,3,99,112,121,0,13,4,5,0,
12,6,0,9,98,97,115,101,95,100,109,97,112,0,0,0,
13,5,6,0,31,3,5,1,19,3,4,3,14,2,3,0,
30,6,0,37,32,32,32,32,32,32,32,32,115,101,108,102,
46,97,100,118,97,110,99,101,40,41,0,0,12,4,0,7,
97,100,118,97,110,99,101,0,9,3,1,4,31,2,0,0,
19,2,3,2,0,0,0,0,12,7,0,4,105,110,105,116,
0,0,0,0,10,4,7,6,16,7,1,21,44,12,0,0,
30,8,0,38,32,32,32,32,100,101,102,32,97,100,118,97,
110,99,101,40,115,101,108,102,44,118,97,108,61,78,111,110,
101,41,58,0,12,1,0,8,112,97,114,115,101,46,112,121,
0,0,0,0,33,1,0,0,12,1,0,7,97,100,118,97,
110,99,101,0,34,1,0,0,28,2,0,0,9,1,0,2,
28,2,0,0,28,3,0,0,32,2,0,3,30,10,0,39,
32,32,32,32,32,32,32,32,105,102,32,110,111,116,32,99,
104,101,99,107,40,115,101,108,102,46,116,111,107,101,110,44,
118,97,108,41,58,0,0,0,12,6,0,5,99,104,101,99,
107,0,0,0,13,5,6,0,12,8,0,5,116,111,107,101,
110,0,0,0,9,6,1,8,15,7,2,0,31,4,6,2,
19,4,5,4,47,3,4,0,21,3,0,0,18,0,0,30,
30,12,0,40,32,32,32,32,32,32,32,32,32,32,32,32,
101,114,114,111,114,40,39,101,120,112,101,99,116,101,100,32,
39,43,118,97,108,44,115,101,108,102,46,116,111,107,101,110,
41,0,0,0,12,5,0,5,101,114,114,111,114,0,0,0,
13,4,5,0,12,5,0,9,101,120,112,101,99,116,101,100,
32,0,0,0,1,5,5,2,12,7,0,5,116,111,107,101,
110,0,0,0,9,6,1,7,31,3,5,2,19,3,4,3,
18,0,0,1,30,10,0,41,32,32,32,32,32,32,32,32,
105,102,32,115,101,108,102,46,112,111,115,32,60,32,108,101,
110,40,115,101,108,102,46,116,111,107,101,110,115,41,58,0,
12,4,0,3,112,111,115,0,9,3,1,4,12,6,0,3,
108,101,110,0,13,5,6,0,12,7,0,6,116,111,107,101,
110,115,0,0,9,6,1,7,31,4,6,1,19,4,5,4,
25,3,3,4,21,3,0,0,18,0,0,40,30,10,0,42,
32,32,32,32,32,32,32,32,32,32,32,32,116,32,61,32,
115,101,108,102,46,116,111,107,101,110,115,91,115,101,108,102,
46,112,111,115,93,0,0,0,12,5,0,6,116,111,107,101,
110,115,0,0,9,4,1,5,12,6,0,3,112,111,115,0,
9,5,1,6,9,4,4,5,15,3,4,0,30,7,0,43,
32,32,32,32,32,32,32,32,32,32,32,32,115,101,108,102,
46,112,111,115,32,43,61,32,49,0,0,0,12,5,0,3,
112,111,115,0,9,4,1,5,11,5,0,0,0,0,0,0,
0,0,240,63,1,4,4,5,12,5,0,3,112,111,115,0,
10,1,5,4,18,0,0,32,30,11,0,45,32,32,32,32,
32,32,32,32,32,32,32,32,116,32,61,32,84,111,107,101,
110,40,40,48,44,48,41,44,39,101,111,102,39,44,39,101,
111,102,39,41,0,0,0,0,12,6,0,5,84,111,107,101,
110,0,0,0,13,5,6,0,11,9,0,0,0,0,0,0,
0,0,0,0,11,10,0,0,0,0,0,0,0,0,0,0,
27,6,9,2,12,7,0,3,101,111,102,0,12,8,0,3,
101,111,102,0,31,4,6,3,19,4,5,4,15,3,4,0,
18,0,0,1,30,7,0,46,32,32,32,32,32,32,32,32,
115,101,108,102,46,116,111,107,101,110,32,61,32,100,111,40,
116,41,0,0,12,6,0,2,100,111,0,0,13,5,6,0,
15,6,3,0,31,4,6,1,19,4,5,4,12,5,0,5,
116,111,107,101,110,0,0,0,10,1,5,4,30,7,0,48,
32,32,32,32,32,32,32,32,115,101,108,102,46,95,116,101,
114,109,105,110,97,108,32,43,61,32,49,0,12,5,0,9,
95,116,101,114,109,105,110,97,108,0,0,0,9,4,1,5,
11,5,0,0,0,0,0,0,0,0,240,63,1,4,4,5,
12,5,0,9,95,116,101,114,109,105,110,97,108,0,0,0,
10,1,5,4,30,14,0,49,32,32,32,32,32,32,32,32,
105,102,32,99,104,101,99,107,40,115,101,108,102,46,116,111,
107,101,110,44,39,110,108,39,44,39,101,111,102,39,44,39,
59,39,44,39,100,101,100,101,110,116,39,41,58,0,0,0,
12,6,0,5,99,104,101,99,107,0,0,0,13,5,6,0,
12,11,0,5,116,111,107,101,110,0,0,0,9,6,1,11,
12,7,0,2,110,108,0,0,12,8,0,3,101,111,102,0,
12,9,0,1,59,0,0,0,12,10,0,6,100,101,100,101,
110,116,0,0,31,4,6,5,19,4,5,4,21,4,0,0,
18,0,0,19,30,8,0,50,32,32,32,32,32,32,32,32,
32,32,32,32,115,101,108,102,46,95,116,101,114,109,105,110,
97,108,32,61,32,48,0,0,11,4,0,0,0,0,0,0,
0,0,0,0,12,5,0,9,95,116,101,114,109,105,110,97,
108,0,0,0,10,1,5,4,18,0,0,1,30,5,0,51,
32,32,32,32,32,32,32,32,114,101,116,117,114,110,32,116,
0,0,0,0,20,3,0,0,0,0,0,0,12,8,0,7,
97,100,118,97,110,99,101,0,10,4,8,7,16,8,0,73,
44,7,0,0,30,6,0,53,32,32,32,32,100,101,102,32,
116,101,114,109,105,110,97,108,40,115,101,108,102,41,58,0,
12,1,0,8,112,97,114,115,101,46,112,121,0,0,0,0,
33,1,0,0,12,1,0,8,116,101,114,109,105,110,97,108,
0,0,0,0,34,1,0,0,28,2,0,0,9,1,0,2,
30,8,0,54,32,32,32,32,32,32,32,32,105,102,32,115,
101,108,102,46,95,116,101,114,109,105,110,97,108,32,62,32,
49,58,0,0,11,2,0,0,0,0,0,0,0,0,240,63,
12,4,0,9,95,116,101,114,109,105,110,97,108,0,0,0,
9,3,1,4,25,2,2,3,21,2,0,0,18,0,0,32,
30,13,0,55,32,32,32,32,32,32,32,32,32,32,32,32,
101,114,114,111,114,40,39,105,110,118,97,108,105,100,32,115,
116,97,116,101,109,101,110,116,39,44,115,101,108,102,46,116,
111,107,101,110,41,0,0,0,12,4,0,5,101,114,114,111,
114,0,0,0,13,3,4,0,12,4,0,17,105,110,118,97,
108,105,100,32,115,116,97,116,101,109,101,110,116,0,0,0,
12,6,0,5,116,111,107,101,110,0,0,0,9,5,1,6,
31,2,4,2,19,2,3,2,18,0,0,1,0,0,0,0,
12,9,0,8,116,101,114,109,105,110,97,108,0,0,0,0,
10,4,9,8,30,5,0,57,100,101,102,32,101,114,114,111,
114,40,99,116,120,44,116,41,58,0,0,0,16,4,0,53,
44,9,0,0,30,5,0,57,100,101,102,32,101,114,114,111,
114,40,99,116,120,44,116,41,58,0,0,0,12,1,0,8,
112,97,114,115,101,46,112,121,0,0,0,0,33,1,0,0,
12,1,0,5,101,114,114,111,114,0,0,0,34,1,0,0,
28,2,0,0,9,1,0,2,28,3,0,0,9,2,0,3,
30,9,0,58,32,32,32,32,116,111,107,101,110,105,122,101,
46,117,95,101,114,114,111,114,40,99,116,120,44,80,46,115,
44,116,46,112,111,115,41,0,12,5,0,8,116,111,107,101,
110,105,122,101,0,0,0,0,13,4,5,0,12,5,0,7,
117,95,101,114,114,111,114,0,9,4,4,5,15,5,1,0,
12,8,0,1,80,0,0,0,13,6,8,0,12,8,0,1,
115,0,0,0,9,6,6,8,12,8,0,3,112,111,115,0,
9,7,2,8,31,3,5,3,19,3,4,3,0,0,0,0,
12,9,0,5,101,114,114,111,114,0,0,0,14,9,4,0,
30,3,0,60,100,101,102,32,110,117,100,40,116,41,58,0,
16,9,0,30,44,5,0,0,30,3,0,60,100,101,102,32,
110,117,100,40,116,41,58,0,12,1,0,8,112,97,114,115,
101,46,112,121,0,0,0,0,33,1,0,0,12,1,0,3,
110,117,100,0,34,1,0,0,28,2,0,0,9,1,0,2,
30,5,0,63,32,32,32,32,114,101,116,117,114,110,32,116,
46,110,117,100,40,116,41,0,12,4,0,3,110,117,100,0,
9,3,1,4,15,4,1,0,31,2,4,1,19,2,3,2,
20,2,0,0,0,0,0,0,12,10,0,3,110,117,100,0,
14,10,9,0,30,5,0,64,100,101,102,32,108,101,100,40,
116,44,108,101,102,116,41,58,0,0,0,0,16,10,0,37,
44,7,0,0,30,5,0,64,100,101,102,32,108,101,100,40,
116,44,108,101,102,116,41,58,0,0,0,0,12,1,0,8,
112,97,114,115,101,46,112,121,0,0,0,0,33,1,0,0,
12,1,0,3,108,101,100,0,34,1,0,0,28,2,0,0,
9,1,0,2,28,3,0,0,9,2,0,3,30,7,0,67,
32,32,32,32,114,101,116,117,114,110,32,116,46,108,101,100,
40,116,44,108,101,102,116,41,0,0,0,0,12,5,0,3,
108,101,100,0,9,4,1,5,15,5,1,0,15,6,2,0,
31,3,5,2,19,3,4,3,20,3,0,0,0,0,0,0,
12,11,0,3,108,101,100,0,14,11,10,0,30,4,0,68,
100,101,102,32,103,101,116,95,108,98,112,40,116,41,58,0,
16,11,0,29,44,4,0,0,30,4,0,68,100,101,102,32,
103,101,116,95,108,98,112,40,116,41,58,0,12,1,0,8,
112,97,114,115,101,46,112,121,0,0,0,0,33,1,0,0,
12,1,0,7,103,101,116,95,108,98,112,0,34,1,0,0,
28,2,0,0,9,1,0,2,30,5,0,71,32,32,32,32,
114,101,116,117,114,110,32,116,46,108,98,112,0,0,0,0,
12,3,0,3,108,98,112,0,9,2,1,3,20,2,0,0,
0,0,0,0,12,12,0,7,103,101,116,95,108,98,112,0,
14,12,11,0,30,5,0,72,100,101,102,32,103,101,116,95,
105,116,101,109,115,40,116,41,58,0,0,0,16,12,0,32,
44,4,0,0,30,5,0,72,100,101,102,32,103,101,116,95,
105,116,101,109,115,40,116,41,58,0,0,0,12,1,0,8,
112,97,114,115,101,46,112,121,0,0,0,0,33,1,0,0,
12,1,0,9,103,101,116,95,105,116,101,109,115,0,0,0,
34,1,0,0,28,2,0,0,9,1,0,2,30,5,0,75,
32,32,32,32,114,101,116,117,114,110,32,116,46,105,116,101,
109,115,0,0,12,3,0,5,105,116,101,109,115,0,0,0,
9,2,1,3,20,2,0,0,0,0,0,0,12,13,0,9,
103,101,116,95,105,116,101,109,115,0,0,0,14,13,12,0,
30,6,0,77,100,101,102,32,101,120,112,114,101,115,115,105,
111,110,40,114,98,112,41,58,0,0,0,0,16,13,0,134,
44,9,0,0,30,6,0,77,100,101,102,32,101,120,112,114,
101,115,115,105,111,110,40,114,98,112,41,58,0,0,0,0,
12,1,0,8,112,97,114,115,101,46,112,121,0,0,0,0,
33,1,0,0,12,1,0,10,101,120,112,114,101,115,115,105,
111,110,0,0,34,1,0,0,28,2,0,0,9,1,0,2,
30,4,0,78,32,32,32,32,116,32,61,32,80,46,116,111,
107,101,110,0,12,4,0,1,80,0,0,0,13,3,4,0,
12,4,0,5,116,111,107,101,110,0,0,0,9,3,3,4,
15,2,3,0,30,4,0,79,32,32,32,32,97,100,118,97,
110,99,101,40,41,0,0,0,12,5,0,7,97,100,118,97,
110,99,101,0,13,4,5,0,31,3,0,0,19,3,4,3,
30,5,0,80,32,32,32,32,108,101,102,116,32,61,32,110,
117,100,40,116,41,0,0,0,12,6,0,3,110,117,100,0,
13,5,6,0,15,6,2,0,31,4,6,1,19,4,5,4,
15,3,4,0,30,9,0,81,32,32,32,32,119,104,105,108,
101,32,114,98,112,32,60,32,103,101,116,95,108,98,112,40,
80,46,116,111,107,101,110,41,58,0,0,0,12,7,0,7,
103,101,116,95,108,98,112,0,13,6,7,0,12,8,0,1,
80,0,0,0,13,7,8,0,12,8,0,5,116,111,107,101,
110,0,0,0,9,7,7,8,31,5,7,1,19,5,6,5,
25,4,1,5,21,4,0,0,18,0,0,44,30,5,0,82,
32,32,32,32,32,32,32,32,116,32,61,32,80,46,116,111,
107,101,110,0,12,5,0,1,80,0,0,0,13,4,5,0,
12,5,0,5,116,111,107,101,110,0,0,0,9,4,4,5,
15,2,4,0,30,5,0,83,32,32,32,32,32,32,32,32,
97,100,118,97,110,99,101,40,41,0,0,0,12,6,0,7,
97,100,118,97,110,99,101,0,13,5,6,0,31,4,0,0,
19,4,5,4,30,7,0,84,32,32,32,32,32,32,32,32,
108,101,102,116,32,61,32,108,101,100,40,116,44,108,101,102,
116,41,0,0,12,6,0,3,108,101,100,0,13,5,6,0,
15,6,2,0,15,7,3,0,31,4,6,2,19,4,5,4,
15,3,4,0,18,0,255,198,30,4,0,85,32,32,32,32,
114,101,116,117,114,110,32,108,101,102,116,0,20,3,0,0,
0,0,0,0,12,14,0,10,101,120,112,114,101,115,115,105,
111,110,0,0,14,14,13,0,30,6,0,87,100,101,102,32,
105,110,102,105,120,95,108,101,100,40,116,44,108,101,102,116,
41,58,0,0,16,14,0,57,44,9,0,0,30,6,0,87,
100,101,102,32,105,110,102,105,120,95,108,101,100,40,116,44,
108,101,102,116,41,58,0,0,12,1,0,8,112,97,114,115,
101,46,112,121,0,0,0,0,33,1,0,0,12,1,0,9,
105,110,102,105,120,95,108,101,100,0,0,0,34,1,0,0,
28,2,0,0,9,1,0,2,28,3,0,0,9,2,0,3,
30,10,0,88,32,32,32,32,116,46,105,116,101,109,115,32,
61,32,91,108,101,102,116,44,101,120,112,114,101,115,115,105,
111,110,40,116,46,98,112,41,93,0,0,0,15,4,2,0,
12,7,0,10,101,120,112,114,101,115,115,105,111,110,0,0,
13,6,7,0,12,8,0,2,98,112,0,0,9,7,1,8,
31,5,7,1,19,5,6,5,27,3,4,2,12,4,0,5,
105,116,101,109,115,0,0,0,10,1,4,3,30,4,0,89,
32,32,32,32,114,101,116,117,114,110,32,116,0,0,0,0,
20,1,0,0,0,0,0,0,12,15,0,9,105,110,102,105,
120,95,108,101,100,0,0,0,14,15,14,0,30,6,0,90,
100,101,102,32,105,110,102,105,120,95,105,115,40,116,44,108,
101,102,116,41,58,0,0,0,16,15,0,112,44,9,0,0,
30,6,0,90,100,101,102,32,105,110,102,105,120,95,105,115,
40,116,44,108,101,102,116,41,58,0,0,0,12,1,0,8,
112,97,114,115,101,46,112,121,0,0,0,0,33,1,0,0,
12,1,0,8,105,110,102,105,120,95,105,115,0,0,0,0,
34,1,0,0,28,2,0,0,9,1,0,2,28,3,0,0,
9,2,0,3,30,8,0,91,32,32,32,32,105,102,32,99,
104,101,99,107,40,80,46,116,111,107,101,110,44,39,110,111,
116,39,41,58,0,0,0,0,12,5,0,5,99,104,101,99,
107,0,0,0,13,4,5,0,12,7,0,1,80,0,0,0,
13,5,7,0,12,7,0,5,116,111,107,101,110,0,0,0,
9,5,5,7,12,6,0,3,110,111,116,0,31,3,5,2,
19,3,4,3,21,3,0,0,18,0,0,30,30,6,0,92,
32,32,32,32,32,32,32,32,116,46,118,97,108,32,61,32,
39,105,115,110,111,116,39,0,12,3,0,5,105,115,110,111,
116,0,0,0,12,4,0,3,118,97,108,0,10,1,4,3,
30,6,0,93,32,32,32,32,32,32,32,32,97,100,118,97,
110,99,101,40,39,110,111,116,39,41,0,0,12,5,0,7,
97,100,118,97,110,99,101,0,13,4,5,0,12,5,0,3,
110,111,116,0,31,3,5,1,19,3,4,3,18,0,0,1,
30,10,0,94,32,32,32,32,116,46,105,116,101,109,115,32,
61,32,91,108,101,102,116,44,101,120,112,114,101,115,115,105,
111,110,40,116,46,98,112,41,93,0,0,0,15,4,2,0,
12,7,0,10,101,120,112,114,101,115,115,105,111,110,0,0,
13,6,7,0,12,8,0,2,98,112,0,0,9,7,1,8,
31,5,7,1,19,5,6,5,27,3,4,2,12,4,0,5,
105,116,101,109,115,0,0,0,10,1,4,3,30,4,0,95,
32,32,32,32,114,101,116,117,114,110,32,116,0,0,0,0,
20,1,0,0,0,0,0,0,12,16,0,8,105,110,102,105,
120,95,105,115,0,0,0,0,14,16,15,0,30,6,0,96,
100,101,102,32,105,110,102,105,120,95,110,111,116,40,116,44,
108,101,102,116,41,58,0,0,16,16,0,83,44,9,0,0,
30,6,0,96,100,101,102,32,105,110,102,105,120,95,110,111,
116,40,116,44,108,101,102,116,41,58,0,0,12,1,0,8,
112,97,114,115,101,46,112,121,0,0,0,0,33,1,0,0,
12,1,0,9,105,110,102,105,120,95,110,111,116,0,0,0,
34,1,0,0,28,2,0,0,9,1,0,2,28,3,0,0,
9,2,0,3,30,5,0,97,32,32,32,32,97,100,118,97,
110,99,101,40,39,105,110,39,41,0,0,0,12,5,0,7,
97,100,118,97,110,99,101,0,13,4,5,0,12,5,0,2,
105,110,0,0,31,3,5,1,19,3,4,3,30,5,0,98,
32,32,32,32,116,46,118,97,108,32,61,32,39,110,111,116,
105,110,39,0,12,3,0,5,110,111,116,105,110,0,0,0,
12,4,0,3,118,97,108,0,10,1,4,3,30,10,0,99,
32,32,32,32,116,46,105,116,101,109,115,32,61,32,91,108,
101,102,116,44,101,120,112,114,101,115,115,105,111,110,40,116,
46,98,112,41,93,0,0,0,15,4,2,0,12,7,0,10,
101,120,112,114,101,115,115,105,111,110,0,0,13,6,7,0,
12,8,0,2,98,112,0,0,9,7,1,8,31,5,7,1,
19,5,6,5,27,3,4,2,12,4,0,5,105,116,101,109,
115,0,0,0,10,1,4,3,30,4,0,100,32,32,32,32,
114,101,116,117,114,110,32,116,0,0,0,0,20,1,0,0,
0,0,0,0,12,17,0,9,105,110,102,105,120,95,110,111,
116,0,0,0,14,17,16,0,30,7,0,101,100,101,102,32,
105,110,102,105,120,95,116,117,112,108,101,40,116,44,108,101,
102,116,41,58,0,0,0,0,16,17,0,121,44,8,0,0,
30,7,0,101,100,101,102,32,105,110,102,105,120,95,116,117,
112,108,101,40,116,44,108,101,102,116,41,58,0,0,0,0,
12,1,0,8,112,97,114,115,101,46,112,121,0,0,0,0,
33,1,0,0,12,1,0,11,105,110,102,105,120,95,116,117,
112,108,101,0,34,1,0,0,28,2,0,0,9,1,0,2,
28,3,0,0,9,2,0,3,30,7,0,102,32,32,32,32,
114,32,61,32,101,120,112,114,101,115,115,105,111,110,40,116,
46,98,112,41,0,0,0,0,12,6,0,10,101,120,112,114,
101,115,115,105,111,110,0,0,13,5,6,0,12,7,0,2,
98,112,0,0,9,6,1,7,31,4,6,1,19,4,5,4,
15,3,4,0,30,6,0,103,32,32,32,32,105,102,32,108,
101,102,116,46,118,97,108,32,61,61,32,39,44,39,58,0,
12,5,0,3,118,97,108,0,9,4,2,5,12,5,0,1,
44,0,0,0,23,4,4,5,21,4,0,0,18,0,0,29,
30,8,0,104,32,32,32,32,32,32,32,32,108,101,102,116,
46,105,116,101,109,115,46,97,112,112,101,110,100,40,114,41,
0,0,0,0,12,6,0,5,105,116,101,109,115,0,0,0,
9,5,2,6,12,6,0,6,97,112,112,101,110,100,0,0,
9,5,5,6,15,6,3,0,31,4,6,1,19,4,5,4,
30,5,0,105,32,32,32,32,32,32,32,32,114,101,116,117,
114,110,32,108,101,102,116,0,20,2,0,0,18,0,0,1,
30,6,0,106,32,32,32,32,116,46,105,116,101,109,115,32,
61,32,91,108,101,102,116,44,114,93,0,0,15,5,2,0,
15,6,3,0,27,4,5,2,12,5,0,5,105,116,101,109,
115,0,0,0,10,1,5,4,30,6,0,107,32,32,32,32,
116,46,116,121,112,101,32,61,32,39,116,117,112,108,101,39,
0,0,0,0,12,4,0,5,116,117,112,108,101,0,0,0,
12,5,0,4,116,121,112,101,0,0,0,0,10,1,5,4,
30,4,0,108,32,32,32,32,114,101,116,117,114,110,32,116,
0,0,0,0,20,1,0,0,0,0,0,0,12,18,0,11,
105,110,102,105,120,95,116,117,112,108,101,0,14,18,17,0,
30,3,0,109,100,101,102,32,108,115,116,40,116,41,58,0,
16,18,0,88,44,8,0,0,30,3,0,109,100,101,102,32,
108,115,116,40,116,41,58,0,12,1,0,8,112,97,114,115,
101,46,112,121,0,0,0,0,33,1,0,0,12,1,0,3,
108,115,116,0,34,1,0,0,28,2,0,0,9,1,0,2,
30,7,0,110,32,32,32,32,105,102,32,116,32,61,61,32,
78,111,110,101,58,32,114,101,116,117,114,110,32,91,93,0,
28,3,0,0,23,2,1,3,21,2,0,0,18,0,0,4,
27,2,0,0,20,2,0,0,18,0,0,1,30,11,0,111,
32,32,32,32,105,102,32,99,104,101,99,107,40,116,44,39,
44,39,44,39,116,117,112,108,101,39,44,39,115,116,97,116,
101,109,101,110,116,115,39,41,58,0,0,0,12,4,0,5,
99,104,101,99,107,0,0,0,13,3,4,0,15,4,1,0,
12,5,0,1,44,0,0,0,12,6,0,5,116,117,112,108,
101,0,0,0,12,7,0,10,115,116,97,116,101,109,101,110,
116,115,0,0,31,2,4,4,19,2,3,2,21,2,0,0,
18,0,0,19,30,7,0,112,32,32,32,32,32,32,32,32,
114,101,116,117,114,110,32,103,101,116,95,105,116,101,109,115,
40,116,41,0,12,4,0,9,103,101,116,95,105,116,101,109,
115,0,0,0,13,3,4,0,15,4,1,0,31,2,4,1,
19,2,3,2,20,2,0,0,18,0,0,1,30,4,0,113,
32,32,32,32,114,101,116,117,114,110,32,91,116,93,0,0,
15,3,1,0,27,2,3,1,20,2,0,0,0,0,0,0,
12,19,0,3,108,115,116,0,14,19,18,0,30,5,0,114,
100,101,102,32,105,108,115,116,40,116,121,112,44,116,41,58,
0,0,0,0,16,19,0,51,44,11,0,0,30,5,0,114,
100,101,102,32,105,108,115,116,40,116,121,112,44,116,41,58,
0,0,0,0,12,1,0,8,112,97,114,115,101,46,112,121,
0,0,0,0,33,1,0,0,12,1,0,4,105,108,115,116,
0,0,0,0,34,1,0,0,28,2,0,0,9,1,0,2,
28,3,0,0,9,2,0,3,30,10,0,115,32,32,32,32,
114,101,116,117,114,110,32,84,111,107,101,110,40,116,46,112,
111,115,44,116,121,112,44,116,121,112,44,108,115,116,40,116,
41,41,0,0,12,5,0,5,84,111,107,101,110,0,0,0,
13,4,5,0,12,9,0,3,112,111,115,0,9,5,2,9,
15,6,1,0,15,7,1,0,12,10,0,3,108,115,116,0,
13,9,10,0,15,10,2,0,31,8,10,1,19,8,9,8,
31,3,5,4,19,3,4,3,20,3,0,0,0,0,0,0,
12,20,0,4,105,108,115,116,0,0,0,0,14,20,19,0,
30,6,0,117,100,101,102,32,99,97,108,108,95,108,101,100,
40,116,44,108,101,102,116,41,58,0,0,0,16,20,0,198,
44,11,0,0,30,6,0,117,100,101,102,32,99,97,108,108,
95,108,101,100,40,116,44,108,101,102,116,41,58,0,0,0,
12,1,0,8,112,97,114,115,101,46,112,121,0,0,0,0,
33,1,0,0,12,1,0,8,99,97,108,108,95,108,101,100,
0,0,0,0,34,1,0,0,28,2,0,0,9,1,0,2,
28,3,0,0,9,2,0,3,30,10,0,118,32,32,32,32,
114,32,61,32,84,111,107,101,110,40,116,46,112,111,115,44,
39,99,97,108,108,39,44,39,36,39,44,91,108,101,102,116,
93,41,0,0,12,6,0,5,84,111,107,101,110,0,0,0,
13,5,6,0,12,10,0,3,112,111,115,0,9,6,1,10,
12,7,0,4,99,97,108,108,0,0,0,0,12,8,0,1,
36,0,0,0,15,10,2,0,27,9,10,1,31,4,6,4,
19,4,5,4,15,3,4,0,30,9,0,119,32,32,32,32,
119,104,105,108,101,32,110,111,116,32,99,104,101,99,107,40,
80,46,116,111,107,101,110,44,39,41,39,41,58,0,0,0,
12,7,0,5,99,104,101,99,107,0,0,0,13,6,7,0,
12,9,0,1,80,0,0,0,13,7,9,0,12,9,0,5,
116,111,107,101,110,0,0,0,9,7,7,9,12,8,0,1,
41,0,0,0,31,5,7,2,19,5,6,5,47,4,5,0,
21,4,0,0,18,0,0,99,30,6,0,120,32,32,32,32,
32,32,32,32,116,119,101,97,107,40,39,44,39,44,48,41,
0,0,0,0,12,6,0,5,116,119,101,97,107,0,0,0,
13,5,6,0,12,6,0,1,44,0,0,0,11,7,0,0,
0,0,0,0,0,0,0,0,31,4,6,2,19,4,5,4,
30,10,0,121,32,32,32,32,32,32,32,32,114,46,105,116,
101,109,115,46,97,112,112,101,110,100,40,101,120,112,114,101,
115,115,105,111,110,40,48,41,41,0,0,0,12,6,0,5,
105,116,101,109,115,0,0,0,9,5,3,6,12,6,0,6,
97,112,112,101,110,100,0,0,9,5,5,6,12,8,0,10,
101,120,112,114,101,115,115,105,111,110,0,0,13,7,8,0,
11,8,0,0,0,0,0,0,0,0,0,0,31,6,8,1,
19,6,7,6,31,4,6,1,19,4,5,4,30,11,0,122,
32,32,32,32,32,32,32,32,105,102,32,80,46,116,111,107,
101,110,46,118,97,108,32,61,61,32,39,44,39,58,32,97,
100,118,97,110,99,101,40,39,44,39,41,0,12,5,0,1,
80,0,0,0,13,4,5,0,12,5,0,5,116,111,107,101,
110,0,0,0,9,4,4,5,12,5,0,3,118,97,108,0,
9,4,4,5,12,5,0,1,44,0,0,0,23,4,4,5,
21,4,0,0,18,0,0,10,12,6,0,7,97,100,118,97,
110,99,101,0,13,5,6,0,12,6,0,1,44,0,0,0,
31,4,6,1,19,4,5,4,18,0,0,1,30,5,0,123,
32,32,32,32,32,32,32,32,114,101,115,116,111,114,101,40,
41,0,0,0,12,6,0,7,114,101,115,116,111,114,101,0,
13,5,6,0,31,4,0,0,19,4,5,4,18,0,255,141,
30,5,0,124,32,32,32,32,97,100,118,97,110,99,101,40,
34,41,34,41,0,0,0,0,12,6,0,7,97,100,118,97,
110,99,101,0,13,5,6,0,12,6,0,1,41,0,0,0,
31,4,6,1,19,4,5,4,30,4,0,125,32,32,32,32,
114,101,116,117,114,110,32,114,0,0,0,0,20,3,0,0,
0,0,0,0,12,21,0,8,99,97,108,108,95,108,101,100,
0,0,0,0,14,21,20,0,30,6,0,126,100,101,102,32,
103,101,116,95,108,101,100,40,116,44,108,101,102,116,41,58,
0,0,0,0,16,21,1,148,44,17,0,0,30,6,0,126,
100,101,102,32,103,101,116,95,108,101,100,40,116,44,108,101,
102,116,41,58,0,0,0,0,12,1,0,8,112,97,114,115,
101,46,112,121,0,0,0,0,33,1,0,0,12,1,0,7,
103,101,116,95,108,101,100,0,34,1,0,0,28,2,0,0,
9,1,0,2,28,3,0,0,9,2,0,3,30,10,0,127,
32,32,32,32,114,32,61,32,84,111,107,101,110,40,116,46,
112,111,115,44,39,103,101,116,39,44,39,46,39,44,91,108,
101,102,116,93,41,0,0,0,12,6,0,5,84,111,107,101,
110,0,0,0,13,5,6,0,12,10,0,3,112,111,115,0,
9,6,1,10,12,7,0,3,103,101,116,0,12,8,0,1,
46,0,0,0,15,10,2,0,27,9,10,1,31,4,6,4,
19,4,5,4,15,3,4,0,30,5,0,128,32,32,32,32,
105,116,101,109,115,32,61,32,32,91,108,101,102,116,93,0,
15,6,2,0,27,5,6,1,15,4,5,0,30,5,0,129,
32,32,32,32,109,111,114,101,32,61,32,70,97,108,115,101,
0,0,0,0,11,6,0,0,0,0,0,0,0,0,0,0,
15,5,6,0,30,9,0,130,32,32,32,32,119,104,105,108,
101,32,110,111,116,32,99,104,101,99,107,40,80,46,116,111,
107,101,110,44,39,93,39,41,58,0,0,0,12,9,0,5,
99,104,101,99,107,0,0,0,13,8,9,0,12,11,0,1,
80,0,0,0,13,9,11,0,12,11,0,5,116,111,107,101,
110,0,0,0,9,9,9,11,12,10,0,1,93,0,0,0,
31,7,9,2,19,7,8,7,47,6,7,0,21,6,0,0,
18,0,0,167,30,6,0,131,32,32,32,32,32,32,32,32,
109,111,114,101,32,61,32,70,97,108,115,101,0,0,0,0,
11,6,0,0,0,0,0,0,0,0,0,0,15,5,6,0,
30,8,0,132,32,32,32,32,32,32,32,32,105,102,32,99,
104,101,99,107,40,80,46,116,111,107,101,110,44,39,58,39,
41,58,0,0,12,8,0,5,99,104,101,99,107,0,0,0,
13,7,8,0,12,10,0,1,80,0,0,0,13,8,10,0,
12,10,0,5,116,111,107,101,110,0,0,0,9,8,8,10,
12,9,0,1,58,0,0,0,31,6,8,2,19,6,7,6,
21,6,0,0,18,0,0,47,30,16,0,133,32,32,32,32,
32,32,32,32,32,32,32,32,105,116,101,109,115,46,97,112,
112,101,110,100,40,84,111,107,101,110,40,80,46,116,111,107,
101,110,46,112,111,115,44,39,115,121,109,98,111,108,39,44,
39,78,111,110,101,39,41,41,0,0,0,0,12,8,0,6,
97,112,112,101,110,100,0,0,9,7,4,8,12,10,0,5,
84,111,107,101,110,0,0,0,13,9,10,0,12,13,0,1,
80,0,0,0,13,10,13,0,12,13,0,5,116,111,107,101,
110,0,0,0,9,10,10,13,12,13,0,3,112,111,115,0,
9,10,10,13,12,11,0,6,115,121,109,98,111,108,0,0,
12,12,0,4,78,111,110,101,0,0,0,0,31,8,10,3,
19,8,9,8,31,6,8,1,19,6,7,6,18,0,0,29,
30,10,0,135,32,32,32,32,32,32,32,32,32,32,32,32,
105,116,101,109,115,46,97,112,112,101,110,100,40,101,120,112,
114,101,115,115,105,111,110,40,48,41,41,0,12,8,0,6,
97,112,112,101,110,100,0,0,9,7,4,8,12,10,0,10,
101,120,112,114,101,115,115,105,111,110,0,0,13,9,10,0,
11,10,0,0,0,0,0,0,0,0,0,0,31,8,10,1,
19,8,9,8,31,6,8,1,19,6,7,6,18,0,0,1,
30,8,0,136,32,32,32,32,32,32,32,32,105,102,32,99,
104,101,99,107,40,80,46,116,111,107,101,110,44,39,58,39,
41,58,0,0,12,8,0,5,99,104,101,99,107,0,0,0,
13,7,8,0,12,10,0,1,80,0,0,0,13,8,10,0,
12,10,0,5,116,111,107,101,110,0,0,0,9,8,8,10,
12,9,0,1,58,0,0,0,31,6,8,2,19,6,7,6,
21,6,0,0,18,0,0,29,30,7,0,137,32,32,32,32,
32,32,32,32,32,32,32,32,97,100,118,97,110,99,101,40,
39,58,39,41,0,0,0,0,12,8,0,7,97,100,118,97,
110,99,101,0,13,7,8,0,12,8,0,1,58,0,0,0,
31,6,8,1,19,6,7,6,30,6,0,138,32,32,32,32,
32,32,32,32,32,32,32,32,109,111,114,101,32,61,32,84,
114,117,101,0,11,6,0,0,0,0,0,0,0,0,240,63,
15,5,6,0,18,0,0,1,18,0,255,73,30,4,0,139,
32,32,32,32,105,102,32,109,111,114,101,58,0,0,0,0,
21,5,0,0,18,0,0,46,30,15,0,140,32,32,32,32,
32,32,32,32,105,116,101,109,115,46,97,112,112,101,110,100,
40,84,111,107,101,110,40,80,46,116,111,107,101,110,46,112,
111,115,44,39,115,121,109,98,111,108,39,44,39,78,111,110,
101,39,41,41,0,0,0,0,12,8,0,6,97,112,112,101,
110,100,0,0,9,7,4,8,12,10,0,5,84,111,107,101,
110,0,0,0,13,9,10,0,12,13,0,1,80,0,0,0,
13,10,13,0,12,13,0,5,116,111,107,101,110,0,0,0,
9,10,10,13,12,13,0,3,112,111,115,0,9,10,10,13,
12,11,0,6,115,121,109,98,111,108,0,0,12,12,0,4,
78,111,110,101,0,0,0,0,31,8,10,3,19,8,9,8,
31,6,8,1,19,6,7,6,18,0,0,1,30,6,0,141,
32,32,32,32,105,102,32,108,101,110,40,105,116,101,109,115,
41,32,62,32,50,58,0,0,11,6,0,0,0,0,0,0,
0,0,0,64,12,9,0,3,108,101,110,0,13,8,9,0,
15,9,4,0,31,7,9,1,19,7,8,7,25,6,6,7,
21,6,0,0,18,0,0,41,30,15,0,142,32,32,32,32,
32,32,32,32,105,116,101,109,115,32,61,32,91,108,101,102,
116,44,84,111,107,101,110,40,116,46,112,111,115,44,39,115,
108,105,99,101,39,44,39,58,39,44,105,116,101,109,115,91,
49,58,93,41,93,0,0,0,15,7,2,0,12,10,0,5,
84,111,107,101,110,0,0,0,13,9,10,0,12,14,0,3,
112,111,115,0,9,10,1,14,12,11,0,5,115,108,105,99,
101,0,0,0,12,12,0,1,58,0,0,0,11,15,0,0,
0,0,0,0,0,0,240,63,28,16,0,0,27,14,15,2,
9,13,4,14,31,8,10,4,19,8,9,8,27,6,7,2,
15,4,6,0,18,0,0,1,30,5,0,143,32,32,32,32,
114,46,105,116,101,109,115,32,61,32,105,116,101,109,115,0,
12,6,0,5,105,116,101,109,115,0,0,0,10,3,6,4,
30,5,0,144,32,32,32,32,97,100,118,97,110,99,101,40,
34,93,34,41,0,0,0,0,12,8,0,7,97,100,118,97,
110,99,101,0,13,7,8,0,12,8,0,1,93,0,0,0,
31,6,8,1,19,6,7,6,30,4,0,145,32,32,32,32,
114,101,116,117,114,110,32,114,0,0,0,0,20,3,0,0,
0,0,0,0,12,22,0,7,103,101,116,95,108,101,100,0,
14,22,21,0,30,6,0,146,100,101,102,32,100,111,116,95,
108,101,100,40,116,44,108,101,102,116,41,58,0,0,0,0,
16,22,0,76,44,8,0,0,30,6,0,146,100,101,102,32,
100,111,116,95,108,101,100,40,116,44,108,101,102,116,41,58,
0,0,0,0,12,1,0,8,112,97,114,115,101,46,112,121,
0,0,0,0,33,1,0,0,12,1,0,7,100,111,116,95,
108,101,100,0,34,1,0,0,28,2,0,0,9,1,0,2,
28,3,0,0,9,2,0,3,30,7,0,147,32,32,32,32,
114,32,61,32,101,120,112,114,101,115,115,105,111,110,40,116,
46,98,112,41,0,0,0,0,12,6,0,10,101,120,112,114,
101,115,115,105,111,110,0,0,13,5,6,0,12,7,0,2,
98,112,0,0,9,6,1,7,31,4,6,1,19,4,5,4,
15,3,4,0,30,6,0,148,32,32,32,32,114,46,116,121,
112,101,32,61,32,39,115,116,114,105,110,103,39,0,0,0,
12,4,0,6,115,116,114,105,110,103,0,0,12,5,0,4,
116,121,112,101,0,0,0,0,10,3,5,4,30,6,0,149,
32,32,32,32,116,46,105,116,101,109,115,32,61,32,91,108,
101,102,116,44,114,93,0,0,15,5,2,0,15,6,3,0,
27,4,5,2,12,5,0,5,105,116,101,109,115,0,0,0,
10,1,5,4,30,4,0,150,32,32,32,32,114,101,116,117,
114,110,32,116,0,0,0,0,20,1,0,0,0,0,0,0,
12,23,0,7,100,111,116,95,108,101,100,0,14,23,22,0,
30,4,0,152,100,101,102,32,105,116,115,101,108,102,40,116,
41,58,0,0,16,23,0,25,44,3,0,0,30,4,0,152,
100,101,102,32,105,116,115,101,108,102,40,116,41,58,0,0,
12,1,0,8,112,97,114,115,101,46,112,121,0,0,0,0,
33,1,0,0,12,1,0,6,105,116,115,101,108,102,0,0,
34,1,0,0,28,2,0,0,9,1,0,2,30,4,0,153,
32,32,32,32,114,101,116,117,114,110,32,116,0,0,0,0,
20,1,0,0,0,0,0,0,12,24,0,6,105,116,115,101,
108,102,0,0,14,24,23,0,30,5,0,154,100,101,102,32,
112,97,114,101,110,95,110,117,100,40,116,41,58,0,0,0,
16,24,0,87,44,6,0,0,30,5,0,154,100,101,102,32,
112,97,114,101,110,95,110,117,100,40,116,41,58,0,0,0,
12,1,0,8,112,97,114,115,101,46,112,121,0,0,0,0,
33,1,0,0,12,1,0,9,112,97,114,101,110,95,110,117,
100,0,0,0,34,1,0,0,28,2,0,0,9,1,0,2,
30,5,0,155,32,32,32,32,116,119,101,97,107,40,39,44,
39,44,49,41,0,0,0,0,12,4,0,5,116,119,101,97,
107,0,0,0,13,3,4,0,12,4,0,1,44,0,0,0,
11,5,0,0,0,0,0,0,0,0,240,63,31,2,4,2,
19,2,3,2,30,6,0,156,32,32,32,32,114,32,61,32,
101,120,112,114,101,115,115,105,111,110,40,48,41,0,0,0,
12,5,0,10,101,120,112,114,101,115,115,105,111,110,0,0,
13,4,5,0,11,5,0,0,0,0,0,0,0,0,0,0,
31,3,5,1,19,3,4,3,15,2,3,0,30,4,0,157,
32,32,32,32,114,101,115,116,111,114,101,40,41,0,0,0,
12,5,0,7,114,101,115,116,111,114,101,0,13,4,5,0,
31,3,0,0,19,3,4,3,30,5,0,158,32,32,32,32,
97,100,118,97,110,99,101,40,39,41,39,41,0,0,0,0,
12,5,0,7,97,100,118,97,110,99,101,0,13,4,5,0,
12,5,0,1,41,0,0,0,31,3,5,1,19,3,4,3,
30,4,0,159,32,32,32,32,114,101,116,117,114,110,32,114,
0,0,0,0,20,2,0,0,0,0,0,0,12,25,0,9,
112,97,114,101,110,95,110,117,100,0,0,0,14,25,24,0,
30,5,0,160,100,101,102,32,108,105,115,116,95,110,117,100,
40,116,41,58,0,0,0,0,16,25,1,131,44,10,0,0,
30,5,0,160,100,101,102,32,108,105,115,116,95,110,117,100,
40,116,41,58,0,0,0,0,12,1,0,8,112,97,114,115,
101,46,112,121,0,0,0,0,33,1,0,0,12,1,0,8,
108,105,115,116,95,110,117,100,0,0,0,0,34,1,0,0,
28,2,0,0,9,1,0,2,30,5,0,161,32,32,32,32,
116,46,116,121,112,101,32,61,32,39,108,105,115,116,39,0,
12,2,0,4,108,105,115,116,0,0,0,0,12,3,0,4,
116,121,112,101,0,0,0,0,10,1,3,2,30,5,0,162,
32,32,32,32,116,46,118,97,108,32,61,32,39,91,93,39,
0,0,0,0,12,2,0,2,91,93,0,0,12,3,0,3,
118,97,108,0,10,1,3,2,30,5,0,163,32,32,32,32,
116,46,105,116,101,109,115,32,61,32,91,93,0,0,0,0,
27,2,0,0,12,3,0,5,105,116,101,109,115,0,0,0,
10,1,3,2,30,5,0,164,32,32,32,32,110,101,120,116,
32,61,32,80,46,116,111,107,101,110,0,0,12,4,0,1,
80,0,0,0,13,3,4,0,12,4,0,5,116,111,107,101,
110,0,0,0,9,3,3,4,15,2,3,0,30,5,0,165,
32,32,32,32,116,119,101,97,107,40,39,44,39,44,48,41,
0,0,0,0,12,5,0,5,116,119,101,97,107,0,0,0,
13,4,5,0,12,5,0,1,44,0,0,0,11,6,0,0,
0,0,0,0,0,0,0,0,31,3,5,2,19,3,4,3,
30,10,0,166,32,32,32,32,119,104,105,108,101,32,110,111,
116,32,99,104,101,99,107,40,80,46,116,111,107,101,110,44,
39,102,111,114,39,44,39,93,39,41,58,0,12,6,0,5,
99,104,101,99,107,0,0,0,13,5,6,0,12,9,0,1,
80,0,0,0,13,6,9,0,12,9,0,5,116,111,107,101,
110,0,0,0,9,6,6,9,12,7,0,3,102,111,114,0,
12,8,0,1,93,0,0,0,31,4,6,3,19,4,5,4,
47,3,4,0,21,3,0,0,18,0,0,76,30,7,0,167,
32,32,32,32,32,32,32,32,114,32,61,32,101,120,112,114,
101,115,115,105,111,110,40,48,41,0,0,0,12,6,0,10,
101,120,112,114,101,115,115,105,111,110,0,0,13,5,6,0,
11,6,0,0,0,0,0,0,0,0,0,0,31,4,6,1,
19,4,5,4,15,3,4,0,30,7,0,168,32,32,32,32,
32,32,32,32,116,46,105,116,101,109,115,46,97,112,112,101,
110,100,40,114,41,0,0,0,12,6,0,5,105,116,101,109,
115,0,0,0,9,5,1,6,12,6,0,6,97,112,112,101,
110,100,0,0,9,5,5,6,15,6,3,0,31,4,6,1,
19,4,5,4,30,11,0,169,32,32,32,32,32,32,32,32,
105,102,32,80,46,116,111,107,101,110,46,118,97,108,32,61,
61,32,39,44,39,58,32,97,100,118,97,110,99,101,40,39,
44,39,41,0,12,5,0,1,80,0,0,0,13,4,5,0,
12,5,0,5,116,111,107,101,110,0,0,0,9,4,4,5,
12,5,0,3,118,97,108,0,9,4,4,5,12,5,0,1,
44,0,0,0,23,4,4,5,21,4,0,0,18,0,0,10,
12,6,0,7,97,100,118,97,110,99,101,0,13,5,6,0,
12,6,0,1,44,0,0,0,31,4,6,1,19,4,5,4,
18,0,0,1,18,0,255,162,30,8,0,170,32,32,32,32,
105,102,32,99,104,101,99,107,40,80,46,116,111,107,101,110,
44,39,102,111,114,39,41,58,0,0,0,0,12,6,0,5,
99,104,101,99,107,0,0,0,13,5,6,0,12,8,0,1,
80,0,0,0,13,6,8,0,12,8,0,5,116,111,107,101,
110,0,0,0,9,6,6,8,12,7,0,3,102,111,114,0,
31,4,6,2,19,4,5,4,21,4,0,0,18,0,0,138,
30,6,0,171,32,32,32,32,32,32,32,32,116,46,116,121,
112,101,32,61,32,39,99,111,109,112,39,0,12,4,0,4,
99,111,109,112,0,0,0,0,12,5,0,4,116,121,112,101,
0,0,0,0,10,1,5,4,30,6,0,172,32,32,32,32,
32,32,32,32,97,100,118,97,110,99,101,40,39,102,111,114,
39,41,0,0,12,6,0,7,97,100,118,97,110,99,101,0,
13,5,6,0,12,6,0,3,102,111,114,0,31,4,6,1,
19,4,5,4,30,6,0,173,32,32,32,32,32,32,32,32,
116,119,101,97,107,40,39,105,110,39,44,48,41,0,0,0,
12,6,0,5,116,119,101,97,107,0,0,0,13,5,6,0,
12,6,0,2,105,110,0,0,11,7,0,0,0,0,0,0,
0,0,0,0,31,4,6,2,19,4,5,4,30,10,0,174,
32,32,32,32,32,32,32,32,116,46,105,116,101,109,115,46,
97,112,112,101,110,100,40,101,120,112,114,101,115,115,105,111,
110,40,48,41,41,0,0,0,12,6,0,5,105,116,101,109,
115,0,0,0,9,5,1,6,12,6,0,6,97,112,112,101,
110,100,0,0,9,5,5,6,12,8,0,10,101,120,112,114,
101,115,115,105,111,110,0,0,13,7,8,0,11,8,0,0,
0,0,0,0,0,0,0,0,31,6,8,1,19,6,7,6,
31,4,6,1,19,4,5,4,30,6,0,175,32,32,32,32,
32,32,32,32,97,100,118,97,110,99,101,40,39,105,110,39,
41,0,0,0,12,6,0,7,97,100,118,97,110,99,101,0,
13,5,6,0,12,6,0,2,105,110,0,0,31,4,6,1,
19,4,5,4,30,10,0,176,32,32,32,32,32,32,32,32,
116,46,105,116,101,109,115,46,97,112,112,101,110,100,40,101,
120,112,114,101,115,115,105,111,110,40,48,41,41,0,0,0,
12,6,0,5,105,116,101,109,115,0,0,0,9,5,1,6,
12,6,0,6,97,112,112,101,110,100,0,0,9,5,5,6,
12,8,0,10,101,120,112,114,101,115,115,105,111,110,0,0,
13,7,8,0,11,8,0,0,0,0,0,0,0,0,0,0,
31,6,8,1,19,6,7,6,31,4,6,1,19,4,5,4,
30,5,0,177,32,32,32,32,32,32,32,32,114,101,115,116,
111,114,101,40,41,0,0,0,12,6,0,7,114,101,115,116,
111,114,101,0,13,5,6,0,31,4,0,0,19,4,5,4,
18,0,0,1,30,4,0,178,32,32,32,32,114,101,115,116,
111,114,101,40,41,0,0,0,12,6,0,7,114,101,115,116,
111,114,101,0,13,5,6,0,31,4,0,0,19,4,5,4,
30,5,0,179,32,32,32,32,97,100,118,97,110,99,101,40,
39,93,39,41,0,0,0,0,12,6,0,7,97,100,118,97,
110,99,101,0,13,5,6,0,12,6,0,1,93,0,0,0,
31,4,6,1,19,4,5,4,30,4,0,180,32,32,32,32,
114,101,116,117,114,110,32,116,0,0,0,0,20,1,0,0,
0,0,0,0,12,26,0,8,108,105,115,116,95,110,117,100,
0,0,0,0,14,26,25,0,30,5,0,181,100,101,102,32,
100,105,99,116,95,110,117,100,40,116,41,58,0,0,0,0,
16,26,0,203,44,8,0,0,30,5,0,181,100,101,102,32,
100,105,99,116,95,110,117,100,40,116,41,58,0,0,0,0,
12,1,0,8,112,97,114,115,101,46,112,121,0,0,0,0,
33,1,0,0,12,1,0,8,100,105,99,116,95,110,117,100,
0,0,0,0,34,1,0,0,28,2,0,0,9,1,0,2,
30,5,0,182,32,32,32,32,116,46,116,121,112,101,61,39,
100,105,99,116,39,0,0,0,12,2,0,4,100,105,99,116,
0,0,0,0,12,3,0,4,116,121,112,101,0,0,0,0,
10,1,3,2,30,5,0,183,32,32,32,32,116,46,118,97,
108,32,61,32,39,123,125,39,0,0,0,0,12,2,0,2,
123,125,0,0,12,3,0,3,118,97,108,0,10,1,3,2,
30,5,0,184,32,32,32,32,116,46,105,116,101,109,115,32,
61,32,91,93,0,0,0,0,27,2,0,0,12,3,0,5,
105,116,101,109,115,0,0,0,10,1,3,2,30,5,0,185,
32,32,32,32,116,119,101,97,107,40,39,44,39,44,48,41,
0,0,0,0,12,4,0,5,116,119,101,97,107,0,0,0,
13,3,4,0,12,4,0,1,44,0,0,0,11,5,0,0,
0,0,0,0,0,0,0,0,31,2,4,2,19,2,3,2,
30,9,0,186,32,32,32,32,119,104,105,108,101,32,110,111,
116,32,99,104,101,99,107,40,80,46,116,111,107,101,110,44,
39,125,39,41,58,0,0,0,12,5,0,5,99,104,101,99,
107,0,0,0,13,4,5,0,12,7,0,1,80,0,0,0,
13,5,7,0,12,7,0,5,116,111,107,101,110,0,0,0,
9,5,5,7,12,6,0,1,125,0,0,0,31,3,5,2,
19,3,4,3,47,2,3,0,21,2,0,0,18,0,0,72,
30,10,0,187,32,32,32,32,32,32,32,32,116,46,105,116,
101,109,115,46,97,112,112,101,110,100,40,101,120,112,114,101,
115,115,105,111,110,40,48,41,41,0,0,0,12,4,0,5,
105,116,101,109,115,0,0,0,9,3,1,4,12,4,0,6,
97,112,112,101,110,100,0,0,9,3,3,4,12,6,0,10,
101,120,112,114,101,115,115,105,111,110,0,0,13,5,6,0,
11,6,0,0,0,0,0,0,0,0,0,0,31,4,6,1,
19,4,5,4,31,2,4,1,19,2,3,2,30,12,0,188,
32,32,32,32,32,32,32,32,105,102,32,99,104,101,99,107,
40,80,46,116,111,107,101,110,44,39,58,39,44,39,44,39,
41,58,32,97,100,118,97,110,99,101,40,41,0,0,0,0,
12,4,0,5,99,104,101,99,107,0,0,0,13,3,4,0,
12,7,0,1,80,0,0,0,13,4,7,0,12,7,0,5,
116,111,107,101,110,0,0,0,9,4,4,7,12,5,0,1,
58,0,0,0,12,6,0,1,44,0,0,0,31,2,4,3,
19,2,3,2,21,2,0,0,18,0,0,8,12,4,0,7,
97,100,118,97,110,99,101,0,13,3,4,0,31,2,0,0,
19,2,3,2,18,0,0,1,18,0,255,168,30,4,0,189,
32,32,32,32,114,101,115,116,111,114,101,40,41,0,0,0,
12,4,0,7,114,101,115,116,111,114,101,0,13,3,4,0,
31,2,0,0,19,2,3,2,30,5,0,190,32,32,32,32,
97,100,118,97,110,99,101,40,39,125,39,41,0,0,0,0,
12,4,0,7,97,100,118,97,110,99,101,0,13,3,4,0,
12,4,0,1,125,0,0,0,31,2,4,1,19,2,3,2,
30,4,0,191,32,32,32,32,114,101,116,117,114,110,32,116,
0,0,0,0,20,1,0,0,0,0,0,0,12,27,0,8,
100,105,99,116,95,110,117,100,0,0,0,0,14,27,26,0,
30,6,0,193,100,101,102,32,97,100,118,97,110,99,101,40,
116,61,78,111,110,101,41,58,0,0,0,0,16,27,0,40,
44,5,0,0,30,6,0,193,100,101,102,32,97,100,118,97,
110,99,101,40,116,61,78,111,110,101,41,58,0,0,0,0,
12,1,0,8,112,97,114,115,101,46,112,121,0,0,0,0,
33,1,0,0,12,1,0,7,97,100,118,97,110,99,101,0,
34,1,0,0,28,1,0,0,28,2,0,0,32,1,0,2,
30,6,0,194,32,32,32,32,114,101,116,117,114,110,32,80,
46,97,100,118,97,110,99,101,40,116,41,0,12,4,0,1,
80,0,0,0,13,3,4,0,12,4,0,7,97,100,118,97,
110,99,101,0,9,3,3,4,15,4,1,0,31,2,4,1,
19,2,3,2,20,2,0,0,0,0,0,0,12,28,0,7,
97,100,118,97,110,99,101,0,14,28,27,0,30,5,0,196,
100,101,102,32,105,98,108,111,99,107,40,105,116,101,109,115,
41,58,0,0,16,28,0,188,44,8,0,0,30,5,0,196,
100,101,102,32,105,98,108,111,99,107,40,105,116,101,109,115,
41,58,0,0,12,1,0,8,112,97,114,115,101,46,112,121,
0,0,0,0,33,1,0,0,12,1,0,6,105,98,108,111,
99,107,0,0,34,1,0,0,28,2,0,0,9,1,0,2,
30,12,0,197,32,32,32,32,119,104,105,108,101,32,99,104,
101,99,107,40,80,46,116,111,107,101,110,44,39,110,108,39,
44,39,59,39,41,58,32,97,100,118,97,110,99,101,40,41,
0,0,0,0,12,4,0,5,99,104,101,99,107,0,0,0,
13,3,4,0,12,7,0,1,80,0,0,0,13,4,7,0,
12,7,0,5,116,111,107,101,110,0,0,0,9,4,4,7,
12,5,0,2,110,108,0,0,12,6,0,1,59,0,0,0,
31,2,4,3,19,2,3,2,21,2,0,0,18,0,0,8,
12,4,0,7,97,100,118,97,110,99,101,0,13,3,4,0,
31,2,0,0,19,2,3,2,18,0,255,231,30,4,0,198,
32,32,32,32,119,104,105,108,101,32,84,114,117,101,58,0,
11,2,0,0,0,0,0,0,0,0,240,63,21,2,0,0,
18,0,0,120,30,9,0,199,32,32,32,32,32,32,32,32,
105,116,101,109,115,46,97,112,112,101,110,100,40,101,120,112,
114,101,115,115,105,111,110,40,48,41,41,0,12,4,0,6,
97,112,112,101,110,100,0,0,9,3,1,4,12,6,0,10,
101,120,112,114,101,115,115,105,111,110,0,0,13,5,6,0,
11,6,0,0,0,0,0,0,0,0,0,0,31,4,6,1,
19,4,5,4,31,2,4,1,19,2,3,2,30,6,0,200,
32,32,32,32,32,32,32,32,80,46,116,101,114,109,105,110,
97,108,40,41,0,0,0,0,12,4,0,1,80,0,0,0,
13,3,4,0,12,4,0,8,116,101,114,109,105,110,97,108,
0,0,0,0,9,3,3,4,31,2,0,0,19,2,3,2,
30,13,0,201,32,32,32,32,32,32,32,32,119,104,105,108,
101,32,99,104,101,99,107,40,80,46,116,111,107,101,110,44,
39,110,108,39,44,39,59,39,41,58,32,97,100,118,97,110,
99,101,40,41,0,0,0,0,12,4,0,5,99,104,101,99,
107,0,0,0,13,3,4,0,12,7,0,1,80,0,0,0,
13,4,7,0,12,7,0,5,116,111,107,101,110,0,0,0,
9,4,4,7,12,5,0,2,110,108,0,0,12,6,0,1,
59,0,0,0,31,2,4,3,19,2,3,2,21,2,0,0,
18,0,0,8,12,4,0,7,97,100,118,97,110,99,101,0,
13,3,4,0,31,2,0,0,19,2,3,2,18,0,255,231,
30,12,0,202,32,32,32,32,32,32,32,32,105,102,32,99,
104,101,99,107,40,80,46,116,111,107,101,110,44,39,100,101,
100,101,110,116,39,44,39,101,111,102,39,41,58,32,98,114,
101,97,107,0,12,4,0,5,99,104,101,99,107,0,0,0,
13,3,4,0,12,7,0,1,80,0,0,0,13,4,7,0,
12,7,0,5,116,111,107,101,110,0,0,0,9,4,4,7,
12,5,0,6,100,101,100,101,110,116,0,0,12,6,0,3,
101,111,102,0,31,2,4,3,19,2,3,2,21,2,0,0,
18,0,0,3,18,0,0,3,18,0,0,1,18,0,255,133,
0,0,0,0,12,29,0,6,105,98,108,111,99,107,0,0,
14,29,28,0,30,4,0,204,100,101,102,32,98,108,111,99,
107,40,41,58,0,0,0,0,16,29,1,113,44,10,0,0,
30,4,0,204,100,101,102,32,98,108,111,99,107,40,41,58,
0,0,0,0,12,1,0,8,112,97,114,115,101,46,112,121,
0,0,0,0,33,1,0,0,12,1,0,5,98,108,111,99,
107,0,0,0,34,1,0,0,30,4,0,205,32,32,32,32,
105,116,101,109,115,32,61,32,91,93,0,0,27,2,0,0,
15,1,2,0,30,5,0,206,32,32,32,32,116,111,107,32,
61,32,80,46,116,111,107,101,110,0,0,0,12,4,0,1,
80,0,0,0,13,3,4,0,12,4,0,5,116,111,107,101,
110,0,0,0,9,3,3,4,15,2,3,0,30,7,0,208,
32,32,32,32,105,102,32,99,104,101,99,107,40,80,46,116,
111,107,101,110,44,39,110,108,39,41,58,0,12,5,0,5,
99,104,101,99,107,0,0,0,13,4,5,0,12,7,0,1,
80,0,0,0,13,5,7,0,12,7,0,5,116,111,107,101,
110,0,0,0,9,5,5,7,12,6,0,2,110,108,0,0,
31,3,5,2,19,3,4,3,21,3,0,0,18,0,0,87,
30,12,0,209,32,32,32,32,32,32,32,32,119,104,105,108,
101,32,99,104,101,99,107,40,80,46,116,111,107,101,110,44,
39,110,108,39,41,58,32,97,100,118,97,110,99,101,40,41,
0,0,0,0,12,5,0,5,99,104,101,99,107,0,0,0,
13,4,5,0,12,7,0,1,80,0,0,0,13,5,7,0,
12,7,0,5,116,111,107,101,110,0,0,0,9,5,5,7,
12,6,0,2,110,108,0,0,31,3,5,2,19,3,4,3,
21,3,0,0,18,0,0,8,12,5,0,7,97,100,118,97,
110,99,101,0,13,4,5,0,31,3,0,0,19,3,4,3,
18,0,255,233,30,7,0,210,32,32,32,32,32,32,32,32,
97,100,118,97,110,99,101,40,39,105,110,100,101,110,116,39,
41,0,0,0,12,5,0,7,97,100,118,97,110,99,101,0,
13,4,5,0,12,5,0,6,105,110,100,101,110,116,0,0,
31,3,5,1,19,3,4,3,30,6,0,211,32,32,32,32,
32,32,32,32,105,98,108,111,99,107,40,105,116,101,109,115,
41,0,0,0,12,5,0,6,105,98,108,111,99,107,0,0,
13,4,5,0,15,5,1,0,31,3,5,1,19,3,4,3,
30,7,0,212,32,32,32,32,32,32,32,32,97,100,118,97,
110,99,101,40,39,100,101,100,101,110,116,39,41,0,0,0,
12,5,0,7,97,100,118,97,110,99,101,0,13,4,5,0,
12,5,0,6,100,101,100,101,110,116,0,0,31,3,5,1,
19,3,4,3,18,0,0,120,30,3,0,213,32,32,32,32,
101,108,115,101,58,0,0,0,30,9,0,214,32,32,32,32,
32,32,32,32,105,116,101,109,115,46,97,112,112,101,110,100,
40,101,120,112,114,101,115,115,105,111,110,40,48,41,41,0,
12,5,0,6,97,112,112,101,110,100,0,0,9,4,1,5,
12,7,0,10,101,120,112,114,101,115,115,105,111,110,0,0,
13,6,7,0,11,7,0,0,0,0,0,0,0,0,0,0,
31,5,7,1,19,5,6,5,31,3,5,1,19,3,4,3,
30,9,0,215,32,32,32,32,32,32,32,32,119,104,105,108,
101,32,99,104,101,99,107,40,80,46,116,111,107,101,110,44,
39,59,39,41,58,0,0,0,12,5,0,5,99,104,101,99,
107,0,0,0,13,4,5,0,12,7,0,1,80,0,0,0,
13,5,7,0,12,7,0,5,116,111,107,101,110,0,0,0,
9,5,5,7,12,6,0,1,59,0,0,0,31,3,5,2,
19,3,4,3,21,3,0,0,18,0,0,45,30,7,0,216,
32,32,32,32,32,32,32,32,32,32,32,32,97,100,118,97,
110,99,101,40,39,59,39,41,0,0,0,0,12,5,0,7,
97,100,118,97,110,99,101,0,13,4,5,0,12,5,0,1,
59,0,0,0,31,3,5,1,19,3,4,3,30,10,0,217,
32,32,32,32,32,32,32,32,32,32,32,32,105,116,101,109,
115,46,97,112,112,101,110,100,40,101,120,112,114,101,115,115,
105,111,110,40,48,41,41,0,12,5,0,6,97,112,112,101,
110,100,0,0,9,4,1,5,12,7,0,10,101,120,112,114,
101,115,115,105,111,110,0,0,13,6,7,0,11,7,0,0,
0,0,0,0,0,0,0,0,31,5,7,1,19,5,6,5,
31,3,5,1,19,3,4,3,18,0,255,196,30,6,0,218,
32,32,32,32,32,32,32,32,80,46,116,101,114,109,105,110,
97,108,40,41,0,0,0,0,12,5,0,1,80,0,0,0,
13,4,5,0,12,5,0,8,116,101,114,109,105,110,97,108,
0,0,0,0,9,4,4,5,31,3,0,0,19,3,4,3,
18,0,0,1,30,11,0,219,32,32,32,32,119,104,105,108,
101,32,99,104,101,99,107,40,80,46,116,111,107,101,110,44,
39,110,108,39,41,58,32,97,100,118,97,110,99,101,40,41,
0,0,0,0,12,5,0,5,99,104,101,99,107,0,0,0,
13,4,5,0,12,7,0,1,80,0,0,0,13,5,7,0,
12,7,0,5,116,111,107,101,110,0,0,0,9,5,5,7,
12,6,0,2,110,108,0,0,31,3,5,2,19,3,4,3,
21,3,0,0,18,0,0,8,12,5,0,7,97,100,118,97,
110,99,101,0,13,4,5,0,31,3,0,0,19,3,4,3,
18,0,255,233,30,6,0,221,32,32,32,32,105,102,32,108,
101,110,40,105,116,101,109,115,41,32,62,32,49,58,0,0,
11,3,0,0,0,0,0,0,0,0,240,63,12,6,0,3,
108,101,110,0,13,5,6,0,15,6,1,0,31,4,6,1,
19,4,5,4,25,3,3,4,21,3,0,0,18,0,0,34,
30,14,0,222,32,32,32,32,32,32,32,32,114,101,116,117,
114,110,32,84,111,107,101,110,40,116,111,107,46,112,111,115,
44,39,115,116,97,116,101,109,101,110,116,115,39,44,39,59,
39,44,105,116,101,109,115,41,0,0,0,0,12,5,0,5,
84,111,107,101,110,0,0,0,13,4,5,0,12,9,0,3,
112,111,115,0,9,5,2,9,12,6,0,10,115,116,97,116,
101,109,101,110,116,115,0,0,12,7,0,1,59,0,0,0,
15,8,1,0,31,3,5,4,19,3,4,3,20,3,0,0,
18,0,0,1,30,6,0,223,32,32,32,32,114,101,116,117,
114,110,32,105,116,101,109,115,46,112,111,112,40,41,0,0,
12,5,0,3,112,111,112,0,9,4,1,5,31,3,0,0,
19,3,4,3,20,3,0,0,0,0,0,0,12,30,0,5,
98,108,111,99,107,0,0,0,14,30,29,0,30,4,0,225,
100,101,102,32,100,101,102,95,110,117,100,40,116,41,58,0,
16,30,1,43,44,11,0,0,30,4,0,225,100,101,102,32,
100,101,102,95,110,117,100,40,116,41,58,0,12,1,0,8,
112,97,114,115,101,46,112,121,0,0,0,0,33,1,0,0,
12,1,0,7,100,101,102,95,110,117,100,0,34,1,0,0,
28,2,0,0,9,1,0,2,30,7,0,226,32,32,32,32,
105,116,101,109,115,32,61,32,116,46,105,116,101,109,115,32,
61,32,91,93,0,0,0,0,27,3,0,0,12,4,0,5,
105,116,101,109,115,0,0,0,10,1,4,3,15,2,3,0,
30,10,0,227,32,32,32,32,105,116,101,109,115,46,97,112,
112,101,110,100,40,80,46,116,111,107,101,110,41,59,32,97,
100,118,97,110,99,101,40,41,0,0,0,0,12,5,0,6,
97,112,112,101,110,100,0,0,9,4,2,5,12,6,0,1,
80,0,0,0,13,5,6,0,12,6,0,5,116,111,107,101,
110,0,0,0,9,5,5,6,31,3,5,1,19,3,4,3,
12,5,0,7,97,100,118,97,110,99,101,0,13,4,5,0,
31,3,0,0,19,3,4,3,30,5,0,228,32,32,32,32,
97,100,118,97,110,99,101,40,39,40,39,41,0,0,0,0,
12,5,0,7,97,100,118,97,110,99,101,0,13,4,5,0,
12,5,0,1,40,0,0,0,31,3,5,1,19,3,4,3,
30,10,0,229,32,32,32,32,114,32,61,32,84,111,107,101,
110,40,116,46,112,111,115,44,39,115,121,109,98,111,108,39,
44,39,40,41,58,39,44,91,93,41,0,0,12,6,0,5,
84,111,107,101,110,0,0,0,13,5,6,0,12,10,0,3,
112,111,115,0,9,6,1,10,12,7,0,6,115,121,109,98,
111,108,0,0,12,8,0,3,40,41,58,0,27,9,0,0,
31,4,6,4,19,4,5,4,15,3,4,0,30,5,0,230,
32,32,32,32,105,116,101,109,115,46,97,112,112,101,110,100,
40,114,41,0,12,6,0,6,97,112,112,101,110,100,0,0,
9,5,2,6,15,6,3,0,31,4,6,1,19,4,5,4,
30,9,0,231,32,32,32,32,119,104,105,108,101,32,110,111,
116,32,99,104,101,99,107,40,80,46,116,111,107,101,110,44,
39,41,39,41,58,0,0,0,12,7,0,5,99,104,101,99,
107,0,0,0,13,6,7,0,12,9,0,1,80,0,0,0,
13,7,9,0,12,9,0,5,116,111,107,101,110,0,0,0,
9,7,7,9,12,8,0,1,41,0,0,0,31,5,7,2,
19,5,6,5,47,4,5,0,21,4,0,0,18,0,0,101,
30,6,0,232,32,32,32,32,32,32,32,32,116,119,101,97,
107,40,39,44,39,44,48,41,0,0,0,0,12,6,0,5,
116,119,101,97,107,0,0,0,13,5,6,0,12,6,0,1,
44,0,0,0,11,7,0,0,0,0,0,0,0,0,0,0,
31,4,6,2,19,4,5,4,30,10,0,233,32,32,32,32,
32,32,32,32,114,46,105,116,101,109,115,46,97,112,112,101,
110,100,40,101,120,112,114,101,115,115,105,111,110,40,48,41,
41,0,0,0,12,6,0,5,105,116,101,109,115,0,0,0,
9,5,3,6,12,6,0,6,97,112,112,101,110,100,0,0,
9,5,5,6,12,8,0,10,101,120,112,114,101,115,115,105,
111,110,0,0,13,7,8,0,11,8,0,0,0,0,0,0,
0,0,0,0,31,6,8,1,19,6,7,6,31,4,6,1,
19,4,5,4,30,11,0,234,32,32,32,32,32,32,32,32,
105,102,32,99,104,101,99,107,40,80,46,116,111,107,101,110,
44,39,44,39,41,58,32,97,100,118,97,110,99,101,40,39,
44,39,41,0,12,6,0,5,99,104,101,99,107,0,0,0,
13,5,6,0,12,8,0,1,80,0,0,0,13,6,8,0,
12,8,0,5,116,111,107,101,110,0,0,0,9,6,6,8,
12,7,0,1,44,0,0,0,31,4,6,2,19,4,5,4,
21,4,0,0,18,0,0,10,12,6,0,7,97,100,118,97,
110,99,101,0,13,5,6,0,12,6,0,1,44,0,0,0,
31,4,6,1,19,4,5,4,18,0,0,1,30,5,0,235,
32,32,32,32,32,32,32,32,114,101,115,116,111,114,101,40,
41,0,0,0,12,6,0,7,114,101,115,116,111,114,101,0,
13,5,6,0,31,4,0,0,19,4,5,4,18,0,255,139,
30,5,0,236,32,32,32,32,97,100,118,97,110,99,101,40,
39,41,39,41,0,0,0,0,12,6,0,7,97,100,118,97,
110,99,101,0,13,5,6,0,12,6,0,1,41,0,0,0,
31,4,6,1,19,4,5,4,30,5,0,237,32,32,32,32,
97,100,118,97,110,99,101,40,39,58,39,41,0,0,0,0,
12,6,0,7,97,100,118,97,110,99,101,0,13,5,6,0,
12,6,0,1,58,0,0,0,31,4,6,1,19,4,5,4,
30,7,0,238,32,32,32,32,105,116,101,109,115,46,97,112,
112,101,110,100,40,98,108,111,99,107,40,41,41,0,0,0,
12,6,0,6,97,112,112,101,110,100,0,0,9,5,2,6,
12,8,0,5,98,108,111,99,107,0,0,0,13,7,8,0,
31,6,0,0,19,6,7,6,31,4,6,1,19,4,5,4,
30,4,0,239,32,32,32,32,114,101,116,117,114,110,32,116,
0,0,0,0,20,1,0,0,0,0,0,0,12,31,0,7,
100,101,102,95,110,117,100,0,14,31,30,0,30,5,0,242,
100,101,102,32,119,104,105,108,101,95,110,117,100,40,116,41,
58,0,0,0,16,31,0,100,44,8,0,0,30,5,0,242,
100,101,102,32,119,104,105,108,101,95,110,117,100,40,116,41,
58,0,0,0,12,1,0,8,112,97,114,115,101,46,112,121,
0,0,0,0,33,1,0,0,12,1,0,9,119,104,105,108,
101,95,110,117,100,0,0,0,34,1,0,0,28,2,0,0,
9,1,0,2,30,7,0,243,32,32,32,32,105,116,101,109,
115,32,61,32,116,46,105,116,101,109,115,32,61,32,91,93,
0,0,0,0,27,3,0,0,12,4,0,5,105,116,101,109,
115,0,0,0,10,1,4,3,15,2,3,0,30,8,0,244,
32,32,32,32,105,116,101,109,115,46,97,112,112,101,110,100,
40,101,120,112,114,101,115,115,105,111,110,40,48,41,41,0,
12,5,0,6,97,112,112,101,110,100,0,0,9,4,2,5,
12,7,0,10,101,120,112,114,101,115,115,105,111,110,0,0,
13,6,7,0,11,7,0,0,0,0,0,0,0,0,0,0,
31,5,7,1,19,5,6,5,31,3,5,1,19,3,4,3,
30,5,0,245,32,32,32,32,97,100,118,97,110,99,101,40,
39,58,39,41,0,0,0,0,12,5,0,7,97,100,118,97,
110,99,101,0,13,4,5,0,12,5,0,1,58,0,0,0,
31,3,5,1,19,3,4,3,30,7,0,246,32,32,32,32,
105,116,101,109,115,46,97,112,112,101,110,100,40,98,108,111,
99,107,40,41,41,0,0,0,12,5,0,6,97,112,112,101,
110,100,0,0,9,4,2,5,12,7,0,5,98,108,111,99,
107,0,0,0,13,6,7,0,31,5,0,0,19,5,6,5,
31,3,5,1,19,3,4,3,30,4,0,247,32,32,32,32,
114,101,116,117,114,110,32,116,0,0,0,0,20,1,0,0,
0,0,0,0,12,32,0,9,119,104,105,108,101,95,110,117,
100,0,0,0,14,32,31,0,30,5,0,248,100,101,102,32,
99,108,97,115,115,95,110,117,100,40,116,41,58,0,0,0,
16,32,0,113,44,11,0,0,30,5,0,248,100,101,102,32,
99,108,97,115,115,95,110,117,100,40,116,41,58,0,0,0,
12,1,0,8,112,97,114,115,101,46,112,121,0,0,0,0,
33,1,0,0,12,1,0,9,99,108,97,115,115,95,110,117,
100,0,0,0,34,1,0,0,28,2,0,0,9,1,0,2,
30,7,0,249,32,32,32,32,105,116,101,109,115,32,61,32,
116,46,105,116,101,109,115,32,61,32,91,93,0,0,0,0,
27,3,0,0,12,4,0,5,105,116,101,109,115,0,0,0,
10,1,4,3,15,2,3,0,30,8,0,250,32,32,32,32,
105,116,101,109,115,46,97,112,112,101,110,100,40,101,120,112,
114,101,115,115,105,111,110,40,48,41,41,0,12,5,0,6,
97,112,112,101,110,100,0,0,9,4,2,5,12,7,0,10,
101,120,112,114,101,115,115,105,111,110,0,0,13,6,7,0,
11,7,0,0,0,0,0,0,0,0,0,0,31,5,7,1,
19,5,6,5,31,3,5,1,19,3,4,3,30,5,0,251,
32,32,32,32,97,100,118,97,110,99,101,40,39,58,39,41,
0,0,0,0,12,5,0,7,97,100,118,97,110,99,101,0,
13,4,5,0,12,5,0,1,58,0,0,0,31,3,5,1,
19,3,4,3,30,11,0,252,32,32,32,32,105,116,101,109,
115,46,97,112,112,101,110,100,40,105,108,115,116,40,39,109,
101,116,104,111,100,115,39,44,98,108,111,99,107,40,41,41,
41,0,0,0,12,5,0,6,97,112,112,101,110,100,0,0,
9,4,2,5,12,7,0,4,105,108,115,116,0,0,0,0,
13,6,7,0,12,7,0,7,109,101,116,104,111,100,115,0,
12,10,0,5,98,108,111,99,107,0,0,0,13,9,10,0,
31,8,0,0,19,8,9,8,31,5,7,2,19,5,6,5,
31,3,5,1,19,3,4,3,30,4,0,253,32,32,32,32,
114,101,116,117,114,110,32,116,0,0,0,0,20,1,0,0,
0,0,0,0,12,33,0,9,99,108,97,115,115,95,110,117,
100,0,0,0,14,33,32,0,30,5,0,255,100,101,102,32,
102,114,111,109,95,110,117,100,40,116,41,58,0,0,0,0,
16,33,0,107,44,8,0,0,30,5,0,255,100,101,102,32,
102,114,111,109,95,110,117,100,40,116,41,58,0,0,0,0,
12,1,0,8,112,97,114,115,101,46,112,121,0,0,0,0,
33,1,0,0,12,1,0,8,102,114,111,109,95,110,117,100,
0,0,0,0,34,1,0,0,28,2,0,0,9,1,0,2,
30,7,1,0,32,32,32,32,105,116,101,109,115,32,61,32,
116,46,105,116,101,109,115,32,61,32,91,93,0,0,0,0,
27,3,0,0,12,4,0,5,105,116,101,109,115,0,0,0,
10,1,4,3,15,2,3,0,30,8,1,1,32,32,32,32,
105,116,101,109,115,46,97,112,112,101,110,100,40,101,120,112,
114,101,115,115,105,111,110,40,48,41,41,0,12,5,0,6,
97,112,112,101,110,100,0,0,9,4,2,5,12,7,0,10,
101,120,112,114,101,115,115,105,111,110,0,0,13,6,7,0,
11,7,0,0,0,0,0,0,0,0,0,0,31,5,7,1,
19,5,6,5,31,3,5,1,19,3,4,3,30,6,1,2,
32,32,32,32,97,100,118,97,110,99,101,40,39,105,109,112,
111,114,116,39,41,0,0,0,12,5,0,7,97,100,118,97,
110,99,101,0,13,4,5,0,12,5,0,6,105,109,112,111,
114,116,0,0,31,3,5,1,19,3,4,3,30,8,1,3,
32,32,32,32,105,116,101,109,115,46,97,112,112,101,110,100,
40,101,120,112,114,101,115,115,105,111,110,40,48,41,41,0,
12,5,0,6,97,112,112,101,110,100,0,0,9,4,2,5,
12,7,0,10,101,120,112,114,101,115,115,105,111,110,0,0,
13,6,7,0,11,7,0,0,0,0,0,0,0,0,0,0,
31,5,7,1,19,5,6,5,31,3,5,1,19,3,4,3,
30,4,1,4,32,32,32,32,114,101,116,117,114,110,32,116,
0,0,0,0,20,1,0,0,0,0,0,0,12,34,0,8,
102,114,111,109,95,110,117,100,0,0,0,0,14,34,33,0,
30,4,1,6,100,101,102,32,102,111,114,95,110,117,100,40,
116,41,58,0,16,34,0,165,44,8,0,0,30,4,1,6,
100,101,102,32,102,111,114,95,110,117,100,40,116,41,58,0,
12,1,0,8,112,97,114,115,101,46,112,121,0,0,0,0,
33,1,0,0,12,1,0,7,102,111,114,95,110,117,100,0,
34,1,0,0,28,2,0,0,9,1,0,2,30,7,1,7,
32,32,32,32,105,116,101,109,115,32,61,32,116,46,105,116,
101,109,115,32,61,32,91,93,0,0,0,0,27,3,0,0,
12,4,0,5,105,116,101,109,115,0,0,0,10,1,4,3,
15,2,3,0,30,5,1,8,32,32,32,32,116,119,101,97,
107,40,39,105,110,39,44,48,41,0,0,0,12,5,0,5,
116,119,101,97,107,0,0,0,13,4,5,0,12,5,0,2,
105,110,0,0,11,6,0,0,0,0,0,0,0,0,0,0,
31,3,5,2,19,3,4,3,30,8,1,9,32,32,32,32,
105,116,101,109,115,46,97,112,112,101,110,100,40,101,120,112,
114,101,115,115,105,111,110,40,48,41,41,0,12,5,0,6,
97,112,112,101,110,100,0,0,9,4,2,5,12,7,0,10,
101,120,112,114,101,115,115,105,111,110,0,0,13,6,7,0,
11,7,0,0,0,0,0,0,0,0,0,0,31,5,7,1,
19,5,6,5,31,3,5,1,19,3,4,3,30,5,1,10,
32,32,32,32,97,100,118,97,110,99,101,40,39,105,110,39,
41,0,0,0,12,5,0,7,97,100,118,97,110,99,101,0,
13,4,5,0,12,5,0,2,105,110,0,0,31,3,5,1,
19,3,4,3,30,8,1,11,32,32,32,32,105,116,101,109,
115,46,97,112,112,101,110,100,40,101,120,112,114,101,115,115,
105,111,110,40,48,41,41,0,12,5,0,6,97,112,112,101,
110,100,0,0,9,4,2,5,12,7,0,10,101,120,112,114,
101,115,115,105,111,110,0,0,13,6,7,0,11,7,0,0,
0,0,0,0,0,0,0,0,31,5,7,1,19,5,6,5,
31,3,5,1,19,3,4,3,30,4,1,12,32,32,32,32,
114,101,115,116,111,114,101,40,41,0,0,0,12,5,0,7,
114,101,115,116,111,114,101,0,13,4,5,0,31,3,0,0,
19,3,4,3,30,5,1,13,32,32,32,32,97,100,118,97,
110,99,101,40,39,58,39,41,0,0,0,0,12,5,0,7,
97,100,118,97,110,99,101,0,13,4,5,0,12,5,0,1,
58,0,0,0,31,3,5,1,19,3,4,3,30,7,1,14,
32,32,32,32,105,116,101,109,115,46,97,112,112,101,110,100,
40,98,108,111,99,107,40,41,41,0,0,0,12,5,0,6,
97,112,112,101,110,100,0,0,9,4,2,5,12,7,0,5,
98,108,111,99,107,0,0,0,13,6,7,0,31,5,0,0,
19,5,6,5,31,3,5,1,19,3,4,3,30,4,1,15,
32,32,32,32,114,101,116,117,114,110,32,116,0,0,0,0,
20,1,0,0,0,0,0,0,12,35,0,7,102,111,114,95,
110,117,100,0,14,35,34,0,30,4,1,16,100,101,102,32,
105,102,95,110,117,100,40,116,41,58,0,0,16,35,1,137,
44,16,0,0,30,4,1,16,100,101,102,32,105,102,95,110,
117,100,40,116,41,58,0,0,12,1,0,8,112,97,114,115,
101,46,112,121,0,0,0,0,33,1,0,0,12,1,0,6,
105,102,95,110,117,100,0,0,34,1,0,0,28,2,0,0,
9,1,0,2,30,7,1,17,32,32,32,32,105,116,101,109,
115,32,61,32,116,46,105,116,101,109,115,32,61,32,91,93,
0,0,0,0,27,3,0,0,12,4,0,5,105,116,101,109,
115,0,0,0,10,1,4,3,15,2,3,0,30,6,1,18,
32,32,32,32,97,32,61,32,101,120,112,114,101,115,115,105,
111,110,40,48,41,0,0,0,12,6,0,10,101,120,112,114,
101,115,115,105,111,110,0,0,13,5,6,0,11,6,0,0,
0,0,0,0,0,0,0,0,31,4,6,1,19,4,5,4,
15,3,4,0,30,5,1,19,32,32,32,32,97,100,118,97,
110,99,101,40,39,58,39,41,0,0,0,0,12,6,0,7,
97,100,118,97,110,99,101,0,13,5,6,0,12,6,0,1,
58,0,0,0,31,4,6,1,19,4,5,4,30,4,1,20,
32,32,32,32,98,32,61,32,98,108,111,99,107,40,41,0,
12,7,0,5,98,108,111,99,107,0,0,0,13,6,7,0,
31,5,0,0,19,5,6,5,15,4,5,0,30,13,1,21,
32,32,32,32,105,116,101,109,115,46,97,112,112,101,110,100,
40,84,111,107,101,110,40,116,46,112,111,115,44,39,101,108,
105,102,39,44,39,101,108,105,102,39,44,91,97,44,98,93,
41,41,0,0,12,7,0,6,97,112,112,101,110,100,0,0,
9,6,2,7,12,9,0,5,84,111,107,101,110,0,0,0,
13,8,9,0,12,13,0,3,112,111,115,0,9,9,1,13,
12,10,0,4,101,108,105,102,0,0,0,0,12,11,0,4,
101,108,105,102,0,0,0,0,15,13,3,0,15,14,4,0,
27,12,13,2,31,7,9,4,19,7,8,7,31,5,7,1,
19,5,6,5,30,9,1,22,32,32,32,32,119,104,105,108,
101,32,99,104,101,99,107,40,80,46,116,111,107,101,110,44,
39,101,108,105,102,39,41,58,0,0,0,0,12,7,0,5,
99,104,101,99,107,0,0,0,13,6,7,0,12,9,0,1,
80,0,0,0,13,7,9,0,12,9,0,5,116,111,107,101,
110,0,0,0,9,7,7,9,12,8,0,4,101,108,105,102,
0,0,0,0,31,5,7,2,19,5,6,5,21,5,0,0,
18,0,0,120,30,6,1,23,32,32,32,32,32,32,32,32,
116,111,107,32,61,32,80,46,116,111,107,101,110,0,0,0,
12,7,0,1,80,0,0,0,13,6,7,0,12,7,0,5,
116,111,107,101,110,0,0,0,9,6,6,7,15,5,6,0,
30,6,1,24,32,32,32,32,32,32,32,32,97,100,118,97,
110,99,101,40,39,101,108,105,102,39,41,0,12,8,0,7,
97,100,118,97,110,99,101,0,13,7,8,0,12,8,0,4,
101,108,105,102,0,0,0,0,31,6,8,1,19,6,7,6,
30,7,1,25,32,32,32,32,32,32,32,32,97,32,61,32,
101,120,112,114,101,115,115,105,111,110,40,48,41,0,0,0,
12,8,0,10,101,120,112,114,101,115,115,105,111,110,0,0,
13,7,8,0,11,8,0,0,0,0,0,0,0,0,0,0,
31,6,8,1,19,6,7,6,15,3,6,0,30,6,1,26,
32,32,32,32,32,32,32,32,97,100,118,97,110,99,101,40,
39,58,39,41,0,0,0,0,12,8,0,7,97,100,118,97,
110,99,101,0,13,7,8,0,12,8,0,1,58,0,0,0,
31,6,8,1,19,6,7,6,30,5,1,27,32,32,32,32,
32,32,32,32,98,32,61,32,98,108,111,99,107,40,41,0,
12,8,0,5,98,108,111,99,107,0,0,0,13,7,8,0,
31,6,0,0,19,6,7,6,15,4,6,0,30,15,1,28,
32,32,32,32,32,32,32,32,105,116,101,109,115,46,97,112,
112,101,110,100,40,84,111,107,101,110,40,116,111,107,46,112,
111,115,44,39,101,108,105,102,39,44,39,101,108,105,102,39,
44,91,97,44,98,93,41,41,0,0,0,0,12,8,0,6,
97,112,112,101,110,100,0,0,9,7,2,8,12,10,0,5,
84,111,107,101,110,0,0,0,13,9,10,0,12,14,0,3,
112,111,115,0,9,10,5,14,12,11,0,4,101,108,105,102,
0,0,0,0,12,12,0,4,101,108,105,102,0,0,0,0,
15,14,3,0,15,15,4,0,27,13,14,2,31,8,10,4,
19,8,9,8,31,6,8,1,19,6,7,6,18,0,255,120,
30,8,1,29,32,32,32,32,105,102,32,99,104,101,99,107,
40,80,46,116,111,107,101,110,44,39,101,108,115,101,39,41,
58,0,0,0,12,8,0,5,99,104,101,99,107,0,0,0,
13,7,8,0,12,10,0,1,80,0,0,0,13,8,10,0,
12,10,0,5,116,111,107,101,110,0,0,0,9,8,8,10,
12,9,0,4,101,108,115,101,0,0,0,0,31,6,8,2,
19,6,7,6,21,6,0,0,18,0,0,99,30,6,1,30,
32,32,32,32,32,32,32,32,116,111,107,32,61,32,80,46,
116,111,107,101,110,0,0,0,12,7,0,1,80,0,0,0,
13,6,7,0,12,7,0,5,116,111,107,101,110,0,0,0,
9,6,6,7,15,5,6,0,30,6,1,31,32,32,32,32,
32,32,32,32,97,100,118,97,110,99,101,40,39,101,108,115,
101,39,41,0,12,8,0,7,97,100,118,97,110,99,101,0,
13,7,8,0,12,8,0,4,101,108,115,101,0,0,0,0,
31,6,8,1,19,6,7,6,30,6,1,32,32,32,32,32,
32,32,32,32,97,100,118,97,110,99,101,40,39,58,39,41,
0,0,0,0,12,8,0,7,97,100,118,97,110,99,101,0,
13,7,8,0,12,8,0,1,58,0,0,0,31,6,8,1,
19,6,7,6,30,5,1,33,32,32,32,32,32,32,32,32,
98,32,61,32,98,108,111,99,107,40,41,0,12,8,0,5,
98,108,111,99,107,0,0,0,13,7,8,0,31,6,0,0,
19,6,7,6,15,4,6,0,30,14,1,34,32,32,32,32,
32,32,32,32,105,116,101,109,115,46,97,112,112,101,110,100,
40,84,111,107,101,110,40,116,111,107,46,112,111,115,44,39,
101,108,115,101,39,44,39,101,108,115,101,39,44,91,98,93,
41,41,0,0,12,8,0,6,97,112,112,101,110,100,0,0,
9,7,2,8,12,10,0,5,84,111,107,101,110,0,0,0,
13,9,10,0,12,14,0,3,112,111,115,0,9,10,5,14,
12,11,0,4,101,108,115,101,0,0,0,0,12,12,0,4,
101,108,115,101,0,0,0,0,15,14,4,0,27,13,14,1,
31,8,10,4,19,8,9,8,31,6,8,1,19,6,7,6,
18,0,0,1,30,4,1,35,32,32,32,32,114,101,116,117,
114,110,32,116,0,0,0,0,20,1,0,0,0,0,0,0,
12,36,0,6,105,102,95,110,117,100,0,0,14,36,35,0,
30,4,1,36,100,101,102,32,116,114,121,95,110,117,100,40,
116,41,58,0,16,36,1,28,44,16,0,0,30,4,1,36,
100,101,102,32,116,114,121,95,110,117,100,40,116,41,58,0,
12,1,0,8,112,97,114,115,101,46,112,121,0,0,0,0,
33,1,0,0,12,1,0,7,116,114,121,95,110,117,100,0,
34,1,0,0,28,2,0,0,9,1,0,2,30,7,1,37,
32,32,32,32,105,116,101,109,115,32,61,32,116,46,105,116,
101,109,115,32,61,32,91,93,0,0,0,0,27,3,0,0,
12,4,0,5,105,116,101,109,115,0,0,0,10,1,4,3,
15,2,3,0,30,5,1,38,32,32,32,32,97,100,118,97,
110,99,101,40,39,58,39,41,0,0,0,0,12,5,0,7,
97,100,118,97,110,99,101,0,13,4,5,0,12,5,0,1,
58,0,0,0,31,3,5,1,19,3,4,3,30,4,1,39,
32,32,32,32,98,32,61,32,98,108,111,99,107,40,41,0,
12,6,0,5,98,108,111,99,107,0,0,0,13,5,6,0,
31,4,0,0,19,4,5,4,15,3,4,0,30,5,1,40,
32,32,32,32,105,116,101,109,115,46,97,112,112,101,110,100,
40,98,41,0,12,6,0,6,97,112,112,101,110,100,0,0,
9,5,2,6,15,6,3,0,31,4,6,1,19,4,5,4,
30,9,1,41,32,32,32,32,119,104,105,108,101,32,99,104,
101,99,107,40,80,46,116,111,107,101,110,44,39,101,120,99,
101,112,116,39,41,58,0,0,12,6,0,5,99,104,101,99,
107,0,0,0,13,5,6,0,12,8,0,1,80,0,0,0,
13,6,8,0,12,8,0,5,116,111,107,101,110,0,0,0,
9,6,6,8,12,7,0,6,101,120,99,101,112,116,0,0,
31,4,6,2,19,4,5,4,21,4,0,0,18,0,0,179,
30,6,1,42,32,32,32,32,32,32,32,32,116,111,107,32,
61,32,80,46,116,111,107,101,110,0,0,0,12,6,0,1,
80,0,0,0,13,5,6,0,12,6,0,5,116,111,107,101,
110,0,0,0,9,5,5,6,15,4,5,0,30,7,1,43,
32,32,32,32,32,32,32,32,97,100,118,97,110,99,101,40,
39,101,120,99,101,112,116,39,41,0,0,0,12,7,0,7,
97,100,118,97,110,99,101,0,13,6,7,0,12,7,0,6,
101,120,99,101,112,116,0,0,31,5,7,1,19,5,6,5,
30,14,1,44,32,32,32,32,32,32,32,32,105,102,32,110,
111,116,32,99,104,101,99,107,40,80,46,116,111,107,101,110,
44,39,58,39,41,58,32,97,32,61,32,101,120,112,114,101,
115,115,105,111,110,40,48,41,0,0,0,0,12,8,0,5,
99,104,101,99,107,0,0,0,13,7,8,0,12,10,0,1,
80,0,0,0,13,8,10,0,12,10,0,5,116,111,107,101,
110,0,0,0,9,8,8,10,12,9,0,1,58,0,0,0,
31,6,8,2,19,6,7,6,47,5,6,0,21,5,0,0,
18,0,0,13,12,8,0,10,101,120,112,114,101,115,115,105,
111,110,0,0,13,7,8,0,11,8,0,0,0,0,0,0,
0,0,0,0,31,6,8,1,19,6,7,6,15,5,6,0,
18,0,0,32,30,13,1,45,32,32,32,32,32,32,32,32,
101,108,115,101,58,32,97,32,61,32,84,111,107,101,110,40,
116,111,107,46,112,111,115,44,39,115,121,109,98,111,108,39,
44,39,78,111,110,101,39,41,0,0,0,0,12,8,0,5,
84,111,107,101,110,0,0,0,13,7,8,0,12,11,0,3,
112,111,115,0,9,8,4,11,12,9,0,6,115,121,109,98,
111,108,0,0,12,10,0,4,78,111,110,101,0,0,0,0,
31,6,8,3,19,6,7,6,15,5,6,0,18,0,0,1,
30,6,1,46,32,32,32,32,32,32,32,32,97,100,118,97,
110,99,101,40,39,58,39,41,0,0,0,0,12,8,0,7,
97,100,118,97,110,99,101,0,13,7,8,0,12,8,0,1,
58,0,0,0,31,6,8,1,19,6,7,6,30,5,1,47,
32,32,32,32,32,32,32,32,98,32,61,32,98,108,111,99,
107,40,41,0,12,8,0,5,98,108,111,99,107,0,0,0,
13,7,8,0,31,6,0,0,19,6,7,6,15,3,6,0,
30,16,1,48,32,32,32,32,32,32,32,32,105,116,101,109,
115,46,97,112,112,101,110,100,40,84,111,107,101,110,40,116,
111,107,46,112,111,115,44,39,101,120,99,101,112,116,39,44,
39,101,120,99,101,112,116,39,44,91,97,44,98,93,41,41,
0,0,0,0,12,8,0,6,97,112,112,101,110,100,0,0,
9,7,2,8,12,10,0,5,84,111,107,101,110,0,0,0,
13,9,10,0,12,14,0,3,112,111,115,0,9,10,4,14,
12,11,0,6,101,120,99,101,112,116,0,0,12,12,0,6,
101,120,99,101,112,116,0,0,15,14,5,0,15,15,3,0,
27,13,14,2,31,8,10,4,19,8,9,8,31,6,8,1,
19,6,7,6,18,0,255,61,30,4,1,56,32,32,32,32,
114,101,116,117,114,110,32,116,0,0,0,0,20,1,0,0,
0,0,0,0,12,37,0,7,116,114,121,95,110,117,100,0,
14,37,36,0,30,5,1,57,100,101,102,32,112,114,101,102,
105,120,95,110,117,100,40,116,41,58,0,0,16,37,0,58,
44,7,0,0,30,5,1,57,100,101,102,32,112,114,101,102,
105,120,95,110,117,100,40,116,41,58,0,0,12,1,0,8,
112,97,114,115,101,46,112,121,0,0,0,0,33,1,0,0,
12,1,0,10,112,114,101,102,105,120,95,110,117,100,0,0,
34,1,0,0,28,2,0,0,9,1,0,2,30,4,1,60,
32,32,32,32,98,112,32,61,32,116,46,98,112,0,0,0,
12,4,0,2,98,112,0,0,9,3,1,4,15,2,3,0,
30,8,1,61,32,32,32,32,116,46,105,116,101,109,115,32,
61,32,91,101,120,112,114,101,115,115,105,111,110,40,98,112,
41,93,0,0,12,6,0,10,101,120,112,114,101,115,115,105,
111,110,0,0,13,5,6,0,15,6,2,0,31,4,6,1,
19,4,5,4,27,3,4,1,12,4,0,5,105,116,101,109,
115,0,0,0,10,1,4,3,30,4,1,62,32,32,32,32,
114,101,116,117,114,110,32,116,0,0,0,0,20,1,0,0,
0,0,0,0,12,38,0,10,112,114,101,102,105,120,95,110,
117,100,0,0,14,38,37,0,30,5,1,63,100,101,102,32,
112,114,101,102,105,120,95,110,117,100,48,40,116,41,58,0,
16,38,0,79,44,10,0,0,30,5,1,63,100,101,102,32,
112,114,101,102,105,120,95,110,117,100,48,40,116,41,58,0,
12,1,0,8,112,97,114,115,101,46,112,121,0,0,0,0,
33,1,0,0,12,1,0,11,112,114,101,102,105,120,95,110,
117,100,48,0,34,1,0,0,28,2,0,0,9,1,0,2,
30,14,1,64,32,32,32,32,105,102,32,99,104,101,99,107,
40,80,46,116,111,107,101,110,44,39,110,108,39,44,39,59,
39,44,39,101,111,102,39,44,39,100,101,100,101,110,116,39,
41,58,32,114,101,116,117,114,110,32,116,0,12,4,0,5,
99,104,101,99,107,0,0,0,13,3,4,0,12,9,0,1,
80,0,0,0,13,4,9,0,12,9,0,5,116,111,107,101,
110,0,0,0,9,4,4,9,12,5,0,2,110,108,0,0,
12,6,0,1,59,0,0,0,12,7,0,3,101,111,102,0,
12,8,0,6,100,101,100,101,110,116,0,0,31,2,4,5,
19,2,3,2,21,2,0,0,18,0,0,3,20,1,0,0,
18,0,0,1,30,7,1,65,32,32,32,32,114,101,116,117,
114,110,32,112,114,101,102,105,120,95,110,117,100,40,116,41,
0,0,0,0,12,4,0,10,112,114,101,102,105,120,95,110,
117,100,0,0,13,3,4,0,15,4,1,0,31,2,4,1,
19,2,3,2,20,2,0,0,0,0,0,0,12,39,0,11,
112,114,101,102,105,120,95,110,117,100,48,0,14,39,38,0,
30,5,1,66,100,101,102,32,112,114,101,102,105,120,95,110,
117,100,115,40,116,41,58,0,16,39,0,59,44,8,0,0,
30,5,1,66,100,101,102,32,112,114,101,102,105,120,95,110,
117,100,115,40,116,41,58,0,12,1,0,8,112,97,114,115,
101,46,112,121,0,0,0,0,33,1,0,0,12,1,0,11,
112,114,101,102,105,120,95,110,117,100,115,0,34,1,0,0,
28,2,0,0,9,1,0,2,30,6,1,67,32,32,32,32,
114,32,61,32,101,120,112,114,101,115,115,105,111,110,40,48,
41,0,0,0,12,5,0,10,101,120,112,114,101,115,115,105,
111,110,0,0,13,4,5,0,11,5,0,0,0,0,0,0,
0,0,0,0,31,3,5,1,19,3,4,3,15,2,3,0,
30,7,1,68,32,32,32,32,114,101,116,117,114,110,32,105,
108,115,116,40,116,46,116,121,112,101,44,114,41,0,0,0,
12,5,0,4,105,108,115,116,0,0,0,0,13,4,5,0,
12,7,0,4,116,121,112,101,0,0,0,0,9,5,1,7,
15,6,2,0,31,3,5,2,19,3,4,3,20,3,0,0,
0,0,0,0,12,40,0,11,112,114,101,102,105,120,95,110,
117,100,115,0,14,40,39,0,30,5,1,70,100,101,102,32,
112,114,101,102,105,120,95,110,101,103,40,116,41,58,0,0,
16,40,0,134,44,11,0,0,30,5,1,70,100,101,102,32,
112,114,101,102,105,120,95,110,101,103,40,116,41,58,0,0,
12,1,0,8,112,97,114,115,101,46,112,121,0,0,0,0,
33,1,0,0,12,1,0,10,112,114,101,102,105,120,95,110,
101,103,0,0,34,1,0,0,28,2,0,0,9,1,0,2,
30,6,1,71,32,32,32,32,114,32,61,32,101,120,112,114,
101,115,115,105,111,110,40,53,48,41,0,0,12,5,0,10,
101,120,112,114,101,115,115,105,111,110,0,0,13,4,5,0,
11,5,0,0,0,0,0,0,0,0,73,64,31,3,5,1,
19,3,4,3,15,2,3,0,30,7,1,72,32,32,32,32,
105,102,32,114,46,116,121,112,101,32,61,61,32,39,110,117,
109,98,101,114,39,58,0,0,12,4,0,4,116,121,112,101,
0,0,0,0,9,3,2,4,12,4,0,6,110,117,109,98,
101,114,0,0,23,3,3,4,21,3,0,0,18,0,0,40,
30,9,1,73,32,32,32,32,32,32,32,32,114,46,118,97,
108,32,61,32,115,116,114,40,45,102,108,111,97,116,40,114,
46,118,97,108,41,41,0,0,12,5,0,3,115,116,114,0,
13,4,5,0,11,5,0,0,0,0,0,0,0,0,0,0,
12,8,0,5,102,108,111,97,116,0,0,0,13,7,8,0,
12,9,0,3,118,97,108,0,9,8,2,9,31,6,8,1,
19,6,7,6,2,5,5,6,31,3,5,1,19,3,4,3,
12,4,0,3,118,97,108,0,10,2,4,3,30,5,1,74,
32,32,32,32,32,32,32,32,114,101,116,117,114,110,32,114,
0,0,0,0,20,2,0,0,18,0,0,1,30,11,1,75,
32,32,32,32,116,46,105,116,101,109,115,32,61,32,91,84,
111,107,101,110,40,116,46,112,111,115,44,39,110,117,109,98,
101,114,39,44,39,48,39,41,44,114,93,0,12,7,0,5,
84,111,107,101,110,0,0,0,13,6,7,0,12,10,0,3,
112,111,115,0,9,7,1,10,12,8,0,6,110,117,109,98,
101,114,0,0,12,9,0,1,48,0,0,0,31,4,7,3,
19,4,6,4,15,5,2,0,27,3,4,2,12,4,0,5,
105,116,101,109,115,0,0,0,10,1,4,3,30,4,1,76,
32,32,32,32,114,101,116,117,114,110,32,116,0,0,0,0,
20,1,0,0,0,0,0,0,12,41,0,10,112,114,101,102,
105,120,95,110,101,103,0,0,14,41,40,0,30,5,1,77,
100,101,102,32,118,97,114,103,115,95,110,117,100,40,116,41,
58,0,0,0,16,41,0,66,44,6,0,0,30,5,1,77,
100,101,102,32,118,97,114,103,115,95,110,117,100,40,116,41,
58,0,0,0,12,1,0,8,112,97,114,115,101,46,112,121,
0,0,0,0,33,1,0,0,12,1,0,9,118,97,114,103,
115,95,110,117,100,0,0,0,34,1,0,0,28,2,0,0,
9,1,0,2,30,6,1,78,32,32,32,32,114,32,61,32,
112,114,101,102,105,120,95,110,117,100,40,116,41,0,0,0,
12,5,0,10,112,114,101,102,105,120,95,110,117,100,0,0,
13,4,5,0,15,5,1,0,31,3,5,1,19,3,4,3,
15,2,3,0,30,5,1,79,32,32,32,32,116,46,116,121,
112,101,32,61,32,39,97,114,103,115,39,0,12,3,0,4,
97,114,103,115,0,0,0,0,12,4,0,4,116,121,112,101,
0,0,0,0,10,1,4,3,30,4,1,80,32,32,32,32,
116,46,118,97,108,32,61,32,39,42,39,0,12,3,0,1,
42,0,0,0,12,4,0,3,118,97,108,0,10,1,4,3,
30,4,1,81,32,32,32,32,114,101,116,117,114,110,32,116,
0,0,0,0,20,1,0,0,0,0,0,0,12,42,0,9,
118,97,114,103,115,95,110,117,100,0,0,0,14,42,41,0,
30,5,1,82,100,101,102,32,110,97,114,103,115,95,110,117,
100,40,116,41,58,0,0,0,16,42,0,68,44,6,0,0,
30,5,1,82,100,101,102,32,110,97,114,103,115,95,110,117,
100,40,116,41,58,0,0,0,12,1,0,8,112,97,114,115,
101,46,112,121,0,0,0,0,33,1,0,0,12,1,0,9,
110,97,114,103,115,95,110,117,100,0,0,0,34,1,0,0,
28,2,0,0,9,1,0,2,30,6,1,83,32,32,32,32,
114,32,61,32,112,114,101,102,105,120,95,110,117,100,40,116,
41,0,0,0,12,5,0,10,112,114,101,102,105,120,95,110,
117,100,0,0,13,4,5,0,15,5,1,0,31,3,5,1,
19,3,4,3,15,2,3,0,30,6,1,84,32,32,32,32,
116,46,116,121,112,101,32,61,32,39,110,97,114,103,115,39,
0,0,0,0,12,3,0,5,110,97,114,103,115,0,0,0,
12,4,0,4,116,121,112,101,0,0,0,0,10,1,4,3,
30,5,1,85,32,32,32,32,116,46,118,97,108,32,61,32,
39,42,42,39,0,0,0,0,12,3,0,2,42,42,0,0,
12,4,0,3,118,97,108,0,10,1,4,3,30,4,1,86,
32,32,32,32,114,101,116,117,114,110,32,116,0,0,0,0,
20,1,0,0,0,0,0,0,12,43,0,9,110,97,114,103,
115,95,110,117,100,0,0,0,14,43,42,0,30,4,1,89,
98,97,115,101,95,100,109,97,112,32,61,32,123,0,0,0,
12,43,0,9,98,97,115,101,95,100,109,97,112,0,0,0,
30,12,1,90,32,32,32,32,39,44,39,58,123,39,108,98,
112,39,58,50,48,44,39,98,112,39,58,50,48,44,39,108,
101,100,39,58,105,110,102,105,120,95,116,117,112,108,101,125,
44,0,0,0,12,45,0,1,44,0,0,0,12,103,0,3,
108,98,112,0,11,104,0,0,0,0,0,0,0,0,52,64,
12,105,0,2,98,112,0,0,11,106,0,0,0,0,0,0,
0,0,52,64,12,107,0,3,108,101,100,0,12,109,0,11,
105,110,102,105,120,95,116,117,112,108,101,0,13,108,109,0,
26,46,103,6,30,11,1,91,32,32,32,32,39,43,39,58,
123,39,108,98,112,39,58,53,48,44,39,98,112,39,58,53,
48,44,39,108,101,100,39,58,105,110,102,105,120,95,108,101,
100,125,44,0,12,47,0,1,43,0,0,0,12,103,0,3,
108,98,112,0,11,104,0,0,0,0,0,0,0,0,73,64,
12,105,0,2,98,112,0,0,11,106,0,0,0,0,0,0,
0,0,73,64,12,107,0,3,108,101,100,0,12,109,0,9,
105,110,102,105,120,95,108,101,100,0,0,0,13,108,109,0,
26,48,103,6,30,9,1,92,32,32,32,32,39,45,39,58,
123,39,108,98,112,39,58,53,48,44,39,110,117,100,39,58,
112,114,101,102,105,120,95,110,101,103,44,0,12,49,0,1,
45,0,0,0,12,103,0,3,108,98,112,0,11,104,0,0,
0,0,0,0,0,0,73,64,12,105,0,3,110,117,100,0,
12,111,0,10,112,114,101,102,105,120,95,110,101,103,0,0,
13,106,111,0,30,9,1,93,32,32,32,32,32,32,32,32,
39,98,112,39,58,53,48,44,39,108,101,100,39,58,105,110,
102,105,120,95,108,101,100,125,44,0,0,0,12,107,0,2,
98,112,0,0,11,108,0,0,0,0,0,0,0,0,73,64,
12,109,0,3,108,101,100,0,12,111,0,9,105,110,102,105,
120,95,108,101,100,0,0,0,13,110,111,0,26,50,103,8,
30,12,1,94,32,32,32,32,39,110,111,116,39,58,123,39,
108,98,112,39,58,51,53,44,39,110,117,100,39,58,112,114,
101,102,105,120,95,110,117,100,44,39,98,112,39,58,51,53,
44,0,0,0,12,51,0,3,110,111,116,0,12,103,0,3,
108,98,112,0,11,104,0,0,0,0,0,0,0,128,65,64,
12,105,0,3,110,117,100,0,12,113,0,10,112,114,101,102,
105,120,95,110,117,100,0,0,13,106,113,0,12,107,0,2,
98,112,0,0,11,108,0,0,0,0,0,0,0,128,65,64,
30,9,1,95,32,32,32,32,32,32,32,32,39,98,112,39,
58,51,53,44,39,108,101,100,39,58,105,110,102,105,120,95,
110,111,116,32,125,44,0,0,12,109,0,2,98,112,0,0,
11,110,0,0,0,0,0,0,0,128,65,64,12,111,0,3,
108,101,100,0,12,113,0,9,105,110,102,105,120,95,110,111,
116,0,0,0,13,112,113,0,26,52,103,10,30,11,1,96,
32,32,32,32,39,37,39,58,123,39,108,98,112,39,58,54,
48,44,39,98,112,39,58,54,48,44,39,108,101,100,39,58,
105,110,102,105,120,95,108,101,100,125,44,0,12,53,0,1,
37,0,0,0,12,103,0,3,108,98,112,0,11,104,0,0,
0,0,0,0,0,0,78,64,12,105,0,2,98,112,0,0,
11,106,0,0,0,0,0,0,0,0,78,64,12,107,0,3,
108,101,100,0,12,109,0,9,105,110,102,105,120,95,108,101,
100,0,0,0,13,108,109,0,26,54,103,6,30,9,1,97,
32,32,32,32,39,42,39,58,123,39,108,98,112,39,58,54,
48,44,39,110,117,100,39,58,118,97,114,103,115,95,110,117,
100,44,0,0,12,55,0,1,42,0,0,0,12,103,0,3,
108,98,112,0,11,104,0,0,0,0,0,0,0,0,78,64,
12,105,0,3,110,117,100,0,12,111,0,9,118,97,114,103,
115,95,110,117,100,0,0,0,13,106,111,0,30,9,1,98,
32,32,32,32,32,32,32,32,39,98,112,39,58,54,48,44,
39,108,101,100,39,58,105,110,102,105,120,95,108,101,100,44,
125,44,0,0,12,107,0,2,98,112,0,0,11,108,0,0,
0,0,0,0,0,0,78,64,12,109,0,3,108,101,100,0,
12,111,0,9,105,110,102,105,120,95,108,101,100,0,0,0,
13,110,111,0,26,56,103,8,30,10,1,99,32,32,32,32,
39,42,42,39,58,32,123,39,108,98,112,39,58,54,53,44,
39,110,117,100,39,58,110,97,114,103,115,95,110,117,100,44,
0,0,0,0,12,57,0,2,42,42,0,0,12,103,0,3,
108,98,112,0,11,104,0,0,0,0,0,0,0,64,80,64,
12,105,0,3,110,117,100,0,12,111,0,9,110,97,114,103,
115,95,110,117,100,0,0,0,13,106,111,0,30,9,1,100,
32,32,32,32,32,32,32,32,39,98,112,39,58,54,53,44,
39,108,101,100,39,58,105,110,102,105,120,95,108,101,100,44,
125,44,0,0,12,107,0,2,98,112,0,0,11,108,0,0,
0,0,0,0,0,64,80,64,12,109,0,3,108,101,100,0,
12,111,0,9,105,110,102,105,120,95,108,101,100,0,0,0,
13,110,111,0,26,58,103,8,30,11,1,101,32,32,32,32,
39,47,39,58,123,39,108,98,112,39,58,54,48,44,39,98,
112,39,58,54,48,44,39,108,101,100,39,58,105,110,102,105,
120,95,108,101,100,125,44,0,12,59,0,1,47,0,0,0,
12,103,0,3,108,98,112,0,11,104,0,0,0,0,0,0,
0,0,78,64,12,105,0,2,98,112,0,0,11,106,0,0,
0,0,0,0,0,0,78,64,12,107,0,3,108,101,100,0,
12,109,0,9,105,110,102,105,120,95,108,101,100,0,0,0,
13,108,109,0,26,60,103,6,30,9,1,102,32,32,32,32,
39,40,39,58,123,39,108,98,112,39,58,55,48,44,39,110,
117,100,39,58,112,97,114,101,110,95,110,117,100,44,0,0,
12,61,0,1,40,0,0,0,12,103,0,3,108,98,112,0,
11,104,0,0,0,0,0,0,0,128,81,64,12,105,0,3,
110,117,100,0,12,111,0,9,112,97,114,101,110,95,110,117,
100,0,0,0,13,106,111,0,30,9,1,103,32,32,32,32,
32,32,32,32,39,98,112,39,58,56,48,44,39,108,101,100,
39,58,99,97,108,108,95,108,101,100,44,125,44,0,0,0,
12,107,0,2,98,112,0,0,11,108,0,0,0,0,0,0,
0,0,84,64,12,109,0,3,108,101,100,0,12,111,0,8,
99,97,108,108,95,108,101,100,0,0,0,0,13,110,111,0,
26,62,103,8,30,9,1,104,32,32,32,32,39,91,39,58,
123,39,108,98,112,39,58,55,48,44,39,110,117,100,39,58,
108,105,115,116,95,110,117,100,44,0,0,0,12,63,0,1,
91,0,0,0,12,103,0,3,108,98,112,0,11,104,0,0,
0,0,0,0,0,128,81,64,12,105,0,3,110,117,100,0,
12,111,0,8,108,105,115,116,95,110,117,100,0,0,0,0,
13,106,111,0,30,9,1,105,32,32,32,32,32,32,32,32,
39,98,112,39,58,56,48,44,39,108,101,100,39,58,103,101,
116,95,108,101,100,44,125,44,0,0,0,0,12,107,0,2,
98,112,0,0,11,108,0,0,0,0,0,0,0,0,84,64,
12,109,0,3,108,101,100,0,12,111,0,7,103,101,116,95,
108,101,100,0,13,110,111,0,26,64,103,8,30,9,1,106,
32,32,32,32,39,123,39,58,123,39,108,98,112,39,58,48,
44,39,110,117,100,39,58,100,105,99,116,95,110,117,100,44,
125,44,0,0,12,65,0,1,123,0,0,0,12,103,0,3,
108,98,112,0,11,104,0,0,0,0,0,0,0,0,0,0,
12,105,0,3,110,117,100,0,12,107,0,8,100,105,99,116,
95,110,117,100,0,0,0,0,13,106,107,0,26,66,103,4,
30,14,1,107,32,32,32,32,39,46,39,58,123,39,108,98,
112,39,58,56,48,44,39,98,112,39,58,56,48,44,39,108,
101,100,39,58,100,111,116,95,108,101,100,44,39,116,121,112,
101,39,58,39,103,101,116,39,44,125,44,0,12,67,0,1,
46,0,0,0,12,103,0,3,108,98,112,0,11,104,0,0,
0,0,0,0,0,0,84,64,12,105,0,2,98,112,0,0,
11,106,0,0,0,0,0,0,0,0,84,64,12,107,0,3,
108,101,100,0,12,111,0,7,100,111,116,95,108,101,100,0,
13,108,111,0,12,109,0,4,116,121,112,101,0,0,0,0,
12,110,0,3,103,101,116,0,26,68,103,8,30,13,1,108,
32,32,32,32,39,98,114,101,97,107,39,58,123,39,108,98,
112,39,58,48,44,39,110,117,100,39,58,105,116,115,101,108,
102,44,39,116,121,112,101,39,58,39,98,114,101,97,107,39,
125,44,0,0,12,69,0,5,98,114,101,97,107,0,0,0,
12,103,0,3,108,98,112,0,11,104,0,0,0,0,0,0,
0,0,0,0,12,105,0,3,110,117,100,0,12,109,0,6,
105,116,115,101,108,102,0,0,13,106,109,0,12,107,0,4,
116,121,112,101,0,0,0,0,12,108,0,5,98,114,101,97,
107,0,0,0,26,70,103,6,30,13,1,109,32,32,32,32,
39,112,97,115,115,39,58,123,39,108,98,112,39,58,48,44,
39,110,117,100,39,58,105,116,115,101,108,102,44,39,116,121,
112,101,39,58,39,112,97,115,115,39,125,44,0,0,0,0,
12,71,0,4,112,97,115,115,0,0,0,0,12,103,0,3,
108,98,112,0,11,104,0,0,0,0,0,0,0,0,0,0,
12,105,0,3,110,117,100,0,12,109,0,6,105,116,115,101,
108,102,0,0,13,106,109,0,12,107,0,4,116,121,112,101,
0,0,0,0,12,108,0,4,112,97,115,115,0,0,0,0,
26,72,103,6,30,15,1,110,32,32,32,32,39,99,111,110,
116,105,110,117,101,39,58,123,39,108,98,112,39,58,48,44,
39,110,117,100,39,58,105,116,115,101,108,102,44,39,116,121,
112,101,39,58,39,99,111,110,116,105,110,117,101,39,125,44,
0,0,0,0,12,73,0,8,99,111,110,116,105,110,117,101,
0,0,0,0,12,103,0,3,108,98,112,0,11,104,0,0,
0,0,0,0,0,0,0,0,12,105,0,3,110,117,100,0,
12,109,0,6,105,116,115,101,108,102,0,0,13,106,109,0,
12,107,0,4,116,121,112,101,0,0,0,0,12,108,0,8,
99,111,110,116,105,110,117,101,0,0,0,0,26,74,103,6,
30,12,1,111,32,32,32,32,39,101,111,102,39,58,123,39,
108,98,112,39,58,48,44,39,116,121,112,101,39,58,39,101,
111,102,39,44,39,118,97,108,39,58,39,101,111,102,39,125,
44,0,0,0,12,75,0,3,101,111,102,0,12,103,0,3,
108,98,112,0,11,104,0,0,0,0,0,0,0,0,0,0,
12,105,0,4,116,121,112,101,0,0,0,0,12,106,0,3,
101,111,102,0,12,107,0,3,118,97,108,0,12,108,0,3,
101,111,102,0,26,76,103,6,30,13,1,112,32,32,32,32,
39,100,101,102,39,58,123,39,108,98,112,39,58,48,44,39,
110,117,100,39,58,100,101,102,95,110,117,100,44,39,116,121,
112,101,39,58,39,100,101,102,39,44,125,44,0,0,0,0,
12,77,0,3,100,101,102,0,12,103,0,3,108,98,112,0,
11,104,0,0,0,0,0,0,0,0,0,0,12,105,0,3,
110,117,100,0,12,109,0,7,100,101,102,95,110,117,100,0,
13,106,109,0,12,107,0,4,116,121,112,101,0,0,0,0,
12,108,0,3,100,101,102,0,26,78,103,6,30,14,1,113,
32,32,32,32,39,119,104,105,108,101,39,58,123,39,108,98,
112,39,58,48,44,39,110,117,100,39,58,119,104,105,108,101,
95,110,117,100,44,39,116,121,112,101,39,58,39,119,104,105,
108,101,39,44,125,44,0,0,12,79,0,5,119,104,105,108,
101,0,0,0,12,103,0,3,108,98,112,0,11,104,0,0,
0,0,0,0,0,0,0,0,12,105,0,3,110,117,100,0,
12,109,0,9,119,104,105,108,101,95,110,117,100,0,0,0,
13,106,109,0,12,107,0,4,116,121,112,101,0,0,0,0,
12,108,0,5,119,104,105,108,101,0,0,0,26,80,103,6,
30,13,1,114,32,32,32,32,39,102,111,114,39,58,123,39,
108,98,112,39,58,48,44,39,110,117,100,39,58,102,111,114,
95,110,117,100,44,39,116,121,112,101,39,58,39,102,111,114,
39,44,125,44,0,0,0,0,12,81,0,3,102,111,114,0,
12,103,0,3,108,98,112,0,11,104,0,0,0,0,0,0,
0,0,0,0,12,105,0,3,110,117,100,0,12,109,0,7,
102,111,114,95,110,117,100,0,13,106,109,0,12,107,0,4,
116,121,112,101,0,0,0,0,12,108,0,3,102,111,114,0,
26,82,103,6,30,13,1,115,32,32,32,32,39,116,114,121,
39,58,123,39,108,98,112,39,58,48,44,39,110,117,100,39,
58,116,114,121,95,110,117,100,44,39,116,121,112,101,39,58,
39,116,114,121,39,44,125,44,0,0,0,0,12,83,0,3,
116,114,121,0,12,103,0,3,108,98,112,0,11,104,0,0,
0,0,0,0,0,0,0,0,12,105,0,3,110,117,100,0,
12,109,0,7,116,114,121,95,110,117,100,0,13,106,109,0,
12,107,0,4,116,121,112,101,0,0,0,0,12,108,0,3,
116,114,121,0,26,84,103,6,30,12,1,116,32,32,32,32,
39,105,102,39,58,123,39,108,98,112,39,58,48,44,39,110,
117,100,39,58,105,102,95,110,117,100,44,39,116,121,112,101,
39,58,39,105,102,39,44,125,44,0,0,0,12,85,0,2,
105,102,0,0,12,103,0,3,108,98,112,0,11,104,0,0,
0,0,0,0,0,0,0,0,12,105,0,3,110,117,100,0,
12,109,0,6,105,102,95,110,117,100,0,0,13,106,109,0,
12,107,0,4,116,121,112,101,0,0,0,0,12,108,0,2,
105,102,0,0,26,86,103,6,30,14,1,117,32,32,32,32,
39,99,108,97,115,115,39,58,123,39,108,98,112,39,58,48,
44,39,110,117,100,39,58,99,108,97,115,115,95,110,117,100,
44,39,116,121,112,101,39,58,39,99,108,97,115,115,39,44,
125,44,0,0,12,87,0,5,99,108,97,115,115,0,0,0,
12,103,0,3,108,98,112,0,11,104,0,0,0,0,0,0,
0,0,0,0,12,105,0,3,110,117,100,0,12,109,0,9,
99,108,97,115,115,95,110,117,100,0,0,0,13,106,109,0,
12,107,0,4,116,121,112,101,0,0,0,0,12,108,0,5,
99,108,97,115,115,0,0,0,26,88,103,6,30,17,1,118,
32,32,32,32,39,114,97,105,115,101,39,58,123,39,108,98,
112,39,58,48,44,39,110,117,100,39,58,112,114,101,102,105,
120,95,110,117,100,48,44,39,116,121,112,101,39,58,39,114,
97,105,115,101,39,44,39,98,112,39,58,50,48,44,125,44,
0,0,0,0,12,89,0,5,114,97,105,115,101,0,0,0,
12,103,0,3,108,98,112,0,11,104,0,0,0,0,0,0,
0,0,0,0,12,105,0,3,110,117,100,0,12,111,0,11,
112,114,101,102,105,120,95,110,117,100,48,0,13,106,111,0,
12,107,0,4,116,121,112,101,0,0,0,0,12,108,0,5,
114,97,105,115,101,0,0,0,12,109,0,2,98,112,0,0,
11,110,0,0,0,0,0,0,0,0,52,64,26,90,103,8,
30,17,1,119,32,32,32,32,39,114,101,116,117,114,110,39,
58,123,39,108,98,112,39,58,48,44,39,110,117,100,39,58,
112,114,101,102,105,120,95,110,117,100,48,44,39,116,121,112,
101,39,58,39,114,101,116,117,114,110,39,44,39,98,112,39,
58,49,48,44,125,44,0,0,12,91,0,6,114,101,116,117,
114,110,0,0,12,103,0,3,108,98,112,0,11,104,0,0,
0,0,0,0,0,0,0,0,12,105,0,3,110,117,100,0,
12,111,0,11,112,114,101,102,105,120,95,110,117,100,48,0,
13,106,111,0,12,107,0,4,116,121,112,101,0,0,0,0,
12,108,0,6,114,101,116,117,114,110,0,0,12,109,0,2,
98,112,0,0,11,110,0,0,0,0,0,0,0,0,36,64,
26,92,103,8,30,17,1,120,32,32,32,32,39,105,109,112,
111,114,116,39,58,123,39,108,98,112,39,58,48,44,39,110,
117,100,39,58,112,114,101,102,105,120,95,110,117,100,115,44,
39,116,121,112,101,39,58,39,105,109,112,111,114,116,39,44,
39,98,112,39,58,50,48,44,125,44,0,0,12,93,0,6,
105,109,112,111,114,116,0,0,12,103,0,3,108,98,112,0,
11,104,0,0,0,0,0,0,0,0,0,0,12,105,0,3,
110,117,100,0,12,111,0,11,112,114,101,102,105,120,95,110,
117,100,115,0,13,106,111,0,12,107,0,4,116,121,112,101,
0,0,0,0,12,108,0,6,105,109,112,111,114,116,0,0,
12,109,0,2,98,112,0,0,11,110,0,0,0,0,0,0,
0,0,52,64,26,94,103,8,30,15,1,121,32,32,32,32,
39,102,114,111,109,39,58,123,39,108,98,112,39,58,48,44,
39,110,117,100,39,58,102,114,111,109,95,110,117,100,44,39,
116,121,112,101,39,58,39,102,114,111,109,39,44,39,98,112,
39,58,50,48,44,125,44,0,12,95,0,4,102,114,111,109,
0,0,0,0,12,103,0,3,108,98,112,0,11,104,0,0,
0,0,0,0,0,0,0,0,12,105,0,3,110,117,100,0,
12,111,0,8,102,114,111,109,95,110,117,100,0,0,0,0,
13,106,111,0,12,107,0,4,116,121,112,101,0,0,0,0,
12,108,0,4,102,114,111,109,0,0,0,0,12,109,0,2,
98,112,0,0,11,110,0,0,0,0,0,0,0,0,52,64,
26,96,103,8,30,16,1,122,32,32,32,32,39,100,101,108,
39,58,123,39,108,98,112,39,58,48,44,39,110,117,100,39,
58,112,114,101,102,105,120,95,110,117,100,115,44,39,116,121,
112,101,39,58,39,100,101,108,39,44,39,98,112,39,58,49,
48,44,125,44,0,0,0,0,12,97,0,3,100,101,108,0,
12,103,0,3,108,98,112,0,11,104,0,0,0,0,0,0,
0,0,0,0,12,105,0,3,110,117,100,0,12,111,0,11,
112,114,101,102,105,120,95,110,117,100,115,0,13,106,111,0,
12,107,0,4,116,121,112,101,0,0,0,0,12,108,0,3,
100,101,108,0,12,109,0,2,98,112,0,0,11,110,0,0,
0,0,0,0,0,0,36,64,26,98,103,8,30,17,1,123,
32,32,32,32,39,103,108,111,98,97,108,39,58,123,39,108,
98,112,39,58,48,44,39,110,117,100,39,58,112,114,101,102,
105,120,95,110,117,100,115,44,39,116,121,112,101,39,58,39,
103,108,111,98,97,108,115,39,44,39,98,112,39,58,50,48,
44,125,44,0,12,99,0,6,103,108,111,98,97,108,0,0,
12,103,0,3,108,98,112,0,11,104,0,0,0,0,0,0,
0,0,0,0,12,105,0,3,110,117,100,0,12,111,0,11,
112,114,101,102,105,120,95,110,117,100,115,0,13,106,111,0,
12,107,0,4,116,121,112,101,0,0,0,0,12,108,0,7,
103,108,111,98,97,108,115,0,12,109,0,2,98,112,0,0,
11,110,0,0,0,0,0,0,0,0,52,64,26,100,103,8,
30,3,1,125,32,32,32,32,39,61,39,58,123,0,0,0,
12,101,0,1,61,0,0,0,30,11,1,126,32,32,32,32,
32,32,32,32,39,108,98,112,39,58,49,48,44,39,98,112,
39,58,57,44,39,108,101,100,39,58,105,110,102,105,120,95,
108,101,100,44,0,0,0,0,12,103,0,3,108,98,112,0,
11,104,0,0,0,0,0,0,0,0,36,64,12,105,0,2,
98,112,0,0,11,106,0,0,0,0,0,0,0,0,34,64,
12,107,0,3,108,101,100,0,12,109,0,9,105,110,102,105,
120,95,108,101,100,0,0,0,13,108,109,0,26,102,103,6,
26,44,45,58,14,43,44,0,30,7,1,130,100,101,102,32,
105,95,105,110,102,105,120,40,98,112,44,108,101,100,44,42,
118,115,41,58,0,0,0,0,16,43,0,66,44,14,0,0,
30,7,1,130,100,101,102,32,105,95,105,110,102,105,120,40,
98,112,44,108,101,100,44,42,118,115,41,58,0,0,0,0,
12,1,0,8,112,97,114,115,101,46,112,121,0,0,0,0,
33,1,0,0,12,1,0,7,105,95,105,110,102,105,120,0,
34,1,0,0,28,2,0,0,9,1,0,2,28,3,0,0,
9,2,0,3,12,4,0,1,42,0,0,0,9,3,0,4,
30,16,1,131,32,32,32,32,102,111,114,32,118,32,105,110,
32,118,115,58,32,98,97,115,101,95,100,109,97,112,91,118,
93,32,61,32,123,39,108,98,112,39,58,98,112,44,39,98,
112,39,58,98,112,44,39,108,101,100,39,58,108,101,100,125,
0,0,0,0,11,5,0,0,0,0,0,0,0,0,0,0,
42,4,3,5,18,0,0,18,12,7,0,9,98,97,115,101,
95,100,109,97,112,0,0,0,13,6,7,0,12,8,0,3,
108,98,112,0,15,9,1,0,12,10,0,2,98,112,0,0,
15,11,1,0,12,12,0,3,108,101,100,0,15,13,2,0,
26,7,8,6,10,6,4,7,18,0,255,238,0,0,0,0,
12,44,0,7,105,95,105,110,102,105,120,0,14,44,43,0,
30,13,1,132,105,95,105,110,102,105,120,40,52,48,44,105,
110,102,105,120,95,108,101,100,44,39,60,39,44,39,62,39,
44,39,60,61,39,44,39,62,61,39,44,39,33,61,39,44,
39,61,61,39,41,0,0,0,12,46,0,7,105,95,105,110,
102,105,120,0,13,45,46,0,11,46,0,0,0,0,0,0,
0,0,68,64,12,54,0,9,105,110,102,105,120,95,108,101,
100,0,0,0,13,47,54,0,12,48,0,1,60,0,0,0,
12,49,0,1,62,0,0,0,12,50,0,2,60,61,0,0,
12,51,0,2,62,61,0,0,12,52,0,2,33,61,0,0,
12,53,0,2,61,61,0,0,31,44,46,8,19,44,45,44,
30,8,1,133,105,95,105,110,102,105,120,40,52,48,44,105,
110,102,105,120,95,105,115,44,39,105,115,39,44,39,105,110,
39,41,0,0,12,46,0,7,105,95,105,110,102,105,120,0,
13,45,46,0,11,46,0,0,0,0,0,0,0,0,68,64,
12,50,0,8,105,110,102,105,120,95,105,115,0,0,0,0,
13,47,50,0,12,48,0,2,105,115,0,0,12,49,0,2,
105,110,0,0,31,44,46,4,19,44,45,44,30,15,1,134,
105,95,105,110,102,105,120,40,49,48,44,105,110,102,105,120,
95,108,101,100,44,39,43,61,39,44,39,45,61,39,44,39,
42,61,39,44,39,47,61,39,44,32,39,38,61,39,44,32,
39,124,61,39,44,32,39,94,61,39,41,0,12,46,0,7,
105,95,105,110,102,105,120,0,13,45,46,0,11,46,0,0,
0,0,0,0,0,0,36,64,12,55,0,9,105,110,102,105,
120,95,108,101,100,0,0,0,13,47,55,0,12,48,0,2,
43,61,0,0,12,49,0,2,45,61,0,0,12,50,0,2,
42,61,0,0,12,51,0,2,47,61,0,0,12,52,0,2,
38,61,0,0,12,53,0,2,124,61,0,0,12,54,0,2,
94,61,0,0,31,44,46,9,19,44,45,44,30,8,1,135,
105,95,105,110,102,105,120,40,51,50,44,105,110,102,105,120,
95,108,101,100,44,39,97,110,100,39,44,39,38,39,41,0,
12,46,0,7,105,95,105,110,102,105,120,0,13,45,46,0,
11,46,0,0,0,0,0,0,0,0,64,64,12,50,0,9,
105,110,102,105,120,95,108,101,100,0,0,0,13,47,50,0,
12,48,0,3,97,110,100,0,12,49,0,1,38,0,0,0,
31,44,46,4,19,44,45,44,30,7,1,136,105,95,105,110,
102,105,120,40,51,49,44,105,110,102,105,120,95,108,101,100,
44,39,94,39,41,0,0,0,12,46,0,7,105,95,105,110,
102,105,120,0,13,45,46,0,11,46,0,0,0,0,0,0,
0,0,63,64,12,49,0,9,105,110,102,105,120,95,108,101,
100,0,0,0,13,47,49,0,12,48,0,1,94,0,0,0,
31,44,46,3,19,44,45,44,30,8,1,137,105,95,105,110,
102,105,120,40,51,48,44,105,110,102,105,120,95,108,101,100,
44,39,111,114,39,44,39,124,39,41,0,0,12,46,0,7,
105,95,105,110,102,105,120,0,13,45,46,0,11,46,0,0,
0,0,0,0,0,0,62,64,12,50,0,9,105,110,102,105,
120,95,108,101,100,0,0,0,13,47,50,0,12,48,0,2,
111,114,0,0,12,49,0,1,124,0,0,0,31,44,46,4,
19,44,45,44,30,8,1,138,105,95,105,110,102,105,120,40,
51,54,44,105,110,102,105,120,95,108,101,100,44,39,60,60,
39,44,39,62,62,39,41,0,12,46,0,7,105,95,105,110,
102,105,120,0,13,45,46,0,11,46,0,0,0,0,0,0,
0,0,66,64,12,50,0,9,105,110,102,105,120,95,108,101,
100,0,0,0,13,47,50,0,12,48,0,2,60,60,0,0,
12,49,0,2,62,62,0,0,31,44,46,4,19,44,45,44,
30,5,1,139,100,101,102,32,105,95,116,101,114,109,115,40,
42,118,115,41,58,0,0,0,16,44,0,60,44,11,0,0,
30,5,1,139,100,101,102,32,105,95,116,101,114,109,115,40,
42,118,115,41,58,0,0,0,12,1,0,8,112,97,114,115,
101,46,112,121,0,0,0,0,33,1,0,0,12,1,0,7,
105,95,116,101,114,109,115,0,34,1,0,0,12,2,0,1,
42,0,0,0,9,1,0,2,30,14,1,140,32,32,32,32,
102,111,114,32,118,32,105,110,32,118,115,58,32,98,97,115,
101,95,100,109,97,112,91,118,93,32,61,32,123,39,108,98,
112,39,58,48,44,39,110,117,100,39,58,105,116,115,101,108,
102,125,0,0,11,3,0,0,0,0,0,0,0,0,0,0,
42,2,1,3,18,0,0,20,12,5,0,9,98,97,115,101,
95,100,109,97,112,0,0,0,13,4,5,0,12,6,0,3,
108,98,112,0,11,7,0,0,0,0,0,0,0,0,0,0,
12,8,0,3,110,117,100,0,12,10,0,6,105,116,115,101,
108,102,0,0,13,9,10,0,26,5,6,4,10,4,2,5,
18,0,255,236,0,0,0,0,12,45,0,7,105,95,116,101,
114,109,115,0,14,45,44,0,30,31,1,141,105,95,116,101,
114,109,115,40,39,41,39,44,39,125,39,44,39,93,39,44,
39,59,39,44,39,58,39,44,39,110,108,39,44,39,101,108,
105,102,39,44,39,101,108,115,101,39,44,39,84,114,117,101,
39,44,39,70,97,108,115,101,39,44,39,78,111,110,101,39,
44,39,110,97,109,101,39,44,39,115,116,114,105,110,103,39,
44,39,110,117,109,98,101,114,39,44,39,105,110,100,101,110,
116,39,44,39,100,101,100,101,110,116,39,44,39,101,120,99,
101,112,116,39,41,0,0,0,12,47,0,7,105,95,116,101,
114,109,115,0,13,46,47,0,12,47,0,1,41,0,0,0,
12,48,0,1,125,0,0,0,12,49,0,1,93,0,0,0,
12,50,0,1,59,0,0,0,12,51,0,1,58,0,0,0,
12,52,0,2,110,108,0,0,12,53,0,4,101,108,105,102,
0,0,0,0,12,54,0,4,101,108,115,101,0,0,0,0,
12,55,0,4,84,114,117,101,0,0,0,0,12,56,0,5,
70,97,108,115,101,0,0,0,12,57,0,4,78,111,110,101,
0,0,0,0,12,58,0,4,110,97,109,101,0,0,0,0,
12,59,0,6,115,116,114,105,110,103,0,0,12,60,0,6,
110,117,109,98,101,114,0,0,12,61,0,6,105,110,100,101,
110,116,0,0,12,62,0,6,100,101,100,101,110,116,0,0,
12,63,0,6,101,120,99,101,112,116,0,0,31,45,47,17,
19,45,46,45,30,8,1,142,98,97,115,101,95,100,109,97,
112,91,39,110,108,39,93,91,39,118,97,108,39,93,32,61,
32,39,110,108,39,0,0,0,12,46,0,9,98,97,115,101,
95,100,109,97,112,0,0,0,13,45,46,0,12,46,0,2,
110,108,0,0,9,45,45,46,12,46,0,2,110,108,0,0,
12,47,0,3,118,97,108,0,10,45,47,46,30,4,1,144,
100,101,102,32,103,109,97,112,40,116,44,118,41,58,0,0,
16,45,0,75,44,7,0,0,30,4,1,144,100,101,102,32,
103,109,97,112,40,116,44,118,41,58,0,0,12,1,0,8,
112,97,114,115,101,46,112,121,0,0,0,0,33,1,0,0,
12,1,0,4,103,109,97,112,0,0,0,0,34,1,0,0,
28,2,0,0,9,1,0,2,28,3,0,0,9,2,0,3,
30,6,1,145,32,32,32,32,105,102,32,118,32,110,111,116,
32,105,110,32,100,109,97,112,58,0,0,0,12,4,0,4,
100,109,97,112,0,0,0,0,13,3,4,0,36,3,3,2,
11,4,0,0,0,0,0,0,0,0,0,0,23,3,3,4,
21,3,0,0,18,0,0,25,30,9,1,146,32,32,32,32,
32,32,32,32,101,114,114,111,114,40,39,117,110,107,110,111,
119,110,32,34,37,115,34,39,37,118,44,116,41,0,0,0,
12,5,0,5,101,114,114,111,114,0,0,0,13,4,5,0,
12,5,0,12,117,110,107,110,111,119,110,32,34,37,115,34,
0,0,0,0,39,5,5,2,15,6,1,0,31,3,5,2,
19,3,4,3,18,0,0,1,30,5,1,147,32,32,32,32,
114,101,116,117,114,110,32,100,109,97,112,91,118,93,0,0,
12,4,0,4,100,109,97,112,0,0,0,0,13,3,4,0,
9,3,3,2,20,3,0,0,0,0,0,0,12,46,0,4,
103,109,97,112,0,0,0,0,14,46,45,0,30,3,1,149,
100,101,102,32,100,111,40,116,41,58,0,0,16,46,0,93,
44,8,0,0,30,3,1,149,100,101,102,32,100,111,40,116,
41,58,0,0,12,1,0,8,112,97,114,115,101,46,112,121,
0,0,0,0,33,1,0,0,12,1,0,2,100,111,0,0,
34,1,0,0,28,2,0,0,9,1,0,2,30,12,1,150,
32,32,32,32,105,102,32,116,46,116,121,112,101,32,61,61,
32,39,115,121,109,98,111,108,39,58,32,114,32,61,32,103,
109,97,112,40,116,44,116,46,118,97,108,41,0,0,0,0,
12,3,0,4,116,121,112,101,0,0,0,0,9,2,1,3,
12,3,0,6,115,121,109,98,111,108,0,0,23,2,2,3,
21,2,0,0,18,0,0,13,12,5,0,4,103,109,97,112,
0,0,0,0,13,4,5,0,15,5,1,0,12,7,0,3,
118,97,108,0,9,6,1,7,31,3,5,2,19,3,4,3,
15,2,3,0,18,0,0,23,30,8,1,151,32,32,32,32,
101,108,115,101,58,32,114,32,61,32,103,109,97,112,40,116,
44,116,46,116,121,112,101,41,0,0,0,0,12,5,0,4,
103,109,97,112,0,0,0,0,13,4,5,0,15,5,1,0,
12,7,0,4,116,121,112,101,0,0,0,0,9,6,1,7,
31,3,5,2,19,3,4,3,15,2,3,0,18,0,0,1,
30,4,1,152,32,32,32,32,109,101,114,103,101,40,116,44,
114,41,0,0,12,5,0,5,109,101,114,103,101,0,0,0,
13,4,5,0,15,5,1,0,15,6,2,0,31,3,5,2,
19,3,4,3,30,4,1,153,32,32,32,32,114,101,116,117,
114,110,32,116,0,0,0,0,20,1,0,0,0,0,0,0,
12,47,0,2,100,111,0,0,14,47,46,0,30,5,1,154,
100,101,102,32,100,111,95,109,111,100,117,108,101,40,41,58,
0,0,0,0,16,47,0,132,44,10,0,0,30,5,1,154,
100,101,102,32,100,111,95,109,111,100,117,108,101,40,41,58,
0,0,0,0,12,1,0,8,112,97,114,115,101,46,112,121,
0,0,0,0,33,1,0,0,12,1,0,9,100,111,95,109,
111,100,117,108,101,0,0,0,34,1,0,0,30,5,1,155,
32,32,32,32,116,111,107,32,61,32,80,46,116,111,107,101,
110,0,0,0,12,3,0,1,80,0,0,0,13,2,3,0,
12,3,0,5,116,111,107,101,110,0,0,0,9,2,2,3,
15,1,2,0,30,4,1,156,32,32,32,32,105,116,101,109,
115,32,61,32,91,93,0,0,27,3,0,0,15,2,3,0,
30,5,1,157,32,32,32,32,105,98,108,111,99,107,40,105,
116,101,109,115,41,0,0,0,12,5,0,6,105,98,108,111,
99,107,0,0,13,4,5,0,15,5,2,0,31,3,5,1,
19,3,4,3,30,5,1,158,32,32,32,32,97,100,118,97,
110,99,101,40,39,101,111,102,39,41,0,0,12,5,0,7,
97,100,118,97,110,99,101,0,13,4,5,0,12,5,0,3,
101,111,102,0,31,3,5,1,19,3,4,3,30,6,1,159,
32,32,32,32,105,102,32,108,101,110,40,105,116,101,109,115,
41,32,62,32,49,58,0,0,11,3,0,0,0,0,0,0,
0,0,240,63,12,6,0,3,108,101,110,0,13,5,6,0,
15,6,2,0,31,4,6,1,19,4,5,4,25,3,3,4,
21,3,0,0,18,0,0,34,30,14,1,160,32,32,32,32,
32,32,32,32,114,101,116,117,114,110,32,84,111,107,101,110,
40,116,111,107,46,112,111,115,44,39,115,116,97,116,101,109,
101,110,116,115,39,44,39,59,39,44,105,116,101,109,115,41,
0,0,0,0,12,5,0,5,84,111,107,101,110,0,0,0,
13,4,5,0,12,9,0,3,112,111,115,0,9,5,1,9,
12,6,0,10,115,116,97,116,101,109,101,110,116,115,0,0,
12,7,0,1,59,0,0,0,15,8,2,0,31,3,5,4,
19,3,4,3,20,3,0,0,18,0,0,1,30,6,1,161,
32,32,32,32,114,101,116,117,114,110,32,105,116,101,109,115,
46,112,111,112,40,41,0,0,12,5,0,3,112,111,112,0,
9,4,2,5,31,3,0,0,19,3,4,3,20,3,0,0,
0,0,0,0,12,48,0,9,100,111,95,109,111,100,117,108,
101,0,0,0,14,48,47,0,30,7,1,163,100,101,102,32,
112,97,114,115,101,40,115,44,116,111,107,101,110,115,44,119,
114,97,112,61,48,41,58,0,16,48,0,113,44,9,0,0,
30,7,1,163,100,101,102,32,112,97,114,115,101,40,115,44,
116,111,107,101,110,115,44,119,114,97,112,61,48,41,58,0,
12,1,0,8,112,97,114,115,101,46,112,121,0,0,0,0,
33,1,0,0,12,1,0,5,112,97,114,115,101,0,0,0,
34,1,0,0,28,2,0,0,9,1,0,2,28,3,0,0,
9,2,0,3,11,3,0,0,0,0,0,0,0,0,0,0,
28,4,0,0,32,3,0,4,30,4,1,164,32,32,32,32,
103,108,111,98,97,108,32,80,0,0,0,0,30,7,1,165,
32,32,32,32,115,32,61,32,116,111,107,101,110,105,122,101,
46,99,108,101,97,110,40,115,41,0,0,0,12,6,0,8,
116,111,107,101,110,105,122,101,0,0,0,0,13,5,6,0,
12,6,0,5,99,108,101,97,110,0,0,0,9,5,5,6,
15,6,1,0,31,4,6,1,19,4,5,4,15,1,4,0,
30,8,1,166,32,32,32,32,80,61,80,68,97,116,97,40,
115,44,116,111,107,101,110,115,41,59,32,80,46,105,110,105,
116,40,41,0,12,4,0,1,80,0,0,0,12,7,0,5,
80,68,97,116,97,0,0,0,13,6,7,0,15,7,1,0,
15,8,2,0,31,5,7,2,19,5,6,5,14,4,5,0,
12,6,0,1,80,0,0,0,13,5,6,0,12,6,0,4,
105,110,105,116,0,0,0,0,9,5,5,6,31,4,0,0,
19,4,5,4,30,5,1,167,32,32,32,32,114,32,61,32,
100,111,95,109,111,100,117,108,101,40,41,0,12,7,0,9,
100,111,95,109,111,100,117,108,101,0,0,0,13,6,7,0,
31,5,0,0,19,5,6,5,15,4,5,0,30,4,1,168,
32,32,32,32,80,32,61,32,78,111,110,101,0,0,0,0,
12,5,0,1,80,0,0,0,28,6,0,0,14,5,6,0,
30,4,1,169,32,32,32,32,114,101,116,117,114,110,32,114,
0,0,0,0,20,4,0,0,0,0,0,0,12,49,0,5,
112,97,114,115,101,0,0,0,14,49,48,0,0,0,0,0,
};
unsigned char tp_encode[] = {
44,104,0,0,30,6,0,1,105,109,112,111,114,116,32,116,
111,107,101,110,105,122,101,44,32,115,121,115,0,0,0,0,
12,0,0,9,101,110,99,111,100,101,46,112,121,0,0,0,
33,0,0,0,12,0,0,1,63,0,0,0,34,0,0,0,
12,2,0,6,105,109,112,111,114,116,0,0,13,1,2,0,
12,2,0,8,116,111,107,101,110,105,122,101,0,0,0,0,
31,0,2,1,19,0,1,0,12,1,0,8,116,111,107,101,
110,105,122,101,0,0,0,0,14,1,0,0,12,2,0,6,
105,109,112,111,114,116,0,0,13,1,2,0,12,2,0,3,
115,121,115,0,31,0,2,1,19,0,1,0,12,1,0,3,
115,121,115,0,14,1,0,0,30,7,0,2,102,114,111,109,
32,116,111,107,101,110,105,122,101,32,105,109,112,111,114,116,
32,84,111,107,101,110,0,0,12,2,0,6,105,109,112,111,
114,116,0,0,13,1,2,0,12,2,0,8,116,111,107,101,
110,105,122,101,0,0,0,0,31,0,2,1,19,0,1,0,
12,2,0,8,95,95,100,105,99,116,95,95,0,0,0,0,
13,1,2,0,12,3,0,5,84,111,107,101,110,0,0,0,
9,2,0,3,12,0,0,5,84,111,107,101,110,0,0,0,
10,1,0,2,30,8,0,3,105,102,32,110,111,116,32,34,
116,105,110,121,112,121,34,32,105,110,32,115,121,115,46,118,
101,114,115,105,111,110,58,0,12,2,0,3,115,121,115,0,
13,1,2,0,12,2,0,7,118,101,114,115,105,111,110,0,
9,1,1,2,12,2,0,6,116,105,110,121,112,121,0,0,
36,1,1,2,47,0,1,0,21,0,0,0,18,0,0,30,
30,6,0,4,32,32,32,32,102,114,111,109,32,98,111,111,
116,32,105,109,112,111,114,116,32,42,0,0,12,2,0,6,
105,109,112,111,114,116,0,0,13,1,2,0,12,2,0,4,
98,111,111,116,0,0,0,0,31,0,2,1,19,0,1,0,
12,3,0,5,109,101,114,103,101,0,0,0,13,2,3,0,
12,5,0,8,95,95,100,105,99,116,95,95,0,0,0,0,
13,3,5,0,15,4,0,0,31,1,3,2,19,1,2,1,
18,0,0,1,30,94,0,6,69,79,70,44,65,68,68,44,
83,85,66,44,77,85,76,44,68,73,86,44,80,79,87,44,
66,73,84,65,78,68,44,66,73,84,79,82,44,67,77,80,
44,71,69,84,44,83,69,84,44,78,85,77,66,69,82,44,
83,84,82,73,78,71,44,71,71,69,84,44,71,83,69,84,
44,77,79,86,69,44,68,69,70,44,80,65,83,83,44,74,
85,77,80,44,67,65,76,76,44,82,69,84,85,82,78,44,
73,70,44,68,69,66,85,71,44,69,81,44,76,69,44,76,
84,44,68,73,67,84,44,76,73,83,84,44,78,79,78,69,
44,76,69,78,44,80,79,83,44,80,65,82,65,77,83,44,
73,71,69,84,44,70,73,76,69,44,78,65,77,69,44,78,
69,44,72,65,83,44,82,65,73,83,69,44,83,69,84,74,
77,80,44,77,79,68,44,76,83,72,44,82,83,72,44,73,
84,69,82,44,68,69,76,44,82,69,71,83,44,66,73,84,
88,79,82,44,73,70,78,44,78,79,84,44,66,73,84,78,
79,84,32,61,32,48,44,49,44,50,44,51,44,52,44,53,
44,54,44,55,44,56,44,57,44,49,48,44,49,49,44,49,
50,44,49,51,44,49,52,44,49,53,44,49,54,44,49,55,
44,49,56,44,49,57,44,50,48,44,50,49,44,50,50,44,
50,51,44,50,52,44,50,53,44,50,54,44,50,55,44,50,
56,44,50,57,44,51,48,44,51,49,44,51,50,44,51,51,
44,51,52,44,51,53,44,51,54,44,51,55,44,51,56,44,
51,57,44,52,48,44,52,49,44,52,50,44,52,51,44,52,
52,44,52,53,44,52,54,44,52,55,44,52,56,0,0,0,
11,1,0,0,0,0,0,0,0,0,0,0,15,0,1,0,
11,2,0,0,0,0,0,0,0,0,240,63,15,1,2,0,
11,3,0,0,0,0,0,0,0,0,0,64,15,2,3,0,
11,4,0,0,0,0,0,0,0,0,8,64,15,3,4,0,
11,5,0,0,0,0,0,0,0,0,16,64,15,4,5,0,
11,6,0,0,0,0,0,0,0,0,20,64,15,5,6,0,
11,7,0,0,0,0,0,0,0,0,24,64,15,6,7,0,
11,8,0,0,0,0,0,0,0,0,28,64,15,7,8,0,
11,9,0,0,0,0,0,0,0,0,32,64,15,8,9,0,
11,10,0,0,0,0,0,0,0,0,34,64,15,9,10,0,
11,11,0,0,0,0,0,0,0,0,36,64,15,10,11,0,
11,12,0,0,0,0,0,0,0,0,38,64,15,11,12,0,
11,13,0,0,0,0,0,0,0,0,40,64,15,12,13,0,
11,14,0,0,0,0,0,0,0,0,42,64,15,13,14,0,
11,15,0,0,0,0,0,0,0,0,44,64,15,14,15,0,
11,16,0,0,0,0,0,0,0,0,46,64,15,15,16,0,
11,17,0,0,0,0,0,0,0,0,48,64,15,16,17,0,
11,18,0,0,0,0,0,0,0,0,49,64,15,17,18,0,
11,19,0,0,0,0,0,0,0,0,50,64,15,18,19,0,
11,20,0,0,0,0,0,0,0,0,51,64,15,19,20,0,
11,21,0,0,0,0,0,0,0,0,52,64,15,20,21,0,
11,22,0,0,0,0,0,0,0,0,53,64,15,21,22,0,
11,23,0,0,0,0,0,0,0,0,54,64,15,22,23,0,
11,24,0,0,0,0,0,0,0,0,55,64,15,23,24,0,
11,25,0,0,0,0,0,0,0,0,56,64,15,24,25,0,
11,26,0,0,0,0,0,0,0,0,57,64,15,25,26,0,
11,27,0,0,0,0,0,0,0,0,58,64,15,26,27,0,
11,28,0,0,0,0,0,0,0,0,59,64,15,27,28,0,
11,29,0,0,0,0,0,0,0,0,60,64,15,28,29,0,
11,30,0,0,0,0,0,0,0,0,61,64,15,29,30,0,
11,31,0,0,0,0,0,0,0,0,62,64,15,30,31,0,
11,32,0,0,0,0,0,0,0,0,63,64,15,31,32,0,
11,33,0,0,0,0,0,0,0,0,64,64,15,32,33,0,
11,34,0,0,0,0,0,0,0,128,64,64,15,33,34,0,
11,35,0,0,0,0,0,0,0,0,65,64,15,34,35,0,
11,36,0,0,0,0,0,0,0,128,65,64,15,35,36,0,
11,37,0,0,0,0,0,0,0,0,66,64,15,36,37,0,
11,38,0,0,0,0,0,0,0,128,66,64,15,37,38,0,
11,39,0,0,0,0,0,0,0,0,67,64,15,38,39,0,
11,40,0,0,0,0,0,0,0,128,67,64,15,39,40,0,
11,41,0,0,0,0,0,0,0,0,68,64,15,40,41,0,
11,42,0,0,0,0,0,0,0,128,68,64,15,41,42,0,
11,43,0,0,0,0,0,0,0,0,69,64,15,42,43,0,
11,44,0,0,0,0,0,0,0,128,69,64,15,43,44,0,
11,45,0,0,0,0,0,0,0,0,70,64,15,44,45,0,
11,46,0,0,0,0,0,0,0,128,70,64,15,45,46,0,
11,47,0,0,0,0,0,0,0,0,71,64,15,46,47,0,
11,48,0,0,0,0,0,0,0,128,71,64,15,47,48,0,
11,49,0,0,0,0,0,0,0,0,72,64,15,48,49,0,
12,49,0,3,69,79,70,0,14,49,0,0,12,0,0,3,
65,68,68,0,14,0,1,0,12,0,0,3,83,85,66,0,
14,0,2,0,12,0,0,3,77,85,76,0,14,0,3,0,
12,0,0,3,68,73,86,0,14,0,4,0,12,0,0,3,
80,79,87,0,14,0,5,0,12,0,0,6,66,73,84,65,
78,68,0,0,14,0,6,0,12,0,0,5,66,73,84,79,
82,0,0,0,14,0,7,0,12,0,0,3,67,77,80,0,
14,0,8,0,12,0,0,3,71,69,84,0,14,0,9,0,
12,0,0,3,83,69,84,0,14,0,10,0,12,0,0,6,
78,85,77,66,69,82,0,0,14,0,11,0,12,0,0,6,
83,84,82,73,78,71,0,0,14,0,12,0,12,0,0,4,
71,71,69,84,0,0,0,0,14,0,13,0,12,0,0,4,
71,83,69,84,0,0,0,0,14,0,14,0,12,0,0,4,
77,79,86,69,0,0,0,0,14,0,15,0,12,0,0,3,
68,69,70,0,14,0,16,0,12,0,0,4,80,65,83,83,
0,0,0,0,14,0,17,0,12,0,0,4,74,85,77,80,
0,0,0,0,14,0,18,0,12,0,0,4,67,65,76,76,
0,0,0,0,14,0,19,0,12,0,0,6,82,69,84,85,
82,78,0,0,14,0,20,0,12,0,0,2,73,70,0,0,
14,0,21,0,12,0,0,5,68,69,66,85,71,0,0,0,
14,0,22,0,12,0,0,2,69,81,0,0,14,0,23,0,
12,0,0,2,76,69,0,0,14,0,24,0,12,0,0,2,
76,84,0,0,14,0,25,0,12,0,0,4,68,73,67,84,
0,0,0,0,14,0,26,0,12,0,0,4,76,73,83,84,
0,0,0,0,14,0,27,0,12,0,0,4,78,79,78,69,
0,0,0,0,14,0,28,0,12,0,0,3,76,69,78,0,
14,0,29,0,12,0,0,3,80,79,83,0,14,0,30,0,
12,0,0,6,80,65,82,65,77,83,0,0,14,0,31,0,
12,0,0,4,73,71,69,84,0,0,0,0,14,0,32,0,
12,0,0,4,70,73,76,69,0,0,0,0,14,0,33,0,
12,0,0,4,78,65,77,69,0,0,0,0,14,0,34,0,
12,0,0,2,78,69,0,0,14,0,35,0,12,0,0,3,
72,65,83,0,14,0,36,0,12,0,0,5,82,65,73,83,
69,0,0,0,14,0,37,0,12,0,0,6,83,69,84,74,
77,80,0,0,14,0,38,0,12,0,0,3,77,79,68,0,
14,0,39,0,12,0,0,3,76,83,72,0,14,0,40,0,
12,0,0,3,82,83,72,0,14,0,41,0,12,0,0,4,
73,84,69,82,0,0,0,0,14,0,42,0,12,0,0,3,
68,69,76,0,14,0,43,0,12,0,0,4,82,69,71,83,
0,0,0,0,14,0,44,0,12,0,0,6,66,73,84,88,
79,82,0,0,14,0,45,0,12,0,0,3,73,70,78,0,
14,0,46,0,12,0,0,3,78,79,84,0,14,0,47,0,
12,0,0,6,66,73,84,78,79,84,0,0,14,0,48,0,
30,4,0,8,99,108,97,115,115,32,68,83,116,97,116,101,
58,0,0,0,26,0,0,0,12,1,0,6,68,83,116,97,
116,101,0,0,14,1,0,0,12,3,0,7,115,101,116,109,
101,116,97,0,13,2,3,0,15,3,0,0,12,5,0,6,
111,98,106,101,99,116,0,0,13,4,5,0,31,1,3,2,
19,1,2,1,16,1,0,166,44,11,0,0,30,9,0,9,
32,32,32,32,100,101,102,32,95,95,105,110,105,116,95,95,
40,115,101,108,102,44,99,111,100,101,44,102,110,97,109,101,
41,58,0,0,12,1,0,9,101,110,99,111,100,101,46,112,
121,0,0,0,33,1,0,0,12,1,0,8,95,95,105,110,
105,116,95,95,0,0,0,0,34,1,0,0,28,2,0,0,
9,1,0,2,28,3,0,0,9,2,0,3,28,4,0,0,
9,3,0,4,30,11,0,10,32,32,32,32,32,32,32,32,
115,101,108,102,46,99,111,100,101,44,32,115,101,108,102,46,
102,110,97,109,101,32,61,32,99,111,100,101,44,102,110,97,
109,101,0,0,15,4,2,0,15,5,3,0,12,6,0,4,
99,111,100,101,0,0,0,0,10,1,6,4,12,4,0,5,
102,110,97,109,101,0,0,0,10,1,4,5,30,11,0,11,
32,32,32,32,32,32,32,32,115,101,108,102,46,108,105,110,
101,115,32,61,32,115,101,108,102,46,99,111,100,101,46,115,
112,108,105,116,40,39,92,110,39,41,0,0,12,6,0,4,
99,111,100,101,0,0,0,0,9,5,1,6,12,6,0,5,
115,112,108,105,116,0,0,0,9,5,5,6,12,6,0,1,
10,0,0,0,31,4,6,1,19,4,5,4,12,5,0,5,
108,105,110,101,115,0,0,0,10,1,5,4,30,27,0,13,
32,32,32,32,32,32,32,32,115,101,108,102,46,115,116,97,
99,107,44,115,101,108,102,46,111,117,116,44,115,101,108,102,
46,95,115,99,111,112,101,105,44,115,101,108,102,46,116,115,
116,97,99,107,44,115,101,108,102,46,95,116,97,103,105,44,
115,101,108,102,46,100,97,116,97,32,61,32,91,93,44,91,
40,39,116,97,103,39,44,39,69,79,70,39,41,93,44,48,
44,91,93,44,48,44,123,125,0,0,0,0,27,5,0,0,
15,4,5,0,12,8,0,3,116,97,103,0,12,9,0,3,
69,79,70,0,27,7,8,2,27,6,7,1,15,5,6,0,
11,7,0,0,0,0,0,0,0,0,0,0,15,6,7,0,
27,8,0,0,15,7,8,0,11,9,0,0,0,0,0,0,
0,0,0,0,15,8,9,0,26,10,0,0,15,9,10,0,
12,10,0,5,115,116,97,99,107,0,0,0,10,1,10,4,
12,4,0,3,111,117,116,0,10,1,4,5,12,4,0,7,
95,115,99,111,112,101,105,0,10,1,4,6,12,4,0,6,
116,115,116,97,99,107,0,0,10,1,4,7,12,4,0,5,
95,116,97,103,105,0,0,0,10,1,4,8,12,4,0,4,
100,97,116,97,0,0,0,0,10,1,4,9,30,7,0,14,
32,32,32,32,32,32,32,32,115,101,108,102,46,101,114,114,
111,114,32,61,32,70,97,108,115,101,0,0,11,4,0,0,
0,0,0,0,0,0,0,0,12,5,0,5,101,114,114,111,
114,0,0,0,10,1,5,4,0,0,0,0,12,2,0,8,
95,95,105,110,105,116,95,95,0,0,0,0,10,0,2,1,
16,2,1,92,44,19,0,0,30,8,0,15,32,32,32,32,
100,101,102,32,98,101,103,105,110,40,115,101,108,102,44,103,
98,108,61,70,97,108,115,101,41,58,0,0,12,1,0,9,
101,110,99,111,100,101,46,112,121,0,0,0,33,1,0,0,
12,1,0,5,98,101,103,105,110,0,0,0,34,1,0,0,
28,2,0,0,9,1,0,2,11,2,0,0,0,0,0,0,
0,0,0,0,28,3,0,0,32,2,0,3,30,46,0,16,
32,32,32,32,32,32,32,32,105,102,32,108,101,110,40,115,
101,108,102,46,115,116,97,99,107,41,58,32,115,101,108,102,
46,115,116,97,99,107,46,97,112,112,101,110,100,40,40,115,
101,108,102,46,118,97,114,115,44,115,101,108,102,46,114,50,
110,44,115,101,108,102,46,110,50,114,44,115,101,108,102,46,
95,116,109,112,105,44,115,101,108,102,46,109,114,101,103,44,
115,101,108,102,46,115,110,117,109,44,115,101,108,102,46,95,
103,108,111,98,97,108,115,44,115,101,108,102,46,108,105,110,
101,110,111,44,115,101,108,102,46,103,108,111,98,97,108,115,
44,115,101,108,102,46,114,103,108,111,98,97,108,115,44,115,
101,108,102,46,99,114,101,103,115,44,115,101,108,102,46,116,
109,112,99,41,41,0,0,0,12,5,0,3,108,101,110,0,
13,4,5,0,12,6,0,5,115,116,97,99,107,0,0,0,
9,5,1,6,31,3,5,1,19,3,4,3,21,3,0,0,
18,0,0,61,12,5,0,5,115,116,97,99,107,0,0,0,
9,4,1,5,12,5,0,6,97,112,112,101,110,100,0,0,
9,4,4,5,12,18,0,4,118,97,114,115,0,0,0,0,
9,6,1,18,12,18,0,3,114,50,110,0,9,7,1,18,
12,18,0,3,110,50,114,0,9,8,1,18,12,18,0,5,
95,116,109,112,105,0,0,0,9,9,1,18,12,18,0,4,
109,114,101,103,0,0,0,0,9,10,1,18,12,18,0,4,
115,110,117,109,0,0,0,0,9,11,1,18,12,18,0,8,
95,103,108,111,98,97,108,115,0,0,0,0,9,12,1,18,
12,18,0,6,108,105,110,101,110,111,0,0,9,13,1,18,
12,18,0,7,103,108,111,98,97,108,115,0,9,14,1,18,
12,18,0,8,114,103,108,111,98,97,108,115,0,0,0,0,
9,15,1,18,12,18,0,5,99,114,101,103,115,0,0,0,
9,16,1,18,12,18,0,4,116,109,112,99,0,0,0,0,
9,17,1,18,27,5,6,12,31,3,5,1,19,3,4,3,
18,0,0,24,30,10,0,17,32,32,32,32,32,32,32,32,
101,108,115,101,58,32,115,101,108,102,46,115,116,97,99,107,
46,97,112,112,101,110,100,40,78,111,110,101,41,0,0,0,
12,5,0,5,115,116,97,99,107,0,0,0,9,4,1,5,
12,5,0,6,97,112,112,101,110,100,0,0,9,4,4,5,
28,5,0,0,31,3,5,1,19,3,4,3,18,0,0,1,
30,50,0,18,32,32,32,32,32,32,32,32,115,101,108,102,
46,118,97,114,115,44,115,101,108,102,46,114,50,110,44,115,
101,108,102,46,110,50,114,44,115,101,108,102,46,95,116,109,
112,105,44,115,101,108,102,46,109,114,101,103,44,115,101,108,
102,46,115,110,117,109,44,115,101,108,102,46,95,103,108,111,
98,97,108,115,44,115,101,108,102,46,108,105,110,101,110,111,
44,115,101,108,102,46,103,108,111,98,97,108,115,44,115,101,
108,102,46,114,103,108,111,98,97,108,115,44,115,101,108,102,
46,99,114,101,103,115,44,115,101,108,102,46,116,109,112,99,
32,61,32,91,93,44,123,125,44,123,125,44,48,44,48,44,
115,116,114,40,115,101,108,102,46,95,115,99,111,112,101,105,
41,44,103,98,108,44,45,49,44,91,93,44,91,93,44,91,
39,114,101,103,115,39,93,44,48,0,0,0,27,4,0,0,
15,3,4,0,26,5,0,0,15,4,5,0,26,6,0,0,
15,5,6,0,11,7,0,0,0,0,0,0,0,0,0,0,
15,6,7,0,11,8,0,0,0,0,0,0,0,0,0,0,
15,7,8,0,12,11,0,3,115,116,114,0,13,10,11,0,
12,12,0,7,95,115,99,111,112,101,105,0,9,11,1,12,
31,9,11,1,19,9,10,9,15,8,9,0,15,9,2,0,
11,11,0,0,0,0,0,0,0,0,240,191,15,10,11,0,
27,12,0,0,15,11,12,0,27,13,0,0,15,12,13,0,
12,15,0,4,114,101,103,115,0,0,0,0,27,14,15,1,
15,13,14,0,11,15,0,0,0,0,0,0,0,0,0,0,
15,14,15,0,12,15,0,4,118,97,114,115,0,0,0,0,
10,1,15,3,12,3,0,3,114,50,110,0,10,1,3,4,
12,3,0,3,110,50,114,0,10,1,3,5,12,3,0,5,
95,116,109,112,105,0,0,0,10,1,3,6,12,3,0,4,
109,114,101,103,0,0,0,0,10,1,3,7,12,3,0,4,
115,110,117,109,0,0,0,0,10,1,3,8,12,3,0,8,
95,103,108,111,98,97,108,115,0,0,0,0,10,1,3,9,
12,3,0,6,108,105,110,101,110,111,0,0,10,1,3,10,
12,3,0,7,103,108,111,98,97,108,115,0,10,1,3,11,
12,3,0,8,114,103,108,111,98,97,108,115,0,0,0,0,
10,1,3,12,12,3,0,5,99,114,101,103,115,0,0,0,
10,1,3,13,12,3,0,4,116,109,112,99,0,0,0,0,
10,1,3,14,30,7,0,19,32,32,32,32,32,32,32,32,
115,101,108,102,46,95,115,99,111,112,101,105,32,43,61,32,
49,0,0,0,12,4,0,7,95,115,99,111,112,101,105,0,
9,3,1,4,11,4,0,0,0,0,0,0,0,0,240,63,
1,3,3,4,12,4,0,7,95,115,99,111,112,101,105,0,
10,1,4,3,30,7,0,20,32,32,32,32,32,32,32,32,
105,110,115,101,114,116,40,115,101,108,102,46,99,114,101,103,
115,41,0,0,12,5,0,6,105,110,115,101,114,116,0,0,
13,4,5,0,12,6,0,5,99,114,101,103,115,0,0,0,
9,5,1,6,31,3,5,1,19,3,4,3,0,0,0,0,
12,3,0,5,98,101,103,105,110,0,0,0,10,0,3,2,
16,3,1,50,44,7,0,0,30,5,0,21,32,32,32,32,
100,101,102,32,101,110,100,40,115,101,108,102,41,58,0,0,
12,1,0,9,101,110,99,111,100,101,46,112,121,0,0,0,
33,1,0,0,12,1,0,3,101,110,100,0,34,1,0,0,
28,2,0,0,9,1,0,2,30,10,0,22,32,32,32,32,
32,32,32,32,115,101,108,102,46,99,114,101,103,115,46,97,
112,112,101,110,100,40,115,101,108,102,46,109,114,101,103,41,
0,0,0,0,12,4,0,5,99,114,101,103,115,0,0,0,
9,3,1,4,12,4,0,6,97,112,112,101,110,100,0,0,
9,3,3,4,12,5,0,4,109,114,101,103,0,0,0,0,
9,4,1,5,31,2,4,1,19,2,3,2,30,5,0,23,
32,32,32,32,32,32,32,32,99,111,100,101,40,69,79,70,
41,0,0,0,12,4,0,4,99,111,100,101,0,0,0,0,
13,3,4,0,12,5,0,3,69,79,70,0,13,4,5,0,
31,2,4,1,19,2,3,2,30,7,0,29,32,32,32,32,
32,32,32,32,105,102,32,115,101,108,102,46,116,109,112,99,
32,33,61,32,48,58,0,0,12,3,0,4,116,109,112,99,
0,0,0,0,9,2,1,3,11,3,0,0,0,0,0,0,
0,0,0,0,35,2,2,3,21,2,0,0,18,0,0,39,
30,17,0,30,32,32,32,32,32,32,32,32,32,32,32,32,
112,114,105,110,116,40,34,87,97,114,110,105,110,103,58,92,
110,101,110,99,111,100,101,46,112,121,32,99,111,110,116,97,
105,110,115,32,97,32,114,101,103,105,115,116,101,114,32,108,
101,97,107,92,110,34,41,0,12,4,0,5,112,114,105,110,
116,0,0,0,13,3,4,0,12,4,0,44,87,97,114,110,
105,110,103,58,10,101,110,99,111,100,101,46,112,121,32,99,
111,110,116,97,105,110,115,32,97,32,114,101,103,105,115,116,
101,114,32,108,101,97,107,10,0,0,0,0,31,2,4,1,
19,2,3,2,18,0,0,1,30,8,0,32,32,32,32,32,
32,32,32,32,105,102,32,108,101,110,40,115,101,108,102,46,
115,116,97,99,107,41,32,62,32,49,58,0,11,2,0,0,
0,0,0,0,0,0,240,63,12,5,0,3,108,101,110,0,
13,4,5,0,12,6,0,5,115,116,97,99,107,0,0,0,
9,5,1,6,31,3,5,1,19,3,4,3,25,2,2,3,
21,2,0,0,18,0,0,149,30,41,0,33,32,32,32,32,
32,32,32,32,32,32,32,32,115,101,108,102,46,118,97,114,
115,44,115,101,108,102,46,114,50,110,44,115,101,108,102,46,
110,50,114,44,115,101,108,102,46,95,116,109,112,105,44,115,
101,108,102,46,109,114,101,103,44,115,101,108,102,46,115,110,
117,109,44,115,101,108,102,46,95,103,108,111,98,97,108,115,
44,115,101,108,102,46,108,105,110,101,110,111,44,115,101,108,
102,46,103,108,111,98,97,108,115,44,115,101,108,102,46,114,
103,108,111,98,97,108,115,44,115,101,108,102,46,99,114,101,
103,115,44,115,101,108,102,46,116,109,112,99,32,61,32,115,
101,108,102,46,115,116,97,99,107,46,112,111,112,40,41,0,
12,4,0,5,115,116,97,99,107,0,0,0,9,3,1,4,
12,4,0,3,112,111,112,0,9,3,3,4,31,2,0,0,
19,2,3,2,11,4,0,0,0,0,0,0,0,0,0,0,
9,3,2,4,12,4,0,4,118,97,114,115,0,0,0,0,
10,1,4,3,11,4,0,0,0,0,0,0,0,0,240,63,
9,3,2,4,12,4,0,3,114,50,110,0,10,1,4,3,
11,4,0,0,0,0,0,0,0,0,0,64,9,3,2,4,
12,4,0,3,110,50,114,0,10,1,4,3,11,4,0,0,
0,0,0,0,0,0,8,64,9,3,2,4,12,4,0,5,
95,116,109,112,105,0,0,0,10,1,4,3,11,4,0,0,
0,0,0,0,0,0,16,64,9,3,2,4,12,4,0,4,
109,114,101,103,0,0,0,0,10,1,4,3,11,4,0,0,
0,0,0,0,0,0,20,64,9,3,2,4,12,4,0,4,
115,110,117,109,0,0,0,0,10,1,4,3,11,4,0,0,
0,0,0,0,0,0,24,64,9,3,2,4,12,4,0,8,
95,103,108,111,98,97,108,115,0,0,0,0,10,1,4,3,
11,4,0,0,0,0,0,0,0,0,28,64,9,3,2,4,
12,4,0,6,108,105,110,101,110,111,0,0,10,1,4,3,
11,4,0,0,0,0,0,0,0,0,32,64,9,3,2,4,
12,4,0,7,103,108,111,98,97,108,115,0,10,1,4,3,
11,4,0,0,0,0,0,0,0,0,34,64,9,3,2,4,
12,4,0,8,114,103,108,111,98,97,108,115,0,0,0,0,
10,1,4,3,11,4,0,0,0,0,0,0,0,0,36,64,
9,3,2,4,12,4,0,5,99,114,101,103,115,0,0,0,
10,1,4,3,11,4,0,0,0,0,0,0,0,0,38,64,
9,3,2,4,12,4,0,4,116,109,112,99,0,0,0,0,
10,1,4,3,18,0,0,20,30,8,0,34,32,32,32,32,
32,32,32,32,101,108,115,101,58,32,115,101,108,102,46,115,
116,97,99,107,46,112,111,112,40,41,0,0,12,4,0,5,
115,116,97,99,107,0,0,0,9,3,1,4,12,4,0,3,
112,111,112,0,9,3,3,4,31,2,0,0,19,2,3,2,
18,0,0,1,0,0,0,0,12,4,0,3,101,110,100,0,
10,0,4,3,30,8,0,37,100,101,102,32,105,110,115,101,
114,116,40,118,41,58,32,68,46,111,117,116,46,97,112,112,
101,110,100,40,118,41,0,0,16,0,0,36,44,5,0,0,
30,8,0,37,100,101,102,32,105,110,115,101,114,116,40,118,
41,58,32,68,46,111,117,116,46,97,112,112,101,110,100,40,
118,41,0,0,12,1,0,9,101,110,99,111,100,101,46,112,
121,0,0,0,33,1,0,0,12,1,0,6,105,110,115,101,
114,116,0,0,34,1,0,0,28,2,0,0,9,1,0,2,
12,4,0,1,68,0,0,0,13,3,4,0,12,4,0,3,
111,117,116,0,9,3,3,4,12,4,0,6,97,112,112,101,
110,100,0,0,9,3,3,4,15,4,1,0,31,2,4,1,
19,2,3,2,0,0,0,0,12,4,0,6,105,110,115,101,
114,116,0,0,14,4,0,0,30,4,0,38,100,101,102,32,
119,114,105,116,101,40,118,41,58,0,0,0,16,4,0,120,
44,14,0,0,30,4,0,38,100,101,102,32,119,114,105,116,
101,40,118,41,58,0,0,0,12,1,0,9,101,110,99,111,
100,101,46,112,121,0,0,0,33,1,0,0,12,1,0,5,
119,114,105,116,101,0,0,0,34,1,0,0,28,2,0,0,
9,1,0,2,30,7,0,39,32,32,32,32,105,102,32,105,
115,116,121,112,101,40,118,44,39,108,105,115,116,39,41,58,
0,0,0,0,12,4,0,6,105,115,116,121,112,101,0,0,
13,3,4,0,15,4,1,0,12,5,0,4,108,105,115,116,
0,0,0,0,31,2,4,2,19,2,3,2,21,2,0,0,
18,0,0,22,30,5,0,40,32,32,32,32,32,32,32,32,
105,110,115,101,114,116,40,118,41,0,0,0,12,4,0,6,
105,110,115,101,114,116,0,0,13,3,4,0,15,4,1,0,
31,2,4,1,19,2,3,2,30,4,0,41,32,32,32,32,
32,32,32,32,114,101,116,117,114,110,0,0,28,2,0,0,
20,2,0,0,18,0,0,1,30,8,0,42,32,32,32,32,
102,111,114,32,110,32,105,110,32,114,97,110,103,101,40,48,
44,108,101,110,40,118,41,44,52,41,58,0,12,5,0,5,
114,97,110,103,101,0,0,0,13,4,5,0,11,5,0,0,
0,0,0,0,0,0,0,0,12,9,0,3,108,101,110,0,
13,8,9,0,15,9,1,0,31,6,9,1,19,6,8,6,
11,7,0,0,0,0,0,0,0,0,16,64,31,3,5,3,
19,3,4,3,11,4,0,0,0,0,0,0,0,0,0,0,
42,2,3,4,18,0,0,29,30,9,0,43,32,32,32,32,
32,32,32,32,105,110,115,101,114,116,40,40,39,100,97,116,
97,39,44,118,91,110,58,110,43,52,93,41,41,0,0,0,
12,7,0,6,105,110,115,101,114,116,0,0,13,6,7,0,
12,8,0,4,100,97,116,97,0,0,0,0,15,11,2,0,
11,13,0,0,0,0,0,0,0,0,16,64,1,12,2,13,
27,10,11,2,9,9,1,10,27,7,8,2,31,5,7,1,
19,5,6,5,18,0,255,227,0,0,0,0,12,5,0,5,
119,114,105,116,101,0,0,0,14,5,4,0,30,4,0,44,
100,101,102,32,115,101,116,112,111,115,40,118,41,58,0,0,
16,5,0,184,44,12,0,0,30,4,0,44,100,101,102,32,
115,101,116,112,111,115,40,118,41,58,0,0,12,1,0,9,
101,110,99,111,100,101,46,112,121,0,0,0,33,1,0,0,
12,1,0,6,115,101,116,112,111,115,0,0,34,1,0,0,
28,2,0,0,9,1,0,2,30,8,0,45,32,32,32,32,
105,102,32,39,45,110,111,112,111,115,39,32,105,110,32,65,
82,71,86,58,32,114,101,116,117,114,110,0,12,3,0,4,
65,82,71,86,0,0,0,0,13,2,3,0,12,3,0,6,
45,110,111,112,111,115,0,0,36,2,2,3,21,2,0,0,
18,0,0,4,28,2,0,0,20,2,0,0,18,0,0,1,
30,4,0,46,32,32,32,32,108,105,110,101,44,120,32,61,
32,118,0,0,11,4,0,0,0,0,0,0,0,0,0,0,
9,3,1,4,15,2,3,0,11,5,0,0,0,0,0,0,
0,0,240,63,9,4,1,5,15,3,4,0,30,8,0,47,
32,32,32,32,105,102,32,108,105,110,101,32,61,61,32,68,
46,108,105,110,101,110,111,58,32,114,101,116,117,114,110,0,
12,5,0,1,68,0,0,0,13,4,5,0,12,5,0,6,
108,105,110,101,110,111,0,0,9,4,4,5,23,1,2,4,
21,1,0,0,18,0,0,4,28,1,0,0,20,1,0,0,
18,0,0,1,30,7,0,48,32,32,32,32,116,101,120,116,
32,61,32,68,46,108,105,110,101,115,91,108,105,110,101,45,
49,93,0,0,12,5,0,1,68,0,0,0,13,4,5,0,
12,5,0,5,108,105,110,101,115,0,0,0,9,4,4,5,
11,6,0,0,0,0,0,0,0,0,240,63,2,5,2,6,
9,4,4,5,15,1,4,0,30,5,0,49,32,32,32,32,
68,46,108,105,110,101,110,111,32,61,32,108,105,110,101,0,
12,5,0,1,68,0,0,0,13,4,5,0,12,5,0,6,
108,105,110,101,110,111,0,0,10,4,5,2,30,10,0,50,
32,32,32,32,118,97,108,32,61,32,116,101,120,116,32,43,
32,34,92,48,34,42,40,52,45,108,101,110,40,116,101,120,
116,41,37,52,41,0,0,0,12,6,0,1,0,0,0,0,
11,7,0,0,0,0,0,0,0,0,16,64,12,10,0,3,
108,101,110,0,13,9,10,0,15,10,1,0,31,8,10,1,
19,8,9,8,11,9,0,0,0,0,0,0,0,0,16,64,
39,8,8,9,2,7,7,8,3,6,6,7,1,5,1,6,
15,4,5,0,30,9,0,51,32,32,32,32,99,111,100,101,
95,49,54,40,80,79,83,44,108,101,110,40,118,97,108,41,
47,52,44,108,105,110,101,41,0,0,0,0,12,7,0,7,
99,111,100,101,95,49,54,0,13,6,7,0,12,10,0,3,
80,79,83,0,13,7,10,0,12,11,0,3,108,101,110,0,
13,10,11,0,15,11,4,0,31,8,11,1,19,8,10,8,
11,10,0,0,0,0,0,0,0,0,16,64,4,8,8,10,
15,9,2,0,31,5,7,3,19,5,6,5,30,4,0,52,
32,32,32,32,119,114,105,116,101,40,118,97,108,41,0,0,
12,7,0,5,119,114,105,116,101,0,0,0,13,6,7,0,
15,7,4,0,31,5,7,1,19,5,6,5,0,0,0,0,
12,6,0,6,115,101,116,112,111,115,0,0,14,6,5,0,
30,7,0,53,100,101,102,32,99,111,100,101,40,105,44,97,
61,48,44,98,61,48,44,99,61,48,41,58,0,0,0,0,
16,6,0,167,44,13,0,0,30,7,0,53,100,101,102,32,
99,111,100,101,40,105,44,97,61,48,44,98,61,48,44,99,
61,48,41,58,0,0,0,0,12,1,0,9,101,110,99,111,
100,101,46,112,121,0,0,0,33,1,0,0,12,1,0,4,
99,111,100,101,0,0,0,0,34,1,0,0,28,2,0,0,
9,1,0,2,11,2,0,0,0,0,0,0,0,0,0,0,
28,3,0,0,32,2,0,3,11,3,0,0,0,0,0,0,
0,0,0,0,28,4,0,0,32,3,0,4,11,4,0,0,
0,0,0,0,0,0,0,0,28,5,0,0,32,4,0,5,
30,10,0,54,32,32,32,32,105,102,32,110,111,116,32,105,
115,116,121,112,101,40,105,44,39,110,117,109,98,101,114,39,
41,58,32,114,97,105,115,101,0,0,0,0,12,8,0,6,
105,115,116,121,112,101,0,0,13,7,8,0,15,8,1,0,
12,9,0,6,110,117,109,98,101,114,0,0,31,6,8,2,
19,6,7,6,47,5,6,0,21,5,0,0,18,0,0,4,
28,5,0,0,37,5,0,0,18,0,0,1,30,10,0,55,
32,32,32,32,105,102,32,110,111,116,32,105,115,116,121,112,
101,40,97,44,39,110,117,109,98,101,114,39,41,58,32,114,
97,105,115,101,0,0,0,0,12,8,0,6,105,115,116,121,
112,101,0,0,13,7,8,0,15,8,2,0,12,9,0,6,
110,117,109,98,101,114,0,0,31,6,8,2,19,6,7,6,
47,5,6,0,21,5,0,0,18,0,0,4,28,5,0,0,
37,5,0,0,18,0,0,1,30,10,0,56,32,32,32,32,
105,102,32,110,111,116,32,105,115,116,121,112,101,40,98,44,
39,110,117,109,98,101,114,39,41,58,32,114,97,105,115,101,
0,0,0,0,12,8,0,6,105,115,116,121,112,101,0,0,
13,7,8,0,15,8,3,0,12,9,0,6,110,117,109,98,
101,114,0,0,31,6,8,2,19,6,7,6,47,5,6,0,
21,5,0,0,18,0,0,4,28,5,0,0,37,5,0,0,
18,0,0,1,30,10,0,57,32,32,32,32,105,102,32,110,
111,116,32,105,115,116,121,112,101,40,99,44,39,110,117,109,
98,101,114,39,41,58,32,114,97,105,115,101,0,0,0,0,
12,8,0,6,105,115,116,121,112,101,0,0,13,7,8,0,
15,8,4,0,12,9,0,6,110,117,109,98,101,114,0,0,
31,6,8,2,19,6,7,6,47,5,6,0,21,5,0,0,
18,0,0,4,28,5,0,0,37,5,0,0,18,0,0,1,
30,7,0,58,32,32,32,32,119,114,105,116,101,40,40,39,
99,111,100,101,39,44,105,44,97,44,98,44,99,41,41,0,
12,7,0,5,119,114,105,116,101,0,0,0,13,6,7,0,
12,8,0,4,99,111,100,101,0,0,0,0,15,9,1,0,
15,10,2,0,15,11,3,0,15,12,4,0,27,7,8,5,
31,5,7,1,19,5,6,5,0,0,0,0,12,7,0,4,
99,111,100,101,0,0,0,0,14,7,6,0,30,5,0,59,
100,101,102,32,99,111,100,101,95,49,54,40,105,44,97,44,
98,41,58,0,16,7,0,79,44,11,0,0,30,5,0,59,
100,101,102,32,99,111,100,101,95,49,54,40,105,44,97,44,
98,41,58,0,12,1,0,9,101,110,99,111,100,101,46,112,
121,0,0,0,33,1,0,0,12,1,0,7,99,111,100,101,
95,49,54,0,34,1,0,0,28,2,0,0,9,1,0,2,
28,3,0,0,9,2,0,3,28,4,0,0,9,3,0,4,
30,7,0,60,32,32,32,32,105,102,32,98,32,60,32,48,
58,32,98,32,43,61,32,48,120,56,48,48,48,0,0,0,
11,5,0,0,0,0,0,0,0,0,0,0,25,4,3,5,
21,4,0,0,18,0,0,7,11,5,0,0,0,0,0,0,
0,0,224,64,1,4,3,5,15,3,4,0,18,0,0,1,
30,10,0,61,32,32,32,32,99,111,100,101,40,105,44,97,
44,40,98,38,48,120,102,102,48,48,41,62,62,56,44,40,
98,38,48,120,102,102,41,62,62,48,41,0,12,6,0,4,
99,111,100,101,0,0,0,0,13,5,6,0,15,6,1,0,
15,7,2,0,11,10,0,0,0,0,0,0,0,224,239,64,
6,8,3,10,11,10,0,0,0,0,0,0,0,0,32,64,
41,8,8,10,11,10,0,0,0,0,0,0,0,224,111,64,
6,9,3,10,11,10,0,0,0,0,0,0,0,0,0,0,
41,9,9,10,31,4,6,4,19,4,5,4,0,0,0,0,
12,8,0,7,99,111,100,101,95,49,54,0,14,8,7,0,
30,6,0,62,100,101,102,32,103,101,116,95,99,111,100,101,
49,54,40,105,44,97,44,98,41,58,0,0,16,8,0,63,
44,11,0,0,30,6,0,62,100,101,102,32,103,101,116,95,
99,111,100,101,49,54,40,105,44,97,44,98,41,58,0,0,
12,1,0,9,101,110,99,111,100,101,46,112,121,0,0,0,
33,1,0,0,12,1,0,10,103,101,116,95,99,111,100,101,
49,54,0,0,34,1,0,0,28,2,0,0,9,1,0,2,
28,3,0,0,9,2,0,3,28,4,0,0,9,3,0,4,
30,13,0,63,32,32,32,32,114,101,116,117,114,110,32,40,
39,99,111,100,101,39,44,105,44,97,44,40,98,38,48,120,
102,102,48,48,41,62,62,56,44,40,98,38,48,120,102,102,
41,62,62,48,41,0,0,0,12,5,0,4,99,111,100,101,
0,0,0,0,15,6,1,0,15,7,2,0,11,10,0,0,
0,0,0,0,0,224,239,64,6,8,3,10,11,10,0,0,
0,0,0,0,0,0,32,64,41,8,8,10,11,10,0,0,
0,0,0,0,0,224,111,64,6,9,3,10,11,10,0,0,
0,0,0,0,0,0,0,0,41,9,9,10,27,4,5,5,
20,4,0,0,0,0,0,0,12,9,0,10,103,101,116,95,
99,111,100,101,49,54,0,0,14,9,8,0,30,7,0,65,
100,101,102,32,95,100,111,95,115,116,114,105,110,103,40,118,
44,114,61,78,111,110,101,41,58,0,0,0,16,9,0,112,
44,11,0,0,30,7,0,65,100,101,102,32,95,100,111,95,
115,116,114,105,110,103,40,118,44,114,61,78,111,110,101,41,
58,0,0,0,12,1,0,9,101,110,99,111,100,101,46,112,
121,0,0,0,33,1,0,0,12,1,0,10,95,100,111,95,
115,116,114,105,110,103,0,0,34,1,0,0,28,2,0,0,
9,1,0,2,28,2,0,0,28,3,0,0,32,2,0,3,
30,5,0,66,32,32,32,32,114,32,61,32,103,101,116,95,
116,109,112,40,114,41,0,0,12,5,0,7,103,101,116,95,
116,109,112,0,13,4,5,0,15,5,2,0,31,3,5,1,
19,3,4,3,15,2,3,0,30,8,0,67,32,32,32,32,
118,97,108,32,61,32,118,32,43,32,34,92,48,34,42,40,
52,45,108,101,110,40,118,41,37,52,41,0,12,5,0,1,
0,0,0,0,11,6,0,0,0,0,0,0,0,0,16,64,
12,9,0,3,108,101,110,0,13,8,9,0,15,9,1,0,
31,7,9,1,19,7,8,7,11,8,0,0,0,0,0,0,
0,0,16,64,39,7,7,8,2,6,6,7,3,5,5,6,
1,4,1,5,15,3,4,0,30,8,0,68,32,32,32,32,
99,111,100,101,95,49,54,40,83,84,82,73,78,71,44,114,
44,108,101,110,40,118,41,41,0,0,0,0,12,6,0,7,
99,111,100,101,95,49,54,0,13,5,6,0,12,9,0,6,
83,84,82,73,78,71,0,0,13,6,9,0,15,7,2,0,
12,10,0,3,108,101,110,0,13,9,10,0,15,10,1,0,
31,8,10,1,19,8,9,8,31,4,6,3,19,4,5,4,
30,4,0,69,32,32,32,32,119,114,105,116,101,40,118,97,
108,41,0,0,12,6,0,5,119,114,105,116,101,0,0,0,
13,5,6,0,15,6,3,0,31,4,6,1,19,4,5,4,
30,4,0,70,32,32,32,32,114,101,116,117,114,110,32,114,
0,0,0,0,20,2,0,0,0,0,0,0,12,10,0,10,
95,100,111,95,115,116,114,105,110,103,0,0,14,10,9,0,
30,7,0,71,100,101,102,32,100,111,95,115,116,114,105,110,
103,40,116,44,114,61,78,111,110,101,41,58,0,0,0,0,
16,10,0,47,44,8,0,0,30,7,0,71,100,101,102,32,
100,111,95,115,116,114,105,110,103,40,116,44,114,61,78,111,
110,101,41,58,0,0,0,0,12,1,0,9,101,110,99,111,
100,101,46,112,121,0,0,0,33,1,0,0,12,1,0,9,
100,111,95,115,116,114,105,110,103,0,0,0,34,1,0,0,
28,2,0,0,9,1,0,2,28,2,0,0,28,3,0,0,
32,2,0,3,30,8,0,72,32,32,32,32,114,101,116,117,
114,110,32,95,100,111,95,115,116,114,105,110,103,40,116,46,
118,97,108,44,114,41,0,0,12,5,0,10,95,100,111,95,
115,116,114,105,110,103,0,0,13,4,5,0,12,7,0,3,
118,97,108,0,9,5,1,7,15,6,2,0,31,3,5,2,
19,3,4,3,20,3,0,0,0,0,0,0,12,11,0,9,
100,111,95,115,116,114,105,110,103,0,0,0,14,11,10,0,
30,7,0,74,100,101,102,32,95,100,111,95,110,117,109,98,
101,114,40,118,44,114,61,78,111,110,101,41,58,0,0,0,
16,11,0,97,44,10,0,0,30,7,0,74,100,101,102,32,
95,100,111,95,110,117,109,98,101,114,40,118,44,114,61,78,
111,110,101,41,58,0,0,0,12,1,0,9,101,110,99,111,
100,101,46,112,121,0,0,0,33,1,0,0,12,1,0,10,
95,100,111,95,110,117,109,98,101,114,0,0,34,1,0,0,
28,2,0,0,9,1,0,2,28,2,0,0,28,3,0,0,
32,2,0,3,30,5,0,75,32,32,32,32,114,32,61,32,
103,101,116,95,116,109,112,40,114,41,0,0,12,5,0,7,
103,101,116,95,116,109,112,0,13,4,5,0,15,5,2,0,
31,3,5,1,19,3,4,3,15,2,3,0,30,6,0,76,
32,32,32,32,99,111,100,101,40,78,85,77,66,69,82,44,
114,44,48,44,48,41,0,0,12,5,0,4,99,111,100,101,
0,0,0,0,13,4,5,0,12,9,0,6,78,85,77,66,
69,82,0,0,13,5,9,0,15,6,2,0,11,7,0,0,
0,0,0,0,0,0,0,0,11,8,0,0,0,0,0,0,
0,0,0,0,31,3,5,4,19,3,4,3,30,7,0,77,
32,32,32,32,119,114,105,116,101,40,102,112,97,99,107,40,
110,117,109,98,101,114,40,118,41,41,41,0,12,5,0,5,
119,114,105,116,101,0,0,0,13,4,5,0,12,7,0,5,
102,112,97,99,107,0,0,0,13,6,7,0,12,9,0,6,
110,117,109,98,101,114,0,0,13,8,9,0,15,9,1,0,
31,7,9,1,19,7,8,7,31,5,7,1,19,5,6,5,
31,3,5,1,19,3,4,3,30,4,0,78,32,32,32,32,
114,101,116,117,114,110,32,114,0,0,0,0,20,2,0,0,
0,0,0,0,12,12,0,10,95,100,111,95,110,117,109,98,
101,114,0,0,14,12,11,0,30,7,0,79,100,101,102,32,
100,111,95,110,117,109,98,101,114,40,116,44,114,61,78,111,
110,101,41,58,0,0,0,0,16,12,0,47,44,8,0,0,
30,7,0,79,100,101,102,32,100,111,95,110,117,109,98,101,
114,40,116,44,114,61,78,111,110,101,41,58,0,0,0,0,
12,1,0,9,101,110,99,111,100,101,46,112,121,0,0,0,
33,1,0,0,12,1,0,9,100,111,95,110,117,109,98,101,
114,0,0,0,34,1,0,0,28,2,0,0,9,1,0,2,
28,2,0,0,28,3,0,0,32,2,0,3,30,8,0,80,
32,32,32,32,114,101,116,117,114,110,32,95,100,111,95,110,
117,109,98,101,114,40,116,46,118,97,108,44,114,41,0,0,
12,5,0,10,95,100,111,95,110,117,109,98,101,114,0,0,
13,4,5,0,12,7,0,3,118,97,108,0,9,5,1,7,
15,6,2,0,31,3,5,2,19,3,4,3,20,3,0,0,
0,0,0,0,12,13,0,9,100,111,95,110,117,109,98,101,
114,0,0,0,14,13,12,0,30,4,0,82,100,101,102,32,
103,101,116,95,116,97,103,40,41,58,0,0,16,13,0,67,
44,6,0,0,30,4,0,82,100,101,102,32,103,101,116,95,
116,97,103,40,41,58,0,0,12,1,0,9,101,110,99,111,
100,101,46,112,121,0,0,0,33,1,0,0,12,1,0,7,
103,101,116,95,116,97,103,0,34,1,0,0,30,6,0,83,
32,32,32,32,107,32,61,32,115,116,114,40,68,46,95,116,
97,103,105,41,0,0,0,0,12,4,0,3,115,116,114,0,
13,3,4,0,12,5,0,1,68,0,0,0,13,4,5,0,
12,5,0,5,95,116,97,103,105,0,0,0,9,4,4,5,
31,2,4,1,19,2,3,2,15,1,2,0,30,5,0,84,
32,32,32,32,68,46,95,116,97,103,105,32,43,61,32,49,
0,0,0,0,12,3,0,1,68,0,0,0,13,2,3,0,
12,4,0,1,68,0,0,0,13,3,4,0,12,4,0,5,
95,116,97,103,105,0,0,0,9,3,3,4,11,4,0,0,
0,0,0,0,0,0,240,63,1,3,3,4,12,4,0,5,
95,116,97,103,105,0,0,0,10,2,4,3,30,4,0,85,
32,32,32,32,114,101,116,117,114,110,32,107,0,0,0,0,
20,1,0,0,0,0,0,0,12,14,0,7,103,101,116,95,
116,97,103,0,14,14,13,0,30,5,0,86,100,101,102,32,
115,116,97,99,107,95,116,97,103,40,41,58,0,0,0,0,
16,14,0,59,44,5,0,0,30,5,0,86,100,101,102,32,
115,116,97,99,107,95,116,97,103,40,41,58,0,0,0,0,
12,1,0,9,101,110,99,111,100,101,46,112,121,0,0,0,
33,1,0,0,12,1,0,9,115,116,97,99,107,95,116,97,
103,0,0,0,34,1,0,0,30,5,0,87,32,32,32,32,
107,32,61,32,103,101,116,95,116,97,103,40,41,0,0,0,
12,4,0,7,103,101,116,95,116,97,103,0,13,3,4,0,
31,2,0,0,19,2,3,2,15,1,2,0,30,6,0,88,
32,32,32,32,68,46,116,115,116,97,99,107,46,97,112,112,
101,110,100,40,107,41,0,0,12,4,0,1,68,0,0,0,
13,3,4,0,12,4,0,6,116,115,116,97,99,107,0,0,
9,3,3,4,12,4,0,6,97,112,112,101,110,100,0,0,
9,3,3,4,15,4,1,0,31,2,4,1,19,2,3,2,
30,4,0,89,32,32,32,32,114,101,116,117,114,110,32,107,
0,0,0,0,20,1,0,0,0,0,0,0,12,15,0,9,
115,116,97,99,107,95,116,97,103,0,0,0,14,15,14,0,
30,4,0,90,100,101,102,32,112,111,112,95,116,97,103,40,
41,58,0,0,16,15,0,35,44,4,0,0,30,4,0,90,
100,101,102,32,112,111,112,95,116,97,103,40,41,58,0,0,
12,1,0,9,101,110,99,111,100,101,46,112,121,0,0,0,
33,1,0,0,12,1,0,7,112,111,112,95,116,97,103,0,
34,1,0,0,30,5,0,91,32,32,32,32,68,46,116,115,
116,97,99,107,46,112,111,112,40,41,0,0,12,3,0,1,
68,0,0,0,13,2,3,0,12,3,0,6,116,115,116,97,
99,107,0,0,9,2,2,3,12,3,0,3,112,111,112,0,
9,2,2,3,31,1,0,0,19,1,2,1,0,0,0,0,
12,16,0,7,112,111,112,95,116,97,103,0,14,16,15,0,
30,4,0,93,100,101,102,32,116,97,103,40,42,116,41,58,
0,0,0,0,16,16,0,86,44,12,0,0,30,4,0,93,
100,101,102,32,116,97,103,40,42,116,41,58,0,0,0,0,
12,1,0,9,101,110,99,111,100,101,46,112,121,0,0,0,
33,1,0,0,12,1,0,3,116,97,103,0,34,1,0,0,
12,2,0,1,42,0,0,0,9,1,0,2,30,13,0,94,
32,32,32,32,116,32,61,32,68,46,115,110,117,109,43,39,
58,39,43,39,58,39,46,106,111,105,110,40,91,115,116,114,
40,118,41,32,102,111,114,32,118,32,105,110,32,116,93,41,
0,0,0,0,12,3,0,1,68,0,0,0,13,2,3,0,
12,3,0,4,115,110,117,109,0,0,0,0,9,2,2,3,
12,3,0,1,58,0,0,0,1,2,2,3,12,4,0,1,
58,0,0,0,12,5,0,4,106,111,105,110,0,0,0,0,
9,4,4,5,27,6,0,0,11,8,0,0,0,0,0,0,
0,0,0,0,42,7,1,8,18,0,0,10,12,11,0,3,
115,116,114,0,13,10,11,0,15,11,7,0,31,9,11,1,
19,9,10,9,28,10,0,0,10,6,10,9,18,0,255,246,
15,5,6,0,31,3,5,1,19,3,4,3,1,2,2,3,
15,1,2,0,30,6,0,95,32,32,32,32,105,110,115,101,
114,116,40,40,39,116,97,103,39,44,116,41,41,0,0,0,
12,4,0,6,105,110,115,101,114,116,0,0,13,3,4,0,
12,8,0,3,116,97,103,0,15,9,1,0,27,4,8,2,
31,2,4,1,19,2,3,2,0,0,0,0,12,17,0,3,
116,97,103,0,14,17,16,0,30,4,0,96,100,101,102,32,
106,117,109,112,40,42,116,41,58,0,0,0,16,17,0,88,
44,12,0,0,30,4,0,96,100,101,102,32,106,117,109,112,
40,42,116,41,58,0,0,0,12,1,0,9,101,110,99,111,
100,101,46,112,121,0,0,0,33,1,0,0,12,1,0,4,
106,117,109,112,0,0,0,0,34,1,0,0,12,2,0,1,
42,0,0,0,9,1,0,2,30,13,0,97,32,32,32,32,
116,32,61,32,68,46,115,110,117,109,43,39,58,39,43,39,
58,39,46,106,111,105,110,40,91,115,116,114,40,118,41,32,
102,111,114,32,118,32,105,110,32,116,93,41,0,0,0,0,
12,3,0,1,68,0,0,0,13,2,3,0,12,3,0,4,
115,110,117,109,0,0,0,0,9,2,2,3,12,3,0,1,
58,0,0,0,1,2,2,3,12,4,0,1,58,0,0,0,
12,5,0,4,106,111,105,110,0,0,0,0,9,4,4,5,
27,6,0,0,11,8,0,0,0,0,0,0,0,0,0,0,
42,7,1,8,18,0,0,10,12,11,0,3,115,116,114,0,
13,10,11,0,15,11,7,0,31,9,11,1,19,9,10,9,
28,10,0,0,10,6,10,9,18,0,255,246,15,5,6,0,
31,3,5,1,19,3,4,3,1,2,2,3,15,1,2,0,
30,6,0,98,32,32,32,32,105,110,115,101,114,116,40,40,
39,106,117,109,112,39,44,116,41,41,0,0,12,4,0,6,
105,110,115,101,114,116,0,0,13,3,4,0,12,8,0,4,
106,117,109,112,0,0,0,0,15,9,1,0,27,4,8,2,
31,2,4,1,19,2,3,2,0,0,0,0,12,18,0,4,
106,117,109,112,0,0,0,0,14,18,17,0,30,4,0,99,
100,101,102,32,115,101,116,106,109,112,40,42,116,41,58,0,
16,18,0,89,44,12,0,0,30,4,0,99,100,101,102,32,
115,101,116,106,109,112,40,42,116,41,58,0,12,1,0,9,
101,110,99,111,100,101,46,112,121,0,0,0,33,1,0,0,
12,1,0,6,115,101,116,106,109,112,0,0,34,1,0,0,
12,2,0,1,42,0,0,0,9,1,0,2,30,13,0,100,
32,32,32,32,116,32,61,32,68,46,115,110,117,109,43,39,
58,39,43,39,58,39,46,106,111,105,110,40,91,115,116,114,
40,118,41,32,102,111,114,32,118,32,105,110,32,116,93,41,
0,0,0,0,12,3,0,1,68,0,0,0,13,2,3,0,
12,3,0,4,115,110,117,109,0,0,0,0,9,2,2,3,
12,3,0,1,58,0,0,0,1,2,2,3,12,4,0,1,
58,0,0,0,12,5,0,4,106,111,105,110,0,0,0,0,
9,4,4,5,27,6,0,0,11,8,0,0,0,0,0,0,
0,0,0,0,42,7,1,8,18,0,0,10,12,11,0,3,
115,116,114,0,13,10,11,0,15,11,7,0,31,9,11,1,
19,9,10,9,28,10,0,0,10,6,10,9,18,0,255,246,
15,5,6,0,31,3,5,1,19,3,4,3,1,2,2,3,
15,1,2,0,30,7,0,101,32,32,32,32,105,110,115,101,
114,116,40,40,39,115,101,116,106,109,112,39,44,116,41,41,
0,0,0,0,12,4,0,6,105,110,115,101,114,116,0,0,
13,3,4,0,12,8,0,6,115,101,116,106,109,112,0,0,
15,9,1,0,27,4,8,2,31,2,4,1,19,2,3,2,
0,0,0,0,12,19,0,6,115,101,116,106,109,112,0,0,
14,19,18,0,30,4,0,102,100,101,102,32,102,110,99,40,
42,116,41,58,0,0,0,0,16,19,0,107,44,12,0,0,
30,4,0,102,100,101,102,32,102,110,99,40,42,116,41,58,
0,0,0,0,12,1,0,9,101,110,99,111,100,101,46,112,
121,0,0,0,33,1,0,0,12,1,0,3,102,110,99,0,
34,1,0,0,12,2,0,1,42,0,0,0,9,1,0,2,
30,13,0,103,32,32,32,32,116,32,61,32,68,46,115,110,
117,109,43,39,58,39,43,39,58,39,46,106,111,105,110,40,
91,115,116,114,40,118,41,32,102,111,114,32,118,32,105,110,
32,116,93,41,0,0,0,0,12,3,0,1,68,0,0,0,
13,2,3,0,12,3,0,4,115,110,117,109,0,0,0,0,
9,2,2,3,12,3,0,1,58,0,0,0,1,2,2,3,
12,4,0,1,58,0,0,0,12,5,0,4,106,111,105,110,
0,0,0,0,9,4,4,5,27,6,0,0,11,8,0,0,
0,0,0,0,0,0,0,0,42,7,1,8,18,0,0,10,
12,11,0,3,115,116,114,0,13,10,11,0,15,11,7,0,
31,9,11,1,19,9,10,9,28,10,0,0,10,6,10,9,
18,0,255,246,15,5,6,0,31,3,5,1,19,3,4,3,
1,2,2,3,15,1,2,0,30,5,0,104,32,32,32,32,
114,32,61,32,103,101,116,95,114,101,103,40,116,41,0,0,
12,5,0,7,103,101,116,95,114,101,103,0,13,4,5,0,
15,5,1,0,31,3,5,1,19,3,4,3,15,2,3,0,
30,6,0,105,32,32,32,32,105,110,115,101,114,116,40,40,
39,102,110,99,39,44,114,44,116,41,41,0,12,5,0,6,
105,110,115,101,114,116,0,0,13,4,5,0,12,8,0,3,
102,110,99,0,15,9,2,0,15,10,1,0,27,5,8,3,
31,3,5,1,19,3,4,3,30,4,0,106,32,32,32,32,
114,101,116,117,114,110,32,114,0,0,0,0,20,2,0,0,
0,0,0,0,12,20,0,3,102,110,99,0,14,20,19,0,
30,4,0,108,100,101,102,32,109,97,112,95,116,97,103,115,
40,41,58,0,16,20,2,175,44,17,0,0,30,4,0,108,
100,101,102,32,109,97,112,95,116,97,103,115,40,41,58,0,
12,1,0,9,101,110,99,111,100,101,46,112,121,0,0,0,
33,1,0,0,12,1,0,8,109,97,112,95,116,97,103,115,
0,0,0,0,34,1,0,0,30,4,0,109,32,32,32,32,
116,97,103,115,32,61,32,123,125,0,0,0,26,2,0,0,
15,1,2,0,30,4,0,110,32,32,32,32,111,117,116,32,
61,32,91,93,0,0,0,0,27,3,0,0,15,2,3,0,
30,3,0,111,32,32,32,32,110,32,61,32,48,0,0,0,
11,4,0,0,0,0,0,0,0,0,0,0,15,3,4,0,
30,6,0,112,32,32,32,32,102,111,114,32,105,116,101,109,
32,105,110,32,68,46,111,117,116,58,0,0,12,6,0,1,
68,0,0,0,13,5,6,0,12,6,0,3,111,117,116,0,
9,5,5,6,11,6,0,0,0,0,0,0,0,0,0,0,
42,4,5,6,18,0,0,145,30,8,0,113,32,32,32,32,
32,32,32,32,105,102,32,105,116,101,109,91,48,93,32,61,
61,32,39,116,97,103,39,58,0,0,0,0,11,8,0,0,
0,0,0,0,0,0,0,0,9,7,4,8,12,8,0,3,
116,97,103,0,23,7,7,8,21,7,0,0,18,0,0,24,
30,8,0,114,32,32,32,32,32,32,32,32,32,32,32,32,
116,97,103,115,91,105,116,101,109,91,49,93,93,32,61,32,
110,0,0,0,11,8,0,0,0,0,0,0,0,0,240,63,
9,7,4,8,10,1,7,3,30,6,0,115,32,32,32,32,
32,32,32,32,32,32,32,32,99,111,110,116,105,110,117,101,
0,0,0,0,18,0,255,215,18,0,0,1,30,8,0,116,
32,32,32,32,32,32,32,32,105,102,32,105,116,101,109,91,
48,93,32,61,61,32,39,114,101,103,115,39,58,0,0,0,
11,8,0,0,0,0,0,0,0,0,0,0,9,7,4,8,
12,8,0,4,114,101,103,115,0,0,0,0,23,7,7,8,
21,7,0,0,18,0,0,59,30,13,0,117,32,32,32,32,
32,32,32,32,32,32,32,32,111,117,116,46,97,112,112,101,
110,100,40,103,101,116,95,99,111,100,101,49,54,40,82,69,
71,83,44,105,116,101,109,91,49,93,44,48,41,41,0,0,
12,9,0,6,97,112,112,101,110,100,0,0,9,8,2,9,
12,11,0,10,103,101,116,95,99,111,100,101,49,54,0,0,
13,10,11,0,12,14,0,4,82,69,71,83,0,0,0,0,
13,11,14,0,11,14,0,0,0,0,0,0,0,0,240,63,
9,12,4,14,11,13,0,0,0,0,0,0,0,0,0,0,
31,9,11,3,19,9,10,9,31,7,9,1,19,7,8,7,
30,5,0,118,32,32,32,32,32,32,32,32,32,32,32,32,
110,32,43,61,32,49,0,0,11,8,0,0,0,0,0,0,
0,0,240,63,1,7,3,8,15,3,7,0,30,6,0,119,
32,32,32,32,32,32,32,32,32,32,32,32,99,111,110,116,
105,110,117,101,0,0,0,0,18,0,255,138,18,0,0,1,
30,7,0,120,32,32,32,32,32,32,32,32,111,117,116,46,
97,112,112,101,110,100,40,105,116,101,109,41,0,0,0,0,
12,9,0,6,97,112,112,101,110,100,0,0,9,8,2,9,
15,9,4,0,31,7,9,1,19,7,8,7,30,4,0,121,
32,32,32,32,32,32,32,32,110,32,43,61,32,49,0,0,
11,8,0,0,0,0,0,0,0,0,240,63,1,7,3,8,
15,3,7,0,18,0,255,111,30,8,0,122,32,32,32,32,
102,111,114,32,110,32,105,110,32,114,97,110,103,101,40,48,
44,108,101,110,40,111,117,116,41,41,58,0,12,7,0,5,
114,97,110,103,101,0,0,0,13,6,7,0,11,7,0,0,
0,0,0,0,0,0,0,0,12,10,0,3,108,101,110,0,
13,9,10,0,15,10,2,0,31,8,10,1,19,8,9,8,
31,5,7,2,19,5,6,5,11,6,0,0,0,0,0,0,
0,0,0,0,42,3,5,6,18,0,0,182,30,6,0,123,
32,32,32,32,32,32,32,32,105,116,101,109,32,61,32,111,
117,116,91,110,93,0,0,0,9,7,2,3,15,4,7,0,
30,8,0,124,32,32,32,32,32,32,32,32,105,102,32,105,
116,101,109,91,48,93,32,61,61,32,39,106,117,109,112,39,
58,0,0,0,11,8,0,0,0,0,0,0,0,0,0,0,
9,7,4,8,12,8,0,4,106,117,109,112,0,0,0,0,
23,7,7,8,21,7,0,0,18,0,0,38,30,14,0,125,
32,32,32,32,32,32,32,32,32,32,32,32,111,117,116,91,
110,93,32,61,32,103,101,116,95,99,111,100,101,49,54,40,
74,85,77,80,44,48,44,116,97,103,115,91,105,116,101,109,
91,49,93,93,45,110,41,0,12,9,0,10,103,101,116,95,
99,111,100,101,49,54,0,0,13,8,9,0,12,12,0,4,
74,85,77,80,0,0,0,0,13,9,12,0,11,10,0,0,
0,0,0,0,0,0,0,0,11,13,0,0,0,0,0,0,
0,0,240,63,9,12,4,13,9,11,1,12,2,11,11,3,
31,7,9,3,19,7,8,7,10,2,3,7,18,0,0,116,
30,9,0,126,32,32,32,32,32,32,32,32,101,108,105,102,
32,105,116,101,109,91,48,93,32,61,61,32,39,115,101,116,
106,109,112,39,58,0,0,0,11,8,0,0,0,0,0,0,
0,0,0,0,9,7,4,8,12,8,0,6,115,101,116,106,
109,112,0,0,23,7,7,8,21,7,0,0,18,0,0,39,
30,15,0,127,32,32,32,32,32,32,32,32,32,32,32,32,
111,117,116,91,110,93,32,61,32,103,101,116,95,99,111,100,
101,49,54,40,83,69,84,74,77,80,44,48,44,116,97,103,
115,91,105,116,101,109,91,49,93,93,45,110,41,0,0,0,
12,9,0,10,103,101,116,95,99,111,100,101,49,54,0,0,
13,8,9,0,12,12,0,6,83,69,84,74,77,80,0,0,
13,9,12,0,11,10,0,0,0,0,0,0,0,0,0,0,
11,13,0,0,0,0,0,0,0,0,240,63,9,12,4,13,
9,11,1,12,2,11,11,3,31,7,9,3,19,7,8,7,
10,2,3,7,18,0,0,58,30,8,0,128,32,32,32,32,
32,32,32,32,101,108,105,102,32,105,116,101,109,91,48,93,
32,61,61,32,39,102,110,99,39,58,0,0,11,8,0,0,
0,0,0,0,0,0,0,0,9,7,4,8,12,8,0,3,
102,110,99,0,23,7,7,8,21,7,0,0,18,0,0,40,
30,16,0,129,32,32,32,32,32,32,32,32,32,32,32,32,
111,117,116,91,110,93,32,61,32,103,101,116,95,99,111,100,
101,49,54,40,68,69,70,44,105,116,101,109,91,49,93,44,
116,97,103,115,91,105,116,101,109,91,50,93,93,45,110,41,
0,0,0,0,12,9,0,10,103,101,116,95,99,111,100,101,
49,54,0,0,13,8,9,0,12,12,0,3,68,69,70,0,
13,9,12,0,11,12,0,0,0,0,0,0,0,0,240,63,
9,10,4,12,11,13,0,0,0,0,0,0,0,0,0,64,
9,12,4,13,9,11,1,12,2,11,11,3,31,7,9,3,
19,7,8,7,10,2,3,7,18,0,0,1,18,0,255,74,
30,8,0,130,32,32,32,32,102,111,114,32,110,32,105,110,
32,114,97,110,103,101,40,48,44,108,101,110,40,111,117,116,
41,41,58,0,12,7,0,5,114,97,110,103,101,0,0,0,
13,6,7,0,11,7,0,0,0,0,0,0,0,0,0,0,
12,10,0,3,108,101,110,0,13,9,10,0,15,10,2,0,
31,8,10,1,19,8,9,8,31,5,7,2,19,5,6,5,
11,6,0,0,0,0,0,0,0,0,0,0,42,3,5,6,
18,0,0,236,30,6,0,131,32,32,32,32,32,32,32,32,
105,116,101,109,32,61,32,111,117,116,91,110,93,0,0,0,
9,7,2,3,15,4,7,0,30,8,0,132,32,32,32,32,
32,32,32,32,105,102,32,105,116,101,109,91,48,93,32,61,
61,32,39,100,97,116,97,39,58,0,0,0,11,8,0,0,
0,0,0,0,0,0,0,0,9,7,4,8,12,8,0,4,
100,97,116,97,0,0,0,0,23,7,7,8,21,7,0,0,
18,0,0,16,30,8,0,133,32,32,32,32,32,32,32,32,
32,32,32,32,111,117,116,91,110,93,32,61,32,105,116,101,
109,91,49,93,0,0,0,0,11,8,0,0,0,0,0,0,
0,0,240,63,9,7,4,8,10,2,3,7,18,0,0,121,
30,8,0,134,32,32,32,32,32,32,32,32,101,108,105,102,
32,105,116,101,109,91,48,93,32,61,61,32,39,99,111,100,
101,39,58,0,11,8,0,0,0,0,0,0,0,0,0,0,
9,7,4,8,12,8,0,4,99,111,100,101,0,0,0,0,
23,7,7,8,21,7,0,0,18,0,0,79,30,8,0,135,
32,32,32,32,32,32,32,32,32,32,32,32,105,44,97,44,
98,44,99,32,61,32,105,116,101,109,91,49,58,93,0,0,
11,9,0,0,0,0,0,0,0,0,240,63,28,10,0,0,
27,8,9,2,9,7,4,8,11,10,0,0,0,0,0,0,
0,0,0,0,9,9,7,10,15,8,9,0,11,11,0,0,
0,0,0,0,0,0,240,63,9,10,7,11,15,9,10,0,
11,12,0,0,0,0,0,0,0,0,0,64,9,11,7,12,
15,10,11,0,11,13,0,0,0,0,0,0,0,0,8,64,
9,12,7,13,15,11,12,0,30,13,0,136,32,32,32,32,
32,32,32,32,32,32,32,32,111,117,116,91,110,93,32,61,
32,99,104,114,40,105,41,43,99,104,114,40,97,41,43,99,
104,114,40,98,41,43,99,104,114,40,99,41,0,0,0,0,
12,13,0,3,99,104,114,0,13,12,13,0,15,13,8,0,
31,7,13,1,19,7,12,7,12,14,0,3,99,104,114,0,
13,13,14,0,15,14,9,0,31,12,14,1,19,12,13,12,
1,7,7,12,12,14,0,3,99,104,114,0,13,13,14,0,
15,14,10,0,31,12,14,1,19,12,13,12,1,7,7,12,
12,14,0,3,99,104,114,0,13,13,14,0,15,14,11,0,
31,12,14,1,19,12,13,12,1,7,7,12,10,2,3,7,
18,0,0,24,30,10,0,138,32,32,32,32,32,32,32,32,
32,32,32,32,114,97,105,115,101,32,115,116,114,40,40,39,
104,117,104,63,39,44,105,116,101,109,41,41,0,0,0,0,
12,13,0,3,115,116,114,0,13,12,13,0,12,14,0,4,
104,117,104,63,0,0,0,0,15,15,4,0,27,13,14,2,
31,7,13,1,19,7,12,7,37,7,0,0,18,0,0,1,
30,8,0,139,32,32,32,32,32,32,32,32,105,102,32,108,
101,110,40,111,117,116,91,110,93,41,32,33,61,32,52,58,
0,0,0,0,12,13,0,3,108,101,110,0,13,12,13,0,
9,13,2,3,31,7,13,1,19,7,12,7,11,12,0,0,
0,0,0,0,0,0,16,64,35,7,7,12,21,7,0,0,
18,0,0,51,30,18,0,140,32,32,32,32,32,32,32,32,
32,32,32,32,114,97,105,115,101,32,40,39,99,111,100,101,
32,39,43,115,116,114,40,110,41,43,39,32,105,115,32,119,
114,111,110,103,32,108,101,110,103,116,104,32,39,43,115,116,
114,40,108,101,110,40,111,117,116,91,110,93,41,41,41,0,
12,7,0,5,99,111,100,101,32,0,0,0,12,14,0,3,
115,116,114,0,13,13,14,0,15,14,3,0,31,12,14,1,
19,12,13,12,1,7,7,12,12,12,0,17,32,105,115,32,
119,114,111,110,103,32,108,101,110,103,116,104,32,0,0,0,
1,7,7,12,12,14,0,3,115,116,114,0,13,13,14,0,
12,16,0,3,108,101,110,0,13,15,16,0,9,16,2,3,
31,14,16,1,19,14,15,14,31,12,14,1,19,12,13,12,
1,7,7,12,37,7,0,0,18,0,0,1,18,0,255,20,
30,4,0,141,32,32,32,32,68,46,111,117,116,32,61,32,
111,117,116,0,12,6,0,1,68,0,0,0,13,5,6,0,
12,6,0,3,111,117,116,0,10,5,6,2,0,0,0,0,
12,21,0,8,109,97,112,95,116,97,103,115,0,0,0,0,
14,21,20,0,30,6,0,143,100,101,102,32,103,101,116,95,
116,109,112,40,114,61,78,111,110,101,41,58,0,0,0,0,
16,21,0,59,44,5,0,0,30,6,0,143,100,101,102,32,
103,101,116,95,116,109,112,40,114,61,78,111,110,101,41,58,
0,0,0,0,12,1,0,9,101,110,99,111,100,101,46,112,
121,0,0,0,33,1,0,0,12,1,0,7,103,101,116,95,
116,109,112,0,34,1,0,0,28,1,0,0,28,2,0,0,
32,1,0,2,30,7,0,144,32,32,32,32,105,102,32,114,
32,33,61,32,78,111,110,101,58,32,114,101,116,117,114,110,
32,114,0,0,28,3,0,0,35,2,1,3,21,2,0,0,
18,0,0,3,20,1,0,0,18,0,0,1,30,7,0,145,
32,32,32,32,114,101,116,117,114,110,32,103,101,116,95,116,
109,112,115,40,49,41,91,48,93,0,0,0,12,4,0,8,
103,101,116,95,116,109,112,115,0,0,0,0,13,3,4,0,
11,4,0,0,0,0,0,0,0,0,240,63,31,2,4,1,
19,2,3,2,11,3,0,0,0,0,0,0,0,0,0,0,
9,2,2,3,20,2,0,0,0,0,0,0,12,22,0,7,
103,101,116,95,116,109,112,0,14,22,21,0,30,5,0,146,
100,101,102,32,103,101,116,95,116,109,112,115,40,116,41,58,
0,0,0,0,16,22,0,149,44,14,0,0,30,5,0,146,
100,101,102,32,103,101,116,95,116,109,112,115,40,116,41,58,
0,0,0,0,12,1,0,9,101,110,99,111,100,101,46,112,
121,0,0,0,33,1,0,0,12,1,0,8,103,101,116,95,
116,109,112,115,0,0,0,0,34,1,0,0,28,2,0,0,
9,1,0,2,30,5,0,147,32,32,32,32,114,115,32,61,
32,97,108,108,111,99,40,116,41,0,0,0,12,5,0,5,
97,108,108,111,99,0,0,0,13,4,5,0,15,5,1,0,
31,3,5,1,19,3,4,3,15,2,3,0,30,7,0,148,
32,32,32,32,114,101,103,115,32,61,32,114,97,110,103,101,
40,114,115,44,114,115,43,116,41,0,0,0,12,6,0,5,
114,97,110,103,101,0,0,0,13,5,6,0,15,6,2,0,
1,7,2,1,31,4,6,2,19,4,5,4,15,3,4,0,
30,5,0,149,32,32,32,32,102,111,114,32,114,32,105,110,
32,114,101,103,115,58,0,0,11,5,0,0,0,0,0,0,
0,0,0,0,42,4,3,5,18,0,0,59,30,9,0,150,
32,32,32,32,32,32,32,32,115,101,116,95,114,101,103,40,
114,44,34,36,34,43,115,116,114,40,68,46,95,116,109,112,
105,41,41,0,12,8,0,7,115,101,116,95,114,101,103,0,
13,7,8,0,15,8,4,0,12,9,0,1,36,0,0,0,
12,12,0,3,115,116,114,0,13,11,12,0,12,13,0,1,
68,0,0,0,13,12,13,0,12,13,0,5,95,116,109,112,
105,0,0,0,9,12,12,13,31,10,12,1,19,10,11,10,
1,9,9,10,31,6,8,2,19,6,7,6,30,6,0,151,
32,32,32,32,32,32,32,32,68,46,95,116,109,112,105,32,
43,61,32,49,0,0,0,0,12,7,0,1,68,0,0,0,
13,6,7,0,12,8,0,1,68,0,0,0,13,7,8,0,
12,8,0,5,95,116,109,112,105,0,0,0,9,7,7,8,
11,8,0,0,0,0,0,0,0,0,240,63,1,7,7,8,
12,8,0,5,95,116,109,112,105,0,0,0,10,6,8,7,
18,0,255,197,30,6,0,152,32,32,32,32,68,46,116,109,
112,99,32,43,61,32,116,32,35,82,69,71,0,0,0,0,
12,6,0,1,68,0,0,0,13,5,6,0,12,7,0,1,
68,0,0,0,13,6,7,0,12,7,0,4,116,109,112,99,
0,0,0,0,9,6,6,7,1,6,6,1,12,7,0,4,
116,109,112,99,0,0,0,0,10,5,7,6,30,4,0,153,
32,32,32,32,114,101,116,117,114,110,32,114,101,103,115,0,
20,3,0,0,0,0,0,0,12,23,0,8,103,101,116,95,
116,109,112,115,0,0,0,0,14,23,22,0,30,4,0,154,
100,101,102,32,97,108,108,111,99,40,116,41,58,0,0,0,
16,23,0,110,44,16,0,0,30,4,0,154,100,101,102,32,
97,108,108,111,99,40,116,41,58,0,0,0,12,1,0,9,
101,110,99,111,100,101,46,112,121,0,0,0,33,1,0,0,
12,1,0,5,97,108,108,111,99,0,0,0,34,1,0,0,
28,2,0,0,9,1,0,2,30,18,0,155,32,32,32,32,
115,32,61,32,39,39,46,106,111,105,110,40,91,34,48,49,
34,91,114,32,105,110,32,68,46,114,50,110,93,32,102,111,
114,32,114,32,105,110,32,114,97,110,103,101,40,48,44,109,
105,110,40,50,53,54,44,68,46,109,114,101,103,43,116,41,
41,93,41,0,12,4,0,0,0,0,0,0,12,5,0,4,
106,111,105,110,0,0,0,0,9,4,4,5,27,6,0,0,
12,10,0,5,114,97,110,103,101,0,0,0,13,9,10,0,
11,10,0,0,0,0,0,0,0,0,0,0,12,13,0,3,
109,105,110,0,13,12,13,0,11,13,0,0,0,0,0,0,
0,0,112,64,12,15,0,1,68,0,0,0,13,14,15,0,
12,15,0,4,109,114,101,103,0,0,0,0,9,14,14,15,
1,14,14,1,31,11,13,2,19,11,12,11,31,8,10,2,
19,8,9,8,11,9,0,0,0,0,0,0,0,0,0,0,
42,7,8,9,18,0,0,14,12,10,0,2,48,49,0,0,
12,12,0,1,68,0,0,0,13,11,12,0,12,12,0,3,
114,50,110,0,9,11,11,12,36,11,11,7,9,10,10,11,
28,11,0,0,10,6,11,10,18,0,255,242,15,5,6,0,
31,3,5,1,19,3,4,3,15,2,3,0,30,7,0,156,
32,32,32,32,114,101,116,117,114,110,32,115,46,105,110,100,
101,120,40,39,48,39,42,116,41,0,0,0,12,5,0,5,
105,110,100,101,120,0,0,0,9,4,2,5,12,5,0,1,
48,0,0,0,3,5,5,1,31,3,5,1,19,3,4,3,
20,3,0,0,0,0,0,0,12,24,0,5,97,108,108,111,
99,0,0,0,14,24,23,0,30,4,0,157,100,101,102,32,
105,115,95,116,109,112,40,114,41,58,0,0,16,24,0,61,
44,4,0,0,30,4,0,157,100,101,102,32,105,115,95,116,
109,112,40,114,41,58,0,0,12,1,0,9,101,110,99,111,
100,101,46,112,121,0,0,0,33,1,0,0,12,1,0,6,
105,115,95,116,109,112,0,0,34,1,0,0,28,2,0,0,
9,1,0,2,30,8,0,158,32,32,32,32,105,102,32,114,
32,105,115,32,78,111,110,101,58,32,114,101,116,117,114,110,
32,70,97,108,115,101,0,0,28,3,0,0,23,2,1,3,
21,2,0,0,18,0,0,6,11,2,0,0,0,0,0,0,
0,0,0,0,20,2,0,0,18,0,0,1,30,8,0,159,
32,32,32,32,114,101,116,117,114,110,32,40,68,46,114,50,
110,91,114,93,91,48,93,32,61,61,32,39,36,39,41,0,
12,3,0,1,68,0,0,0,13,2,3,0,12,3,0,3,
114,50,110,0,9,2,2,3,9,2,2,1,11,3,0,0,
0,0,0,0,0,0,0,0,9,2,2,3,12,3,0,1,
36,0,0,0,23,2,2,3,20,2,0,0,0,0,0,0,
12,25,0,6,105,115,95,116,109,112,0,0,14,25,24,0,
30,4,0,160,100,101,102,32,117,110,95,116,109,112,40,114,
41,58,0,0,16,25,0,63,44,7,0,0,30,4,0,160,
100,101,102,32,117,110,95,116,109,112,40,114,41,58,0,0,
12,1,0,9,101,110,99,111,100,101,46,112,121,0,0,0,
33,1,0,0,12,1,0,6,117,110,95,116,109,112,0,0,
34,1,0,0,28,2,0,0,9,1,0,2,30,5,0,161,
32,32,32,32,110,32,61,32,68,46,114,50,110,91,114,93,
0,0,0,0,12,4,0,1,68,0,0,0,13,3,4,0,
12,4,0,3,114,50,110,0,9,3,3,4,9,3,3,1,
15,2,3,0,30,4,0,162,32,32,32,32,102,114,101,101,
95,114,101,103,40,114,41,0,12,5,0,8,102,114,101,101,
95,114,101,103,0,0,0,0,13,4,5,0,15,5,1,0,
31,3,5,1,19,3,4,3,30,6,0,163,32,32,32,32,
115,101,116,95,114,101,103,40,114,44,39,42,39,43,110,41,
0,0,0,0,12,5,0,7,115,101,116,95,114,101,103,0,
13,4,5,0,15,5,1,0,12,6,0,1,42,0,0,0,
1,6,6,2,31,3,5,2,19,3,4,3,0,0,0,0,
12,26,0,6,117,110,95,116,109,112,0,0,14,26,25,0,
30,5,0,164,100,101,102,32,102,114,101,101,95,116,109,112,
40,114,41,58,0,0,0,0,16,26,0,54,44,5,0,0,
30,5,0,164,100,101,102,32,102,114,101,101,95,116,109,112,
40,114,41,58,0,0,0,0,12,1,0,9,101,110,99,111,
100,101,46,112,121,0,0,0,33,1,0,0,12,1,0,8,
102,114,101,101,95,116,109,112,0,0,0,0,34,1,0,0,
28,2,0,0,9,1,0,2,30,8,0,165,32,32,32,32,
105,102,32,105,115,95,116,109,112,40,114,41,58,32,102,114,
101,101,95,114,101,103,40,114,41,0,0,0,12,4,0,6,
105,115,95,116,109,112,0,0,13,3,4,0,15,4,1,0,
31,2,4,1,19,2,3,2,21,2,0,0,18,0,0,10,
12,4,0,8,102,114,101,101,95,114,101,103,0,0,0,0,
13,3,4,0,15,4,1,0,31,2,4,1,19,2,3,2,
18,0,0,1,30,4,0,166,32,32,32,32,114,101,116,117,
114,110,32,114,0,0,0,0,20,1,0,0,0,0,0,0,
12,27,0,8,102,114,101,101,95,116,109,112,0,0,0,0,
14,27,26,0,30,5,0,167,100,101,102,32,102,114,101,101,
95,116,109,112,115,40,114,41,58,0,0,0,16,27,0,43,
44,7,0,0,30,5,0,167,100,101,102,32,102,114,101,101,
95,116,109,112,115,40,114,41,58,0,0,0,12,1,0,9,
101,110,99,111,100,101,46,112,121,0,0,0,33,1,0,0,
12,1,0,9,102,114,101,101,95,116,109,112,115,0,0,0,
34,1,0,0,28,2,0,0,9,1,0,2,30,7,0,168,
32,32,32,32,102,111,114,32,107,32,105,110,32,114,58,32,
102,114,101,101,95,116,109,112,40,107,41,0,11,3,0,0,
0,0,0,0,0,0,0,0,42,2,1,3,18,0,0,10,
12,6,0,8,102,114,101,101,95,116,109,112,0,0,0,0,
13,5,6,0,15,6,2,0,31,4,6,1,19,4,5,4,
18,0,255,246,0,0,0,0,12,28,0,9,102,114,101,101,
95,116,109,112,115,0,0,0,14,28,27,0,30,4,0,169,
100,101,102,32,103,101,116,95,114,101,103,40,110,41,58,0,
16,28,0,78,44,8,0,0,30,4,0,169,100,101,102,32,
103,101,116,95,114,101,103,40,110,41,58,0,12,1,0,9,
101,110,99,111,100,101,46,112,121,0,0,0,33,1,0,0,
12,1,0,7,103,101,116,95,114,101,103,0,34,1,0,0,
28,2,0,0,9,1,0,2,30,6,0,170,32,32,32,32,
105,102,32,110,32,110,111,116,32,105,110,32,68,46,110,50,
114,58,0,0,12,3,0,1,68,0,0,0,13,2,3,0,
12,3,0,3,110,50,114,0,9,2,2,3,36,2,2,1,
11,3,0,0,0,0,0,0,0,0,0,0,23,2,2,3,
21,2,0,0,18,0,0,26,30,7,0,171,32,32,32,32,
32,32,32,32,115,101,116,95,114,101,103,40,97,108,108,111,
99,40,49,41,44,110,41,0,12,4,0,7,115,101,116,95,
114,101,103,0,13,3,4,0,12,7,0,5,97,108,108,111,
99,0,0,0,13,6,7,0,11,7,0,0,0,0,0,0,
0,0,240,63,31,4,7,1,19,4,6,4,15,5,1,0,
31,2,4,2,19,2,3,2,18,0,0,1,30,5,0,172,
32,32,32,32,114,101,116,117,114,110,32,68,46,110,50,114,
91,110,93,0,12,3,0,1,68,0,0,0,13,2,3,0,
12,3,0,3,110,50,114,0,9,2,2,3,9,2,2,1,
20,2,0,0,0,0,0,0,12,29,0,7,103,101,116,95,
114,101,103,0,14,29,28,0,30,5,0,177,100,101,102,32,
115,101,116,95,114,101,103,40,114,44,110,41,58,0,0,0,
16,29,0,77,44,9,0,0,30,5,0,177,100,101,102,32,
115,101,116,95,114,101,103,40,114,44,110,41,58,0,0,0,
12,1,0,9,101,110,99,111,100,101,46,112,121,0,0,0,
33,1,0,0,12,1,0,7,115,101,116,95,114,101,103,0,
34,1,0,0,28,2,0,0,9,1,0,2,28,3,0,0,
9,2,0,3,30,8,0,178,32,32,32,32,68,46,110,50,
114,91,110,93,32,61,32,114,59,32,68,46,114,50,110,91,
114,93,32,61,32,110,0,0,12,4,0,1,68,0,0,0,
13,3,4,0,12,4,0,3,110,50,114,0,9,3,3,4,
10,3,2,1,12,4,0,1,68,0,0,0,13,3,4,0,
12,4,0,3,114,50,110,0,9,3,3,4,10,3,1,2,
30,8,0,179,32,32,32,32,68,46,109,114,101,103,32,61,
32,109,97,120,40,68,46,109,114,101,103,44,114,43,49,41,
0,0,0,0,12,4,0,1,68,0,0,0,13,3,4,0,
12,6,0,3,109,97,120,0,13,5,6,0,12,8,0,1,
68,0,0,0,13,6,8,0,12,8,0,4,109,114,101,103,
0,0,0,0,9,6,6,8,11,8,0,0,0,0,0,0,
0,0,240,63,1,7,1,8,31,4,6,2,19,4,5,4,
12,5,0,4,109,114,101,103,0,0,0,0,10,3,5,4,
0,0,0,0,12,30,0,7,115,101,116,95,114,101,103,0,
14,30,29,0,30,5,0,180,100,101,102,32,102,114,101,101,
95,114,101,103,40,114,41,58,0,0,0,0,16,30,0,93,
44,5,0,0,30,5,0,180,100,101,102,32,102,114,101,101,
95,114,101,103,40,114,41,58,0,0,0,0,12,1,0,9,
101,110,99,111,100,101,46,112,121,0,0,0,33,1,0,0,
12,1,0,8,102,114,101,101,95,114,101,103,0,0,0,0,
34,1,0,0,28,2,0,0,9,1,0,2,30,8,0,181,
32,32,32,32,105,102,32,105,115,95,116,109,112,40,114,41,
58,32,68,46,116,109,112,99,32,45,61,32,49,0,0,0,
12,4,0,6,105,115,95,116,109,112,0,0,13,3,4,0,
15,4,1,0,31,2,4,1,19,2,3,2,21,2,0,0,
18,0,0,20,12,3,0,1,68,0,0,0,13,2,3,0,
12,4,0,1,68,0,0,0,13,3,4,0,12,4,0,4,
116,109,112,99,0,0,0,0,9,3,3,4,11,4,0,0,
0,0,0,0,0,0,240,63,2,3,3,4,12,4,0,4,
116,109,112,99,0,0,0,0,10,2,4,3,18,0,0,1,
30,12,0,182,32,32,32,32,110,32,61,32,68,46,114,50,
110,91,114,93,59,32,100,101,108,32,68,46,114,50,110,91,
114,93,59,32,100,101,108,32,68,46,110,50,114,91,110,93,
0,0,0,0,12,4,0,1,68,0,0,0,13,3,4,0,
12,4,0,3,114,50,110,0,9,3,3,4,9,3,3,1,
15,2,3,0,12,4,0,1,68,0,0,0,13,3,4,0,
12,4,0,3,114,50,110,0,9,3,3,4,43,3,1,0,
12,4,0,1,68,0,0,0,13,3,4,0,12,4,0,3,
110,50,114,0,9,3,3,4,43,3,2,0,0,0,0,0,
12,31,0,8,102,114,101,101,95,114,101,103,0,0,0,0,
14,31,30,0,30,6,0,184,100,101,102,32,105,109,97,110,
97,103,101,40,111,114,105,103,44,102,110,99,41,58,0,0,
16,31,0,102,44,14,0,0,30,6,0,184,100,101,102,32,
105,109,97,110,97,103,101,40,111,114,105,103,44,102,110,99,
41,58,0,0,12,1,0,9,101,110,99,111,100,101,46,112,
121,0,0,0,33,1,0,0,12,1,0,7,105,109,97,110,
97,103,101,0,34,1,0,0,28,2,0,0,9,1,0,2,
28,3,0,0,9,2,0,3,30,6,0,185,32,32,32,32,
105,116,101,109,115,32,61,32,111,114,105,103,46,105,116,101,
109,115,0,0,12,5,0,5,105,116,101,109,115,0,0,0,
9,4,1,5,15,3,4,0,30,8,0,186,32,32,32,32,
111,114,105,103,46,118,97,108,32,61,32,111,114,105,103,46,
118,97,108,91,58,45,49,93,0,0,0,0,12,5,0,3,
118,97,108,0,9,4,1,5,28,6,0,0,11,7,0,0,
0,0,0,0,0,0,240,191,27,5,6,2,9,4,4,5,
12,5,0,3,118,97,108,0,10,1,5,4,30,14,0,187,
32,32,32,32,116,32,61,32,84,111,107,101,110,40,111,114,
105,103,46,112,111,115,44,39,115,121,109,98,111,108,39,44,
39,61,39,44,91,105,116,101,109,115,91,48,93,44,111,114,
105,103,93,41,0,0,0,0,12,7,0,5,84,111,107,101,
110,0,0,0,13,6,7,0,12,11,0,3,112,111,115,0,
9,7,1,11,12,8,0,6,115,121,109,98,111,108,0,0,
12,9,0,1,61,0,0,0,11,13,0,0,0,0,0,0,
0,0,0,0,9,11,3,13,15,12,1,0,27,10,11,2,
31,5,7,4,19,5,6,5,15,4,5,0,30,5,0,188,
32,32,32,32,114,101,116,117,114,110,32,102,110,99,40,116,
41,0,0,0,15,6,4,0,31,5,6,1,19,5,2,5,
20,5,0,0,0,0,0,0,12,32,0,7,105,109,97,110,
97,103,101,0,14,32,31,0,30,6,0,190,100,101,102,32,
117,110,97,114,121,40,105,44,116,98,44,114,61,78,111,110,
101,41,58,0,16,32,0,92,44,10,0,0,30,6,0,190,
100,101,102,32,117,110,97,114,121,40,105,44,116,98,44,114,
61,78,111,110,101,41,58,0,12,1,0,9,101,110,99,111,
100,101,46,112,121,0,0,0,33,1,0,0,12,1,0,5,
117,110,97,114,121,0,0,0,34,1,0,0,28,2,0,0,
9,1,0,2,28,3,0,0,9,2,0,3,28,3,0,0,
28,4,0,0,32,3,0,4,30,5,0,191,32,32,32,32,
114,32,61,32,103,101,116,95,116,109,112,40,114,41,0,0,
12,6,0,7,103,101,116,95,116,109,112,0,13,5,6,0,
15,6,3,0,31,4,6,1,19,4,5,4,15,3,4,0,
30,4,0,192,32,32,32,32,98,32,61,32,100,111,40,116,
98,41,0,0,12,7,0,2,100,111,0,0,13,6,7,0,
15,7,2,0,31,5,7,1,19,5,6,5,15,4,5,0,
30,4,0,193,32,32,32,32,99,111,100,101,40,105,44,114,
44,98,41,0,12,7,0,4,99,111,100,101,0,0,0,0,
13,6,7,0,15,7,1,0,15,8,3,0,15,9,4,0,
31,5,7,3,19,5,6,5,30,7,0,194,32,32,32,32,
105,102,32,114,32,33,61,32,98,58,32,102,114,101,101,95,
116,109,112,40,98,41,0,0,35,5,3,4,21,5,0,0,
18,0,0,10,12,7,0,8,102,114,101,101,95,116,109,112,
0,0,0,0,13,6,7,0,15,7,4,0,31,5,7,1,
19,5,6,5,18,0,0,1,30,4,0,195,32,32,32,32,
114,101,116,117,114,110,32,114,0,0,0,0,20,3,0,0,
0,0,0,0,12,33,0,5,117,110,97,114,121,0,0,0,
14,33,32,0,30,7,0,196,100,101,102,32,105,110,102,105,
120,40,105,44,116,98,44,116,99,44,114,61,78,111,110,101,
41,58,0,0,16,33,0,123,44,13,0,0,30,7,0,196,
100,101,102,32,105,110,102,105,120,40,105,44,116,98,44,116,
99,44,114,61,78,111,110,101,41,58,0,0,12,1,0,9,
101,110,99,111,100,101,46,112,121,0,0,0,33,1,0,0,
12,1,0,5,105,110,102,105,120,0,0,0,34,1,0,0,
28,2,0,0,9,1,0,2,28,3,0,0,9,2,0,3,
28,4,0,0,9,3,0,4,28,4,0,0,28,5,0,0,
32,4,0,5,30,5,0,197,32,32,32,32,114,32,61,32,
103,101,116,95,116,109,112,40,114,41,0,0,12,7,0,7,
103,101,116,95,116,109,112,0,13,6,7,0,15,7,4,0,
31,5,7,1,19,5,6,5,15,4,5,0,30,7,0,198,
32,32,32,32,98,44,99,32,61,32,100,111,40,116,98,44,
114,41,44,100,111,40,116,99,41,0,0,0,12,8,0,2,
100,111,0,0,13,7,8,0,15,8,2,0,15,9,4,0,
31,6,8,2,19,6,7,6,15,5,6,0,12,9,0,2,
100,111,0,0,13,8,9,0,15,9,3,0,31,7,9,1,
19,7,8,7,15,6,7,0,15,7,5,0,15,5,6,0,
30,5,0,199,32,32,32,32,99,111,100,101,40,105,44,114,
44,98,44,99,41,0,0,0,12,9,0,4,99,111,100,101,
0,0,0,0,13,8,9,0,15,9,1,0,15,10,4,0,
15,11,7,0,15,12,5,0,31,6,9,4,19,6,8,6,
30,7,0,200,32,32,32,32,105,102,32,114,32,33,61,32,
98,58,32,102,114,101,101,95,116,109,112,40,98,41,0,0,
35,6,4,7,21,6,0,0,18,0,0,10,12,9,0,8,
102,114,101,101,95,116,109,112,0,0,0,0,13,8,9,0,
15,9,7,0,31,6,9,1,19,6,8,6,18,0,0,1,
30,4,0,201,32,32,32,32,102,114,101,101,95,116,109,112,
40,99,41,0,12,9,0,8,102,114,101,101,95,116,109,112,
0,0,0,0,13,8,9,0,15,9,5,0,31,6,9,1,
19,6,8,6,30,4,0,202,32,32,32,32,114,101,116,117,
114,110,32,114,0,0,0,0,20,4,0,0,0,0,0,0,
12,34,0,5,105,110,102,105,120,0,0,0,14,34,33,0,
30,10,0,203,100,101,102,32,108,111,103,105,99,95,105,110,
102,105,120,40,111,112,44,32,116,98,44,32,116,99,44,32,
95,114,61,78,111,110,101,41,58,0,0,0,16,34,0,210,
44,12,0,0,30,10,0,203,100,101,102,32,108,111,103,105,
99,95,105,110,102,105,120,40,111,112,44,32,116,98,44,32,
116,99,44,32,95,114,61,78,111,110,101,41,58,0,0,0,
12,1,0,9,101,110,99,111,100,101,46,112,121,0,0,0,
33,1,0,0,12,1,0,11,108,111,103,105,99,95,105,110,
102,105,120,0,34,1,0,0,28,2,0,0,9,1,0,2,
28,3,0,0,9,2,0,3,28,4,0,0,9,3,0,4,
28,4,0,0,28,5,0,0,32,4,0,5,30,5,0,204,
32,32,32,32,116,32,61,32,103,101,116,95,116,97,103,40,
41,32,0,0,12,8,0,7,103,101,116,95,116,97,103,0,
13,7,8,0,31,6,0,0,19,6,7,6,15,5,6,0,
30,5,0,205,32,32,32,32,114,32,61,32,100,111,40,116,
98,44,32,95,114,41,0,0,12,9,0,2,100,111,0,0,
13,8,9,0,15,9,2,0,15,10,4,0,31,7,9,2,
19,7,8,7,15,6,7,0,30,9,0,206,32,32,32,32,
105,102,32,95,114,32,33,61,32,114,58,32,102,114,101,101,
95,116,109,112,40,95,114,41,32,35,82,69,71,0,0,0,
35,7,4,6,21,7,0,0,18,0,0,10,12,9,0,8,
102,114,101,101,95,116,109,112,0,0,0,0,13,8,9,0,
15,9,4,0,31,7,9,1,19,7,8,7,18,0,0,1,
30,9,0,207,32,32,32,32,105,102,32,111,112,32,61,61,
32,39,97,110,100,39,58,32,32,32,99,111,100,101,40,73,
70,44,32,114,41,0,0,0,12,8,0,3,97,110,100,0,
23,7,1,8,21,7,0,0,18,0,0,12,12,9,0,4,
99,111,100,101,0,0,0,0,13,8,9,0,12,11,0,2,
73,70,0,0,13,9,11,0,15,10,6,0,31,7,9,2,
19,7,8,7,18,0,0,27,30,9,0,208,32,32,32,32,
101,108,105,102,32,111,112,32,61,61,32,39,111,114,39,58,
32,32,99,111,100,101,40,73,70,78,44,32,114,41,0,0,
12,8,0,2,111,114,0,0,23,7,1,8,21,7,0,0,
18,0,0,12,12,9,0,4,99,111,100,101,0,0,0,0,
13,8,9,0,12,11,0,3,73,70,78,0,13,9,11,0,
15,10,6,0,31,7,9,2,19,7,8,7,18,0,0,1,
30,5,0,209,32,32,32,32,106,117,109,112,40,116,44,32,
39,101,110,100,39,41,0,0,12,9,0,4,106,117,109,112,
0,0,0,0,13,8,9,0,15,9,5,0,12,10,0,3,
101,110,100,0,31,7,9,2,19,7,8,7,30,3,0,210,
32,32,32,32,95,114,32,61,32,114,0,0,15,4,6,0,
30,5,0,211,32,32,32,32,114,32,61,32,100,111,40,116,
99,44,32,95,114,41,0,0,12,9,0,2,100,111,0,0,
13,8,9,0,15,9,3,0,15,10,4,0,31,7,9,2,
19,7,8,7,15,6,7,0,30,9,0,212,32,32,32,32,
105,102,32,95,114,32,33,61,32,114,58,32,102,114,101,101,
95,116,109,112,40,95,114,41,32,35,82,69,71,0,0,0,
35,7,4,6,21,7,0,0,18,0,0,10,12,9,0,8,
102,114,101,101,95,116,109,112,0,0,0,0,13,8,9,0,
15,9,4,0,31,7,9,1,19,7,8,7,18,0,0,1,
30,5,0,213,32,32,32,32,116,97,103,40,116,44,32,39,
101,110,100,39,41,0,0,0,12,9,0,3,116,97,103,0,
13,8,9,0,15,9,5,0,12,10,0,3,101,110,100,0,
31,7,9,2,19,7,8,7,30,4,0,214,32,32,32,32,
114,101,116,117,114,110,32,114,0,0,0,0,20,6,0,0,
0,0,0,0,12,35,0,11,108,111,103,105,99,95,105,110,
102,105,120,0,14,35,34,0,30,6,0,216,100,101,102,32,
95,100,111,95,110,111,110,101,40,114,61,78,111,110,101,41,
58,0,0,0,16,35,0,60,44,7,0,0,30,6,0,216,
100,101,102,32,95,100,111,95,110,111,110,101,40,114,61,78,
111,110,101,41,58,0,0,0,12,1,0,9,101,110,99,111,
100,101,46,112,121,0,0,0,33,1,0,0,12,1,0,8,
95,100,111,95,110,111,110,101,0,0,0,0,34,1,0,0,
28,1,0,0,28,2,0,0,32,1,0,2,30,5,0,217,
32,32,32,32,114,32,61,32,103,101,116,95,116,109,112,40,
114,41,0,0,12,4,0,7,103,101,116,95,116,109,112,0,
13,3,4,0,15,4,1,0,31,2,4,1,19,2,3,2,
15,1,2,0,30,5,0,218,32,32,32,32,99,111,100,101,
40,78,79,78,69,44,114,41,0,0,0,0,12,4,0,4,
99,111,100,101,0,0,0,0,13,3,4,0,12,6,0,4,
78,79,78,69,0,0,0,0,13,4,6,0,15,5,1,0,
31,2,4,2,19,2,3,2,30,4,0,219,32,32,32,32,
114,101,116,117,114,110,32,114,0,0,0,0,20,1,0,0,
0,0,0,0,12,36,0,8,95,100,111,95,110,111,110,101,
0,0,0,0,14,36,35,0,30,7,0,221,100,101,102,32,
100,111,95,115,121,109,98,111,108,40,116,44,114,61,78,111,
110,101,41,58,0,0,0,0,16,36,3,191,44,31,0,0,
30,7,0,221,100,101,102,32,100,111,95,115,121,109,98,111,
108,40,116,44,114,61,78,111,110,101,41,58,0,0,0,0,
12,1,0,9,101,110,99,111,100,101,46,112,121,0,0,0,
33,1,0,0,12,1,0,9,100,111,95,115,121,109,98,111,
108,0,0,0,34,1,0,0,28,2,0,0,9,1,0,2,
28,2,0,0,28,3,0,0,32,2,0,3,30,5,0,222,
32,32,32,32,115,101,116,115,32,61,32,91,39,61,39,93,
0,0,0,0,12,5,0,1,61,0,0,0,27,4,5,1,
15,3,4,0,30,13,0,223,32,32,32,32,105,115,101,116,
115,32,61,32,91,39,43,61,39,44,39,45,61,39,44,39,
42,61,39,44,39,47,61,39,44,32,39,124,61,39,44,32,
39,38,61,39,44,32,39,94,61,39,93,0,12,6,0,2,
43,61,0,0,12,7,0,2,45,61,0,0,12,8,0,2,
42,61,0,0,12,9,0,2,47,61,0,0,12,10,0,2,
124,61,0,0,12,11,0,2,38,61,0,0,12,12,0,2,
94,61,0,0,27,5,6,7,15,4,5,0,30,11,0,224,
32,32,32,32,99,109,112,115,32,61,32,91,39,60,39,44,
39,62,39,44,39,60,61,39,44,39,62,61,39,44,39,61,
61,39,44,39,33,61,39,93,0,0,0,0,12,7,0,1,
60,0,0,0,12,8,0,1,62,0,0,0,12,9,0,2,
60,61,0,0,12,10,0,2,62,61,0,0,12,11,0,2,
61,61,0,0,12,12,0,2,33,61,0,0,27,6,7,6,
15,5,6,0,30,4,0,225,32,32,32,32,109,101,116,97,
115,32,61,32,123,0,0,0,30,11,0,226,32,32,32,32,
32,32,32,32,39,43,39,58,65,68,68,44,39,42,39,58,
77,85,76,44,39,47,39,58,68,73,86,44,39,42,42,39,
58,80,79,87,44,0,0,0,12,8,0,1,43,0,0,0,
12,30,0,3,65,68,68,0,13,9,30,0,12,10,0,1,
42,0,0,0,12,30,0,3,77,85,76,0,13,11,30,0,
12,12,0,1,47,0,0,0,12,30,0,3,68,73,86,0,
13,13,30,0,12,14,0,2,42,42,0,0,12,30,0,3,
80,79,87,0,13,15,30,0,30,5,0,227,32,32,32,32,
32,32,32,32,39,45,39,58,83,85,66,44,0,0,0,0,
12,16,0,1,45,0,0,0,12,30,0,3,83,85,66,0,
13,17,30,0,30,9,0,228,32,32,32,32,32,32,32,32,
39,37,39,58,77,79,68,44,39,62,62,39,58,82,83,72,
44,39,60,60,39,58,76,83,72,44,0,0,12,18,0,1,
37,0,0,0,12,30,0,3,77,79,68,0,13,19,30,0,
12,20,0,2,62,62,0,0,12,30,0,3,82,83,72,0,
13,21,30,0,12,22,0,2,60,60,0,0,12,30,0,3,
76,83,72,0,13,23,30,0,30,11,0,229,32,32,32,32,
32,32,32,32,39,38,39,58,66,73,84,65,78,68,44,39,
124,39,58,66,73,84,79,82,44,39,94,39,58,66,73,84,
88,79,82,44,0,0,0,0,12,24,0,1,38,0,0,0,
12,30,0,6,66,73,84,65,78,68,0,0,13,25,30,0,
12,26,0,1,124,0,0,0,12,30,0,5,66,73,84,79,
82,0,0,0,13,27,30,0,12,28,0,1,94,0,0,0,
12,30,0,6,66,73,84,88,79,82,0,0,13,29,30,0,
26,7,8,22,15,6,7,0,30,11,0,231,32,32,32,32,
105,102,32,116,46,118,97,108,32,61,61,32,39,78,111,110,
101,39,58,32,114,101,116,117,114,110,32,95,100,111,95,110,
111,110,101,40,114,41,0,0,12,8,0,3,118,97,108,0,
9,7,1,8,12,8,0,4,78,111,110,101,0,0,0,0,
23,7,7,8,21,7,0,0,18,0,0,11,12,9,0,8,
95,100,111,95,110,111,110,101,0,0,0,0,13,8,9,0,
15,9,2,0,31,7,9,1,19,7,8,7,20,7,0,0,
18,0,0,1,30,6,0,232,32,32,32,32,105,102,32,116,
46,118,97,108,32,61,61,32,39,84,114,117,101,39,58,0,
12,8,0,3,118,97,108,0,9,7,1,8,12,8,0,4,
84,114,117,101,0,0,0,0,23,7,7,8,21,7,0,0,
18,0,0,23,30,9,0,233,32,32,32,32,32,32,32,32,
114,101,116,117,114,110,32,95,100,111,95,110,117,109,98,101,
114,40,39,49,39,44,114,41,0,0,0,0,12,9,0,10,
95,100,111,95,110,117,109,98,101,114,0,0,13,8,9,0,
12,9,0,1,49,0,0,0,15,10,2,0,31,7,9,2,
19,7,8,7,20,7,0,0,18,0,0,1,30,7,0,234,
32,32,32,32,105,102,32,116,46,118,97,108,32,61,61,32,
39,70,97,108,115,101,39,58,0,0,0,0,12,8,0,3,
118,97,108,0,9,7,1,8,12,8,0,5,70,97,108,115,
101,0,0,0,23,7,7,8,21,7,0,0,18,0,0,23,
30,9,0,235,32,32,32,32,32,32,32,32,114,101,116,117,
114,110,32,95,100,111,95,110,117,109,98,101,114,40,39,48,
39,44,114,41,0,0,0,0,12,9,0,10,95,100,111,95,
110,117,109,98,101,114,0,0,13,8,9,0,12,9,0,1,
48,0,0,0,15,10,2,0,31,7,9,2,19,7,8,7,
20,7,0,0,18,0,0,1,30,5,0,236,32,32,32,32,
105,116,101,109,115,32,61,32,116,46,105,116,101,109,115,0,
12,9,0,5,105,116,101,109,115,0,0,0,9,8,1,9,
15,7,8,0,30,8,0,238,32,32,32,32,105,102,32,116,
46,118,97,108,32,105,110,32,91,39,97,110,100,39,44,39,
111,114,39,93,58,0,0,0,12,9,0,3,97,110,100,0,
12,10,0,2,111,114,0,0,27,8,9,2,12,10,0,3,
118,97,108,0,9,9,1,10,36,8,8,9,21,8,0,0,
18,0,0,38,30,15,0,239,32,32,32,32,32,32,32,32,
114,101,116,117,114,110,32,108,111,103,105,99,95,105,110,102,
105,120,40,116,46,118,97,108,44,32,105,116,101,109,115,91,
48,93,44,32,105,116,101,109,115,91,49,93,44,32,114,41,
0,0,0,0,12,10,0,11,108,111,103,105,99,95,105,110,
102,105,120,0,13,9,10,0,12,14,0,3,118,97,108,0,
9,10,1,14,11,14,0,0,0,0,0,0,0,0,0,0,
9,11,7,14,11,14,0,0,0,0,0,0,0,0,240,63,
9,12,7,14,15,13,2,0,31,8,10,4,19,8,9,8,
20,8,0,0,18,0,0,1,30,6,0,240,32,32,32,32,
105,102,32,116,46,118,97,108,32,105,110,32,105,115,101,116,
115,58,0,0,12,10,0,3,118,97,108,0,9,9,1,10,
36,8,4,9,21,8,0,0,18,0,0,25,30,9,0,241,
32,32,32,32,32,32,32,32,114,101,116,117,114,110,32,105,
109,97,110,97,103,101,40,116,44,100,111,95,115,121,109,98,
111,108,41,0,12,10,0,7,105,109,97,110,97,103,101,0,
13,9,10,0,15,10,1,0,12,12,0,9,100,111,95,115,
121,109,98,111,108,0,0,0,13,11,12,0,31,8,10,2,
19,8,9,8,20,8,0,0,18,0,0,1,30,6,0,242,
32,32,32,32,105,102,32,116,46,118,97,108,32,61,61,32,
39,105,115,39,58,0,0,0,12,9,0,3,118,97,108,0,
9,8,1,9,12,9,0,2,105,115,0,0,23,8,8,9,
21,8,0,0,18,0,0,34,30,12,0,243,32,32,32,32,
32,32,32,32,114,101,116,117,114,110,32,105,110,102,105,120,
40,69,81,44,105,116,101,109,115,91,48,93,44,105,116,101,
109,115,91,49,93,44,114,41,0,0,0,0,12,10,0,5,
105,110,102,105,120,0,0,0,13,9,10,0,12,14,0,2,
69,81,0,0,13,10,14,0,11,14,0,0,0,0,0,0,
0,0,0,0,9,11,7,14,11,14,0,0,0,0,0,0,
0,0,240,63,9,12,7,14,15,13,2,0,31,8,10,4,
19,8,9,8,20,8,0,0,18,0,0,1,30,7,0,244,
32,32,32,32,105,102,32,116,46,118,97,108,32,61,61,32,
39,105,115,110,111,116,39,58,0,0,0,0,12,9,0,3,
118,97,108,0,9,8,1,9,12,9,0,5,105,115,110,111,
116,0,0,0,23,8,8,9,21,8,0,0,18,0,0,34,
30,12,0,245,32,32,32,32,32,32,32,32,114,101,116,117,
114,110,32,105,110,102,105,120,40,67,77,80,44,105,116,101,
109,115,91,48,93,44,105,116,101,109,115,91,49,93,44,114,
41,0,0,0,12,10,0,5,105,110,102,105,120,0,0,0,
13,9,10,0,12,14,0,3,67,77,80,0,13,10,14,0,
11,14,0,0,0,0,0,0,0,0,0,0,9,11,7,14,
11,14,0,0,0,0,0,0,0,0,240,63,9,12,7,14,
15,13,2,0,31,8,10,4,19,8,9,8,20,8,0,0,
18,0,0,1,30,6,0,246,32,32,32,32,105,102,32,116,
46,118,97,108,32,61,61,32,39,110,111,116,39,58,0,0,
12,9,0,3,118,97,108,0,9,8,1,9,12,9,0,3,
110,111,116,0,23,8,8,9,21,8,0,0,18,0,0,28,
30,10,0,247,32,32,32,32,32,32,32,32,114,101,116,117,
114,110,32,117,110,97,114,121,40,78,79,84,44,32,105,116,
101,109,115,91,48,93,44,32,114,41,0,0,12,10,0,5,
117,110,97,114,121,0,0,0,13,9,10,0,12,13,0,3,
78,79,84,0,13,10,13,0,11,13,0,0,0,0,0,0,
0,0,0,0,9,11,7,13,15,12,2,0,31,8,10,3,
19,8,9,8,20,8,0,0,18,0,0,1,30,6,0,248,
32,32,32,32,105,102,32,116,46,118,97,108,32,61,61,32,
39,105,110,39,58,0,0,0,12,9,0,3,118,97,108,0,
9,8,1,9,12,9,0,2,105,110,0,0,23,8,8,9,
21,8,0,0,18,0,0,34,30,12,0,249,32,32,32,32,
32,32,32,32,114,101,116,117,114,110,32,105,110,102,105,120,
40,72,65,83,44,105,116,101,109,115,91,49,93,44,105,116,
101,109,115,91,48,93,44,114,41,0,0,0,12,10,0,5,
105,110,102,105,120,0,0,0,13,9,10,0,12,14,0,3,
72,65,83,0,13,10,14,0,11,14,0,0,0,0,0,0,
0,0,240,63,9,11,7,14,11,14,0,0,0,0,0,0,
0,0,0,0,9,12,7,14,15,13,2,0,31,8,10,4,
19,8,9,8,20,8,0,0,18,0,0,1,30,7,0,250,
32,32,32,32,105,102,32,116,46,118,97,108,32,61,61,32,
39,110,111,116,105,110,39,58,0,0,0,0,12,9,0,3,
118,97,108,0,9,8,1,9,12,9,0,5,110,111,116,105,
110,0,0,0,23,8,8,9,21,8,0,0,18,0,0,88,
30,11,0,251,32,32,32,32,32,32,32,32,114,32,61,32,
105,110,102,105,120,40,72,65,83,44,105,116,101,109,115,91,
49,93,44,105,116,101,109,115,91,48,93,44,114,41,0,0,
12,10,0,5,105,110,102,105,120,0,0,0,13,9,10,0,
12,14,0,3,72,65,83,0,13,10,14,0,11,14,0,0,
0,0,0,0,0,0,240,63,9,11,7,14,11,14,0,0,
0,0,0,0,0,0,0,0,9,12,7,14,15,13,2,0,
31,8,10,4,19,8,9,8,15,2,8,0,30,8,0,252,
32,32,32,32,32,32,32,32,122,101,114,111,32,61,32,95,
100,111,95,110,117,109,98,101,114,40,39,48,39,41,0,0,
12,11,0,10,95,100,111,95,110,117,109,98,101,114,0,0,
13,10,11,0,12,11,0,1,48,0,0,0,31,9,11,1,
19,9,10,9,15,8,9,0,30,9,0,253,32,32,32,32,
32,32,32,32,99,111,100,101,40,69,81,44,114,44,114,44,
102,114,101,101,95,116,109,112,40,122,101,114,111,41,41,0,
12,11,0,4,99,111,100,101,0,0,0,0,13,10,11,0,
12,15,0,2,69,81,0,0,13,11,15,0,15,12,2,0,
15,13,2,0,12,16,0,8,102,114,101,101,95,116,109,112,
0,0,0,0,13,15,16,0,15,16,8,0,31,14,16,1,
19,14,15,14,31,9,11,4,19,9,10,9,30,5,0,254,
32,32,32,32,32,32,32,32,114,101,116,117,114,110,32,114,
0,0,0,0,20,2,0,0,18,0,0,1,30,6,0,255,
32,32,32,32,105,102,32,116,46,118,97,108,32,105,110,32,
115,101,116,115,58,0,0,0,12,11,0,3,118,97,108,0,
9,10,1,11,36,9,3,10,21,9,0,0,18,0,0,31,
30,12,1,0,32,32,32,32,32,32,32,32,114,101,116,117,
114,110,32,100,111,95,115,101,116,95,99,116,120,40,105,116,
101,109,115,91,48,93,44,105,116,101,109,115,91,49,93,41,
59,0,0,0,12,11,0,10,100,111,95,115,101,116,95,99,
116,120,0,0,13,10,11,0,11,13,0,0,0,0,0,0,
0,0,0,0,9,11,7,13,11,13,0,0,0,0,0,0,
0,0,240,63,9,12,7,13,31,9,11,2,19,9,10,9,
20,9,0,0,18,0,0,215,30,6,1,1,32,32,32,32,
101,108,105,102,32,116,46,118,97,108,32,105,110,32,99,109,
112,115,58,0,12,11,0,3,118,97,108,0,9,10,1,11,
36,9,5,10,21,9,0,0,18,0,0,166,30,8,1,2,
32,32,32,32,32,32,32,32,98,44,99,32,61,32,105,116,
101,109,115,91,48,93,44,105,116,101,109,115,91,49,93,0,
11,11,0,0,0,0,0,0,0,0,0,0,9,10,7,11,
15,9,10,0,11,12,0,0,0,0,0,0,0,0,240,63,
9,11,7,12,15,10,11,0,15,11,9,0,15,9,10,0,
30,5,1,3,32,32,32,32,32,32,32,32,118,32,61,32,
116,46,118,97,108,0,0,0,12,13,0,3,118,97,108,0,
9,12,1,13,15,10,12,0,30,8,1,4,32,32,32,32,
32,32,32,32,105,102,32,118,91,48,93,32,105,110,32,40,
39,62,39,44,39,62,61,39,41,58,0,0,12,13,0,1,
62,0,0,0,12,14,0,2,62,61,0,0,27,12,13,2,
11,14,0,0,0,0,0,0,0,0,0,0,9,13,10,14,
36,12,12,13,21,12,0,0,18,0,0,27,30,9,1,5,
32,32,32,32,32,32,32,32,32,32,32,32,98,44,99,44,
118,32,61,32,99,44,98,44,39,60,39,43,118,91,49,58,
93,0,0,0,15,12,9,0,15,13,11,0,12,15,0,1,
60,0,0,0,11,18,0,0,0,0,0,0,0,0,240,63,
28,19,0,0,27,17,18,2,9,16,10,17,1,15,15,16,
15,14,15,0,15,11,12,0,15,9,13,0,15,10,14,0,
18,0,0,1,30,4,1,6,32,32,32,32,32,32,32,32,
99,100,32,61,32,69,81,0,12,14,0,2,69,81,0,0,
13,13,14,0,15,12,13,0,30,8,1,7,32,32,32,32,
32,32,32,32,105,102,32,118,32,61,61,32,39,60,39,58,
32,99,100,32,61,32,76,84,0,0,0,0,12,14,0,1,
60,0,0,0,23,13,10,14,21,13,0,0,18,0,0,6,
12,14,0,2,76,84,0,0,13,13,14,0,15,12,13,0,
18,0,0,1,30,8,1,8,32,32,32,32,32,32,32,32,
105,102,32,118,32,61,61,32,39,60,61,39,58,32,99,100,
32,61,32,76,69,0,0,0,12,14,0,2,60,61,0,0,
23,13,10,14,21,13,0,0,18,0,0,6,12,14,0,2,
76,69,0,0,13,13,14,0,15,12,13,0,18,0,0,1,
30,8,1,9,32,32,32,32,32,32,32,32,105,102,32,118,
32,61,61,32,39,33,61,39,58,32,99,100,32,61,32,78,
69,0,0,0,12,14,0,2,33,61,0,0,23,13,10,14,
21,13,0,0,18,0,0,6,12,14,0,2,78,69,0,0,
13,13,14,0,15,12,13,0,18,0,0,1,30,8,1,10,
32,32,32,32,32,32,32,32,114,101,116,117,114,110,32,105,
110,102,105,120,40,99,100,44,98,44,99,44,114,41,0,0,
12,15,0,5,105,110,102,105,120,0,0,0,13,14,15,0,
15,15,12,0,15,16,11,0,15,17,9,0,15,18,2,0,
31,13,15,4,19,13,14,13,20,13,0,0,18,0,0,37,
30,14,1,12,32,32,32,32,32,32,32,32,114,101,116,117,
114,110,32,105,110,102,105,120,40,109,101,116,97,115,91,116,
46,118,97,108,93,44,105,116,101,109,115,91,48,93,44,105,
116,101,109,115,91,49,93,44,114,41,0,0,12,15,0,5,
105,110,102,105,120,0,0,0,13,14,15,0,12,20,0,3,
118,97,108,0,9,19,1,20,9,15,6,19,11,19,0,0,
0,0,0,0,0,0,0,0,9,16,7,19,11,19,0,0,
0,0,0,0,0,0,240,63,9,17,7,19,15,18,2,0,
31,13,15,4,19,13,14,13,20,13,0,0,18,0,0,1,
0,0,0,0,12,37,0,9,100,111,95,115,121,109,98,111,
108,0,0,0,14,37,36,0,30,6,1,14,100,101,102,32,
100,111,95,115,101,116,95,99,116,120,40,107,44,118,41,58,
0,0,0,0,16,37,3,102,44,34,0,0,30,6,1,14,
100,101,102,32,100,111,95,115,101,116,95,99,116,120,40,107,
44,118,41,58,0,0,0,0,12,1,0,9,101,110,99,111,
100,101,46,112,121,0,0,0,33,1,0,0,12,1,0,10,
100,111,95,115,101,116,95,99,116,120,0,0,34,1,0,0,
28,2,0,0,9,1,0,2,28,3,0,0,9,2,0,3,
30,7,1,15,32,32,32,32,105,102,32,107,46,116,121,112,
101,32,61,61,32,39,110,97,109,101,39,58,0,0,0,0,
12,4,0,4,116,121,112,101,0,0,0,0,9,3,1,4,
12,4,0,4,110,97,109,101,0,0,0,0,23,3,3,4,
21,3,0,0,18,0,0,222,30,19,1,16,32,32,32,32,
32,32,32,32,105,102,32,40,68,46,95,103,108,111,98,97,
108,115,32,97,110,100,32,107,46,118,97,108,32,110,111,116,
32,105,110,32,68,46,118,97,114,115,41,32,111,114,32,40,
107,46,118,97,108,32,105,110,32,68,46,103,108,111,98,97,
108,115,41,58,0,0,0,0,12,4,0,1,68,0,0,0,
13,3,4,0,12,4,0,8,95,103,108,111,98,97,108,115,
0,0,0,0,9,3,3,4,21,3,0,0,18,0,0,16,
12,4,0,1,68,0,0,0,13,3,4,0,12,4,0,4,
118,97,114,115,0,0,0,0,9,3,3,4,12,5,0,3,
118,97,108,0,9,4,1,5,36,3,3,4,11,4,0,0,
0,0,0,0,0,0,0,0,23,3,3,4,46,3,0,0,
18,0,0,12,12,4,0,1,68,0,0,0,13,3,4,0,
12,4,0,7,103,108,111,98,97,108,115,0,9,3,3,4,
12,5,0,3,118,97,108,0,9,4,1,5,36,3,3,4,
21,3,0,0,18,0,0,92,30,8,1,17,32,32,32,32,
32,32,32,32,32,32,32,32,99,32,61,32,100,111,95,115,
116,114,105,110,103,40,107,41,0,0,0,0,12,6,0,9,
100,111,95,115,116,114,105,110,103,0,0,0,13,5,6,0,
15,6,1,0,31,4,6,1,19,4,5,4,15,3,4,0,
30,6,1,18,32,32,32,32,32,32,32,32,32,32,32,32,
98,32,61,32,100,111,40,118,41,0,0,0,12,7,0,2,
100,111,0,0,13,6,7,0,15,7,2,0,31,5,7,1,
19,5,6,5,15,4,5,0,30,7,1,19,32,32,32,32,
32,32,32,32,32,32,32,32,99,111,100,101,40,71,83,69,
84,44,99,44,98,41,0,0,12,7,0,4,99,111,100,101,
0,0,0,0,13,6,7,0,12,10,0,4,71,83,69,84,
0,0,0,0,13,7,10,0,15,8,3,0,15,9,4,0,
31,5,7,3,19,5,6,5,30,6,1,20,32,32,32,32,
32,32,32,32,32,32,32,32,102,114,101,101,95,116,109,112,
40,99,41,0,12,7,0,8,102,114,101,101,95,116,109,112,
0,0,0,0,13,6,7,0,15,7,3,0,31,5,7,1,
19,5,6,5,30,6,1,21,32,32,32,32,32,32,32,32,
32,32,32,32,102,114,101,101,95,116,109,112,40,98,41,0,
12,7,0,8,102,114,101,101,95,116,109,112,0,0,0,0,
13,6,7,0,15,7,4,0,31,5,7,1,19,5,6,5,
30,5,1,22,32,32,32,32,32,32,32,32,32,32,32,32,
114,101,116,117,114,110,0,0,28,5,0,0,20,5,0,0,
18,0,0,1,30,6,1,23,32,32,32,32,32,32,32,32,
97,32,61,32,100,111,95,108,111,99,97,108,40,107,41,0,
12,8,0,8,100,111,95,108,111,99,97,108,0,0,0,0,
13,7,8,0,15,8,1,0,31,6,8,1,19,6,7,6,
15,5,6,0,30,5,1,24,32,32,32,32,32,32,32,32,
98,32,61,32,100,111,40,118,41,0,0,0,12,8,0,2,
100,111,0,0,13,7,8,0,15,8,2,0,31,6,8,1,
19,6,7,6,15,4,6,0,30,6,1,25,32,32,32,32,
32,32,32,32,99,111,100,101,40,77,79,86,69,44,97,44,
98,41,0,0,12,8,0,4,99,111,100,101,0,0,0,0,
13,7,8,0,12,11,0,4,77,79,86,69,0,0,0,0,
13,8,11,0,15,9,5,0,15,10,4,0,31,6,8,3,
19,6,7,6,30,5,1,26,32,32,32,32,32,32,32,32,
102,114,101,101,95,116,109,112,40,98,41,0,12,8,0,8,
102,114,101,101,95,116,109,112,0,0,0,0,13,7,8,0,
15,8,4,0,31,6,8,1,19,6,7,6,30,5,1,27,
32,32,32,32,32,32,32,32,114,101,116,117,114,110,32,97,
0,0,0,0,20,5,0,0,18,0,1,242,30,10,1,28,
32,32,32,32,101,108,105,102,32,107,46,116,121,112,101,32,
105,110,32,40,39,116,117,112,108,101,39,44,39,108,105,115,
116,39,41,58,0,0,0,0,12,7,0,5,116,117,112,108,
101,0,0,0,12,8,0,4,108,105,115,116,0,0,0,0,
27,6,7,2,12,8,0,4,116,121,112,101,0,0,0,0,
9,7,1,8,36,6,6,7,21,6,0,0,18,0,1,217,
30,10,1,29,32,32,32,32,32,32,32,32,105,102,32,118,
46,116,121,112,101,32,105,110,32,40,39,116,117,112,108,101,
39,44,39,108,105,115,116,39,41,58,0,0,12,7,0,5,
116,117,112,108,101,0,0,0,12,8,0,4,108,105,115,116,
0,0,0,0,27,6,7,2,12,8,0,4,116,121,112,101,
0,0,0,0,9,7,2,8,36,6,6,7,21,6,0,0,
18,0,1,12,30,7,1,30,32,32,32,32,32,32,32,32,
32,32,32,32,110,44,116,109,112,115,32,61,32,48,44,91,
93,0,0,0,11,7,0,0,0,0,0,0,0,0,0,0,
15,6,7,0,27,8,0,0,15,7,8,0,15,8,6,0,
15,6,7,0,30,8,1,31,32,32,32,32,32,32,32,32,
32,32,32,32,102,111,114,32,107,107,32,105,110,32,107,46,
105,116,101,109,115,58,0,0,12,10,0,5,105,116,101,109,
115,0,0,0,9,9,1,10,11,10,0,0,0,0,0,0,
0,0,0,0,42,7,9,10,18,0,0,112,30,8,1,32,
32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,
118,118,32,61,32,118,46,105,116,101,109,115,91,110,93,0,
12,13,0,5,105,116,101,109,115,0,0,0,9,12,2,13,
9,12,12,8,15,11,12,0,30,13,1,33,32,32,32,32,
32,32,32,32,32,32,32,32,32,32,32,32,116,109,112,32,
61,32,103,101,116,95,116,109,112,40,41,59,32,116,109,112,
115,46,97,112,112,101,110,100,40,116,109,112,41,0,0,0,
12,15,0,7,103,101,116,95,116,109,112,0,13,14,15,0,
31,13,0,0,19,13,14,13,15,12,13,0,12,15,0,6,
97,112,112,101,110,100,0,0,9,14,6,15,15,15,12,0,
31,13,15,1,19,13,14,13,30,7,1,34,32,32,32,32,
32,32,32,32,32,32,32,32,32,32,32,32,114,32,61,32,
100,111,40,118,118,41,0,0,12,16,0,2,100,111,0,0,
13,15,16,0,15,16,11,0,31,14,16,1,19,14,15,14,
15,13,14,0,30,9,1,35,32,32,32,32,32,32,32,32,
32,32,32,32,32,32,32,32,99,111,100,101,40,77,79,86,
69,44,116,109,112,44,114,41,0,0,0,0,12,16,0,4,
99,111,100,101,0,0,0,0,13,15,16,0,12,19,0,4,
77,79,86,69,0,0,0,0,13,16,19,0,15,17,12,0,
15,18,13,0,31,14,16,3,19,14,15,14,30,9,1,36,
32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,
102,114,101,101,95,116,109,112,40,114,41,32,35,82,69,71,
0,0,0,0,12,16,0,8,102,114,101,101,95,116,109,112,
0,0,0,0,13,15,16,0,15,16,13,0,31,14,16,1,
19,14,15,14,30,6,1,37,32,32,32,32,32,32,32,32,
32,32,32,32,32,32,32,32,110,43,61,49,0,0,0,0,
11,15,0,0,0,0,0,0,0,0,240,63,1,14,8,15,
15,8,14,0,18,0,255,144,30,5,1,38,32,32,32,32,
32,32,32,32,32,32,32,32,110,32,61,32,48,0,0,0,
11,9,0,0,0,0,0,0,0,0,0,0,15,8,9,0,
30,8,1,39,32,32,32,32,32,32,32,32,32,32,32,32,
102,111,114,32,107,107,32,105,110,32,107,46,105,116,101,109,
115,58,0,0,12,10,0,5,105,116,101,109,115,0,0,0,
9,9,1,10,11,10,0,0,0,0,0,0,0,0,0,0,
42,7,9,10,18,0,0,86,30,8,1,40,32,32,32,32,
32,32,32,32,32,32,32,32,32,32,32,32,118,118,32,61,
32,118,46,105,116,101,109,115,91,110,93,0,12,15,0,5,
105,116,101,109,115,0,0,0,9,14,2,15,9,14,14,8,
15,11,14,0,30,8,1,41,32,32,32,32,32,32,32,32,
32,32,32,32,32,32,32,32,116,109,112,32,61,32,116,109,
112,115,91,110,93,0,0,0,9,14,6,8,15,12,14,0,
30,18,1,42,32,32,32,32,32,32,32,32,32,32,32,32,
32,32,32,32,102,114,101,101,95,116,109,112,40,100,111,95,
115,101,116,95,99,116,120,40,107,107,44,84,111,107,101,110,
40,118,118,46,112,111,115,44,39,114,101,103,39,44,116,109,
112,41,41,41,32,35,82,69,71,0,0,0,12,16,0,8,
102,114,101,101,95,116,109,112,0,0,0,0,13,15,16,0,
12,18,0,10,100,111,95,115,101,116,95,99,116,120,0,0,
13,17,18,0,15,18,7,0,12,21,0,5,84,111,107,101,
110,0,0,0,13,20,21,0,12,24,0,3,112,111,115,0,
9,21,11,24,12,22,0,3,114,101,103,0,15,23,12,0,
31,19,21,3,19,19,20,19,31,16,18,2,19,16,17,16,
31,14,16,1,19,14,15,14,30,6,1,43,32,32,32,32,
32,32,32,32,32,32,32,32,32,32,32,32,110,32,43,61,
32,49,0,0,11,15,0,0,0,0,0,0,0,0,240,63,
1,14,8,15,15,8,14,0,18,0,255,170,30,5,1,44,
32,32,32,32,32,32,32,32,32,32,32,32,114,101,116,117,
114,110,0,0,28,9,0,0,20,9,0,0,18,0,0,1,
30,8,1,46,32,32,32,32,32,32,32,32,114,32,61,32,
100,111,40,118,41,59,32,117,110,95,116,109,112,40,114,41,
0,0,0,0,12,14,0,2,100,111,0,0,13,10,14,0,
15,14,2,0,31,9,14,1,19,9,10,9,15,13,9,0,
12,14,0,6,117,110,95,116,109,112,0,0,13,10,14,0,
15,14,13,0,31,9,14,1,19,9,10,9,30,11,1,47,
32,32,32,32,32,32,32,32,110,44,32,116,109,112,32,61,
32,48,44,32,84,111,107,101,110,40,118,46,112,111,115,44,
39,114,101,103,39,44,114,41,0,0,0,0,11,10,0,0,
0,0,0,0,0,0,0,0,15,9,10,0,12,16,0,5,
84,111,107,101,110,0,0,0,13,15,16,0,12,19,0,3,
112,111,115,0,9,16,2,19,12,17,0,3,114,101,103,0,
15,18,13,0,31,14,16,3,19,14,15,14,15,10,14,0,
15,8,9,0,15,12,10,0,30,7,1,48,32,32,32,32,
32,32,32,32,102,111,114,32,116,116,32,105,110,32,107,46,
105,116,101,109,115,58,0,0,12,14,0,5,105,116,101,109,
115,0,0,0,9,10,1,14,11,14,0,0,0,0,0,0,
0,0,0,0,42,9,10,14,18,0,0,88,30,27,1,49,
32,32,32,32,32,32,32,32,32,32,32,32,102,114,101,101,
95,116,109,112,40,100,111,95,115,101,116,95,99,116,120,40,
116,116,44,84,111,107,101,110,40,116,109,112,46,112,111,115,
44,39,103,101,116,39,44,78,111,110,101,44,91,116,109,112,
44,84,111,107,101,110,40,116,109,112,46,112,111,115,44,39,
110,117,109,98,101,114,39,44,115,116,114,40,110,41,41,93,
41,41,41,32,35,82,69,71,0,0,0,0,12,17,0,8,
102,114,101,101,95,116,109,112,0,0,0,0,13,16,17,0,
12,19,0,10,100,111,95,115,101,116,95,99,116,120,0,0,
13,18,19,0,15,19,9,0,12,22,0,5,84,111,107,101,
110,0,0,0,13,21,22,0,12,26,0,3,112,111,115,0,
9,22,12,26,12,23,0,3,103,101,116,0,28,24,0,0,
15,26,12,0,12,29,0,5,84,111,107,101,110,0,0,0,
13,28,29,0,12,32,0,3,112,111,115,0,9,29,12,32,
12,30,0,6,110,117,109,98,101,114,0,0,12,33,0,3,
115,116,114,0,13,32,33,0,15,33,8,0,31,31,33,1,
19,31,32,31,31,27,29,3,19,27,28,27,27,25,26,2,
31,20,22,4,19,20,21,20,31,17,19,2,19,17,18,17,
31,15,17,1,19,15,16,15,30,5,1,50,32,32,32,32,
32,32,32,32,32,32,32,32,110,32,43,61,32,49,0,0,
11,16,0,0,0,0,0,0,0,0,240,63,1,15,8,16,
15,8,15,0,18,0,255,168,30,5,1,51,32,32,32,32,
32,32,32,32,102,114,101,101,95,114,101,103,40,114,41,0,
12,15,0,8,102,114,101,101,95,114,101,103,0,0,0,0,
13,14,15,0,15,15,13,0,31,10,15,1,19,10,14,10,
30,4,1,52,32,32,32,32,32,32,32,32,114,101,116,117,
114,110,0,0,28,10,0,0,20,10,0,0,18,0,0,1,
30,6,1,53,32,32,32,32,114,32,61,32,100,111,40,107,
46,105,116,101,109,115,91,48,93,41,0,0,12,15,0,2,
100,111,0,0,13,14,15,0,12,16,0,5,105,116,101,109,
115,0,0,0,9,15,1,16,11,16,0,0,0,0,0,0,
0,0,0,0,9,15,15,16,31,10,15,1,19,10,14,10,
15,13,10,0,30,4,1,54,32,32,32,32,114,114,32,61,
32,100,111,40,118,41,0,0,12,16,0,2,100,111,0,0,
13,15,16,0,15,16,2,0,31,14,16,1,19,14,15,14,
15,10,14,0,30,7,1,55,32,32,32,32,116,109,112,32,
61,32,100,111,40,107,46,105,116,101,109,115,91,49,93,41,
0,0,0,0,12,16,0,2,100,111,0,0,13,15,16,0,
12,17,0,5,105,116,101,109,115,0,0,0,9,16,1,17,
11,17,0,0,0,0,0,0,0,0,240,63,9,16,16,17,
31,14,16,1,19,14,15,14,15,12,14,0,30,6,1,56,
32,32,32,32,99,111,100,101,40,83,69,84,44,114,44,116,
109,112,44,114,114,41,0,0,12,16,0,4,99,111,100,101,
0,0,0,0,13,15,16,0,12,20,0,3,83,69,84,0,
13,16,20,0,15,17,13,0,15,18,12,0,15,19,10,0,
31,14,16,4,19,14,15,14,30,6,1,57,32,32,32,32,
102,114,101,101,95,116,109,112,40,114,41,32,35,82,69,71,
0,0,0,0,12,16,0,8,102,114,101,101,95,116,109,112,
0,0,0,0,13,15,16,0,15,16,13,0,31,14,16,1,
19,14,15,14,30,6,1,58,32,32,32,32,102,114,101,101,
95,116,109,112,40,116,109,112,41,32,35,82,69,71,0,0,
12,16,0,8,102,114,101,101,95,116,109,112,0,0,0,0,
13,15,16,0,15,16,12,0,31,14,16,1,19,14,15,14,
30,4,1,59,32,32,32,32,114,101,116,117,114,110,32,114,
114,0,0,0,20,10,0,0,0,0,0,0,12,38,0,10,
100,111,95,115,101,116,95,99,116,120,0,0,14,38,37,0,
30,9,1,61,100,101,102,32,109,97,110,97,103,101,95,115,
101,113,40,105,44,97,44,105,116,101,109,115,44,115,97,118,
61,48,41,58,0,0,0,0,16,38,1,19,44,19,0,0,
30,9,1,61,100,101,102,32,109,97,110,97,103,101,95,115,
101,113,40,105,44,97,44,105,116,101,109,115,44,115,97,118,
61,48,41,58,0,0,0,0,12,1,0,9,101,110,99,111,
100,101,46,112,121,0,0,0,33,1,0,0,12,1,0,10,
109,97,110,97,103,101,95,115,101,113,0,0,34,1,0,0,
28,2,0,0,9,1,0,2,28,3,0,0,9,2,0,3,
28,4,0,0,9,3,0,4,11,4,0,0,0,0,0,0,
0,0,0,0,28,5,0,0,32,4,0,5,30,7,1,62,
32,32,32,32,108,32,61,32,109,97,120,40,115,97,118,44,
108,101,110,40,105,116,101,109,115,41,41,0,12,8,0,3,
109,97,120,0,13,7,8,0,15,8,4,0,12,11,0,3,
108,101,110,0,13,10,11,0,15,11,3,0,31,9,11,1,
19,9,10,9,31,6,8,2,19,6,7,6,15,5,6,0,
30,7,1,63,32,32,32,32,110,44,116,109,112,115,32,61,
32,48,44,103,101,116,95,116,109,112,115,40,108,41,0,0,
11,7,0,0,0,0,0,0,0,0,0,0,15,6,7,0,
12,10,0,8,103,101,116,95,116,109,112,115,0,0,0,0,
13,9,10,0,15,10,5,0,31,8,10,1,19,8,9,8,
15,7,8,0,15,8,6,0,15,6,7,0,30,6,1,64,
32,32,32,32,102,111,114,32,116,116,32,105,110,32,105,116,
101,109,115,58,0,0,0,0,11,9,0,0,0,0,0,0,
0,0,0,0,42,7,3,9,18,0,0,80,30,5,1,65,
32,32,32,32,32,32,32,32,114,32,61,32,116,109,112,115,
91,110,93,0,9,11,6,8,15,10,11,0,30,6,1,66,
32,32,32,32,32,32,32,32,98,32,61,32,100,111,40,116,
116,44,114,41,0,0,0,0,12,14,0,2,100,111,0,0,
13,13,14,0,15,14,7,0,15,15,10,0,31,12,14,2,
19,12,13,12,15,11,12,0,30,5,1,67,32,32,32,32,
32,32,32,32,105,102,32,114,32,33,61,32,98,58,0,0,
35,12,10,11,21,12,0,0,18,0,0,37,30,7,1,68,
32,32,32,32,32,32,32,32,32,32,32,32,99,111,100,101,
40,77,79,86,69,44,114,44,98,41,0,0,12,14,0,4,
99,111,100,101,0,0,0,0,13,13,14,0,12,17,0,4,
77,79,86,69,0,0,0,0,13,14,17,0,15,15,10,0,
15,16,11,0,31,12,14,3,19,12,13,12,30,6,1,69,
32,32,32,32,32,32,32,32,32,32,32,32,102,114,101,101,
95,116,109,112,40,98,41,0,12,14,0,8,102,114,101,101,
95,116,109,112,0,0,0,0,13,13,14,0,15,14,11,0,
31,12,14,1,19,12,13,12,18,0,0,1,30,4,1,70,
32,32,32,32,32,32,32,32,110,32,43,61,49,0,0,0,
11,13,0,0,0,0,0,0,0,0,240,63,1,12,8,13,
15,8,12,0,18,0,255,176,30,6,1,71,32,32,32,32,
105,102,32,110,111,116,32,108,101,110,40,116,109,112,115,41,
58,0,0,0,12,14,0,3,108,101,110,0,13,13,14,0,
15,14,6,0,31,12,14,1,19,12,13,12,47,9,12,0,
21,9,0,0,18,0,0,33,30,6,1,72,32,32,32,32,
32,32,32,32,99,111,100,101,40,105,44,97,44,48,44,48,
41,0,0,0,12,13,0,4,99,111,100,101,0,0,0,0,
13,12,13,0,15,13,1,0,15,14,2,0,11,15,0,0,
0,0,0,0,0,0,0,0,11,16,0,0,0,0,0,0,
0,0,0,0,31,9,13,4,19,9,12,9,30,5,1,73,
32,32,32,32,32,32,32,32,114,101,116,117,114,110,32,48,
0,0,0,0,11,9,0,0,0,0,0,0,0,0,0,0,
20,9,0,0,18,0,0,1,30,9,1,74,32,32,32,32,
99,111,100,101,40,105,44,97,44,116,109,112,115,91,48,93,
44,108,101,110,40,105,116,101,109,115,41,41,0,0,0,0,
12,13,0,4,99,111,100,101,0,0,0,0,13,12,13,0,
15,13,1,0,15,14,2,0,11,17,0,0,0,0,0,0,
0,0,0,0,9,15,6,17,12,18,0,3,108,101,110,0,
13,17,18,0,15,18,3,0,31,16,18,1,19,16,17,16,
31,9,13,4,19,9,12,9,30,7,1,75,32,32,32,32,
102,114,101,101,95,116,109,112,115,40,116,109,112,115,91,115,
97,118,58,93,41,0,0,0,12,13,0,9,102,114,101,101,
95,116,109,112,115,0,0,0,13,12,13,0,15,15,4,0,
28,16,0,0,27,14,15,2,9,13,6,14,31,9,13,1,
19,9,12,9,30,5,1,76,32,32,32,32,114,101,116,117,
114,110,32,116,109,112,115,91,48,93,0,0,11,12,0,0,
0,0,0,0,0,0,0,0,9,9,6,12,20,9,0,0,
0,0,0,0,12,39,0,10,109,97,110,97,103,101,95,115,
101,113,0,0,14,39,38,0,30,6,1,78,100,101,102,32,
112,95,102,105,108,116,101,114,40,105,116,101,109,115,41,58,
0,0,0,0,16,39,0,171,44,12,0,0,30,6,1,78,
100,101,102,32,112,95,102,105,108,116,101,114,40,105,116,101,
109,115,41,58,0,0,0,0,12,1,0,9,101,110,99,111,
100,101,46,112,121,0,0,0,33,1,0,0,12,1,0,8,
112,95,102,105,108,116,101,114,0,0,0,0,34,1,0,0,
28,2,0,0,9,1,0,2,30,8,1,79,32,32,32,32,
97,44,98,44,99,44,100,32,61,32,91,93,44,91,93,44,
78,111,110,101,44,78,111,110,101,0,0,0,27,3,0,0,
15,2,3,0,27,4,0,0,15,3,4,0,28,5,0,0,
15,4,5,0,28,6,0,0,15,5,6,0,15,6,2,0,
15,2,3,0,15,3,4,0,15,4,5,0,30,5,1,80,
32,32,32,32,102,111,114,32,116,32,105,110,32,105,116,101,
109,115,58,0,11,7,0,0,0,0,0,0,0,0,0,0,
42,5,1,7,18,0,0,106,30,15,1,81,32,32,32,32,
32,32,32,32,105,102,32,116,46,116,121,112,101,32,61,61,
32,39,115,121,109,98,111,108,39,32,97,110,100,32,116,46,
118,97,108,32,61,61,32,39,61,39,58,32,98,46,97,112,
112,101,110,100,40,116,41,0,12,9,0,4,116,121,112,101,
0,0,0,0,9,8,5,9,12,9,0,6,115,121,109,98,
111,108,0,0,23,8,8,9,21,8,0,0,18,0,0,7,
12,9,0,3,118,97,108,0,9,8,5,9,12,9,0,1,
61,0,0,0,23,8,8,9,21,8,0,0,18,0,0,9,
12,10,0,6,97,112,112,101,110,100,0,0,9,9,2,10,
15,10,5,0,31,8,10,1,19,8,9,8,18,0,0,63,
30,10,1,82,32,32,32,32,32,32,32,32,101,108,105,102,
32,116,46,116,121,112,101,32,61,61,32,39,97,114,103,115,
39,58,32,99,32,61,32,116,0,0,0,0,12,9,0,4,
116,121,112,101,0,0,0,0,9,8,5,9,12,9,0,4,
97,114,103,115,0,0,0,0,23,8,8,9,21,8,0,0,
18,0,0,3,15,3,5,0,18,0,0,40,30,10,1,83,
32,32,32,32,32,32,32,32,101,108,105,102,32,116,46,116,
121,112,101,32,61,61,32,39,110,97,114,103,115,39,58,32,
100,32,61,32,116,0,0,0,12,9,0,4,116,121,112,101,
0,0,0,0,9,8,5,9,12,9,0,5,110,97,114,103,
115,0,0,0,23,8,8,9,21,8,0,0,18,0,0,3,
15,4,5,0,18,0,0,17,30,7,1,84,32,32,32,32,
32,32,32,32,101,108,115,101,58,32,97,46,97,112,112,101,
110,100,40,116,41,0,0,0,12,10,0,6,97,112,112,101,
110,100,0,0,9,9,6,10,15,10,5,0,31,8,10,1,
19,8,9,8,18,0,0,1,18,0,255,150,30,5,1,85,
32,32,32,32,114,101,116,117,114,110,32,97,44,98,44,99,
44,100,0,0,15,8,6,0,15,9,2,0,15,10,3,0,
15,11,4,0,27,7,8,4,20,7,0,0,0,0,0,0,
12,40,0,8,112,95,102,105,108,116,101,114,0,0,0,0,
14,40,39,0,30,5,1,87,100,101,102,32,100,111,95,105,
109,112,111,114,116,40,116,41,58,0,0,0,16,40,0,169,
44,21,0,0,30,5,1,87,100,101,102,32,100,111,95,105,
109,112,111,114,116,40,116,41,58,0,0,0,12,1,0,9,
101,110,99,111,100,101,46,112,121,0,0,0,33,1,0,0,
12,1,0,9,100,111,95,105,109,112,111,114,116,0,0,0,
34,1,0,0,28,2,0,0,9,1,0,2,30,6,1,88,
32,32,32,32,102,111,114,32,109,111,100,32,105,110,32,116,
46,105,116,101,109,115,58,0,12,4,0,5,105,116,101,109,
115,0,0,0,9,3,1,4,11,4,0,0,0,0,0,0,
0,0,0,0,42,2,3,4,18,0,0,133,30,7,1,89,
32,32,32,32,32,32,32,32,109,111,100,46,116,121,112,101,
32,61,32,39,115,116,114,105,110,103,39,0,12,5,0,6,
115,116,114,105,110,103,0,0,12,6,0,4,116,121,112,101,
0,0,0,0,10,2,6,5,30,12,1,90,32,32,32,32,
32,32,32,32,118,32,61,32,100,111,95,99,97,108,108,40,
84,111,107,101,110,40,116,46,112,111,115,44,39,99,97,108,
108,39,44,78,111,110,101,44,91,0,0,0,12,8,0,7,
100,111,95,99,97,108,108,0,13,7,8,0,12,10,0,5,
84,111,107,101,110,0,0,0,13,9,10,0,12,14,0,3,
112,111,115,0,9,10,1,14,12,11,0,4,99,97,108,108,
0,0,0,0,28,12,0,0,30,11,1,91,32,32,32,32,
32,32,32,32,32,32,32,32,84,111,107,101,110,40,116,46,
112,111,115,44,39,110,97,109,101,39,44,39,105,109,112,111,
114,116,39,41,44,0,0,0,12,17,0,5,84,111,107,101,
110,0,0,0,13,16,17,0,12,20,0,3,112,111,115,0,
9,17,1,20,12,18,0,4,110,97,109,101,0,0,0,0,
12,19,0,6,105,109,112,111,114,116,0,0,31,14,17,3,
19,14,16,14,30,5,1,92,32,32,32,32,32,32,32,32,
32,32,32,32,109,111,100,93,41,41,0,0,15,15,2,0,
27,13,14,2,31,8,10,4,19,8,9,8,31,6,8,1,
19,6,7,6,15,5,6,0,30,7,1,93,32,32,32,32,
32,32,32,32,109,111,100,46,116,121,112,101,32,61,32,39,
110,97,109,101,39,0,0,0,12,6,0,4,110,97,109,101,
0,0,0,0,12,7,0,4,116,121,112,101,0,0,0,0,
10,2,7,6,30,12,1,94,32,32,32,32,32,32,32,32,
100,111,95,115,101,116,95,99,116,120,40,109,111,100,44,84,
111,107,101,110,40,116,46,112,111,115,44,39,114,101,103,39,
44,118,41,41,0,0,0,0,12,8,0,10,100,111,95,115,
101,116,95,99,116,120,0,0,13,7,8,0,15,8,2,0,
12,11,0,5,84,111,107,101,110,0,0,0,13,10,11,0,
12,14,0,3,112,111,115,0,9,11,1,14,12,12,0,3,
114,101,103,0,15,13,5,0,31,9,11,3,19,9,10,9,
31,6,8,2,19,6,7,6,18,0,255,123,0,0,0,0,
12,41,0,9,100,111,95,105,109,112,111,114,116,0,0,0,
14,41,40,0,30,4,1,95,100,101,102,32,100,111,95,102,
114,111,109,40,116,41,58,0,16,41,1,144,44,23,0,0,
30,4,1,95,100,101,102,32,100,111,95,102,114,111,109,40,
116,41,58,0,12,1,0,9,101,110,99,111,100,101,46,112,
121,0,0,0,33,1,0,0,12,1,0,7,100,111,95,102,
114,111,109,0,34,1,0,0,28,2,0,0,9,1,0,2,
30,6,1,96,32,32,32,32,109,111,100,32,61,32,116,46,
105,116,101,109,115,91,48,93,0,0,0,0,12,4,0,5,
105,116,101,109,115,0,0,0,9,3,1,4,11,4,0,0,
0,0,0,0,0,0,0,0,9,3,3,4,15,2,3,0,
30,6,1,97,32,32,32,32,109,111,100,46,116,121,112,101,
32,61,32,39,115,116,114,105,110,103,39,0,12,3,0,6,
115,116,114,105,110,103,0,0,12,4,0,4,116,121,112,101,
0,0,0,0,10,2,4,3,30,10,1,98,32,32,32,32,
118,32,61,32,100,111,40,84,111,107,101,110,40,116,46,112,
111,115,44,39,99,97,108,108,39,44,78,111,110,101,44,91,
0,0,0,0,12,6,0,2,100,111,0,0,13,5,6,0,
12,8,0,5,84,111,107,101,110,0,0,0,13,7,8,0,
12,12,0,3,112,111,115,0,9,8,1,12,12,9,0,4,
99,97,108,108,0,0,0,0,28,10,0,0,30,10,1,99,
32,32,32,32,32,32,32,32,84,111,107,101,110,40,116,46,
112,111,115,44,39,110,97,109,101,39,44,39,105,109,112,111,
114,116,39,41,44,0,0,0,12,15,0,5,84,111,107,101,
110,0,0,0,13,14,15,0,12,18,0,3,112,111,115,0,
9,15,1,18,12,16,0,4,110,97,109,101,0,0,0,0,
12,17,0,6,105,109,112,111,114,116,0,0,31,12,15,3,
19,12,14,12,30,4,1,100,32,32,32,32,32,32,32,32,
109,111,100,93,41,41,0,0,15,13,2,0,27,11,12,2,
31,6,8,4,19,6,7,6,31,4,6,1,19,4,5,4,
15,3,4,0,30,6,1,101,32,32,32,32,105,116,101,109,
32,61,32,116,46,105,116,101,109,115,91,49,93,0,0,0,
12,6,0,5,105,116,101,109,115,0,0,0,9,5,1,6,
11,6,0,0,0,0,0,0,0,0,240,63,9,5,5,6,
15,4,5,0,30,6,1,102,32,32,32,32,105,102,32,105,
116,101,109,46,118,97,108,32,61,61,32,39,42,39,58,0,
12,6,0,3,118,97,108,0,9,5,4,6,12,6,0,1,
42,0,0,0,23,5,5,6,21,5,0,0,18,0,0,120,
30,12,1,103,32,32,32,32,32,32,32,32,102,114,101,101,
95,116,109,112,40,100,111,40,84,111,107,101,110,40,116,46,
112,111,115,44,39,99,97,108,108,39,44,78,111,110,101,44,
91,0,0,0,12,7,0,8,102,114,101,101,95,116,109,112,
0,0,0,0,13,6,7,0,12,9,0,2,100,111,0,0,
13,8,9,0,12,11,0,5,84,111,107,101,110,0,0,0,
13,10,11,0,12,15,0,3,112,111,115,0,9,11,1,15,
12,12,0,4,99,97,108,108,0,0,0,0,28,13,0,0,
30,11,1,104,32,32,32,32,32,32,32,32,32,32,32,32,
84,111,107,101,110,40,116,46,112,111,115,44,39,110,97,109,
101,39,44,39,109,101,114,103,101,39,41,44,0,0,0,0,
12,19,0,5,84,111,107,101,110,0,0,0,13,18,19,0,
12,22,0,3,112,111,115,0,9,19,1,22,12,20,0,4,
110,97,109,101,0,0,0,0,12,21,0,5,109,101,114,103,
101,0,0,0,31,15,19,3,19,15,18,15,30,11,1,105,
32,32,32,32,32,32,32,32,32,32,32,32,84,111,107,101,
110,40,116,46,112,111,115,44,39,110,97,109,101,39,44,39,
95,95,100,105,99,116,95,95,39,41,44,0,12,19,0,5,
84,111,107,101,110,0,0,0,13,18,19,0,12,22,0,3,
112,111,115,0,9,19,1,22,12,20,0,4,110,97,109,101,
0,0,0,0,12,21,0,8,95,95,100,105,99,116,95,95,
0,0,0,0,31,16,19,3,19,16,18,16,30,11,1,106,
32,32,32,32,32,32,32,32,32,32,32,32,84,111,107,101,
110,40,116,46,112,111,115,44,39,114,101,103,39,44,118,41,
93,41,41,41,32,35,82,69,71,0,0,0,12,19,0,5,
84,111,107,101,110,0,0,0,13,18,19,0,12,22,0,3,
112,111,115,0,9,19,1,22,12,20,0,3,114,101,103,0,
15,21,3,0,31,17,19,3,19,17,18,17,27,14,15,3,
31,9,11,4,19,9,10,9,31,7,9,1,19,7,8,7,
31,5,7,1,19,5,6,5,18,0,0,139,30,3,1,107,
32,32,32,32,101,108,115,101,58,0,0,0,30,8,1,108,
32,32,32,32,32,32,32,32,105,116,101,109,46,116,121,112,
101,32,61,32,39,115,116,114,105,110,103,39,0,0,0,0,
12,5,0,6,115,116,114,105,110,103,0,0,12,6,0,4,
116,121,112,101,0,0,0,0,10,4,6,5,30,8,1,109,
32,32,32,32,32,32,32,32,102,114,101,101,95,116,109,112,
40,100,111,95,115,101,116,95,99,116,120,40,0,0,0,0,
12,7,0,8,102,114,101,101,95,116,109,112,0,0,0,0,
13,6,7,0,12,9,0,10,100,111,95,115,101,116,95,99,
116,120,0,0,13,8,9,0,30,19,1,110,32,32,32,32,
32,32,32,32,32,32,32,32,84,111,107,101,110,40,116,46,
112,111,115,44,39,103,101,116,39,44,78,111,110,101,44,91,
32,84,111,107,101,110,40,116,46,112,111,115,44,39,110,97,
109,101,39,44,39,95,95,100,105,99,116,95,95,39,41,44,
105,116,101,109,93,41,44,0,12,12,0,5,84,111,107,101,
110,0,0,0,13,11,12,0,12,16,0,3,112,111,115,0,
9,12,1,16,12,13,0,3,103,101,116,0,28,14,0,0,
12,19,0,5,84,111,107,101,110,0,0,0,13,18,19,0,
12,22,0,3,112,111,115,0,9,19,1,22,12,20,0,4,
110,97,109,101,0,0,0,0,12,21,0,8,95,95,100,105,
99,116,95,95,0,0,0,0,31,16,19,3,19,16,18,16,
15,17,4,0,27,15,16,2,31,9,12,4,19,9,11,9,
30,17,1,111,32,32,32,32,32,32,32,32,32,32,32,32,
84,111,107,101,110,40,116,46,112,111,115,44,39,103,101,116,
39,44,78,111,110,101,44,91,32,84,111,107,101,110,40,116,
46,112,111,115,44,39,114,101,103,39,44,118,41,44,105,116,
101,109,93,41,0,0,0,0,12,12,0,5,84,111,107,101,
110,0,0,0,13,11,12,0,12,16,0,3,112,111,115,0,
9,12,1,16,12,13,0,3,103,101,116,0,28,14,0,0,
12,19,0,5,84,111,107,101,110,0,0,0,13,18,19,0,
12,22,0,3,112,111,115,0,9,19,1,22,12,20,0,3,
114,101,103,0,15,21,3,0,31,16,19,3,19,16,18,16,
15,17,4,0,27,15,16,2,31,10,12,4,19,10,11,10,
31,7,9,2,19,7,8,7,31,5,7,1,19,5,6,5,
18,0,0,1,0,0,0,0,12,42,0,7,100,111,95,102,
114,111,109,0,14,42,41,0,30,5,1,115,100,101,102,32,
100,111,95,103,108,111,98,97,108,115,40,116,41,58,0,0,
16,42,0,92,44,8,0,0,30,5,1,115,100,101,102,32,
100,111,95,103,108,111,98,97,108,115,40,116,41,58,0,0,
12,1,0,9,101,110,99,111,100,101,46,112,121,0,0,0,
33,1,0,0,12,1,0,10,100,111,95,103,108,111,98,97,
108,115,0,0,34,1,0,0,28,2,0,0,9,1,0,2,
30,6,1,116,32,32,32,32,102,111,114,32,116,32,105,110,
32,116,46,105,116,101,109,115,58,0,0,0,12,3,0,5,
105,116,101,109,115,0,0,0,9,2,1,3,11,3,0,0,
0,0,0,0,0,0,0,0,42,1,2,3,18,0,0,56,
30,9,1,117,32,32,32,32,32,32,32,32,105,102,32,116,
46,118,97,108,32,110,111,116,32,105,110,32,68,46,103,108,
111,98,97,108,115,58,0,0,12,5,0,1,68,0,0,0,
13,4,5,0,12,5,0,7,103,108,111,98,97,108,115,0,
9,4,4,5,12,6,0,3,118,97,108,0,9,5,1,6,
36,4,4,5,11,5,0,0,0,0,0,0,0,0,0,0,
23,4,4,5,21,4,0,0,18,0,0,28,30,9,1,118,
32,32,32,32,32,32,32,32,32,32,32,32,68,46,103,108,
111,98,97,108,115,46,97,112,112,101,110,100,40,116,46,118,
97,108,41,0,12,6,0,1,68,0,0,0,13,5,6,0,
12,6,0,7,103,108,111,98,97,108,115,0,9,5,5,6,
12,6,0,6,97,112,112,101,110,100,0,0,9,5,5,6,
12,7,0,3,118,97,108,0,9,6,1,7,31,4,6,1,
19,4,5,4,18,0,0,1,18,0,255,200,0,0,0,0,
12,43,0,10,100,111,95,103,108,111,98,97,108,115,0,0,
14,43,42,0,30,4,1,119,100,101,102,32,100,111,95,100,
101,108,40,116,116,41,58,0,16,43,0,125,44,13,0,0,
30,4,1,119,100,101,102,32,100,111,95,100,101,108,40,116,
116,41,58,0,12,1,0,9,101,110,99,111,100,101,46,112,
121,0,0,0,33,1,0,0,12,1,0,6,100,111,95,100,
101,108,0,0,34,1,0,0,28,2,0,0,9,1,0,2,
30,6,1,120,32,32,32,32,102,111,114,32,116,32,105,110,
32,116,116,46,105,116,101,109,115,58,0,0,12,4,0,5,
105,116,101,109,115,0,0,0,9,3,1,4,11,4,0,0,
0,0,0,0,0,0,0,0,42,2,3,4,18,0,0,91,
30,7,1,121,32,32,32,32,32,32,32,32,114,32,61,32,
100,111,40,116,46,105,116,101,109,115,91,48,93,41,0,0,
12,8,0,2,100,111,0,0,13,7,8,0,12,9,0,5,
105,116,101,109,115,0,0,0,9,8,2,9,11,9,0,0,
0,0,0,0,0,0,0,0,9,8,8,9,31,6,8,1,
19,6,7,6,15,5,6,0,30,7,1,122,32,32,32,32,
32,32,32,32,114,50,32,61,32,100,111,40,116,46,105,116,
101,109,115,91,49,93,41,0,12,9,0,2,100,111,0,0,
13,8,9,0,12,10,0,5,105,116,101,109,115,0,0,0,
9,9,2,10,11,10,0,0,0,0,0,0,0,0,240,63,
9,9,9,10,31,7,9,1,19,7,8,7,15,6,7,0,
30,6,1,123,32,32,32,32,32,32,32,32,99,111,100,101,
40,68,69,76,44,114,44,114,50,41,0,0,12,9,0,4,
99,111,100,101,0,0,0,0,13,8,9,0,12,12,0,3,
68,69,76,0,13,9,12,0,15,10,5,0,15,11,6,0,
31,7,9,3,19,7,8,7,30,10,1,124,32,32,32,32,
32,32,32,32,102,114,101,101,95,116,109,112,40,114,41,59,
32,102,114,101,101,95,116,109,112,40,114,50,41,32,35,82,
69,71,0,0,12,9,0,8,102,114,101,101,95,116,109,112,
0,0,0,0,13,8,9,0,15,9,5,0,31,7,9,1,
19,7,8,7,12,9,0,8,102,114,101,101,95,116,109,112,
0,0,0,0,13,8,9,0,15,9,6,0,31,7,9,1,
19,7,8,7,18,0,255,165,0,0,0,0,12,44,0,6,
100,111,95,100,101,108,0,0,14,44,43,0,30,6,1,126,
100,101,102,32,100,111,95,99,97,108,108,40,116,44,114,61,
78,111,110,101,41,58,0,0,16,44,2,124,44,31,0,0,
30,6,1,126,100,101,102,32,100,111,95,99,97,108,108,40,
116,44,114,61,78,111,110,101,41,58,0,0,12,1,0,9,
101,110,99,111,100,101,46,112,121,0,0,0,33,1,0,0,
12,1,0,7,100,111,95,99,97,108,108,0,34,1,0,0,
28,2,0,0,9,1,0,2,28,2,0,0,28,3,0,0,
32,2,0,3,30,5,1,127,32,32,32,32,114,32,61,32,
103,101,116,95,116,109,112,40,114,41,0,0,12,5,0,7,
103,101,116,95,116,109,112,0,13,4,5,0,15,5,2,0,
31,3,5,1,19,3,4,3,15,2,3,0,30,5,1,128,
32,32,32,32,105,116,101,109,115,32,61,32,116,46,105,116,
101,109,115,0,12,5,0,5,105,116,101,109,115,0,0,0,
9,4,1,5,15,3,4,0,30,6,1,129,32,32,32,32,
102,110,99,32,61,32,100,111,40,105,116,101,109,115,91,48,
93,41,0,0,12,7,0,2,100,111,0,0,13,6,7,0,
11,8,0,0,0,0,0,0,0,0,0,0,9,7,3,8,
31,5,7,1,19,5,6,5,15,4,5,0,30,9,1,130,
32,32,32,32,97,44,98,44,99,44,100,32,61,32,112,95,
102,105,108,116,101,114,40,116,46,105,116,101,109,115,91,49,
58,93,41,0,12,7,0,8,112,95,102,105,108,116,101,114,
0,0,0,0,13,6,7,0,12,8,0,5,105,116,101,109,
115,0,0,0,9,7,1,8,11,9,0,0,0,0,0,0,
0,0,240,63,28,10,0,0,27,8,9,2,9,7,7,8,
31,5,7,1,19,5,6,5,11,8,0,0,0,0,0,0,
0,0,0,0,9,7,5,8,15,6,7,0,11,9,0,0,
0,0,0,0,0,0,240,63,9,8,5,9,15,7,8,0,
11,10,0,0,0,0,0,0,0,0,0,64,9,9,5,10,
15,8,9,0,11,11,0,0,0,0,0,0,0,0,8,64,
9,10,5,11,15,9,10,0,30,4,1,131,32,32,32,32,
101,32,61,32,78,111,110,101,0,0,0,0,28,10,0,0,
15,5,10,0,30,9,1,132,32,32,32,32,105,102,32,108,
101,110,40,98,41,32,33,61,32,48,32,111,114,32,100,32,
33,61,32,78,111,110,101,58,0,0,0,0,12,12,0,3,
108,101,110,0,13,11,12,0,15,12,7,0,31,10,12,1,
19,10,11,10,11,11,0,0,0,0,0,0,0,0,0,0,
35,10,10,11,46,10,0,0,18,0,0,3,28,11,0,0,
35,10,9,11,21,10,0,0,18,0,1,14,30,14,1,133,
32,32,32,32,32,32,32,32,101,32,61,32,100,111,40,84,
111,107,101,110,40,116,46,112,111,115,44,39,100,105,99,116,
39,44,78,111,110,101,44,91,93,41,41,59,32,117,110,95,
116,109,112,40,101,41,59,0,12,12,0,2,100,111,0,0,
13,11,12,0,12,14,0,5,84,111,107,101,110,0,0,0,
13,13,14,0,12,18,0,3,112,111,115,0,9,14,1,18,
12,15,0,4,100,105,99,116,0,0,0,0,28,16,0,0,
27,17,0,0,31,12,14,4,19,12,13,12,31,10,12,1,
19,10,11,10,15,5,10,0,12,12,0,6,117,110,95,116,
109,112,0,0,13,11,12,0,15,12,5,0,31,10,12,1,
19,10,11,10,30,5,1,134,32,32,32,32,32,32,32,32,
102,111,114,32,112,32,105,110,32,98,58,0,11,11,0,0,
0,0,0,0,0,0,0,0,42,10,7,11,18,0,0,121,
30,10,1,135,32,32,32,32,32,32,32,32,32,32,32,32,
112,46,105,116,101,109,115,91,48,93,46,116,121,112,101,32,
61,32,39,115,116,114,105,110,103,39,0,0,12,13,0,5,
105,116,101,109,115,0,0,0,9,12,10,13,11,13,0,0,
0,0,0,0,0,0,0,0,9,12,12,13,12,13,0,6,
115,116,114,105,110,103,0,0,12,14,0,4,116,121,112,101,
0,0,0,0,10,12,14,13,30,13,1,136,32,32,32,32,
32,32,32,32,32,32,32,32,116,49,44,116,50,32,61,32,
100,111,40,112,46,105,116,101,109,115,91,48,93,41,44,100,
111,40,112,46,105,116,101,109,115,91,49,93,41,0,0,0,
12,15,0,2,100,111,0,0,13,14,15,0,12,16,0,5,
105,116,101,109,115,0,0,0,9,15,10,16,11,16,0,0,
0,0,0,0,0,0,0,0,9,15,15,16,31,13,15,1,
19,13,14,13,15,12,13,0,12,16,0,2,100,111,0,0,
13,15,16,0,12,17,0,5,105,116,101,109,115,0,0,0,
9,16,10,17,11,17,0,0,0,0,0,0,0,0,240,63,
9,16,16,17,31,14,16,1,19,14,15,14,15,13,14,0,
15,14,12,0,15,12,13,0,30,8,1,137,32,32,32,32,
32,32,32,32,32,32,32,32,99,111,100,101,40,83,69,84,
44,101,44,116,49,44,116,50,41,0,0,0,12,16,0,4,
99,111,100,101,0,0,0,0,13,15,16,0,12,20,0,3,
83,69,84,0,13,16,20,0,15,17,5,0,15,18,14,0,
15,19,12,0,31,13,16,4,19,13,15,13,30,11,1,138,
32,32,32,32,32,32,32,32,32,32,32,32,102,114,101,101,
95,116,109,112,40,116,49,41,59,32,102,114,101,101,95,116,
109,112,40,116,50,41,32,35,82,69,71,0,12,16,0,8,
102,114,101,101,95,116,109,112,0,0,0,0,13,15,16,0,
15,16,14,0,31,13,16,1,19,13,15,13,12,16,0,8,
102,114,101,101,95,116,109,112,0,0,0,0,13,15,16,0,
15,16,12,0,31,13,16,1,19,13,15,13,18,0,255,135,
30,30,1,139,32,32,32,32,32,32,32,32,105,102,32,100,
58,32,102,114,101,101,95,116,109,112,40,100,111,40,84,111,
107,101,110,40,116,46,112,111,115,44,39,99,97,108,108,39,
44,78,111,110,101,44,91,84,111,107,101,110,40,116,46,112,
111,115,44,39,110,97,109,101,39,44,39,109,101,114,103,101,
39,41,44,84,111,107,101,110,40,116,46,112,111,115,44,39,
114,101,103,39,44,101,41,44,100,46,105,116,101,109,115,91,
48,93,93,41,41,41,32,35,82,69,71,0,21,9,0,0,
18,0,0,63,12,15,0,8,102,114,101,101,95,116,109,112,
0,0,0,0,13,13,15,0,12,17,0,2,100,111,0,0,
13,16,17,0,12,19,0,5,84,111,107,101,110,0,0,0,
13,18,19,0,12,23,0,3,112,111,115,0,9,19,1,23,
12,20,0,4,99,97,108,108,0,0,0,0,28,21,0,0,
12,27,0,5,84,111,107,101,110,0,0,0,13,26,27,0,
12,30,0,3,112,111,115,0,9,27,1,30,12,28,0,4,
110,97,109,101,0,0,0,0,12,29,0,5,109,101,114,103,
101,0,0,0,31,23,27,3,19,23,26,23,12,27,0,5,
84,111,107,101,110,0,0,0,13,26,27,0,12,30,0,3,
112,111,115,0,9,27,1,30,12,28,0,3,114,101,103,0,
15,29,5,0,31,24,27,3,19,24,26,24,12,26,0,5,
105,116,101,109,115,0,0,0,9,25,9,26,11,26,0,0,
0,0,0,0,0,0,0,0,9,25,25,26,27,22,23,3,
31,17,19,4,19,17,18,17,31,15,17,1,19,15,16,15,
31,11,15,1,19,11,13,11,18,0,0,1,18,0,0,1,
30,7,1,140,32,32,32,32,109,97,110,97,103,101,95,115,
101,113,40,80,65,82,65,77,83,44,114,44,97,41,0,0,
12,15,0,10,109,97,110,97,103,101,95,115,101,113,0,0,
13,13,15,0,12,18,0,6,80,65,82,65,77,83,0,0,
13,15,18,0,15,16,2,0,15,17,6,0,31,11,15,3,
19,11,13,11,30,5,1,141,32,32,32,32,105,102,32,99,
32,33,61,32,78,111,110,101,58,0,0,0,28,13,0,0,
35,11,8,13,21,11,0,0,18,0,0,88,30,12,1,142,
32,32,32,32,32,32,32,32,116,49,44,116,50,32,61,32,
95,100,111,95,115,116,114,105,110,103,40,39,42,39,41,44,
100,111,40,99,46,105,116,101,109,115,91,48,93,41,0,0,
12,16,0,10,95,100,111,95,115,116,114,105,110,103,0,0,
13,15,16,0,12,16,0,1,42,0,0,0,31,13,16,1,
19,13,15,13,15,11,13,0,12,17,0,2,100,111,0,0,
13,16,17,0,12,18,0,5,105,116,101,109,115,0,0,0,
9,17,8,18,11,18,0,0,0,0,0,0,0,0,0,0,
9,17,17,18,31,15,17,1,19,15,16,15,15,13,15,0,
15,14,11,0,15,12,13,0,30,7,1,143,32,32,32,32,
32,32,32,32,99,111,100,101,40,83,69,84,44,114,44,116,
49,44,116,50,41,0,0,0,12,15,0,4,99,111,100,101,
0,0,0,0,13,13,15,0,12,19,0,3,83,69,84,0,
13,15,19,0,15,16,2,0,15,17,14,0,15,18,12,0,
31,11,15,4,19,11,13,11,30,10,1,144,32,32,32,32,
32,32,32,32,102,114,101,101,95,116,109,112,40,116,49,41,
59,32,102,114,101,101,95,116,109,112,40,116,50,41,32,35,
82,69,71,0,12,15,0,8,102,114,101,101,95,116,109,112,
0,0,0,0,13,13,15,0,15,15,14,0,31,11,15,1,
19,11,13,11,12,15,0,8,102,114,101,101,95,116,109,112,
0,0,0,0,13,13,15,0,15,15,12,0,31,11,15,1,
19,11,13,11,18,0,0,1,30,5,1,145,32,32,32,32,
105,102,32,101,32,33,61,32,78,111,110,101,58,0,0,0,
28,13,0,0,35,11,5,13,21,11,0,0,18,0,0,53,
30,6,1,146,32,32,32,32,32,32,32,32,116,49,32,61,
32,95,100,111,95,110,111,110,101,40,41,0,12,15,0,8,
95,100,111,95,110,111,110,101,0,0,0,0,13,13,15,0,
31,11,0,0,19,11,13,11,15,14,11,0,30,7,1,147,
32,32,32,32,32,32,32,32,99,111,100,101,40,83,69,84,
44,114,44,116,49,44,101,41,0,0,0,0,12,15,0,4,
99,111,100,101,0,0,0,0,13,13,15,0,12,19,0,3,
83,69,84,0,13,15,19,0,15,16,2,0,15,17,14,0,
15,18,5,0,31,11,15,4,19,11,13,11,30,7,1,148,
32,32,32,32,32,32,32,32,102,114,101,101,95,116,109,112,
40,116,49,41,32,35,82,69,71,0,0,0,12,15,0,8,
102,114,101,101,95,116,109,112,0,0,0,0,13,13,15,0,
15,15,14,0,31,11,15,1,19,11,13,11,18,0,0,1,
30,6,1,149,32,32,32,32,99,111,100,101,40,67,65,76,
76,44,114,44,102,110,99,44,114,41,0,0,12,15,0,4,
99,111,100,101,0,0,0,0,13,13,15,0,12,19,0,4,
67,65,76,76,0,0,0,0,13,15,19,0,15,16,2,0,
15,17,4,0,15,18,2,0,31,11,15,4,19,11,13,11,
30,6,1,150,32,32,32,32,102,114,101,101,95,116,109,112,
40,102,110,99,41,32,35,82,69,71,0,0,12,15,0,8,
102,114,101,101,95,116,109,112,0,0,0,0,13,13,15,0,
15,15,4,0,31,11,15,1,19,11,13,11,30,4,1,151,
32,32,32,32,114,101,116,117,114,110,32,114,0,0,0,0,
20,2,0,0,0,0,0,0,12,45,0,7,100,111,95,99,
97,108,108,0,14,45,44,0,30,6,1,153,100,101,102,32,
100,111,95,110,97,109,101,40,116,44,114,61,78,111,110,101,
41,58,0,0,16,45,0,186,44,10,0,0,30,6,1,153,
100,101,102,32,100,111,95,110,97,109,101,40,116,44,114,61,
78,111,110,101,41,58,0,0,12,1,0,9,101,110,99,111,
100,101,46,112,121,0,0,0,33,1,0,0,12,1,0,7,
100,111,95,110,97,109,101,0,34,1,0,0,28,2,0,0,
9,1,0,2,28,2,0,0,28,3,0,0,32,2,0,3,
30,6,1,154,32,32,32,32,105,102,32,116,46,118,97,108,
32,105,110,32,68,46,118,97,114,115,58,0,12,4,0,1,
68,0,0,0,13,3,4,0,12,4,0,4,118,97,114,115,
0,0,0,0,9,3,3,4,12,5,0,3,118,97,108,0,
9,4,1,5,36,3,3,4,21,3,0,0,18,0,0,21,
30,8,1,155,32,32,32,32,32,32,32,32,114,101,116,117,
114,110,32,100,111,95,108,111,99,97,108,40,116,44,114,41,
0,0,0,0,12,5,0,8,100,111,95,108,111,99,97,108,
0,0,0,0,13,4,5,0,15,5,1,0,15,6,2,0,
31,3,5,2,19,3,4,3,20,3,0,0,18,0,0,1,
30,8,1,156,32,32,32,32,105,102,32,116,46,118,97,108,
32,110,111,116,32,105,110,32,68,46,114,103,108,111,98,97,
108,115,58,0,12,4,0,1,68,0,0,0,13,3,4,0,
12,4,0,8,114,103,108,111,98,97,108,115,0,0,0,0,
9,3,3,4,12,5,0,3,118,97,108,0,9,4,1,5,
36,3,3,4,11,4,0,0,0,0,0,0,0,0,0,0,
23,3,3,4,21,3,0,0,18,0,0,29,30,9,1,157,
32,32,32,32,32,32,32,32,68,46,114,103,108,111,98,97,
108,115,46,97,112,112,101,110,100,40,116,46,118,97,108,41,
0,0,0,0,12,5,0,1,68,0,0,0,13,4,5,0,
12,5,0,8,114,103,108,111,98,97,108,115,0,0,0,0,
9,4,4,5,12,5,0,6,97,112,112,101,110,100,0,0,
9,4,4,5,12,6,0,3,118,97,108,0,9,5,1,6,
31,3,5,1,19,3,4,3,18,0,0,1,30,5,1,158,
32,32,32,32,114,32,61,32,103,101,116,95,116,109,112,40,
114,41,0,0,12,5,0,7,103,101,116,95,116,109,112,0,
13,4,5,0,15,5,2,0,31,3,5,1,19,3,4,3,
15,2,3,0,30,6,1,159,32,32,32,32,99,32,61,32,
100,111,95,115,116,114,105,110,103,40,116,41,0,0,0,0,
12,6,0,9,100,111,95,115,116,114,105,110,103,0,0,0,
13,5,6,0,15,6,1,0,31,4,6,1,19,4,5,4,
15,3,4,0,30,5,1,160,32,32,32,32,99,111,100,101,
40,71,71,69,84,44,114,44,99,41,0,0,12,6,0,4,
99,111,100,101,0,0,0,0,13,5,6,0,12,9,0,4,
71,71,69,84,0,0,0,0,13,6,9,0,15,7,2,0,
15,8,3,0,31,4,6,3,19,4,5,4,30,4,1,161,
32,32,32,32,102,114,101,101,95,116,109,112,40,99,41,0,
12,6,0,8,102,114,101,101,95,116,109,112,0,0,0,0,
13,5,6,0,15,6,3,0,31,4,6,1,19,4,5,4,
30,4,1,162,32,32,32,32,114,101,116,117,114,110,32,114,
0,0,0,0,20,2,0,0,0,0,0,0,12,46,0,7,
100,111,95,110,97,109,101,0,14,46,45,0,30,6,1,164,
100,101,102,32,100,111,95,108,111,99,97,108,40,116,44,114,
61,78,111,110,101,41,58,0,16,46,0,177,44,9,0,0,
30,6,1,164,100,101,102,32,100,111,95,108,111,99,97,108,
40,116,44,114,61,78,111,110,101,41,58,0,12,1,0,9,
101,110,99,111,100,101,46,112,121,0,0,0,33,1,0,0,
12,1,0,8,100,111,95,108,111,99,97,108,0,0,0,0,
34,1,0,0,28,2,0,0,9,1,0,2,28,2,0,0,
28,3,0,0,32,2,0,3,30,7,1,165,32,32,32,32,
105,102,32,116,46,118,97,108,32,105,110,32,68,46,114,103,
108,111,98,97,108,115,58,0,12,4,0,1,68,0,0,0,
13,3,4,0,12,4,0,8,114,103,108,111,98,97,108,115,
0,0,0,0,9,3,3,4,12,5,0,3,118,97,108,0,
9,4,1,5,36,3,3,4,21,3,0,0,18,0,0,62,
30,6,1,166,32,32,32,32,32,32,32,32,68,46,101,114,
114,111,114,32,61,32,84,114,117,101,0,0,12,4,0,1,
68,0,0,0,13,3,4,0,11,4,0,0,0,0,0,0,
0,0,240,63,12,5,0,5,101,114,114,111,114,0,0,0,
10,3,5,4,30,15,1,167,32,32,32,32,32,32,32,32,
116,111,107,101,110,105,122,101,46,117,95,101,114,114,111,114,
40,39,85,110,98,111,117,110,100,76,111,99,97,108,69,114,
114,111,114,39,44,68,46,99,111,100,101,44,116,46,112,111,
115,41,0,0,12,5,0,8,116,111,107,101,110,105,122,101,
0,0,0,0,13,4,5,0,12,5,0,7,117,95,101,114,
114,111,114,0,9,4,4,5,12,5,0,17,85,110,98,111,
117,110,100,76,111,99,97,108,69,114,114,111,114,0,0,0,
12,8,0,1,68,0,0,0,13,6,8,0,12,8,0,4,
99,111,100,101,0,0,0,0,9,6,6,8,12,8,0,3,
112,111,115,0,9,7,1,8,31,3,5,3,19,3,4,3,
18,0,0,1,30,7,1,168,32,32,32,32,105,102,32,116,
46,118,97,108,32,110,111,116,32,105,110,32,68,46,118,97,
114,115,58,0,12,4,0,1,68,0,0,0,13,3,4,0,
12,4,0,4,118,97,114,115,0,0,0,0,9,3,3,4,
12,5,0,3,118,97,108,0,9,4,1,5,36,3,3,4,
11,4,0,0,0,0,0,0,0,0,0,0,23,3,3,4,
21,3,0,0,18,0,0,27,30,8,1,169,32,32,32,32,
32,32,32,32,68,46,118,97,114,115,46,97,112,112,101,110,
100,40,116,46,118,97,108,41,0,0,0,0,12,5,0,1,
68,0,0,0,13,4,5,0,12,5,0,4,118,97,114,115,
0,0,0,0,9,4,4,5,12,5,0,6,97,112,112,101,
110,100,0,0,9,4,4,5,12,6,0,3,118,97,108,0,
9,5,1,6,31,3,5,1,19,3,4,3,18,0,0,1,
30,7,1,170,32,32,32,32,114,101,116,117,114,110,32,103,
101,116,95,114,101,103,40,116,46,118,97,108,41,0,0,0,
12,5,0,7,103,101,116,95,114,101,103,0,13,4,5,0,
12,6,0,3,118,97,108,0,9,5,1,6,31,3,5,1,
19,3,4,3,20,3,0,0,0,0,0,0,12,47,0,8,
100,111,95,108,111,99,97,108,0,0,0,0,14,47,46,0,
30,7,1,172,100,101,102,32,100,111,95,100,101,102,40,116,
111,107,44,107,108,115,61,78,111,110,101,41,58,0,0,0,
16,47,3,64,44,25,0,0,30,7,1,172,100,101,102,32,
100,111,95,100,101,102,40,116,111,107,44,107,108,115,61,78,
111,110,101,41,58,0,0,0,12,1,0,9,101,110,99,111,
100,101,46,112,121,0,0,0,33,1,0,0,12,1,0,6,
100,111,95,100,101,102,0,0,34,1,0,0,28,2,0,0,
9,1,0,2,28,2,0,0,28,3,0,0,32,2,0,3,
30,6,1,173,32,32,32,32,105,116,101,109,115,32,61,32,
116,111,107,46,105,116,101,109,115,0,0,0,12,5,0,5,
105,116,101,109,115,0,0,0,9,4,1,5,15,3,4,0,
30,5,1,175,32,32,32,32,116,32,61,32,103,101,116,95,
116,97,103,40,41,0,0,0,12,7,0,7,103,101,116,95,
116,97,103,0,13,6,7,0,31,5,0,0,19,5,6,5,
15,4,5,0,30,6,1,176,32,32,32,32,114,102,32,61,
32,102,110,99,40,116,44,39,101,110,100,39,41,0,0,0,
12,8,0,3,102,110,99,0,13,7,8,0,15,8,4,0,
12,9,0,3,101,110,100,0,31,6,8,2,19,6,7,6,
15,5,6,0,30,4,1,178,32,32,32,32,68,46,98,101,
103,105,110,40,41,0,0,0,12,8,0,1,68,0,0,0,
13,7,8,0,12,8,0,5,98,101,103,105,110,0,0,0,
9,7,7,8,31,6,0,0,19,6,7,6,30,5,1,179,
32,32,32,32,115,101,116,112,111,115,40,116,111,107,46,112,
111,115,41,0,12,8,0,6,115,101,116,112,111,115,0,0,
13,7,8,0,12,9,0,3,112,111,115,0,9,8,1,9,
31,6,8,1,19,6,7,6,30,13,1,180,32,32,32,32,
114,32,61,32,100,111,95,108,111,99,97,108,40,84,111,107,
101,110,40,116,111,107,46,112,111,115,44,39,110,97,109,101,
39,44,39,95,95,112,97,114,97,109,115,39,41,41,0,0,
12,9,0,8,100,111,95,108,111,99,97,108,0,0,0,0,
13,8,9,0,12,11,0,5,84,111,107,101,110,0,0,0,
13,10,11,0,12,14,0,3,112,111,115,0,9,11,1,14,
12,12,0,4,110,97,109,101,0,0,0,0,12,13,0,8,
95,95,112,97,114,97,109,115,0,0,0,0,31,9,11,3,
19,9,10,9,31,7,9,1,19,7,8,7,15,6,7,0,
30,7,1,181,32,32,32,32,100,111,95,105,110,102,111,40,
105,116,101,109,115,91,48,93,46,118,97,108,41,0,0,0,
12,9,0,7,100,111,95,105,110,102,111,0,13,8,9,0,
11,10,0,0,0,0,0,0,0,0,0,0,9,9,3,10,
12,10,0,3,118,97,108,0,9,9,9,10,31,7,9,1,
19,7,8,7,30,10,1,182,32,32,32,32,97,44,98,44,
99,44,100,32,61,32,112,95,102,105,108,116,101,114,40,105,
116,101,109,115,91,49,93,46,105,116,101,109,115,41,0,0,
12,9,0,8,112,95,102,105,108,116,101,114,0,0,0,0,
13,8,9,0,11,10,0,0,0,0,0,0,0,0,240,63,
9,9,3,10,12,10,0,5,105,116,101,109,115,0,0,0,
9,9,9,10,31,7,9,1,19,7,8,7,11,10,0,0,
0,0,0,0,0,0,0,0,9,9,7,10,15,8,9,0,
11,11,0,0,0,0,0,0,0,0,240,63,9,10,7,11,
15,9,10,0,11,12,0,0,0,0,0,0,0,0,0,64,
9,11,7,12,15,10,11,0,11,13,0,0,0,0,0,0,
0,0,8,64,9,12,7,13,15,11,12,0,30,4,1,183,
32,32,32,32,102,111,114,32,112,32,105,110,32,97,58,0,
11,12,0,0,0,0,0,0,0,0,0,0,42,7,8,12,
18,0,0,70,30,6,1,184,32,32,32,32,32,32,32,32,
118,32,61,32,100,111,95,108,111,99,97,108,40,112,41,0,
12,16,0,8,100,111,95,108,111,99,97,108,0,0,0,0,
13,15,16,0,15,16,7,0,31,14,16,1,19,14,15,14,
15,13,14,0,30,7,1,185,32,32,32,32,32,32,32,32,
116,109,112,32,61,32,95,100,111,95,110,111,110,101,40,41,
0,0,0,0,12,17,0,8,95,100,111,95,110,111,110,101,
0,0,0,0,13,16,17,0,31,15,0,0,19,15,16,15,
15,14,15,0,30,7,1,186,32,32,32,32,32,32,32,32,
99,111,100,101,40,71,69,84,44,118,44,114,44,116,109,112,
41,0,0,0,12,17,0,4,99,111,100,101,0,0,0,0,
13,16,17,0,12,21,0,3,71,69,84,0,13,17,21,0,
15,18,13,0,15,19,6,0,15,20,14,0,31,15,17,4,
19,15,16,15,30,7,1,187,32,32,32,32,32,32,32,32,
102,114,101,101,95,116,109,112,40,116,109,112,41,32,35,82,
69,71,0,0,12,17,0,8,102,114,101,101,95,116,109,112,
0,0,0,0,13,16,17,0,15,17,14,0,31,15,17,1,
19,15,16,15,18,0,255,186,30,4,1,188,32,32,32,32,
102,111,114,32,112,32,105,110,32,98,58,0,11,12,0,0,
0,0,0,0,0,0,0,0,42,7,9,12,18,0,0,103,
30,9,1,189,32,32,32,32,32,32,32,32,118,32,61,32,
100,111,95,108,111,99,97,108,40,112,46,105,116,101,109,115,
91,48,93,41,0,0,0,0,12,17,0,8,100,111,95,108,
111,99,97,108,0,0,0,0,13,16,17,0,12,18,0,5,
105,116,101,109,115,0,0,0,9,17,7,18,11,18,0,0,
0,0,0,0,0,0,0,0,9,17,17,18,31,15,17,1,
19,15,16,15,15,13,15,0,30,7,1,190,32,32,32,32,
32,32,32,32,100,111,40,112,46,105,116,101,109,115,91,49,
93,44,118,41,0,0,0,0,12,17,0,2,100,111,0,0,
13,16,17,0,12,19,0,5,105,116,101,109,115,0,0,0,
9,17,7,19,11,19,0,0,0,0,0,0,0,0,240,63,
9,17,17,19,15,18,13,0,31,15,17,2,19,15,16,15,
30,7,1,191,32,32,32,32,32,32,32,32,116,109,112,32,
61,32,95,100,111,95,110,111,110,101,40,41,0,0,0,0,
12,17,0,8,95,100,111,95,110,111,110,101,0,0,0,0,
13,16,17,0,31,15,0,0,19,15,16,15,15,14,15,0,
30,7,1,192,32,32,32,32,32,32,32,32,99,111,100,101,
40,73,71,69,84,44,118,44,114,44,116,109,112,41,0,0,
12,17,0,4,99,111,100,101,0,0,0,0,13,16,17,0,
12,21,0,4,73,71,69,84,0,0,0,0,13,17,21,0,
15,18,13,0,15,19,6,0,15,20,14,0,31,15,17,4,
19,15,16,15,30,7,1,193,32,32,32,32,32,32,32,32,
102,114,101,101,95,116,109,112,40,116,109,112,41,32,35,82,
69,71,0,0,12,17,0,8,102,114,101,101,95,116,109,112,
0,0,0,0,13,16,17,0,15,17,14,0,31,15,17,1,
19,15,16,15,18,0,255,153,30,5,1,194,32,32,32,32,
105,102,32,99,32,33,61,32,78,111,110,101,58,0,0,0,
28,15,0,0,35,12,10,15,21,12,0,0,18,0,0,83,
30,9,1,195,32,32,32,32,32,32,32,32,118,32,61,32,
100,111,95,108,111,99,97,108,40,99,46,105,116,101,109,115,
91,48,93,41,0,0,0,0,12,16,0,8,100,111,95,108,
111,99,97,108,0,0,0,0,13,15,16,0,12,17,0,5,
105,116,101,109,115,0,0,0,9,16,10,17,11,17,0,0,
0,0,0,0,0,0,0,0,9,16,16,17,31,12,16,1,
19,12,15,12,15,13,12,0,30,8,1,196,32,32,32,32,
32,32,32,32,116,109,112,32,61,32,95,100,111,95,115,116,
114,105,110,103,40,39,42,39,41,0,0,0,12,16,0,10,
95,100,111,95,115,116,114,105,110,103,0,0,13,15,16,0,
12,16,0,1,42,0,0,0,31,12,16,1,19,12,15,12,
15,14,12,0,30,7,1,197,32,32,32,32,32,32,32,32,
99,111,100,101,40,71,69,84,44,118,44,114,44,116,109,112,
41,0,0,0,12,16,0,4,99,111,100,101,0,0,0,0,
13,15,16,0,12,20,0,3,71,69,84,0,13,16,20,0,
15,17,13,0,15,18,6,0,15,19,14,0,31,12,16,4,
19,12,15,12,30,7,1,198,32,32,32,32,32,32,32,32,
102,114,101,101,95,116,109,112,40,116,109,112,41,32,35,82,
69,71,0,0,12,16,0,8,102,114,101,101,95,116,109,112,
0,0,0,0,13,15,16,0,15,16,14,0,31,12,16,1,
19,12,15,12,18,0,0,1,30,5,1,199,32,32,32,32,
105,102,32,100,32,33,61,32,78,111,110,101,58,0,0,0,
28,15,0,0,35,12,11,15,21,12,0,0,18,0,0,106,
30,9,1,200,32,32,32,32,32,32,32,32,101,32,61,32,
100,111,95,108,111,99,97,108,40,100,46,105,116,101,109,115,
91,48,93,41,0,0,0,0,12,17,0,8,100,111,95,108,
111,99,97,108,0,0,0,0,13,16,17,0,12,18,0,5,
105,116,101,109,115,0,0,0,9,17,11,18,11,18,0,0,
0,0,0,0,0,0,0,0,9,17,17,18,31,15,17,1,
19,15,16,15,15,12,15,0,30,7,1,201,32,32,32,32,
32,32,32,32,99,111,100,101,40,68,73,67,84,44,101,44,
48,44,48,41,0,0,0,0,12,17,0,4,99,111,100,101,
0,0,0,0,13,16,17,0,12,21,0,4,68,73,67,84,
0,0,0,0,13,17,21,0,15,18,12,0,11,19,0,0,
0,0,0,0,0,0,0,0,11,20,0,0,0,0,0,0,
0,0,0,0,31,15,17,4,19,15,16,15,30,7,1,202,
32,32,32,32,32,32,32,32,116,109,112,32,61,32,95,100,
111,95,110,111,110,101,40,41,0,0,0,0,12,17,0,8,
95,100,111,95,110,111,110,101,0,0,0,0,13,16,17,0,
31,15,0,0,19,15,16,15,15,14,15,0,30,7,1,203,
32,32,32,32,32,32,32,32,99,111,100,101,40,73,71,69,
84,44,101,44,114,44,116,109,112,41,0,0,12,17,0,4,
99,111,100,101,0,0,0,0,13,16,17,0,12,21,0,4,
73,71,69,84,0,0,0,0,13,17,21,0,15,18,12,0,
15,19,6,0,15,20,14,0,31,15,17,4,19,15,16,15,
30,7,1,204,32,32,32,32,32,32,32,32,102,114,101,101,
95,116,109,112,40,116,109,112,41,32,35,82,69,71,0,0,
12,17,0,8,102,114,101,101,95,116,109,112,0,0,0,0,
13,16,17,0,15,17,14,0,31,15,17,1,19,15,16,15,
18,0,0,1,30,8,1,205,32,32,32,32,102,114,101,101,
95,116,109,112,40,100,111,40,105,116,101,109,115,91,50,93,
41,41,32,35,82,69,71,0,12,17,0,8,102,114,101,101,
95,116,109,112,0,0,0,0,13,16,17,0,12,19,0,2,
100,111,0,0,13,18,19,0,11,20,0,0,0,0,0,0,
0,0,0,64,9,19,3,20,31,17,19,1,19,17,18,17,
31,15,17,1,19,15,16,15,30,3,1,206,32,32,32,32,
68,46,101,110,100,40,41,0,12,17,0,1,68,0,0,0,
13,16,17,0,12,17,0,3,101,110,100,0,9,16,16,17,
31,15,0,0,19,15,16,15,30,5,1,208,32,32,32,32,
116,97,103,40,116,44,39,101,110,100,39,41,0,0,0,0,
12,17,0,3,116,97,103,0,13,16,17,0,15,17,4,0,
12,18,0,3,101,110,100,0,31,15,17,2,19,15,16,15,
30,5,1,210,32,32,32,32,105,102,32,107,108,115,32,61,
61,32,78,111,110,101,58,0,28,16,0,0,23,15,2,16,
21,15,0,0,18,0,0,98,30,17,1,211,32,32,32,32,
32,32,32,32,105,102,32,68,46,95,103,108,111,98,97,108,
115,58,32,100,111,95,103,108,111,98,97,108,115,40,84,111,
107,101,110,40,116,111,107,46,112,111,115,44,48,44,48,44,
91,105,116,101,109,115,91,48,93,93,41,41,0,0,0,0,
12,16,0,1,68,0,0,0,13,15,16,0,12,16,0,8,
95,103,108,111,98,97,108,115,0,0,0,0,9,15,15,16,
21,15,0,0,18,0,0,29,12,17,0,10,100,111,95,103,
108,111,98,97,108,115,0,0,13,16,17,0,12,19,0,5,
84,111,107,101,110,0,0,0,13,18,19,0,12,23,0,3,
112,111,115,0,9,19,1,23,11,20,0,0,0,0,0,0,
0,0,0,0,11,21,0,0,0,0,0,0,0,0,0,0,
11,24,0,0,0,0,0,0,0,0,0,0,9,23,3,24,
27,22,23,1,31,17,19,4,19,17,18,17,31,15,17,1,
19,15,16,15,18,0,0,1,30,15,1,212,32,32,32,32,
32,32,32,32,114,32,61,32,100,111,95,115,101,116,95,99,
116,120,40,105,116,101,109,115,91,48,93,44,84,111,107,101,
110,40,116,111,107,46,112,111,115,44,39,114,101,103,39,44,
114,102,41,41,0,0,0,0,12,17,0,10,100,111,95,115,
101,116,95,99,116,120,0,0,13,16,17,0,11,19,0,0,
0,0,0,0,0,0,0,0,9,17,3,19,12,20,0,5,
84,111,107,101,110,0,0,0,13,19,20,0,12,23,0,3,
112,111,115,0,9,20,1,23,12,21,0,3,114,101,103,0,
15,22,5,0,31,18,20,3,19,18,19,18,31,15,17,2,
19,15,16,15,15,6,15,0,18,0,0,63,30,3,1,213,
32,32,32,32,101,108,115,101,58,0,0,0,30,9,1,214,
32,32,32,32,32,32,32,32,114,110,32,61,32,100,111,95,
115,116,114,105,110,103,40,105,116,101,109,115,91,48,93,41,
0,0,0,0,12,18,0,9,100,111,95,115,116,114,105,110,
103,0,0,0,13,17,18,0,11,19,0,0,0,0,0,0,
0,0,0,0,9,18,3,19,31,16,18,1,19,16,17,16,
15,15,16,0,30,7,1,215,32,32,32,32,32,32,32,32,
99,111,100,101,40,83,69,84,44,107,108,115,44,114,110,44,
114,102,41,0,12,18,0,4,99,111,100,101,0,0,0,0,
13,17,18,0,12,22,0,3,83,69,84,0,13,18,22,0,
15,19,2,0,15,20,15,0,15,21,5,0,31,16,18,4,
19,16,17,16,30,6,1,216,32,32,32,32,32,32,32,32,
102,114,101,101,95,116,109,112,40,114,110,41,0,0,0,0,
12,18,0,8,102,114,101,101,95,116,109,112,0,0,0,0,
13,17,18,0,15,18,15,0,31,16,18,1,19,16,17,16,
18,0,0,1,30,5,1,218,32,32,32,32,102,114,101,101,
95,116,109,112,40,114,102,41,0,0,0,0,12,18,0,8,
102,114,101,101,95,116,109,112,0,0,0,0,13,17,18,0,
15,18,5,0,31,16,18,1,19,16,17,16,0,0,0,0,
12,48,0,6,100,111,95,100,101,102,0,0,14,48,47,0,
30,5,1,220,100,101,102,32,100,111,95,99,108,97,115,115,
40,116,41,58,0,0,0,0,16,48,1,233,44,26,0,0,
30,5,1,220,100,101,102,32,100,111,95,99,108,97,115,115,
40,116,41,58,0,0,0,0,12,1,0,9,101,110,99,111,
100,101,46,112,121,0,0,0,33,1,0,0,12,1,0,8,
100,111,95,99,108,97,115,115,0,0,0,0,34,1,0,0,
28,2,0,0,9,1,0,2,30,3,1,221,32,32,32,32,
116,111,107,32,61,32,116,0,15,2,1,0,30,5,1,222,
32,32,32,32,105,116,101,109,115,32,61,32,116,46,105,116,
101,109,115,0,12,5,0,5,105,116,101,109,115,0,0,0,
9,4,1,5,15,3,4,0,30,5,1,223,32,32,32,32,
112,97,114,101,110,116,32,61,32,78,111,110,101,0,0,0,
28,5,0,0,15,4,5,0,30,8,1,224,32,32,32,32,
105,102,32,105,116,101,109,115,91,48,93,46,116,121,112,101,
32,61,61,32,39,110,97,109,101,39,58,0,11,6,0,0,
0,0,0,0,0,0,0,0,9,5,3,6,12,6,0,4,
116,121,112,101,0,0,0,0,9,5,5,6,12,6,0,4,
110,97,109,101,0,0,0,0,23,5,5,6,21,5,0,0,
18,0,0,47,30,7,1,225,32,32,32,32,32,32,32,32,
110,97,109,101,32,61,32,105,116,101,109,115,91,48,93,46,
118,97,108,0,11,7,0,0,0,0,0,0,0,0,0,0,
9,6,3,7,12,7,0,3,118,97,108,0,9,6,6,7,
15,5,6,0,30,12,1,226,32,32,32,32,32,32,32,32,
112,97,114,101,110,116,32,61,32,84,111,107,101,110,40,116,
111,107,46,112,111,115,44,39,110,97,109,101,39,44,39,111,
98,106,101,99,116,39,41,0,12,8,0,5,84,111,107,101,
110,0,0,0,13,7,8,0,12,11,0,3,112,111,115,0,
9,8,2,11,12,9,0,4,110,97,109,101,0,0,0,0,
12,10,0,6,111,98,106,101,99,116,0,0,31,6,8,3,
19,6,7,6,15,4,6,0,18,0,0,56,30,3,1,227,
32,32,32,32,101,108,115,101,58,0,0,0,30,10,1,228,
32,32,32,32,32,32,32,32,110,97,109,101,32,61,32,105,
116,101,109,115,91,48,93,46,105,116,101,109,115,91,48,93,
46,118,97,108,0,0,0,0,11,7,0,0,0,0,0,0,
0,0,0,0,9,6,3,7,12,7,0,5,105,116,101,109,
115,0,0,0,9,6,6,7,11,7,0,0,0,0,0,0,
0,0,0,0,9,6,6,7,12,7,0,3,118,97,108,0,
9,6,6,7,15,5,6,0,30,9,1,229,32,32,32,32,
32,32,32,32,112,97,114,101,110,116,32,61,32,105,116,101,
109,115,91,48,93,46,105,116,101,109,115,91,49,93,0,0,
11,7,0,0,0,0,0,0,0,0,0,0,9,6,3,7,
12,7,0,5,105,116,101,109,115,0,0,0,9,6,6,7,
11,7,0,0,0,0,0,0,0,0,240,63,9,6,6,7,
15,4,6,0,18,0,0,1,30,10,1,231,32,32,32,32,
107,108,115,32,61,32,100,111,40,84,111,107,101,110,40,116,
46,112,111,115,44,39,100,105,99,116,39,44,48,44,91,93,
41,41,0,0,12,9,0,2,100,111,0,0,13,8,9,0,
12,11,0,5,84,111,107,101,110,0,0,0,13,10,11,0,
12,15,0,3,112,111,115,0,9,11,1,15,12,12,0,4,
100,105,99,116,0,0,0,0,11,13,0,0,0,0,0,0,
0,0,0,0,27,14,0,0,31,9,11,4,19,9,10,9,
31,7,9,1,19,7,8,7,15,6,7,0,30,4,1,232,
32,32,32,32,117,110,95,116,109,112,40,107,108,115,41,0,
12,9,0,6,117,110,95,116,109,112,0,0,13,8,9,0,
15,9,6,0,31,7,9,1,19,7,8,7,30,7,1,233,
32,32,32,32,116,115,32,61,32,95,100,111,95,115,116,114,
105,110,103,40,110,97,109,101,41,0,0,0,12,10,0,10,
95,100,111,95,115,116,114,105,110,103,0,0,13,9,10,0,
15,10,5,0,31,8,10,1,19,8,9,8,15,7,8,0,
30,6,1,234,32,32,32,32,99,111,100,101,40,71,83,69,
84,44,116,115,44,107,108,115,41,0,0,0,12,10,0,4,
99,111,100,101,0,0,0,0,13,9,10,0,12,13,0,4,
71,83,69,84,0,0,0,0,13,10,13,0,15,11,7,0,
15,12,6,0,31,8,10,3,19,8,9,8,30,6,1,235,
32,32,32,32,102,114,101,101,95,116,109,112,40,116,115,41,
32,35,82,69,71,0,0,0,12,10,0,8,102,114,101,101,
95,116,109,112,0,0,0,0,13,9,10,0,15,10,7,0,
31,8,10,1,19,8,9,8,30,11,1,237,32,32,32,32,
102,114,101,101,95,116,109,112,40,100,111,40,84,111,107,101,
110,40,116,111,107,46,112,111,115,44,39,99,97,108,108,39,
44,78,111,110,101,44,91,0,12,10,0,8,102,114,101,101,
95,116,109,112,0,0,0,0,13,9,10,0,12,12,0,2,
100,111,0,0,13,11,12,0,12,14,0,5,84,111,107,101,
110,0,0,0,13,13,14,0,12,18,0,3,112,111,115,0,
9,14,2,18,12,15,0,4,99,97,108,108,0,0,0,0,
28,16,0,0,30,11,1,238,32,32,32,32,32,32,32,32,
84,111,107,101,110,40,116,111,107,46,112,111,115,44,39,110,
97,109,101,39,44,39,115,101,116,109,101,116,97,39,41,44,
0,0,0,0,12,22,0,5,84,111,107,101,110,0,0,0,
13,21,22,0,12,25,0,3,112,111,115,0,9,22,2,25,
12,23,0,4,110,97,109,101,0,0,0,0,12,24,0,7,
115,101,116,109,101,116,97,0,31,18,22,3,19,18,21,18,
30,9,1,239,32,32,32,32,32,32,32,32,84,111,107,101,
110,40,116,111,107,46,112,111,115,44,39,114,101,103,39,44,
107,108,115,41,44,0,0,0,12,22,0,5,84,111,107,101,
110,0,0,0,13,21,22,0,12,25,0,3,112,111,115,0,
9,22,2,25,12,23,0,3,114,101,103,0,15,24,6,0,
31,19,22,3,19,19,21,19,30,5,1,240,32,32,32,32,
32,32,32,32,112,97,114,101,110,116,93,41,41,41,0,0,
15,20,4,0,27,17,18,3,31,12,14,4,19,12,13,12,
31,10,12,1,19,10,11,10,31,8,10,1,19,8,9,8,
30,9,1,242,32,32,32,32,102,111,114,32,109,101,109,98,
101,114,32,105,110,32,105,116,101,109,115,91,49,93,46,105,
116,101,109,115,58,0,0,0,11,10,0,0,0,0,0,0,
0,0,240,63,9,9,3,10,12,10,0,5,105,116,101,109,
115,0,0,0,9,9,9,10,11,10,0,0,0,0,0,0,
0,0,0,0,42,8,9,10,18,0,0,93,30,13,1,243,
32,32,32,32,32,32,32,32,105,102,32,109,101,109,98,101,
114,46,116,121,112,101,32,61,61,32,39,100,101,102,39,58,
32,100,111,95,100,101,102,40,109,101,109,98,101,114,44,107,
108,115,41,0,12,12,0,4,116,121,112,101,0,0,0,0,
9,11,8,12,12,12,0,3,100,101,102,0,23,11,11,12,
21,11,0,0,18,0,0,10,12,13,0,6,100,111,95,100,
101,102,0,0,13,12,13,0,15,13,8,0,15,14,6,0,
31,11,13,2,19,11,12,11,18,0,0,60,30,21,1,244,
32,32,32,32,32,32,32,32,101,108,105,102,32,109,101,109,
98,101,114,46,116,121,112,101,32,61,61,32,39,115,121,109,
98,111,108,39,32,97,110,100,32,109,101,109,98,101,114,46,
118,97,108,32,61,61,32,39,61,39,58,32,100,111,95,99,
108,97,115,115,118,97,114,40,109,101,109,98,101,114,44,107,
108,115,41,0,12,12,0,4,116,121,112,101,0,0,0,0,
9,11,8,12,12,12,0,6,115,121,109,98,111,108,0,0,
23,11,11,12,21,11,0,0,18,0,0,7,12,12,0,3,
118,97,108,0,9,11,8,12,12,12,0,1,61,0,0,0,
23,11,11,12,21,11,0,0,18,0,0,11,12,13,0,11,
100,111,95,99,108,97,115,115,118,97,114,0,13,12,13,0,
15,13,8,0,15,14,6,0,31,11,13,2,19,11,12,11,
18,0,0,10,30,6,1,245,32,32,32,32,32,32,32,32,
101,108,115,101,58,32,99,111,110,116,105,110,117,101,0,0,
18,0,255,165,18,0,0,1,18,0,255,163,30,6,1,247,
32,32,32,32,102,114,101,101,95,114,101,103,40,107,108,115,
41,32,35,82,69,71,0,0,12,11,0,8,102,114,101,101,
95,114,101,103,0,0,0,0,13,10,11,0,15,11,6,0,
31,9,11,1,19,9,10,9,0,0,0,0,12,49,0,8,
100,111,95,99,108,97,115,115,0,0,0,0,14,49,48,0,
30,6,1,249,100,101,102,32,100,111,95,99,108,97,115,115,
118,97,114,40,116,44,114,41,58,0,0,0,16,49,0,118,
44,12,0,0,30,6,1,249,100,101,102,32,100,111,95,99,
108,97,115,115,118,97,114,40,116,44,114,41,58,0,0,0,
12,1,0,9,101,110,99,111,100,101,46,112,121,0,0,0,
33,1,0,0,12,1,0,11,100,111,95,99,108,97,115,115,
118,97,114,0,34,1,0,0,28,2,0,0,9,1,0,2,
28,3,0,0,9,2,0,3,30,8,1,250,32,32,32,32,
118,97,114,32,61,32,100,111,95,115,116,114,105,110,103,40,
116,46,105,116,101,109,115,91,48,93,41,0,12,6,0,9,
100,111,95,115,116,114,105,110,103,0,0,0,13,5,6,0,
12,7,0,5,105,116,101,109,115,0,0,0,9,6,1,7,
11,7,0,0,0,0,0,0,0,0,0,0,9,6,6,7,
31,4,6,1,19,4,5,4,15,3,4,0,30,7,1,251,
32,32,32,32,118,97,108,32,61,32,100,111,40,116,46,105,
116,101,109,115,91,49,93,41,0,0,0,0,12,7,0,2,
100,111,0,0,13,6,7,0,12,8,0,5,105,116,101,109,
115,0,0,0,9,7,1,8,11,8,0,0,0,0,0,0,
0,0,240,63,9,7,7,8,31,5,7,1,19,5,6,5,
15,4,5,0,30,6,1,252,32,32,32,32,99,111,100,101,
40,83,69,84,44,114,44,118,97,114,44,118,97,108,41,0,
12,7,0,4,99,111,100,101,0,0,0,0,13,6,7,0,
12,11,0,3,83,69,84,0,13,7,11,0,15,8,2,0,
15,9,3,0,15,10,4,0,31,5,7,4,19,5,6,5,
30,5,1,253,32,32,32,32,102,114,101,101,95,114,101,103,
40,118,97,114,41,0,0,0,12,7,0,8,102,114,101,101,
95,114,101,103,0,0,0,0,13,6,7,0,15,7,3,0,
31,5,7,1,19,5,6,5,30,5,1,254,32,32,32,32,
102,114,101,101,95,114,101,103,40,118,97,108,41,0,0,0,
12,7,0,8,102,114,101,101,95,114,101,103,0,0,0,0,
13,6,7,0,15,7,4,0,31,5,7,1,19,5,6,5,
0,0,0,0,12,50,0,11,100,111,95,99,108,97,115,115,
118,97,114,0,14,50,49,0,30,5,2,0,100,101,102,32,
100,111,95,119,104,105,108,101,40,116,41,58,0,0,0,0,
16,50,0,221,44,10,0,0,30,5,2,0,100,101,102,32,
100,111,95,119,104,105,108,101,40,116,41,58,0,0,0,0,
12,1,0,9,101,110,99,111,100,101,46,112,121,0,0,0,
33,1,0,0,12,1,0,8,100,111,95,119,104,105,108,101,
0,0,0,0,34,1,0,0,28,2,0,0,9,1,0,2,
30,5,2,1,32,32,32,32,105,116,101,109,115,32,61,32,
116,46,105,116,101,109,115,0,12,4,0,5,105,116,101,109,
115,0,0,0,9,3,1,4,15,2,3,0,30,5,2,2,
32,32,32,32,116,32,61,32,115,116,97,99,107,95,116,97,
103,40,41,0,12,5,0,9,115,116,97,99,107,95,116,97,
103,0,0,0,13,4,5,0,31,3,0,0,19,3,4,3,
15,1,3,0,30,5,2,3,32,32,32,32,116,97,103,40,
116,44,39,98,101,103,105,110,39,41,0,0,12,5,0,3,
116,97,103,0,13,4,5,0,15,5,1,0,12,6,0,5,
98,101,103,105,110,0,0,0,31,3,5,2,19,3,4,3,
30,6,2,4,32,32,32,32,116,97,103,40,116,44,39,99,
111,110,116,105,110,117,101,39,41,0,0,0,12,5,0,3,
116,97,103,0,13,4,5,0,15,5,1,0,12,6,0,8,
99,111,110,116,105,110,117,101,0,0,0,0,31,3,5,2,
19,3,4,3,30,6,2,5,32,32,32,32,114,32,61,32,
100,111,40,105,116,101,109,115,91,48,93,41,0,0,0,0,
12,6,0,2,100,111,0,0,13,5,6,0,11,7,0,0,
0,0,0,0,0,0,0,0,9,6,2,7,31,4,6,1,
19,4,5,4,15,3,4,0,30,4,2,6,32,32,32,32,
99,111,100,101,40,73,70,44,114,41,0,0,12,6,0,4,
99,111,100,101,0,0,0,0,13,5,6,0,12,8,0,2,
73,70,0,0,13,6,8,0,15,7,3,0,31,4,6,2,
19,4,5,4,30,6,2,7,32,32,32,32,102,114,101,101,
95,116,109,112,40,114,41,32,35,82,69,71,0,0,0,0,
12,6,0,8,102,114,101,101,95,116,109,112,0,0,0,0,
13,5,6,0,15,6,3,0,31,4,6,1,19,4,5,4,
30,5,2,8,32,32,32,32,106,117,109,112,40,116,44,39,
101,110,100,39,41,0,0,0,12,6,0,4,106,117,109,112,
0,0,0,0,13,5,6,0,15,6,1,0,12,7,0,3,
101,110,100,0,31,4,6,2,19,4,5,4,30,8,2,9,
32,32,32,32,102,114,101,101,95,116,109,112,40,100,111,40,
105,116,101,109,115,91,49,93,41,41,32,35,82,69,71,0,
12,6,0,8,102,114,101,101,95,116,109,112,0,0,0,0,
13,5,6,0,12,8,0,2,100,111,0,0,13,7,8,0,
11,9,0,0,0,0,0,0,0,0,240,63,9,8,2,9,
31,6,8,1,19,6,7,6,31,4,6,1,19,4,5,4,
30,5,2,10,32,32,32,32,106,117,109,112,40,116,44,39,
98,101,103,105,110,39,41,0,12,6,0,4,106,117,109,112,
0,0,0,0,13,5,6,0,15,6,1,0,12,7,0,5,
98,101,103,105,110,0,0,0,31,4,6,2,19,4,5,4,
30,5,2,11,32,32,32,32,116,97,103,40,116,44,39,98,
114,101,97,107,39,41,0,0,12,6,0,3,116,97,103,0,
13,5,6,0,15,6,1,0,12,7,0,5,98,114,101,97,
107,0,0,0,31,4,6,2,19,4,5,4,30,5,2,12,
32,32,32,32,116,97,103,40,116,44,39,101,110,100,39,41,
0,0,0,0,12,6,0,3,116,97,103,0,13,5,6,0,
15,6,1,0,12,7,0,3,101,110,100,0,31,4,6,2,
19,4,5,4,30,4,2,13,32,32,32,32,112,111,112,95,
116,97,103,40,41,0,0,0,12,6,0,7,112,111,112,95,
116,97,103,0,13,5,6,0,31,4,0,0,19,4,5,4,
0,0,0,0,12,51,0,8,100,111,95,119,104,105,108,101,
0,0,0,0,14,51,50,0,30,5,2,15,100,101,102,32,
100,111,95,102,111,114,40,116,111,107,41,58,0,0,0,0,
16,51,1,10,44,14,0,0,30,5,2,15,100,101,102,32,
100,111,95,102,111,114,40,116,111,107,41,58,0,0,0,0,
12,1,0,9,101,110,99,111,100,101,46,112,121,0,0,0,
33,1,0,0,12,1,0,6,100,111,95,102,111,114,0,0,
34,1,0,0,28,2,0,0,9,1,0,2,30,6,2,16,
32,32,32,32,105,116,101,109,115,32,61,32,116,111,107,46,
105,116,101,109,115,0,0,0,12,4,0,5,105,116,101,109,
115,0,0,0,9,3,1,4,15,2,3,0,30,8,2,18,
32,32,32,32,114,101,103,32,61,32,100,111,95,108,111,99,
97,108,40,105,116,101,109,115,91,48,93,41,0,0,0,0,
12,6,0,8,100,111,95,108,111,99,97,108,0,0,0,0,
13,5,6,0,11,7,0,0,0,0,0,0,0,0,0,0,
9,6,2,7,31,4,6,1,19,4,5,4,15,3,4,0,
30,6,2,19,32,32,32,32,105,116,114,32,61,32,100,111,
40,105,116,101,109,115,91,49,93,41,0,0,12,7,0,2,
100,111,0,0,13,6,7,0,11,8,0,0,0,0,0,0,
0,0,240,63,9,7,2,8,31,5,7,1,19,5,6,5,
15,4,5,0,30,6,2,20,32,32,32,32,105,32,61,32,
95,100,111,95,110,117,109,98,101,114,40,39,48,39,41,0,
12,8,0,10,95,100,111,95,110,117,109,98,101,114,0,0,
13,7,8,0,12,8,0,1,48,0,0,0,31,6,8,1,
19,6,7,6,15,5,6,0,30,14,2,22,32,32,32,32,
116,32,61,32,115,116,97,99,107,95,116,97,103,40,41,59,
32,116,97,103,40,116,44,39,108,111,111,112,39,41,59,32,
116,97,103,40,116,44,39,99,111,110,116,105,110,117,101,39,
41,0,0,0,12,9,0,9,115,116,97,99,107,95,116,97,
103,0,0,0,13,8,9,0,31,7,0,0,19,7,8,7,
15,6,7,0,12,9,0,3,116,97,103,0,13,8,9,0,
15,9,6,0,12,10,0,4,108,111,111,112,0,0,0,0,
31,7,9,2,19,7,8,7,12,9,0,3,116,97,103,0,
13,8,9,0,15,9,6,0,12,10,0,8,99,111,110,116,
105,110,117,101,0,0,0,0,31,7,9,2,19,7,8,7,
30,10,2,23,32,32,32,32,99,111,100,101,40,73,84,69,
82,44,114,101,103,44,105,116,114,44,105,41,59,32,106,117,
109,112,40,116,44,39,101,110,100,39,41,0,12,9,0,4,
99,111,100,101,0,0,0,0,13,8,9,0,12,13,0,4,
73,84,69,82,0,0,0,0,13,9,13,0,15,10,3,0,
15,11,4,0,15,12,5,0,31,7,9,4,19,7,8,7,
12,9,0,4,106,117,109,112,0,0,0,0,13,8,9,0,
15,9,6,0,12,10,0,3,101,110,100,0,31,7,9,2,
19,7,8,7,30,8,2,24,32,32,32,32,102,114,101,101,
95,116,109,112,40,100,111,40,105,116,101,109,115,91,50,93,
41,41,32,35,82,69,71,0,12,9,0,8,102,114,101,101,
95,116,109,112,0,0,0,0,13,8,9,0,12,11,0,2,
100,111,0,0,13,10,11,0,11,12,0,0,0,0,0,0,
0,0,0,64,9,11,2,12,31,9,11,1,19,9,10,9,
31,7,9,1,19,7,8,7,30,5,2,25,32,32,32,32,
106,117,109,112,40,116,44,39,108,111,111,112,39,41,0,0,
12,9,0,4,106,117,109,112,0,0,0,0,13,8,9,0,
15,9,6,0,12,10,0,4,108,111,111,112,0,0,0,0,
31,7,9,2,19,7,8,7,30,11,2,26,32,32,32,32,
116,97,103,40,116,44,39,98,114,101,97,107,39,41,59,32,
116,97,103,40,116,44,39,101,110,100,39,41,59,32,112,111,
112,95,116,97,103,40,41,0,12,9,0,3,116,97,103,0,
13,8,9,0,15,9,6,0,12,10,0,5,98,114,101,97,
107,0,0,0,31,7,9,2,19,7,8,7,12,9,0,3,
116,97,103,0,13,8,9,0,15,9,6,0,12,10,0,3,
101,110,100,0,31,7,9,2,19,7,8,7,12,9,0,7,
112,111,112,95,116,97,103,0,13,8,9,0,31,7,0,0,
19,7,8,7,30,6,2,28,32,32,32,32,102,114,101,101,
95,116,109,112,40,105,116,114,41,32,35,82,69,71,0,0,
12,9,0,8,102,114,101,101,95,116,109,112,0,0,0,0,
13,8,9,0,15,9,4,0,31,7,9,1,19,7,8,7,
30,4,2,29,32,32,32,32,102,114,101,101,95,116,109,112,
40,105,41,0,12,9,0,8,102,114,101,101,95,116,109,112,
0,0,0,0,13,8,9,0,15,9,5,0,31,7,9,1,
19,7,8,7,0,0,0,0,12,52,0,6,100,111,95,102,
111,114,0,0,14,52,51,0,30,6,2,31,100,101,102,32,
100,111,95,99,111,109,112,40,116,44,114,61,78,111,110,101,
41,58,0,0,16,52,1,14,44,18,0,0,30,6,2,31,
100,101,102,32,100,111,95,99,111,109,112,40,116,44,114,61,
78,111,110,101,41,58,0,0,12,1,0,9,101,110,99,111,
100,101,46,112,121,0,0,0,33,1,0,0,12,1,0,7,
100,111,95,99,111,109,112,0,34,1,0,0,28,2,0,0,
9,1,0,2,28,2,0,0,28,3,0,0,32,2,0,3,
30,8,2,32,32,32,32,32,110,97,109,101,32,61,32,39,
99,111,109,112,58,39,43,103,101,116,95,116,97,103,40,41,
0,0,0,0,12,4,0,5,99,111,109,112,58,0,0,0,
12,7,0,7,103,101,116,95,116,97,103,0,13,6,7,0,
31,5,0,0,19,5,6,5,1,4,4,5,15,3,4,0,
30,11,2,33,32,32,32,32,114,32,61,32,100,111,95,108,
111,99,97,108,40,84,111,107,101,110,40,116,46,112,111,115,
44,39,110,97,109,101,39,44,110,97,109,101,41,41,0,0,
12,6,0,8,100,111,95,108,111,99,97,108,0,0,0,0,
13,5,6,0,12,8,0,5,84,111,107,101,110,0,0,0,
13,7,8,0,12,11,0,3,112,111,115,0,9,8,1,11,
12,9,0,4,110,97,109,101,0,0,0,0,15,10,3,0,
31,6,8,3,19,6,7,6,31,4,6,1,19,4,5,4,
15,2,4,0,30,6,2,34,32,32,32,32,99,111,100,101,
40,76,73,83,84,44,114,44,48,44,48,41,0,0,0,0,
12,6,0,4,99,111,100,101,0,0,0,0,13,5,6,0,
12,10,0,4,76,73,83,84,0,0,0,0,13,6,10,0,
15,7,2,0,11,8,0,0,0,0,0,0,0,0,0,0,
11,9,0,0,0,0,0,0,0,0,0,0,31,4,6,4,
19,4,5,4,30,9,2,35,32,32,32,32,107,101,121,32,
61,32,84,111,107,101,110,40,116,46,112,111,115,44,39,103,
101,116,39,44,78,111,110,101,44,91,0,0,12,7,0,5,
84,111,107,101,110,0,0,0,13,6,7,0,12,11,0,3,
112,111,115,0,9,7,1,11,12,8,0,3,103,101,116,0,
28,9,0,0,30,9,2,36,32,32,32,32,32,32,32,32,
32,32,32,32,84,111,107,101,110,40,116,46,112,111,115,44,
39,114,101,103,39,44,114,41,44,0,0,0,12,14,0,5,
84,111,107,101,110,0,0,0,13,13,14,0,12,17,0,3,
112,111,115,0,9,14,1,17,12,15,0,3,114,101,103,0,
15,16,2,0,31,11,14,3,19,11,13,11,30,11,2,37,
32,32,32,32,32,32,32,32,32,32,32,32,84,111,107,101,
110,40,116,46,112,111,115,44,39,115,121,109,98,111,108,39,
44,39,78,111,110,101,39,41,93,41,0,0,12,14,0,5,
84,111,107,101,110,0,0,0,13,13,14,0,12,17,0,3,
112,111,115,0,9,14,1,17,12,15,0,6,115,121,109,98,
111,108,0,0,12,16,0,4,78,111,110,101,0,0,0,0,
31,12,14,3,19,12,13,12,27,10,11,2,31,5,7,4,
19,5,6,5,15,4,5,0,30,13,2,38,32,32,32,32,
97,112,32,61,32,84,111,107,101,110,40,116,46,112,111,115,
44,39,115,121,109,98,111,108,39,44,39,61,39,44,91,107,
101,121,44,116,46,105,116,101,109,115,91,48,93,93,41,0,
12,8,0,5,84,111,107,101,110,0,0,0,13,7,8,0,
12,12,0,3,112,111,115,0,9,8,1,12,12,9,0,6,
115,121,109,98,111,108,0,0,12,10,0,1,61,0,0,0,
15,12,4,0,12,14,0,5,105,116,101,109,115,0,0,0,
9,13,1,14,11,14,0,0,0,0,0,0,0,0,0,0,
9,13,13,14,27,11,12,2,31,6,8,4,19,6,7,6,
15,5,6,0,30,15,2,39,32,32,32,32,100,111,40,84,
111,107,101,110,40,116,46,112,111,115,44,39,102,111,114,39,
44,78,111,110,101,44,91,116,46,105,116,101,109,115,91,49,
93,44,116,46,105,116,101,109,115,91,50,93,44,97,112,93,
41,41,0,0,12,8,0,2,100,111,0,0,13,7,8,0,
12,10,0,5,84,111,107,101,110,0,0,0,13,9,10,0,
12,14,0,3,112,111,115,0,9,10,1,14,12,11,0,3,
102,111,114,0,28,12,0,0,12,17,0,5,105,116,101,109,
115,0,0,0,9,14,1,17,11,17,0,0,0,0,0,0,
0,0,240,63,9,14,14,17,12,17,0,5,105,116,101,109,
115,0,0,0,9,15,1,17,11,17,0,0,0,0,0,0,
0,0,0,64,9,15,15,17,15,16,5,0,27,13,14,3,
31,8,10,4,19,8,9,8,31,6,8,1,19,6,7,6,
30,4,2,40,32,32,32,32,114,101,116,117,114,110,32,114,
0,0,0,0,20,2,0,0,0,0,0,0,12,53,0,7,
100,111,95,99,111,109,112,0,14,53,52,0,30,4,2,42,
100,101,102,32,100,111,95,105,102,40,116,41,58,0,0,0,
16,53,1,52,44,13,0,0,30,4,2,42,100,101,102,32,
100,111,95,105,102,40,116,41,58,0,0,0,12,1,0,9,
101,110,99,111,100,101,46,112,121,0,0,0,33,1,0,0,
12,1,0,5,100,111,95,105,102,0,0,0,34,1,0,0,
28,2,0,0,9,1,0,2,30,5,2,43,32,32,32,32,
105,116,101,109,115,32,61,32,116,46,105,116,101,109,115,0,
12,4,0,5,105,116,101,109,115,0,0,0,9,3,1,4,
15,2,3,0,30,5,2,44,32,32,32,32,116,32,61,32,
103,101,116,95,116,97,103,40,41,0,0,0,12,5,0,7,
103,101,116,95,116,97,103,0,13,4,5,0,31,3,0,0,
19,3,4,3,15,1,3,0,30,3,2,45,32,32,32,32,
110,32,61,32,48,0,0,0,11,4,0,0,0,0,0,0,
0,0,0,0,15,3,4,0,30,6,2,46,32,32,32,32,
102,111,114,32,116,116,32,105,110,32,105,116,101,109,115,58,
0,0,0,0,11,5,0,0,0,0,0,0,0,0,0,0,
42,4,2,5,18,0,0,220,30,5,2,47,32,32,32,32,
32,32,32,32,116,97,103,40,116,44,110,41,0,0,0,0,
12,8,0,3,116,97,103,0,13,7,8,0,15,8,1,0,
15,9,3,0,31,6,8,2,19,6,7,6,30,8,2,48,
32,32,32,32,32,32,32,32,105,102,32,116,116,46,116,121,
112,101,32,61,61,32,39,101,108,105,102,39,58,0,0,0,
12,7,0,4,116,121,112,101,0,0,0,0,9,6,4,7,
12,7,0,4,101,108,105,102,0,0,0,0,23,6,6,7,
21,6,0,0,18,0,0,100,30,15,2,49,32,32,32,32,
32,32,32,32,32,32,32,32,97,32,61,32,100,111,40,116,
116,46,105,116,101,109,115,91,48,93,41,59,32,99,111,100,
101,40,73,70,44,97,41,59,32,102,114,101,101,95,116,109,
112,40,97,41,59,0,0,0,12,9,0,2,100,111,0,0,
13,8,9,0,12,10,0,5,105,116,101,109,115,0,0,0,
9,9,4,10,11,10,0,0,0,0,0,0,0,0,0,0,
9,9,9,10,31,7,9,1,19,7,8,7,15,6,7,0,
12,9,0,4,99,111,100,101,0,0,0,0,13,8,9,0,
12,11,0,2,73,70,0,0,13,9,11,0,15,10,6,0,
31,7,9,2,19,7,8,7,12,9,0,8,102,114,101,101,
95,116,109,112,0,0,0,0,13,8,9,0,15,9,6,0,
31,7,9,1,19,7,8,7,30,6,2,50,32,32,32,32,
32,32,32,32,32,32,32,32,106,117,109,112,40,116,44,110,
43,49,41,0,12,9,0,4,106,117,109,112,0,0,0,0,
13,8,9,0,15,9,1,0,11,11,0,0,0,0,0,0,
0,0,240,63,1,10,3,11,31,7,9,2,19,7,8,7,
30,11,2,51,32,32,32,32,32,32,32,32,32,32,32,32,
102,114,101,101,95,116,109,112,40,100,111,40,116,116,46,105,
116,101,109,115,91,49,93,41,41,32,35,82,69,71,0,0,
12,9,0,8,102,114,101,101,95,116,109,112,0,0,0,0,
13,8,9,0,12,11,0,2,100,111,0,0,13,10,11,0,
12,12,0,5,105,116,101,109,115,0,0,0,9,11,4,12,
11,12,0,0,0,0,0,0,0,0,240,63,9,11,11,12,
31,9,11,1,19,9,10,9,31,7,9,1,19,7,8,7,
18,0,0,62,30,8,2,52,32,32,32,32,32,32,32,32,
101,108,105,102,32,116,116,46,116,121,112,101,32,61,61,32,
39,101,108,115,101,39,58,0,12,8,0,4,116,121,112,101,
0,0,0,0,9,7,4,8,12,8,0,4,101,108,115,101,
0,0,0,0,23,7,7,8,21,7,0,0,18,0,0,34,
30,11,2,53,32,32,32,32,32,32,32,32,32,32,32,32,
102,114,101,101,95,116,109,112,40,100,111,40,116,116,46,105,
116,101,109,115,91,48,93,41,41,32,35,82,69,71,0,0,
12,9,0,8,102,114,101,101,95,116,109,112,0,0,0,0,
13,8,9,0,12,11,0,2,100,111,0,0,13,10,11,0,
12,12,0,5,105,116,101,109,115,0,0,0,9,11,4,12,
11,12,0,0,0,0,0,0,0,0,0,0,9,11,11,12,
31,9,11,1,19,9,10,9,31,7,9,1,19,7,8,7,
18,0,0,10,30,5,2,55,32,32,32,32,32,32,32,32,
32,32,32,32,114,97,105,115,101,0,0,0,28,7,0,0,
37,7,0,0,18,0,0,1,30,6,2,56,32,32,32,32,
32,32,32,32,106,117,109,112,40,116,44,39,101,110,100,39,
41,0,0,0,12,9,0,4,106,117,109,112,0,0,0,0,
13,8,9,0,15,9,1,0,12,10,0,3,101,110,100,0,
31,7,9,2,19,7,8,7,30,4,2,57,32,32,32,32,
32,32,32,32,110,32,43,61,32,49,0,0,11,8,0,0,
0,0,0,0,0,0,240,63,1,7,3,8,15,3,7,0,
18,0,255,36,30,4,2,58,32,32,32,32,116,97,103,40,
116,44,110,41,0,0,0,0,12,8,0,3,116,97,103,0,
13,7,8,0,15,8,1,0,15,9,3,0,31,5,8,2,
19,5,7,5,30,5,2,59,32,32,32,32,116,97,103,40,
116,44,39,101,110,100,39,41,0,0,0,0,12,8,0,3,
116,97,103,0,13,7,8,0,15,8,1,0,12,9,0,3,
101,110,100,0,31,5,8,2,19,5,7,5,0,0,0,0,
12,54,0,5,100,111,95,105,102,0,0,0,14,54,53,0,
30,4,2,61,100,101,102,32,100,111,95,116,114,121,40,116,
41,58,0,0,16,54,0,184,44,9,0,0,30,4,2,61,
100,101,102,32,100,111,95,116,114,121,40,116,41,58,0,0,
12,1,0,9,101,110,99,111,100,101,46,112,121,0,0,0,
33,1,0,0,12,1,0,6,100,111,95,116,114,121,0,0,
34,1,0,0,28,2,0,0,9,1,0,2,30,5,2,62,
32,32,32,32,105,116,101,109,115,32,61,32,116,46,105,116,
101,109,115,0,12,4,0,5,105,116,101,109,115,0,0,0,
9,3,1,4,15,2,3,0,30,5,2,63,32,32,32,32,
116,32,61,32,103,101,116,95,116,97,103,40,41,0,0,0,
12,5,0,7,103,101,116,95,116,97,103,0,13,4,5,0,
31,3,0,0,19,3,4,3,15,1,3,0,30,6,2,64,
32,32,32,32,115,101,116,106,109,112,40,116,44,39,101,120,
99,101,112,116,39,41,0,0,12,5,0,6,115,101,116,106,
109,112,0,0,13,4,5,0,15,5,1,0,12,6,0,6,
101,120,99,101,112,116,0,0,31,3,5,2,19,3,4,3,
30,8,2,65,32,32,32,32,102,114,101,101,95,116,109,112,
40,100,111,40,105,116,101,109,115,91,48,93,41,41,32,35,
82,69,71,0,12,5,0,8,102,114,101,101,95,116,109,112,
0,0,0,0,13,4,5,0,12,7,0,2,100,111,0,0,
13,6,7,0,11,8,0,0,0,0,0,0,0,0,0,0,
9,7,2,8,31,5,7,1,19,5,6,5,31,3,5,1,
19,3,4,3,30,5,2,66,32,32,32,32,99,111,100,101,
40,83,69,84,74,77,80,44,48,41,0,0,12,5,0,4,
99,111,100,101,0,0,0,0,13,4,5,0,12,7,0,6,
83,69,84,74,77,80,0,0,13,5,7,0,11,6,0,0,
0,0,0,0,0,0,0,0,31,3,5,2,19,3,4,3,
30,5,2,67,32,32,32,32,106,117,109,112,40,116,44,39,
101,110,100,39,41,0,0,0,12,5,0,4,106,117,109,112,
0,0,0,0,13,4,5,0,15,5,1,0,12,6,0,3,
101,110,100,0,31,3,5,2,19,3,4,3,30,5,2,68,
32,32,32,32,116,97,103,40,116,44,39,101,120,99,101,112,
116,39,41,0,12,5,0,3,116,97,103,0,13,4,5,0,
15,5,1,0,12,6,0,6,101,120,99,101,112,116,0,0,
31,3,5,2,19,3,4,3,30,11,2,69,32,32,32,32,
102,114,101,101,95,116,109,112,40,100,111,40,105,116,101,109,
115,91,49,93,46,105,116,101,109,115,91,49,93,41,41,32,
35,82,69,71,0,0,0,0,12,5,0,8,102,114,101,101,
95,116,109,112,0,0,0,0,13,4,5,0,12,7,0,2,
100,111,0,0,13,6,7,0,11,8,0,0,0,0,0,0,
0,0,240,63,9,7,2,8,12,8,0,5,105,116,101,109,
115,0,0,0,9,7,7,8,11,8,0,0,0,0,0,0,
0,0,240,63,9,7,7,8,31,5,7,1,19,5,6,5,
31,3,5,1,19,3,4,3,30,5,2,70,32,32,32,32,
116,97,103,40,116,44,39,101,110,100,39,41,0,0,0,0,
12,5,0,3,116,97,103,0,13,4,5,0,15,5,1,0,
12,6,0,3,101,110,100,0,31,3,5,2,19,3,4,3,
0,0,0,0,12,55,0,6,100,111,95,116,114,121,0,0,
14,55,54,0,30,5,2,72,100,101,102,32,100,111,95,114,
101,116,117,114,110,40,116,41,58,0,0,0,16,55,0,105,
44,8,0,0,30,5,2,72,100,101,102,32,100,111,95,114,
101,116,117,114,110,40,116,41,58,0,0,0,12,1,0,9,
101,110,99,111,100,101,46,112,121,0,0,0,33,1,0,0,
12,1,0,9,100,111,95,114,101,116,117,114,110,0,0,0,
34,1,0,0,28,2,0,0,9,1,0,2,30,9,2,73,
32,32,32,32,105,102,32,116,46,105,116,101,109,115,58,32,
114,32,61,32,100,111,40,116,46,105,116,101,109,115,91,48,
93,41,0,0,12,3,0,5,105,116,101,109,115,0,0,0,
9,2,1,3,21,2,0,0,18,0,0,16,12,5,0,2,
100,111,0,0,13,4,5,0,12,6,0,5,105,116,101,109,
115,0,0,0,9,5,1,6,11,6,0,0,0,0,0,0,
0,0,0,0,9,5,5,6,31,3,5,1,19,3,4,3,
15,2,3,0,18,0,0,18,30,7,2,74,32,32,32,32,
101,108,115,101,58,32,114,32,61,32,95,100,111,95,110,111,
110,101,40,41,0,0,0,0,12,5,0,8,95,100,111,95,
110,111,110,101,0,0,0,0,13,4,5,0,31,3,0,0,
19,3,4,3,15,2,3,0,18,0,0,1,30,5,2,75,
32,32,32,32,99,111,100,101,40,82,69,84,85,82,78,44,
114,41,0,0,12,5,0,4,99,111,100,101,0,0,0,0,
13,4,5,0,12,7,0,6,82,69,84,85,82,78,0,0,
13,5,7,0,15,6,2,0,31,3,5,2,19,3,4,3,
30,4,2,76,32,32,32,32,102,114,101,101,95,116,109,112,
40,114,41,0,12,5,0,8,102,114,101,101,95,116,109,112,
0,0,0,0,13,4,5,0,15,5,2,0,31,3,5,1,
19,3,4,3,30,3,2,77,32,32,32,32,114,101,116,117,
114,110,0,0,28,3,0,0,20,3,0,0,0,0,0,0,
12,56,0,9,100,111,95,114,101,116,117,114,110,0,0,0,
14,56,55,0,30,5,2,78,100,101,102,32,100,111,95,114,
97,105,115,101,40,116,41,58,0,0,0,0,16,56,0,105,
44,8,0,0,30,5,2,78,100,101,102,32,100,111,95,114,
97,105,115,101,40,116,41,58,0,0,0,0,12,1,0,9,
101,110,99,111,100,101,46,112,121,0,0,0,33,1,0,0,
12,1,0,8,100,111,95,114,97,105,115,101,0,0,0,0,
34,1,0,0,28,2,0,0,9,1,0,2,30,9,2,79,
32,32,32,32,105,102,32,116,46,105,116,101,109,115,58,32,
114,32,61,32,100,111,40,116,46,105,116,101,109,115,91,48,
93,41,0,0,12,3,0,5,105,116,101,109,115,0,0,0,
9,2,1,3,21,2,0,0,18,0,0,16,12,5,0,2,
100,111,0,0,13,4,5,0,12,6,0,5,105,116,101,109,
115,0,0,0,9,5,1,6,11,6,0,0,0,0,0,0,
0,0,0,0,9,5,5,6,31,3,5,1,19,3,4,3,
15,2,3,0,18,0,0,18,30,7,2,80,32,32,32,32,
101,108,115,101,58,32,114,32,61,32,95,100,111,95,110,111,
110,101,40,41,0,0,0,0,12,5,0,8,95,100,111,95,
110,111,110,101,0,0,0,0,13,4,5,0,31,3,0,0,
19,3,4,3,15,2,3,0,18,0,0,1,30,5,2,81,
32,32,32,32,99,111,100,101,40,82,65,73,83,69,44,114,
41,0,0,0,12,5,0,4,99,111,100,101,0,0,0,0,
13,4,5,0,12,7,0,5,82,65,73,83,69,0,0,0,
13,5,7,0,15,6,2,0,31,3,5,2,19,3,4,3,
30,4,2,82,32,32,32,32,102,114,101,101,95,116,109,112,
40,114,41,0,12,5,0,8,102,114,101,101,95,116,109,112,
0,0,0,0,13,4,5,0,15,5,2,0,31,3,5,1,
19,3,4,3,30,3,2,83,32,32,32,32,114,101,116,117,
114,110,0,0,28,3,0,0,20,3,0,0,0,0,0,0,
12,57,0,8,100,111,95,114,97,105,115,101,0,0,0,0,
14,57,56,0,30,6,2,85,100,101,102,32,100,111,95,115,
116,97,116,101,109,101,110,116,115,40,116,41,58,0,0,0,
16,57,0,57,44,10,0,0,30,6,2,85,100,101,102,32,
100,111,95,115,116,97,116,101,109,101,110,116,115,40,116,41,
58,0,0,0,12,1,0,9,101,110,99,111,100,101,46,112,
121,0,0,0,33,1,0,0,12,1,0,13,100,111,95,115,
116,97,116,101,109,101,110,116,115,0,0,0,34,1,0,0,
28,2,0,0,9,1,0,2,30,10,2,86,32,32,32,32,
102,111,114,32,116,116,32,105,110,32,116,46,105,116,101,109,
115,58,32,102,114,101,101,95,116,109,112,40,100,111,40,116,
116,41,41,0,12,4,0,5,105,116,101,109,115,0,0,0,
9,3,1,4,11,4,0,0,0,0,0,0,0,0,0,0,
42,2,3,4,18,0,0,15,12,7,0,8,102,114,101,101,
95,116,109,112,0,0,0,0,13,6,7,0,12,9,0,2,
100,111,0,0,13,8,9,0,15,9,2,0,31,7,9,1,
19,7,8,7,31,5,7,1,19,5,6,5,18,0,255,241,
0,0,0,0,12,58,0,13,100,111,95,115,116,97,116,101,
109,101,110,116,115,0,0,0,14,58,57,0,30,6,2,88,
100,101,102,32,100,111,95,108,105,115,116,40,116,44,114,61,
78,111,110,101,41,58,0,0,16,58,0,69,44,9,0,0,
30,6,2,88,100,101,102,32,100,111,95,108,105,115,116,40,
116,44,114,61,78,111,110,101,41,58,0,0,12,1,0,9,
101,110,99,111,100,101,46,112,121,0,0,0,33,1,0,0,
12,1,0,7,100,111,95,108,105,115,116,0,34,1,0,0,
28,2,0,0,9,1,0,2,28,2,0,0,28,3,0,0,
32,2,0,3,30,5,2,89,32,32,32,32,114,32,61,32,
103,101,116,95,116,109,112,40,114,41,0,0,12,5,0,7,
103,101,116,95,116,109,112,0,13,4,5,0,15,5,2,0,
31,3,5,1,19,3,4,3,15,2,3,0,30,8,2,90,
32,32,32,32,109,97,110,97,103,101,95,115,101,113,40,76,
73,83,84,44,114,44,116,46,105,116,101,109,115,41,0,0,
12,5,0,10,109,97,110,97,103,101,95,115,101,113,0,0,
13,4,5,0,12,8,0,4,76,73,83,84,0,0,0,0,
13,5,8,0,15,6,2,0,12,8,0,5,105,116,101,109,
115,0,0,0,9,7,1,8,31,3,5,3,19,3,4,3,
30,4,2,91,32,32,32,32,114,101,116,117,114,110,32,114,
0,0,0,0,20,2,0,0,0,0,0,0,12,59,0,7,
100,111,95,108,105,115,116,0,14,59,58,0,30,6,2,93,
100,101,102,32,100,111,95,100,105,99,116,40,116,44,114,61,
78,111,110,101,41,58,0,0,16,59,0,69,44,9,0,0,
30,6,2,93,100,101,102,32,100,111,95,100,105,99,116,40,
116,44,114,61,78,111,110,101,41,58,0,0,12,1,0,9,
101,110,99,111,100,101,46,112,121,0,0,0,33,1,0,0,
12,1,0,7,100,111,95,100,105,99,116,0,34,1,0,0,
28,2,0,0,9,1,0,2,28,2,0,0,28,3,0,0,
32,2,0,3,30,5,2,94,32,32,32,32,114,32,61,32,
103,101,116,95,116,109,112,40,114,41,0,0,12,5,0,7,
103,101,116,95,116,109,112,0,13,4,5,0,15,5,2,0,
31,3,5,1,19,3,4,3,15,2,3,0,30,8,2,95,
32,32,32,32,109,97,110,97,103,101,95,115,101,113,40,68,
73,67,84,44,114,44,116,46,105,116,101,109,115,41,0,0,
12,5,0,10,109,97,110,97,103,101,95,115,101,113,0,0,
13,4,5,0,12,8,0,4,68,73,67,84,0,0,0,0,
13,5,8,0,15,6,2,0,12,8,0,5,105,116,101,109,
115,0,0,0,9,7,1,8,31,3,5,3,19,3,4,3,
30,4,2,96,32,32,32,32,114,101,116,117,114,110,32,114,
0,0,0,0,20,2,0,0,0,0,0,0,12,60,0,7,
100,111,95,100,105,99,116,0,14,60,59,0,30,6,2,98,
100,101,102,32,100,111,95,103,101,116,40,116,44,114,61,78,
111,110,101,41,58,0,0,0,16,60,0,66,44,11,0,0,
30,6,2,98,100,101,102,32,100,111,95,103,101,116,40,116,
44,114,61,78,111,110,101,41,58,0,0,0,12,1,0,9,
101,110,99,111,100,101,46,112,121,0,0,0,33,1,0,0,
12,1,0,6,100,111,95,103,101,116,0,0,34,1,0,0,
28,2,0,0,9,1,0,2,28,2,0,0,28,3,0,0,
32,2,0,3,30,5,2,99,32,32,32,32,105,116,101,109,
115,32,61,32,116,46,105,116,101,109,115,0,12,5,0,5,
105,116,101,109,115,0,0,0,9,4,1,5,15,3,4,0,
30,11,2,100,32,32,32,32,114,101,116,117,114,110,32,105,
110,102,105,120,40,71,69,84,44,105,116,101,109,115,91,48,
93,44,105,116,101,109,115,91,49,93,44,114,41,0,0,0,
12,6,0,5,105,110,102,105,120,0,0,0,13,5,6,0,
12,10,0,3,71,69,84,0,13,6,10,0,11,10,0,0,
0,0,0,0,0,0,0,0,9,7,3,10,11,10,0,0,
0,0,0,0,0,0,240,63,9,8,3,10,15,9,2,0,
31,4,6,4,19,4,5,4,20,4,0,0,0,0,0,0,
12,61,0,6,100,111,95,103,101,116,0,0,14,61,60,0,
30,11,2,102,100,101,102,32,100,111,95,98,114,101,97,107,
40,116,41,58,32,106,117,109,112,40,68,46,116,115,116,97,
99,107,91,45,49,93,44,39,98,114,101,97,107,39,41,0,
16,61,0,47,44,7,0,0,30,11,2,102,100,101,102,32,
100,111,95,98,114,101,97,107,40,116,41,58,32,106,117,109,
112,40,68,46,116,115,116,97,99,107,91,45,49,93,44,39,
98,114,101,97,107,39,41,0,12,1,0,9,101,110,99,111,
100,101,46,112,121,0,0,0,33,1,0,0,12,1,0,8,
100,111,95,98,114,101,97,107,0,0,0,0,34,1,0,0,
28,2,0,0,9,1,0,2,12,4,0,4,106,117,109,112,
0,0,0,0,13,3,4,0,12,6,0,1,68,0,0,0,
13,4,6,0,12,6,0,6,116,115,116,97,99,107,0,0,
9,4,4,6,11,6,0,0,0,0,0,0,0,0,240,191,
9,4,4,6,12,5,0,5,98,114,101,97,107,0,0,0,
31,2,4,2,19,2,3,2,0,0,0,0,12,62,0,8,
100,111,95,98,114,101,97,107,0,0,0,0,14,62,61,0,
30,13,2,103,100,101,102,32,100,111,95,99,111,110,116,105,
110,117,101,40,116,41,58,32,106,117,109,112,40,68,46,116,
115,116,97,99,107,91,45,49,93,44,39,99,111,110,116,105,
110,117,101,39,41,0,0,0,16,62,0,50,44,7,0,0,
30,13,2,103,100,101,102,32,100,111,95,99,111,110,116,105,
110,117,101,40,116,41,58,32,106,117,109,112,40,68,46,116,
115,116,97,99,107,91,45,49,93,44,39,99,111,110,116,105,
110,117,101,39,41,0,0,0,12,1,0,9,101,110,99,111,
100,101,46,112,121,0,0,0,33,1,0,0,12,1,0,11,
100,111,95,99,111,110,116,105,110,117,101,0,34,1,0,0,
28,2,0,0,9,1,0,2,12,4,0,4,106,117,109,112,
0,0,0,0,13,3,4,0,12,6,0,1,68,0,0,0,
13,4,6,0,12,6,0,6,116,115,116,97,99,107,0,0,
9,4,4,6,11,6,0,0,0,0,0,0,0,0,240,191,
9,4,4,6,12,5,0,8,99,111,110,116,105,110,117,101,
0,0,0,0,31,2,4,2,19,2,3,2,0,0,0,0,
12,63,0,11,100,111,95,99,111,110,116,105,110,117,101,0,
14,63,62,0,30,7,2,104,100,101,102,32,100,111,95,112,
97,115,115,40,116,41,58,32,99,111,100,101,40,80,65,83,
83,41,0,0,16,63,0,32,44,6,0,0,30,7,2,104,
100,101,102,32,100,111,95,112,97,115,115,40,116,41,58,32,
99,111,100,101,40,80,65,83,83,41,0,0,12,1,0,9,
101,110,99,111,100,101,46,112,121,0,0,0,33,1,0,0,
12,1,0,7,100,111,95,112,97,115,115,0,34,1,0,0,
28,2,0,0,9,1,0,2,12,4,0,4,99,111,100,101,
0,0,0,0,13,3,4,0,12,5,0,4,80,65,83,83,
0,0,0,0,13,4,5,0,31,2,4,1,19,2,3,2,
0,0,0,0,12,64,0,7,100,111,95,112,97,115,115,0,
14,64,63,0,30,6,2,106,100,101,102,32,100,111,95,105,
110,102,111,40,110,97,109,101,61,39,63,39,41,58,0,0,
16,64,0,126,44,11,0,0,30,6,2,106,100,101,102,32,
100,111,95,105,110,102,111,40,110,97,109,101,61,39,63,39,
41,58,0,0,12,1,0,9,101,110,99,111,100,101,46,112,
121,0,0,0,33,1,0,0,12,1,0,7,100,111,95,105,
110,102,111,0,34,1,0,0,12,1,0,1,63,0,0,0,
28,2,0,0,32,1,0,2,30,8,2,107,32,32,32,32,
105,102,32,39,45,110,111,112,111,115,39,32,105,110,32,65,
82,71,86,58,32,114,101,116,117,114,110,0,12,3,0,4,
65,82,71,86,0,0,0,0,13,2,3,0,12,3,0,6,
45,110,111,112,111,115,0,0,36,2,2,3,21,2,0,0,
18,0,0,4,28,2,0,0,20,2,0,0,18,0,0,1,
30,12,2,108,32,32,32,32,99,111,100,101,40,70,73,76,
69,44,102,114,101,101,95,116,109,112,40,95,100,111,95,115,
116,114,105,110,103,40,68,46,102,110,97,109,101,41,41,41,
0,0,0,0,12,4,0,4,99,111,100,101,0,0,0,0,
13,3,4,0,12,6,0,4,70,73,76,69,0,0,0,0,
13,4,6,0,12,7,0,8,102,114,101,101,95,116,109,112,
0,0,0,0,13,6,7,0,12,9,0,10,95,100,111,95,
115,116,114,105,110,103,0,0,13,8,9,0,12,10,0,1,
68,0,0,0,13,9,10,0,12,10,0,5,102,110,97,109,
101,0,0,0,9,9,9,10,31,7,9,1,19,7,8,7,
31,5,7,1,19,5,6,5,31,2,4,2,19,2,3,2,
30,11,2,109,32,32,32,32,99,111,100,101,40,78,65,77,
69,44,102,114,101,101,95,116,109,112,40,95,100,111,95,115,
116,114,105,110,103,40,110,97,109,101,41,41,41,0,0,0,
12,4,0,4,99,111,100,101,0,0,0,0,13,3,4,0,
12,6,0,4,78,65,77,69,0,0,0,0,13,4,6,0,
12,7,0,8,102,114,101,101,95,116,109,112,0,0,0,0,
13,6,7,0,12,9,0,10,95,100,111,95,115,116,114,105,
110,103,0,0,13,8,9,0,15,9,1,0,31,7,9,1,
19,7,8,7,31,5,7,1,19,5,6,5,31,2,4,2,
19,2,3,2,0,0,0,0,12,65,0,7,100,111,95,105,
110,102,111,0,14,65,64,0,30,5,2,110,100,101,102,32,
100,111,95,109,111,100,117,108,101,40,116,41,58,0,0,0,
16,65,0,62,44,8,0,0,30,5,2,110,100,101,102,32,
100,111,95,109,111,100,117,108,101,40,116,41,58,0,0,0,
12,1,0,9,101,110,99,111,100,101,46,112,121,0,0,0,
33,1,0,0,12,1,0,9,100,111,95,109,111,100,117,108,
101,0,0,0,34,1,0,0,28,2,0,0,9,1,0,2,
30,4,2,111,32,32,32,32,100,111,95,105,110,102,111,40,
41,0,0,0,12,4,0,7,100,111,95,105,110,102,111,0,
13,3,4,0,31,2,0,0,19,2,3,2,30,9,2,112,
32,32,32,32,102,114,101,101,95,116,109,112,40,100,111,40,
116,46,105,116,101,109,115,91,48,93,41,41,32,35,82,69,
71,0,0,0,12,4,0,8,102,114,101,101,95,116,109,112,
0,0,0,0,13,3,4,0,12,6,0,2,100,111,0,0,
13,5,6,0,12,7,0,5,105,116,101,109,115,0,0,0,
9,6,1,7,11,7,0,0,0,0,0,0,0,0,0,0,
9,6,6,7,31,4,6,1,19,4,5,4,31,2,4,1,
19,2,3,2,0,0,0,0,12,66,0,9,100,111,95,109,
111,100,117,108,101,0,0,0,14,66,65,0,30,9,2,113,
100,101,102,32,100,111,95,114,101,103,40,116,44,114,61,78,
111,110,101,41,58,32,114,101,116,117,114,110,32,116,46,118,
97,108,0,0,16,66,0,31,44,5,0,0,30,9,2,113,
100,101,102,32,100,111,95,114,101,103,40,116,44,114,61,78,
111,110,101,41,58,32,114,101,116,117,114,110,32,116,46,118,
97,108,0,0,12,1,0,9,101,110,99,111,100,101,46,112,
121,0,0,0,33,1,0,0,12,1,0,6,100,111,95,114,
101,103,0,0,34,1,0,0,28,2,0,0,9,1,0,2,
28,2,0,0,28,3,0,0,32,2,0,3,12,4,0,3,
118,97,108,0,9,3,1,4,20,3,0,0,0,0,0,0,
12,67,0,6,100,111,95,114,101,103,0,0,14,67,66,0,
30,3,2,115,102,109,97,112,32,61,32,123,0,0,0,0,
12,67,0,4,102,109,97,112,0,0,0,0,30,16,2,116,
32,32,32,32,39,109,111,100,117,108,101,39,58,100,111,95,
109,111,100,117,108,101,44,39,115,116,97,116,101,109,101,110,
116,115,39,58,100,111,95,115,116,97,116,101,109,101,110,116,
115,44,39,100,101,102,39,58,100,111,95,100,101,102,44,0,
12,69,0,6,109,111,100,117,108,101,0,0,12,103,0,9,
100,111,95,109,111,100,117,108,101,0,0,0,13,70,103,0,
12,71,0,10,115,116,97,116,101,109,101,110,116,115,0,0,
12,103,0,13,100,111,95,115,116,97,116,101,109,101,110,116,
115,0,0,0,13,72,103,0,12,73,0,3,100,101,102,0,
12,103,0,6,100,111,95,100,101,102,0,0,13,74,103,0,
30,13,2,117,32,32,32,32,39,114,101,116,117,114,110,39,
58,100,111,95,114,101,116,117,114,110,44,39,119,104,105,108,
101,39,58,100,111,95,119,104,105,108,101,44,39,105,102,39,
58,100,111,95,105,102,44,0,12,75,0,6,114,101,116,117,
114,110,0,0,12,103,0,9,100,111,95,114,101,116,117,114,
110,0,0,0,13,76,103,0,12,77,0,5,119,104,105,108,
101,0,0,0,12,103,0,8,100,111,95,119,104,105,108,101,
0,0,0,0,13,78,103,0,12,79,0,2,105,102,0,0,
12,103,0,5,100,111,95,105,102,0,0,0,13,80,103,0,
30,19,2,118,32,32,32,32,39,98,114,101,97,107,39,58,
100,111,95,98,114,101,97,107,44,39,112,97,115,115,39,58,
100,111,95,112,97,115,115,44,39,99,111,110,116,105,110,117,
101,39,58,100,111,95,99,111,110,116,105,110,117,101,44,39,
102,111,114,39,58,100,111,95,102,111,114,44,0,0,0,0,
12,81,0,5,98,114,101,97,107,0,0,0,12,103,0,8,
100,111,95,98,114,101,97,107,0,0,0,0,13,82,103,0,
12,83,0,4,112,97,115,115,0,0,0,0,12,103,0,7,
100,111,95,112,97,115,115,0,13,84,103,0,12,85,0,8,
99,111,110,116,105,110,117,101,0,0,0,0,12,103,0,11,
100,111,95,99,111,110,116,105,110,117,101,0,13,86,103,0,
12,87,0,3,102,111,114,0,12,103,0,6,100,111,95,102,
111,114,0,0,13,88,103,0,30,18,2,119,32,32,32,32,
39,99,108,97,115,115,39,58,100,111,95,99,108,97,115,115,
44,39,114,97,105,115,101,39,58,100,111,95,114,97,105,115,
101,44,39,116,114,121,39,58,100,111,95,116,114,121,44,39,
105,109,112,111,114,116,39,58,100,111,95,105,109,112,111,114,
116,44,0,0,12,89,0,5,99,108,97,115,115,0,0,0,
12,103,0,8,100,111,95,99,108,97,115,115,0,0,0,0,
13,90,103,0,12,91,0,5,114,97,105,115,101,0,0,0,
12,103,0,8,100,111,95,114,97,105,115,101,0,0,0,0,
13,92,103,0,12,93,0,3,116,114,121,0,12,103,0,6,
100,111,95,116,114,121,0,0,13,94,103,0,12,95,0,6,
105,109,112,111,114,116,0,0,12,103,0,9,100,111,95,105,
109,112,111,114,116,0,0,0,13,96,103,0,30,14,2,120,
32,32,32,32,39,103,108,111,98,97,108,115,39,58,100,111,
95,103,108,111,98,97,108,115,44,39,100,101,108,39,58,100,
111,95,100,101,108,44,39,102,114,111,109,39,58,100,111,95,
102,114,111,109,44,0,0,0,12,97,0,7,103,108,111,98,
97,108,115,0,12,103,0,10,100,111,95,103,108,111,98,97,
108,115,0,0,13,98,103,0,12,99,0,3,100,101,108,0,
12,103,0,6,100,111,95,100,101,108,0,0,13,100,103,0,
12,101,0,4,102,114,111,109,0,0,0,0,12,103,0,7,
100,111,95,102,114,111,109,0,13,102,103,0,26,68,69,34,
14,67,68,0,30,3,2,122,114,109,97,112,32,61,32,123,
0,0,0,0,12,67,0,4,114,109,97,112,0,0,0,0,
30,18,2,123,32,32,32,32,39,108,105,115,116,39,58,100,
111,95,108,105,115,116,44,32,39,116,117,112,108,101,39,58,
100,111,95,108,105,115,116,44,32,39,100,105,99,116,39,58,
100,111,95,100,105,99,116,44,32,39,115,108,105,99,101,39,
58,100,111,95,108,105,115,116,44,0,0,0,12,69,0,4,
108,105,115,116,0,0,0,0,12,93,0,7,100,111,95,108,
105,115,116,0,13,70,93,0,12,71,0,5,116,117,112,108,
101,0,0,0,12,93,0,7,100,111,95,108,105,115,116,0,
13,72,93,0,12,73,0,4,100,105,99,116,0,0,0,0,
12,93,0,7,100,111,95,100,105,99,116,0,13,74,93,0,
12,75,0,5,115,108,105,99,101,0,0,0,12,93,0,7,
100,111,95,108,105,115,116,0,13,76,93,0,30,19,2,124,
32,32,32,32,39,99,111,109,112,39,58,100,111,95,99,111,
109,112,44,32,39,110,97,109,101,39,58,100,111,95,110,97,
109,101,44,39,115,121,109,98,111,108,39,58,100,111,95,115,
121,109,98,111,108,44,39,110,117,109,98,101,114,39,58,100,
111,95,110,117,109,98,101,114,44,0,0,0,12,77,0,4,
99,111,109,112,0,0,0,0,12,93,0,7,100,111,95,99,
111,109,112,0,13,78,93,0,12,79,0,4,110,97,109,101,
0,0,0,0,12,93,0,7,100,111,95,110,97,109,101,0,
13,80,93,0,12,81,0,6,115,121,109,98,111,108,0,0,
12,93,0,9,100,111,95,115,121,109,98,111,108,0,0,0,
13,82,93,0,12,83,0,6,110,117,109,98,101,114,0,0,
12,93,0,9,100,111,95,110,117,109,98,101,114,0,0,0,
13,84,93,0,30,17,2,125,32,32,32,32,39,115,116,114,
105,110,103,39,58,100,111,95,115,116,114,105,110,103,44,39,
103,101,116,39,58,100,111,95,103,101,116,44,32,39,99,97,
108,108,39,58,100,111,95,99,97,108,108,44,32,39,114,101,
103,39,58,100,111,95,114,101,103,44,0,0,12,85,0,6,
115,116,114,105,110,103,0,0,12,93,0,9,100,111,95,115,
116,114,105,110,103,0,0,0,13,86,93,0,12,87,0,3,
103,101,116,0,12,93,0,6,100,111,95,103,101,116,0,0,
13,88,93,0,12,89,0,4,99,97,108,108,0,0,0,0,
12,93,0,7,100,111,95,99,97,108,108,0,13,90,93,0,
12,91,0,3,114,101,103,0,12,93,0,6,100,111,95,114,
101,103,0,0,13,92,93,0,26,68,69,24,14,67,68,0,
30,5,2,128,100,101,102,32,100,111,40,116,44,114,61,78,
111,110,101,41,58,0,0,0,16,67,0,197,44,9,0,0,
30,5,2,128,100,101,102,32,100,111,40,116,44,114,61,78,
111,110,101,41,58,0,0,0,12,1,0,9,101,110,99,111,
100,101,46,112,121,0,0,0,33,1,0,0,12,1,0,2,
100,111,0,0,34,1,0,0,28,2,0,0,9,1,0,2,
28,2,0,0,28,3,0,0,32,2,0,3,30,7,2,129,
32,32,32,32,105,102,32,116,46,112,111,115,58,32,115,101,
116,112,111,115,40,116,46,112,111,115,41,0,12,4,0,3,
112,111,115,0,9,3,1,4,21,3,0,0,18,0,0,11,
12,5,0,6,115,101,116,112,111,115,0,0,13,4,5,0,
12,6,0,3,112,111,115,0,9,5,1,6,31,3,5,1,
19,3,4,3,18,0,0,1,30,3,2,130,32,32,32,32,
116,114,121,58,0,0,0,0,38,0,0,70,30,7,2,131,
32,32,32,32,32,32,32,32,105,102,32,116,46,116,121,112,
101,32,105,110,32,114,109,97,112,58,0,0,12,4,0,4,
114,109,97,112,0,0,0,0,13,3,4,0,12,5,0,4,
116,121,112,101,0,0,0,0,9,4,1,5,36,3,3,4,
21,3,0,0,18,0,0,27,30,10,2,132,32,32,32,32,
32,32,32,32,32,32,32,32,114,101,116,117,114,110,32,114,
109,97,112,91,116,46,116,121,112,101,93,40,116,44,114,41,
0,0,0,0,12,5,0,4,114,109,97,112,0,0,0,0,
13,4,5,0,12,6,0,4,116,121,112,101,0,0,0,0,
9,5,1,6,9,4,4,5,15,5,1,0,15,6,2,0,
31,3,5,2,19,3,4,3,20,3,0,0,18,0,0,1,
30,8,2,134,32,32,32,32,32,32,32,32,114,101,116,117,
114,110,32,102,109,97,112,91,116,46,116,121,112,101,93,40,
116,41,0,0,12,5,0,4,102,109,97,112,0,0,0,0,
13,4,5,0,12,6,0,4,116,121,112,101,0,0,0,0,
9,5,1,6,9,4,4,5,15,5,1,0,31,3,5,1,
19,3,4,3,20,3,0,0,38,0,0,0,18,0,0,79,
30,3,2,135,32,32,32,32,101,120,99,101,112,116,58,0,
30,7,2,136,32,32,32,32,32,32,32,32,105,102,32,68,
46,101,114,114,111,114,58,32,114,97,105,115,101,0,0,0,
12,4,0,1,68,0,0,0,13,3,4,0,12,4,0,5,
101,114,114,111,114,0,0,0,9,3,3,4,21,3,0,0,
18,0,0,4,28,3,0,0,37,3,0,0,18,0,0,1,
30,6,2,137,32,32,32,32,32,32,32,32,68,46,101,114,
114,111,114,32,61,32,84,114,117,101,0,0,12,4,0,1,
68,0,0,0,13,3,4,0,11,4,0,0,0,0,0,0,
0,0,240,63,12,5,0,5,101,114,114,111,114,0,0,0,
10,3,5,4,30,12,2,138,32,32,32,32,32,32,32,32,
116,111,107,101,110,105,122,101,46,117,95,101,114,114,111,114,
40,39,101,110,99,111,100,101,39,44,68,46,99,111,100,101,
44,116,46,112,111,115,41,0,12,5,0,8,116,111,107,101,
110,105,122,101,0,0,0,0,13,4,5,0,12,5,0,7,
117,95,101,114,114,111,114,0,9,4,4,5,12,5,0,6,
101,110,99,111,100,101,0,0,12,8,0,1,68,0,0,0,
13,6,8,0,12,8,0,4,99,111,100,101,0,0,0,0,
9,6,6,8,12,8,0,3,112,111,115,0,9,7,1,8,
31,3,5,3,19,3,4,3,0,0,0,0,12,68,0,2,
100,111,0,0,14,68,67,0,30,6,2,140,100,101,102,32,
101,110,99,111,100,101,40,102,110,97,109,101,44,115,44,116,
41,58,0,0,16,68,0,191,44,12,0,0,30,6,2,140,
100,101,102,32,101,110,99,111,100,101,40,102,110,97,109,101,
44,115,44,116,41,58,0,0,12,1,0,9,101,110,99,111,
100,101,46,112,121,0,0,0,33,1,0,0,12,1,0,6,
101,110,99,111,100,101,0,0,34,1,0,0,28,2,0,0,
9,1,0,2,28,3,0,0,9,2,0,3,28,4,0,0,
9,3,0,4,30,11,2,141,32,32,32,32,116,32,61,32,
84,111,107,101,110,40,40,49,44,49,41,44,39,109,111,100,
117,108,101,39,44,39,109,111,100,117,108,101,39,44,91,116,
93,41,0,0,12,6,0,5,84,111,107,101,110,0,0,0,
13,5,6,0,11,10,0,0,0,0,0,0,0,0,240,63,
11,11,0,0,0,0,0,0,0,0,240,63,27,6,10,2,
12,7,0,6,109,111,100,117,108,101,0,0,12,8,0,6,
109,111,100,117,108,101,0,0,15,10,3,0,27,9,10,1,
31,4,6,4,19,4,5,4,15,3,4,0,30,4,2,142,
32,32,32,32,103,108,111,98,97,108,32,68,0,0,0,0,
30,7,2,143,32,32,32,32,115,32,61,32,116,111,107,101,
110,105,122,101,46,99,108,101,97,110,40,115,41,0,0,0,
12,6,0,8,116,111,107,101,110,105,122,101,0,0,0,0,
13,5,6,0,12,6,0,5,99,108,101,97,110,0,0,0,
9,5,5,6,15,6,2,0,31,4,6,1,19,4,5,4,
15,2,4,0,30,6,2,144,32,32,32,32,68,32,61,32,
68,83,116,97,116,101,40,115,44,102,110,97,109,101,41,0,
12,4,0,1,68,0,0,0,12,7,0,6,68,83,116,97,
116,101,0,0,13,6,7,0,15,7,2,0,15,8,1,0,
31,5,7,2,19,5,6,5,14,4,5,0,30,5,2,145,
32,32,32,32,68,46,98,101,103,105,110,40,84,114,117,101,
41,0,0,0,12,6,0,1,68,0,0,0,13,5,6,0,
12,6,0,5,98,101,103,105,110,0,0,0,9,5,5,6,
11,6,0,0,0,0,0,0,0,0,240,63,31,4,6,1,
19,4,5,4,30,3,2,146,32,32,32,32,100,111,40,116,
41,0,0,0,12,6,0,2,100,111,0,0,13,5,6,0,
15,6,3,0,31,4,6,1,19,4,5,4,30,3,2,147,
32,32,32,32,68,46,101,110,100,40,41,0,12,6,0,1,
68,0,0,0,13,5,6,0,12,6,0,3,101,110,100,0,
9,5,5,6,31,4,0,0,19,4,5,4,30,4,2,148,
32,32,32,32,109,97,112,95,116,97,103,115,40,41,0,0,
12,6,0,8,109,97,112,95,116,97,103,115,0,0,0,0,
13,5,6,0,31,4,0,0,19,4,5,4,30,7,2,149,
32,32,32,32,111,117,116,32,61,32,68,46,111,117,116,59,
32,68,32,61,32,78,111,110,101,0,0,0,12,6,0,1,
68,0,0,0,13,5,6,0,12,6,0,3,111,117,116,0,
9,5,5,6,15,4,5,0,12,5,0,1,68,0,0,0,
28,6,0,0,14,5,6,0,30,6,2,150,32,32,32,32,
114,101,116,117,114,110,32,39,39,46,106,111,105,110,40,111,
117,116,41,0,12,6,0,0,0,0,0,0,12,7,0,4,
106,111,105,110,0,0,0,0,9,6,6,7,15,7,4,0,
31,5,7,1,19,5,6,5,20,5,0,0,0,0,0,0,
12,69,0,6,101,110,99,111,100,101,0,0,14,69,68,0,
0,0,0,0,
};
unsigned char tp_py2bc[] = {
44,11,0,0,30,3,0,1,105,109,112,111,114,116,32,115,
121,115,0,0,12,0,0,8,112,121,50,98,99,46,112,121,
0,0,0,0,33,0,0,0,12,0,0,1,63,0,0,0,
34,0,0,0,12,2,0,6,105,109,112,111,114,116,0,0,
13,1,2,0,12,2,0,3,115,121,115,0,31,0,2,1,
19,0,1,0,12,1,0,3,115,121,115,0,14,1,0,0,
30,8,0,2,105,102,32,110,111,116,32,34,116,105,110,121,
112,121,34,32,105,110,32,115,121,115,46,118,101,114,115,105,
111,110,58,0,12,2,0,3,115,121,115,0,13,1,2,0,
12,2,0,7,118,101,114,115,105,111,110,0,9,1,1,2,
12,2,0,6,116,105,110,121,112,121,0,0,36,1,1,2,
47,0,1,0,21,0,0,0,18,0,0,30,30,6,0,3,
32,32,32,32,102,114,111,109,32,98,111,111,116,32,105,109,
112,111,114,116,32,42,0,0,12,2,0,6,105,109,112,111,
114,116,0,0,13,1,2,0,12,2,0,4,98,111,111,116,
0,0,0,0,31,0,2,1,19,0,1,0,12,3,0,5,
109,101,114,103,101,0,0,0,13,2,3,0,12,5,0,8,
95,95,100,105,99,116,95,95,0,0,0,0,13,3,5,0,
15,4,0,0,31,1,3,2,19,1,2,1,18,0,0,1,
30,8,0,5,105,109,112,111,114,116,32,116,111,107,101,110,
105,122,101,44,112,97,114,115,101,44,101,110,99,111,100,101,
0,0,0,0,12,2,0,6,105,109,112,111,114,116,0,0,
13,1,2,0,12,2,0,8,116,111,107,101,110,105,122,101,
0,0,0,0,31,0,2,1,19,0,1,0,12,1,0,8,
116,111,107,101,110,105,122,101,0,0,0,0,14,1,0,0,
12,2,0,6,105,109,112,111,114,116,0,0,13,1,2,0,
12,2,0,5,112,97,114,115,101,0,0,0,31,0,2,1,
19,0,1,0,12,1,0,5,112,97,114,115,101,0,0,0,
14,1,0,0,12,2,0,6,105,109,112,111,114,116,0,0,
13,1,2,0,12,2,0,6,101,110,99,111,100,101,0,0,
31,0,2,1,19,0,1,0,12,1,0,6,101,110,99,111,
100,101,0,0,14,1,0,0,30,6,0,7,100,101,102,32,
95,99,111,109,112,105,108,101,40,115,44,102,110,97,109,101,
41,58,0,0,16,0,0,100,44,11,0,0,30,6,0,7,
100,101,102,32,95,99,111,109,112,105,108,101,40,115,44,102,
110,97,109,101,41,58,0,0,12,1,0,8,112,121,50,98,
99,46,112,121,0,0,0,0,33,1,0,0,12,1,0,8,
95,99,111,109,112,105,108,101,0,0,0,0,34,1,0,0,
28,2,0,0,9,1,0,2,28,3,0,0,9,2,0,3,
30,9,0,8,32,32,32,32,116,111,107,101,110,115,32,61,
32,116,111,107,101,110,105,122,101,46,116,111,107,101,110,105,
122,101,40,115,41,0,0,0,12,6,0,8,116,111,107,101,
110,105,122,101,0,0,0,0,13,5,6,0,12,6,0,8,
116,111,107,101,110,105,122,101,0,0,0,0,9,5,5,6,
15,6,1,0,31,4,6,1,19,4,5,4,15,3,4,0,
30,8,0,9,32,32,32,32,116,32,61,32,112,97,114,115,
101,46,112,97,114,115,101,40,115,44,116,111,107,101,110,115,
41,0,0,0,12,7,0,5,112,97,114,115,101,0,0,0,
13,6,7,0,12,7,0,5,112,97,114,115,101,0,0,0,
9,6,6,7,15,7,1,0,15,8,3,0,31,5,7,2,
19,5,6,5,15,4,5,0,30,9,0,10,32,32,32,32,
114,32,61,32,101,110,99,111,100,101,46,101,110,99,111,100,
101,40,102,110,97,109,101,44,115,44,116,41,0,0,0,0,
12,8,0,6,101,110,99,111,100,101,0,0,13,7,8,0,
12,8,0,6,101,110,99,111,100,101,0,0,9,7,7,8,
15,8,2,0,15,9,1,0,15,10,4,0,31,6,8,3,
19,6,7,6,15,5,6,0,30,4,0,11,32,32,32,32,
114,101,116,117,114,110,32,114,0,0,0,0,20,5,0,0,
0,0,0,0,12,1,0,8,95,99,111,109,112,105,108,101,
0,0,0,0,14,1,0,0,30,5,0,13,100,101,102,32,
95,105,109,112,111,114,116,40,110,97,109,101,41,58,0,0,
16,1,1,32,44,12,0,0,30,5,0,13,100,101,102,32,
95,105,109,112,111,114,116,40,110,97,109,101,41,58,0,0,
12,1,0,8,112,121,50,98,99,46,112,121,0,0,0,0,
33,1,0,0,12,1,0,7,95,105,109,112,111,114,116,0,
34,1,0,0,28,2,0,0,9,1,0,2,30,6,0,14,
32,32,32,32,105,102,32,110,97,109,101,32,105,110,32,77,
79,68,85,76,69,83,58,0,12,3,0,7,77,79,68,85,
76,69,83,0,13,2,3,0,36,2,2,1,21,2,0,0,
18,0,0,17,30,8,0,15,32,32,32,32,32,32,32,32,
114,101,116,117,114,110,32,77,79,68,85,76,69,83,91,110,
97,109,101,93,0,0,0,0,12,3,0,7,77,79,68,85,
76,69,83,0,13,2,3,0,9,2,2,1,20,2,0,0,
18,0,0,1,30,5,0,16,32,32,32,32,112,121,32,61,
32,110,97,109,101,43,34,46,112,121,34,0,12,4,0,3,
46,112,121,0,1,3,1,4,15,2,3,0,30,6,0,17,
32,32,32,32,116,112,99,32,61,32,110,97,109,101,43,34,
46,116,112,99,34,0,0,0,12,5,0,4,46,116,112,99,
0,0,0,0,1,4,1,5,15,3,4,0,30,5,0,18,
32,32,32,32,105,102,32,101,120,105,115,116,115,40,112,121,
41,58,0,0,12,6,0,6,101,120,105,115,116,115,0,0,
13,5,6,0,15,6,2,0,31,4,6,1,19,4,5,4,
21,4,0,0,18,0,0,97,30,14,0,19,32,32,32,32,
32,32,32,32,105,102,32,110,111,116,32,101,120,105,115,116,
115,40,116,112,99,41,32,111,114,32,109,116,105,109,101,40,
112,121,41,32,62,32,109,116,105,109,101,40,116,112,99,41,
58,0,0,0,12,7,0,6,101,120,105,115,116,115,0,0,
13,6,7,0,15,7,3,0,31,5,7,1,19,5,6,5,
47,4,5,0,46,4,0,0,18,0,0,16,12,6,0,5,
109,116,105,109,101,0,0,0,13,5,6,0,15,6,3,0,
31,4,6,1,19,4,5,4,12,7,0,5,109,116,105,109,
101,0,0,0,13,6,7,0,15,7,2,0,31,5,7,1,
19,5,6,5,25,4,4,5,21,4,0,0,18,0,0,54,
30,7,0,20,32,32,32,32,32,32,32,32,32,32,32,32,
115,32,61,32,108,111,97,100,40,112,121,41,0,0,0,0,
12,7,0,4,108,111,97,100,0,0,0,0,13,6,7,0,
15,7,2,0,31,5,7,1,19,5,6,5,15,4,5,0,
30,9,0,21,32,32,32,32,32,32,32,32,32,32,32,32,
99,111,100,101,32,61,32,95,99,111,109,112,105,108,101,40,
115,44,112,121,41,0,0,0,12,8,0,8,95,99,111,109,
112,105,108,101,0,0,0,0,13,7,8,0,15,8,4,0,
15,9,2,0,31,6,8,2,19,6,7,6,15,5,6,0,
30,7,0,22,32,32,32,32,32,32,32,32,32,32,32,32,
115,97,118,101,40,116,112,99,44,99,111,100,101,41,0,0,
12,8,0,4,115,97,118,101,0,0,0,0,13,7,8,0,
15,8,3,0,15,9,5,0,31,6,8,2,19,6,7,6,
18,0,0,1,18,0,0,1,30,8,0,23,32,32,32,32,
105,102,32,110,111,116,32,101,120,105,115,116,115,40,116,112,
99,41,58,32,114,97,105,115,101,0,0,0,12,9,0,6,
101,120,105,115,116,115,0,0,13,8,9,0,15,9,3,0,
31,7,9,1,19,7,8,7,47,6,7,0,21,6,0,0,
18,0,0,4,28,6,0,0,37,6,0,0,18,0,0,1,
30,6,0,24,32,32,32,32,99,111,100,101,32,61,32,108,
111,97,100,40,116,112,99,41,0,0,0,0,12,8,0,4,
108,111,97,100,0,0,0,0,13,7,8,0,15,8,3,0,
31,6,8,1,19,6,7,6,15,5,6,0,30,11,0,25,
32,32,32,32,103,32,61,32,123,39,95,95,110,97,109,101,
95,95,39,58,110,97,109,101,44,39,95,95,99,111,100,101,
95,95,39,58,99,111,100,101,125,0,0,0,12,8,0,8,
95,95,110,97,109,101,95,95,0,0,0,0,15,9,1,0,
12,10,0,8,95,95,99,111,100,101,95,95,0,0,0,0,
15,11,5,0,26,7,8,4,15,6,7,0,30,6,0,26,
32,32,32,32,103,91,39,95,95,100,105,99,116,95,95,39,
93,32,61,32,103,0,0,0,12,7,0,8,95,95,100,105,
99,116,95,95,0,0,0,0,10,6,7,6,30,6,0,27,
32,32,32,32,77,79,68,85,76,69,83,91,110,97,109,101,
93,32,61,32,103,0,0,0,12,8,0,7,77,79,68,85,
76,69,83,0,13,7,8,0,10,7,1,6,30,5,0,28,
32,32,32,32,101,120,101,99,40,99,111,100,101,44,103,41,
0,0,0,0,12,9,0,4,101,120,101,99,0,0,0,0,
13,8,9,0,15,9,5,0,15,10,6,0,31,7,9,2,
19,7,8,7,30,4,0,29,32,32,32,32,114,101,116,117,
114,110,32,103,0,0,0,0,20,6,0,0,0,0,0,0,
12,2,0,7,95,105,109,112,111,114,116,0,14,2,1,0,
30,4,0,32,100,101,102,32,95,105,110,105,116,40,41,58,
0,0,0,0,16,2,0,64,44,4,0,0,30,4,0,32,
100,101,102,32,95,105,110,105,116,40,41,58,0,0,0,0,
12,1,0,8,112,121,50,98,99,46,112,121,0,0,0,0,
33,1,0,0,12,1,0,5,95,105,110,105,116,0,0,0,
34,1,0,0,30,9,0,33,32,32,32,32,66,85,73,76,
84,73,78,83,91,39,99,111,109,112,105,108,101,39,93,32,
61,32,95,99,111,109,112,105,108,101,0,0,12,2,0,8,
66,85,73,76,84,73,78,83,0,0,0,0,13,1,2,0,
12,3,0,8,95,99,111,109,112,105,108,101,0,0,0,0,
13,2,3,0,12,3,0,7,99,111,109,112,105,108,101,0,
10,1,3,2,30,9,0,34,32,32,32,32,66,85,73,76,
84,73,78,83,91,39,105,109,112,111,114,116,39,93,32,61,
32,95,105,109,112,111,114,116,0,0,0,0,12,2,0,8,
66,85,73,76,84,73,78,83,0,0,0,0,13,1,2,0,
12,3,0,7,95,105,109,112,111,114,116,0,13,2,3,0,
12,3,0,6,105,109,112,111,114,116,0,0,10,1,3,2,
0,0,0,0,12,3,0,5,95,105,110,105,116,0,0,0,
14,3,2,0,30,8,0,36,100,101,102,32,105,109,112,111,
114,116,95,102,110,97,109,101,40,102,110,97,109,101,44,110,
97,109,101,41,58,0,0,0,16,3,0,124,44,10,0,0,
30,8,0,36,100,101,102,32,105,109,112,111,114,116,95,102,
110,97,109,101,40,102,110,97,109,101,44,110,97,109,101,41,
58,0,0,0,12,1,0,8,112,121,50,98,99,46,112,121,
0,0,0,0,33,1,0,0,12,1,0,12,105,109,112,111,
114,116,95,102,110,97,109,101,0,0,0,0,34,1,0,0,
28,2,0,0,9,1,0,2,28,3,0,0,9,2,0,3,
30,3,0,37,32,32,32,32,103,32,61,32,123,125,0,0,
26,4,0,0,15,3,4,0,30,7,0,38,32,32,32,32,
103,91,39,95,95,110,97,109,101,95,95,39,93,32,61,32,
110,97,109,101,0,0,0,0,12,4,0,8,95,95,110,97,
109,101,95,95,0,0,0,0,10,3,4,2,30,6,0,39,
32,32,32,32,77,79,68,85,76,69,83,91,110,97,109,101,
93,32,61,32,103,0,0,0,12,5,0,7,77,79,68,85,
76,69,83,0,13,4,5,0,10,4,2,3,30,5,0,40,
32,32,32,32,115,32,61,32,108,111,97,100,40,102,110,97,
109,101,41,0,12,7,0,4,108,111,97,100,0,0,0,0,
13,6,7,0,15,7,1,0,31,5,7,1,19,5,6,5,
15,4,5,0,30,8,0,41,32,32,32,32,99,111,100,101,
32,61,32,95,99,111,109,112,105,108,101,40,115,44,102,110,
97,109,101,41,0,0,0,0,12,8,0,8,95,99,111,109,
112,105,108,101,0,0,0,0,13,7,8,0,15,8,4,0,
15,9,1,0,31,6,8,2,19,6,7,6,15,5,6,0,
30,7,0,42,32,32,32,32,103,91,39,95,95,99,111,100,
101,95,95,39,93,32,61,32,99,111,100,101,0,0,0,0,
12,6,0,8,95,95,99,111,100,101,95,95,0,0,0,0,
10,3,6,5,30,5,0,43,32,32,32,32,101,120,101,99,
40,99,111,100,101,44,103,41,0,0,0,0,12,8,0,4,
101,120,101,99,0,0,0,0,13,7,8,0,15,8,5,0,
15,9,3,0,31,6,8,2,19,6,7,6,30,4,0,44,
32,32,32,32,114,101,116,117,114,110,32,103,0,0,0,0,
20,3,0,0,0,0,0,0,12,4,0,12,105,109,112,111,
114,116,95,102,110,97,109,101,0,0,0,0,14,4,3,0,
30,4,0,46,100,101,102,32,116,105,110,121,112,121,40,41,
58,0,0,0,16,4,0,50,44,6,0,0,30,4,0,46,
100,101,102,32,116,105,110,121,112,121,40,41,58,0,0,0,
12,1,0,8,112,121,50,98,99,46,112,121,0,0,0,0,
33,1,0,0,12,1,0,6,116,105,110,121,112,121,0,0,
34,1,0,0,30,11,0,47,32,32,32,32,114,101,116,117,
114,110,32,105,109,112,111,114,116,95,102,110,97,109,101,40,
65,82,71,86,91,48,93,44,39,95,95,109,97,105,110,95,
95,39,41,0,12,3,0,12,105,109,112,111,114,116,95,102,
110,97,109,101,0,0,0,0,13,2,3,0,12,5,0,4,
65,82,71,86,0,0,0,0,13,3,5,0,11,5,0,0,
0,0,0,0,0,0,0,0,9,3,3,5,12,4,0,8,
95,95,109,97,105,110,95,95,0,0,0,0,31,1,3,2,
19,1,2,1,20,1,0,0,0,0,0,0,12,5,0,6,
116,105,110,121,112,121,0,0,14,5,4,0,30,5,0,49,
100,101,102,32,109,97,105,110,40,115,114,99,44,100,101,115,
116,41,58,0,16,5,0,67,44,9,0,0,30,5,0,49,
100,101,102,32,109,97,105,110,40,115,114,99,44,100,101,115,
116,41,58,0,12,1,0,8,112,121,50,98,99,46,112,121,
0,0,0,0,33,1,0,0,12,1,0,4,109,97,105,110,
0,0,0,0,34,1,0,0,28,2,0,0,9,1,0,2,
28,3,0,0,9,2,0,3,30,5,0,50,32,32,32,32,
115,32,61,32,108,111,97,100,40,115,114,99,41,0,0,0,
12,6,0,4,108,111,97,100,0,0,0,0,13,5,6,0,
15,6,1,0,31,4,6,1,19,4,5,4,15,3,4,0,
30,6,0,51,32,32,32,32,114,32,61,32,95,99,111,109,
112,105,108,101,40,115,44,115,114,99,41,0,12,7,0,8,
95,99,111,109,112,105,108,101,0,0,0,0,13,6,7,0,
15,7,3,0,15,8,1,0,31,5,7,2,19,5,6,5,
15,4,5,0,30,5,0,52,32,32,32,32,115,97,118,101,
40,100,101,115,116,44,114,41,0,0,0,0,12,7,0,4,
115,97,118,101,0,0,0,0,13,6,7,0,15,7,2,0,
15,8,4,0,31,5,7,2,19,5,6,5,0,0,0,0,
12,6,0,4,109,97,105,110,0,0,0,0,14,6,5,0,
30,7,0,54,105,102,32,95,95,110,97,109,101,95,95,32,
61,61,32,39,95,95,109,97,105,110,95,95,39,58,0,0,
12,7,0,8,95,95,110,97,109,101,95,95,0,0,0,0,
13,6,7,0,12,7,0,8,95,95,109,97,105,110,95,95,
0,0,0,0,23,6,6,7,21,6,0,0,18,0,0,242,
30,7,0,55,32,32,32,32,109,97,105,110,40,65,82,71,
86,91,49,93,44,65,82,71,86,91,50,93,41,0,0,0,
12,8,0,4,109,97,105,110,0,0,0,0,13,7,8,0,
12,10,0,4,65,82,71,86,0,0,0,0,13,8,10,0,
11,10,0,0,0,0,0,0,0,0,240,63,9,8,8,10,
12,10,0,4,65,82,71,86,0,0,0,0,13,9,10,0,
11,10,0,0,0,0,0,0,0,0,0,64,9,9,9,10,
31,6,8,2,19,6,7,6,30,7,0,56,32,32,32,32,
109,97,105,110,40,65,82,71,86,91,49,93,44,65,82,71,
86,91,50,93,41,0,0,0,12,8,0,4,109,97,105,110,
0,0,0,0,13,7,8,0,12,10,0,4,65,82,71,86,
0,0,0,0,13,8,10,0,11,10,0,0,0,0,0,0,
0,0,240,63,9,8,8,10,12,10,0,4,65,82,71,86,
0,0,0,0,13,9,10,0,11,10,0,0,0,0,0,0,
0,0,0,64,9,9,9,10,31,6,8,2,19,6,7,6,
30,7,0,57,32,32,32,32,109,97,105,110,40,65,82,71,
86,91,49,93,44,65,82,71,86,91,50,93,41,0,0,0,
12,8,0,4,109,97,105,110,0,0,0,0,13,7,8,0,
12,10,0,4,65,82,71,86,0,0,0,0,13,8,10,0,
11,10,0,0,0,0,0,0,0,0,240,63,9,8,8,10,
12,10,0,4,65,82,71,86,0,0,0,0,13,9,10,0,
11,10,0,0,0,0,0,0,0,0,0,64,9,9,9,10,
31,6,8,2,19,6,7,6,30,7,0,58,32,32,32,32,
109,97,105,110,40,65,82,71,86,91,49,93,44,65,82,71,
86,91,50,93,41,0,0,0,12,8,0,4,109,97,105,110,
0,0,0,0,13,7,8,0,12,10,0,4,65,82,71,86,
0,0,0,0,13,8,10,0,11,10,0,0,0,0,0,0,
0,0,240,63,9,8,8,10,12,10,0,4,65,82,71,86,
0,0,0,0,13,9,10,0,11,10,0,0,0,0,0,0,
0,0,0,64,9,9,9,10,31,6,8,2,19,6,7,6,
30,7,0,59,32,32,32,32,109,97,105,110,40,65,82,71,
86,91,49,93,44,65,82,71,86,91,50,93,41,0,0,0,
12,8,0,4,109,97,105,110,0,0,0,0,13,7,8,0,
12,10,0,4,65,82,71,86,0,0,0,0,13,8,10,0,
11,10,0,0,0,0,0,0,0,0,240,63,9,8,8,10,
12,10,0,4,65,82,71,86,0,0,0,0,13,9,10,0,
11,10,0,0,0,0,0,0,0,0,0,64,9,9,9,10,
31,6,8,2,19,6,7,6,30,7,0,60,32,32,32,32,
109,97,105,110,40,65,82,71,86,91,49,93,44,65,82,71,
86,91,50,93,41,0,0,0,12,8,0,4,109,97,105,110,
0,0,0,0,13,7,8,0,12,10,0,4,65,82,71,86,
0,0,0,0,13,8,10,0,11,10,0,0,0,0,0,0,
0,0,240,63,9,8,8,10,12,10,0,4,65,82,71,86,
0,0,0,0,13,9,10,0,11,10,0,0,0,0,0,0,
0,0,0,64,9,9,9,10,31,6,8,2,19,6,7,6,
30,7,0,61,32,32,32,32,109,97,105,110,40,65,82,71,
86,91,49,93,44,65,82,71,86,91,50,93,41,0,0,0,
12,8,0,4,109,97,105,110,0,0,0,0,13,7,8,0,
12,10,0,4,65,82,71,86,0,0,0,0,13,8,10,0,
11,10,0,0,0,0,0,0,0,0,240,63,9,8,8,10,
12,10,0,4,65,82,71,86,0,0,0,0,13,9,10,0,
11,10,0,0,0,0,0,0,0,0,0,64,9,9,9,10,
31,6,8,2,19,6,7,6,30,7,0,62,32,32,32,32,
109,97,105,110,40,65,82,71,86,91,49,93,44,65,82,71,
86,91,50,93,41,0,0,0,12,8,0,4,109,97,105,110,
0,0,0,0,13,7,8,0,12,10,0,4,65,82,71,86,
0,0,0,0,13,8,10,0,11,10,0,0,0,0,0,0,
0,0,240,63,9,8,8,10,12,10,0,4,65,82,71,86,
0,0,0,0,13,9,10,0,11,10,0,0,0,0,0,0,
0,0,0,64,9,9,9,10,31,6,8,2,19,6,7,6,
18,0,0,1,0,0,0,0,
};
#ifndef TP_COMPILER
#define TP_COMPILER 1
#endif

#ifdef TP_SANDBOX
#endif

void tp_compiler(TP);

tp_obj tp_None = {TP_NONE};

#if TP_COMPILER
void tp_compiler(TP) {
    tp_import(tp,0,"tokenize",tp_tokenize,sizeof(tp_tokenize));
    tp_import(tp,0,"parse",tp_parse,sizeof(tp_parse));
    tp_import(tp,0,"encode",tp_encode,sizeof(tp_encode));
    tp_import(tp,0,"py2bc",tp_py2bc,sizeof(tp_py2bc));
    tp_ez_call(tp,"py2bc","_init",tp_None);
}
#else
void tp_compiler(TP) { }
#endif

/**/

void tp_sandbox(TP, double time_limit, unsigned long mem_limit) {
    tp->time_limit = time_limit;
    tp->mem_limit = mem_limit;
}

void tp_mem_update(TP) {
/*    static long maxmem = 0;
    if (tp->mem_used/1024 > maxmem) {
        maxmem = tp->mem_used/1024;
        fprintf(stderr,"%ld k\n",maxmem);
    }*/
    if((!tp->mem_exceeded) &&
       (tp->mem_used > tp->mem_limit) &&
       (tp->mem_limit != TP_NO_LIMIT)) {
        tp->mem_exceeded = 1;
        tp_raise(,tp_string("(tp_mem_update) SandboxError: memory limit exceeded"));
    }
}

void tp_time_update(TP) {
    clock_t tmp = tp->clocks;
    if(tp->time_limit != TP_NO_LIMIT)
    {
        tp->clocks = clock();
        tp->time_elapsed += ((double) (tp->clocks - tmp) / CLOCKS_PER_SEC) * 1000.0;
        if(tp->time_elapsed >= tp->time_limit)
            tp_raise(,tp_string("(tp_time_update) SandboxError: time limit exceeded"));
    }
}

#ifdef TP_SANDBOX

void *tp_malloc(TP, unsigned long bytes) {
    unsigned long *ptr = (unsigned long *) calloc(bytes + sizeof(unsigned long), 1);
    if(ptr) {
        *ptr = bytes;
        tp->mem_used += bytes + sizeof(unsigned long);
    }
    tp_mem_update(tp);
    return ptr+1;
}

void tp_free(TP, void *ptr) {
    unsigned long *temp = (unsigned long *) ptr;
    if(temp) {
        --temp;
        tp->mem_used -= (*temp + sizeof(unsigned long));
        free(temp);
    }
    tp_mem_update(tp);
}

void *tp_realloc(TP, void *ptr, unsigned long bytes) {
    unsigned long *temp = (unsigned long *) ptr;
    int diff;
    if(temp && bytes) {
        --temp;
        diff = bytes - *temp;
        *temp = bytes;
        tp->mem_used += diff;
        temp = (unsigned long *) realloc(temp, bytes+sizeof(unsigned long));
        return temp+1;
    }
    else if(temp && !bytes) {
        tp_free(tp, temp);
        return NULL;
    }
    else if(!temp && bytes) {
        return tp_malloc(tp, bytes);
    }
    else {
        return NULL;
    }
}

#endif

tp_obj tp_sandbox_(TP) {
    tp_num time = TP_NUM();
    tp_num mem = TP_NUM();
    tp_sandbox(tp, time, mem);
    tp_del(tp, tp->builtins, tp_string("sandbox"));
    tp_del(tp, tp->builtins, tp_string("mtime"));
    tp_del(tp, tp->builtins, tp_string("load"));
    tp_del(tp, tp->builtins, tp_string("save"));
    tp_del(tp, tp->builtins, tp_string("system"));
    return tp_None;
}

void tp_bounds(TP, tp_code *cur, int n) {
    char *s = (char *)(cur + n);
    tp_obj code = tp->frames[tp->cur].code;
    if (s < code.string.val || s > (code.string.val+code.string.len)) {
        tp_raise(,tp_string("(tp_bounds) SandboxError: bytecode bounds reached"));
    }
}
