//---
//	gintrace:menu - Menu helper
//---
#include "gintrace/gui/menu.h"
#include "gintrace/gui/fkey.h"
#include "gintrace/gui/input.h"
#include "gintrace/lexer.h"

#include <gint/std/string.h>
#include <gint/std/stdlib.h>
#include <gint/keyboard.h>
#include <gint/display.h>

/* menu_create(): Create a group of menus */
int menu_create(struct menu_group **gmenu)
{
	*gmenu = calloc(sizeof(struct menu_group), 1);
	if (*gmenu == NULL)
		return (menu_retval_enomem);
	return (menu_retval_success);
}

/* menu_register(): Register a new menu to the internal menu table */
int menu_register(struct menu_group *gmenu,
					struct menu *menu, const char *name)
{
	struct menu_list **node;

	if (gmenu == NULL || name == NULL)
		return (menu_retval_efault);
	node = &gmenu->list;
	while (*node != NULL) {
		if (strcmp((*node)->name, name) == 0)
			return (menu_retval_already_registered);
		node = &(*node)->next;
	}
	*node = calloc(sizeof(struct menu_list), 1);
	if (*node == NULL)
		return (menu_retval_enomem);
	if (menu != NULL && menu->ctor != NULL)
		menu->ctor();
	if (gmenu->selected == NULL)
		gmenu->selected = menu;
	(*node)->menu = menu;
	(*node)->name = name;
	gmenu->menu_counter += 1;
	return (menu_retval_success);
}

/* menu_unregister(): Unregister menu */
int menu_unregister(struct menu_group *gmenu, const char *name)
{
	struct menu_list **node;
	void *tmp;

	if (gmenu == NULL || name == NULL)
		return (menu_retval_efault);
	node = &gmenu->list;
	while (*node != NULL) {
		if (strcmp((*node)->name, name) != 0) {
			node = &(*node)->next;
			continue;
		}
		tmp = *node;
		*node = (*node)->next;
		free(tmp);
		return (menu_retval_success);
	}
	gmenu->menu_counter -= 1;
	return (menu_retval_unregistered);
}

/* menu_init(): Initialize all menu */
int menu_init(struct menu_group *gmenu, volatile void *arg)
{
	struct menu_list *node;

	if (gmenu == NULL)
		return (menu_retval_efault);
	node = gmenu->list;
	while (node != NULL) {
		if (node->menu != NULL && node->menu->init != NULL)
			node->menu->init((void*)arg);
		node = node->next;
	}
	gmenu->arg = arg;
	gmenu->is_open = 0;
	return (menu_retval_success);

}

/* menu_is_open(): Return the menu status */
int menu_is_open(struct menu_group *gmenu)
{
	if (gmenu == NULL)
		return (menu_retval_efault);
	return (gmenu->is_open != 0);
}

/* menu_draw(): Draw menu specific information and menu abstraction overlay */
int menu_draw(struct menu_group *gmenu)
{
	struct menu_list *node;
	int offset;
	int counter;

	if (gmenu == NULL)
		return (menu_retval_efault);
	dclear(C_WHITE);
	if (gmenu->selected != NULL && gmenu->selected->display != NULL)
		gmenu->selected->display((void*)gmenu->arg);
	counter = 0;
	node = gmenu->list;
	offset = gmenu->menu_page * 5;
	while (node != NULL) {
		if (gmenu->menu_counter <= 6) {
			fkey_menu(1 + counter, node->name);
			counter += 1;
			node = node->next;
			continue;
		}
		if (counter - offset >= 5)
			break;
		if (counter - offset >= 0)
			fkey_menu(1 + (counter - offset), node->name);
		node = node->next;
		counter += 1;
	}
	if (gmenu->menu_counter >= 7)
		fkey_action(6, ">");
	dupdate();
	return (menu_retval_success);
}

/* menu_keyboard(): handle keyboard event */
int menu_keyboard(struct menu_group *gmenu)
{
	const int keylist[] = {KEY_F1, KEY_F2, KEY_F3, KEY_F4, KEY_F5, KEY_F6};
	struct menu_list *node;
	int counter;
	int target;
	int key;

	if (gmenu == NULL)
		return (menu_retval_efault);
	key = getkey().key;
	if (key == KEY_EXE) {
		gmenu->is_open = 1;
		return (menu_retval_success);
	}
	if (key == KEY_SQUARE) {
		if (gmenu->selected == NULL
				|| gmenu->selected->command == NULL) {
			goto check_fkeys;
		}
		char buf[256];
		if (input_read(buf, 256, NULL) <= 0)
			goto check_fkeys;
		int ac;
		char **av;
		if (lexer_strtotab(&ac, &av, buf) == 0) {
			gmenu->selected->command((void*)gmenu->arg, ac, av);
			lexer_strtotab_quit(&ac, &av);
		}
	}
check_fkeys:
	for (int i = 0; i < 6; ++i) {
		if (keylist[i] != key)
			continue;
		if (key == KEY_F6 && gmenu->menu_counter >= 7) {
			gmenu->menu_page += 1;
			if (gmenu->menu_page * 5 >= gmenu->menu_counter)
				gmenu->menu_page = 0;
			return (0);
		}
		counter = 0;
		target = (gmenu->menu_page * 5) + i;
		node = gmenu->list;
		while (node != NULL) {
			if (counter == target) {
				gmenu->selected = node->menu;
				return (menu_retval_success);
			}
			counter += 1;
			node = node->next;
		}
		return (menu_retval_success);
	}
	if (gmenu->selected == NULL || gmenu->selected->keyboard == NULL)
		return (menu_retval_success);
	if (gmenu->selected->keyboard((void*)gmenu->arg, key) != 0)
		gmenu->is_open = 1;
	return (menu_retval_success);
}
