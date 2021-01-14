/* Zynq-7000 TTC driver
 * The TTC is the "triple timer counter"
 * There are 2 of these, so we have 6 timers
 *
 * Tom Trebisky  1-13-2021
 */

#define TTC0_BASE	0xf8001000
#define TTC1_BASE	0xf8002000

#define IRQ_TTC0A	42
#define IRQ_TTC0B	43
#define IRQ_TTC0C	44

#define IRQ_TTC1A	69
#define IRQ_TTC1B	70
#define IRQ_TTC1C	71

void timer_handler ( int );

volatile int timer_count;

void
timer_init ( void )
{
    timer_count = 0;

    /* Who knows XXX */
    irq_hookup ( IRQ_TTC0A, timer_handler, 0 );
}

void
timer_ack ( void )
{
        //struct h3_timer *hp = TIMER_BASE;

        // hp->irq_status = IE_T0;
}

/* Called at interrupt level */
void
timer_handler ( int xxx )
{
	printf ( "Timer interrupt %d\n", xxx );
        timer_count++;
        timer_ack ();
}

/* THE END */
