#include "gintrace/menu/disasm.h"
#include "gintrace/ubc.h"

#include "./src/menu/disassembler/dictionary.h"

#include <gint/std/string.h>
#include <gint/std/stdlib.h>
#include <gint/std/stdio.h>
#include <gint/display.h>
#include <gint/keyboard.h>

/* define the menu information */
/* TODO: find way to have local information */
struct tracer tracer;

//---
// Internal disassembler processing
//---
/* dhreverse(): Reverse horizotal area (TODO: move me !!) */
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


/* disasm_update_index(): Little helper to update the circular index */
static int disasm_update_index(int index, int direction)
{
	/* update the counter */
	if (direction > 0)
		index = index - 1;
	if (direction < 0)
		index = index + 1;

	/* check border */
	if (index < 0)
		index = (int)tracer.buffer.size.height - 1;
	if (index >= (int)tracer.buffer.size.height)
		index = 0;

	/* return the index */
	return (index);
}

/* disasm_check_special_addr(): Check if the address is special
 *
 * @note:
 * mode & 1 -> check hardware module register
 * mode & 2 -> check syscall address */
static void disasm_check_special_addr(char *buffer, void *address, int mode)
{
	extern struct tracer tracer;
	const char *addrname;

	addrname = NULL;
	if ((mode & 1) != 0)
		addrname = disasm_dictionary_check_peripheral(address);
	if (addrname == NULL && (mode & 2) != 0)
		addrname = disasm_dictionary_check_syscalls(address);
	if (addrname == NULL)
		return;
	snprintf(buffer, tracer.buffer.size.width,
			"@note: %p -> %s\n", address, addrname);
}

/* disasm_get_mnemonic_info(): Get mnemonic information
 *
 * @note:
 * line[0] -> "address opcode opcode_name"
 * line[1] -> opcode address note
 * line[2] -> first note (arg0)
 * line[3] -> first note (arg1)
 * line[4] -> first note (arg2) */
static void disasm_get_mnemonic_info(int line, uint16_t *pc)
{
	const struct opcode *opcode_info;
	uint16_t opcode;
	uintptr_t arg[3];

	/* Wipe note */
	tracer.buffer.raw[line][0][0] = '\0';
	tracer.buffer.raw[line][1][0] = '\0';
	tracer.buffer.raw[line][2][0] = '\0';
	tracer.buffer.raw[line][3][0] = '\0';
	tracer.buffer.raw[line][4][0] = '\0';

	/* check special address (register, syscall, ...) */
	disasm_check_special_addr(tracer.buffer.raw[line][1], pc, 2);

	/* generate the "default" information (only the address and word) */
	snprintf(&tracer.buffer.raw[line][0][0],
			tracer.buffer.size.width, "%.8lx %.4x ",
			(uintptr_t)pc, (unsigned int)pc[0]);

	/* Try to get the current opcode */
	opcode = pc[0];
	opcode_info = disasm_dictionary_check_opcode(opcode);
	if (opcode_info == NULL) {
		snprintf(&tracer.buffer.raw[line][0][14],
				tracer.buffer.size.width - 14,
				".word\t%#.4x", (int)opcode);
		return;
	}

	/* get mnemonic arguments, if available */
	for (int i = 0 ; i < 3 ; ++i) {
		arg[i] = disasm_dictionary_opcode_get_arg(opcode_info,
								opcode, i, pc);
		if (arg[i] == 0x00000000)
			continue;
		disasm_check_special_addr(
				tracer.buffer.raw[line][2 + i],
				(void*)arg[i], 3
		);
	}

	/* generate the complet mnemonic information */
	snprintf(&tracer.buffer.raw[line][0][14],
			tracer.buffer.size.width - 14,
			opcode_info->name, arg[0], arg[1], arg[2]);
}

/* disasm_buffer_update(): Update the internal display buffer information */
static void disasm_buffer_update(void)
{
	int direction;
	size_t gap;
	int line;
	int pc;

	/* first, we need to calculate the gaps between the current buffer's
	 * based anchor and the new memory information, then determine the
	 * direction */
	direction = (intptr_t)(tracer.buffer.anchor - tracer.memory);
	gap = (direction < 0) ? 0 - direction : direction;
	if (gap > tracer.buffer.size.height) {
		gap = tracer.buffer.size.height;
		direction = -tracer.buffer.size.height;
	}
	if (gap == 0)
		return;

	/* update the virtual PC, used with the internal memory information.
	 * Note that we reach */
	pc = gap - 1;
	if (direction < 0)
		pc = tracer.buffer.size.height - gap;

	/* get the start line index */
	line = tracer.buffer.cursor;
	if (direction > 0)
		line = disasm_update_index(tracer.buffer.cursor, direction);

	/* update the internal buffer */
	tracer.buffer.anchor = tracer.memory;
	while ((signed)--gap >= 0) {
		disasm_get_mnemonic_info(line, &tracer.buffer.anchor[pc]);
		if (direction > 0)
			tracer.buffer.cursor = line;
		line = disasm_update_index(line, direction);
		pc = (direction < 0) ? pc + 1 : pc - 1;
	}
	if (direction < 0)
		tracer.buffer.cursor = line;
}




//---
// Menu functions
//---
/* disasm_ctor(): disasm menu constructor */
static void disasm_ctor(void)
{
	memset(&tracer, 0x00, sizeof(struct tracer));
	tracer.buffer.size.width  = DISASM_NB_COLUMN * 2;
	tracer.buffer.size.height = DISASM_NB_ROW * 2;
	tracer.buffer.raw = calloc(sizeof(void*), tracer.buffer.size.height);
	for (size_t i = 0; i < tracer.buffer.size.height; ++i) {
		tracer.buffer.raw[i] = calloc(sizeof(char*), 5);
		tracer.buffer.raw[i][0] = calloc(tracer.buffer.size.width, 1);
		tracer.buffer.raw[i][1] = calloc(tracer.buffer.size.width, 1);
		tracer.buffer.raw[i][2] = calloc(tracer.buffer.size.width, 1);
		tracer.buffer.raw[i][3] = calloc(tracer.buffer.size.width, 1);
		tracer.buffer.raw[i][4] = calloc(tracer.buffer.size.width, 1);
	}
	tracer.buffer.cursor = 0;
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

/* disasm_init(): Called each breakpoint, update the internal buffer */
static void disasm_init(struct ucontext *context)
{
	tracer.memory = (void*)(uintptr_t)((context->spc + 3) & ~3);
	tracer.next_break = context->spc;
	tracer.next_instruction = context->spc;
	tracer.spc = context->spc;
	disasm_buffer_update();
}

/* disasm_display(); Display trace information */
static void disasm_display(struct ucontext *context)
{
	int line;
	int pc;

	pc = 0;
	line = tracer.buffer.cursor;
	for (int row = 0; row < DISASM_NB_ROW; ++row) {
		/* display notes and opcode information */
		for (int i = 0; i < 4; ++i) {
			if (tracer.buffer.raw[line][i + 1][0] == '\0')
				continue;
			drect(0, row * (FHEIGHT + 1) - 1, DWIDTH,
				(row * (FHEIGHT + 1) - 1) + (FHEIGHT + 2),
				0x556655);
			dtext(tracer.disp.hoffset * (FWIDTH + 1),
					row * (FHEIGHT + 1), C_WHITE,
					tracer.buffer.raw[line][i + 1]);
			row = row + 1;
			if (row >= DISASM_NB_ROW)
				return;
		}
		dtext(tracer.disp.hoffset * (FWIDTH + 1),
				row * (FHEIGHT + 1), C_BLACK,
				tracer.buffer.raw[line][0]);

		/* highlight SPC if possible */
		if ((uintptr_t)&tracer.buffer.anchor[pc] == context->spc)
			dhreverse(row * (FHEIGHT + 1) - 1, FHEIGHT + 2);

		/* draw next break / instruction */
		if (tracer.next_break != context->spc
		&& (uintptr_t)&tracer.buffer.anchor[pc] == tracer.next_break) {
			dhline(row * (FHEIGHT + 1) - 1, C_BLACK);
			dhline((row + 1) * (FHEIGHT + 1) - 1, C_BLACK);
		}

		/* update line position (circular buffer) */
		line = line + 1;
		if (line >= (int)tracer.buffer.size.height)
			line = 0;
		pc = pc + 1;
	}
}

/* disasm_keyboard(): Handle one key event */
static int disasm_keyboard(struct ucontext *context, int key)
{
	/* horizontal update */
	if (key == KEY_LEFT)
		tracer.disp.hoffset += 1;
	if (key == KEY_RIGHT)
		tracer.disp.hoffset -= 1;

	/* vertical update */
	if (key == KEY_UP) {
		tracer.memory = &tracer.memory[-1];
		disasm_buffer_update();
	}
	if (key == KEY_DOWN) {
		tracer.memory = &tracer.memory[1];
		disasm_buffer_update();
	}

	/* move break point */
	if (key == KEY_PLUS)
		tracer.next_break += 2;
	if (key == KEY_MINUS)
		tracer.next_break -= 2;

	/* skip instruction */
	if (key == KEY_OPTN) {
		context->spc = tracer.next_break;
		ubc_set_breakpoint(0, (void*)tracer.next_break, NULL);
		return (1);
	}
	/* go to net break point */
	if (key == KEY_EXE) {
		ubc_set_breakpoint(0, (void*)tracer.next_break, NULL);
		return (1);
	}
	return(0);
}



//---
// Define the menu
//---
struct menu menu_disasm = {
	.ctor     = &disasm_ctor,
	.init     = &disasm_init,
	.display  = &disasm_display,
	.keyboard = &disasm_keyboard,
	.command  = NULL,
	.dtor     = &disasm_dtor
};
