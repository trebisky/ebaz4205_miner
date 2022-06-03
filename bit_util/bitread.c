/* bitread
 *
 * read a Xilinx FPGA bitstream file
 *
 * Tom Trebisky  6-2-2022
 */

#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <arpa/inet.h>

typedef unsigned char u_char;
typedef unsigned short u_short;
typedef unsigned int u_int;

char *bit_path = "clocks_wrapper.bit";
char bin_path[128];

/* The current file is over 2M in size
 * 2083850 bytes
 */

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
read_image ( char *path )
{
	int fd;
	int n;

	fd = open ( path, O_RDONLY );
	n = read ( fd, image, IM_SIZE );
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

	    pos += len;
	}

	printf ( "Header size: %d bytes\n", pos );
	printf ( "Data size: %d bytes\n", len );

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
	write ( fd, bin_buf, bin_size );
	close ( fd );
}

void
write_code ( void )
{
	u_int *ip;
	int i;
	int len;

	len = bin_size / sizeof(u_int);

	ip = (u_int *) bin_buf;
	for ( i=0; i<len; i++ ) {
	    if ( *ip )
		printf ( "%5d: %08x\n", i, *ip );
	    ip++;
	}
}


int option = 'l';

int
main ( int argc, char **argv )
{
	char *p;

	argc--;
	argv++;

	while ( argc ) {
	    p = *argv;
	    if ( *p == '-' && p[1] ) {
		option = p[1];
	    }
	    argc--;
	    argv++;
	}

	read_image ( bit_path );
	read_header ();

	if ( option == 'e' ) {
	    write_binary ();
	}

	if ( option == 'c' ) {
	    write_code ();
	}
}

/* THE END */
