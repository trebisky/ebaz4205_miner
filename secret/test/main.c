/* Spit out endless characters on the uart.
 *
 * Zynq demo
 *
 * Tom Trebisky  5-29-2024
 */

typedef volatile unsigned int vu32;
typedef unsigned int u32;

void main ( void );

#define UART_DATA	0xe0001030

/* I don't expect it to work if we just pound
 * characters full tilt to the uart data register.
 */
void
dork_around ( void )
{
	asm volatile ( "dsb     sy" );
	asm volatile ( "isb     sy" );
	asm volatile ( "nop" );
	asm volatile ( "nop" );
	asm volatile ( "nop" );
	asm volatile ( "nop" );
	asm volatile ( "nop" );
	asm volatile ( "nop" );
	asm volatile ( "nop" );
	asm volatile ( "nop" );
}

void
delay ( void )
{
	// yields about 1 second deley
	// volatile int n = 999999;
	volatile int n = 99999;
	// volatile int n = 999;
	// volatile int n = 99;

	while ( n-- )
	    ;
}

/* Somehow the 'B' gets sent as a 00 byte.
 * which makes no sense.
 */
void
main ( void )
{
	vu32 *up = (vu32 *) UART_DATA;

	for ( ;; ) {
	    *up = 'A';
	    delay ();
	    *up = 'B';
	    delay ();
	}
}

/* This works,
 * with a 99 delay we only ever see 'A' -- apparently
 * the 'B' follows too quickly and is ignored.
 * With a bigger delay we see both.
 */
void
main2 ( void )
{
	vu32 *up = (vu32 *) UART_DATA;

	for ( ;; ) {
	    *up = 'A';
	    *up = 'B';
	    delay ();
	}
}

/* This works
 * clearly characters are being lost, we are writing to the uart
 * far faster than it can send data, but it keeps on as best it can.
 */
void
main1 ( void )
{
	vu32 *up = (vu32 *) UART_DATA;

	for ( ;; ) {
	    *up = 'A';
	    *up = 'B';
	    //dork_around ();
	    // delay ();
	}
}

/* THE END */
