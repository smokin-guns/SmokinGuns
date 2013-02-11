/*
===========================================================================
Copyright (C) 1999-2005 Id Software, Inc.
Copyright (C) 2000-2003 Iron Claw Interactive
Copyright (C) 2005-2013 Smokin' Guns

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

#include "g_local.h"

level_locals_t	level;

typedef struct {
	vmCvar_t	*vmCvar;
	char		*cvarName;
	char		*defaultString;
	int			cvarFlags;
	int			modificationCount;	// for tracking changes
	qboolean	trackChange;		// track this variable, and announce if changed
	qboolean	teamShader;			// track and if changed, update shader state
} cvarTable_t;

gentity_t		g_entities[MAX_GENTITIES];
gclient_t		g_clients[MAX_CLIENTS];

vmCvar_t	g_gametype;

#ifdef SMOKINGUNS

// Joe Kari: minilog init
minilog_t	g_minilog;


//hitdata
hit_data_t	hit_data;

//Spoon
int			g_round;
int			g_maxmapparts;
int			g_roundendtime;
int			g_roundstarttime;

int			g_session;



int		g_humancount;	// human connected (playing or spectating)

vmCvar_t	g_moneyRespawn;
vmCvar_t	g_maxMoney;

// bank robbery
qboolean	g_goldescaped;
qboolean	g_robbed;
int			g_robteam;
int			g_defendteam;

vmCvar_t	g_robberReward;

vmCvar_t	g_redteamcount;
vmCvar_t	g_blueteamcount;

vmCvar_t	g_redteamscore;
vmCvar_t	g_blueteamscore;


//which spawnpoints shell be used?
int			sg_redspawn;
int			sg_bluespawn;

vmCvar_t	g_chaseonly;
vmCvar_t	sg_rtppoints;
vmCvar_t	g_deathcam;
vmCvar_t	g_roundtime;
vmCvar_t	g_specsareflies;

vmCvar_t	g_splitChat;

vmCvar_t	g_startingMoney;
vmCvar_t	m_maxreward;
vmCvar_t	m_teamwin;
vmCvar_t	m_teamlose;


vmCvar_t	g_duellimit;
#endif
vmCvar_t	g_dmflags;
vmCvar_t	g_fraglimit;
vmCvar_t	g_timelimit;
#ifndef SMOKINGUNS
vmCvar_t	g_capturelimit;
#else
vmCvar_t	g_scorelimit;
#endif
vmCvar_t	g_friendlyFire;
#ifdef SMOKINGUNS
vmCvar_t	g_maxteamkills;
vmCvar_t	g_teamkillsforgettime;
vmCvar_t	g_teamkillschecktime;
vmCvar_t	g_mapcycles;
#endif
vmCvar_t	g_password;
vmCvar_t	g_needpass;
vmCvar_t	g_maxclients;
vmCvar_t	g_maxGameClients;
vmCvar_t	g_dedicated;
vmCvar_t	g_speed;
vmCvar_t	g_gravity;
vmCvar_t	g_cheats;
vmCvar_t	g_knockback;
#ifndef SMOKINGUNS
vmCvar_t	g_quadfactor;
#endif
vmCvar_t	g_forcerespawn;
vmCvar_t	g_inactivity;
vmCvar_t	g_debugMove;
vmCvar_t	g_debugDamage;
vmCvar_t	g_debugAlloc;
#ifdef SMOKINGUNS
vmCvar_t	g_debugWeapon;
#endif
vmCvar_t	g_weaponRespawn;
vmCvar_t	g_weaponTeamRespawn;
vmCvar_t	g_motd;
vmCvar_t	g_synchronousClients;
vmCvar_t	g_warmup;
vmCvar_t	g_doWarmup;
vmCvar_t	g_restarted;
vmCvar_t	g_logfile;
vmCvar_t	g_logfileSync;
vmCvar_t	g_blood;
#ifndef SMOKINGUNS
vmCvar_t	g_podiumDist;
vmCvar_t	g_podiumDrop;
#endif
vmCvar_t	g_allowVote;
#ifdef SMOKINGUNS
vmCvar_t	g_allowVoteKick;
vmCvar_t	g_voteMinLevelTime;
vmCvar_t	g_voteDelay;
vmCvar_t	g_maxVote;
vmCvar_t	g_delayedRenaming;
#endif
vmCvar_t	g_teamAutoJoin;
vmCvar_t	g_teamForceBalance;
vmCvar_t	g_banIPs;
vmCvar_t	g_filterBan;
vmCvar_t	g_smoothClients;
vmCvar_t	pmove_fixed;
vmCvar_t	pmove_msec;
#ifndef SMOKINGUNS
vmCvar_t	g_rankings;
#endif
vmCvar_t	g_listEntity;
#ifndef SMOKINGUNS
#ifdef MISSIONPACK
vmCvar_t	g_obeliskHealth;
vmCvar_t	g_obeliskRegenPeriod;
vmCvar_t	g_obeliskRegenAmount;
vmCvar_t	g_obeliskRespawnDelay;
vmCvar_t	g_cubeTimeout;
vmCvar_t	g_redteam;
vmCvar_t	g_blueteam;
vmCvar_t	g_singlePlayer;
vmCvar_t	g_enableDust;
vmCvar_t	g_enableBreath;
vmCvar_t	g_proxMineTimeout;
#endif
#else
vmCvar_t	g_version;
vmCvar_t	g_checkClients;
vmCvar_t	g_breakspawndelay;
vmCvar_t	g_forcebreakrespawn;

//unlagged - server options
vmCvar_t	g_delagHitscan;
vmCvar_t	g_truePing;
vmCvar_t	sv_fps;
//unlagged - server options

vmCvar_t	g_redteam;
vmCvar_t	g_blueteam;

// duel cvars
vmCvar_t	du_enabletrio;
vmCvar_t	du_forcetrio;

// BR cvars
vmCvar_t	br_teamrole;


// experimental cvars
vmCvar_t	g_newShotgunPattern;
vmCvar_t	g_roundNoMoveTime;

vmCvar_t	g_startingWeapon;
vmCvar_t	g_bulletDamageMode;
vmCvar_t	g_bulletDamageAlert;
vmCvar_t	g_bulletDamageALDRmidrangefactor;
vmCvar_t	g_bulletDamageALDRmidpointfactor;
vmCvar_t	g_bulletDamageALDRminifactor;

qboolean b_sWaitingForPlayers = qfalse;
int i_sNextWaitPrint = 0;
int i_sNextCount = 0;
#endif

static cvarTable_t		gameCvarTable[] = {
	// don't override the cheat state set by the system
	{ &g_cheats, "sv_cheats", "", 0, 0, qfalse },

	// noset vars
	{ NULL, "gamename", GAMEVERSION , CVAR_SERVERINFO | CVAR_ROM, 0, qfalse  },
	{ NULL, "gamedate", __DATE__ , CVAR_ROM, 0, qfalse  },
	{ &g_restarted, "g_restarted", "0", CVAR_ROM, 0, qfalse  },

	// latched vars
	{ &g_gametype, "g_gametype", "0", CVAR_SERVERINFO | CVAR_USERINFO | CVAR_LATCH, 0, qfalse  },

	{ &g_maxclients, "sv_maxclients", "8", CVAR_SERVERINFO | CVAR_LATCH | CVAR_ARCHIVE, 0, qfalse  },
	{ &g_maxGameClients, "g_maxGameClients", "0", CVAR_SERVERINFO | CVAR_LATCH | CVAR_ARCHIVE, 0, qfalse  },

	// change anytime vars
	{ &g_dmflags, "dmflags", "0", CVAR_SERVERINFO | CVAR_ARCHIVE, 0, qtrue  },
	{ &g_fraglimit, "fraglimit", "20", CVAR_SERVERINFO | CVAR_ARCHIVE | CVAR_NORESTART, 0, qtrue },
	{ &g_timelimit, "timelimit", "0", CVAR_SERVERINFO | CVAR_ARCHIVE | CVAR_NORESTART, 0, qtrue },
#ifndef SMOKINGUNS
	{ &g_capturelimit, "capturelimit", "8", CVAR_SERVERINFO | CVAR_ARCHIVE | CVAR_NORESTART, 0, qtrue },
#else
	{ &g_scorelimit, "scorelimit", "10", CVAR_SERVERINFO | CVAR_ARCHIVE | CVAR_NORESTART, 0, qtrue },
	{ &g_duellimit, "duellimit", "3", CVAR_SERVERINFO | CVAR_ARCHIVE | CVAR_NORESTART, 0, qtrue },

	{ &du_enabletrio, "du_enabletrio", "0", CVAR_SERVERINFO | CVAR_ARCHIVE | CVAR_NORESTART, 0, qtrue },
	{ &du_forcetrio, "du_forcetrio", "0", CVAR_SERVERINFO | CVAR_ARCHIVE | CVAR_NORESTART, 0, qtrue },

	{ &br_teamrole, "br_teamrole", "0", CVAR_ARCHIVE | CVAR_NORESTART, 0, qtrue },
	{ &g_moneyRespawn, "g_moneyRespawn", "1", CVAR_ARCHIVE | CVAR_NORESTART, 0, qtrue },
	{ &g_maxMoney, "g_maxMoney", "200", CVAR_SERVERINFO | CVAR_ARCHIVE | CVAR_LATCH, 0, qtrue },

	{ &g_newShotgunPattern, "g_newShotgunPattern", "1", CVAR_SERVERINFO | CVAR_ARCHIVE | CVAR_LATCH, 0, qtrue },
	{ &g_roundNoMoveTime, "g_roundNoMoveTime", "3", CVAR_SERVERINFO | CVAR_ARCHIVE | CVAR_NORESTART, 0, qtrue },
#endif

	{ &g_synchronousClients, "g_synchronousClients", "0", CVAR_SYSTEMINFO, 0, qfalse  },

#ifndef SMOKINGUNS
	{ &g_friendlyFire, "g_friendlyFire", "0", CVAR_ARCHIVE, 0, qtrue  },
#else
	{ &g_friendlyFire, "g_friendlyFire", "1", CVAR_ARCHIVE, 0, qtrue  },
	{ &g_maxteamkills, "g_maxteamkills", "0", CVAR_ARCHIVE | CVAR_NORESTART, 0, qtrue },
	{ &g_teamkillsforgettime, "g_teamkillsforgettime", "0", CVAR_ARCHIVE | CVAR_NORESTART, 0, qtrue },
	{ &g_teamkillschecktime, "g_teamkillschecktime", "0", CVAR_ARCHIVE | CVAR_NORESTART, 0, qtrue },
	{ &g_mapcycles, "g_mapcycles", "", CVAR_ARCHIVE, 0, qtrue },
#endif

	{ &g_teamAutoJoin, "g_teamAutoJoin", "0", CVAR_ARCHIVE  },
	{ &g_teamForceBalance, "g_teamForceBalance", "0", CVAR_ARCHIVE  },

	{ &g_warmup, "g_warmup", "20", CVAR_ARCHIVE, 0, qtrue  },
	{ &g_doWarmup, "g_doWarmup", "0", CVAR_ARCHIVE, 0, qtrue  },
	{ &g_logfile, "g_log", "games.log", CVAR_ARCHIVE, 0, qfalse  },
	{ &g_logfileSync, "g_logsync", "0", CVAR_ARCHIVE, 0, qfalse  },

	{ &g_password, "g_password", "", CVAR_USERINFO, 0, qfalse  },

	{ &g_banIPs, "g_banIPs", "", CVAR_ARCHIVE, 0, qfalse  },
	{ &g_filterBan, "g_filterBan", "1", CVAR_ARCHIVE, 0, qfalse  },

	{ &g_needpass, "g_needpass", "0", CVAR_SERVERINFO | CVAR_ROM, 0, qfalse },

	{ &g_dedicated, "dedicated", "0", 0, 0, qfalse  },

#ifndef SMOKINGUNS
	{ &g_speed, "g_speed", "320", 0, 0, qtrue  },
	{ &g_gravity, "g_gravity", "800", 0, 0, qtrue  },
#else
	{ &g_speed, "g_speed", "200", 0, 0, qtrue  },
	{ &g_gravity, "g_gravity", "900", 0, 0, qtrue  },
#endif
	{ &g_knockback, "g_knockback", "1000", 0, 0, qtrue  },
#ifndef SMOKINGUNS
	{ &g_quadfactor, "g_quadfactor", "3", 0, 0, qtrue  },
#endif
	{ &g_weaponRespawn, "g_weaponrespawn", "5", 0, 0, qtrue  },
	{ &g_weaponTeamRespawn, "g_weaponTeamRespawn", "30", 0, 0, qtrue },
	{ &g_forcerespawn, "g_forcerespawn", "20", 0, 0, qtrue },
	{ &g_inactivity, "g_inactivity", "0", 0, 0, qtrue },
	{ &g_debugMove, "g_debugMove", "0", 0, 0, qfalse },
	{ &g_debugDamage, "g_debugDamage", "0", 0, 0, qfalse },
#ifdef SMOKINGUNS
	{ &g_debugWeapon, "g_debugWeapon", "0", 0, 0, qfalse },
#endif
	{ &g_debugAlloc, "g_debugAlloc", "0", 0, 0, qfalse },
	{ &g_motd, "g_motd", "", 0, 0, qfalse },
	{ &g_blood, "com_blood", "1", 0, 0, qfalse },

#ifndef SMOKINGUNS
	{ &g_podiumDist, "g_podiumDist", "80", 0, 0, qfalse },
	{ &g_podiumDrop, "g_podiumDrop", "70", 0, 0, qfalse },

	{ &g_allowVote, "g_allowVote", "1", CVAR_ARCHIVE, 0, qfalse },
#else
	{ &g_allowVote, "g_allowVote", "1", CVAR_ARCHIVE | CVAR_SERVERINFO, 0, qfalse },
	{ &g_allowVoteKick, "g_allowVoteKick", "1", CVAR_ARCHIVE | CVAR_SERVERINFO, 0, qfalse },
	{ &g_voteMinLevelTime, "g_voteMinLevelTime", "40", CVAR_ARCHIVE, 0, qfalse },
	{ &g_voteDelay, "g_voteDelay", "60", CVAR_ARCHIVE, 0, qfalse },
	{ &g_maxVote, "g_maxVote", "4", CVAR_ARCHIVE, 0, qfalse },
	{ &g_delayedRenaming, "g_delayedRenaming", "20", CVAR_ARCHIVE, 0, qfalse },
#endif
	{ &g_listEntity, "g_listEntity", "0", 0, 0, qfalse },

#ifndef SMOKINGUNS
#ifdef MISSIONPACK
	{ &g_obeliskHealth, "g_obeliskHealth", "2500", 0, 0, qfalse },
	{ &g_obeliskRegenPeriod, "g_obeliskRegenPeriod", "1", 0, 0, qfalse },
	{ &g_obeliskRegenAmount, "g_obeliskRegenAmount", "15", 0, 0, qfalse },
	{ &g_obeliskRespawnDelay, "g_obeliskRespawnDelay", "10", CVAR_SERVERINFO, 0, qfalse },

	{ &g_cubeTimeout, "g_cubeTimeout", "30", 0, 0, qfalse },
	{ &g_redteam, "g_redteam", "Stroggs", CVAR_ARCHIVE | CVAR_SERVERINFO | CVAR_USERINFO , 0, qtrue, qtrue },
	{ &g_blueteam, "g_blueteam", "Pagans", CVAR_ARCHIVE | CVAR_SERVERINFO | CVAR_USERINFO , 0, qtrue, qtrue  },
	{ &g_singlePlayer, "ui_singlePlayerActive", "", 0, 0, qfalse, qfalse  },

	{ &g_enableDust, "g_enableDust", "0", CVAR_SERVERINFO, 0, qtrue, qfalse },
	{ &g_enableBreath, "g_enableBreath", "0", CVAR_SERVERINFO, 0, qtrue, qfalse },
	{ &g_proxMineTimeout, "g_proxMineTimeout", "20000", 0, 0, qfalse },
#endif
#else
	{ &g_redteam, "g_redteam", DEFAULT_REDTEAM_NAME, CVAR_ARCHIVE | CVAR_SERVERINFO | CVAR_USERINFO , 0, qtrue, qtrue },
	{ &g_blueteam, "g_blueteam", DEFAULT_BLUETEAM_NAME, CVAR_ARCHIVE | CVAR_SERVERINFO | CVAR_USERINFO , 0, qtrue, qtrue  },
#endif

	{ &g_smoothClients, "g_smoothClients", "1", 0, 0, qfalse},
	{ &pmove_fixed, "pmove_fixed", "0", CVAR_SYSTEMINFO, 0, qfalse},
	{ &pmove_msec, "pmove_msec", "8", CVAR_SYSTEMINFO, 0, qfalse},
#ifndef SMOKINGUNS
	{ &g_rankings, "g_rankings", "0", 0, 0, qfalse}
#else
//unlagged - server options
	{ &g_delagHitscan, "g_delagHitscan", "1", CVAR_ARCHIVE | CVAR_SERVERINFO, 0, qtrue },
	{ &g_truePing, "g_truePing", "1", CVAR_ARCHIVE, 0, qtrue },
	// it's CVAR_SYSTEMINFO so the client's sv_fps will be automagically set to its value
	{ &sv_fps, "sv_fps", "20", CVAR_SYSTEMINFO | CVAR_ARCHIVE, 0, qfalse },
//unlagged - server options
		//Spoon
	{ &g_roundtime, "g_roundtime", "4", CVAR_ARCHIVE|CVAR_SERVERINFO, 0, qtrue },
	{ &sg_rtppoints, "sg_rtppoints", "4", CVAR_ROM, 0, qtrue },
	{ &g_deathcam, "g_deathcam", "1", CVAR_ARCHIVE|CVAR_SERVERINFO, 0, qtrue },
	{ &g_chaseonly, "g_chaseonly", "0", CVAR_ARCHIVE|CVAR_SERVERINFO, 0, qtrue  },
	{ &g_specsareflies, "g_specsareflies", "1", CVAR_ARCHIVE|CVAR_SERVERINFO, 0, qtrue  },

	{ &g_splitChat, "g_splitChat", "1", CVAR_ARCHIVE, 0, qtrue  },

	{ &g_redteamcount, "g_redteamcount", "0", CVAR_SERVERINFO, 0, qfalse  },
	{ &g_blueteamcount, "g_blueteamcount", "0", CVAR_SERVERINFO, 0, qfalse  },

	{ &g_robberReward, "g_robberReward", "2", CVAR_ARCHIVE, 0, qfalse  },

	{ &g_redteamscore, "g_redteamscore", "0", CVAR_SERVERINFO, 0, qfalse  },
	{ &g_blueteamscore, "g_blueteamscore", "0", CVAR_SERVERINFO, 0, qfalse  },

	{ &g_startingMoney, "g_startingMoney", MIN_MONEY, CVAR_ARCHIVE, 0, qtrue },
	{ &m_maxreward, "m_maxreward", MAX_REWARD, CVAR_CHEAT, 0, qtrue },
	{ &m_teamwin, "m_teamwin", ROUND_WIN_MONEY, CVAR_CHEAT, 0, qtrue },
	{ &m_teamlose, "m_teamlose", ROUND_LOSE_MONEY, CVAR_CHEAT, 0, qtrue },

	{ &g_version, "sg_version", PRODUCT_VERSION " " PRODUCT_RELEASE, CVAR_ROM | CVAR_SERVERINFO , 0, qtrue },
	{ &g_checkClients, "g_checkClients", "1", CVAR_ARCHIVE, 0, qfalse  },

	// If g_breakspawndelay == 0, use BREAK_RESPAWN_TIME instead in g_mover.c
	{ &g_breakspawndelay, "g_breakspawndelay", "0", 0, 0, qtrue },
	{ &g_forcebreakrespawn, "g_forcebreakrespawn", "0", 0, 0, qtrue },
	{ &g_startingWeapon, "g_startingWeapon", "2", CVAR_ARCHIVE , 0, qtrue },
	{ &g_bulletDamageMode, "g_bulletDamageMode", "0", CVAR_ARCHIVE , 0, qtrue },
	{ &g_bulletDamageAlert, "g_bulletDamageAlert", "25", CVAR_ARCHIVE, 0, qtrue },
	{ &g_bulletDamageALDRmidrangefactor, "g_bulletDamageALDRmidrangefactor", "2", CVAR_ARCHIVE, 0, qtrue },
	{ &g_bulletDamageALDRmidpointfactor, "g_bulletDamageALDRmidpointfactor", "0.66", CVAR_ARCHIVE, 0, qtrue },
	{ &g_bulletDamageALDRminifactor, "g_bulletDamageALDRminifactor", "0.5", CVAR_ARCHIVE, 0, qtrue },
#endif

};

static int gameCvarTableSize = ARRAY_LEN( gameCvarTable );


void G_InitGame( int levelTime, int randomSeed, int restart );
void G_RunFrame( int levelTime );
void G_ShutdownGame( int restart );
void CheckExitRules( void );


/*
================
vmMain

This is the only way control passes into the module.
This must be the very first function compiled into the .q3vm file
================
*/
Q_EXPORT intptr_t vmMain( int command, int arg0, int arg1, int arg2, int arg3, int arg4, int arg5, int arg6, int arg7, int arg8, int arg9, int arg10, int arg11  ) {
	switch ( command ) {
	case GAME_INIT:
		G_InitGame( arg0, arg1, arg2 );
		return 0;
	case GAME_SHUTDOWN:
		G_ShutdownGame( arg0 );
		return 0;
	case GAME_CLIENT_CONNECT:
		return (intptr_t)ClientConnect( arg0, arg1, arg2 );
	case GAME_CLIENT_THINK:
		ClientThink( arg0 );
		return 0;
	case GAME_CLIENT_USERINFO_CHANGED:
		ClientUserinfoChanged( arg0 );
		return 0;
	case GAME_CLIENT_DISCONNECT:
		ClientDisconnect( arg0 );
		return 0;
	case GAME_CLIENT_BEGIN:
		ClientBegin( arg0 );
		return 0;
	case GAME_CLIENT_COMMAND:
		ClientCommand( arg0 );
		return 0;
	case GAME_RUN_FRAME:
		G_RunFrame( arg0 );
		return 0;
	case GAME_CONSOLE_COMMAND:
		return ConsoleCommand();
	case BOTAI_START_FRAME:
		return BotAIStartFrame( arg0 );
	}

	return -1;
}


void QDECL G_Printf( const char *fmt, ... ) {
	va_list		argptr;
	char		text[1024];

	va_start (argptr, fmt);
	Q_vsnprintf (text, sizeof(text), fmt, argptr);
	va_end (argptr);

	trap_Print( text );
}

void QDECL G_Error( const char *fmt, ... ) {
	va_list		argptr;
	char		text[1024];

	va_start (argptr, fmt);
	Q_vsnprintf (text, sizeof(text), fmt, argptr);
	va_end (argptr);

	trap_Error( text );
}

/*
================
G_FindTeams

Chain together all entities with a matching team field.
Entity teams are used for item groups and multi-entity mover groups.

All but the first will have the FL_TEAMSLAVE flag set and teammaster field set
All but the last will have the teamchain field set to the next one
================
*/
void G_FindTeams( void ) {
	gentity_t	*e, *e2;
	int		i, j;
	int		c, c2;

	c = 0;
	c2 = 0;
	for ( i=1, e=g_entities+i ; i < level.num_entities ; i++,e++ ){
		if (!e->inuse)
			continue;
		if (!e->team)
			continue;
		if (e->flags & FL_TEAMSLAVE)
			continue;
		e->teammaster = e;
		c++;
		c2++;
		for (j=i+1, e2=e+1 ; j < level.num_entities ; j++,e2++)
		{
			if (!e2->inuse)
				continue;
			if (!e2->team)
				continue;
			if (e2->flags & FL_TEAMSLAVE)
				continue;
			if (!strcmp(e->team, e2->team))
			{
				c2++;
				e2->teamchain = e->teamchain;
				e->teamchain = e2;
				e2->teammaster = e;
				e2->flags |= FL_TEAMSLAVE;

				// make sure that targets only point at the master
				if ( e2->targetname ) {
					e->targetname = e2->targetname;
					e2->targetname = NULL;
				}
			}
		}
	}

	G_Printf ("%i teams with %i entities\n", c, c2);
}

#ifndef SMOKINGUNS
void G_RemapTeamShaders( void ) {
#ifdef MISSIONPACK
	char string[1024];
	float f = level.time * 0.001;
	Com_sprintf( string, sizeof(string), "team_icon/%s_red", g_redteam.string );
	AddRemap("textures/ctf2/redteam01", string, f);
	AddRemap("textures/ctf2/redteam02", string, f);
	Com_sprintf( string, sizeof(string), "team_icon/%s_blue", g_blueteam.string );
	AddRemap("textures/ctf2/blueteam01", string, f);
	AddRemap("textures/ctf2/blueteam02", string, f);
	trap_SetConfigstring(CS_SHADERSTATE, BuildShaderStateConfig());
#endif
}
#endif


/*
=================
G_RegisterCvars
=================
*/
void G_RegisterCvars( void ) {
	int			i;
	cvarTable_t	*cv;
#ifndef SMOKINGUNS
	qboolean remapped = qfalse;
#endif

	for ( i = 0, cv = gameCvarTable ; i < gameCvarTableSize ; i++, cv++ ) {
		trap_Cvar_Register( cv->vmCvar, cv->cvarName,
			cv->defaultString, cv->cvarFlags );
		if ( cv->vmCvar )
			cv->modificationCount = cv->vmCvar->modificationCount;

#ifndef SMOKINGUNS
		if (cv->teamShader) {
			remapped = qtrue;
		}
#endif
	}

#ifndef SMOKINGUNS
	if (remapped) {
		G_RemapTeamShaders();
	}
#endif

	// check some things
	if ( g_gametype.integer < 0 || g_gametype.integer >= GT_MAX_GAME_TYPE ) {
		G_Printf( "g_gametype %i is out of range, defaulting to 0\n", g_gametype.integer );
		trap_Cvar_Set( "g_gametype", "0" );
		trap_Cvar_Update( &g_gametype );
	}

	level.warmupModificationCount = g_warmup.modificationCount;
}

/*
=================
G_UpdateCvars
=================
*/
void G_UpdateCvars( void ) {
	int			i;
	cvarTable_t	*cv;
#ifndef SMOKINGUNS
	qboolean remapped = qfalse;
#endif

	for ( i = 0, cv = gameCvarTable ; i < gameCvarTableSize ; i++, cv++ ) {
		if ( cv->vmCvar ) {
			trap_Cvar_Update( cv->vmCvar );

			if ( cv->modificationCount != cv->vmCvar->modificationCount ) {
				cv->modificationCount = cv->vmCvar->modificationCount;

				if ( cv->trackChange ) {
					trap_SendServerCommand( -1, va("print \"Server: %s changed to %s\n\"",
						cv->cvarName, cv->vmCvar->string ) );
				}

#ifndef SMOKINGUNS
				if (cv->teamShader) {
					remapped = qtrue;
				}
#endif
			}
		}
	}

#ifndef SMOKINGUNS
	if (remapped) {
		G_RemapTeamShaders();
	}
#endif
}

#ifdef SMOKINGUNS
shaderInfo_t shaderInfo[MAX_BRUSHSIDES];
int	shaderInfoNum;

qboolean Keyword( char *key, char *token, const char* filename){
	if(Q_stricmp(key, token)){
		G_Printf( "\nno \"%s\" in %s\n", key, filename  );
		return qfalse;
	}
	return qtrue;
}


/*
======================
G_ParseTexFile

Read texture information of a map using map.tex
not used anymore
======================
*/
qboolean G_ParseTexFile(const char *filename){
	int	len, i;
	fileHandle_t f;
	char buf[MAX_TEXINFOFILE];
	char **p, *ptr;
	char *token;

	len = trap_FS_FOpenFile( filename, &f, FS_READ );
	if ( !f ) {
		G_Printf(  "tex file not found: %s\n", filename  );
		trap_FS_FCloseFile( f );
		return qfalse;
	}
	if ( len >= MAX_TEXINFOFILE ) {
		G_Printf(  "tex file too large: %s is %i, max allowed is %i\n", filename, len, MAX_TEXINFOFILE  );
		trap_FS_FCloseFile( f );
		return qfalse;
	}

	trap_FS_Read( buf, len, f );
	buf[len] = 0;
	trap_FS_FCloseFile( f );

	//G_Printf(  "Loading tex-file... %s\n", filename  );

	ptr = buf;
	p = &ptr;

	//READ HEADER
	token = COM_ParseExt(p, qtrue);
	if (!token || token[0] == 0)
		return qtrue;

	if(!Keyword("TEXFILE", token, filename))
		return qfalse;

	//get shadernames
	//READ HEADER
	token = COM_ParseExt(p, qtrue);
	shaderInfoNum = atoi(token);
	if(shaderInfoNum > MAX_BRUSHSIDES){
		G_Printf( "Tex-file: %ishaders max allowed %i shaders\n", shaderInfoNum, MAX_BRUSHSIDES  );
		return qfalse;
	}
	//parse surfaces
	for(i=0;i<shaderInfoNum;i++){

		//G_Printf( "%s ", token  );

		token = COM_ParseExt(p, qtrue);
		shaderInfo[i].surfaceFlags = atoi(token);

		//colors
		token = COM_ParseExt(p, qtrue);
		token = COM_ParseExt(p, qtrue);
		token = COM_ParseExt(p, qtrue);
		//G_Printf("%i\n", shaderInfo[i].surfaceFlags  );
	}
	//G_Printf( "%i shaders\n", shaderInfoNum  );
	G_Printf("Server: Loading tex-file... %s\n", filename);
	return qtrue;
}
#endif

/*
============
G_InitGame

============
*/
#ifdef SMOKINGUNS
qboolean G_OpenFileAiNode(const char *filename);
#endif
void G_InitGame( int levelTime, int randomSeed, int restart ) {
	int					i;
#ifdef SMOKINGUNS
	char				map[64];
	char				map2[64];
	char				serverinfo[MAX_INFO_STRING];
	int					prefix_gametype;

	G_Printf( "QAgame version: %s, %s, %s\n", Q3_VERSION, PRODUCT_RELEASE, __TIME__ );
#endif

	G_Printf ("------- Game Initialization -------\n");
	G_Printf ("gamename: %s\n", GAMEVERSION);
	G_Printf ("gamedate: %s\n", __DATE__);

	srand( randomSeed );

	G_RegisterCvars();

	G_ProcessIPBans();

	G_InitMemory();

	// set some level globals
	memset( &level, 0, sizeof( level ) );
	level.time = levelTime;
	level.startTime = levelTime;

	level.snd_fry = G_SoundIndex("sound/player/fry.wav");	// FIXME standing in lava / slime

#ifdef SMOKINGUNS
	trap_GetServerinfo( serverinfo, sizeof(serverinfo) );
	Q_strncpyz(map, Info_ValueForKey( serverinfo, "mapname" ), sizeof(map));

	//check if there is a known prefix
	prefix_gametype = BG_MapPrefix(map, g_gametype.integer);

	trap_Cvar_Set("g_gametype", va("%i", prefix_gametype));
	g_gametype.integer = prefix_gametype;

	//read shader info
	Com_sprintf(map2, sizeof(map), "maps/%s.tex", map);
	G_ParseTexFile(map2);
	// read ainodes
	Com_sprintf(map2, sizeof(map), "maps/%s.ai", map);
	G_OpenFileAiNode(map2);
#endif

	if ( g_gametype.integer != GT_SINGLE_PLAYER && g_logfile.string[0] ) {
		if ( g_logfileSync.integer ) {
			trap_FS_FOpenFile( g_logfile.string, &level.logFile, FS_APPEND_SYNC );
		} else {
			trap_FS_FOpenFile( g_logfile.string, &level.logFile, FS_APPEND );
		}
		if ( !level.logFile ) {
			G_Printf( "WARNING: Couldn't open logfile: %s\n", g_logfile.string );
		} else {
			char	serverinfo[MAX_INFO_STRING];

			trap_GetServerinfo( serverinfo, sizeof( serverinfo ) );

			G_LogPrintf("------------------------------------------------------------\n" );
			G_LogPrintf("InitGame: %s\n", serverinfo );
		}
	} else {
		G_Printf( "Not logging to disk.\n" );
	}

	G_InitWorldSession();

	// initialize all entities for this game
	memset( g_entities, 0, MAX_GENTITIES * sizeof(g_entities[0]) );
	level.gentities = g_entities;

	// initialize all clients for this game
	level.maxclients = g_maxclients.integer;
	memset( g_clients, 0, MAX_CLIENTS * sizeof(g_clients[0]) );
	level.clients = g_clients;

	// set client fields on player ents
	for ( i=0 ; i<level.maxclients ; i++ ) {
		g_entities[i].client = level.clients + i;
	}

	// always leave room for the max number of clients,
	// even if they aren't all used, so numbers inside that
	// range are NEVER anything but clients
	level.num_entities = MAX_CLIENTS;

	for ( i=0 ; i<MAX_CLIENTS ; i++ ) {
		g_entities[i].classname = "clientslot";
	}

	// let the server system know where the entites are
	trap_LocateGameData( level.gentities, level.num_entities, sizeof( gentity_t ),
		&level.clients[0].ps, sizeof( level.clients[0] ) );

	// reserve some spots for dead player bodies
	InitBodyQue();

	ClearRegisteredItems();

	// parse hit files for models
#ifdef SMOKINGUNS
	if(!G_LoadHitFiles(&hit_data)){
		G_Error("Couldn't load hitfiles\n");
	}
#endif

	// parse the key/value pairs and spawn gentities
	G_SpawnEntitiesFromString();

	//determinate max mapparts
#ifdef SMOKINGUNS
	if(g_gametype.integer == GT_DUEL){
		g_maxmapparts = 0;
		for(i = 0; i < MAX_GENTITIES; i++){
			if(g_entities[i].mappart && !Q_stricmp(g_entities[i].classname, "info_player_intermission")){
				g_maxmapparts++;
				// Tequila comment: Read the intermission name from here now
				if (g_entities[i].mappartname && g_entities[i].mappartname[0]
				&& g_entities[i].mappart > 0 && g_entities[i].mappart<=MAX_MAPPARTS) {
					Q_strncpyz(intermission_names[g_entities[i].mappart-1], g_entities[i].mappartname, MAX_MAPPARTS_NAME_LENGTH);
					G_Printf("Loaded mappart %i: '%s'\n", g_entities[i].mappart, g_entities[i].mappartname);
				} else {
					G_Printf("Loaded mappart %i without mappart name\n", g_entities[i].mappart);
				}
			}
		}
		if(!g_maxmapparts){
			G_Printf("Warning: No Map Parts Defined! g_maxmapparts set to 1\n");
			g_maxmapparts = 1;
		}

		G_Printf("mapparts found: %i\n", g_maxmapparts);

	} else if(g_gametype.integer >= GT_RTP){
		int startpoints = 0;
		char points[5];

		for(i=0; i < MAX_GENTITIES; i++){
			gentity_t	*ent = &g_entities[i];
			if(!Q_stricmp(ent->classname, "team_CTF_redplayer") && startpoints == 0){
				startpoints = 1;
				i = 0;
			} else if(!Q_stricmp(ent->classname, "team_CTF_blueplayer") && startpoints == 1){
				startpoints = 2;
				i = 0;
			} else if(!Q_stricmp(ent->classname, "team_CTF_redspawn") && startpoints == 2){
				startpoints = 3;
				i = 0;
			} else if(!Q_stricmp(ent->classname, "team_CTF_bluespawn") && startpoints == 3){
				startpoints = 4;
				break;
			}
		}

		if(startpoints < 2 || startpoints > 4){
			startpoints = 2;
			G_Printf("Error: RTP-Startpoints aren't set correctly!");
		}

		Com_sprintf(points, sizeof(points), "%i", startpoints);

		trap_Cvar_Set( "sg_rtppoints", points);

		G_Printf("%i team-startpoints parsed\n", startpoints);
	}
#endif

	// general initialization
	G_FindTeams();

	// make sure we have flags for CTF, etc
#ifndef SMOKINGUNS
	if( g_gametype.integer >= GT_TEAM ) {
		G_CheckTeamItems();
	}
#endif

	SaveRegisteredItems();

	G_Printf ("-----------------------------------\n");

	if( g_gametype.integer == GT_SINGLE_PLAYER || trap_Cvar_VariableIntegerValue( "com_buildScript" ) ) {
		G_ModelIndex( SP_PODIUM_MODEL );
	}

	if ( trap_Cvar_VariableIntegerValue( "bot_enable" ) ) {
		BotAISetup( restart );
		BotAILoadMap( restart );
		G_InitBots( restart );
	}

#ifdef SMOKINGUNS
	PushMinilogf( "MAP: %s" , map ) ;
#else
	G_RemapTeamShaders();
#endif
}



/*
=================
G_ShutdownGame
=================
*/
void G_ShutdownGame( int restart ) {
	G_Printf ("==== ShutdownGame ====\n");

	if ( level.logFile ) {
		G_LogPrintf("ShutdownGame:\n" );
		G_LogPrintf("------------------------------------------------------------\n" );
		trap_FS_FCloseFile( level.logFile );
		level.logFile = 0;
	}

	// write all the client session data so we can get it back
	G_WriteSessionData();

	if ( trap_Cvar_VariableIntegerValue( "bot_enable" ) ) {
		BotAIShutdown( restart );
	}
}



//===================================================================

void QDECL Com_Error ( int level, const char *error, ... ) {
	va_list		argptr;
	char		text[1024];

	va_start (argptr, error);
	Q_vsnprintf (text, sizeof(text), error, argptr);
	va_end (argptr);

	trap_Error( text );
}

void QDECL Com_Printf( const char *msg, ... ) {
	va_list		argptr;
	char		text[1024];

	va_start (argptr, msg);
	Q_vsnprintf (text, sizeof(text), msg, argptr);
	va_end (argptr);

	trap_Print( text );
}


/*
========================================================================

PLAYER COUNTING / SCORE SORTING

========================================================================
*/

/*
=============
AddTournamentPlayer

If there are less than two tournament players, put a
spectator in the game and restart
=============
*/
void AddTournamentPlayer( void ) {
	int			i;
	gclient_t	*client;
	gclient_t	*nextInLine;

	if ( level.numPlayingClients >= 2 ) {
		return;
	}

	// never change during intermission
	if ( level.intermissiontime ) {
		return;
	}

	nextInLine = NULL;

	for ( i = 0 ; i < level.maxclients ; i++ ) {
		client = &level.clients[i];
		if ( client->pers.connected != CON_CONNECTED ) {
			continue;
		}
#ifndef SMOKINGUNS
		if ( client->sess.sessionTeam != TEAM_SPECTATOR ) {
#else
		if ( client->sess.sessionTeam < TEAM_SPECTATOR ) {
#endif
			continue;
		}
		// never select the dedicated follow or scoreboard clients
		if ( client->sess.spectatorState == SPECTATOR_SCOREBOARD ||
			client->sess.spectatorClient < 0  ) {
			continue;
		}

		if(!nextInLine || client->sess.spectatorNum > nextInLine->sess.spectatorNum)
			nextInLine = client;
	}

	if ( !nextInLine ) {
		return;
	}

	level.warmupTime = -1;

	// set them to free-for-all team
	SetTeam( &g_entities[ nextInLine - level.clients ], "f" );
}

/*
=======================
AddTournamentQueue

Add client to end of tournament queue
=======================
*/

void AddTournamentQueue(gclient_t *client)
{
	int index;
	gclient_t *curclient;
	
	for(index = 0; index < level.maxclients; index++)
	{
		curclient = &level.clients[index];
		
		if(curclient->pers.connected != CON_DISCONNECTED)
		{
			if(curclient == client)
				curclient->sess.spectatorNum = 0;
			else if(curclient->sess.sessionTeam == TEAM_SPECTATOR)
				curclient->sess.spectatorNum++;
		}
	}
}

/*
=======================
RemoveTournamentLoser

Make the loser a spectator at the back of the line
=======================
*/
void RemoveTournamentLoser( void ) {
	int			clientNum;

	if ( level.numPlayingClients != 2 ) {
		return;
	}

	clientNum = level.sortedClients[1];

	if ( level.clients[ clientNum ].pers.connected != CON_CONNECTED ) {
		return;
	}

	// make them a spectator
	SetTeam( &g_entities[ clientNum ], "s" );
}

/*
=======================
RemoveTournamentWinner
=======================
*/
void RemoveTournamentWinner( void ) {
	int			clientNum;

	if ( level.numPlayingClients != 2 ) {
		return;
	}

	clientNum = level.sortedClients[0];

	if ( level.clients[ clientNum ].pers.connected != CON_CONNECTED ) {
		return;
	}

	// make them a spectator
	SetTeam( &g_entities[ clientNum ], "s" );
}

/*
=======================
AdjustTournamentScores
=======================
*/
void AdjustTournamentScores( void ) {
	int			clientNum;

	clientNum = level.sortedClients[0];
	if ( level.clients[ clientNum ].pers.connected == CON_CONNECTED ) {
		level.clients[ clientNum ].sess.wins++;
		ClientUserinfoChanged( clientNum );
	}

	clientNum = level.sortedClients[1];
	if ( level.clients[ clientNum ].pers.connected == CON_CONNECTED ) {
		level.clients[ clientNum ].sess.losses++;
		ClientUserinfoChanged( clientNum );
	}

}

/*
=============
SortRanks

=============
*/
int QDECL SortRanks( const void *a, const void *b ) {
	gclient_t	*ca, *cb;
#ifdef SMOKINGUNS
	qboolean a_realspec;
	qboolean b_realspec;
#endif

	ca = &level.clients[*(int *)a];
	cb = &level.clients[*(int *)b];

#ifdef SMOKINGUNS
	a_realspec = (ca->realspec || g_gametype.integer != GT_DUEL);
	b_realspec = (cb->realspec || g_gametype.integer != GT_DUEL);
#endif

	// sort special clients last
	if ( ca->sess.spectatorState == SPECTATOR_SCOREBOARD || ca->sess.spectatorClient < 0 ) {
		return 1;
	}
	if ( cb->sess.spectatorState == SPECTATOR_SCOREBOARD || cb->sess.spectatorClient < 0  ) {
		return -1;
	}

	// then connecting clients
	if ( ca->pers.connected == CON_CONNECTING ) {
		return 1;
	}
	if ( cb->pers.connected == CON_CONNECTING ) {
		return -1;
	}


#ifndef SMOKINGUNS
	// then spectators
	if ( ca->sess.sessionTeam == TEAM_SPECTATOR && cb->sess.sessionTeam == TEAM_SPECTATOR ) {
#else
	// then spectators (red specs and blue specs don't go in here)
	if ( ca->sess.sessionTeam == TEAM_SPECTATOR && cb->sess.sessionTeam == TEAM_SPECTATOR
		&& a_realspec && b_realspec) {
#endif
		if ( ca->sess.spectatorNum > cb->sess.spectatorNum ) {
			return -1;
		}
		if ( ca->sess.spectatorNum < cb->sess.spectatorNum ) {
			return 1;
		}
		return 0;
	}
#ifndef SMOKINGUNS
	if ( ca->sess.sessionTeam == TEAM_SPECTATOR ) {
#else
	if ( ca->sess.sessionTeam == TEAM_SPECTATOR && a_realspec) {
#endif
		return 1;
	}
#ifndef SMOKINGUNS
	if ( cb->sess.sessionTeam == TEAM_SPECTATOR ) {
#else
	if ( cb->sess.sessionTeam == TEAM_SPECTATOR && b_realspec) {
#endif
		return -1;
	}

	// if in duel, first sort by won rounds(=sess.losses)
#ifdef SMOKINGUNS
	if(g_gametype.integer == GT_DUEL){
		if( ca->sess.losses >
			cb->sess.losses ){
			return -1;
		} else if( ca->sess.losses <
			cb->sess.losses ){
			return 1;
		}

		// then by won duels this round
		if(ca->sess.wins >
			cb->sess.wins){
			return -1;

		} else if( ca->sess.wins <
			cb->sess.wins ){
			return 1;
		}
	}
#endif

	// then sort by score
	if ( ca->ps.persistant[PERS_SCORE]
		> cb->ps.persistant[PERS_SCORE] ) {
		return -1;
	}
	if ( ca->ps.persistant[PERS_SCORE]
		< cb->ps.persistant[PERS_SCORE] ) {
		return 1;
	}
	return 0;
}

/*
============
CalculateRanks

Recalculates the score ranks of all players
This will be called on every client connect, begin, disconnect, death,
and team change.
============
*/
void CalculateRanks( void ) {
	int		i;
	int		rank;
	int		score;
	int		newScore;
#ifdef SMOKINGUNS
	int		humancount;
#endif
	gclient_t	*cl;

	level.follow1 = -1;
	level.follow2 = -1;
	level.numConnectedClients = 0;
	level.numNonSpectatorClients = 0;
	level.numPlayingClients = 0;
	level.numVotingClients = 0;		// don't count bots
#ifdef SMOKINGUNS
	humancount = 0;
#endif
	for (i = 0; i < ARRAY_LEN(level.numteamVotingClients); i++)
		level.numteamVotingClients[i] = 0;

	for ( i = 0 ; i < level.maxclients ; i++ ) {
		if ( level.clients[i].pers.connected != CON_DISCONNECTED ) {
			level.sortedClients[level.numConnectedClients] = i;
			level.numConnectedClients++;

#ifndef SMOKINGUNS
			if ( level.clients[i].sess.sessionTeam != TEAM_SPECTATOR ) {
#else
			if ( !(g_entities[i].r.svFlags & SVF_BOT) )  humancount ++;

			if ( level.clients[i].sess.sessionTeam != TEAM_SPECTATOR ||
				(level.clients[i].sess.sessionTeam == TEAM_SPECTATOR &&
				!level.clients[i].realspec && g_gametype.integer == GT_DUEL)) {
#endif
				level.numNonSpectatorClients++;

				// decide if this should be auto-followed
				if ( level.clients[i].pers.connected == CON_CONNECTED ) {
					level.numPlayingClients++;
					if ( !(g_entities[i].r.svFlags & SVF_BOT) ) {
						level.numVotingClients++;
						if ( level.clients[i].sess.sessionTeam == TEAM_RED )
							level.numteamVotingClients[0]++;
						else if ( level.clients[i].sess.sessionTeam == TEAM_BLUE )
							level.numteamVotingClients[1]++;
					}
					if ( level.follow1 == -1 ) {
						level.follow1 = i;
					} else if ( level.follow2 == -1 ) {
						level.follow2 = i;
					}
				}
			}
		}
	}

	qsort( level.sortedClients, level.numConnectedClients,
		sizeof(level.sortedClients[0]), SortRanks );

	// set the rank value for all clients that are connected and not spectators
	if ( g_gametype.integer >= GT_TEAM ) {
		// in team games, rank is just the order of the teams, 0=red, 1=blue, 2=tied
		for ( i = 0;  i < level.numConnectedClients; i++ ) {
			cl = &level.clients[ level.sortedClients[i] ];
			if ( level.teamScores[TEAM_RED] == level.teamScores[TEAM_BLUE] ) {
				cl->ps.persistant[PERS_RANK] = 2;
			} else if ( level.teamScores[TEAM_RED] > level.teamScores[TEAM_BLUE] ) {
				cl->ps.persistant[PERS_RANK] = 0;
			} else {
				cl->ps.persistant[PERS_RANK] = 1;
			}
		}
	} else {
		rank = -1;
		score = 0;
		for ( i = 0;  i < level.numPlayingClients; i++ ) {
			cl = &level.clients[ level.sortedClients[i] ];
			newScore = cl->ps.persistant[PERS_SCORE];
			if ( i == 0 || newScore != score ) {
				rank = i;
				// assume we aren't tied until the next client is checked
				level.clients[ level.sortedClients[i] ].ps.persistant[PERS_RANK] = rank;
			} else {
				// we are tied with the previous client
				level.clients[ level.sortedClients[i-1] ].ps.persistant[PERS_RANK] = rank | RANK_TIED_FLAG;
				level.clients[ level.sortedClients[i] ].ps.persistant[PERS_RANK] = rank | RANK_TIED_FLAG;
			}
			score = newScore;
			if ( g_gametype.integer == GT_SINGLE_PLAYER && level.numPlayingClients == 1 ) {
				level.clients[ level.sortedClients[i] ].ps.persistant[PERS_RANK] = rank | RANK_TIED_FLAG;
			}
		}
	}

	// set the CS_SCORES1/2 configstrings, which will be visible to everyone
	if ( g_gametype.integer >= GT_TEAM ) {
		trap_SetConfigstring( CS_SCORES1, va("%i", level.teamScores[TEAM_RED] ) );
		trap_SetConfigstring( CS_SCORES2, va("%i", level.teamScores[TEAM_BLUE] ) );
	} else {
		if ( level.numConnectedClients == 0 ) {
			trap_SetConfigstring( CS_SCORES1, va("%i", SCORE_NOT_PRESENT) );
			trap_SetConfigstring( CS_SCORES2, va("%i", SCORE_NOT_PRESENT) );
		} else if ( level.numConnectedClients == 1 ) {
			trap_SetConfigstring( CS_SCORES1, va("%i", level.clients[ level.sortedClients[0] ].ps.persistant[PERS_SCORE] ) );
			trap_SetConfigstring( CS_SCORES2, va("%i", SCORE_NOT_PRESENT) );
		} else {
			trap_SetConfigstring( CS_SCORES1, va("%i", level.clients[ level.sortedClients[0] ].ps.persistant[PERS_SCORE] ) );
			trap_SetConfigstring( CS_SCORES2, va("%i", level.clients[ level.sortedClients[1] ].ps.persistant[PERS_SCORE] ) );
		}
	}

	// see if it is time to end the level
	CheckExitRules();

	// if we are at the intermission, send the new info to everyone
	if ( level.intermissiontime ) {
		SendScoreboardMessageToAllClients();
	}
#ifdef SMOKINGUNS
	g_humancount = humancount ;
#endif
}


/*
========================================================================

MAP CHANGING

========================================================================
*/

/*
========================
SendScoreboardMessageToAllClients

Do this at BeginIntermission time and whenever ranks are recalculated
due to enters/exits/forced team changes
========================
*/
void SendScoreboardMessageToAllClients( void ) {
	int		i;

	for ( i = 0 ; i < level.maxclients ; i++ ) {
		if ( level.clients[ i ].pers.connected == CON_CONNECTED ) {
			DeathmatchScoreboardMessage( g_entities + i );
		}
	}
}

/*
========================
MoveClientToIntermission

When the intermission starts, this will be called for all players.
If a new client connects, this will be called after the spawn function.
========================
*/
void MoveClientToIntermission( gentity_t *ent ) {
	// take out of follow mode if needed
#ifndef SMOKINGUNS
	if ( ent->client->sess.spectatorState == SPECTATOR_FOLLOW ) {
#else
	if ( ent->client->sess.spectatorState == SPECTATOR_FOLLOW
		|| ent->client->sess.spectatorState == SPECTATOR_CHASECAM
		|| ent->client->sess.spectatorState == SPECTATOR_FIXEDCAM ) {
#endif
		StopFollowing( ent );
	}

#ifdef SMOKINGUNS
	if(g_gametype.integer == GT_DUEL && g_maxmapparts)
		FindIntermissionPoint((rand()%g_maxmapparts)+1);
	else
		FindIntermissionPoint(0);
#else
	FindIntermissionPoint();
#endif
	// move to the spot
	VectorCopy( level.intermission_origin, ent->s.origin );
	VectorCopy( level.intermission_origin, ent->client->ps.origin );
	VectorCopy (level.intermission_angle, ent->client->ps.viewangles);
	ent->client->ps.pm_type = PM_INTERMISSION;

	// clean up powerup info
	memset( ent->client->ps.powerups, 0, sizeof(ent->client->ps.powerups) );

	ent->client->ps.eFlags = 0;
	ent->s.eFlags = 0;
	ent->s.eType = ET_GENERAL;
	ent->s.modelindex = 0;
	ent->s.loopSound = 0;
	ent->s.event = 0;
	ent->r.contents = 0;
}

/*
==================
FindIntermissionPoint

This is also used for spectator spawns
==================
*/
#ifndef SMOKINGUNS
void FindIntermissionPoint( void ) {
#else
void FindIntermissionPoint( int mappart ) {
#endif
	gentity_t	*ent, *target;
	vec3_t		dir;

	// find the intermission spot
#ifndef SMOKINGUNS
	ent = G_Find (NULL, FOFS(classname), "info_player_intermission");
#else
	if(g_gametype.integer != GT_DUEL){
		ent = G_Find (NULL, FOFS(classname), "info_player_intermission");
	} else {
		//find intermission spot for current mappart
		ent = G_Find (NULL, FOFS(classname), "info_player_intermission");

		while(1){

			if( ent == NULL){
				ent = G_Find (NULL, FOFS(classname), "info_player_intermission");
				//G_Printf("Warning: No info_player_intermission for current mappart found. Using default\n");
				break;
			}

			if(ent->mappart == mappart)
				break;

			ent = G_Find(ent, FOFS(classname), "info_player_intermission");
		}
	}
#endif

	if ( !ent ) {	// the map creator forgot to put in an intermission point...
#ifndef SMOKINGUNS
		SelectSpawnPoint ( vec3_origin, level.intermission_origin, level.intermission_angle, qfalse );
#else
		SelectSpawnPoint ( vec3_origin, level.intermission_origin, level.intermission_angle, qfalse, mappart, NULL );
#endif
	} else {
		VectorCopy (ent->s.origin, level.intermission_origin);
		VectorCopy (ent->s.angles, level.intermission_angle);
		// if it has a target, look towards it
		if ( ent->target ) {
			target = G_PickTarget( ent->target );
			if ( target ) {
				VectorSubtract( target->s.origin, level.intermission_origin, dir );
				vectoangles( dir, level.intermission_angle );
			}
		}
	}

	// neccesary for cgame
#ifdef SMOKINGUNS
	if(ent)
		VectorCopy(level.intermission_angle, ent->s.angles);
#endif
}

/*
==================
BeginIntermission
==================
*/
void BeginIntermission( void ) {
	int			i;
	gentity_t	*client;

	if ( level.intermissiontime ) {
		return;		// already active
	}

	// if in tournement mode, change the wins / losses
#ifndef SMOKINGUNS
	if ( g_gametype.integer == GT_TOURNAMENT ) {
		AdjustTournamentScores();
	}
#endif

	level.intermissiontime = level.time;
	// move all clients to the intermission point
	for (i=0 ; i< level.maxclients ; i++) {
		client = g_entities + i;
		if (!client->inuse)
			continue;
		// respawn if dead
		if (client->health <= 0) {
			ClientRespawn(client);
		}
		MoveClientToIntermission( client );
	}
#ifndef SMOKINGUNS
#ifdef MISSIONPACK
	if (g_singlePlayer.integer) {
		trap_Cvar_Set("ui_singlePlayerActive", "0");
		UpdateTournamentInfo();
	}
#else
	// if single player game
	if ( g_gametype.integer == GT_SINGLE_PLAYER ) {
		UpdateTournamentInfo();
		SpawnModelsOnVictoryPads();
	}
#endif
#endif

	// send the current scoring to all clients
	SendScoreboardMessageToAllClients();

}


/*
=============
ExitLevel

When the intermission has been exited, the server is either killed
or moved to a new level based on the "nextmap" cvar

=============
*/
#ifdef SMOKINGUNS
void ClearMedals(void);
void ClearDuelData(qboolean all);
#endif
void ExitLevel (void) {
	int		i;
	gclient_t *cl;
#ifndef SMOKINGUNS
	char nextmap[MAX_STRING_CHARS];
	char d1[MAX_STRING_CHARS];
#endif

	//bot interbreeding
	BotInterbreedEndMatch();

	// if we are running a tournement map, kick the loser to spectator status,
	// which will automatically grab the next spectator and restart
#ifndef SMOKINGUNS
	if ( g_gametype.integer == GT_TOURNAMENT  ) {
		if ( !level.restarted ) {
			RemoveTournamentLoser();
			trap_SendConsoleCommand( EXEC_APPEND, "map_restart 0\n" );
			level.restarted = qtrue;
			level.changemap = NULL;
			level.intermissiontime = 0;
		}
		return;
	}

	trap_Cvar_VariableStringBuffer( "nextmap", nextmap, sizeof(nextmap) );
	trap_Cvar_VariableStringBuffer( "d1", d1, sizeof(d1) );

	if( !Q_stricmp( nextmap, "map_restart 0" ) && Q_stricmp( d1, "" ) ) {
		trap_Cvar_Set( "nextmap", "vstr d2" );
		trap_SendConsoleCommand( EXEC_APPEND, "vstr d1\n" );
	} else {
		trap_SendConsoleCommand( EXEC_APPEND, "vstr nextmap\n" );
	}
#else
	trap_SendConsoleCommand( EXEC_APPEND, "vstr nextmap\n" );
#endif

	level.changemap = NULL;
	level.intermissiontime = 0;

	// reset all the scores so we don't enter the intermission again
	level.teamScores[TEAM_RED] = 0;
	level.teamScores[TEAM_BLUE] = 0;
	for ( i=0 ; i< g_maxclients.integer ; i++ ) {
		cl = level.clients + i;
		if ( cl->pers.connected != CON_CONNECTED ) {
			continue;
		}
		cl->ps.persistant[PERS_SCORE] = 0;
#ifdef SMOKINGUNS
		if(g_gametype.integer == GT_DUEL)
			cl->sess.losses = 0;
#endif
	}

	// if we're playing duel remove duel stats and spawn them again
#ifdef SMOKINGUNS
	if(g_gametype.integer == GT_DUEL){
		ClearMedals();
		ClearDuelData(qtrue);
	}
#endif

	// we need to do this here before changing to CON_CONNECTING
	G_WriteSessionData();

	// change all client states to connecting, so the early players into the
	// next level will know the others aren't done reconnecting
	for (i=0 ; i< g_maxclients.integer ; i++) {
		if ( level.clients[i].pers.connected == CON_CONNECTED ) {
			level.clients[i].pers.connected = CON_CONNECTING;
		}
	}

}

/*
=================
G_LogPrintf

Print to the logfile with a time stamp if it is open
=================
*/
void QDECL G_LogPrintf( const char *fmt, ... ) {
	va_list		argptr;
	char		string[1024];
	int			min, tens, sec;

	sec = ( level.time - level.startTime ) / 1000;

	min = sec / 60;
	sec -= min * 60;
	tens = sec / 10;
	sec -= tens * 10;

	Com_sprintf( string, sizeof(string), "%3i:%i%i ", min, tens, sec );

	va_start( argptr, fmt );
	Q_vsnprintf(string + 7, sizeof(string) - 7, fmt, argptr);
	va_end( argptr );

	if ( g_dedicated.integer ) {
		G_Printf( "%s", string + 7 );
	}

	if ( !level.logFile ) {
		return;
	}

	trap_FS_Write( string, strlen( string ), level.logFile );
}

/*
================
LogExit

Append information about this game to the log file
================
*/
void LogExit( const char *string ) {
	int				i, numSorted;
	gclient_t		*cl;
	qtime_t q;
#ifndef SMOKINGUNS
	qboolean won = qtrue;
#endif
	G_LogPrintf( "Exit: %s\n", string );

	level.intermissionQueued = level.time;

	// this will keep the clients from playing any voice sounds
	// that will get cut off when the queued intermission starts
	trap_SetConfigstring( CS_INTERMISSION, "1" );

	// don't send more than 32 scores (FIXME?)
	numSorted = level.numConnectedClients;
	if ( numSorted > 32 ) {
		numSorted = 32;
	}

	if ( g_gametype.integer >= GT_TEAM ) {
		G_LogPrintf( "red:%i  blue:%i\n",
			level.teamScores[TEAM_RED], level.teamScores[TEAM_BLUE] );
	}

	trap_RealTime(&q);

	for (i=0 ; i < numSorted ; i++) {
		int		ping;

		cl = &level.clients[level.sortedClients[i]];

		if ( cl->sess.sessionTeam == TEAM_SPECTATOR ) {
			continue;
		}
		if ( cl->pers.connected == CON_CONNECTING ) {
			continue;
		}

		ping = cl->ps.ping < 999 ? cl->ps.ping : 999;

		// TheDoctor: patch demo: extended score log entry for demo recording
		G_LogPrintf( "score: %i  ping: %i  client: %i  date: %s  name: %s\n", cl->ps.persistant[PERS_SCORE], ping, level.sortedClients[i], va("%02i.%02i.%04i %02i:%02i", q.tm_mday, q.tm_mon+1, 1900+q.tm_year,q.tm_hour,q.tm_min), cl->pers.netname );
#ifndef SMOKINGUNS
		if (g_singlePlayer.integer && g_gametype.integer == GT_TOURNAMENT) {
			if (g_entities[cl - level.clients].r.svFlags & SVF_BOT && cl->ps.persistant[PERS_RANK] == 0) {
				won = qfalse;
			}
		}
#endif

	}

#ifndef SMOKINGUNS
	if (g_singlePlayer.integer) {
		if (g_gametype.integer >= GT_CTF) {
			won = level.teamScores[TEAM_RED] > level.teamScores[TEAM_BLUE];
		}
		trap_SendConsoleCommand( EXEC_APPEND, (won) ? "spWin\n" : "spLose\n" );
	}
#endif


}


/*
=================
CheckIntermissionExit

The level will stay at the intermission for a minimum of 5 seconds
If all players wish to continue, the level will then exit.
If one or more players have not acknowledged the continue, the game will
wait 10 seconds before going on.
=================
*/
void CheckIntermissionExit( void ) {
	int			ready, notReady, playerCount;
	int			i;
	gclient_t	*cl;
	int			readyMask;

	if ( g_gametype.integer == GT_SINGLE_PLAYER ) {
		return;
	}

	// see which players are ready
	ready = 0;
	notReady = 0;
	readyMask = 0;
	playerCount = 0;
	for (i=0 ; i< g_maxclients.integer ; i++) {
		cl = level.clients + i;
		if ( cl->pers.connected != CON_CONNECTED ) {
			continue;
		}
		if ( g_entities[i].r.svFlags & SVF_BOT ) {
			continue;
		}

		playerCount++;
		if ( cl->readyToExit ) {
			ready++;
			if ( i < 16 ) {
				readyMask |= 1 << i;
			}
		} else {
			notReady++;
		}
	}

	// copy the readyMask to each player's stats so
	// it can be displayed on the scoreboard
	for (i=0 ; i< g_maxclients.integer ; i++) {
		cl = level.clients + i;
		if ( cl->pers.connected != CON_CONNECTED ) {
			continue;
		}
		cl->ps.stats[STAT_CLIENTS_READY] = readyMask;
	}

	// never exit in less than five seconds
	if ( level.time < level.intermissiontime + 5000 ) {
		return;
	}

	// only test ready status when there are real players present
	if ( playerCount > 0 ) {
		// if nobody wants to go, clear timer
		if ( !ready ) {
			level.readyToExit = qfalse;
			return;
		}

		// if everyone wants to go, go now
		if ( !notReady ) {
			ExitLevel();
			return;
		}
	}

	// the first person to ready starts the ten second timeout
	if ( !level.readyToExit ) {
		level.readyToExit = qtrue;
		level.exitTime = level.time;
	}

	// if we have waited ten seconds since at least one player
	// wanted to exit, go ahead
	if ( level.time < level.exitTime + 10000 ) {
		return;
	}

	ExitLevel();
}

/*
=============
ScoreIsTied
=============
*/
qboolean ScoreIsTied( void ) {
	int		a, b;

	if ( level.numPlayingClients < 2 ) {
		return qfalse;
	}

	if ( g_gametype.integer >= GT_TEAM ) {
		return level.teamScores[TEAM_RED] == level.teamScores[TEAM_BLUE];
	}

	a = level.clients[level.sortedClients[0]].ps.persistant[PERS_SCORE];
	b = level.clients[level.sortedClients[1]].ps.persistant[PERS_SCORE];

	return a == b;
}

/*
=================
CheckExitRules

There will be a delay between the time the exit is qualified for
and the time everyone is moved to the intermission spot, so you
can see the last frag.
=================
*/
void CheckExitRules( void ) {
 	int			i;
	gclient_t	*cl;
	// if at the intermission, wait for all non-bots to
	// signal ready, then go to next level
	if ( level.intermissiontime ) {
		CheckIntermissionExit ();
		return;
	}

	if ( level.intermissionQueued ) {
#ifndef SMOKINGUNS
		int time = (g_singlePlayer.integer) ? SP_INTERMISSION_DELAY_TIME : INTERMISSION_DELAY_TIME;
		if ( level.time - level.intermissionQueued >= time ) {
			level.intermissionQueued = 0;
			BeginIntermission();
		}
#else
		if ( level.time - level.intermissionQueued >= INTERMISSION_DELAY_TIME ) {
			level.intermissionQueued = 0;
			BeginIntermission();
		}
#endif
		return;
	}

	// check for sudden death
	if ( ScoreIsTied() ) {
		// always wait for sudden death
		return;
	}

#ifndef SMOKINGUNS
	if ( g_timelimit.integer && !level.warmupTime ) {
#else
	if ( g_timelimit.integer && (!level.warmupTime || g_gametype.integer >= GT_RTP)) {
#endif
		
		if ( ( level.time - level.startTime >= g_timelimit.integer * 60000 ) 
			// Joe Kari: no Timelimit if the round has already begun (more than 15 seconds)
			&& ( g_gametype.integer < GT_RTP || level.time < g_roundstarttime + 15000 || level.time > g_roundendtime ) )
		{
			trap_SendServerCommand( -1, "print \"Timelimit hit.\n\"");
			PushMinilog( "TIMELIMIT:" ) ;
			LogExit( "Timelimit hit." );
			return;
		}
	}

#ifndef SMOKINGUNS
	if ( g_gametype.integer < GT_CTF && g_fraglimit.integer ) {
		if ( level.teamScores[TEAM_RED] >= g_fraglimit.integer ) {
			trap_SendServerCommand( -1, "print \"Red hit the fraglimit.\n\"" );
			LogExit( "Fraglimit hit." );
			return;
		}

		if ( level.teamScores[TEAM_BLUE] >= g_fraglimit.integer ) {
			trap_SendServerCommand( -1, "print \"Blue hit the fraglimit.\n\"" );
			LogExit( "Fraglimit hit." );
			return;
		}
#else
	if(g_gametype.integer == GT_DUEL && g_duellimit.integer){

		for ( i=0 ; i< g_maxclients.integer ; i++ ) {

			cl = level.clients + i;
			if ( cl->pers.connected != CON_CONNECTED ) {
				continue;
			}

			if(cl->sess.sessionTeam != TEAM_FREE && cl->realspec)
				continue;

			if ( cl->sess.losses >= g_duellimit.integer ) {
				LogExit( "Duellimit hit." );
				PushMinilogf( "DUELLIMIT: %i" , i ) ;
				trap_SendServerCommand( -1, va("print \"%s" S_COLOR_WHITE " hit the duellimit.\n\"",
					cl->pers.netname ) );
				return;
			}
		}
	}

	if ( g_gametype.integer < GT_RTP && g_gametype.integer != GT_DUEL &&
		g_fraglimit.integer ) {
		if ( level.teamScores[TEAM_RED] >= g_fraglimit.integer ) {
			trap_SendServerCommand( -1, va("print \"%s won the game.\n\"", g_redteam.string) );
			PushMinilog( "TEAMFRAGLIMIT: lawmen" ) ;
			LogExit( "Fraglimit hit." );
			return;
		}

		if ( level.teamScores[TEAM_BLUE] >= g_fraglimit.integer ) {
			trap_SendServerCommand( -1, va("print \"%s won the game.\n\"", g_blueteam.string) );
			PushMinilog( "TEAMFRAGLIMIT: outlaws" ) ;
			LogExit( "Fraglimit hit." );
			return;
		}
#endif

		for ( i=0 ; i< g_maxclients.integer ; i++ ) {
			cl = level.clients + i;
			if ( cl->pers.connected != CON_CONNECTED ) {
				continue;
			}
			if ( cl->sess.sessionTeam != TEAM_FREE ) {
				continue;
			}

			if ( cl->ps.persistant[PERS_SCORE] >= g_fraglimit.integer ) {
				LogExit( "Fraglimit hit." );
				PushMinilogf( "FRAGLIMIT: %i" , i ) ;
				trap_SendServerCommand( -1, va("print \"%s" S_COLOR_WHITE " hit the fraglimit.\n\"",
					cl->pers.netname ) );
				return;
			}
		}
	}

#ifndef SMOKINGUNS
	if ( g_gametype.integer >= GT_CTF && g_capturelimit.integer ) {

		if ( level.teamScores[TEAM_RED] >= g_capturelimit.integer ) {
			trap_SendServerCommand( -1, "print \"Red hit the capturelimit.\n\"" );
			LogExit( "Capturelimit hit." );
			return;
		}

		if ( level.teamScores[TEAM_BLUE] >= g_capturelimit.integer ) {
			trap_SendServerCommand( -1, "print \"Blue hit the capturelimit.\n\"" );
			LogExit( "Capturelimit hit." );
			return;
		}
	}
#else
	if ( g_gametype.integer >= GT_RTP && g_scorelimit.integer ) {

		if ( level.teamScores[TEAM_RED] >= g_scorelimit.integer ) {
			char	str[64];

			PushMinilog( "SCORELIMIT: lawmen" ) ;
			Com_sprintf(str, sizeof(str), "print \"%s hit the scorelimit.\n\"",
				g_redteam.string);

			trap_SendServerCommand( -1, str );
			LogExit( "Scorelimit hit." );
			return;
		}

		if ( level.teamScores[TEAM_BLUE] >= g_scorelimit.integer ) {
			char str[64];

			PushMinilog( "SCORELIMIT: outlaws" ) ;
			Com_sprintf(str, sizeof(str), "print \"%s hit the scorelimit.\n\"",
				g_blueteam.string);

			trap_SendServerCommand( -1, str );
			LogExit( "Scorelimit hit." );
			return;
		}


	}
#endif
}



/*
========================================================================

FUNCTIONS CALLED EVERY FRAME

========================================================================
*/

#ifdef SMOKINGUNS
int			countdown;

/*
==============
ClearItems by Spoon
==============
*/
void SP_func_breakable (gentity_t *ent);
void SP_func_door (gentity_t *ent);
void SP_func_door_rotating (gentity_t *ent);

#define DOOR_RETURN 8
#define TRIGGER_DOOR 16

void ClearItems(void){
	int i;
	gentity_t	*te;

	// clear fire-entities in cgame
	te = G_TempEntity( vec3_origin, EV_ROUND_START );
	te->r.svFlags |= SVF_BROADCAST;

	for(i = 0; i < MAX_GENTITIES; i++){
		gentity_t *ent = &g_entities[i];

		// items
		if(g_entities[i].item){
			if((ent->flags & FL_DROPPED_ITEM)||
				(ent->flags & FL_THROWN_ITEM)){
				G_FreeEntity(ent);
			} else {
				RespawnItem(ent);
			}
		}

		// also remove missiles (flying/firing dynamites/molotov-drops)
		if(ent->s.eType == ET_MISSILE){

			if(ent->methodOfDeath == MOD_DYNAMITE)
				G_FreeEntity(ent);
			if(ent->s.weapon == WP_MOLOTOV)
				G_FreeEntity(ent);
		}

		if(!Q_stricmp(ent->classname, "whiskey_pool")){
			G_FreeEntity(ent);
		}

		// sound
		if(ent->flags & EF_SOUNDOFF){
			ent->s.loopSound = ent->noise_index;	// start it
			ent->flags &= ~EF_SOUNDOFF;
		}

		// gatling gun
		if(ent->s.eType == ET_TURRET){
			G_FreeEntity(ent);
		}

		// doors , pay attention to the funcs:
		//		don't try to initialize them "too" much after the first spawn
		if(!Q_stricmp(ent->classname, "func_door") &&
			!(ent->spawnflags & DOOR_RETURN)){
			SP_func_door (ent);
		}

		if(!Q_stricmp(ent->classname, "func_door_rotating") &&
			!(ent->spawnflags & DOOR_RETURN)){
			VectorCopy(ent->pos1, ent->s.apos.trBase);
			SP_func_door_rotating( ent );
		}

		// breakables
		if(ent->flags & EF_BROKEN){
			VectorCopy(ent->pos1, ent->s.origin);
			SP_func_breakable (ent);
		}
		if(!g_round){
			if(ent->s.eType == ET_BREAKABLE)
				ent->s.angles2[0] = g_entities[i].health;
		}

		// reset spawn points in duel
		if(g_gametype.integer == GT_DUEL){
			if(!Q_stricmp(ent->classname, "info_player_deathmatch"))
				ent->used = qfalse;
		}
	}
}

//SetSpawnPos by Spoon

void SetSpawnPos(int *teamblue, int *teamred) {
	int i1 = rand()% sg_rtppoints.integer;
	int i2 = rand()% sg_rtppoints.integer;

	if(g_gametype.integer == GT_RTP){
		while(i1 == i2){
			i2 = rand() % sg_rtppoints.integer;
		}

		if(sg_rtppoints.integer ==2){
			switch(g_round % 2){
			case 0:
				i1 = 0;
				i2 = 1;
				break;
			default:
			case 1:
				i1 = 1;
				i2 = 0;
				break;
			}
		}

		*teamblue = i1;
		*teamred  = i2;

	} else if(g_gametype.integer == GT_BR){
		if(!g_robteam && !g_defendteam){
			switch ( br_teamrole.integer ) {
				case 2:
					// team red always defend
					g_defendteam = TEAM_RED;
					g_robteam = TEAM_BLUE;
					break;
				case 3:
					// team blue always defend
					g_defendteam = TEAM_BLUE;
					g_robteam = TEAM_RED;
					break;
				case 1:
				case 4:
				default:
					g_robteam = (rand()%2)+1;
					g_defendteam = g_robteam == TEAM_RED ? TEAM_BLUE : TEAM_RED;
					break;
			}
		}
	}
}

/*
===============
Check_Gatling
when player is spawned in RTP and used
Gatling before or bought it give it to him
and release the "USE_GATLING"-flag"
===============
*/
static void Check_Gatling(playerState_t	*ps){
	//player uses gatling -> player gets gatling
	if(ps->weapon == WP_GATLING && ps->stats[STAT_GATLING_MODE]){
		ps->stats[STAT_GATLING_MODE] = 0;

	//player built up gatling -> player gets gatling if gatling was unused
	} else if( ps->weapon != WP_GATLING){
		int i;

		for( i=0; i<MAX_GENTITIES; i++){

			if(g_entities[i].s.eType == ET_TURRET && g_entities[i].s.torsoAnim == ps->clientNum
				&& g_entities[i].s.eventParm == -1) {

				if(!BG_FindPlayerWeapon(WP_WINCHESTER66, WP_DYNAMITE, ps))
					ps->stats[STAT_WEAPONS] |= (1 << WP_GATLING);

				break;
			}
		}
	}
}

void Setup_NewRound(void){
	int i;
	gentity_t	*te;
	int		msec;

	// time to start the next round
	msec = trap_Cvar_VariableValue("g_roundtime")*60000;
	if(msec < 0)
		msec = 4*60000;

	g_roundendtime = level.time + msec;
	g_roundstarttime = level.time;

	if(msec == 0)
		g_roundendtime = 0;

	level.nextroundstart = -1;
	level.roundNoMoveTime = (int)(g_roundNoMoveTime.value * 1000);

	G_LogPrintf( "ROUND: %i end.\n", (g_round));
	if ( g_round > 0 ) { PushMinilogf( "ENDROUND: %i", (g_round)); }

	SetSpawnPos( &sg_bluespawn, &sg_redspawn);

	for (i = 0; i < level.maxclients; i++)
	{
		gclient_t *client= &level.clients[i];

		if (client->pers.connected != CON_CONNECTED)
				continue;

		// Tequila: Be sure to release pending renaming
		if (client->pers.renameTime)
			client->pers.renameTime = level.time ;

		Check_Gatling(&client->ps);

		if (client->sess.sessionTeam == TEAM_BLUE_SPECTATOR) {
			SetTeam( &g_entities[i], "blue" );
		} else if (client->sess.sessionTeam == TEAM_RED_SPECTATOR) {
			SetTeam( &g_entities[i], "red" );
		} else if (client->sess.sessionTeam == TEAM_RED) {
			g_entities[i].client->player_died = qfalse;
			ClientSpawn( &g_entities[i] );
		} else if (client->sess.sessionTeam == TEAM_BLUE) {
			g_entities[i].client->player_died = qfalse;
			ClientSpawn( &g_entities[i] );
		} else
			continue;

		if (g_entities[i].client->ps.stats[STAT_ARMOR])
			g_entities[i].client->ps.stats[STAT_ARMOR]=BOILER_PLATE;

		g_entities[i].takedamage = qtrue;
		g_entities[i].teamchange = 0;

		if(client->sess.sessionTeam == g_robteam)
			client->ps.persistant[PERS_ROBBER] = 1;
		else
			client->ps.persistant[PERS_ROBBER] = 0;
	}

	// Set New Round:
	ClearItems();

	g_round++;

	G_LogPrintf( "ROUND: %i start.\n", (g_round));
	G_Printf ( "Round %i\n", (g_round));
	PushMinilogf( "NEWROUND: %i", (g_round));

	// takedamage countdown
	if(g_round){
		i_sNextCount = g_roundstarttime + 3000;
		countdown = 4;
	}

	g_goldescaped = qfalse;
	g_robbed = qfalse;

	te = G_TempEntity( vec3_origin, EV_ROUND_TIME );

	//play music
	if(g_round%2)
		te->s.eventParm = 0;
	else
		te->s.eventParm = 10;

	//sets cg.roundendtime
	te->s.time = g_roundstarttime;
	if(msec)
		te->s.time2 = g_roundendtime;
	te->r.svFlags |= SVF_BROADCAST;
}

static void BankRobbed( void ){
	int i;

	//robbers robbed the bank
	if(level.nextroundstart <= level.time) // only add score if round hasn't been already set up
		AddScoreRTPTeam(g_robteam, 1);

	if(g_robteam == TEAM_BLUE){
		trap_SendServerCommand( -1, va( "cp \"%s robbed the bank.\"",g_blueteam.string ) );
		G_LogPrintf( "ROUND: Won: %s\n", g_blueteam.string);
		PushMinilogf( "TEAMWON: outlaws => %i" , level.teamScores[ TEAM_BLUE ] );
	} else {
		trap_SendServerCommand( -1, va( "cp \"%s robbed the bank.\"", g_redteam.string ) );
		G_LogPrintf( "ROUND: Won: %s\n", g_redteam.string);
		PushMinilogf( "TEAMWON: lawmen => %i" , level.teamScores[ TEAM_RED ] );
	}

	//add won money
	for (i = 0; i < level.maxclients; i++){
		gclient_t *client= &level.clients[i];

		if(client->ps.stats[STAT_MONEY] < g_startingMoney.integer)
			client->ps.stats[STAT_MONEY] = g_startingMoney.integer;

		if(client->sess.sessionTeam == g_robteam ||
			client->sess.sessionTeam == g_robteam+3){

			client->ps.stats[STAT_MONEY] += m_teamwin.integer+25;
			client->pers.savedMoney += m_teamwin.integer+25;

		} else if(client->sess.sessionTeam == g_defendteam ||
			client->sess.sessionTeam == g_defendteam+3){

			client->ps.stats[STAT_MONEY] += m_teamlose.integer;
			client->pers.savedMoney += m_teamlose.integer;
		}

		if(client->ps.stats[STAT_MONEY] > g_maxMoney.integer)
				client->ps.stats[STAT_MONEY] = g_maxMoney.integer;
		if(client->pers.savedMoney > g_maxMoney.integer)
							client->pers.savedMoney = g_maxMoney.integer;
	}

	//the bank was robbed
	if(level.nextroundstart <= level.time){
		switch ( br_teamrole.integer ) {
			case 1:
				// winner attack next round
				g_robteam = g_robteam == TEAM_RED ? TEAM_RED : TEAM_BLUE;
				g_defendteam = g_robteam == TEAM_RED ? TEAM_BLUE : TEAM_RED;
				break;
			case 2:
				// team red always defend
				g_defendteam = TEAM_RED;
				g_robteam = TEAM_BLUE;
				break;
			case 3:
				// team blue always defend
				g_defendteam = TEAM_BLUE;
				g_robteam = TEAM_RED;
				break;
			case 4:
			default:
				// default behaviour: winner defend next round
				g_robteam = g_robteam == TEAM_BLUE ? TEAM_RED : TEAM_BLUE;
				g_defendteam = g_robteam == TEAM_RED ? TEAM_BLUE : TEAM_RED;
				break;
		}
	}

	// buffer time until the next round starts, set round now!!!
	if (!g_round)
		level.nextroundstart = level.time + 8000;
	else
		level.nextroundstart = level.time + 6000;

	trap_SetConfigstring( CS_WARMUP, va("%i", level.warmupTime) );
}



// countdown
void Countdown(int endtime, int step){

	if( level.time < endtime - 2*step && level.time >= endtime - 3*step){

		if(countdown == 4){
			trap_SendServerCommand( -1, "cp \"3\"" );
			countdown--;
		}
	} else if(level.time < endtime - 1*step){

		if(countdown == 3){
			trap_SendServerCommand( -1, "cp \"2\"" );
			countdown--;
		}
	} else if(level.time < endtime){

		if(countdown == 2){
			trap_SendServerCommand( -1, "cp \"1\"" );
			countdown--;
		}
	} else if(level.time < endtime + 500){

		if(countdown == 1){
			int i;

			// bank robbery
			if(g_gametype.integer == GT_BR){

				for (i = 0; i < level.maxclients; i++){
					gclient_t *client= &level.clients[i];

					if(client->sess.sessionTeam == g_robteam)
						trap_SendServerCommand( i, "cp \"Rob the Bank!\n\"" );
					else if(client->sess.sessionTeam == g_defendteam)
						trap_SendServerCommand( i, "cp \"Defend the Bank!\n\"" );
				}

			// duel
			} else if(g_gametype.integer == GT_DUEL){

				trap_SendServerCommand( -1, "cp \"DRAW!\"" );

			// normal round teamplay
			} else {

				trap_SendServerCommand( -1, "cp \"FIGHT!\"" );


			}

			for( i = 0; i < level.maxclients; i++){

				if(level.clients[i].pers.connected != CON_CONNECTED)
					continue;

				g_entities[i].takedamage = qtrue;
			}

			countdown--;
		}
	}
}

/*
==================
CheckRound by Spoon
Round Teamplay and Bank Robbery
==================
*/
void CheckRound(void){

	if (g_gametype.integer < GT_RTP)
		return;

	if(level.intermissiontime)
		return;

	if (g_doWarmup.integer != 0)
		trap_SendConsoleCommand( EXEC_INSERT, "g_doWarmup 0" );

	// if we don't have two players, go back to "waiting for players"
	if ( !(TeamCount( -1, TEAM_RED)+TeamCount( -1, TEAM_RED_SPECTATOR)) ||
		!(TeamCount( -1, TEAM_BLUE)+TeamCount( -1, TEAM_BLUE_SPECTATOR))) {
		if ( level.warmupTime != -1 ) {
			level.warmupTime = -1;
			level.nextroundstart = -1;
			trap_SetConfigstring( CS_WARMUP, va("%i", level.warmupTime) );
			G_LogPrintf( "Warmup:\n" );
		}
		return;
	}

	Countdown(i_sNextCount, 1000);

	if(g_goldescaped && g_gametype.integer == GT_BR){
		if(!g_robbed){
			BankRobbed();
			g_robbed = qtrue;
			return;
		} else if( level.time > level.nextroundstart && level.warmupTime != -1 && level.nextroundstart != -1) {
			Setup_NewRound();
			return;
		}
	}

	// check for stuff that has happened while we're playing
	if (TeamCount( -1, TEAM_RED ) == 0 || TeamCount( -1, TEAM_BLUE ) == 0 ||
		(level.time >= g_roundendtime && g_roundendtime)){
		// the round is finished
		if (level.nextroundstart == -1 && level.warmupTime != -1) {	// the round has just been won
			int loser = 0, winner = 0, i;
			qboolean tied = qfalse;

			if (TeamCount( -1, TEAM_RED ) && !TeamCount( -1, TEAM_BLUE )){
				winner = TEAM_RED;
				loser = TEAM_BLUE;
			} else if (TeamCount( -1, TEAM_BLUE ) && !TeamCount( -1, TEAM_RED )) {
				winner = TEAM_BLUE;
				loser = TEAM_RED;
			} else if ( !TeamCount( -1, TEAM_BLUE ) && !TeamCount( -1, TEAM_RED )) {
				tied = qtrue;
			} else {
				// Joe Kari: defenders in BR should win if time run out
				// and people are still alive in both team
				if (g_gametype.integer == GT_BR) {
					winner = g_defendteam ;
					loser = g_robteam ;
				} else {
					tied = qtrue;
				}
			}


			if (!tied) {	// there is someone left in the game -- they won

				gentity_t	*tent;

				AddScoreRTPTeam(winner , 1);

				if(g_defendteam != winner){
					if(winner == TEAM_BLUE){
						trap_SendServerCommand( -1, va( "cp \"%s won.\"",g_blueteam.string ) );
						G_LogPrintf( "ROUND: Won: %s\n",g_blueteam.string);
						PushMinilogf( "TEAMWON: outlaws => %i" , level.teamScores[ TEAM_BLUE ] );
					} else {
						trap_SendServerCommand( -1, va( "cp \"%s won.\"", g_redteam.string ) );
						G_LogPrintf( "ROUND: Won: %s\n", g_redteam.string);
						PushMinilogf( "TEAMWON: lawmen => %i" , level.teamScores[ TEAM_RED ] );
					}
				} else {
					if(winner == TEAM_BLUE){
						trap_SendServerCommand( -1, va( "cp \"%s defended the bank.\"",g_blueteam.string ) );
						G_LogPrintf( "ROUND: Won: %s\n", g_blueteam.string);
						PushMinilogf( "TEAMWON: outlaws => %i" , level.teamScores[ TEAM_BLUE ] );
					} else {
						trap_SendServerCommand( -1, va( "cp \"%s defended the bank.\"", g_redteam.string ) );
						G_LogPrintf( "ROUND: Won: %s\n", g_redteam.string);
						PushMinilogf( "TEAMWON: lawmen => %i" , level.teamScores[ TEAM_RED ] );
					}
				}
				if(g_round%2){
					tent = G_TempEntity( vec3_origin, EV_ROUND_TIME );
					tent->s.eventParm = winner;
					tent->r.svFlags |= SVF_BROADCAST;
				}

				//add won money
				for (i = 0; i < level.maxclients; i++){
					gclient_t *client= &level.clients[i];

					if(client->ps.stats[STAT_MONEY] < g_startingMoney.integer)
						client->ps.stats[STAT_MONEY] = g_startingMoney.integer;

					if(client->sess.sessionTeam == winner ||
						client->sess.sessionTeam == winner+3){

						client->ps.stats[STAT_MONEY] += m_teamwin.integer;
						client->pers.savedMoney += m_teamwin.integer;

					} else if(client->sess.sessionTeam == loser ||
						client->sess.sessionTeam == loser+3){

						client->ps.stats[STAT_MONEY] += m_teamlose.integer;
						client->pers.savedMoney += m_teamlose.integer;

					}

					if(client->ps.stats[STAT_MONEY] > g_maxMoney.integer)
							client->ps.stats[STAT_MONEY] = g_maxMoney.integer;
					if(client->pers.savedMoney > g_maxMoney.integer)
						client->pers.savedMoney = g_maxMoney.integer;
				}

			} else {

				if(level.time >= g_roundendtime && g_roundendtime){
					trap_SendServerCommand( -1, "cp \"Time Out\"" );
					PushMinilog( "TIED: timeout" );
				} else {
					trap_SendServerCommand( -1, "cp \"Round was tied!\"" );
					PushMinilog( "TIED:" );
				}

				//add money
				for (i = 0; i < level.maxclients; i++){
					gclient_t *client= &level.clients[i];

					if(client->ps.stats[STAT_MONEY] < g_startingMoney.integer)
						client->ps.stats[STAT_MONEY] = g_startingMoney.integer;

					if(client->sess.sessionTeam != TEAM_SPECTATOR){

						client->ps.stats[STAT_MONEY] += m_teamlose.integer;
						client->pers.savedMoney += m_teamlose.integer;
					}

					if(client->ps.stats[STAT_MONEY] > g_maxMoney.integer)
							client->ps.stats[STAT_MONEY] = g_maxMoney.integer;
					if(client->pers.savedMoney > g_maxMoney.integer)
							client->pers.savedMoney = g_maxMoney.integer;
				}

				G_LogPrintf( "ROUND: Tied.\n" );
			}
			// buffer time until the next round starts
			if (!g_round || winner == g_robteam)
				level.nextroundstart = level.time + 8000;
			else
				level.nextroundstart = level.time + 6000;

			trap_SetConfigstring( CS_WARMUP, va("%i", level.warmupTime) );

			if(g_gametype.integer == GT_BR) {
				switch ( br_teamrole.integer ) {
					case 1:
						// winner attack next round
						g_robteam = winner == TEAM_RED ? TEAM_RED : TEAM_BLUE;
						g_defendteam = g_robteam == TEAM_RED ? TEAM_BLUE : TEAM_RED;
						break;
					case 2:
						// team red always defend
						g_defendteam = TEAM_RED;
						g_robteam = TEAM_BLUE;
						break;
					case 3:
						// team blue always defend
						g_defendteam = TEAM_BLUE;
						g_robteam = TEAM_RED;
						break;
					case 4:
						// always swap defender and robbers
						g_robteam = g_robteam == TEAM_BLUE ? TEAM_RED : TEAM_BLUE;
						g_defendteam = g_robteam == TEAM_RED ? TEAM_BLUE : TEAM_RED;
						break;
					default:
						// default behaviour: winner defend next round
						g_robteam = winner == TEAM_BLUE ? TEAM_RED : TEAM_BLUE;
						g_defendteam = g_robteam == TEAM_RED ? TEAM_BLUE : TEAM_RED;
						break;
				}
			}

		} else if(level.warmupTime == -1){
			level.warmupTime = 0;
			trap_SetConfigstring( CS_WARMUP, va("%i", level.warmupTime) );
			level.nextroundstart = level.time + 8000;
		} else if ( level.time > level.nextroundstart && level.warmupTime != -1 && level.nextroundstart != -1) {
			Setup_NewRound();
		}
	}
}



/*
////////////////////
====================
// DUEL MODE ///////
====================
////////////////////
*/
int			du_spawnlist[MAX_CLIENTS];
qboolean	du_validround;
int			du_nextroundstart;
int			du_introend;
char		intermission_names[MAX_MAPPARTS][MAX_MAPPARTS_NAME_LENGTH];
int			du_winnercount;
vec3_t		du_introangles[MAX_CLIENTS];
qboolean	du_vs_message;
qboolean	du_cl_medals;

mappart_t		du_mapparts[MAX_MAPPARTS];
clientmappart_t du_clientmapparts[MAX_CLIENTS];

void ClearClients(qboolean all){
	int i;

	for(i = 0; i < MAX_CLIENTS; i++){

		if(all){
			du_clientmapparts[i].count = 1;
			du_clientmapparts[i].won = qfalse;
			du_spawnlist[i] = -1;
		}

		if(g_entities[i].client && level.clients[i].pers.connected == CON_CONNECTED){
			g_entities[i].client->ps.stats[STAT_FLAGS] &= ~SF_DU_WON;
			g_entities[i].client->wontime = 0;
			if (all) {
				g_entities[i].client->pers.savedMoney = 0;
				g_entities[i].client->pers.savedWins = 0;
			}
		}
		du_clientmapparts[i].inuse = qfalse;
	}
}

void ClearMedals(void){
	int i;

	if(!du_cl_medals)
		return;

	for(i = 0; i < MAX_CLIENTS; i++){

		if(level.clients[i].pers.connected == CON_CONNECTED){
			g_entities[i].client->ps.stats[STAT_MONEY] = DU_MIN_MONEY;
			g_entities[i].client->ps.stats[STAT_WEAPONS] = 0;
			g_entities[i].client->ps.stats[STAT_WEAPONS] |= ( 1 << WP_REM58);
			g_entities[i].client->ps.stats[STAT_WEAPONS] |= ( 1 << WP_KNIFE);
			g_entities[i].client->ps.ammo[WP_KNIFE] = -1;
			g_entities[i].client->ps.stats[STAT_FLAGS] &= ~SF_SEC_PISTOL;

			g_entities[i].client->ps.stats[STAT_WINS] = 0;
			g_entities[i].client->sess.wins = 0;
			g_entities[i].client->player_died = qtrue;
			ClientUserinfoChanged(i);
		}
	}

	du_cl_medals = qfalse;
}

void ClearDuelData(qboolean all){
	int i;

	for(i = 0; i < MAX_MAPPARTS; i++){
		du_mapparts[i].inuse = qfalse;
		du_mapparts[i].numplayers = 0;
		du_mapparts[i].players[0] = -1;
		du_mapparts[i].players[1] = -1;
		du_mapparts[i].players[2] = -1;
		du_mapparts[i].players[3] = -1;

		if(all)
			du_mapparts[i].count = 0;
	}

	ClearClients(all);
}

/*
=============
CreateSpawnlist
look for players which are spectators now
and add them to the spawn list
=============
*/
void CreateSpawnlist(void){
	int i, num;

	// first delete spawnlist
	for(num = 0; num < MAX_CLIENTS; num++){
		du_spawnlist[num] = -1;
	}

	for(i = 0, num = 0; i < level.maxclients; i++){

		if(level.clients[i].sess.sessionTeam == TEAM_SPECTATOR &&
			!level.clients[i].realspec){
			du_spawnlist[num] = i;
			num++;
		}
	}
}

/*
============
ChooseNextSpawn
chooses the next player from the spawnlist
============
*/
int ChooseNextSpawn(void){
	int	i, num = 0;
	int player;

	// search for a valid player
	while(1){
		player = du_spawnlist[num];

		// no players in spawnlist
		if(player == -1)
			return player;

		if(level.clients[player].sess.sessionTeam != TEAM_SPECTATOR ||
			level.clients[player].realspec ||
			level.clients[player].pers.connected != CON_CONNECTED){
			// this is not a valid player anymore
			num++;
		} else {
			break;
		}
	}

	// now delete the player from the spawnlist and move the others
	for(i = 0; i < MAX_CLIENTS; i++){

		if(i+1+num <= MAX_CLIENTS){
			du_spawnlist[i] = du_spawnlist[i+1+num];
		} else {
			du_spawnlist[i] = -1;
		}
	}

	return player;
}

/*
=============
SetCameraValues
set the values to the players
=============
*/
void SetCameraValues( int mappart){
	int i;

	// set the angles of the intermission point, sending them to the clients
	FindIntermissionPoint(mappart+1);

	// send the mappart info to every player
	for(i = 0; i < 4; i++){
		int num = du_mapparts[mappart].players[i];
		//gentity_t *ent = &g_entities[num];

		if(num == -1)
			continue;

		trap_SendServerCommand(num, va("cmappart %i", mappart));
	}
}

/*
==================
Count winners and remove "illegal" winners
==================
*/
int CountWinners( void ){
	int i, count = 0;

	for(i = 0; i < level.maxclients; i++){
		if(du_clientmapparts[i].won &&
			level.clients[i].pers.connected == CON_CONNECTED)
			count++;
		else
			du_clientmapparts[i].won = qfalse;
	}

	return count;
}

/*
=============
SetupSession
by Spoon
chooses player and mappart for one session
=============
*/
int SetupSession(int numplayers){
	int i;
	int mappart;
	int players[4];
	int winner = -1; // has been chosen a winner of the last round?
					// if not -1 his last mappart will be taken

	// choose the players
	// first choose the winners
	// then choose the players in the spawnlist
	// then all others
	for(i = 0; i < numplayers; i++){
		gclient_t *client;

		// search for a player
		do {
			// try to get players from the spawnlist
			if(!du_winnercount){
				players[i] = ChooseNextSpawn();

				if(players[i] == -1)
					players[i] = rand()%level.maxclients;

			} else {
				players[i] = rand()%level.maxclients;
			}

			client = g_entities[players[i]].client;

		} while(du_clientmapparts[players[i]].inuse ||
			client->pers.connected != CON_CONNECTED || client->realspec ||
			// if there are winners left -> choose em
			(!du_clientmapparts[players[i]].won && du_winnercount));

		if(du_clientmapparts[players[i]].won){

			winner = players[i];
			// decrease winnercount
			du_winnercount--;
		}

		// mark as used
		du_clientmapparts[players[i]].inuse = qtrue;
	}

	if( winner != -1 && du_clientmapparts[winner].count < 3){
		mappart = du_clientmapparts[winner].current;
	} else {
		int min = 9999;
		int count = 0;
		int mapparts[MAX_MAPPARTS];

		// search for some mapparts with the fewest uses and choose one of them
		for(i = 0; i < g_maxmapparts; i++){

			if(du_mapparts[i].inuse)
				continue;

			if(du_mapparts[i].count < min){
				mapparts[0] = i;
				count=1;
				min = du_mapparts[i].count;
				continue;
			}

			if(du_mapparts[i].count == min){
				mapparts[count] = i;
				count++;
				continue;
			}
		}

		if(count == 1)
			mappart = mapparts[0];
		else if(count > 1)
			mappart = mapparts[rand()%count];
		else // should not happen, but just in case
			return -1;
	}

	// set mappart as used
	du_mapparts[mappart].inuse = qtrue;
	du_mapparts[mappart].numplayers = numplayers;
	du_mapparts[mappart].count++;
	//G_Printf("mappart %i used\n", mappart);

	for(i = 0; i < numplayers; i++){
		// count
		if(du_clientmapparts[players[i]].current != mappart)
			du_clientmapparts[players[i]].count = 1;
		else
			du_clientmapparts[players[i]].count++;

		// set the part for the players and let them join
		du_clientmapparts[players[i]].current = mappart;
		du_mapparts[mappart].players[i] = players[i];

		// used for spawning and searching for the right spawnpoint
		g_entities[players[i]].mappart = mappart+1;

		// mark this player as a trio player
		if(numplayers == 3){
			g_entities[players[i]].client->trio = qtrue;
		} else {
			g_entities[players[i]].client->trio = qfalse;
		}
	}

	// send camera movement values to cgame
	SetCameraValues(mappart);

	return mappart;
}

/*
=============
SetupNextRound
=============
*/
void SetupNextRound(void){
	int i;
	int sessions = 0; // number of duel sessions that will be running
	int players;
	int triples = 0; // number of triple duels
	gentity_t *tent;

	// set up the next round
	ClearDuelData(qfalse);
	du_winnercount = CountWinners();
	players = TeamCount( -1, TEAM_SPECTATOR) + TeamCount(-1, TEAM_FREE);

	if(!du_forcetrio.integer){
		if(players % 2) {
			sessions = (players-1)/2;
			// one triple duel will be played
			if(du_enabletrio.integer){
				triples = 1;
			}
		} else {
			sessions = players/2;
		}
	} else {
		// force triple duels
		if(players % 3){
			// two normal duels have to be played
			if((players % 3) == 1){
				sessions = (players-1)/3 + 1;
				triples = sessions - 2;
			// one normal duel has to be played
			} else {
				sessions = (players-2)/3 + 1;
				triples = sessions -1;
			}
		} else {
			sessions = players/3;
			triples = sessions;
		}
	}

	if(sessions > g_maxmapparts)
		sessions = g_maxmapparts;

	for(i = 0; i < sessions; i++){
		int numplayers;

		if(triples){
			numplayers = 3;
			triples--;
		} else {
			numplayers = 2;
		}

		// set up the session
		SetupSession(numplayers);
	}

	tent = G_TempEntity(vec3_origin, EV_DUEL_INTRO);
	tent->s.time = du_nextroundstart;
	tent->s.time2 = g_round*20+g_session;
	tent->r.svFlags |= SVF_BROADCAST;

	// player can't move till then
	du_introend = du_nextroundstart + DU_INTRO_CAM + DU_INTRO_DRAW;
}

int Du_UsedMapparts( void ){
	int i, count = 0;

	for(i = 0; i < g_maxmapparts; i++){

		if(du_mapparts[i].inuse){
			count++;
		}
	}
	return count;
}

#define DU_VS_MESSAGE 2000
void Du_PrintVSMessage( void ) {
	if(du_introend - DU_INTRO_CAM - DU_INTRO_DRAW + DU_VS_MESSAGE <= level.time &&
		!du_vs_message){
		int i;

		du_vs_message = qtrue;

		for(i = 0; i < g_maxmapparts; i++){
			int j;
			char players[4][32];

			if(!du_mapparts[i].inuse)
				continue;


			for(j = 0; j < du_mapparts[i].numplayers; j++){
				int player = du_mapparts[i].players[j];

				strcpy(players[j], level.clients[player].pers.netname);

			}

			for(j = 0; j < du_mapparts[i].numplayers; j++){
				int player = du_mapparts[i].players[j];

				switch(du_mapparts[i].numplayers){
				case 2:
					trap_SendServerCommand( player, va("cp \"%s vs %s\"", players[0], players[1]) );
					break;
				case 3:
					trap_SendServerCommand( player, va("cp \"%s vs %s vs %s\"", players[0], players[1], players[2]) );
					break;
				case 4:
					trap_SendServerCommand( player, va("cp \"%s & %s vs %s & %s\"", players[0], players[1], players[2], players[3]) );
					break;
				default:
					G_Printf("Error: can't get names of the players :(\n");
					break;
				}

			}
		}
	}
}

#define DU_SETUPTIME	3000
int du_setuptime;

/*
=============
CheckDuel
by Spoon
=============
*/
void CheckDuel( void ) {
	int i, j;

	if (g_gametype.integer != GT_DUEL)
		return;

	if(level.intermissiontime)
		return;

	// give the permission to winspec after having got the medal
	for(i=0; i < level.maxclients; i++){
		gclient_t *client = &level.clients[i];

		if(level.clients[i].pers.connected != CON_CONNECTED)
			continue;

		if(client->wontime + MEDAL_TIME < level.time && client->wontime){
			client->ps.stats[STAT_FLAGS] |= SF_DU_WON;
			client->wontime = 0;
		}
	}

	// if we don't have two players, go back to "waiting for players"
	if ( (TeamCount( -1, TEAM_FREE) +
		TeamCount( -1, TEAM_SPECTATOR)) < 2) {
		if ( level.warmupTime != -1 ) {
			du_validround = qfalse;
			du_nextroundstart = 0;
			du_setuptime = 0;

			level.warmupTime = -1;
			trap_SetConfigstring( CS_WARMUP, va("%i", level.warmupTime) );
			G_LogPrintf( "Warmup:\n" );
		}
		return;
	}

	// prepare first round if enough players
	if(!du_validround && !du_nextroundstart && !du_setuptime){
		du_nextroundstart = level.time + 8000;
		du_setuptime = du_nextroundstart - DU_SETUPTIME;
		ClearClients(qtrue);

		level.warmupTime = 0;
		trap_SetConfigstring( CS_WARMUP, va("%i", level.warmupTime) );
		return;
	}

	// set the clients a bit later
	if(!du_validround && du_setuptime && du_setuptime <= level.time){
		SetupNextRound();
		du_setuptime = 0;
	}

	// show who is playing against who
	Du_PrintVSMessage();

	// if no round is running pick some players and get it started
	if(!du_validround && du_nextroundstart <= level.time && du_nextroundstart){
		int lastmappart=0;
		qboolean newround = du_cl_medals; // player's inventory will be reset

		ClearMedals();

		du_nextroundstart = 0;
		du_validround = qtrue;
		du_vs_message = qfalse;
		countdown = 4;	// set countdown

		g_session++;
		G_Printf("Duel: %i\n", g_session);
		ClearItems();

		// let the players join the game
		for(i = 0; i < g_maxmapparts; i++){

			if(!du_mapparts[i].inuse)
				continue;

			lastmappart = i;

			//G_Printf("---\nmappart %i\n", i);

			for(j = 0; j < du_mapparts[i].numplayers; j++){
				gentity_t *ent = &g_entities[du_mapparts[i].players[j]];

				//G_Printf("player: %s\n", ent->client->pers.netname);

				if(intermission_names[i][0])
					trap_SendServerCommand( du_mapparts[i].players[j], va("cp \"%s\"", intermission_names[i]) );

				// player hasn't died
				if(ent->client->specwatch && !newround){
					ent->client->specwatch = qfalse;
					ent->client->player_died = qfalse;
				}

				if(ent->client->sess.sessionTeam == TEAM_SPECTATOR) {
					SetTeam(ent, "ffa");
				} else {
					if(!newround)
						ent->client->player_died = qfalse;
					ClientSpawn(ent);
				}

				// reset SF_DU_WON
				ent->client->ps.stats[STAT_FLAGS] &= ~SF_DU_WON;
			}

		}

		// assign all other specs
		for(j = 0; j < level.maxclients; j++){
			gclient_t *client = g_entities[j].client;

			if(client->won){
				client->won = qfalse;
				ClientUserinfoChanged(j);
			}

			if(client->sess.sessionTeam == TEAM_SPECTATOR
				&& !client->realspec){

				du_clientmapparts[j].won = qfalse;
				du_clientmapparts[j].inuse = qfalse;

				g_entities[j].mappart = lastmappart+1;
				client->specmappart = lastmappart+1;

				// needed to select a new "info_player_intermission"
				client->realspec = qtrue;
				ClientSpawn(&g_entities[j]);
				client->realspec = qfalse;
			}

			// if player was a winner, and round has been over and he's not selected
			// move him to the specs
			if(client->sess.sessionTeam == TEAM_FREE &&
				!du_clientmapparts[j].inuse && client->pers.connected == CON_CONNECTED){

				du_clientmapparts[j].won = qfalse;
				du_clientmapparts[j].inuse = qfalse;

				g_entities[j].mappart = lastmappart+1;
				client->specmappart = lastmappart+1;

				client->realspec = qtrue;
				SetTeam(&g_entities[j], "s");
				client->realspec = qfalse;
			}
		}

		// create the spawnlist for the next round
		CreateSpawnlist();
	}

	Countdown(du_introend, 1000);

	// if round is running check if important things have happened, uiuiuiui "important things", are you a asshole or what? "uiuiuiuiu" damn it!
	if(du_validround && du_nextroundstart == 0){

		// go through each mappart and see if they've finished
		for(i = 0; i < g_maxmapparts; i++){
			int count = 0;	// count fo dead players in the part
			int survivor = -1;	// clientNum of the survivor, important to know who won

			if(!du_mapparts[i].inuse)
				continue;

			// look for dead bodies (everywhere)
			for(j = 0; j < du_mapparts[i].numplayers; j++){
				int player = du_mapparts[i].players[j];

				// one more dead
				if (level.clients[player].pers.connected != CON_CONNECTED) {
					count++;
					continue;
				}

				if(level.clients[player].sess.sessionTeam == TEAM_SPECTATOR ){
					count++;
					continue;
				}

				survivor = player;

			}

			// survivor won
			if(count == du_mapparts[i].numplayers - 1){
				// disable mappart
				du_mapparts[i].inuse = qfalse;

				// send text message
				for(j = 0; j < du_mapparts[i].numplayers; j++){
					int player = du_mapparts[i].players[j];

					trap_SendServerCommand( player, va("cp \"%s won the Duel!\"",
						g_entities[survivor].client->pers.netname));

					// set loser, winner stays in the mappart
					if(player != survivor){
						du_clientmapparts[player].won = qfalse;
					} else {
						// don't let him join the specs if all duels are finished
						if(Du_UsedMapparts() && level.clients[player].pers.connected == CON_CONNECTED){
							g_entities[player].client->wontime = level.time;
						}

						du_clientmapparts[player].won = qtrue;
						level.clients[player].won = qtrue;
						g_entities[player].client->sess.wins++;
						g_entities[player].client->ps.stats[STAT_WINS]++;
					}
					ClientUserinfoChanged( player );
				}
			}
			// round was tied
			else if(count == du_mapparts[i].numplayers){
				// disable mappart
				du_mapparts[i].inuse = qfalse;

				// send text message
				for(j = 0; j < du_mapparts[i].numplayers; j++){
					int player = du_mapparts[i].players[j];

					trap_SendServerCommand( player, "cp \"Duel was tied!\"" );

					// noone stays in this part
					du_clientmapparts[player].won = qfalse;
				}
			}
		}

		// check if all mapparts have finished their duel and start next round
		if(!Du_UsedMapparts()){
			int i;
			int player = -1;
			qboolean real_winner = qtrue;

			du_nextroundstart = level.time + 5000;
			G_Printf("All duels have been finished.\n");

			// look for a player which has 5 stars
			for(i = 0; i < level.maxclients; i++){

				if(level.clients[i].pers.connected != CON_CONNECTED)
					continue;

				// this player has 5 stars
				if(level.clients[i].sess.wins == 5){
					// more than one player with 5 stars
					if(player != -1){
						real_winner = qfalse;
						break;
					}
					player = i;
				}
			}

			if(player != -1){
				g_round++;
				g_session = 0;
				G_Printf("---\nRound: %i\n", g_round);

				// only one player with 5 stars -> winner
				if(real_winner){
					trap_SendServerCommand( -1, va("cp \"%s WON THE ROUND!\"", g_entities[player].client->pers.netname));
					PushMinilogf( "WON: %i" , g_entities[player].s.number );
					// in Duel: losses mean round-wins
					level.clients[player].sess.losses++;
					ClientUserinfoChanged(player);
				} else {
					trap_SendServerCommand( -1, "cp \"ROUND WAS TIED!\"");
					PushMinilog( "TIED:" );
				}

				ClearClients(qtrue);
				du_cl_medals = qtrue;

				//next round begins later
				du_nextroundstart = level.time + 10000;
			}


			du_setuptime = du_nextroundstart - DU_SETUPTIME;
			du_validround = qfalse;
		}
	}
}
#endif

/*
=============
CheckTournament

Once a frame, check for changes in tournement player state
=============
*/
#ifndef SMOKINGUNS
void CheckTournament( void ) {
#else
void CheckMapRestart( void ) {
#endif
	// check because we run 3 game frames before calling Connect and/or ClientBegin
	// for clients on a map_restart
	if ( level.numPlayingClients == 0 ) {
		return;
	}

#ifndef SMOKINGUNS
	if ( g_gametype.integer == GT_TOURNAMENT ) {

		// pull in a spectator if needed
		if ( level.numPlayingClients < 2 ) {
			AddTournamentPlayer();
		}

		// if we don't have two players, go back to "waiting for players"
		if ( level.numPlayingClients != 2 ) {
			if ( level.warmupTime != -1 ) {
				level.warmupTime = -1;
				trap_SetConfigstring( CS_WARMUP, va("%i", level.warmupTime) );
				G_LogPrintf( "Warmup:\n" );
			}
			return;
		}

		if ( level.warmupTime == 0 ) {
			return;
		}

		// if the warmup is changed at the console, restart it
		if ( g_warmup.modificationCount != level.warmupModificationCount ) {
			level.warmupModificationCount = g_warmup.modificationCount;
			level.warmupTime = -1;
		}

		// if all players have arrived, start the countdown
		if ( level.warmupTime < 0 ) {
			if ( level.numPlayingClients == 2 ) {
				// fudge by -1 to account for extra delays
				if ( g_warmup.integer > 1 ) {
					level.warmupTime = level.time + ( g_warmup.integer - 1 ) * 1000;
				} else {
					level.warmupTime = 0;
				}

				trap_SetConfigstring( CS_WARMUP, va("%i", level.warmupTime) );
			}
			return;
		}

		// if the warmup time has counted down, restart
		if ( level.time > level.warmupTime ) {
			level.warmupTime += 10000;
			trap_Cvar_Set( "g_restarted", "1" );
			trap_SendConsoleCommand( EXEC_APPEND, "map_restart 0\n" );
			level.restarted = qtrue;
			return;
		}
	} else if ( g_gametype.integer != GT_SINGLE_PLAYER && level.warmupTime != 0 ) {
#else
	if ( g_gametype.integer != GT_SINGLE_PLAYER && level.warmupTime != 0 && g_gametype.integer <GT_RTP &&
		g_gametype.integer != GT_DUEL) {
#endif
		int		counts[TEAM_NUM_TEAMS];
		qboolean	notEnough = qfalse;

		if ( g_gametype.integer > GT_TEAM ) {
			counts[TEAM_BLUE] = TeamCount( -1, TEAM_BLUE );
			counts[TEAM_RED] = TeamCount( -1, TEAM_RED );

			if (counts[TEAM_RED] < 1 || counts[TEAM_BLUE] < 1) {
				notEnough = qtrue;
			}
		} else if ( level.numPlayingClients < 2 ) {
			notEnough = qtrue;
		}

		if ( notEnough ) {
			if ( level.warmupTime != -1 ) {
				level.warmupTime = -1;
				trap_SetConfigstring( CS_WARMUP, va("%i", level.warmupTime) );
				G_LogPrintf( "Warmup:\n" );
			}
			return; // still waiting for team members
		}

		if ( level.warmupTime == 0 ) {
			return;
		}

		// if the warmup is changed at the console, restart it
		if ( g_warmup.modificationCount != level.warmupModificationCount ) {
			level.warmupModificationCount = g_warmup.modificationCount;
			level.warmupTime = -1;
		}

		// if all players have arrived, start the countdown
		if ( level.warmupTime < 0 ) {
			// fudge by -1 to account for extra delays
			if ( g_warmup.integer > 1 ) {
				level.warmupTime = level.time + ( g_warmup.integer - 1 ) * 1000;
			} else {
				level.warmupTime = 0;
			}

			trap_SetConfigstring( CS_WARMUP, va("%i", level.warmupTime) );
			return;
		}

		// if the warmup time has counted down, restart
		if ( level.time > level.warmupTime ) {
			level.warmupTime += 10000;
			trap_Cvar_Set( "g_restarted", "1" );
			trap_SendConsoleCommand( EXEC_APPEND, "map_restart 0\n" );
			level.restarted = qtrue;
			return;
		}
	}
}


/*
==================
CheckVote
==================
*/
void CheckVote( void ) {
	if ( level.voteExecuteTime && level.voteExecuteTime < level.time ) {
		level.voteExecuteTime = 0;
		trap_SendConsoleCommand( EXEC_APPEND, va("%s\n", level.voteString ) );
	}
	if ( !level.voteTime ) {
		return;
	}
	if ( level.time - level.voteTime >= VOTE_TIME ) {
		trap_SendServerCommand( -1, "print \"Vote failed.\n\"" );
	} else {
		// ATVI Q3 1.32 Patch #9, WNF
		if ( level.voteYes > level.numVotingClients/2 ) {
			// execute the command, then remove the vote
			trap_SendServerCommand( -1, "print \"Vote passed.\n\"" );
			level.voteExecuteTime = level.time + 3000;
		} else if ( level.voteNo >= level.numVotingClients/2 ) {
			// same behavior as a timeout
			trap_SendServerCommand( -1, "print \"Vote failed.\n\"" );
		} else {
			// still waiting for a majority
			return;
		}
	}
	level.voteTime = 0;
	trap_SetConfigstring( CS_VOTE_TIME, "" );

}

/*
==================
PrintTeam
==================
*/
void PrintTeam(int team, char *message) {
	int i;

	for ( i = 0 ; i < level.maxclients ; i++ ) {
		if (level.clients[i].sess.sessionTeam != team)
			continue;
		trap_SendServerCommand( i, message );
	}
}

/*
==================
SetLeader
==================
*/
void SetLeader(int team, int client) {
	int i;
#ifndef SMOKINGUNS

	if ( level.clients[client].pers.connected == CON_DISCONNECTED ) {
		PrintTeam(team, va("print \"%s is not connected\n\"", level.clients[client].pers.netname) );
		return;
	}
	if (level.clients[client].sess.sessionTeam != team) {
		PrintTeam(team, va("print \"%s is not on the team anymore\n\"", level.clients[client].pers.netname) );
		return;
	}
	for ( i = 0 ; i < level.maxclients ; i++ ) {
		if (level.clients[i].sess.sessionTeam != team)
			continue;
		if (level.clients[i].sess.teamLeader) {
			level.clients[i].sess.teamLeader = qfalse;
			ClientUserinfoChanged(i);
		}
	}
	level.clients[client].sess.teamLeader = qtrue;
	ClientUserinfoChanged( client );
	PrintTeam(team, va("print \"%s is the new team leader\n\"", level.clients[client].pers.netname) );
#else
	int team2;

	if(g_gametype.integer >= GT_RTP){
		if(team > TEAM_SPECTATOR)
			team2 = team - 3;
		else
			team2 = team + 3;

		if ( level.clients[client].pers.connected == CON_DISCONNECTED ) {
			PrintTeam(team, va("print \"%s is not connected\n\"", level.clients[client].pers.netname) );
			PrintTeam(team2, va("print \"%s is not connected\n\"", level.clients[client].pers.netname) );
			return;
		}
		if (level.clients[client].sess.sessionTeam != team) {
			PrintTeam(team, va("print \"%s is not on the team anymore\n\"", level.clients[client].pers.netname) );
			PrintTeam(team2, va("print \"%s is not on the team anymore\n\"", level.clients[client].pers.netname) );
			return;
		}
		for ( i = 0 ; i < level.maxclients ; i++ ) {
			if (level.clients[i].sess.sessionTeam != team &&
				level.clients[i].sess.sessionTeam != team2)
				continue;
			if (level.clients[i].sess.teamLeader) {
				level.clients[i].sess.teamLeader = qfalse;
				ClientUserinfoChanged(i);
			}
		}
		level.clients[client].sess.teamLeader = qtrue;
		ClientUserinfoChanged( client );

		//PrintTeam(team, va("print \"%s is the new team leader\n\"", level.clients[client].pers.netname) );
		//PrintTeam(team2, va("print \"%s is the new team leader\n\"", level.clients[client].pers.netname) );
	} else {
		if ( level.clients[client].pers.connected == CON_DISCONNECTED ) {
			PrintTeam(team, va("print \"%s is not connected\n\"", level.clients[client].pers.netname) );
			return;
		}
		if (level.clients[client].sess.sessionTeam != team) {
			PrintTeam(team, va("print \"%s is not on the team anymore\n\"", level.clients[client].pers.netname) );
			return;
		}
		for ( i = 0 ; i < level.maxclients ; i++ ) {
			if (level.clients[i].sess.sessionTeam != team)
				continue;
			if (level.clients[i].sess.teamLeader) {
				level.clients[i].sess.teamLeader = qfalse;
				ClientUserinfoChanged(i);
			}
		}
		level.clients[client].sess.teamLeader = qtrue;
		ClientUserinfoChanged( client );

		//PrintTeam(team, va("print \"%s is the new team leader\n\"", level.clients[client].pers.netname) );
	}
#endif
}

/*
==================
CheckTeamLeader
==================
*/
void CheckTeamLeader( int team ) {
	int i;
#ifndef SMOKINGUNS

	for ( i = 0 ; i < level.maxclients ; i++ ) {
		if (level.clients[i].sess.sessionTeam != team)
			continue;
		if (level.clients[i].sess.teamLeader)
			break;
	}
	if (i >= level.maxclients) {
		for ( i = 0 ; i < level.maxclients ; i++ ) {
			if (level.clients[i].sess.sessionTeam != team)
				continue;
			if (!(g_entities[i].r.svFlags & SVF_BOT)) {
				level.clients[i].sess.teamLeader = qtrue;
				break;
			}
		}

		if (i >= level.maxclients) {
			for ( i = 0 ; i < level.maxclients ; i++ ) {
				if (level.clients[i].sess.sessionTeam != team)
					continue;
				level.clients[i].sess.teamLeader = qtrue;
				break;
			}
		}
	}
#else
	int team2;

	if(team == TEAM_SPECTATOR)
		return;

	if(g_gametype.integer >= GT_RTP){

		if(team > TEAM_SPECTATOR)
			team2 = team - 3;
		else
			team2 = team + 3;

		for ( i = 0 ; i < level.maxclients ; i++ ) {
			if (level.clients[i].sess.sessionTeam != team &&
				level.clients[i].sess.sessionTeam != team2)
				continue;
			if (level.clients[i].sess.teamLeader)
				break;
		}
		if (i >= level.maxclients) {
			for ( i = 0 ; i < level.maxclients ; i++ ) {
				if (level.clients[i].sess.sessionTeam != team &&
					level.clients[i].sess.sessionTeam != team2)
					continue;
				if (!(g_entities[i].r.svFlags & SVF_BOT)) {
					level.clients[i].sess.teamLeader = qtrue;
					return;
				}
			}
			for ( i = 0 ; i < level.maxclients ; i++ ) {
				if (level.clients[i].sess.sessionTeam != team &&
					level.clients[i].sess.sessionTeam != team2)
					continue;
				level.clients[i].sess.teamLeader = qtrue;
				break;
			}
		}
	} else {
				for ( i = 0 ; i < level.maxclients ; i++ ) {
			if (level.clients[i].sess.sessionTeam != team)
				continue;
			if (level.clients[i].sess.teamLeader)
				break;
		}
		if (i >= level.maxclients) {
			for ( i = 0 ; i < level.maxclients ; i++ ) {
				if (level.clients[i].sess.sessionTeam != team)
					continue;
				if (!(g_entities[i].r.svFlags & SVF_BOT)) {
					level.clients[i].sess.teamLeader = qtrue;
					return;
				}
			}
			for ( i = 0 ; i < level.maxclients ; i++ ) {
				if (level.clients[i].sess.sessionTeam != team)
					continue;
				level.clients[i].sess.teamLeader = qtrue;
				break;
			}
		}
	}
#endif
}

/*
==================
CheckTeamVote
==================
*/
void CheckTeamVote( int team ) {
	int cs_offset;

	if ( team == TEAM_RED )
		cs_offset = 0;
	else if ( team == TEAM_BLUE )
		cs_offset = 1;
	else
		return;

	if ( !level.teamVoteTime[cs_offset] ) {
		return;
	}
	if ( level.time - level.teamVoteTime[cs_offset] >= VOTE_TIME ) {
		trap_SendServerCommand( -1, "print \"Team vote failed.\n\"" );
	} else {
		if ( level.teamVoteYes[cs_offset] > level.numteamVotingClients[cs_offset]/2 ) {
			// execute the command, then remove the vote
			trap_SendServerCommand( -1, "print \"Team vote passed.\n\"" );
			//
			if ( !Q_strncmp( "leader", level.teamVoteString[cs_offset], 6) ) {
				//set the team leader
				SetLeader(team, atoi(level.teamVoteString[cs_offset] + 7));
			}
			else {
				trap_SendConsoleCommand( EXEC_APPEND, va("%s\n", level.teamVoteString[cs_offset] ) );
			}
		} else if ( level.teamVoteNo[cs_offset] >= level.numteamVotingClients[cs_offset]/2 ) {
			// same behavior as a timeout
			trap_SendServerCommand( -1, "print \"Team vote failed.\n\"" );
		} else {
			// still waiting for a majority
			return;
		}
	}
	level.teamVoteTime[cs_offset] = 0;
	trap_SetConfigstring( CS_TEAMVOTE_TIME + cs_offset, "" );

}


/*
==================
CheckCvars
==================
*/
void CheckCvars( void ) {
#ifdef SMOKINGUNS
	static int g_passwordMod = -1;
	static int g_mapcyclesMod = -1;

	if ( g_password.modificationCount != g_passwordMod ) {
		g_passwordMod = g_password.modificationCount;
		if ( *g_password.string && Q_stricmp( g_password.string, "none" ) ) {
			trap_Cvar_Set( "g_needpass", "1" );
		} else {
			trap_Cvar_Set( "g_needpass", "0" );
		}
	}

	if ( g_mapcycles.modificationCount != g_mapcyclesMod ) {
		g_mapcyclesMod = g_mapcycles.modificationCount;
		trap_SetConfigstring( CS_MAPCYCLES, g_mapcycles.string );
	}
#else
	static int lastMod = -1;

	if ( g_password.modificationCount != lastMod ) {
		lastMod = g_password.modificationCount;
		if ( *g_password.string && Q_stricmp( g_password.string, "none" ) ) {
			trap_Cvar_Set( "g_needpass", "1" );
		} else {
			trap_Cvar_Set( "g_needpass", "0" );
		}
	}
#endif
}

/*
=============
G_RunThink

Runs thinking code for this frame if necessary
=============
*/
void G_RunThink (gentity_t *ent) {
	float	thinktime;

	thinktime = ent->nextthink;
	if (thinktime <= 0) {
		return;
	}
	if (thinktime > level.time) {
		return;
	}

	ent->nextthink = 0;
	if (!ent->think) {
#ifndef SMOKINGUNS
		G_Error ( "NULL ent->think");
#else
		G_Printf("Error: %s: NULL ent->think", ent->classname);
	} else
#endif
		ent->think (ent);
}

/*
================
G_UpdateTeamCount

Counts team members for the joinmenu
================
*/
#ifdef SMOKINGUNS
static void G_UpdateTeamCount(void){
	int i, redcount, bluecount;

	if(g_gametype.integer < GT_TEAM)
		return;

	for(i = 0, redcount = 0, bluecount = 0; i < level.maxclients; i++){
		if(level.clients[i].pers.connected != CON_CONNECTED)
			continue;

		if(level.clients[i].sess.sessionTeam == TEAM_BLUE ||
			level.clients[i].sess.sessionTeam == TEAM_BLUE_SPECTATOR)
			bluecount ++;
		else if(level.clients[i].sess.sessionTeam == TEAM_RED ||
			level.clients[i].sess.sessionTeam == TEAM_RED_SPECTATOR)
			redcount++;
	}

	trap_Cvar_Set("g_redteamcount", va("%i", redcount));
	trap_Cvar_Set("g_blueteamcount", va("%i", bluecount));

	trap_Cvar_Set("g_redteamscore", va("%i", level.teamScores[TEAM_RED]));
	trap_Cvar_Set("g_blueteamscore", va("%i", level.teamScores[TEAM_BLUE]));
}
#endif

/*
================
G_RunFrame

Advances the non-player objects in the world
================
*/
void G_RunFrame( int levelTime ) {
	int			i;
	gentity_t	*ent;

	// if we are waiting for the level to restart, do nothing
	if ( level.restarted ) {
		return;
	}

	level.framenum++;
	level.previousTime = level.time;
	level.time = levelTime;

	// get any cvar changes
	G_UpdateCvars();

	// update teamcount
#ifdef SMOKINGUNS
	G_UpdateTeamCount();
#endif

	//
	// go through all allocated objects
	//
	ent = &g_entities[0];
	for (i=0 ; i<level.num_entities ; i++, ent++) {
		if ( !ent->inuse ) {
			continue;
		}

		// clear events that are too old
		if ( level.time - ent->eventTime > EVENT_VALID_MSEC ) {
			if ( ent->s.event ) {
				ent->s.event = 0;	// &= EV_EVENT_BITS;
				if ( ent->client ) {
					ent->client->ps.externalEvent = 0;
					// predicted events should never be set to zero
					//ent->client->ps.events[0] = 0;
					//ent->client->ps.events[1] = 0;
				}
			}
			if ( ent->freeAfterEvent ) {
				// tempEntities or dropped items completely go away after their event
				G_FreeEntity( ent );
				continue;
			} else if ( ent->unlinkAfterEvent ) {
				// items that will respawn will hide themselves after their pickup event
				ent->unlinkAfterEvent = qfalse;
				trap_UnlinkEntity( ent );
			}
		}

		// temporary entities don't think
		if ( ent->freeAfterEvent ) {
			continue;
		}

		if ( !ent->r.linked && ent->neverFree ) {
			continue;
		}

#ifdef SMOKINGUNS
//unlagged - backward reconciliation #2
		// we'll run missiles separately to save CPU in backward reconciliation
/*
		if ( ent->s.eType == ET_MISSILE ) {
			G_RunMissile( ent );
			continue;
		}
*/
//unlagged - backward reconciliation #2
#endif

		if ( ent->s.eType == ET_ITEM || ent->physicsObject ) {
			G_RunItem( ent );
			continue;
		}

		if ( ent->s.eType == ET_MOVER ) {
			G_RunMover( ent );
			continue;
		}

		if ( i < MAX_CLIENTS ) {
			G_RunClient( ent );
			continue;
		}

		// if we have a breakable, look if it has to be respawned
#ifdef SMOKINGUNS
		if( ent->s.eType == ET_BREAKABLE &&
			ent->wait && ent->wait <= level.time &&
			(g_gametype.integer == GT_FFA || g_gametype.integer == GT_TEAM) &&
			(ent->flags & EF_BROKEN)){
			G_BreakableRespawn( ent );
		}
#endif

		G_RunThink( ent );
	}

#ifdef SMOKINGUNS
//unlagged - backward reconciliation #2
	// NOW run the missiles, with all players backward-reconciled
	// to the positions they were in exactly 50ms ago, at the end
	// of the last server frame
	G_TimeShiftAllClients( level.previousTime, NULL );

	ent = &g_entities[0];
	for (i=0 ; i<level.num_entities ; i++, ent++) {
		if ( !ent->inuse ) {
			continue;
		}

		// temporary entities don't think
		if ( ent->freeAfterEvent ) {
			continue;
		}

		if ( ent->s.eType == ET_MISSILE ) {
			G_RunMissile( ent );
		}
	}

	G_UnTimeShiftAllClients( NULL );
//unlagged - backward reconciliation #2
#endif

	// perform final fixups on the players
	ent = &g_entities[0];
	for (i=0 ; i < level.maxclients ; i++, ent++ ) {
		if ( ent->inuse ) {
			ClientEndFrame( ent );
		}
	}

#ifndef SMOKINGUNS
	// see if it is time to do a tournement restart
	CheckTournament();
#else
	// see if the round is finished
	CheckRound();

	CheckDuel();

	// see if it is time to do a map restart
	CheckMapRestart();
#endif

	// see if it is time to end the level
	CheckExitRules();

	// update to team status?
	CheckTeamStatus();

	// cancel vote if timed out
	CheckVote();

	// check team votes
	CheckTeamVote( TEAM_RED );
	CheckTeamVote( TEAM_BLUE );

	// for tracking changes
	CheckCvars();

	if (g_listEntity.integer) {
		for (i = 0; i < MAX_GENTITIES; i++) {
			G_Printf("%4i: %s\n", i, g_entities[i].classname);
		}
		trap_Cvar_Set("g_listEntity", "0");
	}

#ifdef SMOKINGUNS
//unlagged - backward reconciliation #4
	// record the time at the end of this frame - it should be about
	// the time the next frame begins - when the server starts
	// accepting commands from connected clients
	level.frameStartTime = trap_Milliseconds();
//unlagged - backward reconciliation #4
#endif
}

#ifdef SMOKINGUNS
void trap_Trace_New( trace_t *results, const vec3_t start, const vec3_t mins, const vec3_t maxs, const vec3_t end, int passEntityNum, int contentmask ) {
	int shaderNum;

	trap_Trace(results, start, mins, maxs, end, passEntityNum, contentmask);

	//added by Spoon to decompress surfaceFlags
	if((results->contents & CONTENTS_SOLID) || (results->contents & CONTENTS_PLAYERCLIP)){
		shaderNum = results->surfaceFlags;
		results->surfaceFlags = shaderInfo[shaderNum].surfaceFlags;
	}

	//G_Printf("%i %i \n", shaderNum, results->surfaceFlags);
}

int trap_Trace_New2( trace_t *results, const vec3_t start, const vec3_t mins, const vec3_t maxs, const vec3_t end, int passEntityNum, int contentmask ) {
	int shaderNum;

	trap_Trace(results, start, mins, maxs, end, passEntityNum, contentmask);

	if((results->contents & CONTENTS_SOLID) || (results->contents & CONTENTS_PLAYERCLIP)){

		//added by Spoon to decompress surfaceFlags
		shaderNum = results->surfaceFlags;
		results->surfaceFlags = shaderInfo[shaderNum].surfaceFlags;
	} else
		shaderNum= -1;

	//G_Printf("%i\n", shaderNum, results->surfaceFlags);
	return shaderNum;
}
#endif
