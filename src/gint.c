#include "gintrace/ubc.h"

#include <gint/gint.h>
#include <gint/atomic.h>
#include <gint/defs/attributes.h>
#include <gint/drivers.h>

/* external symbols */
extern void *kernel_env_casio;
extern void *kernel_env_gint;

/* gint_switch(): Temporarily switch out of gint */
void gint_switch(void (*function)(void))
{
	/* Switch from gint to the OS after a short wait */
	ubc_uninstall();
	drivers_wait();
	drivers_switch(kernel_env_gint, kernel_env_casio);
	ubc_install();

	if(function != NULL)
		function();

	/* then switch back to gint once the os finishes working */
	ubc_uninstall();
	drivers_wait();
	drivers_switch(kernel_env_casio, kernel_env_gint);
	ubc_install();
}

/* Switch from gint to the OS after a short wait */
void gint_switch_to_casio(void)
{
	ubc_uninstall();
	drivers_wait();
	drivers_switch(kernel_env_gint, kernel_env_casio);
	ubc_install();
}

/* Switch from gint to the OS after a short wait */
void gint_switch_to_gint(void)
{
	ubc_uninstall();
	drivers_wait();
	drivers_switch(kernel_env_casio, kernel_env_gint);
	ubc_install();
}
