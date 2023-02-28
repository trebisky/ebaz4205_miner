/* Zynq-7000 driver to download bitstreams to the pl
 *
 * Perhaps I should name it "bitstream" or even "devcfg",
 * but "pl" is easy to type.

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
        vu32	_pad1;
        vu32	write_count;	/* 0x88 */
        vu32	read_count;	/* 0x8c */
};


/* Bits in the control register */
#define CTRL_PL_RESET	BIT(30)		/* PROG B */
#define CTRL_PCAP_PR	BIT(27)		/* partial */
#define CTRL_PCAP_MODE	BIT(26)
#define CTRL_RATE	BIT(25)

#ifdef notdef
#define DEVCFG_CTRL_PCFG_PROG_B         0x40000000
#define DEVCFG_CTRL_PCAP_RATE_EN_MASK   0x02000000
#endif


#define CTRL_NIDEN	0x10		/* non invasive debug enable */

/* Bits in the status register */
#define ST_DMA_Q_FULL	BIT(31)
#define ST_DMA_Q_EMPTY	BIT(30)

#define ST_PL_INIT	BIT(4)

/* Bits in the mctrl register */
#define MC_LOOPBACK	BIT(4)

/* Bits in interrupt status */

#define IS_DMA_DONE	BIT(13)
#define IS_D_P_DONE	BIT(12)
#define IS_PL_DONE	BIT(2)

#define ST_FATAL	0x00740040
#define ST_ERROR	0x00340840


#define DEVCFG_BASE ((struct devcfg *) 0xF8007000)

/* -------------------------------------------------------- */
/* -------------------------------------------------------- */

/* From U-boot hardware.h
 */
struct slcr {
        vu32 scl; /* 0x0 */
        vu32 slcr_lock; /* 0x4 */
        vu32 slcr_unlock; /* 0x8 */
        vu32 __reserved0_1[61];
        vu32 arm_pll_ctrl; /* 0x100 */
        vu32 ddr_pll_ctrl; /* 0x104 */
        vu32 io_pll_ctrl; /* 0x108 */
        vu32 __reserved0_2[5];
        vu32 arm_clk_ctrl; /* 0x120 */
        vu32 ddr_clk_ctrl; /* 0x124 */
        vu32 dci_clk_ctrl; /* 0x128 */
        vu32 aper_clk_ctrl; /* 0x12c */
        vu32 __reserved0_3[2];
        vu32 gem0_rclk_ctrl; /* 0x138 */
        vu32 gem1_rclk_ctrl; /* 0x13c */
        vu32 gem0_clk_ctrl; /* 0x140 */
        vu32 gem1_clk_ctrl; /* 0x144 */
        vu32 smc_clk_ctrl; /* 0x148 */
        vu32 lqspi_clk_ctrl; /* 0x14c */
        vu32 sdio_clk_ctrl; /* 0x150 */
        vu32 uart_clk_ctrl; /* 0x154 */
        vu32 spi_clk_ctrl; /* 0x158 */
        vu32 can_clk_ctrl; /* 0x15c */
        vu32 can_mioclk_ctrl; /* 0x160 */
        vu32 dbg_clk_ctrl; /* 0x164 */
        vu32 pcap_clk_ctrl; /* 0x168 */
        vu32 __reserved0_4[1];
        vu32 fpga0_clk_ctrl; /* 0x170 */
        vu32 __reserved0_5[3];
        vu32 fpga1_clk_ctrl; /* 0x180 */
        vu32 __reserved0_6[3];
        vu32 fpga2_clk_ctrl; /* 0x190 */
        vu32 __reserved0_7[3];
        vu32 fpga3_clk_ctrl; /* 0x1a0 */
        vu32 __reserved0_8[8];
        vu32 clk_621_true; /* 0x1c4 */
        vu32 __reserved1[14];
        vu32 pss_rst_ctrl; /* 0x200 */
        vu32 __reserved2[15];
        vu32 fpga_rst_ctrl; /* 0x240 */
        vu32 __reserved3[5];
        vu32 reboot_status; /* 0x258 */
        vu32 boot_mode; /* 0x25c */
        vu32 __reserved4[116];
        vu32 trust_zone; /* 0x430 XXX */
        vu32 __reserved5_1[63];
        vu32 pss_idcode; /* 0x530 */
        vu32 __reserved5_2[51];
        vu32 ddr_urgent; /* 0x600 */
        vu32 __reserved6[6];
        vu32 ddr_urgent_sel; /* 0x61c */
        vu32 __reserved7[56];
        vu32 mio_pin[54]; /* 0x700 - 0x7D4 */
        vu32 __reserved8[74];
        vu32 lvl_shftr_en; /* 0x900 */
        vu32 __reserved9[3];
        vu32 ocm_cfg; /* 0x910 */
};

#define SLCR_BASE ((struct slcr *) 0xF8000000)

/* SLCR stuff */

/* SLCR - System Level Control Registers.
 * TRM section 4.3 and B.28 (page 1570)
 * There are a lot more of these, but these
 * are the ones we need to manipulate to load PL images
 */

#define SLCR_LOCK_MAGIC		0x767B
#define SLCR_UNLOCK_MAGIC	0xDF0D

static int slcr_locked = 1; /* 1 means locked, 0 means unlocked */

static void
slcr_lock(void)
{
	struct slcr *sp = SLCR_BASE;

	if ( ! slcr_locked ) {
		sp->slcr_lock = SLCR_LOCK_MAGIC;
		// writel(SLCR_LOCK_MAGIC, &slcr_base->slcr_lock);
		slcr_locked = 1;
	}
}

static void
slcr_unlock(void)
{
	struct slcr *sp = SLCR_BASE;

	if (slcr_locked) {
		sp->slcr_unlock = SLCR_UNLOCK_MAGIC;
		// writel(SLCR_UNLOCK_MAGIC, &slcr_base->slcr_unlock);
		slcr_locked = 0;
	}
}

#ifdef notdef
/* Reset the entire system */
void
zynq_slcr_cpu_reset(void)
{
	/*
	 * Unlock the SLCR then reset the system.
	 * Note that this seems to require raw i/o
	 * functions or there's a lockup?
	 */
	zynq_slcr_unlock();

	/*
	 * Clear 0x0F000000 bits of reboot status register to workaround
	 * the FSBL not loading the bitstream after soft-reboot
	 * This is a temporary solution until we know more.
	 */
	clrbits_le32(&slcr_base->reboot_status, 0xF000000);

	writel(1, &slcr_base->pss_rst_ctrl);
}
#endif

static void
slcr_fpga_disable(void)
{
	// u32 reg_val;
	struct slcr *sp = SLCR_BASE;

	slcr_unlock();

	/* Disable AXI interface by asserting FPGA resets */
	// writel(0xF, &slcr_base->fpga_rst_ctrl);
	sp->fpga_rst_ctrl = 0xf;

	/* Disable Level shifters before setting PS-PL */
	// reg_val = readl(&slcr_base->lvl_shftr_en);
	// reg_val &= ~0xF;
	// writel(reg_val, &slcr_base->lvl_shftr_en);
	sp->lvl_shftr_en &= ~0xf;

	/* Set Level Shifters DT618760 */
	// writel(0xA, &slcr_base->lvl_shftr_en);
	sp->lvl_shftr_en = 0xa;

	slcr_lock();
}

static void
slcr_fpga_enable(void)
{
	struct slcr *sp = SLCR_BASE;

	slcr_unlock();

	/* Set Level Shifters DT618760 */
	// writel(0xF, &slcr_base->lvl_shftr_en);
	sp->lvl_shftr_en = 0xf;

	/* Enable AXI interface by de-asserting FPGA resets */
	// writel(0x0, &slcr_base->fpga_rst_ctrl);
	sp->fpga_rst_ctrl = 0;

	slcr_lock();
}

/* Does not seem to be needed */
static void
slcr_pcap_clock_enable ( void )
{
	struct slcr *sp = SLCR_BASE;

	slcr_unlock ();

#define SLCR_PCAP_CLOCK_ENA	1
	/* Enable the PCAP clock */
	sp->pcap_clk_ctrl |= SLCR_PCAP_CLOCK_ENA;

	printf ( "clock = %X\n", sp->pcap_clk_ctrl );
	/* I find this set 0x0501
	 * which is divisor = 5, source = IO PLL
	 */

	slcr_lock ();
}

/* -------------------------------------------------------- */
/* -------------------------------------------------------- */

/* It turns out that the control register reads as 0x4c00e07f
 * In particular, the 0x40000000 bit is the PL reset.
 */

static int
devcfg_selftest ( void )
{
	struct devcfg *dp = DEVCFG_BASE;
	u32 save;
	int rv = 0;

	/* Set a sane control register value.
	 * Note that PCAP and such are enabled
	 */
	// printf ( "Self test, control reg = %X\n", dp->control );
	// dp->control = 0x4c006000;
	// printf ( "Self test, control reg = %X\n", dp->control );

	save = dp->control;

	dp->control |= CTRL_NIDEN;
	if ( ! (dp->control & CTRL_NIDEN) )
	    rv = 1;

	dp->control &= ~CTRL_NIDEN;
	if ( (dp->control & CTRL_NIDEN) )
	    rv = 2;

	dp->control = save;

	return rv;
}

int
pl_setup ( void )
{
	struct devcfg *dp = DEVCFG_BASE;
	int s;
	unsigned int tmo;

	// printf ( "Devcfg at %X\n", dp );

	// printf ( "mctrl is at: %X\n", &dp->mctrl );
	// printf ( "status = %X\n", dp->status );

	if ( devcfg_selftest () ) {
	    printf ( "Self test fails: %d\n", s );
	    return 1;
	}

	/* Clear internal PCAP loopback */
	dp->mctrl &= ~MC_LOOPBACK;

	/* Unlock the devcfg interface.
	 * Writing this magic value does it, but as near
	 * as I can tell, it has already been done.
	 */
	// dp->unlock = 0x757BDF0D;

	/* U-boot also writes this as part of
	 * the unlock sequence.
	 */
	// dp->rom_shadow = 0xffffffff;

	// printf ( "control = %X\n", dp->control );

	dp->control |= CTRL_PL_RESET;	/* Prog B */
	dp->control &= ~CTRL_PL_RESET;	/* Prog B */

	/* Poll the INIT bit.
	 * Wait for it to be clear.
	 */
	tmo = 999999;
	while ( --tmo && (dp->status & ST_PL_INIT) )
	    ;

	if ( ! tmo ) {
	    printf ( "PL reset timed out\n" );
	    return 1;
	}
	// printf ( "PL reset: %d\n", tmo );

	dp->control |= CTRL_PL_RESET;	/* Prog B */

	/* Now wait for the bit to be set */
	tmo = 999999;
	while ( --tmo && ! (dp->status & ST_PL_INIT) )
	    ;

	if ( ! tmo ) {
	    printf ( "PL ready timed out\n" );
	    return 1;
	}

	/* clear all interrupts, or try to anyhow */
	dp->i_status = 0xffffffff;

	if ( dp->i_status & ST_FATAL ) {
	    printf ( "PL devconfig fatal errors set: %X\n", dp->status );
	    return 1;
	}

	/* Is DMA busy? */
	if ( dp->status & ST_DMA_Q_FULL ) {
	    printf ( "DMA queue full - PL busy\n" );
	    return 1;
	}

	/* --------------------------------------------- */
	/* These things are usually OK, but be sure */

	/* Clear 1/4 rate bit */
	dp->control &= ~CTRL_RATE;

	/* Select PCAP FOR  partial reconfiguration */
	dp->control |= CTRL_PCAP_PR;

	/* Enable PCAP */
	dp->control |= CTRL_PCAP_MODE;

	// printf ( "control = %X\n", dp->control );

	return 0;
}

#define PCAP_LAST_XFER		1

/* The example code makes this 0xffffffff and then ORs on
 * the last transfer bit, which of course accomplishes nothing.
 * The TRM says the last 2 bits must be 01 and must match the
 * same bits in the source address, but using all ones works.
 */
// #define PL_ADDRESS		0xfffffffd
#define PL_ADDRESS		0xffffffff

void
pl_dma ( char *image, int size )
{
	struct devcfg *dp = DEVCFG_BASE;
	int tmo;

#ifdef notdef
	printf ( "Devcfg I status: %X\n", dp->i_status );
	/* clear by writing to the bit */
	dp->i_status = ( IS_DMA_DONE | IS_D_P_DONE | IS_PL_DONE );
	printf ( "Devcfg I status: %X\n", dp->i_status );
#endif

	/* Now, set up DMA
	 * When the 2 last bits of an address are
	 * 01 it indicates the last DMA of an overall transfer.
	 * As near as I can tell, writing into these 4 registers
	 * sets things in motion.
	 */
	// printf ( "Set DMA for %X, %d words\n", image, size );
	// printf ( "DMA write count: %d\n",  dp->write_count );

	dp->dma_src_addr = ((u32) image) | PCAP_LAST_XFER;
	dp->dma_dst_addr = PL_ADDRESS;

	/* sizes are in counts of 4 byte words */
	dp->dma_src_len = size;
	dp->dma_dst_len = 0;

	// printf ( "Polling for DMA done\n" );
	// for ( tmo = 500000; tmo; tmo-- )
	for ( tmo = 999999; tmo; tmo-- )
	    if ( dp->i_status & IS_DMA_DONE )
		break;
	/* I see this finishing with tmo = 367320 (starting with 500000 */
	// printf ( "Devcfg I status: %X, %d\n", dp->i_status, tmo );

#ifdef notdef
	// printf ( "Polling for PL done\n" );
	for ( tmo = 500000; tmo; tmo-- )
	    if ( dp->i_status & IS_PL_DONE )
		break;
	// This is always ready the first check, I see 500000
	// printf ( "Devcfg I status: %X, %d\n", dp->i_status, tmo );
#endif

	if ( dp->i_status & ST_ERROR ) {
	    printf ( "PL transfer ended with errors: %X\n", dp->i_status );
	}

	/* These show 520935 for write, 0 for read */
	// printf ( "DMA write count: %d\n",  dp->write_count );
	// printf ( "DMA read count: %d\n",  dp->read_count );

	dp->i_status = IS_DMA_DONE | IS_PL_DONE;
}

/* Load a bitstream image into the PL.
 * Size is in 32 bit words.
 */
void
pl_load_image ( char *image, int size )
{
	slcr_fpga_disable ();
	if ( pl_setup () ) {
	    printf ( "PL setup failed\n" );
	    return;
	}
	pl_dma ( image, size );
	slcr_fpga_enable ();

}

/* ----------------------------------------------------------- */
/* ----------------------------------------------------------- */

#define ___swab32(x) \
        ((u32)( \
                (((u32)(x) & (u32)0x000000ffUL) << 24) | \
                (((u32)(x) & (u32)0x0000ff00UL) <<  8) | \
                (((u32)(x) & (u32)0x00ff0000UL) >>  8) | \
                (((u32)(x) & (u32)0xff000000UL) >> 24) ))

static void
swap_em ( u32 *buf, int count )
{
	int i;

	for ( i=0; i<count; i++ ) {
	    //if ( i< 32 ) printf ( "Before swap: %X\n", buf[i] );
	    buf[i] = ___swab32 ( buf[i] );
	    //if ( i< 32 ) printf ( "After swap: %X\n", buf[i] );
	}
}

extern unsigned int pl_comp_data[];

static char im_raw[3*1024*1024];
static char *im;

void
pl_load ( void )
{
	int size;	/* word count */

	/* Hack to meet DMA alignment requirements */
	// im = im_raw;
	im = (char *) (((u32)im_raw) & ~0xff);

	size = pl_expand ( im, pl_comp_data );

	// printf ( "PL image of %d words, loaded to buffer at: %X\n", size, im );

	swap_em ( (u32 *) im, size );

	/* These call the code taken from uboot (in zpl.c)
	 * "full" works, "partial" does not.
	 */
	// zynq_load_full ( im, size*4 );

	// NO! zynq_load_partial ( im, size*4 );

	flush_dcache_buffer ( im, size*4 );
	pl_load_image ( im, size );

	printf ( "PL image done loading and should be running\n" );
	//printf ( "Done loading PL image\n" );
	//printf ( "Should be running now\n" );
}

#define ZFLAG   0x80000000
#define EFLAG   0xC0000000

int
pl_expand ( u32 *dest, u32 *src )
{
	u32 *ip;
	u32 *dp;
	int n;

	ip = src;
	dp = dest;

	while ( *ip != EFLAG ) {
	    if ( *ip & ZFLAG ) {
		n = *ip & ~ZFLAG;
		while ( n-- )
		    *dp++ = 0;
		++ip;
	    } else {
		n = *ip++;
		while ( n -- )
		    *dp++ = *ip++;
	    }
	}

	/* The image we are using has 2083740 bytes (520935 words)
	 */
	n = dp - dest;
	// printf ( "Generated %d words (%d bytes)\n", n, n*4 );
	printf ( "PL bitstream decompressed to %d words (%d bytes)\n", n, n*4 );
	return n;
}

#ifdef notdef
extern int uart_character;

static int
wait_char ( void )
{
	int rv;

	while ( ! uart_character )
	    ms_delay ( 1 );
	rv = uart_character;
	uart_character = 0;
	return rv;
}
#endif

/* This was used once for some experiments.
 *
 * If we set this bit low, then download a bitstream via JTAG
 * it seems to load, but does not run.
 * If we set it low, a running bitstream seems to stop
 * the DONE led goes out and any LED action being caused by
 * the PL stops.
 *
 * The bit is high after reboot, and I can download a bitstream
 * via JTAG.  Setting this bit high when it is already high
 * does nothing - the bitstream continues to run.
 * If I set it low, the bitstream stops, as described above.
 * Setting it high again does not resume the bitstream.
 */
static void
pl_reset ( int what )
{
	struct devcfg *dp = DEVCFG_BASE;

	printf ( "PL reset, control reg = %X\n", dp->control );
	if ( what )
	    dp->control |= CTRL_PL_RESET;
	else
	    dp->control &= ~CTRL_PL_RESET;
	printf ( "PL reset, control reg = %X\n", dp->control );
}


/* This is a hook called by main()
 *  to allow testing.
 */
void
pl_test ( void )
{
	int cc;

	pl_load ();

#ifdef notdef
	for ( ;; ) {
	    cc = wait_char ();
	    if ( cc == 'r' ) {
		printf ( "PL reset low\n" );
		pl_reset ( 0 );
	    } else {
		printf ( "PL reset high\n" );
		pl_reset ( 1 );
	    }
	}
#endif
}

/* THE END */
