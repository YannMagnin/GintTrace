#include "gintrace/ubc.h"

#include <gint/mpu/power.h>
#include <gint/drivers.h>

/* internal UBC driver information */
struct sh7305_ubc_context ubctx;
void (*ubc_handler)(struct ucontext *ctx) = NULL;
int ubc_driver_installed = 0;

/* ubc_install(): Install the UBC driver */
void ubc_install(void)
{
	/* power on the UBC */
	SH7305_POWER.MSTPCR0.UDB = 0;

	/* switch the DBR */
	ubc_kernel_dbr_set(*ubc_kernel_inth);

	/* check if the driver has already been installed */
	if (ubc_driver_installed != 0) {
		SH7305_UBC.CBR0.lword  = ubctx.cbr0;
		SH7305_UBC.CRR0.lword  = ubctx.crr0;
		SH7305_UBC.CAR0        = ubctx.car0;
		SH7305_UBC.CAMR0       = ubctx.camr0;
		SH7305_UBC.CBR1.lword  = ubctx.cbr1;
		SH7305_UBC.CRR1.lword  = ubctx.crr1;
		SH7305_UBC.CAR1        = ubctx.car1;
		SH7305_UBC.CAMR1       = ubctx.camr1;
		SH7305_UBC.CDR1        = ubctx.cdr1;
		SH7305_UBC.CDMR1       = ubctx.cdmr1;
		SH7305_UBC.CETR1.lword = ubctx.cdr1;
		return;
	}

	/* wipe sofware information */
	ubc_handler = NULL;

	/* Default initialization */
	SH7305_UBC.CBR0.MFI = 0b000000;
	SH7305_UBC.CBR0.CE = 0;
	SH7305_UBC.CBR1.MFI = 0b000000;
	SH7305_UBC.CBR1.CE = 0;
	SH7305_UBC.CBCR.UBDE = 1;

	/* indicate that the UBC driver is installed */
	ubc_driver_installed = 1;
}

/* ubc_uninstall(): Uninstall the UBC driver */
void ubc_uninstall(void)
{
	if (ubc_driver_installed == 1) {
		ubctx.cbr0  = SH7305_UBC.CBR0.lword;
		ubctx.crr0  = SH7305_UBC.CRR0.lword;
		ubctx.car0  = SH7305_UBC.CAR0;
		ubctx.camr0 = SH7305_UBC.CAMR0;
		ubctx.cbr1  = SH7305_UBC.CBR1.lword;
		ubctx.crr1  = SH7305_UBC.CRR1.lword;
		ubctx.car1  = SH7305_UBC.CAR1;
		ubctx.camr1 = SH7305_UBC.CAMR1;
		ubctx.cdr1  = SH7305_UBC.CDR1;
		ubctx.cdmr1 = SH7305_UBC.CDMR1;
		ubctx.cdr1  = SH7305_UBC.CETR1.lword;
	}
}

/* ubc_set_breakpoint(): Setup one breakpoint */
int ubc_set_breakpoint(int channel, void *address, void *mask)
{
	if (channel == 0) {
		// Setup Channel 0.
		SH7305_UBC.CRR0.PCB	= 1;			// Set PC break after instruction break.
		SH7305_UBC.CRR0.BIE	= 1;			// Request a Break.
		SH7305_UBC.CBR0.MFE	= 0;			// Enable Match Flag.
		SH7305_UBC.CBR0.MFI	= 0b000000;		// Set UBC.CCMFR.MF0 = 1, when break occur.
		SH7305_UBC.CBR0.AIE	= 0;			// Disable ASID check.
		SH7305_UBC.CBR0.SZ 	= 0b010;		// Disable Match condition.
		SH7305_UBC.CBR0.CD 	= 0;			// Use Operand Bus for Operand Access.
		SH7305_UBC.CBR0.ID	= 0b01;			// Selecte instruction Fetch cycle.
		SH7305_UBC.CBR0.RW	= 0b11;			// Use Read or Write for match condition.
		SH7305_UBC.CBR0.CE	= 0;			// Disable Channel 0.
		SH7305_UBC.CAR0		= (uintptr_t)address;	// update break address.
		SH7305_UBC.CAMR0	= (uintptr_t)mask;	// update break address.
		SH7305_UBC.CBR0.CE	= 1; 			// enable channel.
	} else {
		//TODO
		return (-1);
	}
	ubc_kernel_update();
	return (0);
}

/* ubc_set_handler(): Set the "user" handler */
int ubc_set_handler(void (*handler)(struct ucontext *ctxt))
{
	ubc_handler = handler;
	return (0);
}


static uint32_t __ubc_block_counter = 0;
static uint32_t __ubc_cbcr_ubde = 0;
static uint32_t __ubc_cbr0_ce = 0;

/* ubc_block(): Block UBC interruption */
int ubc_block(void)
{
	if (__ubc_block_counter == 0) {
		__ubc_cbr0_ce = SH7305_UBC.CBR0.CE;
		__ubc_cbcr_ubde = SH7305_UBC.CBCR.UBDE;
		SH7305_UBC.CBCR.UBDE = 0;
		SH7305_UBC.CBR0.CE = 0;
		ubc_kernel_update();
	}
	__ubc_block_counter += 1;
	return (0);
}


/* ubc_block(): Block UBC interruption */
int ubc_unblock(void)
{
	if (__ubc_block_counter <= 0)
		return (-1);
	__ubc_block_counter -= 1;
	if (__ubc_block_counter == 0) {
		SH7305_UBC.CBCR.UBDE = __ubc_cbcr_ubde;
		SH7305_UBC.CBR0.CE = __ubc_cbr0_ce;
		ubc_kernel_update();
	}
	return (0);
}
