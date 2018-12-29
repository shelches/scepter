#include <errno.h>
#include <signal.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <sys/select.h>
#include <sys/time.h>

void Log( char *fmt, ... )
{
	FILE *fp = fopen( "/tmp/mud.log", "a" );

	if	( fp ) {
		va_list ap;
		time_t t = time( NULL );
		struct tm *tp = localtime( &t );

		fprintf( fp
			, "%.2d/%.2d %.2d:%.2d:%.2d %5d "
			, tp->tm_mon + 1
			, tp->tm_mday
			, tp->tm_hour
			, tp->tm_min
			, tp->tm_sec
			, getpid( )
			);

		va_start( ap, fmt );
		vfprintf( fp, fmt, ap );
		va_end( ap );

		putc( '\n', fp );
		fclose( fp );
	}
}

void sigtrap( int sig )
{
	Log( "Got signal: %d", sig );
	exit( 0 );
}

char *translate( char c )
{
	static char buf[4];

	if	( c < 32 || c > 126 )
		sprintf( buf, "%.2X", c );
	else
		sprintf( buf, "'%c'", c );

	return( buf );
}

void readline( void )
{
	char line[256];
	char *token;

	if	( !fgets( line, sizeof( line ) - 1, stdin ) ) {
		Log( "EOF." );
		exit( 0 );
	}

	if	( ( token = strtok( line, "\r\n" ) ) != NULL ) {
		printf( "You sent: %s\r\n", token );
		fflush( stdout );
	}
}

void main( void )
{
	fd_set rfd;

	signal( SIGHUP, sigtrap );

	Log( "Connected." );

	puts( "As you approach the gates, a little gnome jumps out from behind\r" );
	puts( "a rock.  He whines, 'Go back!  The masters are not ready for\r" );
	puts( "you yet!'  The gnome then disappears into the brush.\r" );
	puts( "\r" );

	fflush( stdout );

	for ( ;; ) {
		FD_ZERO( &rfd );
		FD_SET( 0, &rfd );

		switch ( select( 1, &rfd, NULL, NULL, NULL ) ) {
		case -1:
			Log( "Error %d in select.", errno );
			exit( 0 );

		case 0:
			Log( "Time-out?!" );
			break;

		default:
			if	( FD_ISSET( 0, &rfd ) ) {
				Log( "Got input." );
				readline( );
			}
			break;
		}
	}
}
