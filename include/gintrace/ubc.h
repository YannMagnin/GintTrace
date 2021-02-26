#ifndef __GINTRACE_UBC_H__
# define __GINTRACE_UBC_H__

#include <stddef.h>
#include <stdint.h>

#include <gint/defs/types.h>

//---
//	Hardware information - Same as the SH7724
//---
#define SH7305_UBC	(*(volatile struct sh7305_ubc *)0xff200000)
struct sh7305_ubc {
	//---
	//	Channel 0
	//---
	lword_union(CBR0,
		uint32_t MFE	: 1;	/* Match Flag Enable */
		uint32_t AIE	: 1;	/* ASID Enable */
		uint32_t MFI	: 6;	/* Match Flag Specify */
		uint32_t AIV	: 8;	/* ASID Specify */
		uint32_t const	: 1;	/* All 0 */
		uint32_t SZ	: 3;	/* Operend Size Select */
		uint32_t const	: 4;	/* All 0 */
		uint32_t CD	: 2;	/* Bus Select */
		uint32_t ID	: 2;	/* Instr Fetch/Operand Access Select */
		uint32_t const	: 1;	/* All 0 */
		uint32_t RW	: 2;	/* Bus Command Select */
		uint32_t CE	: 1;	/* Channel Enable */
	);
	lword_union(CRR0,
		uint32_t const	: 18;	/* All 0 */
		uint32_t const	: 1;	/* All 1 */
		uint32_t const	: 11;	/* All 0 */
		uint32_t PCB	: 1;	/* PC Breack Select */
		uint32_t BIE	: 1;	/* Breack Enable */
	);
	uint32_t CAR0;			/* Compare Address */
	uint32_t CAMR0;			/* Compare Address Mask */
	pad(0x10);

	//---
	//	Channel 1
	//---
	lword_union(CBR1,
		uint32_t MFE	: 1;	/* Match Flag Enable */
		uint32_t AIE	: 1;	/* ASID Enable */
		uint32_t MFI	: 6;	/* Match Flag Specify */
		uint32_t AIV	: 8;	/* ASID Specify */
		uint32_t DBE	: 1;	/* Data Value Enable */
		uint32_t SZ	: 3;	/* Operend Size Select */
		uint32_t ETBE	: 1;	/* Execution Count Value Enable */
		uint32_t const	: 3;	/* All 0 */
		uint32_t CD	: 2;	/* Bus Select */
		uint32_t ID	: 2;	/* Instr Fetch/Operand Access Select */
		uint32_t const	: 1;	/* All 0 */
		uint32_t RW	: 2;	/* Bus Command Select */
		uint32_t CE	: 1;	/* Channel Enable */
	);
	lword_union(CRR1,
		uint32_t const	: 18;	/* All 0 */
		uint32_t const	: 1;	/* All 1 */
		uint32_t const	: 11;	/* All 0 */
		uint32_t PCB	: 1;	/* PC Breack Select */
		uint32_t BIE	: 1;	/* Breack Enable */
	);
	uint32_t CAR1;			/* Compare Address */
	uint32_t CAMR1;			/* Compare Address Mask */

	//--
	//	Compare Data Part.
	//---
	uint32_t CDR1;			/* Compare Data Value */
	uint32_t CDMR1;			/* Compare Data Value Mask */


	//---
	//	Execution Count Register
	//---
	lword_union(CETR1,
		uint32_t const 	: 20;	/* All 0 */
		uint32_t CET	: 12;	/* Execution Count */
	);
	pad(0x5c4);

	//---
	//	Channel Match Flag Register
	//---
	lword_union(CCMFR,
		uint32_t const	: 30;	/* All 0 */
		uint32_t MF1	: 1;	/* Channel 1 Condition Match Flag */
		uint32_t MF0	: 1;	/* Channel 0 Condition Match Flag */
	);
	pad(0x1c);

	//---
	//	Control Register
	//---
	lword_union(CBCR,
		uint32_t const	: 31;	/* All 0 */
		uint32_t UBDE	: 1;	/* UBC Debugging Support Enable */
	);
};

/* hardware context */
struct sh7305_ubc_context {
	uint32_t cbr0;
	uint32_t crr0;
	uint32_t car0;
	uint32_t camr0;
	uint32_t cbr1;
	uint32_t crr1;
	uint32_t car1;
	uint32_t camr1;
	uint32_t cdr1;
	uint32_t cdmr1;
	uint32_t cetr1;
	uint32_t ccmfr;
	uint32_t cbr;
};




//---
//	Software information
//---
/* ucontext: UBC context */
struct ucontext {
	uintptr_t reg[16];
	uintptr_t gbr;
	uintptr_t macl;
	uintptr_t mach;
	uintptr_t ssr;
	uintptr_t spc;
	uintptr_t pr;
};

//---
//	User API
//---
/* ub_install(): Install UBC driver */
extern void ubc_install(void);
extern void ubc_uninstall(void);

/* ubc_set_handler(): Setup the handler */
extern int ubc_set_handler(void (*handler)(struct ucontext *ctx));

/* ubc_set_breakpoint(): Setup one breakpoint */
extern int ubc_set_breakpoint(int channel, void *address, void *mask);

/* ubc_block(): Block UBC interruption */
extern int ubc_block(void);
/* ubc_unblock(): Unblock UBC interruption */
extern int ubc_unblock(void);

//---
//	"low-level" interface
//---
/* ubc_kernel_dbr_set(): Switch DBR register */
extern void *ubc_kernel_dbr_set(void *dbr);

/* ubc_kernel_inth: Content the relocalised UBC interrupt handler */
extern void (*ubc_kernel_inth)(void);

/* ubc_kernel_update(): Perform a cache reset */
extern void ubc_kernel_update(void);

#endif /*__GINTRACE_UBC_H__*/
