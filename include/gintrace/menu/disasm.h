#ifndef __GINTRACE_TRACER_H__
# define __GINTRACE_TRACER_H__

#include <stddef.h>
#include <stdint.h>

#include "gintrace/ubc.h"
#include "gintrace/gui/menu.h"

#include <gint/defs/types.h>
#include <gint/display.h>

/* define font information */
#ifdef FXCG50
#define FWIDTH	8
#define FHEIGHT	9
#endif
#ifdef FX9860G
#define FWIDTH	5
#define FHEIGHT	7
#endif

/* define display information */
#define DISASM_NB_COLUMN	(DWIDTH / (FWIDTH + 1))
#define DISASM_NB_ROW		(DHEIGHT / (FHEIGHT + 1))

/* tracer: internal information used to display disassembly view */
struct tracer {
	/* circular buffer information.
	 * Note that the circular buffer is very special, it refert, on the
	 * first level, the "instruction address" then, the second level its
	 * instruction note and mnemonic.
	 *
	 * TODO: doc */
	struct {
		char ***raw;
		struct {
			size_t width;
			size_t height;
		} size;
		off_t cursor;
		uint16_t *anchor;
	} buffer;

	/* display offset */
	struct {
		int hoffset;
		int voffset;
	} disp;

	/* memory information */
	uint16_t *memory;
	uintptr_t next_break;
	uintptr_t next_instruction;
	uintptr_t spc;
};

/* extern menu information */
extern struct menu menu_disasm;

#endif /*__GINTRACE_TRACER_H__*/
