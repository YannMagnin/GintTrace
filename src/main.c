#include "gintrace/tracer.h"

#include <gint/bfile.h>
#include <gint/display.h>
#include <gint/keyboard.h>
#include <gint/gint.h>


/* main(): User entry */
int main(void)
{
	struct tsession *session;
	void **systab;
	void *syscall;

	/* get syscall address */
	systab = *(void ***)0x8002007c;
	//syscall = systab[0x1e48];	// Fugue_debug_menu
	syscall = systab[0x1da3];	// Bfile_OpenFile_OS


	/* prepare tracer */
	session = tracer_create_session(syscall,
			TRACER_DISASM | TRACER_CONTEXT | TRACER_HEXDUMP);
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

	gint_switch_to_casio();
	bfile_openfile_os = syscall;
	bfile_openfile_os(u"\\\\fls0\\abcdefgijklmn", BFile_ReadOnly, 0);
	gint_switch_to_gint();



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
