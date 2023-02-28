/* Code to initialize FPGA fabric clocks
 *
 * Tom Trebisky  2-28-2023
 */

typedef volatile unsigned int vu32;
typedef unsigned int u32;

#include "hardware.h"

#define BIT(x)  (1<<(x))

/* Before we do anything, we see this:
 * FPGA0 clock ctrl: 00200400
 * FPGA1 clock ctrl: 00500800
 */

#define	SRC_IO		0x00
#define	SRC_IOx		0x10
#define	SRC_ARM		0x20
#define	SRC_DDR		0x30

#define DIV0_SHIFT	8
#define DIV1_SHIFT	20

void
fabric_set ( int src, int div0, int div1 )
{
	struct slcr_regs *sp;
	u32 val;

	sp = (struct slcr_regs *) ZYNQ_SYS_CTRL_BASEADDR;

	val = div1 << DIV1_SHIFT | div0 << DIV0_SHIFT + src;

	zynq_slcr_unlock();
	sp->fpga0_clk_ctrl = val;
	zynq_slcr_lock();
}


void
fabric_test ( void )
{
	struct slcr_regs *sp;
	u32 val;

	sp = (struct slcr_regs *) ZYNQ_SYS_CTRL_BASEADDR;

	val = sp->fpga0_clk_ctrl;
	printf ( "FPGA0 clock ctrl: %X\n", val );
	val = sp->fpga1_clk_ctrl;
	printf ( "FPGA1 clock ctrl: %X\n", val );

	// normal
	// fabric_set ( SRC_IO, 4, 2 );
}

void
fabric_slower ( void )
{
	fabric_set ( SRC_IO, 4, 4 );
}

void
fabric_faster ( void )
{
	fabric_set ( SRC_IO, 4, 1 );
}

void
fabric_norm ( void )
{
	fabric_set ( SRC_IO, 4, 2 );
}

/* THE END */
