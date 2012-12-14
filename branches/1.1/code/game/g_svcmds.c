/*
===========================================================================
Copyright (C) 1999-2005 Id Software, Inc.
Copyright (C) 2000-2003 Iron Claw Interactive
Copyright (C) 2005-2010 Smokin' Guns

This file is part of Smokin' Guns.

Smokin' Guns is free software; you can redistribute it
and/or modify it under the terms of the GNU General Public License as
published by the Free Software Foundation; either version 2 of the License,
or (at your option) any later version.

Smokin' Guns is distributed in the hope that it will be
useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Smokin' Guns; if not, write to the Free Software
Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
===========================================================================
*/
//

// this file holds commands that can be executed by the server console, but not remote clients

#include "g_local.h"


/*
==============================================================================

PACKET FILTERING


You can add or remove addresses from the filter list with:

addip <ip>
removeip <ip>

The ip address is specified in dot format, and you can use '*' to match any value
so you can specify an entire class C network with "addip 192.246.40.*"

Removeip will only remove an address specified exactly the same way.  You cannot addip a subnet, then removeip a single host.

listip
Prints the current list of filters.

g_filterban <0 or 1>

If 1 (the default), then ip addresses matching the current list will be prohibited from entering the game.  This is the default setting.

If 0, then only addresses matching the list will be allowed.  This lets you easily set up a private game, or a game that only allows players from your local network.

TTimo NOTE: for persistence, bans are stored in g_banIPs cvar MAX_CVAR_VALUE_STRING
The size of the cvar string buffer is limiting the banning to around 20 masks
this could be improved by putting some g_banIPs2 g_banIps3 etc. maybe
still, you should rely on PB for banning instead

==============================================================================
*/

typedef struct ipFilter_s
{
	unsigned	mask;
	unsigned	compare;
} ipFilter_t;

#define	MAX_IPFILTERS	1024

static ipFilter_t	ipFilters[MAX_IPFILTERS];
static int			numIPFilters;

#ifdef SMOKINGUNS
// Just to be disabled by Svcmd_BigText_f() to avoid unwanted verbose message
static qboolean verbose = qtrue ;
#endif

/*
=================
StringToFilter
=================
*/
static qboolean StringToFilter (char *s, ipFilter_t *f)
{
	char	num[128];
	int		i, j;
	byte	b[4];
	byte	m[4];

	for (i=0 ; i<4 ; i++)
	{
		b[i] = 0;
		m[i] = 0;
	}

	for (i=0 ; i<4 ; i++)
	{
		if (*s < '0' || *s > '9')
		{
			if (*s == '*') // 'match any'
			{
				// b[i] and m[i] to 0
				s++;
				if (!*s)
					break;
				s++;
				continue;
			}
			G_Printf( "Bad filter address: %s\n", s );
			return qfalse;
		}

		j = 0;
		while (*s >= '0' && *s <= '9')
		{
			num[j++] = *s++;
		}
		num[j] = 0;
		b[i] = atoi(num);
		m[i] = 255;

		if (!*s)
			break;
		s++;
	}

	f->mask = *(unsigned *)m;
	f->compare = *(unsigned *)b;

	return qtrue;
}

/*
=================
UpdateIPBans
=================
*/
static void UpdateIPBans (void)
{
	byte	b[4];
	byte	m[4];
	int		i,j;
	char	iplist_final[MAX_CVAR_VALUE_STRING];
	char	ip[64];

	*iplist_final = 0;
	for (i = 0 ; i < numIPFilters ; i++)
	{
		if (ipFilters[i].compare == 0xffffffff)
			continue;

		*(unsigned *)b = ipFilters[i].compare;
		*(unsigned *)m = ipFilters[i].mask;
		*ip = 0;
		for (j = 0 ; j < 4 ; j++)
		{
			if (m[j]!=255)
				Q_strcat(ip, sizeof(ip), "*");
			else
				Q_strcat(ip, sizeof(ip), va("%i", b[j]));
			Q_strcat(ip, sizeof(ip), (j<3) ? "." : " ");
		}
		if (strlen(iplist_final)+strlen(ip) < MAX_CVAR_VALUE_STRING)
		{
			Q_strcat( iplist_final, sizeof(iplist_final), ip);
		}
		else
		{
			Com_Printf("g_banIPs overflowed at MAX_CVAR_VALUE_STRING\n");
			break;
		}
	}

	trap_Cvar_Set( "g_banIPs", iplist_final );
}

/*
=================
G_FilterPacket
=================
*/
qboolean G_FilterPacket (char *from)
{
	int		i;
	unsigned	in;
	byte m[4];
	char *p;

	i = 0;
	p = from;
	while (*p && i < 4) {
		m[i] = 0;
		while (*p >= '0' && *p <= '9') {
			m[i] = m[i]*10 + (*p - '0');
			p++;
		}
		if (!*p || *p == ':')
			break;
		i++, p++;
	}

	in = *(unsigned *)m;

	for (i=0 ; i<numIPFilters ; i++)
		if ( (in & ipFilters[i].mask) == ipFilters[i].compare)
			return g_filterBan.integer != 0;

	return g_filterBan.integer == 0;
}

/*
=================
AddIP
=================
*/
static void AddIP( char *str )
{
	int		i;

	for (i = 0 ; i < numIPFilters ; i++)
		if (ipFilters[i].compare == 0xffffffff)
			break;		// free spot
	if (i == numIPFilters)
	{
		if (numIPFilters == MAX_IPFILTERS)
		{
			G_Printf ("IP filter list is full\n");
			return;
		}
		numIPFilters++;
	}

	if (!StringToFilter (str, &ipFilters[i]))
		ipFilters[i].compare = 0xffffffffu;

	UpdateIPBans();
}

/*
=================
G_ProcessIPBans
=================
*/
void G_ProcessIPBans(void)
{
	char *s, *t;
	char		str[MAX_CVAR_VALUE_STRING];

	Q_strncpyz( str, g_banIPs.string, sizeof(str) );

	for (t = s = g_banIPs.string; *t; /* */ ) {
		s = strchr(s, ' ');
		if (!s)
			break;
		while (*s == ' ')
			*s++ = 0;
		if (*t)
			AddIP( t );
		t = s;
	}
}


/*
=================
Svcmd_AddIP_f
=================
*/
void Svcmd_AddIP_f (void)
{
	char		str[MAX_TOKEN_CHARS];

	if ( trap_Argc() < 2 ) {
		G_Printf("Usage:  addip <ip-mask>\n");
		return;
	}

	trap_Argv( 1, str, sizeof( str ) );

	AddIP( str );

}

/*
=================
Svcmd_RemoveIP_f
=================
*/
void Svcmd_RemoveIP_f (void)
{
	ipFilter_t	f;
	int			i;
	char		str[MAX_TOKEN_CHARS];

	if ( trap_Argc() < 2 ) {
		G_Printf("Usage:  sv removeip <ip-mask>\n");
		return;
	}

	trap_Argv( 1, str, sizeof( str ) );

	if (!StringToFilter (str, &f))
		return;

	for (i=0 ; i<numIPFilters ; i++) {
		if (ipFilters[i].mask == f.mask	&&
			ipFilters[i].compare == f.compare) {
			ipFilters[i].compare = 0xffffffffu;
			G_Printf ("Removed.\n");

			UpdateIPBans();
			return;
		}
	}

	G_Printf ( "Didn't find %s.\n", str );
}

/*
===================
Svcmd_EntityList_f
===================
*/
void	Svcmd_EntityList_f (void) {
	int			e;
	gentity_t		*check;

	check = g_entities+1;
	for (e = 1; e < level.num_entities ; e++, check++) {
		if ( !check->inuse ) {
			continue;
		}
		G_Printf("%3i:", e);
		switch ( check->s.eType ) {
		case ET_GENERAL:
			G_Printf("ET_GENERAL          ");
			break;
		case ET_PLAYER:
			G_Printf("ET_PLAYER           ");
			break;
		case ET_ITEM:
			G_Printf("ET_ITEM             ");
			break;
		case ET_MISSILE:
			G_Printf("ET_MISSILE          ");
			break;
		case ET_MOVER:
			G_Printf("ET_MOVER            ");
			break;
		case ET_BEAM:
			G_Printf("ET_BEAM             ");
			break;
		case ET_PORTAL:
			G_Printf("ET_PORTAL           ");
			break;
		case ET_SPEAKER:
			G_Printf("ET_SPEAKER          ");
			break;
		case ET_PUSH_TRIGGER:
			G_Printf("ET_PUSH_TRIGGER     ");
			break;
		case ET_TELEPORT_TRIGGER:
			G_Printf("ET_TELEPORT_TRIGGER ");
			break;
		case ET_INVISIBLE:
			G_Printf("ET_INVISIBLE        ");
			break;
#ifndef SMOKINGUNS
		case ET_GRAPPLE:
			G_Printf("ET_GRAPPLE          ");
			break;
#else
		case ET_FLY:
			G_Printf("ET_FLY              ");
			break;
		case ET_BREAKABLE:
			G_Printf("ET_BREAKABLE        ");
			break;
		case ET_INTERMISSION:
			G_Printf("ET_INTERMISSION     ");
			break;
		case ET_FLARE:
			G_Printf("ET_FLARE            ");
			break;
		case ET_SMOKE:
			G_Printf("ET_SMOKE            ");
			break;
		case ET_TURRET:
			G_Printf("ET_TURRET           ");
			break;
		case ET_ESCAPE:
			G_Printf("ET_ESCAPE           ");
			break;
#endif
		default:
			G_Printf("%3i                 ", check->s.eType);
			break;
		}

		if ( check->classname ) {
			G_Printf("%s", check->classname);
		}
		G_Printf("\n");
	}
}

gclient_t	*ClientForString( const char *s ) {
	gclient_t	*cl;
	int			i;
	int			idnum;

	// numeric values are just slot numbers
	if ( s[0] >= '0' && s[0] <= '9' ) {
		idnum = atoi( s );
		if ( idnum < 0 || idnum >= level.maxclients ) {
			Com_Printf( "Bad client slot: %i\n", idnum );
			return NULL;
		}

		cl = &level.clients[idnum];
		if ( cl->pers.connected == CON_DISCONNECTED ) {
			G_Printf( "Client %i is not connected\n", idnum );
			return NULL;
		}
		return cl;
	}

	// check for a name match
	for ( i=0 ; i < level.maxclients ; i++ ) {
		cl = &level.clients[i];
		if ( cl->pers.connected == CON_DISCONNECTED ) {
			continue;
		}
#ifndef SMOKINGUNS
		if ( !Q_stricmp( cl->pers.netname, s ) ) {
#else
		// Tequila: netname should match an unique name, so name can't be stolen
		if ( !strcmp( cl->pers.netname, s ) ) {
#endif
			return cl;
		}
	}

#ifdef SMOKINGUNS
	if (verbose)
#endif
		G_Printf( "User %s is not on the server\n", s );

	return NULL;
}

/*
===================
Svcmd_ForceTeam_f

forceteam <player> <team>
===================
*/
void	Svcmd_ForceTeam_f( void ) {
	gclient_t	*cl;
	char		str[MAX_TOKEN_CHARS];

	// find the player
	trap_Argv( 1, str, sizeof( str ) );
	cl = ClientForString( str );
	if ( !cl ) {
		return;
	}

	// set the team
	trap_Argv( 2, str, sizeof( str ) );
	SetTeam( &g_entities[cl - level.clients], str );
}

char	*ConcatArgs( int start );

#ifdef SMOKINGUNS
void Svcmd_KickBots_f(void){
	int i;
	gentity_t *ent;

	for(i=0;i<level.maxclients;i++){
		ent = &g_entities[i];

		if(ent->r.svFlags & SVF_BOT){
			trap_SendConsoleCommand( EXEC_INSERT, va("clientkick %i\n", i) );
		}
	}
}

/*
=================
CMD_WEAPON_INFO

Display or change weapon properties.
=================
*/
#define CMD_WEAPON_INFO( field )													   \
do {																				   \
	char		arg[MAX_TOKEN_CHARS];												   \
	int			argc = trap_Argc();													   \
	int			weapon;																   \
	float		oldVal, newVal;														   \
	const char	*name;																   \
																					   \
	if ( argc > 3 )																	   \
		goto usage_##field;															   \
																					   \
	if ( argc == 1 ) {  /* list value for all weapons */							   \
		for ( weapon = 1; weapon < WP_NUM_WEAPONS; weapon++) {						   \
			name = bg_weaponlist[weapon].name;										   \
			oldVal = bg_weaponlist[weapon].field;									   \
			if ( weapon == 2 || weapon == 5 || weapon == 8 || weapon == 11 )		   \
				G_Printf("\n");														   \
			G_Printf( "%2d: %-22s " #field " = %.1f\n", weapon, name, oldVal );		   \
		}																			   \
	}																				   \
	else if ( argc >= 2 ) {															   \
		trap_Argv( 1, arg, sizeof( arg ) );											   \
		weapon = atoi( arg );														   \
		if ( weapon < 1 || weapon >= WP_NUM_WEAPONS )								   \
			goto usage_##field;														   \
																					   \
		name = bg_weaponlist[weapon].name;											   \
		oldVal = bg_weaponlist[weapon].field;										   \
																					   \
		if ( argc == 2 ) {  /* show value for this weapon */						   \
			G_Printf( "%s " #field " is %.1f\n", name, oldVal );					   \
		}																			   \
		else if ( argc == 3 ) {  /* set value for this weapon */					   \
			char buf[256];															   \
																					   \
			trap_Argv( 2, arg, sizeof( arg ) );										   \
			newVal = atof( arg );													   \
			bg_weaponlist[weapon].field = newVal;									   \
			Com_sprintf( buf, sizeof( buf ), "%s " #field " changed from %.1f to %.1f\n",\
															    name, oldVal, newVal );\
			trap_SendServerCommand( -1, va( "print \"server: %s\"", buf ) );		   \
			G_UpdateWeaponConfigString();											   \
		}																			   \
	}																				   \
																					   \
	break;																			   \
																					   \
usage_##field:																		   \
	G_Printf(#field " [weapon (1-13) [newValue]]\n");								   \
																					   \
} while (0)

/*
===================
Svcmd_BigText_f
Tequila: BigText command suggested by villa
[bigtext|cp] [-1|clientNumber|playername] <message>
===================
*/
void Svcmd_BigText_f(void) {
	gclient_t	*cl;
	char		str[MAX_TOKEN_CHARS];

	trap_Argv( 1, str, sizeof( str ) );
	verbose = qfalse ;
	cl = ClientForString( str );
	verbose = qtrue ;

	if ( cl ) {
		trap_SendServerCommand( cl->ps.clientNum, va("cp \"%s\"", ConcatArgs(2) ) );
	} else	if ( Q_stricmp ("-1", str) == 0 )
		trap_SendServerCommand( -1, va("cp \"%s\"", ConcatArgs(2) ) );
	else
		trap_SendServerCommand( -1, va("cp \"%s\"", ConcatArgs(1) ) );
}

/*
===================
Svcmd_SendAway_f
Tequila: Replacement for kick command from server engine with sendaway one
*
sendaway <playername>
===================
*/
void Svcmd_SendAway_f(void) {
	// find the player
	gclient_t *cl = ClientForString( ConcatArgs(1) );
	if ( cl )
		trap_SendConsoleCommand( EXEC_INSERT, va("clientkick %i\n", cl->ps.clientNum) );
	else
		trap_SendServerCommand( -1, va("print \"Can't kick %s\"", ConcatArgs(1)) );
}

/*
==================
Svcmd_CancelVote_f
Tequila: Admin tool to cancel a vote
==================
*/
void Svcmd_CancelVote_f( void ) {
	if ( !level.voteTime && !level.voteExecuteTime ) {
		G_Printf( "No vote to cancel.\n" );
	} else {
		level.voteExecuteTime = 0;
		level.voteTime = 0;
		trap_SetConfigstring( CS_VOTE_TIME, "" );
		trap_SendServerCommand( -1, "print \"server: Vote cancelled.\"" );
	}
}



/*
====================
Svcmd_PlayerStatus_f
Joe Kari: New admin command that provide usefull info to identify people.
====================
*/
static void Svcmd_PlayerStatus_f( void ) {
	int		i, j, l;
	gclient_t	*cl;
	const char	*s;
	
	for ( i = 0 , cl = level.clients ; i < level.maxclients ; i ++ , cl ++ )
	{
		if ( cl->pers.connected != CON_CONNECTED )
			continue;
		
		Com_Printf ("\n");
		
		// ID
		Com_Printf ("^5ID^7:%i  ", i);
		// Name
		Com_Printf ("^5name^7:%s^7\n", cl->pers.netname);
		
		// Score
		Com_Printf ("^2S^7:%i  ", cl->ps.persistant[PERS_SCORE]);
		// Kill
		Com_Printf ("^2K^7:%i   ", cl->pers.kill);
		// Death
		Com_Printf ("^2D^7:%i  ", cl->pers.death);
		// Teamkill
		Com_Printf ("^2TK^7:%i  ", cl->pers.teamkill);
		// Selfkill
		Com_Printf ("^2SK^7:%i  ", cl->pers.selfkill);
		// Money
		Com_Printf ("^2$^7:%i  ", cl->ps.stats[STAT_MONEY]);
		// Health/Max (handicap)
		Com_Printf ("^2HP^7:%i/%i  ", cl->ps.stats[STAT_HEALTH], cl->pers.maxHealth);
		// Team and robber
		switch ( cl->ps.persistant[PERS_TEAM] )
		{
			case 1 : s = "lawmen" ; break ;
			case 2 : s = "outlaw" ; break ;
			case 3 : s = "spec" ; break ;
			case 4 : s = "lawmen[D]" ; break ;
			case 5 : s = "outlaw[D]" ; break ;
			default: s = "-" ;
		}
		Com_Printf ("^2team^7:%s%s  ", s, cl->ps.persistant[PERS_ROBBER] ? "[R]" : "" );
		// Time
		Com_Printf ("^2time^7:%i  ", (int)( level.time - cl->pers.enterTime ) / 1000 ) ;
		
		Com_Printf ("\n");
	}
	Com_Printf ("\n");
}

#endif

/*
=================
ConsoleCommand

=================
*/
qboolean	ConsoleCommand( void ) {
	char	cmd[MAX_TOKEN_CHARS];

	trap_Argv( 0, cmd, sizeof( cmd ) );

	if ( Q_stricmp (cmd, "entitylist") == 0 ) {
		Svcmd_EntityList_f();
		return qtrue;
	}

	if ( Q_stricmp (cmd, "forceteam") == 0 ) {
		Svcmd_ForceTeam_f();
		return qtrue;
	}

	if (Q_stricmp (cmd, "game_memory") == 0) {
		Svcmd_GameMem_f();
		return qtrue;
	}

	if (Q_stricmp (cmd, "addbot") == 0) {
		Svcmd_AddBot_f();
		return qtrue;
	}

#ifdef SMOKINGUNS
	if (Q_stricmp (cmd, "kickbots") == 0) {
		Svcmd_KickBots_f();
		return qtrue;
	}
#endif

	if (Q_stricmp (cmd, "botlist") == 0) {
		Svcmd_BotList_f();
		return qtrue;
	}

#ifndef SMOKINGUNS
	if (Q_stricmp (cmd, "abort_podium") == 0) {
		Svcmd_AbortPodium_f();
		return qtrue;
	}
#endif

	if (Q_stricmp (cmd, "addip") == 0) {
		Svcmd_AddIP_f();
		return qtrue;
	}

	if (Q_stricmp (cmd, "removeip") == 0) {
		Svcmd_RemoveIP_f();
		return qtrue;
	}

	if (Q_stricmp (cmd, "listip") == 0) {
		trap_SendConsoleCommand( EXEC_NOW, "g_banIPs\n" );
		return qtrue;
	}

#ifdef SMOKINGUNS
	// commands for changing weapon properties
	if (Q_stricmp(cmd, "spread") == 0) {
		CMD_WEAPON_INFO(spread);
		return qtrue;
	}

	if (Q_stricmp(cmd, "damage") == 0) {
		CMD_WEAPON_INFO(damage);
		return qtrue;
	}

	if (Q_stricmp(cmd, "range") == 0) {
		CMD_WEAPON_INFO(range);
		return qtrue;
	}

	// Tequila: Big text announcement command
	if (Q_stricmp (cmd, "bigtext") == 0 || Q_stricmp (cmd, "cp") == 0) {
		Svcmd_BigText_f();
		return qtrue;
	}

	// Tequila: Replacement for kick command from server engine with sendaway one
	if (Q_stricmp (cmd, "sendaway") == 0) {
		Svcmd_SendAway_f();
		return qtrue;
	}

	// Tequila: New admin command to cancel a vote
	// Joe Kari: Move it out of "if (g_dedicated.integer) {}"
	if (Q_stricmp (cmd, "cancelvote") == 0) {
		Svcmd_CancelVote_f();
		return qtrue;
	}
	
	if (Q_stricmp (cmd, "playerstatus") == 0) {
		Svcmd_PlayerStatus_f();
		return qtrue;
	}
	
	// Tequila: New command to add a map entity for testing purpose
	if (Q_stricmp (cmd, "addentity") == 0) {
		Svcmd_AddEntity_f();
		return qtrue;
	}

#endif

	if (g_dedicated.integer) {
		if (Q_stricmp (cmd, "say") == 0) {
			trap_SendServerCommand( -1, va("print \"server: %s\"", ConcatArgs(1) ) );
			return qtrue;
		}
		// everything else will also be printed as a say command
		trap_SendServerCommand( -1, va("print \"server: %s\"", ConcatArgs(0) ) );
		return qtrue;
	}

	return qfalse;
}

