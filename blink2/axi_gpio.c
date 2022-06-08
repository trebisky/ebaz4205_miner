/* Drive for an axi gpio device that has been
 * established in the PL by downloading an appropriate
 * bitstream to the Zynq FPGA.
 *
 * Tom Trebisky  6-7-2022
 */

typedef volatile unsigned int vu32;
typedef unsigned int u32;

#define BIT(x)  (1<<(x))

struct axi_gpio {
	vu32	control;
};

#define AXI_GPIO_BASE	(struct axi_gpio *) 0x4120_0000)

/* THE END */
