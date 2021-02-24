#include "gintrace/gui/menu.h"
#include "gintrace/menu/disasm.h"
#include "gintrace/menu/context.h"
#include "gintrace/menu/hexdump.h"
#include "gintrace/ubc.h"

#include <gint/gint.h>
#include <gint/keyboard.h>
#include <gint/display.h>

/* save the selected menu */
static struct menu_group *gmenu = NULL;

/* gintrac_handler(): UBC handler */
static void gintrace_handler(struct ucontext *context)
{
	gint_switch_to_gint();
	menu_init(gmenu, context);
	while (menu_is_open(gmenu) == 0) {
		menu_draw(gmenu);
		menu_keyboard(gmenu);
	}
	gint_switch_to_casio();
}

int main(void)
{
	/* initialize all internal menu and get the "first" menu to display */
	menu_create(&gmenu);
	menu_register(gmenu, &menu_disasm, "Disasm");
	menu_register(gmenu, &menu_context, "Context");
	menu_register(gmenu, &menu_hexdump, "Hexdump");
	menu_register(gmenu, NULL, "CallG");

	/* intialize UBC information */
	ubc_install();
	ubc_set_handler(&gintrace_handler);
	//ubc_set_breakpoint(0, (void*)0x80358a6c, NULL);
	ubc_set_breakpoint(0, (void*)0x80358a7e, NULL);

	/* try to trace the function */
	gint_switch((void *)0x80358a6c);

	//TODO : destructor part !!
	return (0);
}
