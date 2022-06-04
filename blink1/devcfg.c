/* Zynq-7000 devcfg driver
 *
 * Tom Trebisky  6-3-2022
 *
 * See page 1144 (B.16) in the TRM.
 */

typedef volatile unsigned int vu32;
typedef unsigned int u32;

#define BIT(x)  (1<<(x))

struct devcfg {
	vu32	control;	/* 00 */
	vu32	lock;
	vu32	config;
	vu32	i_status;
	vu32	i_mask;		/* 10 */
	vu32	status;
	vu32	dma_src_addr;
	vu32	dma_dst_addr;
	vu32	dma_src_len;	/* 20 */
	vu32	dma_dst_len;
	vu32	rom_shadow;
	vu32	multiboot_addr;
	vu32	sw_id;		/* 30 */
	vu32	unlock;
	int	_pad0[18];
	vu32	mctrl;		/* 80 - misc control */
};

/* Bits in the control register */
#define CTRL_RATE	BIT(25)

/* Bits in the status register */
#define ST_DMA_Q_FULL	BIT(31)
#define ST_DMA_Q_EMPTY	BIT(30)

#define ST_PL_INIT	BIT(4)

/* Bits in the mctrl register */
#define MC_LOOPBACK	BIT(4)

#define DEVCFG_BASE ((struct devcfg *) 0xF8007000)

void
devcfg_init ( void )
{
	struct devcfg *dp = DEVCFG_BASE;

	// printf ( "mctrl = %X\n", &dp->mctrl );
	printf ( "status = %X\n", dp->status );

	/* Unlock the interface.
	 * Writing this magic value does it, but as near
	 * as I can tell, it has already been done.
	 */
	// dp->unlock = 0x757BDF0D;

}

#define PCAP_LAST_XFER		1

/* The example code makes this 0xffffffff and then ORs on
 * the last transfer bit, which of course accomplishes nothing.
 * The TRM says the last 2 bits must be 01 and must match the
 * same bits in the source address, so we do this.
 */
#define PL_ADDRESS		0xfffffffd

/* Load a bitstream image into the PL.
 * Size is in 32 bit words.
 */
void
pl_load_image ( char *image, int size )
{
	struct devcfg *dp = DEVCFG_BASE;

	/* Is DMA busy? */
	if ( dp->status & ST_DMA_Q_FULL ) {
	    printf ( "devcfg DMA queue full" );
	    return;
	}

	/* Is the PL ready */
	if ( ! (dp->status & ST_PL_INIT) ) {
	    printf ( "devcfg PL not ready" );
	    return;
	}

	/* Clear internal PCAP loopback */
	dp->mctrl &= ~MC_LOOPBACK;

	/* Clear 1/4 rate bit */
	dp->control &= ~CTRL_RATE;

	/* Now, set up DMA
	 * When the 2 last bits of an address are
	 * 01 it indicates the last DMA of an overall transfer.
	 * As near as I can tell, writing into these 4 registers
	 * sets things in motion.
	 */
	dp->dma_src_addr = ((u32) image) | PCAP_LAST_XFER;
	dp->dma_dst_addr = PL_ADDRESS;
	dp->dma_src_len = size;
	dp->dma_dst_len = 0;
}

static char im[4];
static int im_size;

void
pl_load ( void *info )
{
	im_size = 4;
	pl_load_image ( im, im_size );
}

/* THE END */
