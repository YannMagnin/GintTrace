#ifndef __GINTRACE_MENU_CALLGRAPH_H__
# define __GINTRACE_MENU_CALLGRAPH_H__

#include <stddef.h>
#include <stdint.h>

#include "gintrace/ubc.h"

struct callnode {
	struct ucontext context;
	uintptr_t address;
	enum {
		callnode_type_root = 0,
		callnode_type_bsr  = 1,
		callnode_type_bsrf = 2,
		callnode_type_jsr  = 3,
		callnode_type_jmp  = 4,
		callnode_type_rte  = 5,
		callnode_type_rts  = 6,
	} type;
	struct callnode *parent;
	struct callnode *sibling;
	struct callnode *child;
};

struct callgraph {
	struct {
		int hoffset;
		int voffset;
	} cursor;
	struct callnode *root;
	struct callnode *parent;
};

/* extern menu information */
extern struct menu menu_callgraph;

#endif /*__GINTRACE_MENU_CALLGRAPH_H__*/
