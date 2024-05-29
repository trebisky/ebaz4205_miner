/* The game here is to get interrupts happening
 *   on the Orange Pi PC.
 *
 * After pulling our hair out with a simple interrupt demo,
 *  I just grabbed thw startup.S file from kyu and patch
 *  in a substantial but minimal set of files, making
 *  merciless hacks.  And it works!
 *
 * This is mostly about writing a driver for the ARM GIC device.
 * The interrupt source in question is TIMER 0, which
 * I have running to interrupt at 10 Hz.
 *
 * There is some cruft hanging around from the original
 *  timer experiments.
 *
 * Tom Trebisky  1-4-2017
 */

typedef volatile unsigned int vu32;
typedef unsigned int u32;

void main ( void );
void printf ( char *, ... );

/* Something in the eabi library for gcc wants this */
int
raise (int signum)
{
	return 0;
}

/* --------------------------------------- */

//void led_init ( void );
//void led_on ( void );
//void led_off ( void );
//void status_on ( void );
//void status_off ( void );

void uart_init ( void );
void uart_puts ( char * );

#define MS_300	50000000;

/* A reasonable delay for blinking an LED.
 * This began as a wild guess, but
 * in fact yields a 300ms delay
 * as calibrated below.
 */
void
delay_x ( void )
{
	volatile int count = MS_300;

	while ( count-- )
	    ;
}

#define MS_1	166667

void
delay_ms ( int ms )
{
	volatile int count = ms * MS_1;

	while ( count-- )
	    ;
}

#ifdef notdef
void
blink ( void )
{
	led_init ();

	for ( ;; ) {
	    led_off ();
	    status_on ();
	    // uart_puts("OFF\n");
	    delay_x ();
	    //led_on ();
	    //status_off ();
	    // uart_puts("ON\n");
	    delay_x ();
	}
}
#endif

void
show_stuff ( void )
{
	unsigned long id;
	unsigned long sp;

	id = 1<<31;
	printf ( "test: %08x\n", id );
	id = 0xdeadbeef;
	printf ( "test: %08x\n", id );

        asm volatile ("add %0, sp, #0" : "=r" (sp));
	printf ( "sp: %08x\n", sp );

        asm volatile ("mrc p15, 0, %0, c0, c0, 0" : "=r" (id));

	printf ( "ARM id register: %08x\n", id );

#ifdef notdef
// Special register on the H3 chip
#define ROM_START       ((unsigned long *) 0x01f01da4)

	id = *ROM_START;
	printf ( "rom before: %08x\n", id );

	*ROM_START = (unsigned long) blink;

	id = *ROM_START;
	printf ( "rom after: %08x\n", id );
#endif
}

#define PMCR_ENABLE     0x01    /* enable all counters */
#define PMCR_EV_RESET   0x02    /* reset all event counters */
#define PMCR_CC_RESET   0x04    /* reset CCNT */
#define PMCR_CC_DIV     0x08    /* divide CCNT by 64 */
#define PMCR_EXPORT     0x10    /* export events */
#define PMCR_CC_DISABLE 0x20    /* disable CCNT in non-invasive regions */

/* There are 4 counters besides the CCNT (we ignore them at present) */
#define CENA_CCNT       0x80000000
#define CENA_CTR3       0x00000008
#define CENA_CTR2       0x00000004
#define CENA_CTR1       0x00000002
#define CENA_CTR0       0x00000001

void
ccnt_enable ( int div64 )
{
        int val;

        // val = get_pmcr ();
	asm volatile ("mrc p15, 0, %0, c9, c12, 0" : "=r"(val) );
	// printf ( " PMCR = %08x\n", val );
        val |= PMCR_ENABLE;
        if ( div64 )
            val |= PMCR_CC_DIV;
        // set_pmcr ( val );
	asm volatile ("mcr p15, 0, %0, c9, c12, 0" : : "r"(val) );

	asm volatile ("mrc p15, 0, %0, c9, c12, 0" : "=r"(val) );
	// printf ( " PMCR = %08x\n", val );

        // set_cena ( CENA_CCNT );
	val = CENA_CCNT;
	asm volatile ("mcr p15, 0, %0, c9, c12, 1" : : "r"(val) );

	asm volatile ("mrc p15, 0, %0, c9, c12, 1" : "=r"(val) );
	// printf ( " CENA = %08x\n", val );
}


void
ccnt_reset ( void )
{
	int val;

        // set_pmcr ( get_pmcr() | PMCR_CC_RESET );
	asm volatile ("mrc p15, 0, %0, c9, c12, 0" : "=r"(val) );
	val |= PMCR_CC_RESET;
	asm volatile ("mcr p15, 0, %0, c9, c12, 0" : : "r"(val) );
}

static inline int 
ccnt_read ( void )
{
	int count;

	asm volatile ("mrc p15, 0, %0, c9, c13, 0" : "=r"(count) );
	return count;
}

#ifdef ORANGE_PI
#define GIG	1000000000
#define MEG	1000000

void
gig_delayX ( void )
{
	int count;

	ccnt_reset ();

	for ( ;; ) {
	    asm volatile ("mrc p15, 0, %0, c9, c13, 0" : "=r"(count) );
	    if ( count > GIG )
		break;
	}
}

/* OK for up to 16 seconds at 1 Ghz clock */
void
gig_delay ( int secs )
{
	int count;
	int limit = secs * GIG;

	ccnt_reset ();

	for ( ;; ) {
	    asm volatile ("mrc p15, 0, %0, c9, c13, 0" : "=r"(count) );
	    if ( count > limit )
		break;
	}
}

void
ms_delay ( int ms )
{
	int count;
	int limit = ms * MEG;

	ccnt_reset ();

	for ( ;; ) {
	    asm volatile ("mrc p15, 0, %0, c9, c13, 0" : "=r"(count) );
	    if ( count > limit )
		break;
	}
}
#endif

/* Near as I can tell we are running at 10 Mhz */
#define CLOCK_HZ	10000000
#define TICKS_PER_MS	(CLOCK_HZ / 1000)

void
ms_delay ( int ms )
{
	int count;
	int limit = ms * TICKS_PER_MS;

	ccnt_reset ();

	for ( ;; ) {
	    asm volatile ("mrc p15, 0, %0, c9, c13, 0" : "=r"(count) );
	    if ( count > limit )
		break;
	}
}

/* This gives about a 1 second delay
 * on the EBAZ4205 with what we have determined is
 * a 10 Mhz cpu clock.
 */
void
delay_1s ( void )
{
	volatile int count = 100000000;

	while ( count-- )
	    ;
}

static inline void
spin ( void )
{
	for ( ;; )
	    ;
}

/* --------------------------------------- */

void timer_init ( int );
void timer_watch ( void );
void gic_watch ( void );

void gic_init ( void );

extern volatile int timer_count;

void timer_test ( void )
{
	int i;
	int last_count;
	int cur_count;

	last_count = 0;

	i = 0;
	for ( ;; ) {
	    ms_delay ( 1000 );
	    cur_count = timer_count;
	    i++;
	    // asm volatile ("add %0, sp, #0" : "=r"(cur_sp) );
	    // printf ( "Count: %d sp = %h %d %d\n", timer_count, cur_sp, cur_count-last_count, i );
	    printf ( "Count: %d %d %d\n", timer_count, cur_count-last_count, i );
	    last_count = cur_count;
	}
}

/* Doing some kind of sleep is better than just spinning.
 * Using the WFE is experimental and seems to work fine.
 * The ms_delay is almost certainly not needed.
 */
void
snooze ( void )
{
	printf ( "Snoozing ...\n" );
	for ( ;; ) {

	    asm volatile ("wfe");

	    // ms_delay ( 1000 );
	    ms_delay ( 2 );
	}
}

void swap ( void );

void
main ( void )
{
#ifdef notdef
	int count1;
	int count2;
	int tick = 0;
	int val;
	int msecs;
	int cur_sp;
	int x;
#endif

	init_thread ();
	gic_init ();
	uart_init();

	uart_puts("\n" );
	printf ("Starting timer/interrupt test\n");

	// led_init ();
	// status_on ();
	// status_off ();
	// led_off ();

	ccnt_enable ( 0 );
	ccnt_reset ();

	timer_init ( 10000 );
	// timer_init ( 1000 );

	// timer_one ( 2000 );

	timer_count = 0;
	// last_count = timer_count;

	printf ( "Enabling IRQ\n" );
	ms_delay ( 100 );
	enable_irq ();

	// timer_watch ();
	// gic_watch ();

	// timer_test ();

	swap ();

	snooze ();
}

/* The stuff below is the code added for the "swap" demo.
 */
#define	OCM_CONFIG	0xf8000910

static void
ez_dump ( void )
{
	int i;
	vu32 *p;
	vu32 val;

	for ( i=0; i<256; i += 4 ) {
	    p = (vu32 *) i;
	    val = *p;
	    printf ( "%X: %X\n", i, val );
	}
}

/* The first 4 words of the bootrom are:
00000000:       ea00000f
00000004:       e320f000
00000008:       e320f000
0000000c:       e320f000
*/

/* SLCR unlock */
static void
unlock ( void )
{
	vu32 *p_unlock = (vu32 *) 0xf8000008;

	*p_unlock = 0xdf0ddf0d;
	asm volatile ( "dsb     sy" );
	asm volatile ( "isb     sy" );
}

/* I dumped addresses at 0, and I see the code for this demo
 * (which should be loaded to and running at 0x2000_0000)
 * So, as an experiment, I cleared whatever it is that is
 * sitting at 0, and sure enough I kill this code.
 */
static void
ocm_clear ( void )
{
	vu32 *addr = (vu32 *) 0;
	int n = 65536 / 4;

	while ( n-- ) {
	    *addr++ = 0xdeadbeef;
	}
}

static void
check ( vu32 *addr )
{
	vu32 val;
	u32 code;

	printf ( "check -- %X: ", addr );
	val = *addr;
	printf ( "%X --> ", val );
	code = ((u32) addr) >> 16;
	*addr = 0xdead0000 + code;
	val = *addr;
	printf ( "%X\n", val );
}

/* Running this on an EBAZ-4205 with 256M of ram, I see:
 * check -- 0FFFFFF0: DEA5BEEB --> DEAD0FFF
 * check -- 10000000: E10F0000 --> DEAD1000
 * check -- 20000000: DEAD1000 --> DEAD2000
 * check -- 30000000: DEAD2000 --> DEAD3000
 * check -- 3FFFFFF0: DEA5BEEB --> DEAD3FFF
 * check -- 40000000: 00000085 --> 00000085
 *
 * This all makes sense.  The same 1/4 G of ram is redundantly mapped
 * to 0, 1, 2, and 3 above.
 * 4 is not ram (it is access to the PL, see TRM page 112
 *
 * Most importantly, DDR is on top of (and hiding) OCM.
 *
 * The TRM shows 2 cases.  One where the address is filtered by the SCU
 * which is apparently this case, as the CPU will see DDR at address zero
 * in that case, just as we do.  If we can figure out how to turn off
 * SCU filtering, we might see OCM at address 0.
 * They say "the SCU can filter addresses destined for the OCM low address
 *    range to the DDR DRAM controller instead."
 * See page 1436 in the TRM for this (under APU (mpcore))
 * The SCU control register at 0xf8f00000 controls this.
 *
 * See page 60 of the TRM.  The SCU is the "snoop control unit".
 */

#define SCU_CONTROL	0xf8f00000

static void
scu_fiddle ( void )
{
	vu32 *p_scu = (vu32 *) SCU_CONTROL;

	printf ( "SCU control = %X\n", *p_scu );
	/* We read 3, the low bit is SCU enable */

	// *p_scu = 1;
	// Once we do this, we lose the processor

	// *p_scu = 0;
	// Once we do this, we also lose the processor

	printf ( "SCU control = %X\n", *p_scu );
}

static void
ram_check ( void )
{
	// If we do this, we get an undefined instruction exception
	// which is surprising, we ought to be well beyond that address.
	// check ( (vu32 *) 0x00000000 );
	check ( (vu32 *) 0x00010000 );
	check ( (vu32 *) 0x0ffffff0 );
	check ( (vu32 *) 0x10000000 );
	check ( (vu32 *) 0x20000000 );
	check ( (vu32 *) 0x30000000 );
	check ( (vu32 *) 0x3ffffff0 );
	check ( (vu32 *) 0x40000000 );	/* Works, but not ram */
}

int	_end;

void
swap ( void )
{
	vu32 *p_swap = (vu32 *) OCM_CONFIG;
	vu32 val;

	// ocm_clear ();
	ram_check ();

	scu_fiddle ();

	unlock ();

	printf ( "_end: %X\n", &_end );
	/* Shows 0x2000_2348 */

	val = *p_swap;
	// printf ( "OCM CONFIG = %8x\n", val );
	printf ( "OCM CONFIG = %X\n", val );
	// The above shows the value 0x1f

	ez_dump ();

	// *p_swap = 0x8;
	// 00000000: EB0F0009
	// 00000004: E38000F0
	// 00000008: E129F080
	// 0000000C: F3280080

	// *p_swap = 0xffffffff;
	// 00000000: E30F0001
	// 00000004: E38000D0
	// 00000008: E129F000
	// 0000000C: E3200080

	// *p_swap = 0x0;
	// 00000000: EB0F0809
	// 00000004: E38900F0
	// 00000008: E129F099
	// 0000000C: F3288082

	// Different results each time!
	// As it turns out, different results on each reboot.
	// But never any change when we mess with the OCM
	// register.

	asm volatile ( "dsb     sy" );
	asm volatile ( "isb     sy" );

	printf ( "\n" );
	// ez_dump ();
}

/* THE END */
