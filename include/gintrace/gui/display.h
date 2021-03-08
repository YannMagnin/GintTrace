#ifndef __GINTRACE_GUI_DISPLAY_H__
# define __GINTRACE_GUI_DISPLAY_H__

#include <gint/display.h>

/* define font information */
#ifdef FXCG50
#define FWIDTH	9
#define FHEIGHT	10
#endif
#ifdef FX9860G
#define FWIDTH	5
#define FHEIGHT	7
#endif

/* define display information */
#define GUI_DISP_NB_COLUMN	(DWIDTH / (FWIDTH + 1))
#define GUI_DISP_NB_ROW		(DHEIGHT / (FHEIGHT + 1))

/* color scheme definition */
struct colorscheme {
	uint16_t background;
	uint16_t font;
	uint16_t note;
	uint16_t line;
};

/* gcolor_scheme_change(): Change the color scheme */
extern void gcolor_scheme_change(void);

/* gclear(): Clear the VRAM and set the appropriate background color */
extern void gclear(void);
/* gupdate(): Display the VRAM to the screen */
extern void gupdate(void);

/* gtextXY(): Display plain text */
extern void gtextXY(int x, int y, const char *text);
/* gprintXY(): Display printf-format text */
extern void gprintXY(int x, int y, const char *format, ...);
/* gprintXY(): Display note */
extern void gnoteXY(int x, int y, const char *note);

/* ghreverse(): Reverse pixels color on a horizontal area */
extern void ghreverse(int ypos, size_t size);

/* ghline(): Display horizontal line */
extern void ghline(int ypos);

#endif /*__GINTRACE_GUI_DISPLAY_H__*/
