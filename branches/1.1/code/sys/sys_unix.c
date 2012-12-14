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

#include "../qcommon/q_shared.h"
#include "../qcommon/qcommon.h"
#include "sys_local.h"

#include <signal.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>
#include <stdio.h>
#include <dirent.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/time.h>
#include <pwd.h>
#include <libgen.h>
#include <fcntl.h>

qboolean stdinIsATTY;

// Used to determine where to store user-specific files
static char homePath[ MAX_OSPATH ] = { 0 };

#if defined SMOKINGUNS && defined DEDICATED
uid_t saved_euid;
#endif

/*
==================
Sys_DefaultHomePath
==================
*/
char *Sys_DefaultHomePath(void)
{
	char *p;

	if( !*homePath )
	{
		if( ( p = getenv( "HOME" ) ) != NULL )
		{
			Q_strncpyz( homePath, p, sizeof( homePath ) );
#ifdef MACOS_X
			Q_strcat( homePath, sizeof( homePath ),
#ifndef SMOKINGUNS
					"/Library/Application Support/Quake3" );
#else
					"/Library/Application Support/" PRODUCT_SHORTNAME );
#endif
#else
#ifndef SMOKINGUNS
			Q_strcat( homePath, sizeof( homePath ), "/.q3a" );
#else
			Q_strcat( homePath, sizeof( homePath ), "/." BASEGAME );
#endif
#endif
		}
	}

	return homePath;
}

#ifndef MACOS_X
/*
================
Sys_TempPath
================
*/
const char *Sys_TempPath( void )
{
	const char *TMPDIR = getenv( "TMPDIR" );

	if( TMPDIR == NULL || TMPDIR[ 0 ] == '\0' )
		return "/tmp";
	else
		return TMPDIR;
}
#endif

/*
=================
Sys_TestSysInstallPath
=================
*/
#ifdef SMOKINGUNS
#define BASEPAK "sg_pak0.pk3"
qboolean Sys_TestSysInstallPath(const char *path)
{
	char *testpath;
	FILE *f;
	testpath = FS_BuildOSPath( path, BASEGAME, BASEPAK );
	f = fopen( testpath, "r" );
	if (f) {
		fclose( f );
		return qtrue;
	} else {
		return qfalse;
	}
}
#endif

/*
==================
Sys_Readlink
==================
*/
#ifdef SMOKINGUNS
int Sys_Readlink( const char *path, char *buf, int bufsiz )
{
	return readlink( path, buf, bufsiz );
}
#endif

/*
==================
Sys_GetSystemInstallPath
==================
*/
#ifdef SMOKINGUNS
const char *Sys_GetSystemInstallPath(const char *path)
{
#ifndef MACOS_X
// MacOSX has already figured this out, so we will just return the path
	const char *p[]= {

// Packagers: change PREFIX here to your favourite location.
// Or change DEFAULT_BASEDIR in Makefile.local
#ifndef PREFIX
#define PREFIX /usr/share/games
#endif
#ifdef DEFAULT_BASEDIR
		XSTRING(DEFAULT_BASEDIR),
#endif
		XSTRING(PREFIX) "/" PRODUCT_SHORTNAME,
		"/usr/local/" PRODUCT_SHORTNAME,
		"/opt/" PRODUCT_SHORTNAME,
		"/opt/games/" PRODUCT_SHORTNAME,
		"/usr/games/" PRODUCT_SHORTNAME,
		"/" PRODUCT_SHORTNAME,
		"/",
		NULL,
	};

	char real_path[MAX_OSPATH];
	const char *sp, *hp;
	char homedir[MAX_OSPATH];
	int i;

	// You can now rely on SG_BASEPATH for the installed game
	if ((sp = getenv("SG_BASEPATH")) != NULL) {
		return va("%s",sp);
	}

	for (i=0; p[i] != NULL; i++) {
		if (Sys_TestSysInstallPath(p[i])) {
			return p[i];
		}
	}

	// Let's also try ~/SmokinGuns/ - just for kicks
	if ((hp = getenv("HOME")) != NULL) {
		Q_strncpyz(homedir, hp, sizeof(homedir));
		Q_strcat(homedir, sizeof(homedir), "/" PRODUCT_SHORTNAME);
		if (Sys_TestSysInstallPath(homedir)) {
			return va("%s",homedir);
		}

		// Let's also try ~/Smokin' Guns/ - just to support default
		// folder set from the Smokin' Guns 1.0 zip file
		Q_strncpyz(homedir, hp, sizeof(homedir));
		Q_strcat(homedir, sizeof(homedir), "/" PRODUCT_NAME);
		if (Sys_TestSysInstallPath(homedir)) {
			return va("%s",homedir);
		}
	}


	// Prototype code for resolving a symbolic link.
	// However, this works only, if the full path is
	// given on the command line. But since the last option
	// takes the full path, it's better to try to resolve
	// the real path first.
	// reading /proc/self/exe on linux or /proc/curproc/file on FreeBSD
	// would work but make it non-portable ... OpenBSD ??
	if (Sys_Readlink(path, real_path, MAX_OSPATH) > 0) {
		return (const char *)Sys_Dirname(real_path);
	}
	if (Sys_TestSysInstallPath(Sys_Dirname((char *)path))) {
		return (const char *)Sys_Dirname((char *)path);
	}
#endif
	return path;
}
#endif

/*
================
Sys_Milliseconds
================
*/
/* base time in seconds, that's our origin
   timeval:tv_sec is an int:
   assuming this wraps every 0x7fffffff - ~68 years since the Epoch (1970) - we're safe till 2038
   using unsigned long data type to work right with Sys_XTimeToSysTime */
unsigned long sys_timeBase = 0;
/* current time in ms, using sys_timeBase as origin
   NOTE: sys_timeBase*1000 + curtime -> ms since the Epoch
     0x7fffffff ms - ~24 days
   although timeval:tv_usec is an int, I'm not sure wether it is actually used as an unsigned int
     (which would affect the wrap period) */
int curtime;
int Sys_Milliseconds (void)
{
	struct timeval tp;

	gettimeofday(&tp, NULL);

	if (!sys_timeBase)
	{
		sys_timeBase = tp.tv_sec;
		return tp.tv_usec/1000;
	}

	curtime = (tp.tv_sec - sys_timeBase)*1000 + tp.tv_usec/1000;

	return curtime;
}

#if !id386
/*
==================
fastftol
==================
*/
long fastftol( float f )
{
	return (long)f;
}

/*
==================
Sys_SnapVector
==================
*/
void Sys_SnapVector( float *v )
{
	v[0] = rint(v[0]);
	v[1] = rint(v[1]);
	v[2] = rint(v[2]);
}
#endif


/*
==================
Sys_RandomBytes
==================
*/
qboolean Sys_RandomBytes( byte *string, int len )
{
	FILE *fp;

	fp = fopen( "/dev/urandom", "r" );
	if( !fp )
		return qfalse;

	if( !fread( string, sizeof( byte ), len, fp ) )
	{
		fclose( fp );
		return qfalse;
	}

	fclose( fp );
	return qtrue;
}

/*
==================
Sys_GetCurrentUser
==================
*/
char *Sys_GetCurrentUser( void )
{
	struct passwd *p;

	if ( (p = getpwuid( getuid() )) == NULL ) {
		return "player";
	}
	return p->pw_name;
}

/*
==================
Sys_GetClipboardData
==================
*/
char *Sys_GetClipboardData(void)
{
	return NULL;
}

#define MEM_THRESHOLD 96*1024*1024

/*
==================
Sys_LowPhysicalMemory

TODO
==================
*/
qboolean Sys_LowPhysicalMemory( void )
{
	return qfalse;
}

/*
==================
Sys_Basename
==================
*/
const char *Sys_Basename( char *path )
{
	return basename( path );
}

/*
==================
Sys_Dirname
==================
*/
const char *Sys_Dirname( char *path )
{
	static char dir[MAX_OSPATH];
	Com_sprintf( dir, sizeof(dir), "%s", path );
	return dirname( dir );
}

/*
==================
Sys_Mkdir
==================
*/
qboolean Sys_Mkdir( const char *path )
{
	int result = mkdir( path, 0750 );

	if( result != 0 )
		return errno == EEXIST;

	return qtrue;
}

/*
==================
Sys_Cwd
==================
*/
char *Sys_Cwd( void )
{
	static char cwd[MAX_OSPATH];

	char *result = getcwd( cwd, sizeof( cwd ) - 1 );
	if( result != cwd )
		return NULL;

	cwd[MAX_OSPATH-1] = 0;

	return cwd;
}

/*
==============================================================

DIRECTORY SCANNING

==============================================================
*/

#define MAX_FOUND_FILES 0x1000

/*
==================
Sys_ListFilteredFiles
==================
*/
void Sys_ListFilteredFiles( const char *basedir, char *subdirs, char *filter, char **list, int *numfiles )
{
	char          search[MAX_OSPATH], newsubdirs[MAX_OSPATH];
	char          filename[MAX_OSPATH];
	DIR           *fdir;
	struct dirent *d;
	struct stat   st;

	if ( *numfiles >= MAX_FOUND_FILES - 1 ) {
		return;
	}

	if (strlen(subdirs)) {
		Com_sprintf( search, sizeof(search), "%s/%s", basedir, subdirs );
	}
	else {
		Com_sprintf( search, sizeof(search), "%s", basedir );
	}

	if ((fdir = opendir(search)) == NULL) {
		return;
	}

	while ((d = readdir(fdir)) != NULL) {
		Com_sprintf(filename, sizeof(filename), "%s/%s", search, d->d_name);
		if (stat(filename, &st) == -1)
			continue;

		if (st.st_mode & S_IFDIR) {
			if (Q_stricmp(d->d_name, ".") && Q_stricmp(d->d_name, "..")) {
				if (strlen(subdirs)) {
					Com_sprintf( newsubdirs, sizeof(newsubdirs), "%s/%s", subdirs, d->d_name);
				}
				else {
					Com_sprintf( newsubdirs, sizeof(newsubdirs), "%s", d->d_name);
				}
				Sys_ListFilteredFiles( basedir, newsubdirs, filter, list, numfiles );
			}
		}
		if ( *numfiles >= MAX_FOUND_FILES - 1 ) {
			break;
		}
		Com_sprintf( filename, sizeof(filename), "%s/%s", subdirs, d->d_name );
		if (!Com_FilterPath( filter, filename, qfalse ))
			continue;
		list[ *numfiles ] = CopyString( filename );
		(*numfiles)++;
	}

	closedir(fdir);
}

/*
==================
Sys_ListFiles
==================
*/
char **Sys_ListFiles( const char *directory, const char *extension, char *filter, int *numfiles, qboolean wantsubs )
{
	struct dirent *d;
	DIR           *fdir;
	qboolean      dironly = wantsubs;
	char          search[MAX_OSPATH];
	int           nfiles;
	char          **listCopy;
	char          *list[MAX_FOUND_FILES];
	int           i;
	struct stat   st;

	int           extLen;

	if (filter) {

		nfiles = 0;
		Sys_ListFilteredFiles( directory, "", filter, list, &nfiles );

		list[ nfiles ] = NULL;
		*numfiles = nfiles;

		if (!nfiles)
			return NULL;

		listCopy = Z_Malloc( ( nfiles + 1 ) * sizeof( *listCopy ) );
		for ( i = 0 ; i < nfiles ; i++ ) {
			listCopy[i] = list[i];
		}
		listCopy[i] = NULL;

		return listCopy;
	}

	if ( !extension)
		extension = "";

	if ( extension[0] == '/' && extension[1] == 0 ) {
		extension = "";
		dironly = qtrue;
	}

	extLen = strlen( extension );

	// search
	nfiles = 0;

	if ((fdir = opendir(directory)) == NULL) {
		*numfiles = 0;
		return NULL;
	}

	while ((d = readdir(fdir)) != NULL) {
		Com_sprintf(search, sizeof(search), "%s/%s", directory, d->d_name);
		if (stat(search, &st) == -1)
			continue;
		if ((dironly && !(st.st_mode & S_IFDIR)) ||
			(!dironly && (st.st_mode & S_IFDIR)))
			continue;

		if (*extension) {
			if ( strlen( d->d_name ) < strlen( extension ) ||
				Q_stricmp(
					d->d_name + strlen( d->d_name ) - strlen( extension ),
					extension ) ) {
				continue; // didn't match
			}
		}

		if ( nfiles == MAX_FOUND_FILES - 1 )
			break;
		list[ nfiles ] = CopyString( d->d_name );
		nfiles++;
	}

	list[ nfiles ] = NULL;

	closedir(fdir);

	// return a copy of the list
	*numfiles = nfiles;

	if ( !nfiles ) {
		return NULL;
	}

	listCopy = Z_Malloc( ( nfiles + 1 ) * sizeof( *listCopy ) );
	for ( i = 0 ; i < nfiles ; i++ ) {
		listCopy[i] = list[i];
	}
	listCopy[i] = NULL;

	return listCopy;
}

/*
==================
Sys_FreeFileList
==================
*/
void Sys_FreeFileList( char **list )
{
	int i;

	if ( !list ) {
		return;
	}

	for ( i = 0 ; list[i] ; i++ ) {
		Z_Free( list[i] );
	}

	Z_Free( list );
}

/*
==================
Sys_Sleep

Block execution for msec or until input is recieved.
==================
*/
void Sys_Sleep( int msec )
{
	if( msec == 0 )
		return;

	if( stdinIsATTY )
	{
		fd_set fdset;

		FD_ZERO(&fdset);
		FD_SET(STDIN_FILENO, &fdset);
		if( msec < 0 )
		{
			select(STDIN_FILENO + 1, &fdset, NULL, NULL, NULL);
		}
		else
		{
			struct timeval timeout;

			timeout.tv_sec = msec/1000;
			timeout.tv_usec = (msec%1000)*1000;
			select(STDIN_FILENO + 1, &fdset, NULL, NULL, &timeout);
		}
	}
	else
	{
		// With nothing to select() on, we can't wait indefinitely
		if( msec < 0 )
			msec = 10;

		usleep( msec * 1000 );
	}
}

/*
==============
Sys_ErrorDialog

Display an error message
==============
*/
void Sys_ErrorDialog( const char *error )
{
	char buffer[ 1024 ];
	unsigned int size;
	int f = -1;
	const char *homepath = Cvar_VariableString( "fs_homepath" );
	const char *gamedir = Cvar_VariableString( "fs_gamedir" );
	const char *fileName = "crashlog.txt";
	char *ospath = FS_BuildOSPath( homepath, gamedir, fileName );

	Sys_Print( va( "%s\n", error ) );

#ifndef DEDICATED
	Sys_Dialog( DT_ERROR, va( "%s. See \"%s\" for details.", error, ospath ), "Error" );
#endif

	// Make sure the write path for the crashlog exists...
	if( FS_CreatePath( ospath ) ) {
		Com_Printf( "ERROR: couldn't create path '%s' for crash log.\n", ospath );
		return;
	}

	// We might be crashing because we maxed out the Quake MAX_FILE_HANDLES,
	// which will come through here, so we don't want to recurse forever by
	// calling FS_FOpenFileWrite()...use the Unix system APIs instead.
	f = open( ospath, O_CREAT | O_TRUNC | O_WRONLY, 0640 );
	if( f == -1 )
	{
		Com_Printf( "ERROR: couldn't open %s\n", fileName );
		return;
	}

	// We're crashing, so we don't care much if write() or close() fails.
	while( ( size = CON_LogRead( buffer, sizeof( buffer ) ) ) > 0 ) {
		if( write( f, buffer, size ) != size ) {
			Com_Printf( "ERROR: couldn't fully write to %s\n", fileName );
			break;
		}
	}

	close( f );
}

#ifndef MACOS_X
/*
==============
Sys_ZenityCommand
==============
*/
static int Sys_ZenityCommand( dialogType_t type, const char *message, const char *title )	
{
	const char *options = "";
	char       command[ 1024 ];

	switch( type )
	{
		default:
		case DT_INFO:      options = "--info"; break;
		case DT_WARNING:   options = "--warning"; break;
		case DT_ERROR:     options = "--error"; break;
		case DT_YES_NO:    options = "--question --ok-label=\"Yes\" --cancel-label=\"No\""; break;
		case DT_OK_CANCEL: options = "--question --ok-label=\"OK\" --cancel-label=\"Cancel\""; break;
	}

	Com_sprintf( command, sizeof( command ), "zenity %s --text=\"%s\" --title=\"%s\"",
		options, message, title );

	return system( command );
}

/*
==============
Sys_KdialogCommand
==============
*/
static int Sys_KdialogCommand( dialogType_t type, const char *message, const char *title )
{
	const char *options = "";
	char       command[ 1024 ];

	switch( type )
	{
		default:
		case DT_INFO:      options = "--msgbox"; break;
		case DT_WARNING:   options = "--sorry"; break;
		case DT_ERROR:     options = "--error"; break;
		case DT_YES_NO:    options = "--warningyesno"; break;
		case DT_OK_CANCEL: options = "--warningcontinuecancel"; break;
	}

	Com_sprintf( command, sizeof( command ), "kdialog %s \"%s\" --title \"%s\"",
		options, message, title );

	return system( command );
}

/*
==============
Sys_XmessageCommand
==============
*/
static int Sys_XmessageCommand( dialogType_t type, const char *message, const char *title )
{
	const char *options = "";
	char       command[ 1024 ];

	switch( type )
	{
		default:           options = "-buttons OK"; break;
		case DT_YES_NO:    options = "-buttons Yes:0,No:1"; break;
		case DT_OK_CANCEL: options = "-buttons OK:0,Cancel:1"; break;
	}

	Com_sprintf( command, sizeof( command ), "xmessage -center %s \"%s\"",
		options, message );

	return system( command );
}

/*
==============
Sys_Dialog

Display a *nix dialog box
==============
*/
dialogResult_t Sys_Dialog( dialogType_t type, const char *message, const char *title )
{
	typedef enum
	{
		NONE = 0,
		ZENITY,
		KDIALOG,
		XMESSAGE,
		NUM_DIALOG_PROGRAMS
	} dialogCommandType_t;
	typedef int (*dialogCommandBuilder_t)( dialogType_t, const char *, const char * );

	const char              *session = getenv( "DESKTOP_SESSION" );
	qboolean                tried[ NUM_DIALOG_PROGRAMS ] = { qfalse };
	dialogCommandBuilder_t  commands[ NUM_DIALOG_PROGRAMS ] = { NULL };
	dialogCommandType_t     preferredCommandType = NONE;

	commands[ ZENITY ] = &Sys_ZenityCommand;
	commands[ KDIALOG ] = &Sys_KdialogCommand;
	commands[ XMESSAGE ] = &Sys_XmessageCommand;

	// This may not be the best way
	if( !Q_stricmp( session, "gnome" ) )
		preferredCommandType = ZENITY;
	else if( !Q_stricmp( session, "kde" ) )
		preferredCommandType = KDIALOG;

	while( 1 )
	{
		int i;
		int exitCode;

		for( i = NONE + 1; i < NUM_DIALOG_PROGRAMS; i++ )
		{
			if( preferredCommandType != NONE && preferredCommandType != i )
				continue;

			if( !tried[ i ] )
			{
				exitCode = commands[ i ]( type, message, title );

				if( exitCode >= 0 )
				{
					switch( type )
					{
						case DT_YES_NO:    return exitCode ? DR_NO : DR_YES;
						case DT_OK_CANCEL: return exitCode ? DR_CANCEL : DR_OK;
						default:           return DR_OK;
					}
				}

				tried[ i ] = qtrue;

				// The preference failed, so start again in order
				if( preferredCommandType != NONE )
				{
					preferredCommandType = NONE;
					break;
				}
			}
		}

		for( i = NONE + 1; i < NUM_DIALOG_PROGRAMS; i++ )
		{
			if( !tried[ i ] )
				continue;
		}

		break;
	}

	Com_DPrintf( S_COLOR_YELLOW "WARNING: failed to show a dialog\n" );
	return DR_OK;
}
#endif

/*
==============
Sys_GLimpSafeInit

Unix specific "safe" GL implementation initialisation
==============
*/
void Sys_GLimpSafeInit( void )
{
	// NOP
}

/*
==============
Sys_GLimpInit

Unix specific GL implementation initialisation
==============
*/
void Sys_GLimpInit( void )
{
	// NOP
}

/*
==============
Sys_PlatformInit

Unix specific initialisation
==============
*/
void Sys_PlatformInit( void )
{
	const char* term = getenv( "TERM" );

	signal( SIGHUP, Sys_SigHandler );
	signal( SIGQUIT, Sys_SigHandler );
	signal( SIGTRAP, Sys_SigHandler );
	signal( SIGIOT, Sys_SigHandler );
	signal( SIGBUS, Sys_SigHandler );

	stdinIsATTY = isatty( STDIN_FILENO ) &&
		!( term && ( !strcmp( term, "raw" ) || !strcmp( term, "dumb" ) ) );
}

/*
==============
Sys_SetEnv

set/unset environment variables (empty value removes it)
==============
*/

void Sys_SetEnv(const char *name, const char *value)
{
	if(value && *value)
		setenv(name, value, 1);
	else
		unsetenv(name);
}

/*
==============
Sys_GetEnv

get environment variables
==============
*/
#ifdef SMOKINGUNS
char *Sys_GetEnv(const char *name)
{
	return getenv(name);
}
#endif

/*
==============
Sys_PID
==============
*/
int Sys_PID( void )
{
	return getpid( );
}

/*
==============
Sys_PIDIsRunning
==============
*/
qboolean Sys_PIDIsRunning( int pid )
{
	return kill( pid, 0 ) == 0;
}

/*
==============
Sys_PlatformPostInit

Unix specific post init
==============
*/
#if defined SMOKINGUNS
void Sys_PlatformPostInit( char *progname )
{
#ifdef DEDICATED
	cvar_t *cv_pid, *net_port;
	FILE *fd;
	char *pid;

	printf("UID %d EUID %d\n", Sys_Getuid(), Sys_Geteuid());

	// Original code for handling the PID file by hika AT bsdmon DOT com

	// Get net_port number
	net_port = Cvar_Get( "net_port", va( "%i", PORT_SERVER ), CVAR_LATCH );

	// Get pid file path
	cv_pid = Cvar_Get ("sv_pidfile", va("/var/run/%s-%s.pid", Sys_Basename(progname), net_port->string) , CVAR_INIT);

	if (cv_pid && cv_pid->string[0]) {
		fd = fopen(cv_pid->string, "w");

		if (fd == NULL) {
			// Try to open PID file in Smokin'Guns home directory if sytem folder is not writable
			Cvar_Set( "sv_pidfile", va("%s/%s-%s.pid", homePath, Sys_Basename(progname), net_port->string) );
			fd = fopen(cv_pid->string, "w");
		}

		if (fd != NULL) {
			// Write pid to a file
			pid = va("%d\n", Sys_Getpid());
			fwrite(pid, sizeof(char), strlen(pid), fd);
			fclose(fd);
		}
		else {
			printf("Cannot open %s for writing : %s\n", cv_pid->string, strerror(errno));
			Cvar_Set( "sv_pidfile", '\0' );
		}
	}
#endif
}

/*
==============
Sys_PlatformExit

Unix specific exit
==============
*/
void Sys_PlatformExit( void )
{
#ifdef DEDICATED
	char pidfile[MAX_OSPATH];

	// single exit point (regular exit or in case of signal fault)
	// includes unlinking of the PID file. Original code for handling
	// the PID file by hika AT bsdmon DOT com
	Cvar_VariableStringBuffer("sv_pidfile", pidfile, sizeof(pidfile));

	if (pidfile[0]) {
		// Try to unlink the pid file
		if (Sys_Unlink(pidfile) != 0)
			printf("Cannot unlink %s : %s\n", pidfile, strerror(errno));
	}
#endif
}

/*
==============
Sys_Getuid

Get user UID
==============
*/
uid_t Sys_Getuid( void )
{
	return getuid();
}

/*
==============
Sys_Geteuid

Get user effective UID
==============
*/
uid_t Sys_Geteuid( void )
{
	return geteuid();
}

/*
==============
Sys_Unlink

Unlink a file
==============
*/
int Sys_Unlink( char *file )
{
	return unlink(file);
}

#ifdef DEDICATED
/*
==============
Sys_Getpid

Get user UID
==============
*/
pid_t Sys_Getpid( void )
{
	return getpid();
}

/*
==============
Sys_Daemonize

Daemonize by forking
==============
*/
void Sys_Daemonize( void )
{
	errno = 0 ; // Reset errno
	switch (fork()) {
		case -1:
			// Fork error: probably a memory or system problem
			printf("Warning: Could not daemonize myself : %s\n", strerror(errno));
			break;
		case 0:
			// Child process: work in background
			if (setsid() == -1) {
				printf("Warning: Could not daemonize myself : %s\n", strerror(errno));
			}
			else {
				close(STDIN_FILENO);
				close(STDOUT_FILENO);
				close(STDERR_FILENO);
				// Tequila: Disable sleeping with a select timeout on STDIN
				stdinIsATTY = qfalse;
			}
			break;
		default:
			// Parent process: exit program as the child will relay the parent job
			exit(0);
			break;
	}
}

/*
==============
Sys_LockMyself

Includes chroot / setuid
Original code by sig11 AT bootblock DOT de
==============
*/
void Sys_LockMyself(const char *qjail, const char *quser) {
	struct passwd* pw = NULL;

	if (geteuid ()) return;

	if (quser != NULL && *quser){
		pw = getpwnam (quser);
	}

	if (qjail != NULL && *qjail){
		if (chroot(qjail) || chdir("/")) {
			Com_Printf("Warning: Unable to chroot to %s\n", qjail);
		} else {
			Com_Printf("Chrooted to %s\n", qjail);
		}
	}

	if (quser != NULL && *quser){
		if (pw == NULL) {
			Com_Printf("Warning: Could not get ID of user %s\n", quser);
		} else {
			if (setgid (pw->pw_gid) || setuid (pw->pw_uid)) {
				Com_Printf("Warning: Could not switch to %s\n", quser);
			} else {
				Com_Printf("Switched to user %s\n", quser);

				// Change the HOME environment variable as it is used
				// in Sys_DefaultHomePath() (see above)
				if (setenv("HOME", pw->pw_dir, 1)) {
					Com_Printf("Warning: Could not change HOME to %s\n", pw->pw_dir);
				} else {
					Com_Printf("Changed HOME to %s\n", pw->pw_dir);
				}
			}
		}
	}
}
#endif
#endif
