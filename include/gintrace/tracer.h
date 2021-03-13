#ifndef __GINTRACE_TRACER_H__
# define __GINTRACE_TRACER_H__

#include <stddef.h>
#include <stdint.h>

#include "gintrace/menu/hexdump.h"
#include "gintrace/menu/callgraph.h"
#include "gintrace/menu/disasm.h"
#include "gintrace/menu/context.h"
#include "gintrace/ubc.h"

/* tsession: define the trace session information */
struct tsession {
	/* session information */
	struct {
		void *starting;
		void *removed;
		struct ucontext *context;
	} info;

	/* display information */
	struct {
		struct menu_group *gmenu;
	} display;

	/* menu information */
	struct {
		struct disasm disasm;
		struct callgraph callgraph;
		struct context context;
		struct hexdump hexdump;
	} menu;
};

//---
// User API
//---
/* tracer_new_session(): Create a new session
 * @note
 *   This function will create a new session based on the given menu
 *   information and the firt breakpoint address.
 * */
#define TRACER_DISASM		(1 << 0)
#define TRACER_CONTEXT		(1 << 1)
#define TRACER_HEXDUMP		(1 << 2)
#define TRACER_CALLGRAPH	(1 << 3)
extern struct tsession *tracer_create_session(void *address, int menu);

/* tracer_get_session(): Get the current session. */
extern struct tsession *tracer_get_session(void);

/* tracer_get_session(): Set the current session. */
extern struct tsession *tracer_set_session(struct tsession *session);

#endif /*__GINTRACE_TRACER_H__*/
