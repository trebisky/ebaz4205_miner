/*
 * Copyright (C) 2016  Tom Trebisky  <tom@mmto.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation. See README and COPYING for
 * more details.
 */

#include "protos.h"

/* cq.c
 * Taken from Kyu/src/kyulib.c
 */

/* Allow 5 queues for now.
 * 1 for x86 keyboard
 * 2 for each of two x86 serial ports (in and out)
 * So far has only been used in x86 days.
 *
 * XXX - someday should add dynamic allocator both for
 * the control structure and the buffer itself.
 * We never expect to free these resources (maybe someday
 * if we should ever allow modular drivers, but even then
 * in a real embedded application, if you need a driver,
 * you always need it.)
 */
#define MAX_CQ		5

struct cqueue cq_table[MAX_CQ];
static int cq_next = 0;

struct cqueue *
cq_init ( int size )
{
	struct cqueue *qp;

	if ( cq_next >= MAX_CQ )
	    return (struct cqueue *) 0;

	qp = &cq_table[cq_next++];

	/* XXX - argument is ignored */
	qp->size = MAX_CQ_SIZE;
	qp->count = 0;
	qp->ip = qp->op = qp->bp = qp->buf;
	qp->limit = &qp->bp[qp->size];
	qp->toss = 0;

	return qp;
}

/* At first, was just going let folks examine
 * the count element of the structure, but
 * perhaps someday I will eliminate it and be
 * thankful for this accessor function.
 *
 * (Right now, I am keeping the count element,
 * it actually makes checks faster at interrupt
 * level unless I get a bit more clever and
 * sacrifice one element of storage.)
 * The key assertion is that *ip always points
 * to a valid place to dump a character.
 */
int
cq_count ( struct cqueue *qp )
{
	return qp->count;
#ifdef notdef
	/* works, but slower than just
	 * returning count
	 */
	if ( qp->ip < qp->op )
	    return qp->size - (qp->op - qp->ip);
	else
	    return qp->ip - qp->op;
#endif
}

/* return amount of available space in queue
 */
int
cq_space ( struct cqueue *qp )
{
	return qp->size - qp->count;
}

/* Almost certainly gets called from interrupt level.
 *	(must not block.)
 * Should surely have locks wrapped around it, in the
 * usual producer/consumer situation it is intended for.
 * (This will usually be called in an interrupt routine,
 *  where locking will be implicit.)
 */
void
cq_add ( struct cqueue *qp, int ch )
{
	if ( qp->count < qp->size ) {
	    qp->count++;
	    *(qp->ip)++ = ch;
	    if ( qp->ip >= qp->limit )
		qp->ip = qp->bp;
	} else {
	    qp->toss++;
	}
}

int
cq_toss ( struct cqueue *qp )
{
	return qp->toss;
}

/* Once upon a time, I had locking in here,
 * but now the onus is on the caller to do
 * any locking external to this facility.
 */
int
cq_remove ( struct cqueue *qp )
{
        int ch;

        if ( qp->count < 1 )
            return -1;
        else {
            ch = *(qp->op)++;
            if ( qp->op >= qp->limit )
                qp->op = qp->bp;
            qp->count--;
        }
        return ch;
}

/* THE END */
