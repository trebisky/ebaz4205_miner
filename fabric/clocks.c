/* Code to initialize FPGA fabric clocks
 *
 * Tom Trebisky  2-28-2023
 *
 * Take a look in U-boot.git
 *  drivers/clk/clk_zynq.c
 *  arch/arm/mach-zynq/slcr.c
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

static void
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

static void
reg_show ( char *msg, u32 val )
{
	printf ( "%s %X\n", msg, val );
}

void
fabric_test ( void )
{
	struct slcr_regs *sp;

	sp = (struct slcr_regs *) ZYNQ_SYS_CTRL_BASEADDR;

	/* These yield:
	 * PLL -- arm: 00028008
	 * PLL -- ddr: 00020008
	 * PLL -- io : 0001E008
	 *  We have a 33.33 Mhz crystal, so using the multipliers we get:
	 *  33.3 * 0x28 (40) = 1332 Mhz (cpu runs at half this, i.e. 666 Mhz
	 *  33.3 * 0x20 (32) = 1066 Mhz
	 *  33.3 * 0x18 (30) = 1000 Mhz
	 */
	reg_show ( "PLL -- arm:", sp->arm_pll_ctrl );
	reg_show ( "PLL -- ddr:", sp->ddr_pll_ctrl );
	reg_show ( "PLL -- io :", sp->io_pll_ctrl );

	/* These yield:
	 * FPGA0 clock ctrl: 00200400 - div 2,4 = 8
	 * FPGA1 clock ctrl: 00500800 - div 5,8 = 40
	 * FPGA2 clock ctrl: 00101800 - div 1, 24 = 24
	 * FPGA3 clock ctrl: 00101800 - div 1, 24 = 24
	 *
	 *  I set up an FPGA design to route these to output pins
	 *  and use my Rigol scope to measure the frequencies
	 *
	 * clk0 -- 125  Mhz  (125*8 = 1000)
	 * clk1 -- 25.0 Mhz  (25*40 = 1000)
	 * clk2 -- 41.7 Mhz  (41.7*24 = 1000)
	 * clk3 -- 41.7 Mhz
	 */

	reg_show ( "FPGA0 clock ctrl:", sp->fpga0_clk_ctrl );
	reg_show ( "FPGA1 clock ctrl:", sp->fpga1_clk_ctrl );
	reg_show ( "FPGA2 clock ctrl:", sp->fpga2_clk_ctrl );
	reg_show ( "FPGA3 clock ctrl:", sp->fpga3_clk_ctrl );

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
