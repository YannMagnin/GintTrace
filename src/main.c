#include "gintrace/gui/menu.h"
#include "gintrace/menu/disasm.h"
#include "gintrace/menu/context.h"
#include "gintrace/menu/hexdump.h"
#include "gintrace/menu/callgraph.h"
#include "gintrace/ubc.h"

#include <gint/gint.h>
#include <gint/keyboard.h>
#include <gint/display.h>
#include <gint/bfile.h>

/* workaround test */
extern struct tracer tracer;

/* save the selected menu */
static struct menu_group *gmenu = NULL;

/* syscall address */
static void *syscall = NULL;

/* gintrac_handler(): UBC handler
 * @note:
 *   To force generate the callgraph, we use a dirty workaround to force break
 *   at each instruction. But, the disassembler menu can skip one instruction
 *   using OPTN key, so you should not "unskip" the user action. */
static void gintrace_handler(struct ucontext *context)
{
	static uintptr_t breakpoint = 0x00000000;
	static uintptr_t spc = 0x00000000;

	/* force disable the UBC to avoid error */
	ubc_block();

	/* check callgraph job */
	if (breakpoint != 0x00000000
			&& spc != 0x00000000
			&& spc != breakpoint) {
		menu_callgraph.init(context);
		spc = context->spc;
		ubc_set_breakpoint(0, (void*)context->spc, NULL);
		ubc_unblock();
		return;
	}

	/* user break point */
	gint_switch_to_gint();
	menu_init(gmenu, context);
	while (menu_is_open(gmenu) == 0) {
		menu_draw(gmenu);
		menu_keyboard(gmenu);
	}

	/* if no instruction skip, restore */
	spc = context->spc;
	ubc_set_breakpoint(0, (void*)context->spc, NULL);
	breakpoint = tracer.next_break;

	/* unblock UBC interrupt */
	ubc_unblock();
	gint_switch_to_casio();
}

/* casio_handler(): Casio handler */
static void casio_handler(void)
{
	void (*bfile_openfile_os)(const uint16_t *filename, int mode, int p3);

	bfile_openfile_os = syscall;
	bfile_openfile_os(u"\\fls0\\dyntest", BFile_ReadOnly, 0);
}

/* main(): User entry */
int main(void)
{
	/* initialize all internal menu and get the "first" menu to display */
	menu_create(&gmenu);
	menu_register(gmenu, &menu_disasm, "Disasm");
	menu_register(gmenu, &menu_context, "Context");
	menu_register(gmenu, &menu_hexdump, "Hexdump");
	menu_register(gmenu, &menu_callgraph, "CallG");

	/* get syscall address */
	void **systab = *(void ***)0x8002007c;
	syscall = systab[0x1da3];


	/* intialize UBC information */
	ubc_install();
	ubc_set_handler(&gintrace_handler);
	//ubc_set_breakpoint(0, (void*)0x80358a6c, NULL);
	ubc_set_breakpoint(0, syscall, NULL);

	/* try to trace the function */
	//gint_switch((void *)0x80358a6c);	// stack menu (syscall %1e66)
	gint_switch((void*)&casio_handler);

	//TODO : destructor part !!
	return (0);
}
