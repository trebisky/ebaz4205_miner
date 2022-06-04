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
void snooze ( void )
{
	printf ( "Snoozing ...\n" );
	for ( ;; ) {

	    asm volatile ("wfe");

	    // ms_delay ( 1000 );
	    ms_delay ( 2 );
	}
}

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
	printf ("Starting blink1 test\n");

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

	devcfg_init ();

	pl_load ( (void *) 0 );

	// timer_watch ();
	// gic_watch ();

	// timer_test ();

	snooze ();
}

/* THE END */
