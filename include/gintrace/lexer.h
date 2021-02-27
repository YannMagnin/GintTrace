#ifndef __LEXER_H__
# define __LEXER_H__

#include <stddef.h>
#include <stdint.h>

extern int lexer_strtotab(int *argc, char ***argv, char const *str);
extern void lexer_strtotab_quit(int *argc, char ***argv);

#endif /*__lexer_H__*/
