#include <errno.h>
#include <ioctl.h>
#include <process.h>
#include <signal.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <sys/kernel.h>
#include <sys/name.h>
#include <sys/select.h>
#include <sys/time.h>
#include <wm.h>

#include "common.h"

#define	FALSE	0
#define	TRUE	1

#define	__HERE__	{ Log( "Line %d.", __LINE__ ); }

#define CTRL(x)   ((x) - 64)
#define UNUSED(x) x = x

#define	C_WILL	251
#define	C_WONT	252
#define	C_DO	253
#define	C_DONT	254
#define	C_IAC	255

enum {
	S_TEXT,
	S_IAC,
	S_CODE,
	S_DO,
	S_DONT,
	S_WILL,
	S_WONT
};

static char Line[8192];
static char Msg[256], Reply[256];
static int Pos = 0;
static int Size = 0;
static int State = S_TEXT;
static pid_t Qid;

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

char *translate( char c )
{
	static char buf[4];

	if	( c < 32 || c > 126 )
		sprintf( buf, "%.2X", c );
	else
		sprintf( buf, "'%c'", c );

	return( buf );
}

void check_out( void )
{
	if	( Qid != -1 ) {
		strcpy( Msg, "4" );
		Send( Qid, Msg, Reply, sizeof( Msg ), sizeof( Reply ) );
		Qid = -1;
	}
}

void wait_input( void )
{
	fd_set rfd;

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
				return;
			}
			break;
		}
	}
}

void send_dont( char c )
{
	char buf[3];

	buf[0] = C_IAC;
	buf[1] = C_DONT;
	buf[2] = c;

	write_sock( buf, sizeof( buf ) );
}

void send_wont( char c )
{
	char buf[3];

	buf[0] = C_IAC;
	buf[1] = C_WONT;
	buf[2] = c;

	write_sock( buf, sizeof( buf ) );
}

int getln( char *buf, int len )
{
	int n;

	if	( Pos == Size ) {
		Pos = Size = 0;

		n = read( 0, Line, sizeof( Line ) );

		if	( n < 0 ) {
			if	( errno != EWOULDBLOCK ) {
				Log( "Error %d reading from socket.", errno );
				exit( 0 );
			}

			return( FALSE );
		}

		if	( !n ) {
			Log( "Disconnected!" );
			exit( 0 );
		}

		Size = n;
	}

	n = 0;

	while ( Pos < Size ) {
		switch ( State ) {
		case S_CODE:
		case S_DONT:
		case S_WONT:
			Log( "S_TEXT" );
			State = S_TEXT;
			break;
		case S_DO:
			Log( "S_TEXT" );
			State = S_TEXT;
			send_dont( Line[Pos] );
			break;
		case S_WILL:
			Log( "S_TEXT" );
			State = S_TEXT;
			send_wont( Line[Pos] );
			break;
		case S_IAC:
			if	( Line[Pos] != C_IAC ) {
				if	( n > 0 ) {
					buf[n] = '\0';
					return( TRUE );
				}

				switch ( Line[Pos] ) {
				case C_WILL:
					Log( "S_WILL" );
					State = S_WILL;
					break;
				case C_WONT:
					Log( "S_WONT" );
					State = S_WONT;
					break;
				case C_DO:
					Log( "S_DO" );
					State = S_DO;
					break;
				case C_DONT:
					Log( "S_DONT" );
					State = S_DONT;
					break;
				default:
					Log( "S_CODE" );
					State = S_CODE;
					break;
				}
			}
			else {
				Log( "S_TEXT" );
				State = S_TEXT;
				Pos--;
			}
			break;
		case S_TEXT:
			if	( C_IAC == Line[Pos] ) {
				Log( "S_IAC" );
				State = S_IAC;
				Pos--;
			}
			else if	( n < len )
				buf[n++] = Line[Pos];
			break;
		default:
			Log( "D'Ohh!" );
			Log( "S_TEXT" );
			State = S_TEXT;
			break;
		}

		Pos++;
	}

	if	( S_TEXT == State && n > 0 ) {
		buf[n] = '\0';
		return( TRUE );
	}

	return( FALSE );
}

int readln( char *buf, int len )
{
	static char line[1024];
	char *token;
	int i;

	if	( !getln( line, sizeof( line ) - 1 ) )
		return( FALSE );

	if	( ( token = strtok( line, "\r\n" ) ) == NULL ) {
		Log( "No EOL." );
		return( FALSE );
	}

	for ( i = 0; token[i]; i++ ) {
		if	( token[i] < 32 || token[i] > 126 ) {
			Log( "Removed control character." );
			strcpy( token + i, token + i + 1 );
			i--;
		}
	}

	if	( !token[0] ) {
		Log( "Nothing input." );
		return( FALSE );
	}

	token[len] = '\0';
	strcpy( buf, token );

	Log( "Input text: [%s]", buf );

	return( TRUE );
}

void write_sock( char *buf, int len )
{
	int n;
	int pos = 0;

	while ( pos < len ) {
		n = write( 1, buf + pos, len - pos );

		if	( n < 0 ) {
			if	( errno != EWOULDBLOCK ) {
				Log( "Error %d writing to socket.", errno );
				exit( 0 );
			}
		}
		else if	( n > 0 )
			pos += n;
	}
}

void writeln( char *buf )
{
	write_sock( buf, strlen( buf ) );
}

void writelnf( char *fmt, ... )
{
	va_list ap;
	static char buf[512];

	va_start( ap, fmt );
	vsprintf( buf, fmt, ap );
	va_end( ap );

	writeln( buf );
}

static void sigtrap( int sig )
{
	switch ( sig ) {
	case SIGHUP:
		Log( "Signal caught: SIGHUP" );
		exit( 0 );

	default:
		Log( "Signal caught: %d", sig );
		exit( 0 );
	}
}

pid_t startup( void )
{
	pid_t Qid;
	
	if	( ( Qid = qnx_name_locate( getnid( ), SERVER, 256, NULL ) ) == -1 ) {
		char *argv[2];

		argv[0] = BINDIR "/scepterd";
		argv[1] = NULL;
		
		Qid = qnx_spawn( 0, NULL, getnid(), -1, -1, _SPAWN_NOZOMBIE,
						argv[0], argv, environ, NULL, -1 );
					
		if	( Qid != -1 ) {
			int	i;

			for ( i = 0; i < 5; i++ ) {
				sleep( 2 );
				
				if	( ( Qid = qnx_name_locate( getnid( ), SERVER, 256, NULL ) ) != -1 )
					break;
			}
		}
	}
	
	return Qid;
}

void leave( char *s )
{
	Qid = -1;

	if	( s && *s )
		writelnf( "%s\r\n", s );

	Log( "Leaving." );
	sleep( 5 );

	exit(0);
}

void main( void )
{
	int on = 1;
	int port;

	Log( "Startup." );

	ioctl( 0, FIONBIO, &on );

	if	( ( Qid = startup( ) ) == -1 )
		leave( "Scepter daemon not available\n" );

	atexit( check_out );

	signal( SIGHUP, sigtrap );

	port = 63;
	sprintf( Msg, "0%d", port );
	
	__HERE__
	for ( ;; ) {
		if	( Send( Qid, Msg, Reply, 256, 256 ) == -1 )
			break;

		switch ( *Reply ) {
		case '0':
			writelnf( "%s\r\n", Reply + 1 );
			fflush( stdout );
			strcpy( Msg, "2" );
			break;

		case '1':
			writelnf( "%s", Reply + 1 );

			strcpy( Msg, "1" );

			if	( Reply[1] ) {
				do
					wait_input( );
				while ( !readln( Msg + 1, 255 ) );
			}
			else if	( !readln( Msg + 1, 255 ) )
				Msg[1] = '\0';
			break;

		case '2':
			leave( Reply + 1 );

		case '3':
			leave( "End - aborted\n" );
		}
	}

	leave( "Premature task abort\n" );
}
