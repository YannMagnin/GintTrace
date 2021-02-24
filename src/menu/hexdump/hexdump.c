#include "gintrace/menu/hexdump.h"
#include "gintrace/ubc.h"
#include "gintrace/gui/menu.h"
#include "gintrace/gui/display.h"

#include <gint/std/stdio.h>
#include <gint/keyboard.h>
#include <gint/display.h>

/* define the menu information */
/* TODO: find a way to have local information (session) */
struct hexdump hexdump;

/* hexdump_ctor: Menu constructor */
static void hexdump_ctor(void)
{
	hexdump.cursor.hoffset = 0;
	hexdump.cursor.voffset = 0;
	hexdump.addr = (void*)0x88000000;
}

/* hexdump_display(); Display trace information */
static void hexdump_display(struct ucontext *context)
{
	uint8_t *record;
	int x;
	int y;

	(void)context;
	record = hexdump.addr;
	x = hexdump.cursor.hoffset;
	y = hexdump.cursor.voffset;
	for (int i = 0; i < GUI_DISP_NB_ROW; ++i) {
		dprint(x * (FWIDTH + 1), (y + i) * (FHEIGHT + 1),
						C_BLACK, "%p ", record);
		for (int j = 0; j < 8; ++j) {
			dprint((x + 9 + (j * 3)) * (FWIDTH + 1),
					(y + i) * (FHEIGHT + 1),
					C_BLACK, "%.2x ", record[j]);
			if (record[j] < 0x20  || record[j] > 0x7e) {
				dtext((x + 34 + j) * (FWIDTH + 1),
						(y + i) * (FHEIGHT + 1),
						C_BLACK,  ".");
				continue;
			}
			dprint((x + 34 + j) * (FWIDTH + 1),
					(y + i) * (FHEIGHT + 1),
					C_BLACK, "%c", record[j]);
		}
		dtext((x + 33) * (FWIDTH + 1),
				(y + i) * (FHEIGHT + 1), C_BLACK, "|");
		dtext((x + 42) * (FWIDTH + 1),
				(y + i) * (FHEIGHT + 1), C_BLACK, "|");
		record = &record[8];
	}
}

/* hexdump_keyboard(): Handle one key event */
static int hexdump_keyboard(struct ucontext *context, int key)
{
	(void)context;
	if (key == KEY_LEFT)
		hexdump.cursor.hoffset += 1;
	if (key == KEY_RIGHT)
		hexdump.cursor.hoffset -= 1;

	if (key == KEY_UP)
		hexdump.addr = &hexdump.addr[-8];
	if (key == KEY_DOWN)
		hexdump.addr = &hexdump.addr[8];

	return (0);
}

//---
// Define the menu
//---
struct menu menu_hexdump = {
	.ctor     = &hexdump_ctor,
	.init     = NULL,
	.display  = &hexdump_display,
	.keyboard = &hexdump_keyboard,
	.command  = NULL,
	.dtor     = NULL
};
