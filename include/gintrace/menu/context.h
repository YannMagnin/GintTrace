#ifndef __GINTRACE_MENU_CONTEXT_H__
# define __GINTRACE_MENU_CONTEXT_H__

#include <stddef.h>
#include <stdint.h>

/* mcontext: menu context internal structure */
struct mcontext {
	struct {
		int hoffset;
		int voffset;
	} cursor;
};

/* extern menu information */
extern struct menu menu_context;

#endif /*__GINTRACE_MENU_CONTEXT_H__*/
