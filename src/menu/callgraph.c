#include "gintrace/menu/callgraph.h"
#include "gintrace/ubc.h"
#include "gintrace/gui/menu.h"
#include "gintrace/gui/display.h"

#include <gint/std/stdio.h>
#include <gint/std/stdlib.h>
#include <gint/std/string.h>
#include <gint/keyboard.h>
#include <gint/display.h>

#include "./src/menu/internal/dictionary.h"

/* define the menu information */
/* TODO: find a way to have local information (session) */
struct callgraph callgraph;

//---
// callode management
//---
/* callnode_create(): create a new callnode */
static struct callnode *callnode_create(struct callnode *parent,
					struct ucontext *context,
					int type, uintptr_t address)
{
	struct callnode *node;

	node = calloc(sizeof(struct callnode), 1);
	if (node != NULL) {
		memcpy(&node->context, context, sizeof(struct ucontext));
		node->type = type;
		node->parent = parent;
		node->address = address;
		callgraph.callnode_counter += 1;
	}
	return (node);
}

/* callnode_add_child(): Add child into the child queue */
static void callnode_add_child(struct callnode *parent, struct callnode *child)
{
	struct callnode **sibling;

	sibling = &parent->child;
	while (*sibling != NULL)
		sibling = &(*sibling)->sibling;
	*sibling = child;
}

/* callnode_display(): Display callnode information */
static void callnode_display(struct callnode *node, uint32_t bitmap[4],
							int *row, int depth)
{
	static char line[256];
	char shift;
	int idx;
	int i;

	if (node == NULL || *row + callgraph.cursor.voffset >= GUI_DISP_NB_ROW)
		return;
	/* handle the bitmap (ugly / 20) */
	i = -1;
	idx = 0;
	while (++i < depth) {
		idx = i >> 6;
		if (idx >= 4)
			break;
		shift = i & 0x1f;
		if ((bitmap[idx] & (1 << shift)) != 0
				&& *row + callgraph.cursor.voffset >= 0) {
			dtext((2 + callgraph.cursor.hoffset + (i << 2))
				* (FWIDTH + 1), (*row
				+ callgraph.cursor.voffset)
				* (FHEIGHT + 1), C_BLACK, "|   ");
		}
	}

	/* generate the line */
	char pipe  = '|';
	const char *type = "(err)";
	bitmap[idx] |= 1 << (depth & 0x1f);
	if (node->type == callnode_type_root)
		type = "(root)";
	if (node->type == callnode_type_bsr)
		type = "(bsr)";
	if (node->type == callnode_type_bsrf)
		type = "(bsrf)";
	if (node->type == callnode_type_jsr)
		type = "(jsr)";
	if (node->type == callnode_type_jmp) {
		bitmap[idx] &= ~(1 << (depth & 0x1f));
		type = "(jmp)";
		pipe = '`';
	}
	if (node->type == callnode_type_rte) {
		bitmap[idx] &= ~(1 << (depth & 0x1f));
		type = "(rte)";
		pipe = '`';
	}
	if (node->type == callnode_type_rts) {
		bitmap[idx] &= ~(1 << (depth & 0x1f));
		type = "(rts)";
		pipe = '`';
	}

	/* skip display part */
	if (*row + callgraph.cursor.voffset < 0) {
		*row = *row + 1;
		callnode_display(node->child, bitmap, row, depth + 1);
		callnode_display(node->sibling, bitmap, row, depth);
		return;
	}


	const char *addrname = dictionary_syscalls_check((void*)node->address);
	if (addrname == NULL)
		addrname = dictionary_notes_check((void*)node->address);
	if (addrname == NULL) {
		snprintf(line, 256, "%s %p", type, (void*)node->address);
	} else {
		snprintf(line, 256, "%s %p - %s",
				type, (void*)node->address, addrname);
	}

	/* display the line then check child and siblig */
	if (depth < 0) {
		dtext((callgraph.cursor.hoffset + (i << 2)) * (FWIDTH + 1),
		(*row + callgraph.cursor.voffset) * (FHEIGHT + 1),
								C_BLACK, line);
	} else {
		dprint((2 + callgraph.cursor.hoffset + (i << 2)) * (FWIDTH + 1),
		(*row + callgraph.cursor.voffset) * (FHEIGHT + 1), C_BLACK,
							"%c-- %s", pipe, line);
	}
	*row = *row + 1;
	callnode_display(node->child, bitmap, row, depth + 1);
	callnode_display(node->sibling, bitmap, row, depth);
}

//---
// Menu
//---

/* callgraph_ctor: Menu constructor */
static void callgraph_ctor(void)
{
	callgraph.cursor.hoffset = 0;
	callgraph.cursor.voffset = 0;
	callgraph.root = NULL;
}

/* callgraph_init(): Invoked each time a break point occur */
/* FIXME: recursive !! */
static void callgraph_init(struct ucontext *context)
{
	struct callnode *node;
	uintptr_t address;
	uint16_t *pc;
	uint32_t b;
	int type;

	/* check root node */
	if (callgraph.root == NULL) {
		node = callnode_create(NULL, context,
				callnode_type_root, context->spc);
		if (node == NULL)
			return;
		callgraph.root = node;
		callgraph.parent = callgraph.root;
	}

	/* check error */
	if (callgraph.parent == NULL)
		return;

	/* check opcode */
	type = -1;
	pc = (void*)(uintptr_t)context->spc;
	if ((pc[0] & 0xf000) == 0xb000) {
		type = callnode_type_bsr;
		b = pc[0] & 0x0fff;
		if ((b & 0x800) != 0)
			b = (0xfffff000 | b);
		address = context->spc + 4 + (b << 1);
	}
	if ((pc[0] & 0xf0ff) == 0x0003) {
		type = callnode_type_bsrf;
		b = (pc[0] & 0x0f00) >> 8;
		address = context->reg[b];
	}
	if ((pc[0] & 0xf0ff) == 0x400b) {
		type = callnode_type_jsr;
		b = (pc[0] & 0x0f00) >> 8;
		address = context->reg[b];
	}
	if ((pc[0] & 0xf0ff) == 0x402b) {
		type = callnode_type_jmp;
		b = (pc[0] & 0x0f00) >> 8;
		address = context->reg[b];
	}
	if ((pc[0] & 0xffff) == 0x002b) {
		type = callnode_type_rte;
		address = context->spc;
	}
	if ((pc[0] & 0xffff) == 0x000b) {
		type = callnode_type_rts;
		address = context->spc;
	}
	if (type == -1)
		return;

	/* generate the node then link with its sibling */
	node = callnode_create(callgraph.parent, context, type, address);
	if (node == NULL)
		return;
	callnode_add_child(callgraph.parent, node);

	/* find the next "current" node */
	if (node->type == callnode_type_bsr
			|| node->type == callnode_type_jsr
			|| node->type == callnode_type_bsrf
			|| node->type == callnode_type_jmp) {
		callgraph.parent = node;
		return;
	}
	while (callgraph.parent != NULL) {
		if (callgraph.parent->type == callnode_type_jmp) {
			callgraph.parent = callgraph.parent->parent;
			continue;
		}
		callgraph.parent = callgraph.parent->parent;
		return;
	}
}

/* callgraph_display(); Display trace information */
static void callgraph_display(struct ucontext *context)
{
	uint32_t bitmap[4];
	int row;

	(void)context;
	row = 0;
	memset(bitmap, 0x00, sizeof(bitmap));
	callnode_display(callgraph.root, bitmap, &row, -1);
}

/* callgraph_keyboard(): Handle one key event */
/* TODO: remove the cursor, handle node selection ! */
static int callgraph_keyboard(struct ucontext *context, int key)
{
	(void)context;
	if (key == KEY_LEFT)
		callgraph.cursor.hoffset += 1;
	if (key == KEY_RIGHT)
		callgraph.cursor.hoffset -= 1;

	if (key == KEY_UP)
		callgraph.cursor.voffset += 1;
	if (key == KEY_DOWN)
		callgraph.cursor.voffset -= 1;

	return (0);
}

//---
// Define the menu
//---
struct menu menu_callgraph = {
	.ctor     = &callgraph_ctor,
	.init     = &callgraph_init,
	.display  = &callgraph_display,
	.keyboard = &callgraph_keyboard,
	.command  = NULL,
	.dtor     = NULL
};
