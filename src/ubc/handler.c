#include "gintrace/gui/menu.h"
#include "gintrace/menu/disasm.h"
#include "gintrace/menu/context.h"
#include "gintrace/menu/hexdump.h"
#include "gintrace/menu/callgraph.h"
#include "gintrace/ubc.h"
#include "gintrace/tracer.h"

#include <gint/gint.h>

// custom function
extern void *kernel_env_gint;
extern void *kernel_env_tracer;
extern void *gint_switch_to_world(void *buffctx);

/* gintrac_handler(): UBC handler
 * @note:
 *   To force generate the callgraph, we use a dirty workaround to force break
 *   at each instruction. But, the disassembler menu can skip one instruction
 *   using OPTN key, so you should not "unskip" the user action. */
//FIXME: move breakpoint
//FIXME: move spc
void gintrace_handler(struct ucontext *context)
{
	static uintptr_t breakpoint = 0x00000000;
	static uintptr_t spc = 0x00000000;
	struct tsession *session;
	void *buffctx;

	/* force disable the UBC to avoid error */
	ubc_block();

	/* check session validity */
	session = tracer_get_session();
	if (session == NULL) {
		ubc_unblock();
		return;
	}
	session->info.context = context;

	/* check callgraph job */
	//FIXME: move me
	if (breakpoint != 0x00000000
			&& spc != 0x00000000
			&& spc != breakpoint) {
		menu_callgraph.init(session);
		spc = context->spc;
		ubc_set_breakpoint(0, (void*)context->spc, NULL);
		ubc_unblock();
		return;
	}

	/* user break point */
	buffctx = gint_switch_to_world(kernel_env_tracer);
	menu_init(session->display.gmenu);
	while (menu_is_open(session->display.gmenu) == 0) {
		menu_draw(session->display.gmenu);
		menu_keyboard(session->display.gmenu);
	}

	/* if no instruction skip, restore */
	//FIXME: move me
	if (session->menu.disasm.skip == 0) {
		spc = context->spc;
		ubc_set_breakpoint(0, (void*)context->spc, NULL);
		breakpoint = session->menu.disasm.next_break;
	} else {
		breakpoint = session->menu.disasm.next_break;
		ubc_set_breakpoint(0, (void*)breakpoint, NULL);
		spc = breakpoint;
	}

	/* unblock UBC interrupt */
	ubc_unblock();
	gint_switch_to_world(buffctx);
}
