#ifndef __GINTRACE_GUI_INPUT_H__
# define __GINTRACE_GUI_INPUT_H__

#include <stddef.h>
#include <stdint.h>

/* intput(): Handle user input */
extern int input_read(char *buffer, size_t size);

/* intput(): display message then wait user event */
extern int input_write(const char *format, ...);

#endif /*__GINTRACE_GUI_INPUT_H__*/
