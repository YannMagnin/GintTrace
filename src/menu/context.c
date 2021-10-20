#include "gintrace/menu/context.h"
#include "gintrace/ubc.h"
#include "gintrace/tracer.h"
#include "gintrace/gui/menu.h"
#include "gintrace/gui/display.h"

#include <gint/std/stdio.h>
#include <gint/keyboard.h>

#include "./src/menu/internal/dictionary.h"


//---
// Internal information
//---
/* printXY(): Display register information */
static void printXY(int column, int row, const char *text, uintptr_t reg)
{
	char bf[128];
	const char *addrname;
	size_t idx;
	int a;

	a = 0;
	idx = snprintf(bf, 128, text, reg);
	addrname = dictionary_peripherals_check((void*)reg);
	if (addrname != NULL) {
		idx = snprintf(&bf[idx], 128 - idx, " - %s", addrname);
		a = 1;
	}
	addrname = dictionary_syscalls_check((void*)reg);
	if (addrname != NULL) {
		if (a == 0)
			idx = snprintf(&bf[idx], 128 - idx, " - %s", addrname);
		else
			idx = snprintf(&bf[idx], 128 - idx, ", %s", addrname);
		a = 1;
	}
	addrname = dictionary_notes_check((void*)reg);
	if (addrname != NULL) {
		if (a == 0)
			idx = snprintf(&bf[idx], 128 - idx, " - %s", addrname);
		else
			idx = snprintf(&bf[idx], 128 - idx, ", %s", addrname);
		a = 1;
	}
	gtextXY(column, row, bf);
}

/* context_ctor: Menu constructor */
static void context_ctor(struct tsession *session)
{
	session->menu.context.cursor.hoffset = 0;
	session->menu.context.cursor.voffset = 0;
}

/* context_display(); Display trace information */
static void context_display(struct tsession *session)
{
	uintptr_t __vbr;
	int x;
	int y;

	__asm__ volatile ("stc vbr, %0": "=r"(__vbr)::);

	x = session->menu.context.cursor.hoffset;
	y = session->menu.context.cursor.voffset;
	printXY(0 + x, 0 + y, "vbr  : %p", __vbr);
	printXY(0 + x, 0 + y, "gbr  : %p", session->info.context->gbr);
	printXY(0 + x, 1 + y, "macl : %p", session->info.context->mach);
	printXY(0 + x, 2 + y, "mach : %p", session->info.context->macl);
	printXY(0 + x, 3 + y, "ssr  : %p", session->info.context->ssr);
	printXY(0 + x, 4 + y, "pr   : %p", session->info.context->pr);
	printXY(0 + x, 5 + y, "spc  : %p", session->info.context->spc);

	printXY(0 + x, 7 + y,  "r0  : %p", session->info.context->reg[0]);
	printXY(0 + x, 8 + y,  "r1  : %p", session->info.context->reg[1]);
	printXY(0 + x, 9 + y,  "r2  : %p", session->info.context->reg[2]);
	printXY(0 + x, 10 + y, "r3  : %p", session->info.context->reg[3]);
	printXY(0 + x, 11 + y, "r4  : %p", session->info.context->reg[4]);
	printXY(0 + x, 12 + y, "r5  : %p", session->info.context->reg[5]);
	printXY(0 + x, 13 + y, "r6  : %p", session->info.context->reg[6]);
	printXY(0 + x, 14 + y, "r7  : %p", session->info.context->reg[7]);
	printXY(0 + x, 15 + y, "r8  : %p", session->info.context->reg[8]);
	printXY(0 + x, 16 + y, "r9  : %p", session->info.context->reg[9]);
	printXY(0 + x, 17 + y, "r10 : %p", session->info.context->reg[10]);
	printXY(0 + x, 18 + y, "r11 : %p", session->info.context->reg[11]);
	printXY(0 + x, 19 + y, "r12 : %p", session->info.context->reg[12]);
	printXY(0 + x, 20 + y, "r13 : %p", session->info.context->reg[13]);
	printXY(0 + x, 21 + y, "r14 : %p", session->info.context->reg[14]);
	printXY(0 + x, 22 + y, "r15 : %p", session->info.context->reg[15]);
}

/* context_keyboard(): Handle one key event */
static int context_keyboard(struct tsession *session, int key)
{
	if (key == KEY_LEFT)
		session->menu.context.cursor.hoffset += 1;
	if (key == KEY_RIGHT)
		session->menu.context.cursor.hoffset -= 1;

	if (key == KEY_UP)
		session->menu.context.cursor.voffset += 1;
	if (key == KEY_DOWN)
		session->menu.context.cursor.voffset -= 1;

	return (0);
}

//---
// Define the menu
//---
struct menu menu_context = {
	.ctor     = (void*)&context_ctor,
	.init     = NULL,
	.display  = (void*)&context_display,
	.keyboard = (void*)&context_keyboard,
	.command  = NULL,
	.dtor     = NULL
};
