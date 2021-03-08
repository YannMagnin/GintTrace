#ifndef __GINTRACE_MENU_H__
# define __GINTRACE_MENU_H__

#include <stddef.h>
#include <stdint.h>

#include "gintrace/ubc.h"

/* define menu information */
struct menu {
	void (*ctor)(void *data);
	void (*init)(void *data);
	void (*display)(void *data);
	int  (*keyboard)(void *data, int key);
	void (*command)(void *data, int argc, char **argv);
	void (*dtor)(void *data);
};

/* menu group information */
struct menu_group {
	int is_open;
	int menu_page;
	int menu_counter;
	struct menu *selected;
	struct menu_list {
		const char *name;
		struct menu *menu;
		struct menu_list *next;
	} *list;
	volatile void *arg;
};

//---
//	User API
//---
enum {
	menu_retval_success            = 0,
	menu_retval_enomem             = -1,
	menu_retval_efault             = -2,
	menu_retval_already_registered = -3,
	menu_retval_unregistered       = -4,
};

/* menu_create(): Create a group of menus */
extern int menu_create(struct menu_group **gmenu, volatile void *arg);

/* menu_register(): Register a new menu to the internal menu table */
extern int menu_register(struct menu_group *gmenus,
					struct menu *menu, const char *name);

/* menu_unregister(): Unregister menu */
extern int menu_unregister(struct menu_group *gmenus, const char *name);


/* menu_is_open(): Return the menu status */
extern int menu_is_open(struct menu_group *gmenu);

/* menu_init(): Initialize all menu */
extern int menu_init(struct menu_group *gmenus);

/* menu_draw(): Draw menu specific information and menu abstraction overlay */
extern int menu_draw(struct menu_group *gmenus);

/* menu_keyboard(): handle keyboard event */
extern int menu_keyboard(struct menu_group *gmenu);
#endif /*__GINTRACE_MENU_H__*/
