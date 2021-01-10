/* Button --
 * second bare metal program for the EBAZ4205
 * 
 * This watches for the S2 button to be pressed.
 *
 * Tom Trebisky  1-9-2021
 */

typedef volatile unsigned int vu32;
typedef unsigned int u32;

#define BIT(x)	(1<<(x))

/* The TRM keeps the register details in Appendix B
 */
struct zynq_uart {
	vu32	control;
	vu32	mode;
	vu32	ie;
	vu32	id;

	vu32	im;
	vu32	istatus;
	vu32	baud;
	vu32	rto;

	vu32	rfifo;
	vu32	mcontrol;
	vu32	mstatus;
	vu32	cstatus;

	vu32	fifo;
	vu32	baud_div;
	vu32	fc_delay;
	int	__pad1[2];

	vu32	tfifo;

};

#define UART0_BASE	((struct zynq_uart *) 0xe0000000)
#define UART1_BASE	((struct zynq_uart *) 0xe0001000)

/* The EBAZ uses UART1 */
#define UART_BASE	UART1_BASE

/* This is probably true, from a U-boot config file */
#define UART_CLOCK	100000000

/* Bits in the cstatus register */
#define CS_TXFULL	BIT(4)

/* --------------------------------------------------------------- */
/* gpio */

/* We have 54 MIO bits.  32 on port 0, 22 on port 1
 */

#define MIO_S2_BUTTON	20
#define MIO_S3_BUTTON	32

struct zynq_gpio {
	vu32	output0_low;
	vu32	output0_high;
	vu32	output1_low;
	vu32	output1_high;
	vu32	output2_low;
	vu32	output2_high;
	vu32	output3_low;
	vu32	output3_high;

	int	_pad1[8];

	vu32	output0;	/* 0x40 */
	vu32	output1;
	vu32	output2;
	vu32	output3;

	int	_pad2[4];

	vu32	input0;		/* 0x60 */
	vu32	input1;
	vu32	input2;
	vu32	input3;

	int	_pad3[101];

	vu32	dir0;		/* 0x204 */
	vu32	oe0;
	vu32	im0;
	vu32	ie0;
	vu32	id0;
	vu32	is0;
	vu32	it0;
	vu32	ip0;
	vu32	iany0;

	/* .... */
};

#define GPIO_BASE	((struct zynq_gpio *) 0xe000a000)

void
gpio_init ( void )
{
	struct zynq_gpio *gp = GPIO_BASE;

	/* Make this an input */
	gp->dir0 &= ~BIT(MIO_S2_BUTTON);
}

int
gpio_read ( void )
{
	struct zynq_gpio *gp = GPIO_BASE;

	return gp->input0;
}

/* --------------------------------------------------------------- */
/* slcr */

/* This controls the 4 level mux that configures the MIO pins. */

#define NUM_MIO		54

struct zynq_slcr {
	int pad0[448];
	vu32	mio_pin[NUM_MIO];	/* 0x700 */
	/* ... more ... */
};

/* default value is 0x1601 */
void
mio_pin_config ( int pin, int val )
{
}

#define SLCR_BASE	0xf8000000
/* --------------------------------------------------------------- */

void
uart_init ( void )
{
	/* nothing here, we rely on U-Boot */
}

void
uart_putc ( char c )
{
	struct zynq_uart *up = UART_BASE;

	while ( up->cstatus & CS_TXFULL )
	    ;
	up->fifo = c;
}

void
puts ( char *s )
{
	while ( *s ) {
	    if (*s == '\n')
		uart_putc('\r');
	    uart_putc(*s++);
	}
}

/* This gives about a 1 second delay */
void
delay ( void )
{
	volatile int count = 100000000;

	while ( count-- )
	    ;
}

void
main ( void )
{
	uart_init();
	int last;
	int curr;

	last = gpio_read ();

	for ( ;; ) {
	    curr = gpio_read ();
	    if ( curr != last ) {
		puts ( "- Change\n" );
		last = curr;
	    }
	    else
		puts ( "-\n" );
	    delay ();
	}

	/* NOTREACHED */
}

/* THE END */
