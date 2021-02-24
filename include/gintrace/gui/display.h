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

#endif /*__GINTRACE_GUI_DISPLAY_H__*/
