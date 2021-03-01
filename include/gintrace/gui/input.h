#ifndef __GINTRACE_GUI_INPUT_H__
# define __GINTRACE_GUI_INPUT_H__

#include <stddef.h>
#include <stdint.h>

/* intput_read(): Handle user input */
extern int input_read(char *buffer, size_t size, const char *prefix, ...);

/* intput_write(): display message then wait user event */
extern int input_write(const char *format, ...);
/* intput(): display message */
extern int input_write_noint(const char *format, ...);
#endif /*__GINTRACE_GUI_INPUT_H__*/
