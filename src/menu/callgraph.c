#include "gintrace/menu/callgraph.h"
#include "gintrace/ubc.h"
#include "gintrace/tracer.h"
#include "gintrace/gui/menu.h"
#include "gintrace/gui/display.h"
#include "gintrace/gui/input.h"

#include <gint/std/stdio.h>
#include <gint/std/stdlib.h>
#include <gint/std/string.h>
#include <gint/keyboard.h>
#include <gint/bfile.h>
#include <gint/gint.h>

#include "./src/menu/internal/dictionary.h"

//---
// callode management
//---
/* callnode_create(): create a new callnode */
static struct callnode *callnode_create(struct callnode *parent,
		struct ucontext *context, int type, uintptr_t address)
{
	struct callnode *node;

	node = calloc(sizeof(struct callnode), 1);
	if (node != NULL) {
		memcpy(&node->context, context, sizeof(struct ucontext));
		node->type = type;
		node->parent = parent;
		node->address = address;
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

/* callnode_generate_info(): Genera information about a node */
static size_t callnode_generate_info(char *buf,
					size_t max, struct callnode *node)
{
	const char *addrname;
	const char *type;

	/* generate the line */
	type = "(err)";
	if (node->type == callnode_type_root)
		type = "(root)";
	if (node->type == callnode_type_bsr)
		type = "(bsr)";
	if (node->type == callnode_type_bsrf)
		type = "(bsrf)";
	if (node->type == callnode_type_jsr)
		type = "(jsr)";
	if (node->type == callnode_type_jmp)
		type = "(jmp)";
	if (node->type == callnode_type_rte)
		type = "(rte)";
	if (node->type == callnode_type_rts)
		type = "(rts)";

	/* check special address */
	addrname = dictionary_syscalls_check((void*)node->address);
	if (addrname == NULL)
		addrname = dictionary_notes_check((void*)node->address);
	if (addrname == NULL)
		return(snprintf(buf, max, "%s %p", type, (void*)node->address));
	return (snprintf(buf, max, "%s %p - %s",
				type, (void*)node->address, addrname));
}

/* callnode_get_size(): Count the number of bytes that the callgraph take */
static size_t callnode_export(int fd, struct callnode *node, char line[])
{
	if (node == NULL)
		return (0);

	size_t size = 0;
	size += sizeof(size_t);
	size += sizeof(uintptr_t);
	size += sizeof(struct callnode);
	size += callnode_generate_info(line, 256, node);
	if (fd >= 0) {
		BFile_Write(fd, &size, sizeof(size));
		BFile_Write(fd, &node, sizeof(&node));
		BFile_Write(fd, node, sizeof(struct callnode));
		BFile_Write(fd, line, size);
	}

	/* check other node */
	size += callnode_export(fd, node->child, line);
	size += callnode_export(fd, node->sibling, line);
	return (size);
}

/* callnode_display(): Display callnode information */
struct cdinfo {
	struct callgraph *callgraph;
	uint32_t bitmap[4];
	int row;
};
static void callnode_display(struct callnode *node,
				struct cdinfo *info, int depth, char line[])
{
	char shift;
	char pipe;
	int idx;
	int i;
	int x;
	int y;

	if (node == NULL
	|| info->row + info->callgraph->cursor.voffset >= GUI_DISP_NB_ROW) {
		return;
	}

	/* handle the bitmap (ugly / 20) */
	i = -1;
	idx = 0;
	while (++i < depth) {
		idx = i >> 6;
		if (idx >= 4)
			break;
		shift = i & 0x1f;
		if ((info->bitmap[idx] & (1 << shift)) != 0
		&& info->row + info->callgraph->cursor.voffset >= 0) {
			x = info->callgraph->cursor.hoffset + (i << 2) + 2;
			y = info->callgraph->cursor.voffset + info->row;
			gtextXY(x, y, "|   ");
		}
	}

	/* generate the line */
	pipe  = '|';
	info->bitmap[idx] |= 1 << (depth & 0x1f);
	if (node->type == callnode_type_jmp
			|| node->type == callnode_type_rte
			|| node->type == callnode_type_rts) {
		info->bitmap[idx] &= ~(1 << (depth & 0x1f));
		pipe = '`';
	}

	/* display the line then check child and siblig */
	if (info->row + info->callgraph->cursor.voffset >= 0) {
		callnode_generate_info(line, 256, node);
		if (depth < 0) {
			x = info->callgraph->cursor.hoffset + (i << 2);
			y = info->callgraph->cursor.voffset + info->row;
			gtextXY(x, y, line);
		} else {
			x = info->callgraph->cursor.hoffset + (i << 2) + 2;
			y = info->callgraph->cursor.voffset + info->row;
			gprintXY(x, y, "%c-- %s", pipe, line);
		}
	}
	info->row = info->row + 1;
	callnode_display(node->child, info, depth + 1, line);
	callnode_display(node->sibling, info, depth, line);
}

//---
// Menu
//---

/* callgraph_ctor: Menu constructor */
static void callgraph_ctor(struct tsession *session)
{
	session->menu.callgraph.cursor.hoffset = 0;
	session->menu.callgraph.cursor.voffset = 0;
	session->menu.callgraph.root = NULL;
}

/* callgraph_init(): Invoked each time a break point occur */
static void callgraph_init(struct tsession *session)
{
	struct callnode *node;
	uintptr_t address;
	uint16_t *pc;
	uint32_t b;
	int type;

	/* check root node */
	if (session->menu.callgraph.root == NULL) {
		node = callnode_create(NULL, session->info.context,
				callnode_type_root, session->info.context->spc);
		if (node == NULL)
			return;
		session->menu.callgraph.root = node;
		session->menu.callgraph.parent = session->menu.callgraph.root;
	}

	/* check error */
	if (session->menu.callgraph.parent == NULL)
		return;

	/* check opcode */
	type = -1;
	pc = (void*)(uintptr_t)session->info.context->spc;
	if ((pc[0] & 0xf000) == 0xb000) {
		type = callnode_type_bsr;
		b = pc[0] & 0x0fff;
		if ((b & 0x800) != 0)
			b = (0xfffff000 | b);
		address = session->info.context->spc + 4 + (b << 1);
	}
	if ((pc[0] & 0xf0ff) == 0x0003) {
		type = callnode_type_bsrf;
		b = (pc[0] & 0x0f00) >> 8;
		address = session->info.context->reg[b];
	}
	if ((pc[0] & 0xf0ff) == 0x400b) {
		type = callnode_type_jsr;
		b = (pc[0] & 0x0f00) >> 8;
		address = session->info.context->reg[b];
	}
	if ((pc[0] & 0xf0ff) == 0x402b) {
		type = callnode_type_jmp;
		b = (pc[0] & 0x0f00) >> 8;
		address = session->info.context->reg[b];
	}
	if ((pc[0] & 0xffff) == 0x002b) {
		type = callnode_type_rte;
		address = session->info.context->spc;
	}
	if ((pc[0] & 0xffff) == 0x000b) {
		type = callnode_type_rts;
		address = session->info.context->spc;
	}
	if (type == -1)
		return;

	/* generate the node then link with its sibling */
	node = callnode_create(session->menu.callgraph.parent,
					session->info.context, type, address);
	if (node == NULL)
		return;
	callnode_add_child(session->menu.callgraph.parent, node);

	/* find the next "current" node */
	if (node->type == callnode_type_bsr
			|| node->type == callnode_type_jsr
			|| node->type == callnode_type_bsrf
			|| node->type == callnode_type_jmp) {
		session->menu.callgraph.parent = node;
		return;
	}
	while (session->menu.callgraph.parent != NULL) {
		if (session->menu.callgraph.parent->type == callnode_type_jmp){
			session->menu.callgraph.parent =
				session->menu.callgraph.parent->parent;
			continue;
		}
		session->menu.callgraph.parent =
			session->menu.callgraph.parent->parent;
		return;
	}
}

/* callgraph_display(); Display trace information */
static void callgraph_display(struct tsession *session)
{
	struct cdinfo info;
	char line[256];

	memset(&info, 0x00, sizeof(info));
	info.callgraph = &session->menu.callgraph;
	callnode_display(session->menu.callgraph.root, &info, -1, line);
}

/* callgraph_keyboard(): Handle one key event */
static int callgraph_keyboard(struct tsession *session, int key)
{
	if (key == KEY_LEFT)
		session->menu.callgraph.cursor.hoffset += 1;
	if (key == KEY_RIGHT)
		session->menu.callgraph.cursor.hoffset -= 1;

	if (key == KEY_UP)
		session->menu.callgraph.cursor.voffset += 1;
	if (key == KEY_DOWN)
		session->menu.callgraph.cursor.voffset -= 1;

	return (0);
}

/* callgraph_command(): handle user command */
static void callgraph_command(struct tsession *session, int argc, char **argv)
{
	/* check useless export */
	if (session->menu.callgraph.root == NULL) {
		input_write("nothing to export");
		return;
	}

	/* check argument validity */
	if (argc != 2) {
		input_write("argument missing");
		return;
	}
	if (strcmp(argv[0], "export") != 0) {
		input_write("'%s': command unknown", argv[0]);
		return;
	}

	/* convert the filename (arg2) into Bfile pathname */
	int i = -1;
	uint16_t pathname[14 + strlen(argv[1]) + 1];
	memcpy(pathname, u"\\\\fls0\\", 14);
	while (argv[1][++i] != '\0')
		pathname[7 + i] = argv[1][i];
	pathname[7 + i] = 0x0000;

	/* check if the file exist */
	input_write_noint("Check if the file exist");
	gint_switch_to_casio();
	char line[256];
	int fd = BFile_Open(pathname, BFile_ReadOnly);
	if (fd >= 0) {
		gint_switch_to_gint();
		while (1) {
			if (input_read(line, 3, "'%s' exist, erase ? [n/Y]:",
							argv[1]) <= 0) {
				input_write("export aborded");
				return;
			}
			if (line[0] == 'n') {
				input_write("export aborded");
				return;
			}
			if (line[0] != 'Y')
				continue;
			gint_switch_to_casio();
			BFile_Remove(pathname);
			break;
		}
	}

	/* create the file then dump information */
	gint_switch_to_gint();
	int size = callnode_export(-1, session->menu.callgraph.root, line);
	input_write_noint("Create the file  (%d)", size);
	gint_switch_to_casio();
	fd = BFile_Create(pathname, BFile_File, &size);
	if (fd != 0) {
		gint_switch_to_gint();
		input_write("Bfile_Create: error %d", fd);
		return;
	}
	gint_switch_to_gint();
	input_write_noint("Create success");
	gint_switch_to_casio();
	fd = BFile_Open(pathname, BFile_ReadWrite);
	if (fd < 0) {
		BFile_Remove(pathname);
		gint_switch_to_gint();
		input_write("BFile_Open: error %d", fd);
		return;
	}
	gint_switch_to_gint();
	input_write_noint("Open success, now write...");
	gint_switch_to_casio();
	callnode_export(fd, session->menu.callgraph.root, line);
	BFile_Close(fd);
	gint_switch_to_gint();
	input_write("success");
}

//---
// Define the menu
//---
struct menu menu_callgraph = {
	.ctor     = (void*)&callgraph_ctor,
	.init     = (void*)&callgraph_init,
	.display  = (void*)&callgraph_display,
	.keyboard = (void*)&callgraph_keyboard,
	.command  = (void*)&callgraph_command,
	.dtor     = NULL
};
