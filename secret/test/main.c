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
main ( void )
{
	vu32 *up = (vu32 *) UART_DATA;

	for ( ;; ) {
	    *up = 'N';
	    dork_around ();
	}
}

/* THE END */
