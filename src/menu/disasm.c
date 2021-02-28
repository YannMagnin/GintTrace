#include "gintrace/menu/disasm.h"
#include "gintrace/ubc.h"
#include "gintrace/gui/menu.h"
#include "gintrace/gui/display.h"
#include "gintrace/gui/input.h"

#include "./src/menu/internal/dictionary.h"

#include <gint/std/string.h>
#include <gint/std/stdlib.h>
#include <gint/std/stdio.h>
#include <gint/display.h>
#include <gint/keyboard.h>

/* define the menu information */
/* TODO: find a way to have local information (session) */
struct tracer tracer;

//---
// Internal disassembler processing
//---
/* dhreverse(): Reverse pixels color on a horizontal area (TODO: move me !!) */
static void dhreverse(int ypos, size_t size)
{
#ifdef FXCG50
	/* FIXME: border !!! */
	uint32_t *long_vram;
	int y1;
	int y2;

	y1 = ypos;
	y2 = ypos + size;
	long_vram = (void *)gint_vram;
	for(int i = 198 * y1; i < 198 * y2; i++)
		long_vram[i] = ~long_vram[i];
#endif
}


/* disasm_util_line_update(): Little helper to update the line index
 * @nte:
 *   This function will update the given line circular index based on the given
 *   direction. This is a dirty way to avoid use of modulo.
 *
 * @arg
 *   - line_idx  The line index
 *   - direction The direction: upward (negative) and downward (positive)
 *
 * @return
 *   - the new line index */
static int disasm_util_line_update(int line_idx, int direction)
{
	line_idx = line_idx + direction;
	while (1) {
		if (line_idx < 0) {
			line_idx += (int)tracer.buffer.size.height;
			continue;
		}
		if (line_idx >= (int)tracer.buffer.size.height) {
			line_idx -= (int)tracer.buffer.size.height;
			continue;
		}
		break;
	}

	/* return the index */
	return (line_idx);
}

/* disasm_check_special_addr(): Check if the address is special.
 * @note:
 *   This function will check if the given address is special (like a syscall or
 *   a register) and generate a string which content all information.
 *
 * @arg:
 * - buffer   Buffer to store the string information
 * - address  Address to check
 * - type     Type of check:
 *              type & 1 -> check hardware module register
 *              type & 2 -> check syscall address
 *              type & 4 -> check OS-specific address */
static void disasm_check_special_addr(char *buffer, void *address, int type)
{
	extern struct tracer tracer;
	const char *addrname;

	addrname = NULL;
	if ((type & 1) != 0)
		addrname = dictionary_peripherals_check(address);
	if (addrname == NULL && (type & 2) != 0)
		addrname = dictionary_syscalls_check(address);
	if (addrname == NULL && (type & 4) != 0)
		addrname = dictionary_notes_check(address);
	if (addrname == NULL)
		return;
	snprintf(buffer, tracer.buffer.size.width,
			"@note: %p -> %s\n", address, addrname);
}

/* disasm_get_mnemonic_info(): Get mnemonic information
 *
 * @note:
 *   This function will analyze the given instruction's address. All of its
 *   information will be stored in a buffer "line". A line is constituted of:
 *     - line[0] -> "address opcode opcode_name"
 *     - line[1] -> address note (syscall, register, ...)
 *     - line[2] -> note (arg0)
 *     - line[3] -> note (arg1)
 *     - line[4] -> note (arg2)
 *
 * @arg:
 *   - line   Line index, used to find the buffer line to store information
 *   - pc     Current Program Counter which store the instruction address */
static void disasm_get_mnemonic_info(int line, uint16_t *pc)
{
	const struct opcode *opcode_info;
	uint16_t opcode;
	uintptr_t arg[3];
	int note;

	/* Wipe note */
	tracer.buffer.raw[line][0][0] = '\0';
	tracer.buffer.raw[line][1][0] = '\0';
	tracer.buffer.raw[line][2][0] = '\0';
	tracer.buffer.raw[line][3][0] = '\0';
	tracer.buffer.raw[line][4][0] = '\0';

	/* check special address (register, syscall, ...) */
	note = 1;
	disasm_check_special_addr(tracer.buffer.raw[line][note], pc, 6);
	if (tracer.buffer.raw[line][note][0] != '\0')
		note += 1;

	/* generate the "default" string info (only the address and word) */
	snprintf(&tracer.buffer.raw[line][0][0],
			tracer.buffer.size.width, "%.8lx %.4x ",
			(uintptr_t)pc, (unsigned int)pc[0]);

	/* Try to get the current opcode */
	opcode = pc[0];
	opcode_info = dictionary_opcodes_check(opcode);
	if (opcode_info == NULL) {
		snprintf(&tracer.buffer.raw[line][0][14],
				tracer.buffer.size.width - 14,
				".word\t%#.4x", (int)opcode);
		return;
	}

	/* get mnemonic arguments, if available */
	for (int i = 0 ; i < 3 ; ++i) {
		arg[i] = dictionary_opcodes_get_arg(opcode_info, opcode, i, pc);
		if (arg[i] == 0x00000000)
			continue;
		disasm_check_special_addr(tracer.buffer.raw[line][note++],
				(void*)arg[i], 7);
	}

	/* generate the complete mnemonic information */
	snprintf(&tracer.buffer.raw[line][0][14],
			tracer.buffer.size.width - 14,
			opcode_info->name, arg[0], arg[1], arg[2]);
}

/* disasm_util_note_counter(): Retrun the index max of a line
 * @note
 *   This function will return the maximum index for a given line. That line
 *   contains one instruction information and four possible notes. So, the index
 *   that will be returned can be set in this range [0;4].
 *
 * @arg
 *   - line_idx   Line index
 *
 * @return
 *   The note index max of the line_idx */
static int disasm_util_note_counter(int line_idx)
{
	int note_idx;

	note_idx = 0;
	while (++note_idx < 5) {
		if (tracer.buffer.raw[line_idx][note_idx][0] == '\0')
			break;
	}
	return (note_idx - 1);
}

/* disasm_util_row_update(): Update the row counter and return true if OOB
 * @note:
 *   This function will update the row counter, used to display one line of
 *   information. If the row counter exits the screen, true is returned
 *
 * @arg:
 *   - row       The row counter
 *   - direction The direction: upward (-1), downward (1)
 *
 * @return
 *   0           The row is always visible
 *   1           The row is out of screen */
static int disasm_util_row_update(int *row, int direction)
{
	*row = *row + direction;
	return(*row < 0 || *row >= GUI_DISP_NB_ROW);
}

/* disasm_util_line_fetch(): Generate all information for the given line.
 *
 * @arg:
 *   - line_idx   Line index
 *   - pc         Memory address
 *   - direction  Direction to push the limit */
static void disasm_util_line_fetch(int line_idx, uint16_t *pc, int direction)
{
	if (tracer.buffer.raw[line_idx][0][0] == '\0') {
		disasm_get_mnemonic_info(line_idx, pc);
		line_idx = disasm_util_line_update(line_idx, direction);
		tracer.buffer.raw[line_idx][0][0] = '\0';
	}
}

/* disasm_display_addr_info(): Display entiere line information
 * @note
 *   This function is a bit complex. It will display, if available, one entire
 *   line's information like address, opcode name and notes. If the line is not
 *   available, it will be generated and cached into a circular buffer.
 *
 *   The process to get one instruction information (opcode + notes) cost a lot
 *   of time because of the amount of data to check. This is why a part of the
 *   display is cached into an internal circular buffer to avoid display lags
 *   during the displacement.
 *
 * @args:
 *  - row        current row id (used to display information)
 *  - line_idx   buffer line index
 *  - context    context information
 *  - direction  display upward (negative) or downward (positive)
 *
 * @return
 *  - 0          Row is still on the screen
 *  - 1          Row is out-of-screen */
int disasm_display_addr_info(int *row, struct buffcursor *cursor,
		struct ucontext *context, int direction, int pc)
{
	int note_max_idx;
	void *ptr;

	/* generate the line if missing and move the limit */
	disasm_util_line_fetch(cursor->line_idx,
			&tracer.buffer.anchor.addr[pc], direction);

	/* get the last note index if downward */
	note_max_idx = 5;
	if (direction == 1)
		note_max_idx = disasm_util_note_counter(cursor->line_idx);

	/* walk trough the notes */
	while (cursor->note_idx <= note_max_idx && cursor->note_idx >= 0) {
		/* check loop condition */
		ptr = tracer.buffer.raw[cursor->line_idx][cursor->note_idx];
		if (((char*)ptr)[0] == '\0')
			break;

		/* check note */
		if (cursor->note_idx != 0) {
			drect(0, (*row) * (FHEIGHT + 1) - 1, DWIDTH,
				((*row) * (FHEIGHT + 1) - 1) + (FHEIGHT + 1),
				0x556655);
			dtext(tracer.disp.hoffset * (FWIDTH + 1),
					(*row) * (FHEIGHT + 1), C_WHITE, ptr);
		} else {
			/* check instruction */
			dtext(tracer.disp.hoffset * (FWIDTH + 1),
					(*row) * (FHEIGHT + 1), C_BLACK, ptr);

			/* highlight SPC if possible */
			ptr = &tracer.buffer.anchor.addr[pc];
			if ((uintptr_t)ptr == context->spc)
				dhreverse((*row) * (FHEIGHT + 1) - 1,
								FHEIGHT + 2);
			/* draw next break / instruction */
			if (tracer.next_break != context->spc
					&& ptr == (void*)tracer.next_break) {
				dhline((*row) * (FHEIGHT + 1) - 1, C_BLACK);
				dhline( ((*row) + 1) * (FHEIGHT + 1) - 1,
								C_BLACK);
			}
		}

		/* update internal information */
		if (disasm_util_row_update(row, direction) == 1)
			return (1);
		cursor->note_idx -= direction;
	}
	return (0);
}



//---
// Menu functions
//---
/* disasm_ctor(): disasm menu constructor */
static void disasm_ctor(void)
{
	memset(&tracer, 0x00, sizeof(struct tracer));
	tracer.buffer.size.width  = GUI_DISP_NB_COLUMN * 2;
	tracer.buffer.size.height = GUI_DISP_NB_ROW * 2 + 2;
	tracer.buffer.raw = calloc(sizeof(void*), tracer.buffer.size.height);
	for (size_t i = 0; i < tracer.buffer.size.height; ++i) {
		tracer.buffer.raw[i] = calloc(sizeof(char*), 5);
		tracer.buffer.raw[i][0] = calloc(tracer.buffer.size.width, 1);
		tracer.buffer.raw[i][1] = calloc(tracer.buffer.size.width, 1);
		tracer.buffer.raw[i][2] = calloc(tracer.buffer.size.width, 1);
		tracer.buffer.raw[i][3] = calloc(tracer.buffer.size.width, 1);
		tracer.buffer.raw[i][4] = calloc(tracer.buffer.size.width, 1);
	}
	tracer.buffer.cursor.line_idx = 0;
	tracer.buffer.cursor.note_idx = 0;
	tracer.disp.hoffset = 0;
	tracer.disp.voffset = 0;
}

/* disasm_dtor(): disasm menu destructor */
static void disasm_dtor(void)
{
	if (tracer.buffer.raw != NULL) {
		for (size_t i = 0; i < tracer.buffer.size.height; ++i) {
			if (tracer.buffer.raw[i] == NULL)
				continue;
			free(tracer.buffer.raw[i][0]);
			free(tracer.buffer.raw[i][1]);
			free(tracer.buffer.raw[i][2]);
			free(tracer.buffer.raw[i][3]);
			free(tracer.buffer.raw[i][4]);
			free(tracer.buffer.raw[i]);
		}
		free(tracer.buffer.raw);
	}
	memset(&tracer, 0x00, sizeof(struct tracer));
}

/* disasm_init(): Called at each breakpoint, update the internal buffer */
static void disasm_init(struct ucontext *context)
{
	int a;

	tracer.buffer.anchor.addr = (void*)(uintptr_t)((context->spc + 1) & ~1);
	tracer.next_break = context->spc;
	tracer.next_instruction = context->spc;

	tracer.buffer.cursor.note_idx = 0;
	tracer.buffer.cursor.line_idx = 0;
	a = disasm_util_line_update(tracer.buffer.cursor.line_idx, -1);
	tracer.buffer.raw[a][0][0] = '\0';
	tracer.buffer.raw[tracer.buffer.cursor.line_idx][0][0] = '\0';
	disasm_util_line_fetch(tracer.buffer.cursor.line_idx,
					&tracer.buffer.anchor.addr[0], 1);
}

/* disasm_display(); Display trace information */
static void disasm_display(struct ucontext *context)
{
	struct buffcursor cursor;
	int row;
	int pc;

	/* display the first part (current middle line and before, upward) */
	pc = 0;
	row = GUI_DISP_NB_ROW / 2;
	cursor.line_idx = tracer.buffer.cursor.line_idx;
	cursor.note_idx = tracer.buffer.cursor.note_idx;
	disasm_display_addr_info(&row, &cursor, context, -1, pc);
	while (row >= 0) {
		disasm_display_addr_info(&row, &cursor, context, -1, pc);
		cursor.line_idx = disasm_util_line_update(cursor.line_idx, -1);
		cursor.note_idx = 0;
		pc = pc - 1;
	}

	/* display the second part (after the current middle line, downward)
	 * @note:
	 *   I use a huge dirty workaround to skip the current middle line
	 *   because all the instruction is generated at the display_addr()
	 *   function. So, if the line is not generated, its notes will not be
	 *   displayed and skipped. So, we need to update the line, fetch
	 *   information, get the note index max then start displaying lines. */
	pc = 0;
	row = (GUI_DISP_NB_ROW / 2) + 1;
	cursor.line_idx = tracer.buffer.cursor.line_idx;
	cursor.note_idx = tracer.buffer.cursor.note_idx - 1;
	if (cursor.note_idx < 0) {
		pc = 1;
		cursor.line_idx = disasm_util_line_update(cursor.line_idx, 1);
		disasm_util_line_fetch(cursor.line_idx,
					&tracer.buffer.anchor.addr[pc], 1);
		cursor.note_idx = disasm_util_note_counter(cursor.line_idx);
	}
	while (row <= GUI_DISP_NB_ROW) {
		disasm_display_addr_info(&row, &cursor, context, 1, pc);
		pc = pc + 1;
		cursor.line_idx = disasm_util_line_update(cursor.line_idx, 1);
		disasm_util_line_fetch(cursor.line_idx,
					&tracer.buffer.anchor.addr[pc], 1);
		cursor.note_idx = disasm_util_note_counter(cursor.line_idx);
	}
}

/* disasm_keyboard(): Handle one key event */
static int disasm_keyboard(struct ucontext *context, int key)
{
	int note_idx;
	int line_idx;

	/* horizontal update */
	(void)context;
	if (key == KEY_LEFT)
		tracer.disp.hoffset += 1;
	if (key == KEY_RIGHT)
		tracer.disp.hoffset -= 1;

	/* vertical update */
	if (key == KEY_UP) {
		tracer.buffer.cursor.note_idx += 1;
		note_idx = tracer.buffer.cursor.note_idx;
		line_idx = tracer.buffer.cursor.line_idx;
		if (note_idx >= 5
		|| tracer.buffer.raw[line_idx][note_idx][0] == '\0') {
			tracer.buffer.anchor.addr =
						&tracer.buffer.anchor.addr[-1];
			line_idx = disasm_util_line_update(line_idx, -1);
			tracer.buffer.cursor.line_idx = line_idx;
			tracer.buffer.cursor.note_idx = 0;
		}
	}
	if (key == KEY_DOWN) {
		tracer.buffer.cursor.note_idx -= 1;
		note_idx = tracer.buffer.cursor.note_idx;
		line_idx = tracer.buffer.cursor.line_idx;
		if (note_idx < 0) {
			tracer.buffer.anchor.addr =
						&tracer.buffer.anchor.addr[1];
			line_idx = disasm_util_line_update(line_idx, 1);
			disasm_util_line_fetch(line_idx, tracer.memory, 1);
			note_idx = disasm_util_note_counter(line_idx);
			tracer.buffer.cursor.line_idx = line_idx;
			tracer.buffer.cursor.note_idx = note_idx;
		}
	}

	/* move break point */
	if (key == KEY_PLUS)
		tracer.next_break += 2;
	if (key == KEY_MINUS)
		tracer.next_break -= 2;
	if (key == KEY_NEG)
		tracer.next_break = (uintptr_t)tracer.buffer.anchor.addr;

	/* skip instruction */
	if (key == KEY_OPTN) {
		context->spc = tracer.next_break;
		return (1);
	}

	return(0);
}

/* disasm_command(): Handle user command */
static void disasm_command(struct ucontext *context, int argc, char **argv)
{
	uintptr_t address;
	uint8_t action;
	int idx;
	int i;

	/* argument check */
	if (argc == 0)
		return;
	action  = 0;
	action |= (strcmp(argv[0], "jmp") == 0) << 0;
	action |= (strcmp(argv[0], "syscall") == 0) << 1;
	if (action == 0) {
		input_write("unknown '%s' command", argv[0]);
		return;
	}
	if (argc < 2) {
		input_write("'%s': argument missing", argv[0]);
		return;
	}
	idx = 1;
	if ((action & 2) != 0 && argc != 2) {
		if (argv[1][0] != '-'
				|| argv[1][0] == 'j'
				|| argv[1][0] == '\0') {
			input_write("'syscall': arg '%s' unknown", argv[1]);
			return;
		}
		idx = 2;
	}

	/* handle arguments */
	i = -1;
	address = 0x00000000;
	while (++i < 8 && argv[idx][i] != '\0') {
		address = address << 4;
		if (argv[idx][i] >= '0' && argv[idx][i] <= '9') {
			address = address + argv[idx][i] - '0';
			continue;
		}
		if (argv[idx][i] >= 'a' && argv[idx][i] <= 'f') {
			address = address + argv[idx][i] - 'a' + 10;
			continue;
		}
		if (argv[idx][i] >= 'A' && argv[idx][i] <= 'F') {
			address = address + argv[idx][i] - 'A' + 10;
			continue;
		}
		goto error_part;
	}
	if (i > 8)
		goto error_part;

	/* action */
	if ((action & 2) != 0) {
		if (address >= 0x1fff)
			goto error_part;
#ifdef FXCG50
		uintptr_t *systab = *(uintptr_t **)0x8002007c;
#endif
#ifdef FX9860
		uintptr_t *systab = *(uintptr_t **)0x8001007c;
#endif
		if (idx == 2) {
			input_write("syscall %x: %p", address, systab[address]);
			return;
		}
		address = systab[address];
	}
	tracer.buffer.anchor.addr = (void*)(address & ~1);
	tracer.buffer.cursor.note_idx = 0;
	tracer.buffer.cursor.line_idx = 0;
	i = disasm_util_line_update(tracer.buffer.cursor.line_idx, -1);
	tracer.buffer.raw[i][0][0] = '\0';
	tracer.buffer.raw[tracer.buffer.cursor.line_idx][0][0] = '\0';
	disasm_util_line_fetch(tracer.buffer.cursor.line_idx,
					&tracer.buffer.anchor.addr[0], 1);
	disasm_display(context);
	return;

	/* error part */
error_part:
	input_write("'%s': '%s': argument error", argv[0], argv[idx]);
}

//---
// Define the menu
//---
struct menu menu_disasm = {
	.ctor     = &disasm_ctor,
	.init     = &disasm_init,
	.display  = &disasm_display,
	.keyboard = &disasm_keyboard,
	.command  = &disasm_command,
	.dtor     = &disasm_dtor
};
