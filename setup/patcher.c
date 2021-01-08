/* patcher.c
 *
 * Tom Trebisky  1-7-2021
 */

#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>

char *infile = "b19.img";
char *outfile = "b19p.img";

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
write_file ( char *buf )
{
    int f;
    int n;

    f = creat ( outfile, 0664 );
    n = write ( f, buf, IM_SIZE );
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

static char *ip;

void
init_env ( void )
{
    ip = &image[ENV_OFFSET];
}

void
add_env ( char *s )
{
    strcpy ( ip, s );
    ip += strlen(s) + 1;
}

int
main ( int argc, char **argv )
{
    read_file ( image );
    dump_env ( &image[ENV_OFFSET] );

    init_env ();
    add_env ( "ipaddr=192.168.0.80" );
    add_env ( "serverip=192.168.0.5" );
    add_env ( "ethaddr=00:0a:35:00:01:22" );
    add_env ( "bootdelay=3" );
    add_env ( "bootaddr=0x20000000" );
    add_env ( "boot_kyu=echo Booting Kyu via dhcp ; dhcp ${bootaddr}; go ${bootaddr}" );
    add_env ( "boot_tftp=echo Booting Kyu via tftp ; tftpboot ${bootaddr} bitcoin.bin; go ${bootaddr}" );
    // Go with tftp, dhcp gets weird ideas about the filename (C0A80050.img)
    // This is the IP address converted to hex, but it should be using bitcoin.bin
    add_env ( "bootcmd=run boot_tftp" );
    // add_env ( "bootcmd=run boot_kyu" );
    add_env ( "" );

    /* XXX ??? Does U-Boot do a CRC check on the env information ? */
    /* And what about our boot address ? */

    write_file ( image );

    printf ( "\n" );
    dump_env ( &image[ENV_OFFSET] );
}

/* THE END */
