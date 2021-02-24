#ifndef __GINTRACE_MENU_CONTEXT_H__
# define __GINTRACE_MENU_CONTEXT_H__

#include <stddef.h>
#include <stdint.h>

/* define font information */
/* TODO: move me !*/
#ifdef FXCG50
#define FWIDTH	9
#define FHEIGHT	10
#endif
#ifdef FX9860G
#define FWIDTH	5
#define FHEIGHT	7
#endif

struct mcontext {
	struct {
		int hoffset;
		int voffset;
	} cursor;
};

/* extern menu information */
extern struct menu menu_context;

#endif /*__GINTRACE_MENU_CONTEXT_H__*/
