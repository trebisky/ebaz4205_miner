/* test_data
 *
 * Tom Trebisky  6-3-2022
 *
 * This is a test fixture to see if my bitstream
 *  compress/decompress will regenerate the original data.
 */
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>

typedef unsigned int u_int;

/* This references a compressed bitstream compiled in another file.
 */
extern u_int pl_comp_data[];

#define PL_SIZE	1024*1024
u_int pl_buffer[PL_SIZE];

#define ZFLAG   0x80000000
#define EFLAG   0xC0000000

int
pl_expand ( u_int *dest, u_int *src )
{
	u_int *ip;
	u_int *dp;
	int n;

	ip = src;
	dp = dest;

	//printf ( "First: %08x\n", *ip );

	while ( *ip != EFLAG ) {
	    // printf ( "Got: %08x\n", *ip );
	    if ( *ip & ZFLAG ) {
		n = *ip & ~ZFLAG;
		//printf ( "%d zeros\n", n );
		while ( n-- )
		    *dp++ = 0;
		++ip;
	    } else {
		n = *ip++;
		//printf ( "%d data\n", n );
		while ( n -- )
		    *dp++ = *ip++;
	    }
	}
	n = dp - dest;
	printf ( "Generated %d words (%d bytes)\n", n, n*4 );
	return n*4;
}

int
main ( int argc, char **argv )
{
	int nbytes;
	int fd;

	// printf ( "%d\n", sizeof(pl_comp_data) );
	// printf ( "%08x\n", pl_comp_data[0] );
	nbytes = pl_expand ( pl_buffer, pl_comp_data );

	fd = open ( "verify.bin", O_WRONLY | O_CREAT | O_TRUNC, 0644 );
	if ( fd < 0 ) {
	    printf ( "Disaster\n" );
	    return 1;
	}
	write ( fd, pl_buffer, nbytes );
	close ( fd );
	return 0;
}

/* THE END */
