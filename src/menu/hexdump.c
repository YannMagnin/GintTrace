#include "gintrace/menu/hexdump.h"
#include "gintrace/ubc.h"
#include "gintrace/gui/menu.h"
#include "gintrace/gui/display.h"
#include "gintrace/gui/input.h"

#include <gint/std/stdio.h>
#include <gint/std/string.h>
#include <gint/keyboard.h>

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
		gprintXY(x - 2, y + i, "%p", record);
		for (int j = 0; j < 8; ++j) {
			gprintXY(x + 9 + (j * 3), y + i, "%.2x", record[j]);
			if (record[j] < 0x20  || record[j] > 0x7e) {
				gtextXY(x + 34 + j, y + i, ".");
				continue;
			}
			gprintXY(x + 34 + j, y + i, "%c", record[j]);
		}
		gtextXY(x + 33, y + i, "|");
		gtextXY(x + 42, y + i, "|");
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

/* hexdump_command(): Handle user command */
static void hexdump_command(struct ucontext *context, int argc, char **argv)
{
	uintptr_t address;
	int i;

	(void)context;
	if (argc != 2)
		return;
	if (strcmp(argv[0], "jmp") != 0) {
		input_write("unknown '%s' command", argv[0]);
		return;
	}
	i = -1;
	address = 0x00000000;
	while (++i < 8 && argv[1][i] != '\0') {
		address = address << 4;
		if (argv[1][i] >= '0' && argv[1][i] <= '9') {
			address = address + argv[1][i] - '0';
			continue;
		}
		if (argv[1][i] >= 'a' && argv[1][i] <= 'f') {
			address = address + argv[1][i] - 'a' + 10;
			continue;
		}
		if (argv[1][i] >= 'A' && argv[1][i] <= 'F') {
			address = address + argv[1][i] - 'A' + 10;
			continue;
		}
		input_write("'%s': second argument error", argv[0]);
		return;
	}
	hexdump.addr = (void*)(address & ~3);
}

//---
// Define the menu
//---
struct menu menu_hexdump = {
	.ctor     = &hexdump_ctor,
	.init     = NULL,
	.display  = &hexdump_display,
	.keyboard = &hexdump_keyboard,
	.command  = &hexdump_command,
	.dtor     = NULL
};
