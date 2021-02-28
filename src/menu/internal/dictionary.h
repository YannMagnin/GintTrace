#ifndef __DICTIONARY_H__
# define __DICTIONARY_H__

#include <stddef.h>
#include <stdint.h>

//---
// Internal data information
//---

/* Define the number max of arguments for intrustions. */
#define OPCODE_ARGUMENTS_MAX	3

/* Arguments information
 * 0b0000x -> argument special enable
 * 0bxxxx0 -> special action ID
 *
 * Info organisations
 * 0b*[s000][s000][s000]xxx -> xxx = arg status
 *     act2  act1  act0	    -> s = 1) signed, 0) unsigned */
#define __ARG_SIGN_SET(arg, sign)	((sign & 1) << (3 + 3 + (arg << 2)))
#define __ARG_ACTION_SET(arg, action)	((action & 0b111) << (3 + (arg << 2)))
#define __ARG_STATUS_SET(arg)		(1 << arg)

/* Opcode information help */
#define OPCODE_INFO_IS_SIGNED(info, arg)	\
		(info & __ARG_SIGN_SET(arg, 1))
#define OPCODE_INFO_IS_SET(info, arg)		\
		(info & __ARG_STATUS_SET(arg))
#define OPCODE_INFO_GET_ACTION(info, arg)	\
		((info >> (3 + (arg << 2))) & 0b0111)
#define OPCODE_INFO_SET(arg, action, sign)	\
	( __ARG_SIGN_SET(arg, sign)		\
	 | __ARG_ACTION_SET(arg, action) 	\
	 | __ARG_STATUS_SET(arg))

/* Opcode registers information */
#define OPCODE_ARG_0				0b000
#define OPCODE_ARG_1				0b001
#define OPCODE_ARG_2				0b010
#define OPCODE_ARG_SIGNED			0b1
#define OPCODE_ARG_UNSIGNED			0b0

/* Opcode actions information */
#define OPCODE_ACTION_INST_LONG			0b0000
#define OPCODE_ACTION_INST_WORD			0b0001
#define OPCODE_ACTION_PC_LONG			0b0010
#define OPCODE_ACTION_PC_WORD			0b0011
#define OPCODE_ACTION_DISP_LONG			0b0100
#define OPCODE_ACTION_DISP_WORD			0b0101

/* osnote: OS-specific information */
struct osnote {
	uintptr_t address;
	const char *name;
};

/* opcode - internal instruction data part. */
struct opcode {
	const char *name;
	uint16_t mask;
	uint16_t code;
	struct {
		uint16_t mask;
	} arg[OPCODE_ARGUMENTS_MAX];
	uint16_t info;
} __attribute__((packed, aligned(4)));

/* sysname - syscall name */
struct sysname {
	uint32_t syscall;
	const char *name;
};

/* regname - peripherals name */
struct regname {
	uint32_t address;
	const char *name;
};

//---
// Functions
//---

/* dictionary_notes_check(): Check if the address is a syscall. */
extern const char *dictionary_notes_check(void *address);

/* dictionary_syscalls_check(): Check if the address is a syscall. */
extern const char *dictionary_syscalls_check(void *address);

/* dictionaru_peripherals_check(): Check preipheral address */
extern const char *dictionary_peripherals_check(void *address);


/* disasm_dictionnary_opcodes_get_arg(); return arguments of a given opcode */
extern uint32_t dictionary_opcodes_get_arg(const struct opcode *info,
					uint16_t opcode, int argID, void *pc);

/* disasm_dictionnary_check_opcode(): Try to find opcode information */
extern const struct opcode *dictionary_opcodes_check(uint16_t opcode);


#endif /*__DICTIONARY_H__*/
