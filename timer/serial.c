/* serial.c --
 * lame serial driver for the EBAZ4205
 *
 * relies entirely on U-Boot initialization
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

#define IRQ_UART0	69
#define IRQ_UART1	82

#define UART0_BASE	((struct zynq_uart *) 0xe0000000)
#define UART1_BASE	((struct zynq_uart *) 0xe0001000)

/* The EBAZ uses UART1 */
#define UART_BASE	UART1_BASE
#define IRQ_UART	IRQ_UART1

/* This is probably true, from a U-boot config file */
#define UART_CLOCK	100000000

/* Bits in the IE register */
#define IE_RXE		BIT(1)
#define IE_TXE		BIT(3)

/* Bits in the cstatus register */
#define CS_TXFULL	BIT(4)

void uart_handler ( int );

void
uart_init ( void )
{
	struct zynq_uart *up = UART_BASE;

	/* We rely on U-Boot to have done initialization,
	 * at least so far, we just make some tweaks here
	 * to let us fool around with interrupts.
	 */
	up->ie = IE_RXE | IE_TXE;

	irq_hookup ( IRQ_UART, uart_handler, 0 );
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
uart_puts ( char *s )
{
	while ( *s ) {
	    if (*s == '\n')
		uart_putc('\r');
	    uart_putc(*s++);
	}
}

void
uart_handler ( int xxx )
{
	// ACK the interrupt 
	printf ( "Uart interrupt\n" );
}


/* THE END */
