/* test_data
 *
 * Tom Trebisky  6-3-2022
 *
 * This is a test fixture to see if my bitstream
 *  compress/decompress will regenerate the original data.
 */

#include <stdio.h>

typedef unsigned int u_int;

extern u_int pl_comp_data[];

#define PL_SIZE	1024*1024
u_int pl_buffer[PL_SIZE];

#define ZFLAG   0x80000000
#define EFLAG   0xC0000000

void
pl_expand ( u_int *dest, u_int *src )
{
	u_int *ip;
	u_int *dp;
	int n;

	ip = src;
	dp = dest;

	printf ( "First: %08x\n", *ip );

	while ( *ip != EFLAG ) {
	    printf ( "Got: %08x\n", *ip );
	    if ( *ip & ZFLAG ) {
		n = *ip & ~ZFLAG;
		printf ( "%d zeros\n", n );
		while ( n-- )
		    *dp++ = 0;
		++ip;
	    } else {
		n = *ip++;
		printf ( "%d data\n", n );
		while ( n -- )
		    *dp++ = *ip++;
	    }
	}
	n = dp - dest;
	printf ( "Generated %d words (%d bytes)\n", n, n*4 );
}

int
main ( int argc, char **argv )
{
	// printf ( "%d\n", sizeof(pl_comp_data) );
	// printf ( "%08x\n", pl_comp_data[0] );
	pl_expand ( pl_buffer, pl_comp_data );
}

/* THE END */
