/* hack.c
 *
 * The idea here is to deposit a block of instructions into an
 * existing object file so I can disassemble it.
 *
 * Tom Trebisky  1-18-2021
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>

char *victim = "prf.o";

#define IM_SIZE	64*1024
char image[IM_SIZE];

int size;

void patch_it ( void );

void
error ( char *str )
{
	printf ( "error: %s\n", str );
	exit ( 1 );
}

int
main ( int argc, char **argv )
{
	int fd;
	int ofd;
	int n;

	fd = open ( victim, O_RDONLY );
	if ( fd < 0 )
	    error ( "open" );
	n = read ( fd, image, IM_SIZE );
	printf ( "%d bytes read\n", n );
	close ( fd );
	size = n;

	patch_it ();

	ofd = open ( "hack.out", O_WRONLY|O_CREAT, 0644 );
	write (ofd, image, n );
	close ( ofd );
}

int
find_offset ( void )
{
	int data1 = 0xe1500001;
	int data2 = 0x812fff1e;
	int nl = size / sizeof(int);
	int *ip;
	int i;

	ip = (int *) image;
	for ( i=0; i<size; i++ ) {
	    if ( ip[i] != data1 )
		continue;
	    if ( ip[i+1] != data2 )
		continue;
	    return i;
	}
	return -1;
}

/* This is what I found at 0xffffe00 */

int patch2[] = {
0xE5801000, 0xE5856000, 0xF57FF04F, 0xF57FF06F,
0xE594B000, 0xE5849000, 0xE3A0C002, 0xE58AC000,
0xE3A0C000, 0xE58AC000, 0xF57FF04F, 0xF57FF06F,
0xE584B000, 0xF57FF04F, 0xF57FF06F, 0xE3E040E7,
0xE5947000, 0xE3570008, 0x0A000001, 0xE35E0102,
0x1A00000D, 0xE3E040F7, 0xE5947000, 0xE3E050F3,
0xE5958000, 0xE5878000, 0xE3E040E7, 0xE5947000,
0xE3570008, 0x0A000004, 0xE3E040EF, 0xE5947000,
0xE3E050EB, 0xE5958000, 0xE5878000, 0xE3E040FF,
0xE5947000, 0xE3E050FB, 0xE5958000, 0xE5878000,
0xF57FF04F, 0xF57FF06F, 0xE5832000, 0xF57FF04F,
0xE12FFF1E, 0x00000000, 0x00000000, 0x00000000
};

/* This is what is at 0xffffff20 */

int patch[] = {
0xF57FF04F, 0xE320F002, 0xEAFFFFFC, 0xF57FF04F,
0xE320F002, 0xE3E0000F, 0xE590E000, 0xE37E00D4,
0x0AFFFFF9, 0xEE070F15, 0xEE070FD5, 0xEE080F17,
0xE3A04000, 0xEE014F10, 0xE12FFF1E, 0x00000000
};


/* After some shenanigans we convert this dump to
 * the assembly language below.

FFFFFF20  F57FF04F E320F002 EAFFFFFC F57FF04F
FFFFFF30  E320F002 E3E0000F E590E000 E37E00D4
FFFFFF40  0AFFFFF9 EE070F15 EE070FD5 EE080F17
FFFFFF50  E3A04000 EE014F10 E12FFF1E 00000000

  20:   f57ff04f        dsb     sy
  24:   e320f002        wfe
  28:   eafffffc        b       20 ??

  2c:   f57ff04f        dsb     sy
  30:   e320f002        wfe
  34:   e3e0000f        mvn     r0, #15		; 0xfffffff0
  38:   e590e000        ldr     lr, [r0]
  3c:   e37e00d4        cmn     lr, #212        ; 0xd4
  40:   0afffff9        beq     2c

  44:   ee070f15        mcr     15, 0, r0, cr7, cr5, {0}
  48:   ee070fd5        mcr     15, 0, r0, cr7, cr5, {6}
  4c:   ee080f17        mcr     15, 0, r0, cr8, cr7, {0}
  50:   e3a04000        mov     r4, #0
  50:   ee014f10        mcr     15, 0, r4, cr1, cr0, {0}
  50:   e12fff1e        bx      lr
  50:   00000000
*/


void
patch_it ( void )
{
	int off;
	int n;
	int i;
	int *ip;

	off = find_offset ();
	printf ( "Offset = %d\n", off );
	printf ( "Offset = %08x\n", off * 4 );
	printf ( "Patch = %d\n", sizeof(patch)/sizeof(int) );
	n = sizeof(patch)/sizeof(int);

	ip = (int *) image;
	for ( i=0; i<n; i++ ) {
	    ip[i+off] = patch[i];
	}
}

/* THE END */
