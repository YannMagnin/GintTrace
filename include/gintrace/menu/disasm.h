#ifndef __GINTRACE_TRACER_H__
# define __GINTRACE_TRACER_H__

#include <stddef.h>
#include <stdint.h>

#include "gintrace/ubc.h"
#include "gintrace/gui/menu.h"

#include <gint/defs/types.h>
#include <gint/display.h>

/* buffcursor: buffer cursor (line + note) */
struct buffcursor {
	int line_idx;
	int note_idx;
};

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
		struct {
			off_t line_idx;
			uint16_t *addr;
		} anchor;
		struct buffcursor cursor;
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
	int skip;
};

/* extern menu information */
extern struct menu menu_disasm;

#endif /*__GINTRACE_TRACER_H__*/
