/*
===========================================================================
Copyright (C) 1999-2005 Id Software, Inc.

This file is part of Quake III Arena source code.

Quake III Arena source code is free software; you can redistribute it
and/or modify it under the terms of the GNU General Public License as
published by the Free Software Foundation; either version 2 of the License,
or (at your option) any later version.

Quake III Arena source code is distributed in the hope that it will be
useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Quake III Arena source code; if not, write to the Free Software
Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
===========================================================================
*/

#include <signal.h>
#include <stdlib.h>
#include <limits.h>
#include <sys/types.h>
#include <stdarg.h>
#include <stdio.h>
#include <sys/stat.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>

#ifndef DEDICATED
#ifdef USE_LOCAL_HEADERS
#	include "SDL.h"
#	include "SDL_cpuinfo.h"
#else
#	include <SDL.h>
#	include <SDL_cpuinfo.h>
#endif
#endif

#include "sys_local.h"
#include "sys_loadlib.h"

#include "../qcommon/q_shared.h"
#include "../qcommon/qcommon.h"

#ifdef SMOKINGUNS
#include "../qcommon/sdk_shared.h"
#ifdef SDK_DIFF
// Tool to keep trace of your modification in the code, so you, coder, may comment
// your own modifications for our information. Then Smokin'Guns teal will be able
// to keep trace and eventually get your graceful accepted comments in consideration
// Blame Tequila if you don't like this tool :P
  #include "../../build/sdk_diff.h"
#endif
#endif

static char binaryPath[ MAX_OSPATH ] = { 0 };
static char installPath[ MAX_OSPATH ] = { 0 };

/*
=================
Sys_SetBinaryPath
=================
*/
void Sys_SetBinaryPath(const char *path)
{
	Q_strncpyz(binaryPath, path, sizeof(binaryPath));
}

/*
=================
Sys_BinaryPath
=================
*/
char *Sys_BinaryPath(void)
{
	return binaryPath;
}

/*
=================
Sys_SetDefaultInstallPath
=================
*/
void Sys_SetDefaultInstallPath(const char *path)
{
#ifndef SMOKINGUNS
	Q_strncpyz(installPath, path, sizeof(installPath));
#else
	Q_strncpyz(installPath, Sys_GetSystemInstallPath(path), sizeof(installPath));
#endif
}

/*
=================
Sys_DefaultInstallPath
=================
*/
char *Sys_DefaultInstallPath(void)
{
	if (*installPath)
		return installPath;
	else
		return Sys_Cwd();
}

/*
=================
Sys_DefaultAppPath
=================
*/
char *Sys_DefaultAppPath(void)
{
	return Sys_BinaryPath();
}

/*
=================
Sys_In_Restart_f

Restart the input subsystem
=================
*/
void Sys_In_Restart_f( void )
{
	IN_Restart( );
}

/*
=================
Sys_ConsoleInput

Handle new console input
=================
*/
char *Sys_ConsoleInput(void)
{
	return CON_Input( );
}

/*
=================
Sys_Exit

Single exit point (regular exit or in case of error)
=================
*/
void Sys_Exit( int ex )
{
	CON_Shutdown( );

#ifdef SMOKINGUNS
	Sys_PlatformExit();
#endif

#ifndef DEDICATED
	SDL_Quit( );
#endif

#ifdef NDEBUG
	exit( ex );
#else
	// Cause a backtrace on error exits
	assert( ex == 0 );
	exit( ex );
#endif
}

/*
=================
Sys_Quit
=================
*/
void Sys_Quit( void )
{
	CL_Shutdown( );
	Sys_Exit( 0 );
}

/*
=================
Sys_GetProcessorFeatures
=================
*/
cpuFeatures_t Sys_GetProcessorFeatures( void )
{
	cpuFeatures_t features = 0;

#ifndef DEDICATED
	if( SDL_HasRDTSC( ) )    features |= CF_RDTSC;
	if( SDL_HasMMX( ) )      features |= CF_MMX;
	if( SDL_HasMMXExt( ) )   features |= CF_MMX_EXT;
	if( SDL_Has3DNow( ) )    features |= CF_3DNOW;
	if( SDL_Has3DNowExt( ) ) features |= CF_3DNOW_EXT;
	if( SDL_HasSSE( ) )      features |= CF_SSE;
	if( SDL_HasSSE2( ) )     features |= CF_SSE2;
	if( SDL_HasAltiVec( ) )  features |= CF_ALTIVEC;
#endif

	return features;
}

/*
=================
Sys_Init
=================
*/
void Sys_Init(void)
{
	Cmd_AddCommand( "in_restart", Sys_In_Restart_f );
	Cvar_Set( "arch", OS_STRING " " ARCH_STRING );
	Cvar_Set( "username", Sys_GetCurrentUser( ) );
}

/*
=================
Sys_AnsiColorPrint

Transform Q3 colour codes to ANSI escape sequences
=================
*/
void Sys_AnsiColorPrint( const char *msg )
{
	static char buffer[ MAXPRINTMSG ];
	int         length = 0;
	static int  q3ToAnsi[ NUM_COLORS ] =
	{
		30, // COLOR_BLACK
		31, // COLOR_RED
		32, // COLOR_GREEN
		33, // COLOR_YELLOW
		34, // COLOR_BLUE
		36, // COLOR_CYAN
		35, // COLOR_MAGENTA
		0   // COLOR_WHITE
	};

	while( *msg )
	{
		if( Q_IsColorString( msg ) || *msg == '\n' )
		{
			// First empty the buffer
			if( length > 0 )
			{
				buffer[ length ] = '\0';
				fputs( buffer, stderr );
				length = 0;
			}

			if( *msg == '\n' )
			{
				// Issue a reset and then the newline
				fputs( "\033[0m\n", stderr );
				msg++;
			}
			else
			{
				// Print the color code
				Com_sprintf( buffer, sizeof( buffer ), "\033[%dm",
						q3ToAnsi[ ColorIndex( *( msg + 1 ) ) ] );
				fputs( buffer, stderr );
				msg += 2;
			}
		}
		else
		{
			if( length >= MAXPRINTMSG - 1 )
				break;

			buffer[ length ] = *msg;
			length++;
			msg++;
		}
	}

	// Empty anything still left in the buffer
	if( length > 0 )
	{
		buffer[ length ] = '\0';
		fputs( buffer, stderr );
	}
}

/*
=================
Sys_Print
=================
*/
void Sys_Print( const char *msg )
{
	CON_LogWrite( msg );
	CON_Print( msg );
}

/*
=================
Sys_Error
=================
*/
void Sys_Error( const char *error, ... )
{
	va_list argptr;
	char    string[1024];

	CL_Shutdown ();

	va_start (argptr,error);
	Q_vsnprintf (string, sizeof(string), error, argptr);
	va_end (argptr);

	Sys_ErrorDialog( string );

	Sys_Exit( 1 );
}

/*
=================
Sys_Warn
=================
*/
void Sys_Warn( char *warning, ... )
{
	va_list argptr;
	char    string[1024];

	va_start (argptr,warning);
	Q_vsnprintf (string, sizeof(string), warning, argptr);
	va_end (argptr);

	CON_Print( va( "Warning: %s", string ) );
}

/*
============
Sys_FileTime

returns -1 if not present
============
*/
int Sys_FileTime( char *path )
{
	struct stat buf;

	if (stat (path,&buf) == -1)
		return -1;

	return buf.st_mtime;
}

/*
=================
Sys_UnloadDll
=================
*/
void Sys_UnloadDll( void *dllHandle )
{
	if( !dllHandle )
	{
		Com_Printf("Sys_UnloadDll(NULL)\n");
		return;
	}

	Sys_UnloadLibrary(dllHandle);
}

/*
=================
Sys_TryLibraryLoad
=================
*/
static void* Sys_TryLibraryLoad(const char* base, const char* gamedir, const char* fname, char* fqpath )
{
	void* libHandle;
	char* fn;

	*fqpath = 0;

	fn = FS_BuildOSPath( base, gamedir, fname );
	Com_Printf( "Sys_LoadDll(%s)... \n", fn );

	libHandle = Sys_LoadLibrary(fn);

	if(!libHandle) {
		Com_Printf( "Sys_LoadDll(%s) failed:\n\"%s\"\n", fn, Sys_LibraryError() );
		return NULL;
	}

	Com_Printf ( "Sys_LoadDll(%s): succeeded ...\n", fn );
	Q_strncpyz ( fqpath , fn , MAX_QPATH ) ;

	return libHandle;
}

/*
=================
Sys_LoadDll

Used to load a development dll instead of a virtual machine
#1 look in fs_homepath
#2 look in fs_basepath
=================
*/
void *Sys_LoadDll( const char *name, char *fqpath ,
	intptr_t (**entryPoint)(int, ...),
	intptr_t (*systemcalls)(intptr_t, ...) )
{
	void  *libHandle;
	void  (*dllEntry)( intptr_t (*syscallptr)(intptr_t, ...) );
	char  fname[MAX_OSPATH];
	char  *basepath;
	char  *homepath;
	char  *gamedir;

	assert( name );

	Q_snprintf (fname, sizeof(fname), "%s" ARCH_STRING DLL_EXT, name);

	// TODO: use fs_searchpaths from files.c
	basepath = Cvar_VariableString( "fs_basepath" );
	homepath = Cvar_VariableString( "fs_homepath" );
	gamedir = Cvar_VariableString( "fs_game" );

	libHandle = Sys_TryLibraryLoad(homepath, gamedir, fname, fqpath);

	if(!libHandle && basepath)
		libHandle = Sys_TryLibraryLoad(basepath, gamedir, fname, fqpath);

	if(!libHandle) {
		Com_Printf ( "Sys_LoadDll(%s) failed to load library\n", name );
		return NULL;
	}

	dllEntry = Sys_LoadFunction( libHandle, "dllEntry" );
	*entryPoint = Sys_LoadFunction( libHandle, "vmMain" );

	if ( !*entryPoint || !dllEntry )
	{
		Com_Printf ( "Sys_LoadDll(%s) failed to find vmMain function:\n\"%s\" !\n", name, Sys_LibraryError( ) );
		Sys_UnloadLibrary(libHandle);

		return NULL;
	}

	Com_Printf ( "Sys_LoadDll(%s) found vmMain function at %p\n", name, *entryPoint );
	dllEntry( systemcalls );

	return libHandle;
}

/*
=================
Sys_ParseArgs
=================
*/
void Sys_ParseArgs( int argc, char **argv )
{
	if( argc == 2 )
	{
		if( !strcmp( argv[1], "--version" ) ||
				!strcmp( argv[1], "-v" ) )
		{
			const char* date = __DATE__;
#ifdef SMOKINGUNS
			const char* time = __TIME__;
			fprintf( stdout, "%s\n", argv[0] );
#ifdef DEDICATED
			fprintf( stdout, Q3_VERSION " dedicated server (%s, %s)\n", date, time );
#else
			fprintf( stdout, Q3_VERSION " client (%s, %s)\n", date, time );
#endif
			fprintf( stdout, "Release: " XSTRING(SG_RELEASE) "\n" );
			fprintf( stdout, "Flavour: " OS_STRING " " ARCH_STRING "\n\n" );
			Sys_BinaryEngineComment();
#else
#ifdef DEDICATED
			fprintf( stdout, Q3_VERSION " dedicated server (%s)\n", date );
#else
			fprintf( stdout, Q3_VERSION " client (%s)\n", date );
#endif
#endif
			Sys_Exit(0);
		}
	}
}

#ifndef DEFAULT_BASEDIR
#	ifdef MACOS_X
#		define DEFAULT_BASEDIR Sys_StripAppBundle(Sys_BinaryPath())
#	else
#		define DEFAULT_BASEDIR Sys_BinaryPath()
#	endif
#endif

/*
=================
Sys_SigHandler
=================
*/
void Sys_SigHandler( int signal )
{
	static qboolean signalcaught = qfalse;

	if( signalcaught )
	{
		fprintf( stderr, "DOUBLE SIGNAL FAULT: Received signal %d, exiting...\n",
			signal );
	}
	else
	{
		signalcaught = qtrue;
		fprintf( stderr, "Received signal %d, exiting...\n", signal );
#ifndef DEDICATED
		CL_Shutdown();
#endif
		SV_Shutdown( "Signal caught" );
	}

	Sys_Exit( 0 ); // Exit with 0 to avoid recursive signals
}

/*
==============
Sys_BinaryEngineComment
==============
*/
#ifdef SMOKINGUNS
void Sys_BinaryEngineComment( void ) {
	const char *version = Q3_VERSION;
	const char *platform = PLATFORM_STRING;
	const char *date = __DATE__;
	const char *time = __TIME__;
	const char *comment = SDK_COMMENT;
	const char *contact = SDK_CONTACT;
	char *sum;
	char string[MAX_STRING_CHARS] = "";
	Com_sprintf(string,sizeof(string),"%s, %s %s, %s", version, date, time, platform);
	if (strlen(comment))
		Q_strcat(string,sizeof(string),va("\ncomment: %s",comment));
	if (strlen(contact))
		Q_strcat(string,sizeof(string),va("\ncontact: %s",contact));
	fprintf( stdout, "engine: %s\n", string);
#ifndef SDK_DIFF
	sum = Com_MD5Text(string,strlen(string),NULL,0);
#else
	sum = Com_MD5TextArray(sdk_diff,sdk_diff_size,string,strlen(string));
#endif
	fprintf( stdout, "md5sum: %s\n", sum);
	// Avoid segfault if engine is called with --version argument
	if (!com_version)
		return;
	Cvar_Set ("cl_md5", va("%s",sum));
#ifndef SDK_DIFF
	Cvar_Set("sdk_engine_comment",va("\\version\\%s\\platform\\%s\\date\\%s %s\\md5\\%s\\comment\\%s\\contact\\%s",
		version, platform, date, time, sum, comment, contact));
#else
	Cvar_Set("sdk_engine_comment",va("\\version\\%s\\platform\\%s\\date\\%s %s\\md5\\%s\\diff\\%i\\comment\\%s\\contact\\%s",
		version, platform, date, time, sum, sdk_diff_format, comment, contact));
#endif
}
#endif

/*
=================
main
=================
*/
int main( int argc, char **argv )
{
	int   i;
	char  commandLine[ MAX_STRING_CHARS ] = { 0 };

#if defined SMOKINGUNS && defined DEDICATED
	char *cv_name, *cv_value;
	char *quser = NULL, *qjail = NULL;
	qboolean qdaemon = qfalse;
#endif

#ifndef DEDICATED
	// SDL version check

	// Compile time
#	if !SDL_VERSION_ATLEAST(MINSDL_MAJOR,MINSDL_MINOR,MINSDL_PATCH)
#		error A more recent version of SDL is required
#	endif

	// Run time
	const SDL_version *ver = SDL_Linked_Version( );

#define MINSDL_VERSION \
	XSTRING(MINSDL_MAJOR) "." \
	XSTRING(MINSDL_MINOR) "." \
	XSTRING(MINSDL_PATCH)

	if( SDL_VERSIONNUM( ver->major, ver->minor, ver->patch ) <
			SDL_VERSIONNUM( MINSDL_MAJOR, MINSDL_MINOR, MINSDL_PATCH ) )
	{
		Sys_Print( "SDL version " MINSDL_VERSION " or greater required\n" );
		Sys_Exit( 1 );
	}
#endif

	Sys_PlatformInit( );

	// Set the initial time base
	Sys_Milliseconds( );

	Sys_ParseArgs( argc, argv );
	Sys_SetBinaryPath( Sys_Dirname( argv[ 0 ] ) );
	Sys_SetDefaultInstallPath( DEFAULT_BASEDIR );

	// Concatenate the command line for passing to Com_Init
	for( i = 1; i < argc; i++ )
	{
		const qboolean containsSpaces = strchr(argv[i], ' ') != NULL;
		if (containsSpaces)
			Q_strcat( commandLine, sizeof( commandLine ), "\"" );

		Q_strcat( commandLine, sizeof( commandLine ), argv[ i ] );

		if (containsSpaces)
			Q_strcat( commandLine, sizeof( commandLine ), "\"" );

		Q_strcat( commandLine, sizeof( commandLine ), " " );
#if defined SMOKINGUNS && defined DEDICATED
		// Original code for handling the special cvars by hika AT bsdmon DOT com
		// This is where we can get some special cvars like
		// sv_chroot, sv_user
		// - sv_chroot is the given path for chrooting
		// - sv_user is the system user for setuid and dropping root privileges

		// It seems that we cannot init cvars BEFORE Com_Init() !
		// It will trigger a segmentation fault !
		// Either way, they will be initialized in Com_Init() as startup variables
		if (!strcmp(argv[i], "+set") && i+2 < argc) {
			cv_name = argv[i+1];
			cv_value = argv[i+2];
			// We have to be sure that the values are "cvar validated"
			if (!strcmp(cv_name, "sv_chroot"))
				qjail = cv_value;
			else if (!strcmp(cv_name, "sv_user"))
				quser = cv_value;
			else if (!strcmp(cv_name, "sv_daemon") && atoi(cv_value))
				qdaemon = qtrue;
		}
	}
	Sys_LockMyself(qjail, quser);

	// go back to real user for config loads
	//saved_euid = Sys_geteuid();
	//seteuid(Sys_getuid());

	if (qdaemon) {
		// Code to daemonize the dedicated server
		// That is particularly useful if we want to put the server
		// on background
		Sys_Daemonize();
	}
#else
	}
#endif

	Com_Init( commandLine );
	NET_Init( );

	CON_Init( );

	signal( SIGILL, Sys_SigHandler );
	signal( SIGFPE, Sys_SigHandler );
	signal( SIGSEGV, Sys_SigHandler );
	signal( SIGTERM, Sys_SigHandler );
	signal( SIGINT, Sys_SigHandler );

#ifdef SMOKINGUNS
	Sys_PlatformPostInit(argv[0]);

	// Create a ROM cvar to let the mod know the Smokin' Guns standalone engine is in use.
	Cvar_Get("sa_engine_inuse", "1", CVAR_ROM);
#endif

	while( 1 )
	{
		IN_Frame( );
		Com_Frame( );
	}

	return 0;
}

