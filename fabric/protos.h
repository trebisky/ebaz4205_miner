/* protos.h
 *
 * prototypes for Ebaz bare metal projects
 *
 *  Tom Trebisky 2-28-2023
 */

/* XXX - the following fixed size should really
 * be an argument to cq_init() and be dynamically
 * allocated.
 */

#define MAX_CQ_SIZE     1024

struct cqueue {
        char    buf[MAX_CQ_SIZE];
        char    *bp;
        char    *ip;
        char    *op;
        char    *limit;
        int     size;
        int     count;
        int     toss;
};

struct cqueue * cq_init ( int );
void cq_add ( struct cqueue *, int );
int cq_remove ( struct cqueue * );
int cq_count ( struct cqueue * );

#ifndef NULL
#define NULL    (0)
#endif

/* THE END */
