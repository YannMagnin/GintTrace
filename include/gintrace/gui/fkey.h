#ifndef __GINTRACE_GUI_FKEY_H__
# define __GINTRACE_GUI_FKEY_H__

#include <stddef.h>
#include <stdint.h>

/* fkey_action(): A black-on-white F-key */
extern void fkey_action(int position, char const *text);

/* fkey_button(): A rectangular F-key */
extern void fkey_button(int position, char const *text);

/* fkey_menu(): A rectangular F-key with the bottom right corner removed */
extern void fkey_menu(int position, char const *text);

#endif /*__GINTRACE_GUI_FKEY_H__*/
