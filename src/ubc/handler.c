#include "gintrace/gui/menu.h"
#include "gintrace/menu/disasm.h"
#include "gintrace/menu/context.h"
#include "gintrace/menu/hexdump.h"
#include "gintrace/menu/callgraph.h"
#include "gintrace/ubc.h"
#include "gintrace/tracer.h"

/* world information */
extern void *kernel_env_tracer;
extern void *gint_switch_to_world(void *buffctx);

/* gintrac_handler(): UBC handler
 * @note:
 *   To force generate the callgraph, we use a dirty workaround to force break
 *   at each instruction. But, the disassembler menu can skip one instruction
 *   using OPTN key, so you should not "unskip" the user action. */
void gintrace_handler(struct ucontext *context)
{
	struct tsession *session;

	/* force disable the UBC to avoid error */
	ubc_block();

	/* check session validity */
	session = tracer_get_session();
	if (session == NULL) {
		ubc_unblock();
		return;
	}
	session->info.context = context;

	/* check if the special job force skip the breakpoint
	 * @note:
	 *   This feature is used by the callgraph menu to force the call graph
	 *   generation.*/
	if (menu_special_ctor(session->display.gmenu) != 0) {
		ubc_unblock();
		return;
	}

	/* user break point */
	session->info.buffctx = gint_switch_to_world(kernel_env_tracer);
	menu_init(session->display.gmenu);
	while (menu_is_open(session->display.gmenu) == 0) {
		menu_draw(session->display.gmenu);
		menu_keyboard(session->display.gmenu);
	}

	/* check if a special destructor overwrite the default breakpoint
	 * generation. If not, set the next break point. */
	if (menu_special_dtor(session->display.gmenu) == 0) {
		ubc_set_breakpoint(0,
				(void*)session->menu.disasm.next_break, NULL);
	}

	/* unblock UBC interrupt */
	ubc_unblock();
	gint_switch_to_world(session->info.buffctx);
}
