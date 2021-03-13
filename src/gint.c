#include "gintrace/ubc.h"

#include <gint/gint.h>
#include <gint/atomic.h>
#include <gint/defs/attributes.h>
#include <gint/drivers.h>
#include <gint/display.h>

/* external symbols */
extern void *kernel_env_casio;
extern void *kernel_env_gint;
extern void *kernel_env_tracer;

/* gint_switch(): Temporarily switch out of gint */
void gint_switch(void (*function)(void))
{
	void *buffctx_current;
	void *buffctx;

	/* check useless action */
	if (function == NULL)
		return;

	/* check useless world switch */
	buffctx_current = drivers_get_current();
	if (kernel_env_casio == buffctx_current) {
		function();
		return;
	}

	/* Switch from gint to the OS after a short wait */
	ubc_uninstall();
	buffctx = drivers_switch(kernel_env_casio);
	ubc_install();

	/* involve the function */
	function();

	/* then switch back to gint once the os finishes working */
	ubc_uninstall();
	drivers_switch(buffctx);
	ubc_install();
}

/* Switch from gint to the OS after a short wait */
void *gint_switch_to_world(void *buffctx)
{
	void *buffctx_current;

	buffctx_current = drivers_get_current();
	if (buffctx != buffctx_current) {
		ubc_uninstall();
		drivers_switch(buffctx);
		ubc_install();
	}
	return (buffctx_current);
}
