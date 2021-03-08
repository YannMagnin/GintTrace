#include "gintrace/gui/display.h"

#include <gint/display.h>
#include <gint/std/stdio.h>


/* define all color scheme */
struct colorscheme night = {
	.background = 0x4aec,
	.font = 0xffff,
	.note = 0x5ab0,
	.line = 0xffff
};
struct colorscheme days = {
	.background = 0x0000,
	.font = 0xffff,
	.note = 0xf100,
	.line = 0xffff
};
struct colorscheme days2 = {
	.background = 0xffff,
	.font = 0x0000,
	.note = 0xb900,
	.line = 0x0000
};
struct colorscheme *color_scheme = &days2;
struct colorscheme *color_scheme_list[] = { &night, &days, &days2, NULL };

//---
// Color
//---
/* gcolor_scheme_change(): Change the color scheme */
void gcolor_scheme_change(void)
{
	static int counter = 0;

	counter = counter + 1;
	if (color_scheme_list[counter] == NULL)
		counter = 0;
	color_scheme = color_scheme_list[counter];
}

//---
// Drawing wrapper
//---
/* gclear(): Clear the VRAM and set the appropriate background color */
void gclear(void)
{
	dclear(color_scheme->background);
}
/* gupdate(): Display the VRAM to the screen */
void gupdate(void)
{
	dupdate();
}


/* ghreverse(): Reverse pixels color on a horizontal area */
void ghreverse(int ypos, size_t size)
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

/* ghline(): Display horizontal line */
void ghline(int ypos)
{
	dhline(ypos, color_scheme->line);
}


/* gprintXY(): Display printf-format text */
void gprintXY(int x, int y, const char *format, ...)
{
	char str[512];
	va_list args;

	va_start(args, format);
	vsnprintf(str, 512, format, args);
	va_end(args);

	gtextXY(x, y, str);
}

/* gprintXY(): Display note */
void gnoteXY(int x, int y, const char *note)
{
	drect(0, y * (FHEIGHT + 1) - 1,
			DWIDTH, (y + 1) * (FHEIGHT + 1) - 1,
			color_scheme->note);
	uint16_t bg = color_scheme->background;
	uint16_t fg = color_scheme->font;
	color_scheme->background = color_scheme->note;
	color_scheme->font = 0xffff;
	gtextXY(x, y, note);
	color_scheme->background = bg;
	color_scheme->font = fg;
}


//---
// Text engine
//---
struct textinfo {
	struct {
		uint16_t bg;
		uint16_t fg;
	} color;
	struct {
		int x;
		int y;
	} cursor;
	const char *text;
	int counter;
};

/* gtext_line_discipline(): Check special char */
static int gtext_line_discipline(struct textinfo *info)
{
	int offset;

	switch (info->text[0]) {
	case ' ':
		info->cursor.x = info->cursor.x + 1;
		return (1);
	case '\n':
		info->cursor.x = 0;
		info->cursor.y = info->cursor.y + 1;
		return (1);
	case '\b':
		info->cursor.x = info->cursor.x - 1;
		return (1);
	case '\v':
		info->cursor.y = info->cursor.y - 1;
		return (1);
	case '\r':
		info->cursor.x = 0;
		return (1);
	case '\t':
		offset = 5 - (info->counter - ((info->counter / 5) * 5));
		info->cursor.x = info->cursor.x + offset;
		return (1);
	default:
		return (0);
	}
}

/* gtext_echapement(): Check echapement char */
static int gtext_echapement(struct textinfo *info)
{
	struct {
		int id;
		uint16_t color;
	} color_table[] = {
		{.id = 30, .color = 0x001f},
		{.id = 31, .color = 0x07e0},
		{.id = 32, .color = 0xf800},
		{.id = 33, .color = 0x0000},
		{.id = 34, .color = 0x0000},
		{.id = 35, .color = 0x0000},
		{.id = 36, .color = 0x0000},
		{.id = 37, .color = 0x0000},
	};

	if (info->text[0] != '\\'
			|| info->text[1] != 'e'
			|| info->text[2] != '[') {
		return (0);
	}
	int echap = -1;
	for (int i = 0; info->text[i] != '\0'; ++i) {
		if (info->text[i] == 'm') {
			if (echap == -1)
				return (0);
			if (echap == 0) {
				info->color.fg = color_scheme->font;
				return (i + 1);
			}

			for (int j = 0; color_table[i].id != -1; ++j) {
				if (color_table[i].id != echap)
					continue;
				info->color.fg = color_table[i].color;
				return (i + 1);
			}
			return (0);
		}
		if (info->text[i] < '0' && info->text[i] > '9')
			return (0);
		if (echap == -1)
			echap = 0;
		echap = (echap * 10) + (info->text[i] - '0');
	}
	return (0);
}

/* gtextXY(): Display plain text */
void gtextXY(int x, int y, const char *text)
{
	struct textinfo textinfo = {
		.color = {
			.bg = color_scheme->background,
			.fg = color_scheme->font,
		},
		.cursor = {
			.x = x,
			.y = y,
		},
		.text = text,
		.counter = 0
	};
	uint16_t tmp;

	while (textinfo.text[0] != '\0') {
		tmp = gtext_line_discipline(&textinfo);
		if (tmp != 0) {
			textinfo.text = &textinfo.text[tmp];
			continue;
		}
		tmp = gtext_echapement(&textinfo);
		if (tmp != 0) {
			textinfo.text = &textinfo.text[tmp];
			continue;
		}
		tmp = textinfo.text[0] << 8;
		dtext_opt(textinfo.cursor.x * (FWIDTH + 1),
				textinfo.cursor.y * (FHEIGHT + 1),
				textinfo.color.fg, textinfo.color.bg,
				DTEXT_LEFT, DTEXT_TOP, (void*)&tmp);
		textinfo.text = &textinfo.text[1];
		textinfo.cursor.x += 1;
		textinfo.counter  += 1;
	}
}
