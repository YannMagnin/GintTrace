#include "gintrace/tracer.h"

#include <gint/bfile.h>
#include <gint/display.h>
#include <gint/keyboard.h>
#include <gint/gint.h>

extern void *kernel_env_gint;
extern void *kernel_env_casio;
extern void *gint_switch_to_world(void *buffctx);

/* main(): User entry */
int main(void)
{
	struct tsession *session;
	void **systab;
	void *syscall;

#if 0
	int x = 0;
	int key = 0;
	while (1) {
		dclear(C_WHITE);
		dtext(0, 0, C_BLACK, "Unable to create tracer session");
		dtext(0, 10, C_BLACK, "Press [MENU]...");
		dprint(x, 20, C_BLACK, "buffctx:  %p", drivers_get_current());
		dprint(x, 30, C_BLACK, "gintctx:  %p", kernel_env_gint);
		dprint(x, 40, C_BLACK, "casioctx: %p", kernel_env_casio);
		dupdate();

		key = getkey().key;
		if (key == KEY_LEFT)
			x = x + 10;
		if (key == KEY_RIGHT)
			x = x - 10;
	}
#endif

	/* get syscall address */
	systab = *(void ***)0x8002007c;
	//syscall = systab[0x1e48];	// Fugue_debug_menu
	syscall = systab[0x1da3];	// Bfile_OpenFile_OS


	/* prepare tracer */
	session = tracer_create_session(syscall,
			TRACER_DISASM | TRACER_CONTEXT | TRACER_HEXDUMP | TRACER_CALLGRAPH);
	if (session == NULL) {
		dclear(C_WHITE);
		dtext(0, 0, C_BLACK, "Unable to create tracer session");
		dtext(0, 10, C_BLACK, "Press [MENU]...");
		dupdate();
		while (1) { getkey(); }
	}
	tracer_set_session(session);



	//---
	// TEST part
	//---
	void (*bfile_openfile_os)(const uint16_t *filename, int mode, int p3);

	gint_switch_to_world(kernel_env_casio);
	bfile_openfile_os = syscall;
	bfile_openfile_os(u"\\\\fls0\\abcdefgijklmn", BFile_ReadOnly, 0);
	gint_switch_to_world(kernel_env_gint);



	//---
	// Epilogue
	// TODO: restart the trace ?
	//---
	dclear(C_WHITE);
	dtext(0, 0, C_BLACK, "session come to the end");
	dtext(0, 10, C_BLACK, "Press [MENU] to exit");
	dupdate();
	while (1) { getkey(); }
	return (0);

#if 0
	/* initialize all internal menu and get the "first" menu to display */
	menu_create(&gmenu);
	menu_register(gmenu, &menu_disasm, "Disasm");
	menu_register(gmenu, &menu_context, "Context");
	menu_register(gmenu, &menu_hexdump, "Hexdump");
	menu_register(gmenu, &menu_callgraph, "CallG");

	/* get syscall address */
	void **systab = *(void ***)0x8002007c;
	//syscall = systab[0x1e48];	// Fugue_debug_menu
	syscall = systab[0x1da3];	// Bfile_OpenFile_OS

	/* prepare tracer */
	tracer_create_session(syscall, TRACER_DISASM | TRACER_CONTEXT | TRACER_HEXDUMP);

	/* intialize UBC information */
	ubc_install();
	ubc_set_handler(&gintrace_handler);
	ubc_set_breakpoint(0, syscall, NULL);

	/* try to trace the function */
	gint_switch((void*)&casio_handler);

	//TODO : destructor part !!
	return (0);
#endif
}
