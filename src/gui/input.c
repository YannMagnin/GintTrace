#include "gintrace/gui/input.h"
#include "gintrace/gui/display.h"

#include <gint/std/stdio.h>
#include <gint/std/string.h>
#include <gint/defs/types.h>
#include <gint/keyboard.h>
#include <gint/display.h>
#include <gint/timer.h>

#include <stdarg.h>

/* internal structure used to store many information */
struct {
	struct {
		int visible;
	} cursor;
	struct {
		uint8_t alpha:	1;
		uint8_t shift:	1;
		uint8_t ctrl:	1;
		uint8_t exit:	1;
		uint8_t const:	4;
	} mode;
	struct {
		size_t max;
		size_t size;
		off_t cursor;
		char *data;
	} buffer;
} input_info;

//---
// Display management
//---
/* input_display(): Display the input area */
void input_display(void)
{
	int cursor_x;
	int cursor_y;
	uint16_t tmp;
	int cursor;
	int x;
	int y;

	/* add cursor mark */
	input_info.buffer.data[input_info.buffer.cursor] |= 0x80;

	/* display part */
	cursor_x = 0;
	cursor_y = GUI_DISP_NB_ROW - 1;
	cursor_y -= input_info.buffer.size / GUI_DISP_NB_COLUMN;
	drect(0, cursor_y * (FHEIGHT + 1) - 1, DWIDTH, DHEIGHT, C_WHITE);
	dhline(cursor_y * (FHEIGHT + 1) - 3, C_BLACK);
	dhline(cursor_y * (FHEIGHT + 1) - 2, C_BLACK);
	for (size_t i = 0; i < input_info.buffer.size; ++i) {
		/* get the cursor and remove the potential cursor marker */
		cursor = ((input_info.buffer.data[i] & 0x80) != 0);
		input_info.buffer.data[i] &= 0x7f;

		/* display part (character + cursor if needed) */
		x = cursor_x * (FWIDTH  + 1);
		y = cursor_y * (FHEIGHT + 1);
		tmp = input_info.buffer.data[i] << 8;
		dtext(x, y, C_BLACK, (void*)&tmp);
		if (cursor != 0) {
			dline(x, y + (FHEIGHT + 1), x + (FWIDTH  + 1) - 2,
						y + (FHEIGHT + 1), C_BLACK);
		}

		/* update cursor if needed */
		cursor_x += 1;
		if (cursor_x >= GUI_DISP_NB_COLUMN) {
			cursor_x  = 0;
			cursor_y += 1;
		}
	}
	dupdate();

	/* remove cursor mark */
	input_info.buffer.data[input_info.buffer.cursor] &= ~0x80;
}

//---
// Buffer management
//---
/* input_buffer_remove(): Remove character based on current cursor position */
static void input_buffer_remove(void)
{
	/* check if this action is possible */
	if (input_info.buffer.cursor == 0)
		return;

	/* move data if needed */
	if (input_info.buffer.cursor < input_info.buffer.size - 1) {
		memcpy(
			&input_info.buffer.data[input_info.buffer.cursor - 1],
			&input_info.buffer.data[input_info.buffer.cursor],
			input_info.buffer.size - input_info.buffer.cursor
		);
	}
	/* force NULL-char and update cursor/size */
	input_info.buffer.cursor = input_info.buffer.cursor - 1;
	input_info.buffer.data[--input_info.buffer.size - 1] = '\0';
}

/* input_buffer_insert() - Insert character based on current cursor position */
static int input_buffer_insert(char n)
{
	/* save space for the "\n\0" (EOL) */
	if (input_info.buffer.size + 1 >= input_info.buffer.max)
		return (-1);

	/* move data if needed */
	if (input_info.buffer.cursor < input_info.buffer.size - 1) {
		off_t i = input_info.buffer.size + 1;
		while (--i >= input_info.buffer.cursor) {
			input_info.buffer.data[i] =
						input_info.buffer.data[i - 1];
		}
	}
	/* insert the character and force NULL-char */
	input_info.buffer.data[input_info.buffer.cursor++] = n;
	input_info.buffer.data[++input_info.buffer.size] = '\0';
	return (0);
}

//---
// Key management
//---
/* input_key_handle_special(): Handle special key */
static int input_key_handle_special(key_event_t key_event)
{
	switch (key_event.key) {
	case KEY_SHIFT:	input_info.mode.shift ^= 1; return (1);
	case KEY_ALPHA:	input_info.mode.alpha ^= 1; return (1);
	case KEY_OPTN:	input_info.mode.ctrl  ^= 1; return (1);
	case KEY_DOT:	input_buffer_insert(' ');   return (1);
	case KEY_DEL:	input_buffer_remove();      return (1);
	case KEY_EXIT:
		input_info.mode.exit = 1;
		return (-1);
	case KEY_EXE:
		/* Add End Of Line character */
		input_info.buffer.data[input_info.buffer.size - 1] = '\n';
		input_info.buffer.data[input_info.buffer.size] = '\0';

		/* indicate that the EXE key has been pressed. */
		input_info.mode.exit = 1;
		return (1);
	case KEY_LEFT:
		if (input_info.buffer.cursor > 0)
			input_info.buffer.cursor -= 1;
	      	return (1);
	case KEY_RIGHT:
		if (input_info.buffer.cursor < input_info.buffer.size - 1)
			input_info.buffer.cursor += 1;
		return (1);
	default:
			return (0);
	}
}

/* input_buffer_update() - Update the internal buffer with the given key code*/
static int input_key_buffer_update(key_event_t key_event)
{
	static const uint8_t keylist_alpha[] = {
		KEY_XOT, KEY_LOG, KEY_LN, KEY_SIN, KEY_COS, KEY_TAN,
		KEY_FRAC, KEY_FD, KEY_LEFTP, KEY_RIGHTP, KEY_COMMA, KEY_ARROW,
		KEY_7, KEY_8, KEY_9,
		KEY_4, KEY_5, KEY_6, KEY_MUL, KEY_DIV,
		KEY_1, KEY_2, KEY_3, KEY_PLUS, KEY_MINUS,
		KEY_0, 0xff
	};
	static const uint8_t keylist_num[] = {
		KEY_0, KEY_1, KEY_2, KEY_3, KEY_4,
		KEY_5, KEY_6, KEY_7, KEY_8, KEY_9,
		KEY_PLUS, KEY_MINUS, KEY_MUL, KEY_DIV,
		KEY_LEFTP, KEY_RIGHTP, KEY_COMMA, KEY_POWER,
		KEY_DOT, KEY_FD, KEY_ARROW, 0xff
	};
	static const char keylist_num_char[] = "0123456789+-x/(),^.|_";
	const uint8_t *keycode_list;
	char character;
	int i;

	/* Get the appropriate key list. */
	keycode_list = keylist_alpha;
	if (input_info.mode.shift == 1)
		keycode_list = keylist_num;

	/* Try to find the pressed key. */
	i = -1;
	while (keycode_list[++i] != 0xff && keycode_list[i] != key_event.key);
	if (keycode_list[i] != key_event.key)
		return (0);

	/* handle mode then update the buffer */
	if (input_info.mode.shift == 0) {
		character =  'a' + i;
		if (input_info.mode.alpha == 1)
			character = 'A' + i;;
	} else {
		character = keylist_num_char[i];
	}
	input_buffer_insert(character);
	return (1);
}

/* input_read(): Handle user input */
int input_read(char *buffer, size_t size)
{
	key_event_t key;
	uint16_t *secondary;
	uint16_t *main;
	void *tmp;
	int spe;

	/* check obvious error */
	if (buffer == NULL || size == 0)
		return (0);

	/* initialize internal data */
	memset(&input_info, 0x00, sizeof(input_info));
	memset(buffer, 0x00, size);

	/* save terminal information */
	input_info.buffer.data = buffer;
	input_info.buffer.size = 1;
	input_info.buffer.max = size;

	/* Gint workaround to freeze the current display */
	dgetvram(&main, &secondary);
	if (gint_vram == main) {
		tmp = main;
		main = secondary;
		secondary = tmp;
	}
	memcpy(secondary, main, 2*396*224);

	/* keyboard handling */
	while (input_info.mode.exit == 0) {
		input_display();
		key = getkey_opt(GETKEY_REP_ALL | GETKEY_MENU, NULL);
		spe = input_key_handle_special(key);
		if (spe < 0) {
			input_info.buffer.size = -1;
			break;
		}
		if (spe == 0)
			input_key_buffer_update(key);
	}
	return (input_info.buffer.size);
}

/* input_write(): Display printf-like string */
/* TODO: handle scrool bar if the message is too long to be displayed */
int input_write(const char *format, ...)
{
	char buffer[512];
	va_list _args;
	uint16_t *secondary;
	uint16_t *main;
	size_t size;
	void *tmp;

	va_start(_args, format);
	size = vsnprintf(buffer, 512, format, _args);
	va_end(_args);

	/* initialize internal data */
	memset(&input_info, 0x00, sizeof(input_info));

	/* save terminal information */
	input_info.buffer.data = buffer;
	input_info.buffer.size = size;
	input_info.buffer.max = 512;

	/* Gint workaround to freeze the current display */
	dgetvram(&main, &secondary);
	if (gint_vram == main) {
		tmp = main;
		main = secondary;
		secondary = tmp;
	}
	memcpy(secondary, main, 2*396*224);

	/* display and wait user event */
	input_display();
	getkey();
	return (0);
}
