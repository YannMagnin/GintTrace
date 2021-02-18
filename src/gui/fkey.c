#include "gintrace/gui/fkey.h"

#include <gint/display.h>

/* fkey_action(): A black-on-white F-key */
void fkey_action(int position, char const *text)
{
	int width;
	dsize(text, NULL, &width, NULL);

	int x = 4 + 65 * (position - 1);
	int y = 207;
	int w = 63;

	dline(x + 1, y, x + w - 2, y, C_BLACK);
	dline(x + 1, y + 14, x + w - 2, y + 14, C_BLACK);
	drect(x, y + 1, x + 1, y + 13, C_BLACK);
	drect(x + w - 2, y + 1, x + w - 1, y + 13, C_BLACK);

	dtext(x + ((w - width) >> 1), y + 3, C_BLACK, text);
}

/* fkey_button(): A rectangular F-key */
void fkey_button(int position, char const *text)
{
	int width;
	dsize(text, NULL, &width, NULL);

	int x = 4 + 65 * (position - 1);
	int y = 207;
	int w = 63;

	dline(x + 1, y, x + w - 2, y, C_BLACK);
	dline(x + 1, y + 14, x + w - 2, y + 14, C_BLACK);
	drect(x, y + 1, x + w - 1, y + 13, C_BLACK);

	dtext(x + ((w - width) >> 1), y + 3, C_WHITE, text);
}

/* fkey_menu(): A rectangular F-key with the bottom right corner removed */
void fkey_menu(int position, char const *text)
{
	int x = 4 + 65 * (position - 1);
	int y = 207;
	int w = 63;

	fkey_button(position, text);
	dline(x + w - 1, y + 10, x + w - 5, y + 14, C_WHITE);
	dline(x + w - 1, y + 11, x + w - 4, y + 14, C_WHITE);
	dline(x + w - 1, y + 12, x + w - 3, y + 14, C_WHITE);
	dline(x + w - 1, y + 13, x + w - 2, y + 14, C_WHITE);
}
