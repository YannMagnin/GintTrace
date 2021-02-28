#include "./src/menu/internal/dictionary.h"
#include <gint/std/stdio.h>

//---
// Internal functions
//---
/* internal actions
 * @note:
 * -> o = opcode
 * -> a = action
 * -> i = instruction
 * -> p = PC
 * -> d = direct */
static uint32_t oail(uint32_t pc, int32_t arg) { return (pc + (arg * 4)); };
static uint32_t oaiw(uint32_t pc, int32_t arg) { return (pc + (arg * 2)); };
static uint32_t oadl(uint32_t pc, int32_t arg) { (void)pc; return (arg * 4); };
static uint32_t oadw(uint32_t pc, int32_t arg) { (void)pc; return (arg * 2); };
static uint32_t oapl(uint32_t pc, int32_t arg)
{
	return (((uint32_t*)(uintptr_t)(pc >> 2 << 2))[arg]);
};
static uint32_t oapw(uint32_t pc, int32_t arg)
{
	return (((uint16_t*)(uintptr_t)(pc >> 1 << 1))[arg]);
};

/* internal actions list */
static uint32_t (*actions[6])(uint32_t,int32_t) = {
	&oail, &oaiw, &oapl, &oapw, &oadl, &oadw
};




//---
// Public function
//---
/* disasm_dictionnary_opcode_get_arg(); return arguments of a given opcode */
uint32_t disasm_dictionary_opcode_get_arg(const struct opcode *info,
					uint16_t opcode, int argID, void *pc)
{
	uint32_t argument;
	uint16_t mask;
	int action;

	/* check error */
	if (argID < 0
		|| argID >= OPCODE_ARGUMENTS_MAX
		|| info == NULL
		|| info->arg[argID].mask == 0x00000000) {
		return (0);
	}

	/* Get argument
	 * @note: best way to get the good argument shifting ? */
	mask = info->arg[argID].mask;
	argument = opcode & info->arg[argID].mask;
	while ((mask & 1) == 0) {
		argument = argument >> 1;
		mask = mask >> 1;
	}

	/* check special argument */
	if (OPCODE_INFO_IS_SET(info->info, argID) == 0)
		return (argument);

	/* check if the arg is signed */
	if (OPCODE_INFO_IS_SIGNED(info->info, argID) != 0) {
		uint16_t smask = mask;
		uint16_t is_negative = argument;
		while ((smask & 1) == 1) {
			if (smask != mask)
				is_negative = is_negative >> 1;
			smask = smask >> 1;
		}
		if ((is_negative & 1) == 1) {
			argument = (argument ^ mask) + 1;
			argument = -argument;
		}
	}

	/* call opcode specific action */
	action =OPCODE_INFO_GET_ACTION(info->info, argID);
	return (actions[action]((uintptr_t)pc + 4, argument));
}

/* disasm_dictionnary_check_opcode(): Try to find opcode information */
const struct opcode *disasm_dictionary_check_opcode(uint16_t opcode)
{
	extern const struct opcode opcode_list[];

	for (int i = 0 ; opcode_list[i].name != NULL ; ++i) {
		if ((opcode & opcode_list[i].mask) == opcode_list[i].code)
			return (&opcode_list[i]);
	}
	return (NULL);
}



//---
// Define ALL SH3 / SH4 instructions.
//---
const struct opcode opcode_list[] = {
	{
		.name = "add\t0x%x,r%d",
		.mask =	0b1111000000000000,
		.code = 0b0111000000000000,
		.arg = {
			{.mask = 0b0000000011111111},
			{.mask = 0b0000111100000000},
			{.mask = 0b0000000000000000}
		},
		.info = 0x0000
	},
	{
		.name = "add\tr%d,r%d",
		.mask =	0b1111000000001111,
		.code = 0b0011000000001100,
		.arg = {
			{.mask = 0b0000000011110000},
			{.mask = 0b0000111100000000},
			{.mask = 0b0000000000000000}
		},
		.info = 0x0000
	},
	{
		.name = "addc\tr%d,r%d",
		.mask =	0b1111000000001111,
		.code = 0b0011000000001110,
		.arg = {
			{.mask = 0b0000000011110000},
			{.mask = 0b0000111100000000},
			{.mask = 0b0000000000000000}
		},
		.info = 0x0000
	},
	{
		.name = "addv\tr%d,r%d",
		.mask =	0b1111000000001111,
		.code = 0b0011000000001111,
		.arg = {
			{.mask = 0b0000000011110000},
			{.mask = 0b0000111100000000},
			{.mask = 0b0000000000000000}
		},
		.info = 0x0000
	},
	{
		.name = "and\t0x%x,r0",
		.mask =	0b1111111100000000,
		.code = 0b1100100100000000,
		.arg = {
			{.mask = 0b0000000011111111},
			{.mask = 0b0000000000000000},
			{.mask = 0b0000000000000000}
		},
		.info = 0x0000
	},
	{
		.name = "and\tr%d,r%d",
		.mask =	0b1111000000001111,
		.code = 0b0010000000001001,
		.arg = {
			{.mask = 0b0000000011110000},
			{.mask = 0b0000111100000000},
			{.mask = 0b0000000000000000}
		},
		.info = 0x0000
	},
	{
		.name = "and.b\t0x%x,@(r0,GBR)",
		.mask =	0b1111111100000000,
		.code = 0b1100110100000000,
		.arg = {
			{.mask = 0b0000000011111111},
			{.mask = 0b0000000000000000},
			{.mask = 0b0000000000000000}
		},
		.info = 0x0000
	},
	{
		.name = "bf\t%#.8x",
		.mask =	0b1111111100000000,
		.code = 0b1000101100000000,
		.arg = {
			{.mask = 0b0000000011111111},
			{.mask = 0b0000000000000000},
			{.mask = 0b0000000000000000}
		},
		.info = OPCODE_INFO_SET(OPCODE_ARG_0, OPCODE_ACTION_INST_WORD, OPCODE_ARG_SIGNED)
	},
	{
		.name = "bf/s\t%#.8x",
		.mask =	0b1111111100000000,
		.code = 0b1000111100000000,
		.arg = {
			{.mask = 0b0000000011111111},
			{.mask = 0b0000000000000000},
			{.mask = 0b0000000000000000}
		},
		.info = OPCODE_INFO_SET(OPCODE_ARG_0, OPCODE_ACTION_INST_WORD, OPCODE_ARG_UNSIGNED)
	},
	{
		.name = "bra\t%#.8x",
		.mask =	0b1111000000000000,
		.code = 0b1010000000000000,
		.arg = {
			{.mask = 0b0000111111111111},
			{.mask = 0b0000000000000000},
			{.mask = 0b0000000000000000}
		},
		.info = OPCODE_INFO_SET(OPCODE_ARG_0, OPCODE_ACTION_INST_WORD, OPCODE_ARG_SIGNED)
	},
	{
		.name = "braf\tr%d",
		.mask =	0b1111000011111111,
		.code = 0b0000000000100011,
		.arg = {
			{.mask = 0b0000111100000000},
			{.mask = 0b0000000000000000},
			{.mask = 0b0000000000000000}
		},
		.info = 0x0000
	},
	{
		.name = "bsr\t%#.8x",
		.mask =	0b1111000000000000,
		.code = 0b1011000000000000,
		.arg = {
			{.mask = 0b0000111111111111},
			{.mask = 0b0000000000000000},
			{.mask = 0b0000000000000000}
		},
		.info = OPCODE_INFO_SET(OPCODE_ARG_0, OPCODE_ACTION_INST_WORD, OPCODE_ARG_SIGNED)
	},
	{
		.name = "bsrf\tr%d",
		.mask =	0b1111000011111111,
		.code = 0b0000000000000011,
		.arg = {
			{.mask = 0b0000111100000000},
			{.mask = 0b0000000000000000},
			{.mask = 0b0000000000000000}
		},
		.info = 0x0000
	},
	{
		.name = "bt\t%#.8x",
		.mask =	0b1111111100000000,
		.code = 0b1000100100000000,
		.arg = {
			{.mask = 0b0000000011111111},
			{.mask = 0b0000000000000000},
			{.mask = 0b0000000000000000}
		},
		.info = OPCODE_INFO_SET(OPCODE_ARG_0, OPCODE_ACTION_INST_WORD, OPCODE_ARG_SIGNED)
	},
	{
		.name = "bt/s\t%#.8x",
		.mask =	0b1111111100000000,
		.code = 0b1000110100000000,
		.arg = {
			{.mask = 0b0000000011111111},
			{.mask = 0b0000000000000000},
			{.mask = 0b0000000000000000}
		},
		.info = OPCODE_INFO_SET(OPCODE_ARG_0, OPCODE_ACTION_INST_WORD, OPCODE_ARG_SIGNED)
	},
	{
		.name = "clrmac",
		.mask =	0b1111111111111111,
		.code = 0b0000000000101000,
		.arg = {
			{.mask = 0b0000000000000000},
			{.mask = 0b0000000000000000},
			{.mask = 0b0000000000000000}
		},
		.info = 0x0000
	},
	{
		.name = "clrs",
		.mask =	0b1111111111111111,
		.code = 0b0000000001001000,
		.arg = {
			{.mask = 0b0000000000000000},
			{.mask = 0b0000000000000000},
			{.mask = 0b0000000000000000}
		},
		.info = 0x0000
	},
	{
		.name = "clrt",
		.mask =	0b1111111111111111,
		.code = 0b0000000000001000,
		.arg = {
			{.mask = 0b0000000000000000},
			{.mask = 0b0000000000000000},
			{.mask = 0b0000000000000000}
		},
		.info = 0x0000
	},
	{
		.name = "cmp/eq\t0x%x,r0",
		.mask =	0b1111111100000000,
		.code = 0b1000100000000000,
		.arg = {
			{.mask = 0b0000000011111111},
			{.mask = 0b0000000000000000},
			{.mask = 0b0000000000000000}
		},
		.info = 0x0000
	},
	{
		.name = "cmp/eq\tr%d,r%d",
		.mask =	0b1111000000001111,
		.code = 0b0011000000000000,
		.arg = {
			{.mask = 0b0000000011110000},
			{.mask = 0b0000111100000000},
			{.mask = 0b0000000000000000}
		},
		.info = 0x0000
	},
	{
		.name = "cmp/ge\tr%d,r%d",
		.mask =	0b1111000000001111,
		.code = 0b0011000000000011,
		.arg = {
			{.mask = 0b0000000011110000},
			{.mask = 0b0000111100000000},
			{.mask = 0b0000000000000000}
		},
		.info = 0x0000
	},
	{
		.name = "cmp/gt\tr%d,r%d",
		.mask =	0b1111000000001111,
		.code = 0b0011000000000111,
		.arg = {
			{.mask = 0b0000000011110000},
			{.mask = 0b0000111100000000},
			{.mask = 0b0000000000000000}
		},
		.info = 0x0000
	},
	{
		.name = "cmp/hi\tr%d,r%d",
		.mask =	0b1111000000001111,
		.code = 0b0011000000000110,
		.arg = {
			{.mask = 0b0000000011110000},
			{.mask = 0b0000111100000000},
			{.mask = 0b0000000000000000}
		},
		.info = 0x0000
	},
	{
		.name = "cmp/hs\tr%d,r%d",
		.mask =	0b1111000000001111,
		.code = 0b0011000000000010,
		.arg = {
			{.mask = 0b0000000011110000},
			{.mask = 0b0000111100000000},
			{.mask = 0b0000000000000000}
		},
		.info = 0x0000
	},
	{
		.name = "cmp/pl\tr%d",
		.mask =	0b1111000011111111,
		.code = 0b0100000000010101,
		.arg = {
			{.mask = 0b0000111100000000},
			{.mask = 0b0000000000000000},
			{.mask = 0b0000000000000000}
		},
		.info = 0x0000
	},
	{
		.name = "cmp/pz\tr%d",
		.mask =	0b1111000011111111,
		.code = 0b0100000000010001,
		.arg = {
			{.mask = 0b0000111100000000},
			{.mask = 0b0000000000000000},
			{.mask = 0b0000000000000000}
		},
		.info = 0x0000
	},
	{
		.name = "cmp/str\tr%d,r%d",
		.mask =	0b1111000000001111,
		.code = 0b0010000000001100,
		.arg = {
			{.mask = 0b0000000011110000},
			{.mask = 0b0000111100000000},
			{.mask = 0b0000000000000000}
		},
		.info = 0x0000
	},
	{
		.name = "div0s\tr%d,r%d",
		.mask =	0b1111000000001111,
		.code = 0b0010000000000111,
		.arg = {
			{.mask = 0b0000000011110000},
			{.mask = 0b0000111100000000},
			{.mask = 0b0000000000000000}
		},
		.info = 0x0000
	},
	{
		.name = "div0u",
		.mask =	0b1111111111111111,
		.code = 0b0000000000011001,
		.arg = {
			{.mask = 0b0000000000000000},
			{.mask = 0b0000000000000000},
			{.mask = 0b0000000000000000}
		},
		.info = 0x0000
	},
	{
		.name = "div1\tr%d,r%d",
		.mask =	0b1111000000001111,
		.code = 0b0011000000000100,
		.arg = {
			{.mask = 0b0000000011110000},
			{.mask = 0b0000111100000000},
			{.mask = 0b0000000000000000}
		},
		.info = 0x0000
	},
	{
		.name = "dmuls.l\tr%d,r%d",
		.mask =	0b1111000000001111,
		.code = 0b0011000000001101,
		.arg = {
			{.mask = 0b0000000011110000},
			{.mask = 0b0000111100000000},
			{.mask = 0b0000000000000000}
		},
		.info = 0x0000
	},
	{
		.name = "dmulu.l\tr%d,r%d",
		.mask =	0b1111000000001111,
		.code = 0b0011000000000101,
		.arg = {
			{.mask = 0b0000000011110000},
			{.mask = 0b0000111100000000},
			{.mask = 0b0000000000000000}
		},
		.info = 0x0000
	},
	{
		.name = "dt\tr%d",
		.mask =	0b1111000011111111,
		.code = 0b0100000000010000,
		.arg = {
			{.mask = 0b0000111100000000},
			{.mask = 0b0000000000000000},
			{.mask = 0b0000000000000000}
		},
		.info = 0x0000
	},
	{
		.name = "exts.b\tr%d,r%d",
		.mask =	0b1111000000001111,
		.code = 0b0110000000001110,
		.arg = {
			{.mask = 0b0000000011110000},
			{.mask = 0b0000111100000000},
			{.mask = 0b0000000000000000}
		},
		.info = 0x0000
	},
	{
		.name = "exts.w\tr%d,r%d",
		.mask =	0b1111000000001111,
		.code = 0b0110000000001111,
		.arg = {
			{.mask = 0b0000000011110000},
			{.mask = 0b0000111100000000},
			{.mask = 0b0000000000000000}
		},
		.info = 0x0000
	},
	{
		.name = "extu.b\tr%d,r%d",
		.mask =	0b1111000000001111,
		.code = 0b0110000000001100,
		.arg = {
			{.mask = 0b0000000011110000},
			{.mask = 0b0000111100000000},
			{.mask = 0b0000000000000000}
		},
		.info = 0x0000
	},
	{
		.name = "extu.w\tr%d,r%d",
		.mask =	0b1111000000001111,
		.code = 0b0110000000001101,
		.arg = {
			{.mask = 0b0000000011110000},
			{.mask = 0b0000111100000000},
			{.mask = 0b0000000000000000}
		},
		.info = 0x0000
	},
	{
		.name = "fabs\tfr%d",
		.mask =	0b1111000011111111,
		.code = 0b1111000001011101,
		.arg = {
			{.mask = 0b0000111100000000},
			{.mask = 0b0000000000000000},
			{.mask = 0b0000000000000000}
		},
		.info = 0x0000
	},
	{
		.name = "fadd\tfr%d,fr%d",
		.mask =	0b1111000000001111,
		.code = 0b1111000000000000,
		.arg = {
			{.mask = 0b0000000011110000},
			{.mask = 0b0000111100000000},
			{.mask = 0b0000000000000000}
		},
		.info = 0x0000
	},
	{
		.name = "fcmp/eq\tfr%d,fr%d",
		.mask =	0b1111000000001111,
		.code = 0b1111000000000100,
		.arg = {
			{.mask = 0b0000000011110000},
			{.mask = 0b0000111100000000},
			{.mask = 0b0000000000000000}
		},
		.info = 0x0000
	},
	{
		.name = "fcmp/gt\tfr%d,fr%d",
		.mask =	0b1111000000001111,
		.code = 0b1111000000000101,
		.arg = {
			{.mask = 0b0000000011110000},
			{.mask = 0b0000111100000000},
			{.mask = 0b0000000000000000}
		},
		.info = 0x0000
	},
	{
		.name = "fdiv\tfr%d,fr%d",
		.mask =	0b1111000000001111,
		.code = 0b1111000000000011,
		.arg = {
			{.mask = 0b0000000011110000},
			{.mask = 0b0000111100000000},
			{.mask = 0b0000000000000000}
		},
		.info = 0x0000
	},
	{
		.name = "fldi0\tfr%d",
		.mask =	0b1111000011111111,
		.code = 0b1111000010001101,
		.arg = {
			{.mask = 0b0000111100000000},
			{.mask = 0b0000000000000000},
			{.mask = 0b0000000000000000}
		},
		.info = 0x0000
	},
	{
		.name = "fldi1\tfr%d",
		.mask =	0b1111000011111111,
		.code = 0b1111000010011101,
		.arg = {
			{.mask = 0b0000111100000000},
			{.mask = 0b0000000000000000},
			{.mask = 0b0000000000000000}
		},
		.info = 0x0000
	},
	{
		.name = "flds\tfr%d,FPUL",
		.mask =	0b1111000011111111,
		.code = 0b1111000000011101,
		.arg = {
			{.mask = 0b0000111100000000},
			{.mask = 0b0000000000000000},
			{.mask = 0b0000000000000000}
		},
		.info = 0x0000
	},
	{
		.name = "float\tFPUL,fr%d",
		.mask =	0b1111000011111111,
		.code = 0b1111000000101101,
		.arg = {
			{.mask = 0b0000111100000000},
			{.mask = 0b0000000000000000},
			{.mask = 0b0000000000000000}
		},
		.info = 0x0000
	},
	{
		.name = "fmac\tFR0,fr%d,fr%d",
		.mask =	0b1111000000001111,
		.code = 0b1111000000001110,
		.arg = {
			{.mask = 0b0000000011110000},
			{.mask = 0b0000111100000000},
			{.mask = 0b0000000000000000}
		},
		.info = 0x0000
	},
	{
		.name = "fmov\tfr%d,fr%d",
		.mask =	0b1111000000001111,
		.code = 0b1111000000001100,
		.arg = {
			{.mask = 0b0000000011110000},
			{.mask = 0b0000111100000000},
			{.mask = 0b0000000000000000}
		},
		.info = 0x0000
	},
	{
		.name = "fmov.s\t@(r0,r%d),fr%d",
		.mask =	0b1111000000001111,
		.code = 0b1111000000000110,
		.arg = {
			{.mask = 0b0000000011110000},
			{.mask = 0b0000111100000000},
			{.mask = 0b0000000000000000}
		},
		.info = 0x0000
	},
	{
		.name = "fmov.s\t@r%d+,fr%d",
		.mask =	0b1111000000001111,
		.code = 0b1111000000001001,
		.arg = {
			{.mask = 0b0000000011110000},
			{.mask = 0b0000111100000000},
			{.mask = 0b0000000000000000}
		},
		.info = 0x0000
	},
	{
		.name = "fmov.s\t@r%d,fr%d",
		.mask =	0b1111000000001111,
		.code = 0b1111000000001000,
		.arg = {
			{.mask = 0b0000000011110000},
			{.mask = 0b0000111100000000},
			{.mask = 0b0000000000000000}
		},
		.info = 0x0000
	},
	{
		.name = "fmov.s\tfr%d,@(r0,r%d)",
		.mask =	0b1111000000001111,
		.code = 0b1111000000000111,
		.arg = {
			{.mask = 0b0000000011110000},
			{.mask = 0b0000111100000000},
			{.mask = 0b0000000000000000}
		},
		.info = 0x0000
	},
	{
		.name = "fmov.s\tfr%d,@-r%d",
		.mask =	0b1111000000001111,
		.code = 0b1111000000001011,
		.arg = {
			{.mask = 0b0000000011110000},
			{.mask = 0b0000111100000000},
			{.mask = 0b0000000000000000}
		},
		.info = 0x0000
	},
	{
		.name = "fmov.s\tfr%d,@r%d",
		.mask =	0b1111000000001111,
		.code = 0b1111000000001010,
		.arg = {
			{.mask = 0b0000000011110000},
			{.mask = 0b0000111100000000},
			{.mask = 0b0000000000000000}
		},
		.info = 0x0000
	},
	{
		.name = "fmul\tfr%d,fr%d",
		.mask =	0b1111000000001111,
		.code = 0b1111000000000010,
		.arg = {
			{.mask = 0b0000000011110000},
			{.mask = 0b0000111100000000},
			{.mask = 0b0000000000000000}
		},
		.info = 0x0000
	},
	{
		.name = "fneg\tfr%d",
		.mask =	0b1111000011111111,
		.code = 0b1111000001001101,
		.arg = {
			{.mask = 0b0000111100000000},
			{.mask = 0b0000000000000000},
			{.mask = 0b0000000000000000}
		},
		.info = 0x0000
	},
	{
		.name = "fsqrt\tfr%d",
		.mask =	0b1111000011111111,
		.code = 0b1111000001101101,
		.arg = {
			{.mask = 0b0000111100000000},
			{.mask = 0b0000000000000000},
			{.mask = 0b0000000000000000}
		},
		.info = 0x0000
	},
	{
		.name = "fsts\tFPUL,fr%d",
		.mask =	0b1111000011111111,
		.code = 0b1111000000001101,
		.arg = {
			{.mask = 0b0000111100000000},
			{.mask = 0b0000000000000000},
			{.mask = 0b0000000000000000}
		},
		.info = 0x0000
	},
	{
		.name = "fsub\tfr%d,fr%d",
		.mask =	0b1111000000001111,
		.code = 0b1111000000000001,
		.arg = {
			{.mask = 0b0000000011110000},
			{.mask = 0b0000111100000000},
			{.mask = 0b0000000000000000}
		},
		.info = 0x0000
	},
	{
		.name = "ftrc\tfr%d,FPUL",
		.mask =	0b1111000011111111,
		.code = 0b1111000000111101,
		.arg = {
			{.mask = 0b0000111100000000},
			{.mask = 0b0000000000000000},
			{.mask = 0b0000000000000000}
		},
		.info = 0x0000
	},
	{
		.name = "jmp\t@r%d",
		.mask =	0b1111000011111111,
		.code = 0b0100000000101011,
		.arg = {
			{.mask = 0b0000111100000000},
			{.mask = 0b0000000000000000},
			{.mask = 0b0000000000000000}
		},
		.info = 0x0000
	},
	{
		.name = "jsr\t@r%d",
		.mask =	0b1111000011111111,
		.code = 0b0100000000001011,
		.arg = {
			{.mask = 0b0000111100000000},
			{.mask = 0b0000000000000000},
			{.mask = 0b0000000000000000}
		},
		.info = 0x0000
	},
	{
		.name = "ldc\tr%d,GBR",
		.mask =	0b1111000011111111,
		.code = 0b0100000000011110,
		.arg = {
			{.mask = 0b0000111100000000},
			{.mask = 0b0000000000000000},
			{.mask = 0b0000000000000000}
		},
		.info = 0x0000
	},
	{
		.name = "ldc\tr%d,SR",
		.mask =	0b1111000011111111,
		.code = 0b0100000000001110,
		.arg = {
			{.mask = 0b0000111100000000},
			{.mask = 0b0000000000000000},
			{.mask = 0b0000000000000000}
		},
		.info = 0x0000
	},
	{
		.name = "ldc\tr%d,VBR",
		.mask =	0b1111000011111111,
		.code = 0b0100000000101110,
		.arg = {
			{.mask = 0b0000111100000000},
			{.mask = 0b0000000000000000},
			{.mask = 0b0000000000000000}
		},
		.info = 0x0000
	},
	{
		.name = "ldc\tr%d,SSR",
		.mask =	0b1111000011111111,
		.code = 0b0100000000111110,
		.arg = {
			{.mask = 0b0000111100000000},
			{.mask = 0b0000000000000000},
			{.mask = 0b0000000000000000}
		},
		.info = 0x0000
	},
	{
		.name = "ldc\tr%d,SPC",
		.mask =	0b1111000011111111,
		.code = 0b0100000001001110,
		.arg = {
			{.mask = 0b0000111100000000},
			{.mask = 0b0000000000000000},
			{.mask = 0b0000000000000000}
		},
		.info = 0x0000
	},
	{
		.name = "ldc\tr%d,MOD",
		.mask =	0b1111000011111111,
		.code = 0b0100000001011110,
		.arg = {
			{.mask = 0b0000111100000000},
			{.mask = 0b0000000000000000},
			{.mask = 0b0000000000000000}
		},
		.info = 0x0000
	},
	//FIXME: find difference between "ldc rm,RE" and "ldc rm,RS"
	{
		.name = "ldc\tr%d,RE/RS",
		.mask =	0b1111000011111111,
		.code = 0b0100000001101110,
		.arg = {
			{.mask = 0b0000111100000000},
			{.mask = 0b0000000000000000},
			{.mask = 0b0000000000000000}
		},
		.info = 0x0000
	},
	{
		.name = "ldc\tr%d,R0_BANK",
		.mask =	0b1111000011111111,
		.code = 0b0100000010001110,
		.arg = {
			{.mask = 0b0000111100000000},
			{.mask = 0b0000000000000000},
			{.mask = 0b0000000000000000}
		},
		.info = 0x0000
	},
	{
		.name = "ldc\tr%d,R1_BANK",
		.mask =	0b1111000011111111,
		.code = 0b0100000010011110,
		.arg = {
			{.mask = 0b0000111100000000},
			{.mask = 0b0000000000000000},
			{.mask = 0b0000000000000000}
		},
		.info = 0x0000
	},
	{
		.name = "ldc\tr%d,R2_BANK",
		.mask =	0b1111000011111111,
		.code = 0b0100000010101110,
		.arg = {
			{.mask = 0b0000111100000000},
			{.mask = 0b0000000000000000},
			{.mask = 0b0000000000000000}
		},
		.info = 0x0000
	},
	{
		.name = "ldc\tr%d,R3_BANK",
		.mask =	0b1111000011111111,
		.code = 0b0100000010111110,
		.arg = {
			{.mask = 0b0000111100000000},
			{.mask = 0b0000000000000000},
			{.mask = 0b0000000000000000}
		},
		.info = 0x0000
	},
	{
		.name = "ldc\tr%d,R4_BANK",
		.mask =	0b1111000011111111,
		.code = 0b0100000011001110,
		.arg = {
			{.mask = 0b0000111100000000},
			{.mask = 0b0000000000000000},
			{.mask = 0b0000000000000000}
		},
		.info = 0x0000
	},
	{
		.name = "ldc\tr%d,R5_BANK",
		.mask =	0b1111000011111111,
		.code = 0b0100000011011110,
		.arg = {
			{.mask = 0b0000111100000000},
			{.mask = 0b0000000000000000},
			{.mask = 0b0000000000000000}
		},
		.info = 0x0000
	},
	{
		.name = "ldc\tr%d,R6_BANK",
		.mask =	0b1111000011111111,
		.code = 0b0100000011101110,
		.arg = {
			{.mask = 0b0000111100000000},
			{.mask = 0b0000000000000000},
			{.mask = 0b0000000000000000}
		},
		.info = 0x0000
	},
	{
		.name = "ldc\tr%d,R7_BANK",
		.mask =	0b1111000011111111,
		.code = 0b0100000011111110,
		.arg = {
			{.mask = 0b0000111100000000},
			{.mask = 0b0000000000000000},
			{.mask = 0b0000000000000000}
		},
		.info = 0x0000
	},
	{
		.name = "ldc.l\t@r%d+,GBR",
		.mask =	0b1111000011111111,
		.code = 0b0100000000010111,
		.arg = {
			{.mask = 0b0000111100000000},
			{.mask = 0b0000000000000000},
			{.mask = 0b0000000000000000}
		},
		.info = 0x0000
	},
	{
		.name = "ldc.l\t@r%d+,SR",
		.mask =	0b1111000011111111,
		.code = 0b0100000000000111,
		.arg = {
			{.mask = 0b0000111100000000},
			{.mask = 0b0000000000000000},
			{.mask = 0b0000000000000000}
		},
		.info = 0x0000
	},
	{
		.name = "ldc.l\t@r%d+,VBR",
		.mask =	0b1111000011111111,
		.code = 0b0100000000100111,
		.arg = {
			{.mask = 0b0000111100000000},
			{.mask = 0b0000000000000000},
			{.mask = 0b0000000000000000}
		},
		.info = 0x0000
	},
	{
		.name = "ldc.l\t@r%d+,SSR",
		.mask =	0b1111000011111111,
		.code = 0b0100000000110111,
		.arg = {
			{.mask = 0b0000111100000000},
			{.mask = 0b0000000000000000},
			{.mask = 0b0000000000000000}
		},
		.info = 0x0000
	},
	{
		.name = "ldc.l\t@r%d+,SPC",
		.mask =	0b1111000011111111,
		.code = 0b0100000001000111,
		.arg = {
			{.mask = 0b0000111100000000},
			{.mask = 0b0000000000000000},
			{.mask = 0b0000000000000000}
		},
		.info = 0x0000
	},
	{
		.name = "ldc.l\t@r%d+,MOD",
		.mask =	0b1111000011111111,
		.code = 0b0100000001010111,
		.arg = {
			{.mask = 0b0000111100000000},
			{.mask = 0b0000000000000000},
			{.mask = 0b0000000000000000}
		},
		.info = 0x0000
	},
	{
		.name = "ldc.l\t@r%d+,RE",
		.mask =	0b1111000011111111,
		.code = 0b0100000001110111,
		.arg = {
			{.mask = 0b0000111100000000},
			{.mask = 0b0000000000000000},
			{.mask = 0b0000000000000000}
		},
		.info = 0x0000
	},
	{
		.name = "ldc.l\t@r%d+,RS",
		.mask =	0b1111000011111111,
		.code = 0b0100000001100111,
		.arg = {
			{.mask = 0b0000111100000000},
			{.mask = 0b0000000000000000},
			{.mask = 0b0000000000000000}
		},
		.info = 0x0000
	},
	{
		.name = "ldc.l\t@r%d+,R0_BANK",
		.mask =	0b1111000011111111,
		.code = 0b0100000010000111,
		.arg = {
			{.mask = 0b0000111100000000},
			{.mask = 0b0000000000000000},
			{.mask = 0b0000000000000000}
		},
		.info = 0x0000
	},
	{
		.name = "ldc.l\t@r%d+,R1_BANK",
		.mask =	0b1111000011111111,
		.code = 0b0100000010010111,
		.arg = {
			{.mask = 0b0000111100000000},
			{.mask = 0b0000000000000000},
			{.mask = 0b0000000000000000}
		},
		.info = 0x0000
	},
	{
		.name = "ldc.l\t@r%d+,R2_BANK",
		.mask =	0b1111000011111111,
		.code = 0b0100000010100111,
		.arg = {
			{.mask = 0b0000111100000000},
			{.mask = 0b0000000000000000},
			{.mask = 0b0000000000000000}
		},
		.info = 0x0000
	},
	{
		.name = "ldc.l\t@%d+,R3_BANK",
		.mask =	0b1111000011111111,
		.code = 0b0100000010110111,
		.arg = {
			{.mask = 0b0000111100000000},
			{.mask = 0b0000000000000000},
			{.mask = 0b0000000000000000}
		},
		.info = 0x0000
	},
	{
		.name = "ldc.l\t@r%d+,R4_BANK",
		.mask =	0b1111000011111111,
		.code = 0b0100000011000111,
		.arg = {
			{.mask = 0b0000111100000000},
			{.mask = 0b0000000000000000},
			{.mask = 0b0000000000000000}
		},
		.info = 0x0000
	},
	{
		.name = "ldc.l\t@r%d+,R5_BANK",
		.mask =	0b1111000011111111,
		.code = 0b0100000011010111,
		.arg = {
			{.mask = 0b0000111100000000},
			{.mask = 0b0000000000000000},
			{.mask = 0b0000000000000000}
		},
		.info = 0x0000
	},
	{
		.name = "ldc.l\t@r%d+,R7_BANK",
		.mask =	0b1111000011111111,
		.code = 0b0100000011100111,
		.arg = {
			{.mask = 0b0000111100000000},
			{.mask = 0b0000000000000000},
			{.mask = 0b0000000000000000}
		},
		.info = 0x0000
	},
	{
		.name = "ldc.l\t@r%d+,R7_BANK",
		.mask =	0b1111000011111111,
		.code = 0b0100000011110111,
		.arg = {
			{.mask = 0b0000111100000000},
			{.mask = 0b0000000000000000},
			{.mask = 0b0000000000000000}
		},
		.info = 0x0000
	},
	{
		.name = "ldre\t@(0x%x,PC)",
		.mask =	0b1111111100000000,
		.code = 0b1000111000000000,
		.arg = {
			{.mask = 0b0000000011111111},
			{.mask = 0b0000000000000000},
			{.mask = 0b0000000000000000}
		},
		.info = 0x0000
	},
	{
		.name = "ldrs\t@(0x%x,PC)",
		.mask =	0b1111111100000000,
		.code = 0b1000110000000000,
		.arg = {
			{.mask = 0b0000000011111111},
			{.mask = 0b0000000000000000},
			{.mask = 0b0000000000000000}
		},
		.info = 0x0000
	},
	{
		.name = "lds\tr%d,FPSCR",
		.mask =	0b1111000011111111,
		.code = 0b0100000001101010,
		.arg = {
			{.mask = 0b0000111100000000},
			{.mask = 0b0000000000000000},
			{.mask = 0b0000000000000000}
		},
		.info = 0x0000
	},
	{
		.name = "lds\tr%d,FPUL",
		.mask =	0b1111000011111111,
		.code = 0b0100000001011010,
		.arg = {
			{.mask = 0b0000111100000000},
			{.mask = 0b0000000000000000},
			{.mask = 0b0000000000000000}
		},
		.info = 0x0000
	},
	{
		.name = "lds\tr%d,MACH",
		.mask =	0b1111000011111111,
		.code = 0b0100000000001010,
		.arg = {
			{.mask = 0b0000111100000000},
			{.mask = 0b0000000000000000},
			{.mask = 0b0000000000000000}
		},
		.info = 0x0000
	},
	{
		.name = "lds\tr%d,MACL",
		.mask =	0b1111000011111111,
		.code = 0b0100000000011010,
		.arg = {
			{.mask = 0b0000111100000000},
			{.mask = 0b0000000000000000},
			{.mask = 0b0000000000000000}
		},
		.info = 0x0000
	},
	{
		.name = "lds\tr%d,PR",
		.mask =	0b1111000011111111,
		.code = 0b0100000000101010,
		.arg = {
			{.mask = 0b0000111100000000},
			{.mask = 0b0000000000000000},
			{.mask = 0b0000000000000000}
		},
		.info = 0x0000
	},
	{
		.name = "lds\tr%d,A0",
		.mask =	0b1111000011111111,
		.code = 0b0100000001101010,
		.arg = {
			{.mask = 0b0000111100000000},
			{.mask = 0b0000000000000000},
			{.mask = 0b0000000000000000}
		},
		.info = 0x0000
	},
	{
		.name = "lds\tr%d,DSR",
		.mask =	0b1111000011111111,
		.code = 0b0100000001111010,
		.arg = {
			{.mask = 0b0000111100000000},
			{.mask = 0b0000000000000000},
			{.mask = 0b0000000000000000}
		},
		.info = 0x0000
	},
	{
		.name = "lds\tr%d,X0",
		.mask =	0b1111000011111111,
		.code = 0b0100000010001010,
		.arg = {
			{.mask = 0b0000111100000000},
			{.mask = 0b0000000000000000},
			{.mask = 0b0000000000000000}
		},
		.info = 0x0000
	},
	{
		.name = "lds\tr%d,X1",
		.mask =	0b1111000011111111,
		.code = 0b0100000010011010,
		.arg = {
			{.mask = 0b0000111100000000},
			{.mask = 0b0000000000000000},
			{.mask = 0b0000000000000000}
		},
		.info = 0x0000
	},
	{
		.name = "lds\tr%d,Y0",
		.mask =	0b1111000011111111,
		.code = 0b0100000010101010,
		.arg = {
			{.mask = 0b0000111100000000},
			{.mask = 0b0000000000000000},
			{.mask = 0b0000000000000000}
		},
		.info = 0x0000
	},
	{
		.name = "lds\tr%d,Y1",
		.mask =	0b1111000011111111,
		.code = 0b0100000010111010,
		.arg = {
			{.mask = 0b0000111100000000},
			{.mask = 0b0000000000000000},
			{.mask = 0b0000000000000000}
		},
		.info = 0x0000
	},
	{
		.name = "lds.l\t@r%d+,FPSCR",
		.mask =	0b1111000011111111,
		.code = 0b0100000001100110,
		.arg = {
			{.mask = 0b0000111100000000},
			{.mask = 0b0000000000000000},
			{.mask = 0b0000000000000000}
		},
		.info = 0x0000
	},
	{
		.name = "lds.l\t@r%d+,FPUL",
		.mask =	0b1111000011111111,
		.code = 0b0100000001010110,
		.arg = {
			{.mask = 0b0000111100000000},
			{.mask = 0b0000000000000000},
			{.mask = 0b0000000000000000}
		},
		.info = 0x0000
	},
	{
		.name = "lds.l\t@r%d+,MACH",
		.mask =	0b1111000011111111,
		.code = 0b0100000000000110,
		.arg = {
			{.mask = 0b0000111100000000},
			{.mask = 0b0000000000000000},
			{.mask = 0b0000000000000000}
		},
		.info = 0x0000
	},
	{
		.name = "lds.l\t@r%d+,MACL",
		.mask =	0b1111000011111111,
		.code = 0b0100000000010110,
		.arg = {
			{.mask = 0b0000111100000000},
			{.mask = 0b0000000000000000},
			{.mask = 0b0000000000000000}
		},
		.info = 0x0000
	},
	{
		.name = "lds.l\t@r%d+,PR",
		.mask =	0b1111000011111111,
		.code = 0b0100000000100110,
		.arg = {
			{.mask = 0b0000111100000000},
			{.mask = 0b0000000000000000},
			{.mask = 0b0000000000000000}
		},
		.info = 0x0000
	},
	{
		.name = "lds.l\t@r%d+,DSR",
		.mask =	0b1111000011111111,
		.code = 0b0100000001100110,
		.arg = {
			{.mask = 0b0000111100000000},
			{.mask = 0b0000000000000000},
			{.mask = 0b0000000000000000}
		},
		.info = 0x0000
	},
	{
		.name = "lds.l\t@r%d+,A0",
		.mask =	0b1111000011111111,
		.code = 0b0100000001110110,
		.arg = {
			{.mask = 0b0000111100000000},
			{.mask = 0b0000000000000000},
			{.mask = 0b0000000000000000}
		},
		.info = 0x0000
	},
	{
		.name = "lds.l\t@r%d+,X0",
		.mask =	0b1111000011111111,
		.code = 0b0100000001000110,
		.arg = {
			{.mask = 0b0000111100000000},
			{.mask = 0b0000000000000000},
			{.mask = 0b0000000000000000}
		},
		.info = 0x0000
	},
	{
		.name = "lds.l\t@r%d+,X1",
		.mask =	0b1111000011111111,
		.code = 0b0100000010010110,
		.arg = {
			{.mask = 0b0000111100000000},
			{.mask = 0b0000000000000000},
			{.mask = 0b0000000000000000}
		},
		.info = 0x0000
	},
	{
		.name = "lds.l\t@r%d+,Y0",
		.mask =	0b1111000011111111,
		.code = 0b0100000010100110,
		.arg = {
			{.mask = 0b0000111100000000},
			{.mask = 0b0000000000000000},
			{.mask = 0b0000000000000000}
		},
		.info = 0x0000
	},
	{
		.name = "lds.l\t@r%d+,Y1",
		.mask =	0b1111000011111111,
		.code = 0b0100000010110110,
		.arg = {
			{.mask = 0b0000111100000000},
			{.mask = 0b0000000000000000},
			{.mask = 0b0000000000000000}
		},
		.info = 0x0000
	},
	{
		.name = "ldtlb",
		.mask =	0b1111111111111111,
		.code = 0b0000000000111000,
		.arg = {
			{.mask = 0b0000000000000000},
			{.mask = 0b0000000000000000},
			{.mask = 0b0000000000000000}
		},
		.info = 0x0000
	},
	{
		.name = "mac.l\t@r%d+,@r%d+",
		.mask =	0b1111000000001111,
		.code = 0b0000000000001111,
		.arg = {
			{.mask = 0b0000000011110000},
			{.mask = 0b0000111100000000},
			{.mask = 0b0000000000000000}
		},
		.info = 0x0000
	},
	{
		.name = "mac.w\t@r%d+,@r%d+",
		.mask =	0b1111000000001111,
		.code = 0b0100000000001111,
		.arg = {
			{.mask = 0b0000000011110000},
			{.mask = 0b0000111100000000},
			{.mask = 0b0000000000000000}
		},
		.info = 0x0000
	},
	{
		.name = "mov\t0x%.8x,r%d",
		.mask =	0b1111000000000000,
		.code = 0b1110000000000000,
		.arg = {
			{.mask = 0b0000000011111111},
			{.mask = 0b0000111100000000},
			{.mask = 0b0000000000000000}
		},
		.info = 0x0000
	},
	{
		.name = "mov\tr%d,r%d",
		.mask =	0b1111000000001111,
		.code = 0b0110000000000011,
		.arg = {
			{.mask = 0b0000000011110000},
			{.mask = 0b0000111100000000},
			{.mask = 0b0000000000000000}
		},
		.info = 0x0000
	},
	{
		.name = "mov.b\t@(0x%x,GBR),r0",
		.mask =	0b1111111100000000,
		.code = 0b1100010000000000,
		.arg = {
			{.mask = 0b0000000011111111},
			{.mask = 0b0000000000000000},
			{.mask = 0b0000000000000000}
		},
		.info = 0x0000
	},
	{
		.name = "mov.b\t@(0x%x,r%d),r0",
		.mask =	0b1111111100000000,
		.code = 0b1000010000000000,
		.arg = {
			{.mask = 0b0000000000001111},
			{.mask = 0b0000000011110000},
			{.mask = 0b0000000000000000}
		},
		.info = 0x0000
	},
	{
		.name = "mov.b\t@(r0,r%d),r%d",
		.mask =	0b1111000000001111,
		.code = 0b0000000000001100,
		.arg = {
			{.mask = 0b0000000011110000},
			{.mask = 0b0000111100000000},
			{.mask = 0b0000000000000000}
		},
		.info = 0x0000
	},
	{
		.name = "mov.b\t@r%d+,r%d",
		.mask =	0b1111000000001111,
		.code = 0b0110000000000100,
		.arg = {
			{.mask = 0b0000000011110000},
			{.mask = 0b0000111100000000},
			{.mask = 0b0000000000000000}
		},
		.info = 0x0000
	},
	{
		.name = "mov.b\t@r%d,r%d",
		.mask =	0b1111000000001111,
		.code = 0b0110000000000000,
		.arg = {
			{.mask = 0b0000000011110000},
			{.mask = 0b0000111100000000},
			{.mask = 0b0000000000000000}
		},
		.info = 0x0000
	},
	{
		.name = "mov.b\tr0,@(0x%x,GBR)",
		.mask =	0b1111111100000000,
		.code = 0b1100000000000000,
		.arg = {
			{.mask = 0b0000000011111111},
			{.mask = 0b0000000000000000},
			{.mask = 0b0000000000000000}
		},
		.info = 0x0000
	},
	{
		.name = "mov.b\tr0,@(0x%x,r%d)",
		.mask =	0b1111111100000000,
		.code = 0b1000000000000000,
		.arg = {
			{.mask = 0b0000000000001111},
			{.mask = 0b0000000011110000},
			{.mask = 0b0000000000000000}
		},
		.info = 0x0000
	},
	{
		.name = "mov.b\tr%d,@(r0,r%d)",
		.mask =	0b1111000000001111,
		.code = 0b0000000000000100,
		.arg = {
			{.mask = 0b0000000011110000},
			{.mask = 0b0000111100000000},
			{.mask = 0b0000000000000000}
		},
		.info = 0x0000
	},
	{
		.name = "mov.b\tr%d,@-r%d",
		.mask =	0b1111000000001111,
		.code = 0b0010000000000100,
		.arg = {
			{.mask = 0b0000000011110000},
			{.mask = 0b0000111100000000},
			{.mask = 0b0000000000000000}
		},
		.info = 0x0000
	},
	{
		.name = "mov.b\tr%d,@r%d",
		.mask =	0b1111000000001111,
		.code = 0b0010000000000000,
		.arg = {
			{.mask = 0b0000000011110000},
			{.mask = 0b0000111100000000},
			{.mask = 0b0000000000000000}
		},
		.info = 0x0000
	},
	{
		.name = "mov.l\t@(0x%x,GBR),r0",
		.mask =	0b1111111100000000,
		.code = 0b1100011000000000,
		.arg = {
			{.mask = 0b0000000011111111},
			{.mask = 0b0000000000000000},
			{.mask = 0b0000000000000000}
		},
		.info = 0x0000
	},
	{
		.name = "mov.l\t0x%.8x,r%d",
		.mask =	0b1111000000000000,
		.code = 0b1101000000000000,
		.arg = {
			{.mask = 0b0000000011111111},
			{.mask = 0b0000111100000000},
			{.mask = 0b0000000000000000}
		},
		.info = OPCODE_INFO_SET(OPCODE_ARG_0, OPCODE_ACTION_PC_LONG, OPCODE_ARG_UNSIGNED)
	},
	{
		.name = "mov.l\t@(%d,r%d),r%d",
		.mask =	0b1111000000000000,
		.code = 0b0101000000000000,
		.arg = {
			{.mask = 0b0000000000001111},
			{.mask = 0b0000000011110000},
			{.mask = 0b0000111100000000}
		},
		.info = OPCODE_INFO_SET(OPCODE_ARG_0, OPCODE_ACTION_DISP_LONG, OPCODE_ARG_UNSIGNED)
	},
	{
		.name = "mov.l\t@(r0,r%d),r%d",
		.mask =	0b1111000000001111,
		.code = 0b0000000000001110,
		.arg = {
			{.mask = 0b0000000011110000},
			{.mask = 0b0000111100000000},
			{.mask = 0b0000000000000000}
		},
		.info = 0x0000
	},
	{
		.name = "mov.l\t@r%d+,r%d",
		.mask =	0b1111000000001111,
		.code = 0b0110000000000110,
		.arg = {
			{.mask = 0b0000000011110000},
			{.mask = 0b0000111100000000},
			{.mask = 0b0000000000000000}
		},
		.info = 0x0000
	},
	{
		.name = "mov.l\t@r%d,r%d",
		.mask =	0b1111000000001111,
		.code = 0b0110000000000010,
		.arg = {
			{.mask = 0b0000000011110000},
			{.mask = 0b0000111100000000},
			{.mask = 0b0000000000000000}
		},
		.info = 0x0000
	},
	{
		.name = "mov.l\tr0,@(0x%x,GBR)",
		.mask =	0b1111111100000000,
		.code = 0b1100001000000000,
		.arg = {
			{.mask = 0b0000000011111111},
			{.mask = 0b0000000000000000},
			{.mask = 0b0000000000000000}
		},
		.info = 0x0000
	},
	{
		.name = "mov.l\tr%d,@(%d,r%d)",
		.mask =	0b1111000000000000,
		.code = 0b0001000000000000,
		.arg = {
			{.mask = 0b0000000011110000},
			{.mask = 0b0000000000001111},
			{.mask = 0b0000111100000000}
		},
		.info = OPCODE_INFO_SET(OPCODE_ARG_1, OPCODE_ACTION_DISP_LONG, OPCODE_ARG_UNSIGNED)
	},
	{
		.name = "mov.l\tr%d,@(r0,r%d)",
		.mask =	0b1111000000001111,
		.code = 0b0000000000000110,
		.arg = {
			{.mask = 0b0000000011110000},
			{.mask = 0b0000111100000000},
			{.mask = 0b0000000000000000}
		},
		.info = 0x0000
	},
	{
		.name = "mov.l\tr%d,@-r%d",
		.mask =	0b1111000000001111,
		.code = 0b0010000000000110,
		.arg = {
			{.mask = 0b0000000011110000},
			{.mask = 0b0000111100000000},
			{.mask = 0b0000000000000000}
		},
		.info = 0x0000
	},
	{
		.name = "mov.l\tr%d,@r%d",
		.mask =	0b1111000000001111,
		.code = 0b0010000000000010,
		.arg = {
			{.mask = 0b0000000011110000},
			{.mask = 0b0000111100000000},
			{.mask = 0b0000000000000000}
		},
		.info = 0x0000
	},
	{
		.name = "mov.w\t@(0x%x,GBR),r0",
		.mask =	0b1111111100000000,
		.code = 0b1100010100000000,
		.arg = {
			{.mask = 0b0000000011111111},
			{.mask = 0b0000000000000000},
			{.mask = 0b0000000000000000}
		},
		.info = 0x0000
	},
	{
		.name = "mov.w\t0x%.4x,r%d",
		.mask =	0b1111000000000000,
		.code = 0b1001000000000000,
		.arg = {
			{.mask = 0b0000000011111111},
			{.mask = 0b0000111100000000},
			{.mask = 0b0000000000000000}
		},
		.info = OPCODE_INFO_SET(OPCODE_ARG_0, OPCODE_ACTION_PC_WORD, OPCODE_ARG_UNSIGNED)
	},
	{
		.name = "mov.w\t@(%#x,r%d),r0",
		.mask =	0b1111111100000000,
		.code = 0b1000010100000000,
		.arg = {
			{.mask = 0b0000000000001111},
			{.mask = 0b0000000011110000},
			{.mask = 0b0000000000000000}
		},
		.info = 0x0000
	},
	{
		.name = "mov.w\t@(r0,r%d),r%d",
		.mask =	0b1111000000001111,
		.code = 0b0000000000001101,
		.arg = {
			{.mask = 0b0000000011110000},
			{.mask = 0b0000111100000000},
			{.mask = 0b0000000000000000}
		},
		.info = 0x0000
	},
	{
		.name = "mov.w\t@r%d+,r%d",
		.mask =	0b1111000000001111,
		.code = 0b0110000000000101,
		.arg = {
			{.mask = 0b0000000011110000},
			{.mask = 0b0000111100000000},
			{.mask = 0b0000000000000000}
		},
		.info = 0x0000
	},
	{
		.name = "mov.w\t@r%d,r%d",
		.mask =	0b1111000000001111,
		.code = 0b0110000000000001,
		.arg = {
			{.mask = 0b0000000011110000},
			{.mask = 0b0000111100000000},
			{.mask = 0b0000000000000000}
		},
		.info = 0x0000
	},
	{
		.name = "mov.w\tr0,@(0x%x,GBR)",
		.mask =	0b1111111100000000,
		.code = 0b1100000100000000,
		.arg = {
			{.mask = 0b0000000011111111},
			{.mask = 0b0000000000000000},
			{.mask = 0b0000000000000000}
		},
		.info = 0x0000
	},
	{
		.name = "mov.w\tr0,@(0x%x,,r%d)",
		.mask =	0b1111111100000000,
		.code = 0b1000000100000000,
		.arg = {
			{.mask = 0b0000000000001111},
			{.mask = 0b0000000011110000},
			{.mask = 0b0000000000000000}
		},
		.info = 0x0000
	},
	{
		.name = "mov.w\tr%d,@(r0,r%d)",
		.mask =	0b1111000000001111,
		.code = 0b0000000000000101,
		.arg = {
			{.mask = 0b0000000011110000},
			{.mask = 0b0000111100000000},
			{.mask = 0b0000000000000000}
		},
		.info = 0x0000
	},
	{
		.name = "mov.w\tr%d,@-r%d",
		.mask =	0b1111000000001111,
		.code = 0b0010000000000101,
		.arg = {
			{.mask = 0b0000000011110000},
			{.mask = 0b0000111100000000},
			{.mask = 0b0000000000000000}
		},
		.info = 0x0000
	},
	{
		.name = "mov.w\tr%d,@r%d",
		.mask =	0b1111000000001111,
		.code = 0b0010000000000001,
		.arg = {
			{.mask = 0b0000000011110000},
			{.mask = 0b0000111100000000},
			{.mask = 0b0000000000000000}
		},
		.info = 0x0000
	},
	{
		.name = "mova\t@(0x%x,PC),r0",
		.mask =	0b1111111100000000,
		.code = 0b1100011100000000,
		.arg = {
			{.mask = 0b0000000011111111},
			{.mask = 0b0000000000000000},
			{.mask = 0b0000000000000000}
		},
		.info = 0x0000
	},
	{
		.name = "movt\tr%d",
		.mask =	0b1111000011111111,
		.code = 0b0000000000101001,
		.arg = {
			{.mask = 0b0000111100000000},
			{.mask = 0b0000000000000000},
			{.mask = 0b0000000000000000}
		},
		.info = 0x0000
	},
	{
		.name = "mul.l\tr%d,r%d",
		.mask =	0b1111000000001111,
		.code = 0b0000000000000111,
		.arg = {
			{.mask = 0b0000000011110000},
			{.mask = 0b0000111100000000},
			{.mask = 0b0000000000000000}
		},
		.info = 0x0000
	},
	{
		.name = "muls.w\tr%d,r%d",
		.mask =	0b1111000000001111,
		.code = 0b0010000000001111,
		.arg = {
			{.mask = 0b0000000011110000},
			{.mask = 0b0000111100000000},
			{.mask = 0b0000000000000000}
		},
		.info = 0x0000
	},
	{
		.name = "mulu.w\tr%d,r%d",
		.mask =	0b1111000000001111,
		.code = 0b0010000000001110,
		.arg = {
			{.mask = 0b0000000011110000},
			{.mask = 0b0000111100000000},
			{.mask = 0b0000000000000000}
		},
		.info = 0x0000
	},
	{
		.name = "neg\tr%d,r%d",
		.mask =	0b1111000000001111,
		.code = 0b0110000000001011,
		.arg = {
			{.mask = 0b0000000011110000},
			{.mask = 0b0000111100000000},
			{.mask = 0b0000000000000000}
		},
		.info = 0x0000
	},
	{
		.name = "negc\tr%d,r%d",
		.mask =	0b1111000000001111,
		.code = 0b0110000000001010,
		.arg = {
			{.mask = 0b0000000011110000},
			{.mask = 0b0000111100000000},
			{.mask = 0b0000000000000000}
		},
		.info = 0x0000
	},
	{
		.name = "nop",
		.mask =	0b1111111111111111,
		.code = 0b0000000000001001,
		.arg = {
			{.mask = 0b0000000000000000},
			{.mask = 0b0000000000000000},
			{.mask = 0b0000000000000000}
		},
		.info = 0x0000
	},
	{
		.name = "not\tr%d,r%d",
		.mask =	0b1111000000001111,
		.code = 0b0110000000000111,
		.arg = {
			{.mask = 0b0000000011110000},
			{.mask = 0b0000111100000000},
			{.mask = 0b0000000000000000}
		},
		.info = 0x0000
	},
	{
		.name = "or\t0x%x,r0",
		.mask =	0b1111111100000000,
		.code = 0b1100101100000000,
		.arg = {
			{.mask = 0b0000000011111111},
			{.mask = 0b0000000000000000},
			{.mask = 0b0000000000000000}
		},
		.info = 0x0000
	},
	{
		.name = "or\tr%d,r%d",
		.mask =	0b1111000000001111,
		.code = 0b0010000000001011,
		.arg = {
			{.mask = 0b0000000011110000},
			{.mask = 0b0000111100000000},
			{.mask = 0b0000000000000000}
		},
		.info = 0x0000
	},
	{
		.name = "or.b\t0x%x,@(r0,GBR)",
		.mask =	0b1111111100000000,
		.code = 0b1100111100000000,
		.arg = {
			{.mask = 0b0000000011111111},
			{.mask = 0b0000000000000000},
			{.mask = 0b0000000000000000}
		},
		.info = 0x0000
	},
	{
		.name = "pref\t@r%d",
		.mask =	0b1111000011111111,
		.code = 0b0000000010000011,
		.arg = {
			{.mask = 0b0000111100000000},
			{.mask = 0b0000000000000000},
			{.mask = 0b0000000000000000}
		},
		.info = 0x0000
	},
	{
		.name = "rotcl\tr%d",
		.mask =	0b1111000011111111,
		.code = 0b0100000000100100,
		.arg = {
			{.mask = 0b0000111100000000},
			{.mask = 0b0000000000000000},
			{.mask = 0b0000000000000000}
		},
		.info = 0x0000
	},
	{
		.name = "rotcr\tr%d",
		.mask =	0b1111000011111111,
		.code = 0b0100000000100101,
		.arg = {
			{.mask = 0b0000111100000000},
			{.mask = 0b0000000000000000},
			{.mask = 0b0000000000000000}
		},
		.info = 0x0000
	},
	{
		.name = "rotl\tr%d",
		.mask =	0b1111000011111111,
		.code = 0b0100000000000100,
		.arg = {
			{.mask = 0b0000111100000000},
			{.mask = 0b0000000000000000},
			{.mask = 0b0000000000000000}
		},
		.info = 0x0000
	},
	{
		.name = "rotr\tr%d",
		.mask =	0b1111000011111111,
		.code = 0b0100000000000101,
		.arg = {
			{.mask = 0b0000111100000000},
			{.mask = 0b0000000000000000},
			{.mask = 0b0000000000000000}
		},
		.info = 0x0000
	},
	{
		.name = "rte",
		.mask =	0b1111111111111111,
		.code = 0b0000000000101011,
		.arg = {
			{.mask = 0b0000000000000000},
			{.mask = 0b0000000000000000},
			{.mask = 0b0000000000000000}
		},
		.info = 0x0000
	},
	{
		.name = "rts",
		.mask =	0b1111111111111111,
		.code = 0b0000000000001011,
		.arg = {
			{.mask = 0b0000000000000000},
			{.mask = 0b0000000000000000},
			{.mask = 0b0000000000000000}
		},
		.info = 0x0000
	},
	{
		.name = "setrc\tr%d",
		.mask =	0b1111000011111111,
		.code = 0b0100000000010100,
		.arg = {
			{.mask = 0b0000111100000000},
			{.mask = 0b0000000000000000},
			{.mask = 0b0000000000000000}
		},
		.info = 0x0000
	},
	{
		.name = "setrc\t0x%x",
		.mask =	0b1111111100000000,
		.code = 0b1000001000000000,
		.arg = {
			{.mask = 0b0000000011111111},
			{.mask = 0b0000000000000000},
			{.mask = 0b0000000000000000}
		},
		.info = 0x0000
	},
	{
		.name = "sets",
		.mask =	0b1111111111111111,
		.code = 0b0000000001011000,
		.arg = {
			{.mask = 0b0000000000000000},
			{.mask = 0b0000000000000000},
			{.mask = 0b0000000000000000}
		},
		.info = 0x0000
	},
	{
		.name = "sett",
		.mask =	0b1111111111111111,
		.code = 0b0000000000011000,
		.arg = {
			{.mask = 0b0000000000000000},
			{.mask = 0b0000000000000000},
			{.mask = 0b0000000000000000}
		},
		.info = 0x0000
	},
	{
		.name = "shad\tr%d,r%d",
		.mask =	0b1111000000001111,
		.code = 0b0100000000001100,
		.arg = {
			{.mask = 0b0000000011110000},
			{.mask = 0b0000111100000000},
			{.mask = 0b0000000000000000}
		},
		.info = 0x0000
	},
	{
		.name = "shal\tr%d",
		.mask =	0b1111000011111111,
		.code = 0b0100000000100000,
		.arg = {
			{.mask = 0b0000111100000000},
			{.mask = 0b0000000000000000},
			{.mask = 0b0000000000000000}
		},
		.info = 0x0000
	},
	{
		.name = "shar\tr%d",
		.mask =	0b1111000011111111,
		.code = 0b0100000000100001,
		.arg = {
			{.mask = 0b0000111100000000},
			{.mask = 0b0000000000000000},
			{.mask = 0b0000000000000000}
		},
		.info = 0x0000
	},
	{
		.name = "shld\tr%d,r%d",
		.mask =	0b1111000000001111,
		.code = 0b0100000000001101,
		.arg = {
			{.mask = 0b0000000011110000},
			{.mask = 0b0000111100000000},
			{.mask = 0b0000000000000000}
		},
		.info = 0x0000
	},
	{
		.name = "shll\tr%d",
		.mask =	0b1111000011111111,
		.code = 0b0100000000000000,
		.arg = {
			{.mask = 0b0000111100000000},
			{.mask = 0b0000000000000000},
			{.mask = 0b0000000000000000}
		},
		.info = 0x0000
	},
	{
		.name = "shll2\tr%d",
		.mask =	0b1111000011111111,
		.code = 0b0100000000001000,
		.arg = {
			{.mask = 0b0000111100000000},
			{.mask = 0b0000000000000000},
			{.mask = 0b0000000000000000}
		},
		.info = 0x0000
	},
	{
		.name = "shll8\tr%d",
		.mask =	0b1111000011111111,
		.code = 0b0100000000011000,
		.arg = {
			{.mask = 0b0000111100000000},
			{.mask = 0b0000000000000000},
			{.mask = 0b0000000000000000}
		},
		.info = 0x0000
	},
	{
		.name = "shll16\tr%d",
		.mask =	0b1111000011111111,
		.code = 0b0100000000101000,
		.arg = {
			{.mask = 0b0000111100000000},
			{.mask = 0b0000000000000000},
			{.mask = 0b0000000000000000}
		},
		.info = 0x0000
	},
	{
		.name = "shlr\tr%d",
		.mask =	0b1111000011111111,
		.code = 0b0100000000000001,
		.arg = {
			{.mask = 0b0000111100000000},
			{.mask = 0b0000000000000000},
			{.mask = 0b0000000000000000}
		},
		.info = 0x0000
	},
	{
		.name = "shlr2\tr%d",
		.mask =	0b1111000011111111,
		.code = 0b0100000000001001,
		.arg = {
			{.mask = 0b0000111100000000},
			{.mask = 0b0000000000000000},
			{.mask = 0b0000000000000000}
		},
		.info = 0x0000
	},
	{
		.name = "shlr8\tr%d",
		.mask =	0b1111000011111111,
		.code = 0b0100000000011001,
		.arg = {
			{.mask = 0b0000111100000000},
			{.mask = 0b0000000000000000},
			{.mask = 0b0000000000000000}
		},
		.info = 0x0000
	},
	{
		.name = "shlr16\tr%d",
		.mask =	0b1111000011111111,
		.code = 0b0100000000101001,
		.arg = {
			{.mask = 0b0000111100000000},
			{.mask = 0b0000000000000000},
			{.mask = 0b0000000000000000}
		},
		.info = 0x0000
	},
	{
		.name = "sleep",
		.mask =	0b1111111111111111,
		.code = 0b0000000000011011,
		.arg = {
			{.mask = 0b0000000000000000},
			{.mask = 0b0000000000000000},
			{.mask = 0b0000000000000000}
		},
		.info = 0x0000
	},
	{
		.name = "stc\tGBR,r%d",
		.mask =	0b1111000011111111,
		.code = 0b0000000000010010,
		.arg = {
			{.mask = 0b0000111100000000},
			{.mask = 0b0000000000000000},
			{.mask = 0b0000000000000000}
		},
		.info = 0x0000
	},
	{
		.name = "stc\tSR,r%d",
		.mask =	0b1111000011111111,
		.code = 0b0000000000000010,
		.arg = {
			{.mask = 0b0000111100000000},
			{.mask = 0b0000000000000000},
			{.mask = 0b0000000000000000}
		},
		.info = 0x0000
	},
	{
		.name = "stc\tVBR,r%d",
		.mask =	0b1111000011111111,
		.code = 0b0000000000100010,
		.arg = {
			{.mask = 0b0000111100000000},
			{.mask = 0b0000000000000000},
			{.mask = 0b0000000000000000}
		},
		.info = 0x0000
	},
	{
		.name = "stc\tSSR,r%d",
		.mask =	0b1111000011111111,
		.code = 0b0000000000110010,
		.arg = {
			{.mask = 0b0000111100000000},
			{.mask = 0b0000000000000000},
			{.mask = 0b0000000000000000}
		},
		.info = 0x0000
	},
	{
		.name = "stc\tSPC,r%d",
		.mask =	0b1111000011111111,
		.code = 0b0000000001000010,
		.arg = {
			{.mask = 0b0000111100000000},
			{.mask = 0b0000000000000000},
			{.mask = 0b0000000000000000}
		},
		.info = 0x0000
	},
	{
		.name = "stc\tMOD,r%d",
		.mask =	0b1111000011111111,
		.code = 0b0000000001010010,
		.arg = {
			{.mask = 0b0000111100000000},
			{.mask = 0b0000000000000000},
			{.mask = 0b0000000000000000}
		},
		.info = 0x0000
	},
	{
		.name = "stc\tRE,r%d",
		.mask =	0b1111000011111111,
		.code = 0b0000000001110010,
		.arg = {
			{.mask = 0b0000111100000000},
			{.mask = 0b0000000000000000},
			{.mask = 0b0000000000000000}
		},
		.info = 0x0000
	},
	{
		.name = "stc\tRS,r%d",
		.mask =	0b1111000011111111,
		.code = 0b0000000001100010,
		.arg = {
			{.mask = 0b0000111100000000},
			{.mask = 0b0000000000000000},
			{.mask = 0b0000000000000000}
		},
		.info = 0x0000
	},
	{
		.name = "stc\tR0_BANK,r%d",
		.mask =	0b1111000011111111,
		.code = 0b0000000010000010,
		.arg = {
			{.mask = 0b0000111100000000},
			{.mask = 0b0000000000000000},
			{.mask = 0b0000000000000000}
		},
		.info = 0x0000
	},
	{
		.name = "stc\tR1_BANK,r%d",
		.mask =	0b1111000011111111,
		.code = 0b0000000010010010,
		.arg = {
			{.mask = 0b0000111100000000},
			{.mask = 0b0000000000000000},
			{.mask = 0b0000000000000000}
		},
		.info = 0x0000
	},
	{
		.name = "stc\tR2_BANK,r%d",
		.mask =	0b1111000011111111,
		.code = 0b0000000010100010,
		.arg = {
			{.mask = 0b0000111100000000},
			{.mask = 0b0000000000000000},
			{.mask = 0b0000000000000000}
		},
		.info = 0x0000
	},
	{
		.name = "stc\tR3_BANK,r%d",
		.mask =	0b1111000011111111,
		.code = 0b0000000010110010,
		.arg = {
			{.mask = 0b0000111100000000},
			{.mask = 0b0000000000000000},
			{.mask = 0b0000000000000000}
		},
		.info = 0x0000
	},
	{
		.name = "stc\tR4_BANK,r%d",
		.mask =	0b1111000011111111,
		.code = 0b0000000011000010,
		.arg = {
			{.mask = 0b0000111100000000},
			{.mask = 0b0000000000000000},
			{.mask = 0b0000000000000000}
		},
		.info = 0x0000
	},
	{
		.name = "stc\tR5_BANK,r%d",
		.mask =	0b1111000011111111,
		.code = 0b0000000011010010,
		.arg = {
			{.mask = 0b0000111100000000},
			{.mask = 0b0000000000000000},
			{.mask = 0b0000000000000000}
		},
		.info = 0x0000
	},
	{
		.name = "stc\tR6_BANK,r%d",
		.mask =	0b1111000011111111,
		.code = 0b0000000011100010,
		.arg = {
			{.mask = 0b0000111100000000},
			{.mask = 0b0000000000000000},
			{.mask = 0b0000000000000000}
		},
		.info = 0x0000
	},
	{
		.name = "stc\tR7_BANK,r%d",
		.mask =	0b1111000011111111,
		.code = 0b0000000011110010,
		.arg = {
			{.mask = 0b0000111100000000},
			{.mask = 0b0000000000000000},
			{.mask = 0b0000000000000000}
		},
		.info = 0x0000
	},
	{
		.name = "stc.l\tGBR,@r-%d",
		.mask =	0b1111000011111111,
		.code = 0b0100000000010011,
		.arg = {
			{.mask = 0b0000111100000000},
			{.mask = 0b0000000000000000},
			{.mask = 0b0000000000000000}
		},
		.info = 0x0000
	},
	{
		.name = "stc.l\tSR,@r-%d",
		.mask =	0b1111000011111111,
		.code = 0b0100000000000011,
		.arg = {
			{.mask = 0b0000111100000000},
			{.mask = 0b0000000000000000},
			{.mask = 0b0000000000000000}
		},
		.info = 0x0000
	},
	{
		.name = "stc.l\tVBR,@r-%d",
		.mask =	0b1111000011111111,
		.code = 0b0100000000100011,
		.arg = {
			{.mask = 0b0000111100000000},
			{.mask = 0b0000000000000000},
			{.mask = 0b0000000000000000}
		},
		.info = 0x0000
	},
	{
		.name = "stc.l\tSSR,@r-%d",
		.mask =	0b1111000011111111,
		.code = 0b0100000000110011,
		.arg = {
			{.mask = 0b0000111100000000},
			{.mask = 0b0000000000000000},
			{.mask = 0b0000000000000000}
		},
		.info = 0x0000
	},
	{
		.name = "stc.l\tSPC,@r-%d",
		.mask =	0b1111000011111111,
		.code = 0b0100000001000011,
		.arg = {
			{.mask = 0b0000111100000000},
			{.mask = 0b0000000000000000},
			{.mask = 0b0000000000000000}
		},
		.info = 0x0000
	},
	{
		.name = "stc.l\tMOD,@r-%d",
		.mask =	0b1111000011111111,
		.code = 0b0100000001010011,
		.arg = {
			{.mask = 0b0000111100000000},
			{.mask = 0b0000000000000000},
			{.mask = 0b0000000000000000}
		},
		.info = 0x0000
	},
	{
		.name = "stc.l\tRE,@r-%d",
		.mask =	0b1111000011111111,
		.code = 0b0100000001110011,
		.arg = {
			{.mask = 0b0000111100000000},
			{.mask = 0b0000000000000000},
			{.mask = 0b0000000000000000}
		},
		.info = 0x0000
	},
	{
		.name = "stc.l\tRS,@r-%d",
		.mask =	0b1111000011111111,
		.code = 0b0100000001100011,
		.arg = {
			{.mask = 0b0000111100000000},
			{.mask = 0b0000000000000000},
			{.mask = 0b0000000000000000}
		},
		.info = 0x0000
	},
	{
		.name = "stc.l\tR0_BANK,@r-%d",
		.mask =	0b1111000011111111,
		.code = 0b0100000010000011,
		.arg = {
			{.mask = 0b0000111100000000},
			{.mask = 0b0000000000000000},
			{.mask = 0b0000000000000000}
		},
		.info = 0x0000
	},
	{
		.name = "stc.l\tR1_BANK,@r-%d",
		.mask =	0b1111000011111111,
		.code = 0b0100000010010011,
		.arg = {
			{.mask = 0b0000111100000000},
			{.mask = 0b0000000000000000},
			{.mask = 0b0000000000000000}
		},
		.info = 0x0000
	},
	{
		.name = "stc.l\tR2_BANK,@r-%d",
		.mask =	0b1111000011111111,
		.code = 0b0100000010100011,
		.arg = {
			{.mask = 0b0000111100000000},
			{.mask = 0b0000000000000000},
			{.mask = 0b0000000000000000}
		},
		.info = 0x0000
	},
	{
		.name = "stc.l\tR3_BANK,@r-%d",
		.mask =	0b1111000011111111,
		.code = 0b0100000010110011,
		.arg = {
			{.mask = 0b0000111100000000},
			{.mask = 0b0000000000000000},
			{.mask = 0b0000000000000000}
		},
		.info = 0x0000
	},
	{
		.name = "stc.l\tR4_BANK,@r-%d",
		.mask =	0b1111000011111111,
		.code = 0b0100000011000011,
		.arg = {
			{.mask = 0b0000111100000000},
			{.mask = 0b0000000000000000},
			{.mask = 0b0000000000000000}
		},
		.info = 0x0000
	},
	{
		.name = "stc.l\tR5_BANK,@r-%d",
		.mask =	0b1111000011111111,
		.code = 0b0100000011010011,
		.arg = {
			{.mask = 0b0000111100000000},
			{.mask = 0b0000000000000000},
			{.mask = 0b0000000000000000}
		},
		.info = 0x0000
	},
	{
		.name = "stc.l\tR6_BANK,@r-%d",
		.mask =	0b1111000011111111,
		.code = 0b0100000011100011,
		.arg = {
			{.mask = 0b0000111100000000},
			{.mask = 0b0000000000000000},
			{.mask = 0b0000000000000000}
		},
		.info = 0x0000
	},
	{
		.name = "stc.l\tR7_BANK,@r-%d",
		.mask =	0b1111000011111111,
		.code = 0b0100000011110011,
		.arg = {
			{.mask = 0b0000111100000000},
			{.mask = 0b0000000000000000},
			{.mask = 0b0000000000000000}
		},
		.info = 0x0000
	},
	{
		.name = "sts\tFPSCR,r%d",
		.mask =	0b1111000011111111,
		.code = 0b0000000001101010,
		.arg = {
			{.mask = 0b0000111100000000},
			{.mask = 0b0000000000000000},
			{.mask = 0b0000000000000000}
		},
		.info = 0x0000
	},
	{
		.name = "sts\tFPUL,r%d",
		.mask =	0b1111000011111111,
		.code = 0b0000000001011010,
		.arg = {
			{.mask = 0b0000111100000000},
			{.mask = 0b0000000000000000},
			{.mask = 0b0000000000000000}
		},
		.info = 0x0000
	},
	{
		.name = "sts\tMACH,r%d",
		.mask =	0b1111000011111111,
		.code = 0b0000000000001010,
		.arg = {
			{.mask = 0b0000111100000000},
			{.mask = 0b0000000000000000},
			{.mask = 0b0000000000000000}
		},
		.info = 0x0000
	},
	{
		.name = "sts\tMACL,r%d",
		.mask =	0b1111000011111111,
		.code = 0b0000000000011010,
		.arg = {
			{.mask = 0b0000111100000000},
			{.mask = 0b0000000000000000},
			{.mask = 0b0000000000000000}
		},
		.info = 0x0000
	},
	{
		.name = "sts\tPR,r%d",
		.mask =	0b1111000011111111,
		.code = 0b0000000000101010,
		.arg = {
			{.mask = 0b0000111100000000},
			{.mask = 0b0000000000000000},
			{.mask = 0b0000000000000000}
		},
		.info = 0x0000
	},
	{
		.name = "sts\tDSR,r%d",
		.mask =	0b1111000011111111,
		.code = 0b0000000001101010,
		.arg = {
			{.mask = 0b0000111100000000},
			{.mask = 0b0000000000000000},
			{.mask = 0b0000000000000000}
		},
		.info = 0x0000
	},
	{
		.name = "sts\tA0,r%d",
		.mask =	0b1111000011111111,
		.code = 0b0000000001111010,
		.arg = {
			{.mask = 0b0000111100000000},
			{.mask = 0b0000000000000000},
			{.mask = 0b0000000000000000}
		},
		.info = 0x0000
	},
	{
		.name = "sts\tX0,r%d",
		.mask =	0b1111000011111111,
		.code = 0b0000000010001010,
		.arg = {
			{.mask = 0b0000111100000000},
			{.mask = 0b0000000000000000},
			{.mask = 0b0000000000000000}
		},
		.info = 0x0000
	},
	{
		.name = "sts\tX1,r%d",
		.mask =	0b1111000011111111,
		.code = 0b0000000010011010,
		.arg = {
			{.mask = 0b0000111100000000},
			{.mask = 0b0000000000000000},
			{.mask = 0b0000000000000000}
		},
		.info = 0x0000
	},
	{
		.name = "sts\tY0,r%d",
		.mask =	0b1111000011111111,
		.code = 0b0000000010101010,
		.arg = {
			{.mask = 0b0000111100000000},
			{.mask = 0b0000000000000000},
			{.mask = 0b0000000000000000}
		},
		.info = 0x0000
	},
	{
		.name = "sts\tY1,r%d",
		.mask =	0b1111000011111111,
		.code = 0b0000000010111010,
		.arg = {
			{.mask = 0b0000111100000000},
			{.mask = 0b0000000000000000},
			{.mask = 0b0000000000000000}
		},
		.info = 0x0000
	},
	{
		.name = "sts.l\tFPSCR,@-r%d",
		.mask =	0b1111000011111111,
		.code = 0b0100000001100010,
		.arg = {
			{.mask = 0b0000111100000000},
			{.mask = 0b0000000000000000},
			{.mask = 0b0000000000000000}
		},
		.info = 0x0000
	},
	{
		.name = "sts.l\tFPUL,@-r%d",
		.mask =	0b1111000011111111,
		.code = 0b0100000001010010,
		.arg = {
			{.mask = 0b0000111100000000},
			{.mask = 0b0000000000000000},
			{.mask = 0b0000000000000000}
		},
		.info = 0x0000
	},
	{
		.name = "sts.l\tMACH,@-r%d",
		.mask =	0b1111000011111111,
		.code = 0b0100000000000010,
		.arg = {
			{.mask = 0b0000111100000000},
			{.mask = 0b0000000000000000},
			{.mask = 0b0000000000000000}
		},
		.info = 0x0000
	},
	{
		.name = "sts.l\tMACL,@-r%d",
		.mask =	0b1111000011111111,
		.code = 0b0100000000010010,
		.arg = {
			{.mask = 0b0000111100000000},
			{.mask = 0b0000000000000000},
			{.mask = 0b0000000000000000}
		},
		.info = 0x0000
	},
	{
		.name = "sts.l\tPR,@-r%d",
		.mask =	0b1111000011111111,
		.code = 0b0100000000100010,
		.arg = {
			{.mask = 0b0000111100000000},
			{.mask = 0b0000000000000000},
			{.mask = 0b0000000000000000}
		},
		.info = 0x0000
	},
	{
		.name = "sts.l\tDSR,@-r%d",
		.mask =	0b1111000011111111,
		.code = 0b0100000001100010,
		.arg = {
			{.mask = 0b0000111100000000},
			{.mask = 0b0000000000000000},
			{.mask = 0b0000000000000000}
		},
		.info = 0x0000
	},
	{
		.name = "sts.l\tA0,@-r%d",
		.mask =	0b1111000011111111,
		.code = 0b0100000001110010,
		.arg = {
			{.mask = 0b0000111100000000},
			{.mask = 0b0000000000000000},
			{.mask = 0b0000000000000000}
		},
		.info = 0x0000
	},
	{
		.name = "sts.l\tX0,@-r%d",
		.mask =	0b1111000011111111,
		.code = 0b0100000010000010,
		.arg = {
			{.mask = 0b0000111100000000},
			{.mask = 0b0000000000000000},
			{.mask = 0b0000000000000000}
		},
		.info = 0x0000
	},
	{
		.name = "sts.l\tX1,@-r%d",
		.mask =	0b1111000011111111,
		.code = 0b0100000010010010,
		.arg = {
			{.mask = 0b0000111100000000},
			{.mask = 0b0000000000000000},
			{.mask = 0b0000000000000000}
		},
		.info = 0x0000
	},
	{
		.name = "sts.l\tY0,@-r%d",
		.mask =	0b1111000011111111,
		.code = 0b0100000010100010,
		.arg = {
			{.mask = 0b0000111100000000},
			{.mask = 0b0000000000000000},
			{.mask = 0b0000000000000000}
		},
		.info = 0x0000
	},
	{
		.name = "sts.l\tY1,@-r%d",
		.mask =	0b1111000011111111,
		.code = 0b0100000010110010,
		.arg = {
			{.mask = 0b0000111100000000},
			{.mask = 0b0000000000000000},
			{.mask = 0b0000000000000000}
		},
		.info = 0x0000
	},
	{
		.name = "sub\tr%d,r%d",
		.mask =	0b1111000000001111,
		.code = 0b0011000000001000,
		.arg = {
			{.mask = 0b0000000011110000},
			{.mask = 0b0000111100000000},
			{.mask = 0b0000000000000000}
		},
		.info = 0x0000
	},
	{
		.name = "subc\tr%d,r%d",
		.mask =	0b1111000000001111,
		.code = 0b0011000000001010,
		.arg = {
			{.mask = 0b0000000011110000},
			{.mask = 0b0000111100000000},
			{.mask = 0b0000000000000000}
		},
		.info = 0x0000
	},
	{
		.name = "subv\tr%d,r%d",
		.mask =	0b1111000000001111,
		.code = 0b0011000000001011,
		.arg = {
			{.mask = 0b0000000011110000},
			{.mask = 0b0000111100000000},
			{.mask = 0b0000000000000000}
		},
		.info = 0x0000
	},
	{
		.name = "swap.b\tr%d,r%d",
		.mask =	0b1111000000001111,
		.code = 0b0110000000001000,
		.arg = {
			{.mask = 0b0000000011110000},
			{.mask = 0b0000111100000000},
			{.mask = 0b0000000000000000}
		},
		.info = 0x0000
	},
	{
		.name = "swap.w\tr%d,r%d",
		.mask =	0b1111000000001111,
		.code = 0b0110000000001001,
		.arg = {
			{.mask = 0b0000000011110000},
			{.mask = 0b0000111100000000},
			{.mask = 0b0000000000000000}
		},
		.info = 0x0000
	},
	{
		.name = "tas.b\t@r%d",
		.mask =	0b1111000011111111,
		.code = 0b0100000000011011,
		.arg = {
			{.mask = 0b0000111100000000},
			{.mask = 0b0000000000000000},
			{.mask = 0b0000000000000000}
		},
		.info = 0x0000
	},
	{
		.name = "trapa\t0x%x",
		.mask =	0b1111111100000000,
		.code = 0b1100001100000000,
		.arg = {
			{.mask = 0b0000000011111111},
			{.mask = 0b0000000000000000},
			{.mask = 0b0000000000000000}
		},
		.info = 0x0000
	},
	{
		.name = "tst\t0x%x,r0",
		.mask =	0b1111111100000000,
		.code = 0b1100100000000000,
		.arg = {
			{.mask = 0b0000000011111111},
			{.mask = 0b0000000000000000},
			{.mask = 0b0000000000000000}
		},
		.info = 0x0000
	},
	{
		.name = "tst\tr%d,r%d",
		.mask =	0b1111000000001111,
		.code = 0b0010000000001000,
		.arg = {
			{.mask = 0b0000000011110000},
			{.mask = 0b0000111100000000},
			{.mask = 0b0000000000000000}
		},
		.info = 0x0000
	},
	{
		.name = "tst.b\t0x%x,@(r0,GBR)",
		.mask =	0b1111111100000000,
		.code = 0b1100110000000000,
		.arg = {
			{.mask = 0b0000000011111111},
			{.mask = 0b0000000000000000},
			{.mask = 0b0000000000000000}
		},
		.info = 0x0000
	},
	{
		.name = "xor\t0x%x,r0",
		.mask =	0b1111111100000000,
		.code = 0b1100101000000000,
		.arg = {
			{.mask = 0b0000000011111111},
			{.mask = 0b0000000000000000},
			{.mask = 0b0000000000000000}
		},
		.info = 0x0000
	},
	{
		.name = "xor\tr%d,r%d",
		.mask =	0b1111000000001111,
		.code = 0b0010000000001010,
		.arg = {
			{.mask = 0b0000000011110000},
			{.mask = 0b0000111100000000},
			{.mask = 0b0000000000000000}
		},
		.info = 0x0000
	},
	{
		.name = "wor.b\t0x%x,@(r0,GBR)",
		.mask =	0b1111111100000000,
		.code = 0b1100111000000000,
		.arg = {
			{.mask = 0b0000000011111111},
			{.mask = 0b0000000000000000},
			{.mask = 0b0000000000000000}
		},
		.info = 0x0000
	},
	{
		.name = "xtrct\tr%d,r%d",
		.mask =	0b1111000000001111,
		.code = 0b0010000000001101,
		.arg = {
			{.mask = 0b0000000011110000},
			{.mask = 0b0000111100000000},
			{.mask = 0b0000000000000000}
		},
		.info = 0x0000
	},
	{
		.name = NULL,
		.mask =	0b0000000000000000,
		.code = 0b0000000000000000,
		.arg = {
			{.mask = 0b0000000000000000},
			{.mask = 0b0000000000000000},
			{.mask = 0b0000000000000000}
		},
		.info = 0x0000
	},

};
