/* bitread
 *
 * read a Xilinx FPGA bitstream file
 *
 * Tom Trebisky  6-2-2022
 */

#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>

typedef unsigned char u_char;
typedef unsigned short u_short;
typedef unsigned int u_int;

//char *bit_path = "clocks_wrapper.bit";
char *bit_path = "no_file_here.bit";

char bin_path[128];

/* The current file is over 2M in size
 * 2083850 bytes
 */

int quiet = 0;

#define IM_SIZE	2200000
u_char image[IM_SIZE];

u_char *bin_buf;
int bin_size;

/*
 * Here are the first bytes of the first file I
 * began working with.
00000000 0009 0ff0 0ff0 0ff0 0ff0 0000 0161 0030                a 0
00000010 636c 6f63 6b73 5f77 7261 7070 6572 3b55   clocks_wrapper;U
00000020 7365 7249 443d 3058 4646 4646 4646 4646   serID=0XFFFFFFFF
00000030 3b56 6572 7369 6f6e 3d32 3031 392e 3100   ;Version=2019.1
00000040 6200 0c37 7a30 3130 636c 6734 3030 0063   b  7z010clg400 c
00000050 000b 3230 3232 2f30 362f 3032 0064 0009     2022/06/02 d
00000060 3136 3a31 363a 3039 0065 001f cb9c ffff   16:16:09 e
00000070 ffff ffff ffff ffff ffff ffff ffff ffff
00000080 ffff ffff ffff ffff ffff ffff ffff 0000
00000090 00bb 1122 0044 ffff ffff ffff ffff aa99      " D
 */

void
error ( char *msg )
{
	fprintf ( stderr, "Error: %s\n", msg );
	exit ( 1 );
}

void
read_image ( char *path )
{
	int fd;
	int n;

	fd = open ( path, O_RDONLY );
	if ( fd < 0 )
	    error ( "Cannot open input file" );
	n = read ( fd, image, IM_SIZE );
	if ( ! quiet )
	    printf ( "%d bytes read\n", n );
	close ( fd );
}

void
read_header ( void )
{
	int len;
	int pos;
	u_char type;
	char *date, *time;

	/* Usually 9 bytes of unknown rubbish
	 *  starts the file.
	 * We just skip past it.
	 */
	pos = 0;
	len = ntohs ( *(u_short *) &image[pos] );
	// printf ( "Len = %d\n", len );
	pos = len + 2;

	/* The next count is usually 1 byte.
	 * the single byte should be the letter 'a'
	 */
	len = ntohs ( *(u_short *) &image[pos] );
	// printf ( "Len = %d\n", len );
	pos += 2;

	/* now the scheme changes.
	 * we get a single byte (a,b,c,d,e)
	 * followed by a 2 byte count,
	 * followed by a payload.
	 * but the game changes again when we encounter
	 * the 'e' byte.
	 */
	for ( ;; ) {
	    type = image[pos++];
	    // printf ( "Type: %c\n", type );

	    /* 'e' indicates the end of the header.
	     * it is followed by a 4 byte count,
	     * which is the size of the rest of the file.
	     */
	    if ( type == 'e' ) {
		len = ntohl ( *(u_int *) &image[pos] );
		pos += 4;
		break;
	    }

	    len = ntohs ( *(u_short *) &image[pos] );
	    pos += 2;

	    if ( ! quiet ) {
		if ( type == 'a' )
		    printf ( "Project: %s\n", &image[pos] );
		else if ( type == 'b' )
		    printf ( "Device: %s\n", &image[pos] );
		else if ( type == 'c' )
		    date = &image[pos];
		else if ( type == 'd' ) {
		    time = &image[pos];
		    printf ( "Date: %s  %s\n", date, time );
		} else {
		    printf ( "Type: %c, len, pos = %d %d\n", type, len, pos );
		    printf ( "%c: %s\n", type, &image[pos] );
		}
	    }

	    pos += len;
	}

	if ( ! quiet ) {
	    printf ( "Header size: %d bytes\n", pos );
	    printf ( "Data size: %d bytes\n", len );
	}

	bin_buf = &image[pos];
	bin_size = len;
}

void
write_binary ( void )
{
	char *p;
	int fd;

	strcpy ( bin_path, bit_path );
	p = strchr ( bin_path, '.' );
	strcpy ( p, ".bin" );

	printf ( "extract %s\n", bin_path );

	fd = open ( bin_path, O_WRONLY | O_CREAT | O_TRUNC, 0644 );
	if ( fd < 0 )
	    error ( "Cannot open output file" );
	write ( fd, bin_buf, bin_size );
	close ( fd );
}

/* Convert the bitstream binary into C source.
 * I invented a simple quick and dirty compression scheme that
 *  just aims to compress big runs of zeros.
 * The emphasis is on simplicity for now.
 *
 * The first "customer" for this will be the ebaz "blink1" project
 * so you can look for the decompressor there.
 *
 * The PCAP counts in words of 32 bits when it sends the bitstream to the PL,
 * so we round/pad this to a tidy multiple of 4 bytes.
 */

#define ZFLAG	0x80000000
#define EFLAG	0xC0000000

void
write_code ( void )
{
	u_int *ip;
	int i, j;
	int len;
	int rem;
	int wsize = sizeof(u_int);
	int state;
	int count;
	int zcount;
	u_int *datap;

	if ( bin_size % wsize ) {
	    rem = bin_size % wsize;
	    memset ( &bin_buf[bin_size], 0, wsize-rem );
	    bin_size += wsize - rem;
	    printf ( "// Padded to %d bytes, (%d words)\n", bin_size, bin_size/wsize );
	}

	printf ( "//data\n" );

	len = bin_size / wsize;
	ip = (u_int *) bin_buf;

	zcount = 0;
	count = 0;
	if ( *ip ) {
	    state = 1;
	    datap = ip;
	} else
	    state = 0;

	printf ( "unsigned int pl_comp_data[] = {\n" );
	for ( i=0; i<len; i++ ) {
	    // printf ( "Look %4d: %08x\n", i, *ip );
	    if ( *ip ) {
		if ( state ) {
		    ++count;
		} else {
		    zcount = count;
		    // printf ( " Z: %d\n", count );
		    count = 1;
		    state = 1;
		    datap = ip;
		    // printf ( "Mark: %08x\n", *datap );
		}
	    } else {
		if ( state ) {
		    // printf ( "NZ: %d\n", count );
		    printf ( "  0x%08x, 0x%08x,\n", ZFLAG | zcount, count );
		    // printf ( "  DATA\n" );
		    for ( j=0; j<count; j++ )
			printf ( "  0x%08x,\n", datap[j] );
		    count = 1;
		    state = 0;
		} else {
		    ++count;
		}
	    }
	    ip++;
	}

	// printf ( "END END END %d %d\n", zcount, count );

	if ( zcount )
	    printf ( "  0x%08x,\n", ZFLAG | zcount );
	if ( count ) {
	    printf ( "  0x%08x,\n", count );
	    for ( j=0; j<count; j++ )
		printf ( "  0x%08x,\n", datap[j] );
	}

	printf ( "  0x%08x };\n", EFLAG );

#ifdef notdef
	for ( i=0; i<len; i++ ) {
	    if ( *ip ) {
		if ( state ) {
		    ++count;
		} else {
		    zcount = count;
		    printf ( " Z: %d\n", count );
		    count = 1;
		    state = 1;
		}
	    } else {
		if ( state ) {
		    printf ( "NZ: %d\n", count );
		    count = 1;
		    state = 0;
		} else {
		    ++count;
		}
	    }
	    ip++;
	}
#endif
}

/* default option is just to list the header info and exit */
int option = 'l';

int
main ( int argc, char **argv )
{
	char *p;

	argc--;
	argv++;

	while ( argc && argv[0][0] == '-' ) {
	    p = *argv;
	    if ( *p == '-' && p[1] ) {
		option = p[1];
	    }
	    argc--;
	    argv++;
	}

	if ( argc < 1 )
	    error ( "usage: bitread file.bit" );

	bit_path = argv[0];

	if ( option == 'c' )
	    quiet = 1;

	read_image ( bit_path );
	read_header ();

	if ( option == 'e' ) {
	    write_binary ();
	}

	if ( option == 'c' ) {
	    write_code ();
	}
	return 0;
}

/* THE END */
