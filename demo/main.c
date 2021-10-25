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

#ifdef FXCG50
	/* get syscall address */
	systab = *(void ***)0x8002007c;
	//syscall = systab[0x1e48];	// Fugue_debug_menu()
	//syscall = systab[0x1da3];	// Bfile_OpenFile_OS()
	//syscall = systab[0x1353];	// Comm_Open()
	//syscall = systab[0x1dac];	// Bfile_ReadFile_OS()
	syscall = systab[0x1630];	// App_optimize()
	//syscall = systab[0x1e56];
#endif
#ifdef FX9860G
	systab = *(void ***)0x8001007c;
	syscall = systab[0x90f];		// getkey()
#endif

	/* prepare tracer */
	session = tracer_create_session(
		syscall,
		TRACER_DISASM | TRACER_CONTEXT | TRACER_HEXDUMP
	);
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
	void (*app_optimize)(void) = (void*)syscall;
	gint_world_switch(GINT_CALL(app_optimize));


#if 0
	int (*bfile_openfile_os)(const uint16_t *filename, int mode, int p3);
	int (*bfile_readfile_os)(int handle, void *b, size_t n, off_t p);
	//void (*comm_open)(int mode) = syscall;
	uint8_t buffer[128];
	int handle;

	gint_switch_to_world(kernel_env_casio);
	bfile_openfile_os = systab[0x1da3];
	handle = bfile_openfile_os(u"\\\\fls0\\azerty", BFile_ReadOnly, 0);
	bfile_readfile_os = systab[0x1dac];
	bfile_readfile_os(handle, buffer, 128, 0);
	gint_switch_to_world(kernel_env_gint);
#endif



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
