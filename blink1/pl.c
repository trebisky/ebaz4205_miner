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
#define CTRL_PL_RESET	BIT(30)
#define CTRL_PCAP_PR	BIT(27)
#define CTRL_PCAP_MODE	BIT(26)
#define CTRL_RATE	BIT(25)

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

#define DEVCFG_BASE ((struct devcfg *) 0xF8007000)

/* -------------------------------------------------------- */
/* -------------------------------------------------------- */
/* SLCR - System Level Control Registers.
 * TRM section 4.3 and B.28 (page 1570)
 * There are a lot more of these, but these
 * are the ones we need to manipulate to load PL images
 */

struct slcr_lock {
	vu32	secure_lock;	/* 00 */
	vu32	wp_lock;	/* 04 */
	vu32	wp_unlock;	/* 08 */
};

struct slcr_misc {
	vu32	level_shift;	/* 00 */
};

/* TRM chapter 26 (reset) */
/* There are a myriad of registers before and after, this is a total hack.
 */
struct slcr_pcap_clock {
	vu32	clock;	/* 00 */
};


#define SLCR_LOCK_BASE ((struct slcr_lock *) 0xF8000000)
#define SLCR_MISC_BASE ((struct slcr_misc *) 0xF8000900)

#define SLCR_PCAP_CLOCK_BASE ((struct slcr_pcap_clock *) 0xF8000168)

#define SLCR_LOCK_KEY		0x767b
#define SLCR_UNLOCK_KEY		0xdf0d

#define SLCR_PCAP_CLOCK_ENA	1

#define SLCR_LS_DISABLE		0
#define SLCR_LS_PSPL		0xa
#define SLCR_LS_ALL		0xf

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
	int rv = 2;

	/* Set a sane control register value.
	 * Note that PCAP and such are enabled
	 */
	printf ( "Self test, control reg = %X\n", dp->control );
	dp->control = 0x4c006000;
	printf ( "Self test, control reg = %X\n", dp->control );

	if ( dp->control & CTRL_NIDEN )
	    return 1;
	
	save = dp->control;

	dp->control |= CTRL_NIDEN;
	if ( dp->control & CTRL_NIDEN )
	    rv = 0;

	dp->control = save;

	if ( dp->control & CTRL_NIDEN )
	    return 3;

	return rv;
}

/* If we set this bit low, then download a bitstream via JTAG
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

void
devcfg_init ( void )
{
	struct devcfg *dp = DEVCFG_BASE;
	struct slcr_lock *lockp = SLCR_LOCK_BASE;
	struct slcr_pcap_clock *cp = SLCR_PCAP_CLOCK_BASE;
	struct slcr_misc *mp = SLCR_MISC_BASE;
	int s;

	printf ( "Devcfg at %X\n", dp );

	// printf ( "mctrl is at: %X\n", &dp->mctrl );
	printf ( "status = %X\n", dp->status );

	s = devcfg_selftest ();
	if ( s )
	    printf ( "Self test fails: %d\n", s );
	else
	    printf ( "Self test OK\n" );

	/* Unlock the devcfg interface.
	 * Writing this magic value does it, but as near
	 * as I can tell, it has already been done.
	 */
	dp->unlock = 0x757BDF0D;

	/* U-boot also writes this as part of
	 * the unlock sequence.
	 */
	dp->rom_shadow = 0xffffffff;

	/* Example code unlocks, then locks each time it fiddles
	 * with a SLCR register, but I just unlock it and leave it.
	 */
	lockp->wp_unlock = SLCR_UNLOCK_KEY;

	/* Enable the PCAP clock */
	cp->clock |= SLCR_PCAP_CLOCK_ENA;
	printf ( "clock = %X\n", cp->clock );
	/* I find this set 0x0501
	 * which is divisor = 5, source = IO PLL
	 */

	/* Enable the level shifters */
	mp->level_shift = SLCR_LS_PSPL;

	printf ( "control = %X\n", dp->control );

	/* Clear 1/4 rate bit */
	dp->control &= ~CTRL_RATE;

	/* Select PCAP FOR  partial reconfiguration */
	dp->control |= CTRL_PCAP_PR;

	/* Enable PCAP */
	dp->control |= CTRL_PCAP_MODE;

	printf ( "control = %X\n", dp->control );
}

#define PCAP_LAST_XFER		1

/* The example code makes this 0xffffffff and then ORs on
 * the last transfer bit, which of course accomplishes nothing.
 * The TRM says the last 2 bits must be 01 and must match the
 * same bits in the source address, so we do this.
 */
// #define PL_ADDRESS		0xfffffffd
#define PL_ADDRESS		0xffffffff

/* Load a bitstream image into the PL.
 * Size is in 32 bit words.
 */
void
pl_load_image ( char *image, int size )
{
	struct devcfg *dp = DEVCFG_BASE;
	int tmo;

	/* Is DMA busy? */
	if ( dp->status & ST_DMA_Q_FULL ) {
	    printf ( "devcfg DMA queue full\n" );
	    return;
	}

	/* Is the PL ready */
	if ( ! (dp->status & ST_PL_INIT) ) {
	    printf ( "devcfg PL not ready\n" );
	    return;
	}

	printf ( "Devcfg I status: %X\n", dp->i_status );
	/* clear by writing to the bit */
	dp->i_status = ( IS_DMA_DONE | IS_D_P_DONE | IS_PL_DONE );
	printf ( "Devcfg I status: %X\n", dp->i_status );

	/* Clear internal PCAP loopback */
	dp->mctrl &= ~MC_LOOPBACK;

	/* Now, set up DMA
	 * When the 2 last bits of an address are
	 * 01 it indicates the last DMA of an overall transfer.
	 * As near as I can tell, writing into these 4 registers
	 * sets things in motion.
	 */
	printf ( "Set DMA for %X, %d words\n", image, size );
	printf ( "DMA write count: %d\n",  dp->write_count );

	dp->dma_src_addr = ((u32) image) | PCAP_LAST_XFER;
	dp->dma_dst_addr = PL_ADDRESS;

	printf ( "DMA write count: %d\n",  dp->write_count );

	/* sizes are in counts of 4 byte words */
	dp->dma_src_len = size;
	dp->dma_dst_len = 0;

	printf ( "Polling for DMA done\n" );
	for ( tmo = 500000; tmo; tmo-- )
	    if ( dp->i_status & IS_DMA_DONE )
		break;
	printf ( "Devcfg I status: %X, %d\n", dp->i_status, tmo );

	printf ( "Polling for PL done\n" );
	for ( tmo = 500000; tmo; tmo-- )
	    if ( dp->i_status & IS_PL_DONE )
		break;
	printf ( "Devcfg I status: %X, %d\n", dp->i_status, tmo );

	printf ( "DMA write count: %d\n",  dp->write_count );
	printf ( "DMA read count: %d\n",  dp->read_count );
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

	printf ( "PL image of %d words, loaded to %X\n", size, im );

	swap_em ( (u32 *) im, size );

	// pl_load_image ( im, size );
	zynq_load_full ( im, size*4 );
	// zynq_load_partial ( im, size );

	printf ( "Done loading PL image\n" );
	printf ( "Should be running now\n" );
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

/* This is a hook called by main()
 *  to allow testing.
 */
void
pl_test ( void )
{
	int cc;

	devcfg_init ();
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
