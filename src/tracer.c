#include "gintrace/tracer.h"

#include <gint/std/string.h>
#include <gint/std/stdlib.h>

/* global session  (TODO: thread-safe) */
struct tsession *session = NULL;

/* tracer_new_session(): Create a new session */
struct tsession *tracer_create_session(void *address, int menu)
{
	extern void gintrace_handler(struct ucontext *context);
	struct tsession *session;

	if (address == NULL || (menu & (TRACER_DISASM
					| TRACER_CONTEXT
					| TRACER_HEXDUMP
					| TRACER_CALLGRAPH)) == 0) {
		return(NULL);
	}

	session = calloc(sizeof(struct tsession), 1);
	if (session == NULL)
		return (NULL);
	if (menu_create(&session->display.gmenu, session) != 0) {
		free(session);
		return (NULL);
	}
	if ((menu & TRACER_DISASM) != 0)
		menu_register(session->display.gmenu, &menu_disasm, "Disasm");
	if ((menu & TRACER_CONTEXT) != 0)
		menu_register(session->display.gmenu, &menu_context, "Context");
	if ((menu & TRACER_HEXDUMP) != 0)
		menu_register(session->display.gmenu, &menu_hexdump, "Hexdump");
	if ((menu & TRACER_CALLGRAPH) != 0)
		menu_register(session->display.gmenu, &menu_callgraph, "CallG");

	ubc_install();
	ubc_set_handler(&gintrace_handler);
	ubc_set_breakpoint(0, address, NULL);
	return (session);
}

/* tracer_get_session(): Get the current session. */
struct tsession *tracer_get_session(void)
{
	return (session);
}

/* tracer_get_session(): Set the current session. */
struct tsession *tracer_set_session(struct tsession *new)
{
	void *old;

	old = session;
	session = new;
	return (old);
}
