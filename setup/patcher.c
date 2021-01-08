/* patcher.c
 *
 * Tom Trebisky  1-7-2021
 */

#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>

char *infile = "b19.img";

#define IM_SIZE		128*1024
#define ENV_OFFSET	0xbb5a

char image[IM_SIZE];

/* 128K image is 0 to 0x1ffff */

/* ------------ */

void dopey ( void )
{
    char test[] = "dog " "fish";
    printf ( "Here: %s\n", test );
}

void
read_file ( char *buf )
{
    int f;
    int n;

    f = open ( infile, O_RDONLY );
    n = read ( f, buf, IM_SIZE );
    close ( f );
}

void
dump_env ( char *env )
{
    char *p;

    p = env;
    while ( *p ) {
	printf ( "%s\n", p );
	p += strlen(p) + 1;
    }

}

int
main ( int argc, char **argv )
{
    read_file ( image );
    dump_env ( &image[ENV_OFFSET] );
}

/* THE END */
