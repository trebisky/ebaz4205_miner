/* cores.c
 *
 * This is the heart of the amp "project"
 *
 * Tom Trebisky  1-15-2017
 */

typedef void (*vfptr) ( void );

volatile int core_signal;

#ifdef notdef
#endif

/* Simply patching in this C function did not work.
 * Heaven knows if it even has an decent stack pointer,
 * and how much trouble that might get it into before
 * it could run this code.
 */

#define UART_FIFO	0xe0001030

void
start_core ( void )
{
	int sp;

	core_signal = 0xf00;

	/* This actually works */
	(*(unsigned long *) UART_FIFO) = '$';

	printf ( "New core started\n" );

	asm volatile ("add %0, sp, #0\n" :"=r"(sp));
	printf ( "Core1 SP = %h\n", sp );

	printf ( "Core1 spinning\n" );
	for ( ;; ) ;
}

void
start_core2 ( void )
{
	core_signal = 0xf00;

	// uart_core ();

	(*(unsigned long *) UART_FIFO) = '$';

	printf ( "New core started\n" );

	printf ( "Core1 spinning\n" );
	for ( ;; ) ;
}

unsigned int core_stack = 0x0f000000;

/* Allegedly the second core is happily spinning away in OCM
 * in addresses 0xfffffe00.  It spends most of its time in WFE
 * (or perhaps WFI), but when it pops out, it examines the magic
 * location and if it is non-zero, branches there.
 */

#define MAGIC_LOCATION	0xfffffff0

/* For what it is worth, the SP for cpu0 is 0x0f3056f0 as handed to us
 * by U-Boot.  There is nothing magic about this other than it is near
 * the end of DDR ram.
 */
#define OCM_BASE	0xfffc0000
#define OCM_SIZE	(256 * 1024)

void
search_ocm ( void )
{
	int *ip;
	int n;
	int i;
	int val;

	n = OCM_SIZE / sizeof(int);

	val = 0xe320f002;	/* wfe */
	ip = (int *) OCM_BASE;
	for ( i=0; i<n; i++ ) {
	    if ( ip[i] == val )
		printf ( "WFE at %h\n", &ip[i] );
	}

	val = 0xe320f003;	/* wfi */
	ip = (int *) OCM_BASE;
	for ( i=0; i<n; i++ ) {
	    if ( ip[i] == val )
		printf ( "WFI at %h\n", &ip[i] );
	}
}

/* The following code is just "trash" code to give me
 * information.  In particular, I wanted to know the binary
 * encoding of these instructions, which can be fetched from
 * the dump file.
20001620:       e320f004        sev
20001624:       e320f002        wfe
20001628:       e320f003        wfi
*/

void
ez_core ( void )
{
	int x;

	x = core_stack;
	printf ( "Core stack = %h\n", x );
	asm volatile ("sev\n");
	asm volatile ("wfe\n");
	asm volatile ("wfi\n");
}

/* This is in start.S */
extern void new_core ( void );

/* This is in uart.c */
extern void uart_core ( void );

/* Related to the above, here is a log and description of what
 * I am doing.  First I coded up uart_core() in uart.c, which
 * is a simple loop that should send out 'E' 50 times to the uart.
 * I tested this by calling it from below by core0, and it does
 * work, but only sends about 12 characters.
 * maybe the FIFO is full -- perhaps previous characters are still
 * in the FIFO.  A delay before the call would let the FIFO empty.
 * And indeed, this works.
 *
 * Next I simply let the new core branch to uart_core() without even
 * bounce through start.S.  Looking at the disassembled code, I
 * cannot see how this wouldn't work, but I see no output.
 *
 * The next idea is to relocate this code to the start of OCM
 * and see if it will run there.  This does not work either.
 */

static void dump_ocm ( void );

/* This just needs to work, it doesn't need to be fast or slick
 */
void
mem_copy ( char *s, char *d, int num )
{
	while ( num-- ) {
	    *d++ = *s++;
	}
}

#define OCM_LOOP	0xfffffe00

void
core_test ( void )
{
	vfptr *vp;
	vfptr *rp;
	int i;
	int *ip;

	/* Caching could be causing trouble here. */
	core_signal = 0;

	ip = (int *) MAGIC_LOCATION;
	printf ( "Magic found with %h\n", *ip );
	printf ( "Magic found with %08x\n", *ip );
	printf ( "Magic found with %h\n", *ip );
	printf ( "Magic found with %016x\n", *ip );
	printf ( "Magic found with %16x\n", *ip );
	i = 345;
	printf ( "test: %d\n", i );
	printf ( "test: %5d\n", i );
	printf ( "test: %05d\n", i );

	vp = (vfptr *) MAGIC_LOCATION;

	// a direct call to C code does not work.
	// *vp = start_core;

#ifdef notdef
	/* relocate uart_core() to ocm
	 * looking at the dump (disassembly) this
	 * routine is 8 instructions, but I move twice
	 * as much, just to be sure.
	 */
	mem_copy ( (char *)uart_core, (char *)OCM_BASE, 16 * 4 );
	printf ( "relocated from %h\n", (char *)uart_core );
	ms_delay ( 1000 );

	/* This yields a prefetch abort.
	 * apparently the MMU is setup so that
	 * OCM addresses are invalid for instruction fetch.
	 */
	rp = (vfptr *) OCM_BASE;
	(*rp) ();

	// this will run code in start.S
	// *vp = new_core;

	/* These are exactly the same */
	// *vp = (void (*)(void)) OCM_BASE;
	*vp = (vfptr) OCM_BASE;
#endif

	// dump_ocm ();
	search_ocm ();

	// dump_ocm ();
	// printf ( "\n" );

	// show_reg ( "Magic: ", vp );
	// ez_core ();

	// This works and yielded the first success.
	// *vp = uart_core;

	// try this (direct jump to C code)
	// this works! (even the calls to printf)
	*vp = start_core;

	// arm_sev ();
	asm volatile ("sev\n");

	ms_delay ( 1500 );
	printf ( "\n" );

	// Test this routine in core0
	// uart_core ();

	// for ( i=0; i<60; i++ ) {
	for ( i=0; i<5; i++ ) {
	    ms_delay ( 2000 );
	    uart_puts ( "." );
	    if ( core_signal ) {
		printf ( "Eureka: %h\n", core_signal );
		core_signal = 0;
		break;
	    }
	}

	printf ( "\n" );
	printf ( "Done, core0 spinning\n" );
	for ( ;; ) ;
}

/* I was curious what was in the block of instructions in OCM.
 * Here is what we find:

FFFFFE00  E5801000 E5856000 F57FF04F F57FF06F
FFFFFE10  E594B000 E5849000 E3A0C002 E58AC000
FFFFFE20  E3A0C000 E58AC000 F57FF04F F57FF06F
FFFFFE30  E584B000 F57FF04F F57FF06F E3E040E7
FFFFFE40  E5947000 E3570008 0A000001 E35E0102
FFFFFE50  1A00000D E3E040F7 E5947000 E3E050F3
FFFFFE60  E5958000 E5878000 E3E040E7 E5947000
FFFFFE70  E3570008 0A000004 E3E040EF E5947000
FFFFFE80  E3E050EB E5958000 E5878000 E3E040FF
FFFFFE90  E5947000 E3E050FB E5958000 E5878000
FFFFFEA0  F57FF04F F57FF06F E5832000 F57FF04F
FFFFFEB0  E12FFF1E 00000000 00000000 00000000
FFFFFEC0  00000000 00000000 00000000 00000000
FFFFFED0  00000000 00000000 00000000 00000000
FFFFFEE0  00000000 00000000 00000000 00000000

*/

static void
dump_lll ( void *addr, int n )
{
        unsigned int *p;
        int i;

        p = (unsigned int *) addr;

        printf ( "dumping %d lines\n", n );

        while ( n-- ) {
            // printf ( "%08x  ", (long) addr );
            printf ( "%h  ",  addr );

            for ( i=0; i<4; i++ )
                printf ( "%h ", *p++ );
                // printf ( "%08x ", *p++ );

            printf ( "\n" );
            addr += 16;
        }
}


static void
dump_ocm ( void )
{
	dump_lll ( (void *) 0xfffffe00, 128 );
}

/* THE END */
