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
#ifndef SMOKINGUNS
			Q_strcat( homePath, sizeof( homePath ), "/Library/Application Support/Quake3" );
#else
			Q_strcat( homePath, sizeof( homePath ), "/Library/Application Support/SmokinGuns" );
#endif
#else
#ifndef SMOKINGUNS
			Q_strcat( homePath, sizeof( homePath ), "/.q3a" );
#else
			Q_strcat( homePath, sizeof( homePath ), "/.smokinguns" );
#endif
#endif
			if( mkdir( homePath, 0777 ) )
			{
				if( errno != EEXIST )
				{
					Sys_Error( "Unable to create directory \"%s\", error is %s(%d)\n",
							homePath, strerror( errno ), errno );
				}
			}
		}
	}

	return homePath;
}

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
		XSTRING(PREFIX) "/SmokinGuns",
		"/usr/local/SmokinGuns",
		"/opt/SmokinGuns",
		"/opt/games/SmokinGuns",
		"/usr/games/SmokinGuns",
		"/SmokinGuns",
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
		Q_strcat(homedir, sizeof(homedir), "/SmokinGuns");
		if (Sys_TestSysInstallPath(homedir)) {
			return va("%s",homedir);
		}

		// Let's also try ~/Smokin' Guns/ - just to support default
		// folder set from the Smokin' Guns 1.0 zip file
		Q_strncpyz(homedir, hp, sizeof(homedir));
		Q_strcat(homedir, sizeof(homedir), "/Smokin' Guns");
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
void Sys_Mkdir( const char *path )
{
	mkdir( path, 0777 );
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

#ifdef MACOS_X
/*
=================
Sys_StripAppBundle

Discovers if passed dir is suffixed with the directory structure of a Mac OS X
.app bundle. If it is, the .app directory structure is stripped off the end and
the result is returned. If not, dir is returned untouched.
=================
*/
char *Sys_StripAppBundle( char *dir )
{
	static char cwd[MAX_OSPATH];

	Q_strncpyz(cwd, dir, sizeof(cwd));
	if(strcmp(Sys_Basename(cwd), "MacOS"))
		return dir;
	Q_strncpyz(cwd, Sys_Dirname(cwd), sizeof(cwd));
	if(strcmp(Sys_Basename(cwd), "Contents"))
		return dir;
	Q_strncpyz(cwd, Sys_Dirname(cwd), sizeof(cwd));
	if(!strstr(Sys_Basename(cwd), ".app"))
		return dir;
	Q_strncpyz(cwd, Sys_Dirname(cwd), sizeof(cwd));
	return cwd;
}
#endif // MACOS_X


/*
==================
Sys_Sleep

Block execution for msec or until input is recieved.
==================
*/
void Sys_Sleep( int msec )
{
	fd_set fdset;

	if( msec == 0 )
		return;

	FD_ZERO(&fdset);
	FD_SET(fileno(stdin), &fdset);
	if( msec < 0 )
	{
		select((fileno(stdin) + 1), &fdset, NULL, NULL, NULL);
	}
	else
	{
		struct timeval timeout;

		timeout.tv_sec = msec/1000;
		timeout.tv_usec = (msec%1000)*1000;
		select((fileno(stdin) + 1), &fdset, NULL, NULL, &timeout);
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
	fileHandle_t f;
	const char *fileName = "crashlog.txt";

	Sys_Print( va( "%s\n", error ) );

	// Write console log to file
	f = FS_FOpenFileWrite( fileName );
	if( !f )
	{
		Com_Printf( "ERROR: couldn't open %s\n", fileName );
		return;
	}

	while( ( size = CON_LogRead( buffer, sizeof( buffer ) ) ) > 0 )
		FS_Write( buffer, size, f );

	FS_FCloseFile( f );
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
	signal( SIGHUP, Sys_SigHandler );
	signal( SIGQUIT, Sys_SigHandler );
	signal( SIGTRAP, Sys_SigHandler );
	signal( SIGIOT, Sys_SigHandler );
	signal( SIGBUS, Sys_SigHandler );
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
	cvar_t *cv_pid;
	FILE *fd;
	char *pid;

	printf("UID %d EUID %d\n", Sys_Getuid(), Sys_Geteuid());

	// Original code for handling the PID file by hika AT bsdmon DOT com

	// Get pid file path
	cv_pid = Cvar_Get ("sv_pidfile", va("/var/run/%s.pid", Sys_Basename(progname)) , CVAR_INIT);

	if (cv_pid && cv_pid->string[0]) {
		fd = fopen(cv_pid->string, "w");

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
