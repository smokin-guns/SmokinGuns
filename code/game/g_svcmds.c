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



char	*ConcatArgs( int start );

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
		G_Printf("Usage: addip <ip-mask>\n");
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
		G_Printf("Usage: removeip <ip-mask>\n");
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

	if ( trap_Argc() < 3 ) {
		G_Printf("Usage: forceteam <player> <team>\n");
		return;
	}

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


#ifdef SMOKINGUNS
/*
===================
Svcmd_ForceTeamNum_f
Joe Kari: same than Svcmd_ForceTeam_f() except it accepts a client ID rather than its name. Usefull for RCON script bots.
Also, it prevent user from setting it to anything. Old Svcmd_ForceTeam_f() can lead to some bug.
*
forceteamnum <num> <team>
===================
*/
void Svcmd_ForceTeamNum_f( void ) {
	char		arg[MAX_TOKEN_CHARS];
	gclient_t	*cl;
	int		clientNum;

	trap_Argv( 1, arg, sizeof( arg ) );
	clientNum = atoi( arg );
	
	if ( clientNum >= level.maxclients || clientNum < 0 ) {
		Com_Printf("client not found\n");
		return;
	}
		                
	cl = &level.clients[ clientNum ] ;
	if ( cl->pers.connected != CON_CONNECTED ) {
		Com_Printf("client not found\n");
		return;
	}
	
	// set the team
	trap_Argv( 2, arg, sizeof( arg ) );
	
	if ( !Q_stricmp( arg, "lawmen" ) || !Q_stricmp( arg, "red" ) || !Q_stricmp( arg, "r" ) ) {
		SetTeam( &g_entities[cl - level.clients], "rs" );
	}
	else if ( !Q_stricmp( arg, "outlaws" ) || !Q_stricmp( arg, "blue" ) || !Q_stricmp( arg, "b" ) ) {
		SetTeam( &g_entities[cl - level.clients], "bs" );
	}
	else if ( !Q_stricmp( arg, "spectator" ) || !Q_stricmp( arg, "spec" ) || !Q_stricmp( arg, "s" ) ) {
		SetTeam( &g_entities[cl - level.clients], "s" );
	}
	else if ( !Q_stricmp( arg, "join" ) || !Q_stricmp( arg, "auto" ) ) {
		SetTeam( &g_entities[cl - level.clients], "join" );
	}
	else {
		Com_Printf("bad team name\n");
	}
}



/*
==================
Svcmd_GiveMoney_f
Joe Kari: Give money to a client, usefull for third party RCON script (I planned to write a poker RCON script).
*
givemoney <num> <amount>
==================
*/
void Svcmd_GiveMoney_f ()
{
	char		arg[MAX_TOKEN_CHARS];
	gclient_t	*cl;
	int		clientNum;
	int		amount;
	int		saved_money;

	if ( trap_Argc() < 2 ) {
		G_Printf ( "Usage: givemoney <client number> <amount>\n");
		return;
	}
	
	trap_Argv( 1, arg, sizeof( arg ) );
	clientNum = atoi( arg );
	
	if ( clientNum >= level.maxclients || clientNum < 0 ) {
		Com_Printf("client not found\n");
		return;
	}
		                
	cl = &level.clients[ clientNum ] ;
	if ( cl->pers.connected != CON_CONNECTED ) {
		Com_Printf("client not found\n");
		return;
	}
	
	trap_Argv( 2, arg, sizeof( arg ) );
	amount = atoi( arg );
	
	saved_money = cl->ps.stats[STAT_MONEY] ;
	cl->ps.stats[STAT_MONEY] += amount;
	
	if ( cl->ps.stats[STAT_MONEY] > g_maxMoney.integer )  cl->ps.stats[STAT_MONEY] = g_maxMoney.integer ;
	else if ( cl->ps.stats[STAT_MONEY] < 0 )  cl->ps.stats[STAT_MONEY] = 0 ;
	
	amount = cl->ps.stats[STAT_MONEY] - saved_money ;
	
	// Joe Kari: it's important to advertise here, to prevent abuser (yes, admins can be cheaters too)
	if ( amount > 0 ) {
		trap_SendServerCommand( -1, va( "print \"%s^7 ^2earned %i$ !^7\n\"", cl->pers.netname , amount ) );
	}
	else if ( amount < 0 ) {
		trap_SendServerCommand( -1, va( "print \"%s^7 ^1lost %i$ !^7\n\"", cl->pers.netname , - amount ) );
	}
	
	// RCON script needs to know how many money was given/taken.
	Com_Printf ("%i$\n", amount);
}


/*
==================
Svcmd_GiveItem_f
Joe Kari: Give an item to a client, usefull for third party RCON script.
Mostly a copy and paste of Cmd_BuyItem_f() from g_cmds.c
*
giveitem <num> <item classname>
==================
*/
void Svcmd_GiveItem_f ()
{
	char		arg[MAX_TOKEN_CHARS];
	gclient_t	*cl;
	int		clientNum;
	gitem_t		*item;
	gentity_t	*ent;
	gentity_t	*item_ent;
	int		weapon;
	int		clip;
	int		clipAmmo;
	int		maxAmmo;
	//int		quantity;
	int		belt;
	int		bitmask;

	if ( trap_Argc() < 2 ) {
		G_Printf ( "Usage: giveitem <client number> <item_code>\n");
		return;
	}
	
	trap_Argv( 1, arg, sizeof( arg ) );
	clientNum = atoi( arg );
	
	if ( clientNum >= level.maxclients || clientNum < 0 ) {
		Com_Printf("client not found\n");
		return;
	}
	
	cl = &level.clients[ clientNum ] ;
	if ( cl->pers.connected != CON_CONNECTED ) {
		Com_Printf("client not found\n");
		return;
	}
	
	if(cl->ps.stats[STAT_HEALTH] <= 0) {
		Com_Printf("client is dead\n");
		return;
	}
	
	if(cl->sess.sessionTeam >= TEAM_SPECTATOR) {
		Com_Printf("client is a spectator\n");
		return;
	}
	
	ent = &g_entities[ clientNum ];
	
	trap_Argv( 2, arg, sizeof( arg ) );
	item = BG_FindItemForClassname( arg );
	
	if(!item) {
		Com_Printf("Item not found\n");
		return;
	}
	
	belt = 1;
	if(cl->ps.powerups[PW_BELT])  belt = 2;
	
	// item->giTag item->giType
	switch( item->giType )
	{
		case IT_WEAPON :
			weapon = item->giTag ;
			bitmask = 1 << weapon ;
			
			clip = bg_weaponlist[weapon].clip ;
			maxAmmo = bg_weaponlist[weapon].maxAmmo ;
			clipAmmo = bg_weaponlist[weapon].clipAmmo ;
			
			if ( ! clip ) {
				// Weapons such as dynamite, bowie knifes, etc...
				// We give half of the max ammo value.
				if ( cl->ps.ammo[weapon] >= maxAmmo )  return ;
				cl->ps.ammo[weapon] += (int)( maxAmmo / 2 ) ;
				if ( cl->ps.ammo[weapon] > maxAmmo )  cl->ps.ammo[weapon] = maxAmmo ;
			} else if ( ent->client->ps.stats[STAT_WEAPONS] & bitmask ) {
				// He already have the weapon
				if ( item->weapon_sort != WS_PISTOL )  return ;
				if ( cl->ps.stats[STAT_FLAGS] & SF_SEC_PISTOL )  return ;
				cl->ps.stats[STAT_FLAGS] |= SF_SEC_PISTOL ;
				
				// load it
				cl->ps.ammo[WP_AKIMBO] = clipAmmo ;
				//give ammo
				if ( cl->ps.ammo[clip] < maxAmmo )  cl->ps.ammo[clip] = maxAmmo ;
			} else {
				// add weapon
				ent->client->ps.stats[STAT_WEAPONS] |= bitmask ;
				// load it
				cl->ps.ammo[weapon] = clipAmmo ;
				//give ammo
				if ( cl->ps.ammo[clip] < maxAmmo )  cl->ps.ammo[clip] = maxAmmo ;
			}
			
			break ;
		
		case IT_AMMO :
			if (item->giTag == WP_BULLETS_CLIP) {
				item = BG_FindItemForAmmo(WP_BULLETS_CLIP);
				weapon = WP_PEACEMAKER;
			} else {
				if ( cl->ps.stats[STAT_GATLING_MODE] )  weapon = WP_GATLING;
				else  weapon = BG_FindPlayerWeapon(WP_WINCHESTER66, WP_DYNAMITE, &cl->ps);
				
				if ( weapon )  item = BG_FindItemForAmmo(bg_weaponlist[weapon].clip);
				else  return;
			}
			
			if( cl->ps.ammo[item->giTag] >= belt * bg_weaponlist[weapon].maxAmmo)  return;
			
			cl->ps.ammo[item->giTag] += bg_weaponlist[weapon].maxAmmo ;
			
			if( cl->ps.ammo[item->giTag] >= belt * bg_weaponlist[weapon].maxAmmo)
				cl->ps.ammo[item->giTag] = belt * bg_weaponlist[weapon].maxAmmo ;
			
			break ;
		
		case IT_POWERUP :
			if ( item->giTag == PW_BELT ) {
				cl->ps.ammo[WP_BULLETS_CLIP] = bg_weaponlist[WP_REM58].maxAmmo * 2 ;
				cl->ps.ammo[WP_SHELLS_CLIP] = bg_weaponlist[WP_REMINGTON_GAUGE].maxAmmo * 2 ;
				cl->ps.ammo[WP_CART_CLIP] = bg_weaponlist[WP_WINCHESTER66].maxAmmo * 2 ;
				cl->ps.ammo[WP_GATLING_CLIP] = bg_weaponlist[WP_GATLING].maxAmmo * 2 ;
				cl->ps.ammo[WP_SHARPS_CLIP] = bg_weaponlist[WP_SHARPS].maxAmmo * 2 ;
			}
			else if ( cl->ps.powerups[item->giTag] ) {
				return ;
			}
			
			cl->ps.powerups[item->giTag] = 1 ;
			
			break ;
		
		case IT_ARMOR :
			if ( /* cl->ps.powerups[item->giTag] && */ cl->ps.stats[STAT_ARMOR] >= BOILER_PLATE )  return ;
			//cl->ps.powerups[item->giTag] = 1 ;
			cl->ps.stats[STAT_ARMOR] = BOILER_PLATE ;
			break ;
		
		default :
			return ;
	}
	
	                        
	
	// Joe Kari: it's important to advertise here, to prevent abuser (yes, admins can be cheaters too)
	trap_SendServerCommand( -1, va( "print \"%s^7 ^2earned some %s !^7\n\"", cl->pers.netname , item->pickup_name ) );
}


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
Svcmd_PushMinilog_f
===================
*/
void Svcmd_PushMinilog_f(void) {
	PushMinilog( ConcatArgs(1) ) ;
}

/*
===================
Svcmd_PopMinilog_f
===================
*/
void Svcmd_PopMinilog_f(void) {
	char		str[MAX_TOKEN_CHARS];
	PopMinilog( str ) ;
	Com_Printf( "%s\n" , str ) ;
}

/*
=====================
Svcmd_PopAllMinilog_f
=====================
*/
void Svcmd_PopAllMinilog_f(void) {
	char		str[MAX_TOKEN_CHARS];
	while ( 1 ) {
		PopMinilog( str ) ;
		if ( str[ 0 ] != 0 )  Com_Printf( "%s\n" , str ) ;
		else  break ;
	} ;
}

/*
==================
Svcmd_KickNum_f
Joe Kari: there was in server/sv_ccmds.c a note that mention "FIXME: move to game", so I move it to game...
Additionnaly, you can now provide a reason for kicking someone.
*
kicknum <client number> [reasons]
==================
*/
static void Svcmd_KickNum_f( void ) {
	char		arg[MAX_TOKEN_CHARS];
	int		clientNum;
	gclient_t	*cl;
	char		*reason;

	if ( trap_Argc() < 2 ) {
		G_Printf ( "Usage: kicknum <client number> [reasons]\n");
		return;
	}

	trap_Argv( 1, arg, sizeof( arg ) );
	clientNum = atoi( arg );
	
	if ( clientNum >= level.maxclients || clientNum < 0 ) {
		Com_Printf("client not found\n");
		return;
	}
		                
	cl = &level.clients[ clientNum ] ;
	if ( cl->pers.connected != CON_CONNECTED ) {
		Com_Printf("client not found\n");
		return;
	}
	
	if ( cl->pers.localClient ) {
		Com_Printf("Cannot kick host player\n");
		return;
	}
	
	if ( trap_Argc() > 2 ) {
		Q_strncpyz ( arg, "was kicked: ", sizeof(arg));
		reason = ConcatArgs( 2 ) ;
		Q_strcat( arg, sizeof(arg), reason);
		PushMinilogf( "KICK: %i > %s", clientNum, reason ) ;
		trap_DropClient( clientNum, arg );
	}
	else {
		PushMinilogf( "KICK: %i", clientNum ) ;
		trap_DropClient( clientNum, "was kicked" );
	}
	
	/* FIXME? adapt this part of the server code to the game code?
	cl->lastPacketTime = svs.time;	// in case there is a funny zombie
	*/
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
		
		// ID
		Com_Printf ("^5*** ID^7:%i ", i);
		// Name
		Com_Printf ("^5name^7:%s^7\n", cl->pers.netname);
		
		// Score
		Com_Printf ("^2S^7:%i ", cl->ps.persistant[PERS_SCORE]);
		// Kill
		Com_Printf ("^2K^7:%i ", cl->pers.kill);
		// Death
		Com_Printf ("^2D^7:%i ", cl->pers.death);
		// Teamkill
		Com_Printf ("^2TK^7:%i ", cl->pers.teamkill);
		// Selfkill
		Com_Printf ("^2SK^7:%i ", cl->pers.selfkill);
		// Rob
		Com_Printf ("^2RB^7:%i ", cl->pers.rob);
		// Health/Max (handicap)
		Com_Printf ("^2HP^7:%i/%i ", cl->ps.stats[STAT_HEALTH], cl->pers.maxHealth);
		// Money
		Com_Printf ("^2$^7:%i ", cl->ps.stats[STAT_MONEY]);
		// Team and robber
		switch ( cl->ps.persistant[PERS_TEAM] )
		{
			// use only generic team name here (not real team name, it would confuse admin-bots)
			case 1 : s = "lawmen" ; break ;
			case 2 : s = "outlaws" ; break ;
			case 3 : s = "spec" ; break ;
			case 4 : s = "lawmen[D]" ; break ;
			case 5 : s = "outlaws[D]" ; break ;
			default: s = "free" ;
		}
		Com_Printf ("^2TM^7:%s%s ", s, cl->ps.persistant[PERS_ROBBER] ? "[R]" : "" );
		// Time
		Com_Printf ("^2T^7:%i ", (int)( level.time - cl->pers.enterTime ) / 1000 ) ;
		// Muted
		Com_Printf ("^2M^7:%i ", cl->pers.muted);
		
		Com_Printf ("\n");
	}
}

/*
====================
Svcmd_Mute_f
Joe Kari: Mute or un-mute a client
====================
*/
static void Svcmd_Mute_f( int muted ) {
	char		arg[MAX_TOKEN_CHARS];
	int		clientNum;
	gclient_t	*cl;

	if ( trap_Argc() < 2 ) {
		G_Printf("Usage:  mute <client id>\n");
		return;
	}

	trap_Argv( 1, arg, sizeof( arg ) );
	clientNum = atoi( arg );
	
	if ( clientNum >= level.maxclients || clientNum < 0 ) {
		G_Printf("client not found>\n");
		return;
	}
		                
	cl = &level.clients[ clientNum ] ;
	if ( cl->pers.connected != CON_CONNECTED ) {
		Com_Printf("client not found>\n");
		return;
	}
	
	if ( muted )  cl->pers.muted += muted ;
	else  cl->pers.muted = 0 ;
	
	if ( cl->pers.muted > 2 )  cl->pers.muted = 2 ;
	if ( cl->pers.muted < 0 )  cl->pers.muted = 0 ;
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
	// Tequila: Big text announcement command
	if (Q_stricmp (cmd, "bigtext") == 0 || Q_stricmp (cmd, "cp") == 0) {
		Svcmd_BigText_f();
		return qtrue;
	}

	// Joe Kari: Minilog feature, for admin-bot
	if (Q_stricmp (cmd, "poponelog") == 0 ) {
		Svcmd_PopMinilog_f();
		return qtrue;
	}
	if (Q_stricmp (cmd, "poplog") == 0 ) {
		Svcmd_PopAllMinilog_f();
		return qtrue;
	}
	// It is probably not a good idea to allow an user to push the minilog, it would interfer with game log
	//if (Q_stricmp (cmd, "pushlog") == 0 ) {
	//	Svcmd_PushMinilog_f();
	//	return qtrue;
	//}

	// Joe Kari: Mute command: change everything this client said to "(muted)"
	if (Q_stricmp (cmd, "mute") == 0) {
		Svcmd_Mute_f( 1 );
		return qtrue;
	}
	if (Q_stricmp (cmd, "unmute") == 0) {
		Svcmd_Mute_f( 0 );
		return qtrue;
	}

	// Joe Kari: give some money to a client
	if ( Q_stricmp (cmd, "givemoney") == 0 ) {
		Svcmd_GiveMoney_f();
		return qtrue;
	}

	// Joe Kari: give some money to a client
	if ( Q_stricmp (cmd, "giveitem") == 0 ) {
		Svcmd_GiveItem_f();
		return qtrue;
	}

	// Joe Kari: replacement for forceteam, using client ID instead of name
	if ( Q_stricmp (cmd, "forceteamnum") == 0 ) {
		Svcmd_ForceTeamNum_f();
		return qtrue;
	}

	// Joe Kari: game version of clientkick
	if (Q_stricmp (cmd, "kicknum") == 0) {
		Svcmd_KickNum_f();
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
			trap_SendServerCommand( -1, va("print \"server: %s\n\"", ConcatArgs(1) ) );
			return qtrue;
		}
		// everything else will also be printed as a say command
		trap_SendServerCommand( -1, va("print \"server: %s\n\"", ConcatArgs(0) ) );
		return qtrue;
	}

	return qfalse;
}

