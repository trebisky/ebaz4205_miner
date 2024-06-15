/* Driver for an axi gpio device that has been
 * established in the PL by downloading an appropriate
 * bitstream to the Zynq FPGA.
 *
 * Tom Trebisky  6-7-2022
 */

typedef volatile unsigned int vu32;
typedef unsigned int u32;

#define BIT(x)  (1<<(x))

/* Places to look:
 *
 * In /u1/Xilinx/SDK/2019.1
 *  data/embeddedsw/XilinxProcessorIPLib/drivers/gpio_v4_4/src/xgpio.c
 *  data/embeddedsw/XilinxProcessorIPLib/drivers/gpio_v4_3/src/xgpio.c
 * In /u1/Xilinx/Vitis/2021.2
 *  data/embeddedsw/XilinxProcessorIPLib/drivers/gpio_v4_9/src/xgpio.c
 */

struct axi_gpio {
	vu32	data;
	vu32	dir;	// "tri"
	vu32	data2;
	vu32	dir2;
};

#define AXI_GPIO_BASE	((struct axi_gpio *) 0x41200000)

void
axi_gpio_write ( u32 val )
{
	struct axi_gpio *ap = AXI_GPIO_BASE;

	ap->data = val;
}

u32
axi_gpio_read ( void )
{
	struct axi_gpio *ap = AXI_GPIO_BASE;

	return ap->data;
}

/* Set 1 for inputs */
void
axi_gpio_dir ( u32 val )
{
	struct axi_gpio *ap = AXI_GPIO_BASE;

	ap->dir = val;
}

/* ledio bitstream documentation --
 * We do a 32 bit write, but only the low 8 bits matter.
 * There are two 4 bit groups (and they are identical)
 * The upper controls the green LED, the lower does the red.
 *
 * The 4 bits are:  UPSS
 * The U bit allows direct control of the LED by software on the ARM side.
 * The P bit (phase) lets us invert the signal to the LED.
 *  with P = 0, you write a 0 to U to turn the LED on
 *  with P = 1, you write a 1 to U to turn the LED on
 * The SS bits give you four choices:
 *  00 says to let the U bit run the show
 *  01 says fast blink
 *  10 says medium blink
 *  11 says slow blink
 */

static void
ps_blink ( void )
{
	for ( ;; ) {
	    axi_gpio_write ( 0xC4 );
	    ms_delay ( 100 );
	    axi_gpio_write ( 0x44 );
	    s_delay ( 5 );
	}
}

void
axi_test ( void )
{
	axi_gpio_dir ( 0 );

	// Both LED blink slow and out of phase.
	// Slow is about 1 hz
	// axi_gpio_write ( 0x26 );

	/* Both LED blink together, slow */
	// axi_gpio_write ( 0x22 );

	/* Both LED blink together, fast */
	// axi_gpio_write ( 0x11 );

	// Both LED blink fast and out of phase.
	axi_gpio_write ( 0x15 );

	// Controlled by user code.
	// ps_blink ();
}

void
led_all_off ( void )
{
	axi_gpio_write ( 0x88 );
}

/* At one time, when we got a uart interrupt, we called this
 * to stop all LED blinking.
 */
void
led_cmd ( int c )
{
	led_all_off ();
}

/* THE END */
