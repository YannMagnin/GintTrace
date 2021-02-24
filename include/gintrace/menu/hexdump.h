#ifndef __GINTRACE_MENU_HEXDUMP_H__
# define __GINTRACE_MENU_HEXDUMP_H__

#include <stddef.h>
#include <stdint.h>

/* hexdump: Internal hexdump structure */
struct hexdump {
	struct {
		int hoffset;
		int voffset;
	} cursor;
	uint8_t *addr;
};

/* extern menu information */
extern struct menu menu_hexdump;

#endif /*__GINTRACE_MENU_HEXDUMP_H__*/
