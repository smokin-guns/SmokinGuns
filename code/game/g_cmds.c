/*
===========================================================================
Copyright (C) 1999-2005 Id Software, Inc.
Copyright (C) 2000-2003 Iron Claw Interactive
Copyright (C) 2005-2009 Smokin' Guns

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

#include "../../ui/menudef.h"			// for the voice chats

/*
==================
DeathmatchScoreboardMessage

==================
*/
void DeathmatchScoreboardMessage( gentity_t *ent ) {
	char		entry[1024];
	char		string[1400];
	int			stringlength;
	int			i, j;
	gclient_t	*cl;
#ifndef SMOKINGUNS
	int			numSorted, scoreFlags, accuracy, perfect;
#else
	int			numSorted, scoreFlags;
#endif

	// send the latest information on all clients
	string[0] = 0;
	stringlength = 0;
	scoreFlags = 0;

	numSorted = level.numConnectedClients;

	for (i=0 ; i < numSorted ; i++) {
		int		ping;

		cl = &level.clients[level.sortedClients[i]];

		if ( cl->pers.connected == CON_CONNECTING ) {
			ping = -1;
		} else {
#ifndef SMOKINGUNS
			ping = cl->ps.ping < 999 ? cl->ps.ping : 999;
#else
//unlagged - true ping
			ping = cl->pers.realPing < 999 ? cl->pers.realPing : 999;
//unlagged - true ping
#endif
		}

#ifndef SMOKINGUNS
		if( cl->accuracy_shots ) {
			accuracy = cl->accuracy_hits * 100 / cl->accuracy_shots;
		}
		else {
			accuracy = 0;
		}
		perfect = ( cl->ps.persistant[PERS_RANK] == 0 && cl->ps.persistant[PERS_KILLED] == 0 ) ? 1 : 0;

		Com_sprintf (entry, sizeof(entry),
			" %i %i %i %i %i %i %i %i %i %i %i %i %i %i", level.sortedClients[i],
			cl->ps.persistant[PERS_SCORE], ping, (level.time - cl->pers.enterTime)/60000,
			scoreFlags, g_entities[level.sortedClients[i]].s.powerups, accuracy,
			cl->ps.persistant[PERS_IMPRESSIVE_COUNT],
			cl->ps.persistant[PERS_EXCELLENT_COUNT],
			cl->ps.persistant[PERS_GAUNTLET_FRAG_COUNT],
			cl->ps.persistant[PERS_DEFEND_COUNT],
			cl->ps.persistant[PERS_ASSIST_COUNT],
			perfect,
			cl->ps.persistant[PERS_CAPTURES]);
#else
		Com_sprintf (entry, sizeof(entry),
			" %i %i %i %i %i %i %i %i", level.sortedClients[i],
			cl->ps.persistant[PERS_SCORE], ping, (level.time - cl->pers.enterTime)/60000,
			scoreFlags, g_entities[level.sortedClients[i]].s.powerups,
			cl->realspec,
			cl->won);
#endif
		j = strlen(entry);
		if (stringlength + j > 1024)
			break;
		strcpy (string + stringlength, entry);
		stringlength += j;
	}

	trap_SendServerCommand( ent-g_entities, va("scores %i %i %i%s", i,
		level.teamScores[TEAM_RED], level.teamScores[TEAM_BLUE],
		string ) );
}


/*
==================
Cmd_Score_f

Request current scoreboard information
==================
*/
void Cmd_Score_f( gentity_t *ent ) {
	DeathmatchScoreboardMessage( ent );
}



/*
==================
CheatsOk
==================
*/
qboolean	CheatsOk( gentity_t *ent ) {
	if ( !g_cheats.integer ) {
		trap_SendServerCommand( ent-g_entities, va("print \"Cheats are not enabled on this server.\n\""));
		return qfalse;
	}
	if ( ent->health <= 0 ) {
		trap_SendServerCommand( ent-g_entities, va("print \"You must be alive to use this command.\n\""));
		return qfalse;
	}
	return qtrue;
}


/*
==================
ConcatArgs
==================
*/
char	*ConcatArgs( int start ) {
	int		i, c, tlen;
	static char	line[MAX_STRING_CHARS];
	int		len;
	char	arg[MAX_STRING_CHARS];

	len = 0;
	c = trap_Argc();
	for ( i = start ; i < c ; i++ ) {
		trap_Argv( i, arg, sizeof( arg ) );
		tlen = strlen( arg );
		if ( len + tlen >= MAX_STRING_CHARS - 1 ) {
			break;
		}
		memcpy( line + len, arg, tlen );
		len += tlen;
		if ( i != c - 1 ) {
			line[len] = ' ';
			len++;
		}
	}

	line[len] = 0;

	return line;
}

/*
==================
ClientNumberFromString

Returns a player number for either a number or name string
Returns -1 if invalid
==================
*/
int ClientNumberFromString( gentity_t *to, char *s ) {
	gclient_t	*cl;
	int			idnum;
	char		cleanName[MAX_STRING_CHARS];

	// numeric values are just slot numbers
	if (s[0] >= '0' && s[0] <= '9') {
		idnum = atoi( s );
		if ( idnum < 0 || idnum >= level.maxclients ) {
			trap_SendServerCommand( to-g_entities, va("print \"Bad client slot: %i\n\"", idnum));
			return -1;
		}

		cl = &level.clients[idnum];
		if ( cl->pers.connected != CON_CONNECTED ) {
			trap_SendServerCommand( to-g_entities, va("print \"Client %i is not active\n\"", idnum));
			return -1;
		}
		return idnum;
	}

	// check for a name match
	for ( idnum=0,cl=level.clients ; idnum < level.maxclients ; idnum++,cl++ ) {
		if ( cl->pers.connected != CON_CONNECTED ) {
			continue;
		}
		Q_strncpyz(cleanName, cl->pers.netname, sizeof(cleanName));
		Q_CleanStr(cleanName);
		if ( !Q_stricmp( cleanName, s ) ) {
			return idnum;
		}
	}

	trap_SendServerCommand( to-g_entities, va("print \"User %s is not on the server\n\"", s));
	return -1;
}

/*
==================
Cmd_Give_f

Give items to a client
==================
*/
void Cmd_Give_f (gentity_t *ent)
{
	char		*name;
	gitem_t		*it;
	int			i;
	qboolean	give_all;
	gentity_t		*it_ent;
	trace_t		trace;

	if ( !CheatsOk( ent ) ) {
		return;
	}

	name = ConcatArgs( 1 );

	if (Q_stricmp(name, "all") == 0)
		give_all = qtrue;
	else
		give_all = qfalse;

	if (give_all || Q_stricmp( name, "health") == 0)
	{
		ent->health = ent->client->ps.stats[STAT_MAX_HEALTH];
		if (!give_all)
			return;
	}

	if (give_all || Q_stricmp(name, "weapons") == 0)
	{
#ifndef SMOKINGUNS
		ent->client->ps.stats[STAT_WEAPONS] = (1 << WP_NUM_WEAPONS) - 1 -
			( 1 << WP_GRAPPLING_HOOK ) - ( 1 << WP_NONE );
#else
		ent->client->ps.stats[STAT_WEAPONS] = (1 << WP_NUM_WEAPONS) - 1 -
			( 1 << WP_NONE );
#endif
		if (!give_all)
			return;
	}

	if (give_all || Q_stricmp(name, "ammo") == 0)
	{
#ifndef SMOKINGUNS
		for ( i = 0 ; i < MAX_WEAPONS ; i++ ) {
			ent->client->ps.ammo[i] = 999;
		}
#else
		for ( i = 0 ; i < WP_NUM_WEAPONS ; i++ ) {
			if(bg_weaponlist[i].clip)
				ent->client->ps.ammo[i] = bg_weaponlist[i].clipAmmo;
			else
				ent->client->ps.ammo[i] = bg_weaponlist[i].maxAmmo;
		}
		ent->client->ps.ammo[WP_AKIMBO] = bg_weaponlist[WP_PEACEMAKER].clipAmmo;

		ent->client->ps.ammo[WP_BULLETS_CLIP] = bg_weaponlist[WP_PEACEMAKER].maxAmmo*2;
		ent->client->ps.ammo[WP_SHELLS_CLIP] = bg_weaponlist[WP_REMINGTON_GAUGE].maxAmmo*2;
		ent->client->ps.ammo[WP_CART_CLIP] = bg_weaponlist[WP_WINCHESTER66].maxAmmo*2;
		ent->client->ps.ammo[WP_GATLING_CLIP] = bg_weaponlist[WP_GATLING].maxAmmo*2;
		ent->client->ps.ammo[WP_SHARPS_CLIP] = bg_weaponlist[WP_SHARPS].maxAmmo*2;
#endif
		if (!give_all)
			return;
	}

	if (give_all || Q_stricmp(name, "armor") == 0)
	{
#ifndef SMOKINGUNS
		ent->client->ps.stats[STAT_ARMOR] = 200;
#else
		ent->client->ps.stats[STAT_ARMOR] = BOILER_PLATE;
#endif

		if (!give_all)
			return;
	}

#ifndef SMOKINGUNS
	if (Q_stricmp(name, "excellent") == 0) {
		ent->client->ps.persistant[PERS_EXCELLENT_COUNT]++;
		return;
	}
	if (Q_stricmp(name, "impressive") == 0) {
		ent->client->ps.persistant[PERS_IMPRESSIVE_COUNT]++;
		return;
	}
	if (Q_stricmp(name, "gauntletaward") == 0) {
		ent->client->ps.persistant[PERS_GAUNTLET_FRAG_COUNT]++;
		return;
	}
	if (Q_stricmp(name, "defend") == 0) {
		ent->client->ps.persistant[PERS_DEFEND_COUNT]++;
		return;
	}
	if (Q_stricmp(name, "assist") == 0) {
		ent->client->ps.persistant[PERS_ASSIST_COUNT]++;
		return;
	}
#else
	if (give_all || Q_stricmp(name, "money") == 0)
		ent->client->ps.stats[STAT_MONEY] = MAX_MONEY;

	//give powerups
	if (give_all || !Q_stricmp( name, "powerups")){
		ent->client->ps.powerups[PW_SCOPE] = 2;
		ent->client->ps.powerups[PW_BELT] = 1;
	}

	//give moneybag
	if(!Q_stricmp( name, "gold")){
		ent->client->ps.powerups[PW_GOLD] = 1;
	}
#endif

	// spawn a specific item right on the player
	if ( !give_all ) {
		it = BG_FindItem (name);
		if (!it) {
			return;
		}

		it_ent = G_Spawn();
		VectorCopy( ent->r.currentOrigin, it_ent->s.origin );
		it_ent->classname = it->classname;
		G_SpawnItem (it_ent, it);
		FinishSpawningItem(it_ent );
		memset( &trace, 0, sizeof( trace ) );
		Touch_Item (it_ent, ent, &trace);
		if (it_ent->inuse) {
			G_FreeEntity( it_ent );
		}

#ifdef SMOKINGUNS
		if(it->giType == IT_WEAPON){
			ent->client->ps.ammo[it->giTag] = bg_weaponlist[it->giTag].clipAmmo;
		}
#endif
	}
}


/*
==================
Cmd_God_f

Sets client to godmode

argv(0) god
==================
*/
void Cmd_God_f (gentity_t *ent)
{
	char	*msg;

	if ( !CheatsOk( ent ) ) {
		return;
	}

	ent->flags ^= FL_GODMODE;
	if (!(ent->flags & FL_GODMODE) )
		msg = "godmode OFF\n";
	else
		msg = "godmode ON\n";

	trap_SendServerCommand( ent-g_entities, va("print \"%s\"", msg));
}


/*
==================
Cmd_Notarget_f

Sets client to notarget

argv(0) notarget
==================
*/
void Cmd_Notarget_f( gentity_t *ent ) {
	char	*msg;

	if ( !CheatsOk( ent ) ) {
		return;
	}

	ent->flags ^= FL_NOTARGET;
	if (!(ent->flags & FL_NOTARGET) )
		msg = "notarget OFF\n";
	else
		msg = "notarget ON\n";

	trap_SendServerCommand( ent-g_entities, va("print \"%s\"", msg));
}


/*
==================
Cmd_Noclip_f

argv(0) noclip
==================
*/
void Cmd_Noclip_f( gentity_t *ent ) {
	char	*msg;

	if ( !CheatsOk( ent ) ) {
		return;
	}

	if ( ent->client->noclip ) {
		msg = "noclip OFF\n";
	} else {
		msg = "noclip ON\n";
	}
	ent->client->noclip = !ent->client->noclip;

	trap_SendServerCommand( ent-g_entities, va("print \"%s\"", msg));
}


/*
==================
Cmd_LevelShot_f

This is just to help generate the level pictures
for the menus.  It goes to the intermission immediately
and sends over a command to the client to resize the view,
hide the scoreboard, and take a special screenshot
==================
*/
void Cmd_LevelShot_f( gentity_t *ent ) {
	if ( !CheatsOk( ent ) ) {
		return;
	}

	// doesn't work in single player
#ifndef SMOKINGUNS
	if ( g_gametype.integer != 0 ) {
		trap_SendServerCommand( ent-g_entities,
			"print \"Must be in g_gametype 0 for levelshot\n\"" );
		return;
	}
#endif

	BeginIntermission();
	trap_SendServerCommand( ent-g_entities, "clientLevelShot" );
}


/*
==================
Cmd_LevelShot_f

This is just to help generate the level pictures
for the menus.  It goes to the intermission immediately
and sends over a command to the client to resize the view,
hide the scoreboard, and take a special screenshot
==================
*/
void Cmd_TeamTask_f( gentity_t *ent ) {
	char userinfo[MAX_INFO_STRING];
	char		arg[MAX_TOKEN_CHARS];
	int task;
	int client = ent->client - level.clients;

	if ( trap_Argc() != 2 ) {
		return;
	}
	trap_Argv( 1, arg, sizeof( arg ) );
	task = atoi( arg );

	trap_GetUserinfo(client, userinfo, sizeof(userinfo));
	Info_SetValueForKey(userinfo, "teamtask", va("%d", task));
	trap_SetUserinfo(client, userinfo);
	ClientUserinfoChanged(client);
}



/*
=================
Cmd_Kill_f
=================
*/
void Cmd_Kill_f( gentity_t *ent ) {
#ifndef SMOKINGUNS
	if ( ent->client->sess.sessionTeam == TEAM_SPECTATOR ) {
#else
	if ( ent->client->sess.sessionTeam >= TEAM_SPECTATOR ) {
#endif
		return;
	}
	if (ent->health <= 0) {
		return;
	}
	ent->flags &= ~FL_GODMODE;
	ent->client->ps.stats[STAT_HEALTH] = ent->health = -999;
	player_die (ent, ent, ent, 100000, MOD_SUICIDE);
}

/*
=================
BroadCastTeamChange

Let everyone know about a team change
=================
*/
void BroadcastTeamChange( gclient_t *client, int oldTeam )
{
#ifndef SMOKINGUNS
	if ( client->sess.sessionTeam == TEAM_RED ) {
		trap_SendServerCommand( -1, va("cp \"%s" S_COLOR_WHITE " joined the red team.\n\"",
			client->pers.netname) );
	} else if ( client->sess.sessionTeam == TEAM_BLUE ) {
		trap_SendServerCommand( -1, va("cp \"%s" S_COLOR_WHITE " joined the blue team.\n\"",
		client->pers.netname));
	} else if ( client->sess.sessionTeam == TEAM_SPECTATOR && oldTeam != TEAM_SPECTATOR ) {
		trap_SendServerCommand( -1, va("cp \"%s" S_COLOR_WHITE " joined the spectators.\n\"",
		client->pers.netname));
	} else if ( client->sess.sessionTeam == TEAM_FREE ) {
		trap_SendServerCommand( -1, va("cp \"%s" S_COLOR_WHITE " joined the battle.\n\"",
		client->pers.netname));
	}
#else
	if(g_gametype.integer == GT_DUEL)
		return;

	if(g_gametype.integer == GT_RTP)
		return;

	if ( client->sess.sessionTeam == TEAM_RED ) {
		trap_SendServerCommand( -1, va("cp \"%s" S_COLOR_WHITE " joined %s.\n\"",
			client->pers.netname, g_redteam.string) );
	} else if ( client->sess.sessionTeam == TEAM_BLUE ) {
		trap_SendServerCommand( -1, va("cp \"%s" S_COLOR_WHITE " joined %s.\n\"",
		client->pers.netname, g_blueteam.string));
	} else if ( client->sess.sessionTeam == TEAM_SPECTATOR && oldTeam != TEAM_SPECTATOR ) {
		trap_SendServerCommand( -1, va("cp \"%s" S_COLOR_WHITE " joined the spectators.\n\"",
		client->pers.netname));
	} else if ( client->sess.sessionTeam == TEAM_FREE ) {
		trap_SendServerCommand( -1, va("cp \"%s" S_COLOR_WHITE " joined the game.\n\"",
		client->pers.netname));
	}
#endif
}

/*
=================
SetTeam
=================
*/
void SetTeam( gentity_t *ent, char *s ) {
	int					team, oldTeam;
	gclient_t			*client;
	int					clientNum;
	spectatorState_t	specState;
	int					specClient;
	int					teamLeader;
#ifdef SMOKINGUNS
	vec3_t				viewangles;
	vec3_t				origin;
#endif

	//
	// see what change is requested
	//
	client = ent->client;

#ifdef SMOKINGUNS
	VectorCopy(ent->client->ps.viewangles, viewangles);
	VectorCopy(ent->client->ps.origin, origin);
#endif

	clientNum = client - level.clients;
	specClient = 0;
	specState = SPECTATOR_NOT;
#ifdef SMOKINGUNS
	oldTeam = client->sess.sessionTeam;
#endif
	//blue & red spectators
	if ( !Q_stricmp( s, "scoreboard" ) || !Q_stricmp( s, "score" )  ) {
		team = TEAM_SPECTATOR;
		specState = SPECTATOR_SCOREBOARD;
	} else if ( !Q_stricmp( s, "follow1" ) ) {
		team = TEAM_SPECTATOR;
		specState = SPECTATOR_FOLLOW;
		specClient = -1;
	} else if ( !Q_stricmp( s, "follow2" ) ) {
		team = TEAM_SPECTATOR;
		specState = SPECTATOR_FOLLOW;
		specClient = -2;
	} else if ( !Q_stricmp( s, "spectator" ) || !Q_stricmp( s, "s" ) ) {
		team = TEAM_SPECTATOR;
		specState = SPECTATOR_FREE;
#ifdef SMOKINGUNS
		if ( g_gametype.integer == GT_DUEL && oldTeam != TEAM_SPECTATOR ) {
			client->pers.savedMoney = client->ps.stats[STAT_MONEY];
			client->pers.savedWins = client->ps.stats[STAT_WINS];
		} else {
			client->pers.savedMoney = 0;
			client->pers.savedWins = 0;
			if (oldTeam != TEAM_SPECTATOR) {
				client->pers.savedScore = client->ps.persistant[PERS_SCORE];
			}
		}
#endif
	} else if ( g_gametype.integer >= GT_TEAM ) {
		// if running a team game, assign player to one of the teams
		specState = SPECTATOR_NOT;
		if ( !Q_stricmp( s, "red" ) || !Q_stricmp( s, "r" ) ) {
			team = TEAM_RED;
#ifdef SMOKINGUNS
			client->ps.stats[STAT_MONEY] = client->pers.savedMoney;
#endif
		} else if ( !Q_stricmp( s, "blue" ) || !Q_stricmp( s, "b" ) ) {
			team = TEAM_BLUE;
#ifdef SMOKINGUNS
			client->ps.stats[STAT_MONEY] = client->pers.savedMoney;
		}else if ( !Q_stricmp( s, "bluespec" ) || !Q_stricmp( s, "sb") ) {
			team = TEAM_BLUE_SPECTATOR;
			if ( oldTeam == TEAM_SPECTATOR ) {
				client->pers.savedMoney = 0;
			} else {
				client->pers.savedMoney = client->ps.stats[STAT_MONEY];
			}
		} else if ( !Q_stricmp( s, "redspec" ) || !Q_stricmp( s, "rs") ) {
			team = TEAM_RED_SPECTATOR;
			if ( oldTeam == TEAM_SPECTATOR ) {
				client->pers.savedMoney = 0;
			} else {
				client->pers.savedMoney = client->ps.stats[STAT_MONEY];
			}
#endif
		} else {
			// pick the team with the least number of players
			team = PickTeam( clientNum );

#ifdef SMOKINGUNS
			if ( g_gametype.integer >= GT_RTP ) {
				trap_SendServerCommand( clientNum, "print \"team: Invalid argument\n\"" );
				return;
			}
#endif
		}

		if ( g_teamForceBalance.integer  ) {
			int		counts[TEAM_NUM_TEAMS];

#ifndef SMOKINGUNS
			counts[TEAM_BLUE] = TeamCount( ent->client->ps.clientNum, TEAM_BLUE );
			counts[TEAM_RED] = TeamCount( ent->client->ps.clientNum, TEAM_RED );
#else
			if(g_gametype.integer >= GT_RTP){
				counts[TEAM_BLUE] = TeamCount( ent->client->ps.clientNum, TEAM_BLUE )+
					TeamCount( ent->client->ps.clientNum, TEAM_BLUE_SPECTATOR );
				counts[TEAM_RED] = TeamCount( ent->client->ps.clientNum, TEAM_RED )+
					TeamCount( ent->client->ps.clientNum, TEAM_RED_SPECTATOR );
			} else {
				counts[TEAM_BLUE] = TeamCount( ent->client->ps.clientNum, TEAM_BLUE );
				counts[TEAM_RED] = TeamCount( ent->client->ps.clientNum, TEAM_RED );
			}
#endif

			// We allow a spread of two
			if ( team == TEAM_RED && counts[TEAM_RED] - counts[TEAM_BLUE] > 1 ) {
#ifndef SMOKINGUNS
				trap_SendServerCommand( ent->client->ps.clientNum,
					"cp \"Red team has too many players.\n\"" );
#else
				trap_SendServerCommand( ent->client->ps.clientNum,
					va("cp \"%s has too many players.\n\"", g_redteam.string) );
#endif
				return; // ignore the request
			}
			if ( team == TEAM_BLUE && counts[TEAM_BLUE] - counts[TEAM_RED] > 1 ) {
#ifndef SMOKINGUNS
				trap_SendServerCommand( ent->client->ps.clientNum,
					"cp \"Blue team has too many players.\n\"" );
#else
				trap_SendServerCommand( ent->client->ps.clientNum,
					va("cp \"%s has too many players.\n\"", g_blueteam.string) );
#endif
				return; // ignore the request
			}

			// It's ok, the team we are switching to has less or same number of players
		}

	} else {
		// force them to spectators if there aren't any spots free
		team = TEAM_FREE;
#ifdef SMOKINGUNS
		if ( g_gametype.integer == GT_DUEL ) {
			client->ps.stats[STAT_MONEY] = client->pers.savedMoney;
			client->ps.stats[STAT_WINS] = client->pers.savedWins;
		}
#endif
	}

	// override decision if limiting the players
#ifndef SMOKINGUNS
	if ( (g_gametype.integer == GT_TOURNAMENT)
		&& level.numNonSpectatorClients >= 2 ) {
		team = TEAM_SPECTATOR;
	} else if ( g_maxGameClients.integer > 0 &&
		level.numNonSpectatorClients >= g_maxGameClients.integer ) {
		team = TEAM_SPECTATOR;
	}
#else
	if ( g_maxGameClients.integer > 0 &&
		level.numNonSpectatorClients >= g_maxGameClients.integer ) {
		team = TEAM_SPECTATOR;
	}
#endif

	//
	// decide if we will allow the change
	//
#ifndef SMOKINGUNS
	oldTeam = client->sess.sessionTeam;
#endif
	if ( team == oldTeam && team != TEAM_SPECTATOR ) {
		return;
	}

	//
	// execute the team change
	//

#ifndef SMOKINGUNS
	// if the player was dead leave the body
	if ( client->ps.stats[STAT_HEALTH] <= 0 ) {
		CopyToBodyQue(ent);
	}
#else
	if ((!ent->teamchange && client->sess.sessionTeam < TEAM_SPECTATOR) ||
		ent->client->specwatch){ //make dead bodys
		CopyToBodyQue (ent);
	}
	// if the player was dead leave the body
	else if ( client->ps.stats[STAT_HEALTH] <= 0 ) {
		CopyToBodyQue(ent);
	}
#endif

	// he starts at 'base'
	client->pers.teamState.state = TEAM_BEGIN;
#ifndef SMOKINGUNS
	if ( oldTeam != TEAM_SPECTATOR ) {
		// Kill him (makes sure he loses flags, etc)
#else
	if ( oldTeam < TEAM_SPECTATOR && !ent->client->specwatch  && ent->client->ps.stats[STAT_HEALTH] > 0) {
		ent->client->ps.persistant[PERS_SCORE] += 1; // switching team is not suicide
#endif
		ent->flags &= ~FL_GODMODE;
		ent->client->ps.stats[STAT_HEALTH] = ent->health = 0;
		player_die (ent, ent, ent, 100000, MOD_SUICIDE);

	}
	// they go to the end of the line for tournements
#ifndef SMOKINGUNS
	if ( team == TEAM_SPECTATOR ) {
#else
	if ( team >= TEAM_SPECTATOR ) {
#endif
		client->sess.spectatorTime = level.time;
	}

	client->sess.sessionTeam = team;
	client->sess.spectatorState = specState;
	client->sess.spectatorClient = specClient;

	client->sess.teamLeader = qfalse;
#ifndef SMOKINGUNS
	if ( team == TEAM_RED || team == TEAM_BLUE ) {
#else
	if ( ( team == TEAM_RED && oldTeam != TEAM_RED_SPECTATOR )
		|| (team == TEAM_BLUE && oldTeam != TEAM_BLUE_SPECTATOR)
		|| (oldTeam != TEAM_RED && team == TEAM_RED_SPECTATOR)
		|| (oldTeam != TEAM_BLUE && team == TEAM_BLUE_SPECTATOR)) {
#endif
		teamLeader = TeamLeader( team );
		// if there is no team leader or the team leader is a bot and this client is not a bot
		if ( teamLeader == -1 || ( !(g_entities[clientNum].r.svFlags & SVF_BOT) && (g_entities[teamLeader].r.svFlags & SVF_BOT) ) ) {
			SetLeader( team, clientNum );
		}
	}
	// make sure there is a team leader on the team the player came from
	if ( oldTeam == TEAM_RED || oldTeam == TEAM_BLUE ) {
#ifdef SMOKINGUNS
		if(g_gametype.integer >= GT_RTP && oldTeam == team +3){
		} else
#endif
			CheckTeamLeader( oldTeam );
	}

	BroadcastTeamChange( client, oldTeam );

	// get and distribute relevent paramters
	ClientUserinfoChanged( clientNum );

#ifndef SMOKINGUNS
	ClientBegin( clientNum );
#else
	VectorCopy(viewangles, ent->client->ps.viewangles);
	VectorCopy(origin, ent->client->ps.origin);

	if ( g_gametype.integer >= GT_RTP){
		if ( !Q_stricmp( s, "spectator" ) || !Q_stricmp( s, "s" )
			/*|| (team != oldTeam -3 && team != oldTeam+3)*/)
			ClientBegin( clientNum);
		else {
			ClientSpawn( ent );
			if (oldTeam == TEAM_SPECTATOR) {
				client->ps.persistant[PERS_SCORE] = client->pers.savedScore;
			}
		}
	} else if(g_gametype.integer == GT_DUEL){
		if((oldTeam == TEAM_SPECTATOR && team == TEAM_FREE && !client->realspec) ||
			(oldTeam == TEAM_FREE && team == TEAM_SPECTATOR && !client->realspec)){
			ClientSpawn(ent);
		} else
			ClientBegin(clientNum);
	} else {
		ClientBegin( clientNum );
		if (oldTeam == TEAM_SPECTATOR && team != TEAM_SPECTATOR) {
			client->ps.persistant[PERS_SCORE] = client->pers.savedScore;
		}
	}
#endif
}

/*
=================
StopFollowing

If the client being followed leaves the game, or you just want to drop
to free floating spectator mode
=================
*/
void StopFollowing( gentity_t *ent ) {
	ent->client->ps.persistant[ PERS_TEAM ] = TEAM_SPECTATOR;
#ifndef SMOKINGUNS
	ent->client->sess.sessionTeam = TEAM_SPECTATOR;
#endif
	ent->client->sess.spectatorState = SPECTATOR_FREE;
	ent->client->ps.pm_flags &= ~PMF_FOLLOW;
#ifndef SMOKINGUNS
	// Tequila comment: Fix by torhu a long time ago
	// Check http://www.quake3world.com/forum/viewtopic.php?f=16&t=9220
	// and http://bugzilla.icculus.org/show_bug.cgi?id=3418
	ent->r.svFlags &= ~SVF_BOT;
#endif
	ent->client->ps.clientNum = ent - g_entities;
}

#ifdef SMOKINGUNS
qboolean ChaseCam_CheckClient(int clientnum, gentity_t *ent){

	// can only follow connected clients
	if ( level.clients[ clientnum ].pers.connected != CON_CONNECTED ) {
		return qfalse;
	}

	// can't follow another spectator or enemy
	if (g_gametype.integer >= GT_RTP && ent->client->sess.sessionTeam != TEAM_SPECTATOR){
		if ( level.clients[ clientnum ].sess.sessionTeam != ent->client->sess.sessionTeam - 3 ) {
			return qfalse;
		}
	} else if (level.clients[clientnum].sess.sessionTeam >= TEAM_SPECTATOR)
		return qfalse;

	// can't follow anyone from another part
	if(g_gametype.integer == GT_DUEL && ent->client->specmappart != g_entities[clientnum].mappart)
		return qfalse;

	if (clientnum == ent - g_entities)
		return qfalse;

	if (level.clients[clientnum].ps.stats[STAT_HEALTH] <= 0)
		return qfalse;

	return qtrue;
}


/*
=================
ChaseCam_Start
by Spoon
=================
*/
void ChaseCam_Start( gentity_t *ent, int dir ) {
	int		clientnum;
	int		original;

	clientnum = ent->client->sess.spectatorClient;
	original = clientnum;

	do {

		clientnum += dir;
		if ( clientnum >= level.maxclients ) {
			clientnum = 0;
		}
		if ( clientnum < 0 ) {
			clientnum = level.maxclients - 1;
		}

		if(!ChaseCam_CheckClient(clientnum, ent))
			continue;

		// this is good, we can use it
		ent->client->sess.spectatorClient = clientnum;

		if(ent->client->sess.spectatorState != SPECTATOR_CHASECAM &&
			ent->client->sess.spectatorState != SPECTATOR_FIXEDCAM &&
			ent->client->sess.spectatorState != SPECTATOR_FOLLOW)
			//ent->client->sess.spectatorState = SPECTATOR_CHASECAM;
			//ent->client->sess.spectatorState = SPECTATOR_FIXEDCAM;
			ent->client->sess.spectatorState = SPECTATOR_FOLLOW;

		VectorCopy(level.clients[clientnum].ps.viewangles, ent->client->ps.viewangles);
		VectorCopy(level.clients[clientnum].ps.delta_angles, ent->client->ps.delta_angles);

		return;
	} while ( clientnum != original );

}

/*
=================
ChaseCam_Stop
by Spoon
=================
*/
void ChaseCam_Stop( gentity_t *ent ) {
	ent->client->sess.spectatorState = SPECTATOR_FREE;
	ent->client->ps.pm_flags &= ~PMF_FOLLOW;
}

/*
=================
ChaseCam_Change
by Spoon
=================
*/
void ChaseCam_Change( gentity_t *ent ) {
	if(!ChaseCam_CheckClient(ent->client->sess.spectatorClient, ent))
		return;

	if(ent->client->sess.spectatorState == SPECTATOR_CHASECAM){
			ent->client->sess.spectatorState = SPECTATOR_FIXEDCAM;
			trap_SendServerCommand( ent-g_entities, va("print \"Fixed Chasecam\n\""));
	} else if (ent->client->sess.spectatorState == SPECTATOR_FREE){
		ent->client->sess.spectatorState = SPECTATOR_FIXEDCAM;
		trap_SendServerCommand( ent-g_entities, va("print \"Fixed Chasecam\n\""));
	} else if(ent->client->sess.spectatorState == SPECTATOR_FIXEDCAM){
		ent->client->sess.spectatorState = SPECTATOR_CHASECAM;
		trap_SendServerCommand( ent-g_entities, va("print \"Free Chasecam\n\""));
	}

	if(ent->client->sess.spectatorState != SPECTATOR_FREE){
		VectorCopy(level.clients[ent->client->sess.spectatorClient].ps.viewangles,
			ent->client->ps.viewangles);
		VectorCopy(level.clients[ent->client->sess.spectatorClient].ps.delta_angles,
			ent->client->ps.delta_angles);
		VectorCopy(level.clients[ent->client->sess.spectatorClient].ps.delta_angles,
			ent->s.angles);
	}
}
#endif

/*
=================
Cmd_Team_f
=================
*/
void Cmd_Team_f( gentity_t *ent ) {
	int			oldTeam;
	char		s[MAX_TOKEN_CHARS];

	if ( trap_Argc() != 2 ) {
		oldTeam = ent->client->sess.sessionTeam;
		switch ( oldTeam ) {
		case TEAM_BLUE:
#ifndef SMOKINGUNS
			trap_SendServerCommand( ent-g_entities, "print \"Blue team\n\"" );
#else
		case TEAM_BLUE_SPECTATOR:
			trap_SendServerCommand( ent-g_entities, va("print \"Team: %s\n\"", g_blueteam.string) );
#endif
			break;
		case TEAM_RED:
#ifndef SMOKINGUNS
			trap_SendServerCommand( ent-g_entities, "print \"Red team\n\"" );
#else
		case TEAM_RED_SPECTATOR:
			trap_SendServerCommand( ent-g_entities, va("print \"Team: %s\n\"", g_redteam.string) );
#endif
			break;
		case TEAM_FREE:
			trap_SendServerCommand( ent-g_entities, "print \"Free team\n\"" );
			break;
		case TEAM_SPECTATOR:
			trap_SendServerCommand( ent-g_entities, "print \"Spectator team\n\"" );
			break;
		}
		return;
	}

	if ( ent->client->switchTeamTime > level.time ) {
		trap_SendServerCommand( ent-g_entities, "print \"May not switch teams more than once per 5 seconds.\n\"" );
		return;
	}

	// if they are playing a tournement game, count as a loss
#ifndef SMOKINGUNS
	if ( (g_gametype.integer == GT_TOURNAMENT )
		&& ent->client->sess.sessionTeam == TEAM_FREE ) {
		ent->client->sess.losses++;
	}
#endif

	trap_Argv( 1, s, sizeof( s ) );

#ifdef SMOKINGUNS
	// change to the spectators
	if ( g_gametype.integer >= GT_RTP) {
		if(s[0] == 'b' || s[0] == 'B'){
			// if already in blue team, dont change
			if(ent->client->sess.sessionTeam == TEAM_BLUE ||
				ent->client->sess.sessionTeam == TEAM_BLUE_SPECTATOR)
				return;
			ent->teamchange = qtrue;
			SetTeam( ent, "bluespec");
		}
		else if (s[0] == 'r' || s[0] == 'R') {
			// if already in red team, dont change
			if(ent->client->sess.sessionTeam == TEAM_RED ||
				ent->client->sess.sessionTeam == TEAM_RED_SPECTATOR)
				return;

			ent->teamchange = qtrue;
			SetTeam( ent, "redspec");
		} else if(s[0] == 'f' || s[0] == 'F'){
			int ran = rand()%2;

			ent->teamchange = qtrue;

			switch(ran){
			case 0:
				SetTeam(ent, "redspec");
				break;
			case 1:
				SetTeam(ent, "bluespec");
				break;
			}
		} else {
			SetTeam( ent, s);
		}

		ent->client->switchTeamTime = level.time + 5000;
		return;
	}

	if ( g_gametype.integer == GT_DUEL) {
		if (s[0] != 's' && s[0] != 'S' && Q_stricmp(s, "spec")){
			ent->teamchange = qtrue;

			ent->client->realspec = qfalse;
			SetTeam( ent, "s");

			ent->client->switchTeamTime = level.time + 5000;
			return;
		} else {
			ent->client->realspec = qtrue;
		}
	}
#endif

	SetTeam( ent, s );

	ent->client->switchTeamTime = level.time + 5000;
}


/*
=================
Cmd_Follow_f
=================
*/
void Cmd_Follow_f( gentity_t *ent ) {
	int		i;
	char	arg[MAX_TOKEN_CHARS];

	if ( trap_Argc() != 2 ) {
		if ( ent->client->sess.spectatorState == SPECTATOR_FOLLOW ) {
			StopFollowing( ent );
		}
		return;
	}

	trap_Argv( 1, arg, sizeof( arg ) );
	i = ClientNumberFromString( ent, arg );
	if ( i == -1 ) {
		return;
	}

	// can't follow self
	if ( &level.clients[ i ] == ent->client ) {
		return;
	}

	// can't follow another spectator
#ifndef SMOKINGUNS
	if ( level.clients[ i ].sess.sessionTeam == TEAM_SPECTATOR ) {
#else
	if ( level.clients[ i ].sess.sessionTeam >= TEAM_SPECTATOR ) {
#endif
		return;
	}

	// if they are playing a tournement game, count as a loss
#ifndef SMOKINGUNS
	if ( (g_gametype.integer == GT_TOURNAMENT )
		&& ent->client->sess.sessionTeam == TEAM_FREE ) {
		ent->client->sess.losses++;
	}
#endif

	// first set them to spectator
	if ( ent->client->sess.sessionTeam != TEAM_SPECTATOR ) {
		SetTeam( ent, "spectator" );
	}

	ent->client->sess.spectatorState = SPECTATOR_FOLLOW;
	ent->client->sess.spectatorClient = i;
}

/*
=================
Cmd_FollowCycle_f
=================
*/
void Cmd_FollowCycle_f( gentity_t *ent, int dir ) {
	int		clientnum;
	int		original;

	// if they are playing a tournement game, count as a loss
#ifndef SMOKINGUNS
	if ( (g_gametype.integer == GT_TOURNAMENT )
		&& ent->client->sess.sessionTeam == TEAM_FREE ) {
		ent->client->sess.losses++;
	}
#endif
	// first set them to spectator
	if ( ent->client->sess.spectatorState == SPECTATOR_NOT ) {
		SetTeam( ent, "spectator" );
	}

	if ( dir != 1 && dir != -1 ) {
		G_Error( "Cmd_FollowCycle_f: bad dir %i", dir );
	}

	clientnum = ent->client->sess.spectatorClient;
	original = clientnum;
	do {
		clientnum += dir;
		if ( clientnum >= level.maxclients ) {
			clientnum = 0;
		}
		if ( clientnum < 0 ) {
			clientnum = level.maxclients - 1;
		}

		// can only follow connected clients
		if ( level.clients[ clientnum ].pers.connected != CON_CONNECTED ) {
			continue;
		}

		// can't follow another spectator
		if ( level.clients[ clientnum ].sess.sessionTeam == TEAM_SPECTATOR ) {
			continue;
		}

		// this is good, we can use it
		ent->client->sess.spectatorClient = clientnum;
		ent->client->sess.spectatorState = SPECTATOR_FOLLOW;
		return;
	} while ( clientnum != original );

	// leave it where it was
}


/*
==================
G_Say
==================
*/

static void G_SayTo( gentity_t *ent, gentity_t *other, int mode, int color, const char *name, const char *message ) {
	if (!other) {
		return;
	}
	if (!other->inuse) {
		return;
	}
	if (!other->client) {
		return;
	}
	if ( other->client->pers.connected != CON_CONNECTED ) {
		return;
	}
	if ( mode == SAY_TEAM  && !OnSameTeam(ent, other) ) {
		return;
	}
	// no chatting to players in tournements
#ifndef SMOKINGUNS
	if ( (g_gametype.integer == GT_TOURNAMENT )
		&& other->client->sess.sessionTeam == TEAM_FREE
		&& ent->client->sess.sessionTeam != TEAM_FREE ) {
		return;
	}
#endif

	trap_SendServerCommand( other-g_entities, va("%s \"%s%c%c%s\"",
		mode == SAY_TEAM ? "tchat" : "chat",
		name, Q_COLOR_ESCAPE, color, message));
}

#define EC		"\x19"

void G_Say( gentity_t *ent, gentity_t *target, int mode, const char *chatText ) {
	int			j;
	gentity_t	*other;
	int			color;
	char		name[64];
#ifdef SMOKINGUNS
	char		endname[64];
#endif
	// don't let text be too long for malicious reasons
	char		text[MAX_SAY_TEXT];
	char		location[64];

	if ( g_gametype.integer < GT_TEAM && mode == SAY_TEAM ) {
		mode = SAY_ALL;
	}

	switch ( mode ) {
	default:
	case SAY_ALL:
		G_LogPrintf( "say: %s: %s\n", ent->client->pers.netname, chatText );
		Com_sprintf (name, sizeof(name), "%s%c%c"EC": ", ent->client->pers.netname, Q_COLOR_ESCAPE, COLOR_WHITE );
		color = COLOR_GREEN;
		break;
	case SAY_TEAM:
		G_LogPrintf( "sayteam: %s: %s\n", ent->client->pers.netname, chatText );
		if (Team_GetLocationMsg(ent, location, sizeof(location)))
			Com_sprintf (name, sizeof(name), EC"(%s%c%c"EC") (%s)"EC": ",
				ent->client->pers.netname, Q_COLOR_ESCAPE, COLOR_WHITE, location);
		else
			Com_sprintf (name, sizeof(name), EC"(%s%c%c"EC")"EC": ",
				ent->client->pers.netname, Q_COLOR_ESCAPE, COLOR_WHITE );
		color = COLOR_CYAN;
		break;
	case SAY_TELL:
		if (target && g_gametype.integer >= GT_TEAM &&
			target->client->sess.sessionTeam == ent->client->sess.sessionTeam &&
			Team_GetLocationMsg(ent, location, sizeof(location)))
			Com_sprintf (name, sizeof(name), EC"[%s%c%c"EC"] (%s)"EC": ", ent->client->pers.netname, Q_COLOR_ESCAPE, COLOR_WHITE, location );
		else
			Com_sprintf (name, sizeof(name), EC"[%s%c%c"EC"]"EC": ", ent->client->pers.netname, Q_COLOR_ESCAPE, COLOR_WHITE );
		color = COLOR_MAGENTA;
		break;
	}

	Q_strncpyz( text, chatText, sizeof(text) );

	if ( target ) {
		G_SayTo( ent, target, mode, color, name, text );
		return;
	}

	// echo the text to the console
#ifndef SMOKINGUNS
	if ( g_dedicated.integer ) {
		G_Printf( "%s%s\n", name, text);
	}
#else

	if(ent->client->sess.sessionTeam == TEAM_SPECTATOR){

		if(!ent->client->realspec && g_gametype.integer == GT_DUEL)
			Com_sprintf(endname, sizeof(endname), "[DEAD]%s", name);
		else
			Com_sprintf(endname, sizeof(endname), "[SPEC]%s", name);
	} else if(ent->client->sess.sessionTeam > TEAM_SPECTATOR)
		Com_sprintf(endname, sizeof(endname), "[DEAD]%s", name);
	else
		Com_sprintf(endname, sizeof(endname), "%s", name);
#endif

	// send it to all the apropriate clients
	for (j = 0; j < level.maxclients; j++) {
		other = &g_entities[j];
#ifndef SMOKINGUNS
		G_SayTo( ent, other, mode, color, name, text );
#else
		if(g_splitChat.integer && g_gametype.integer >= GT_RTP){

			if(ent->client->sess.sessionTeam >= TEAM_SPECTATOR &&
				other->client->sess.sessionTeam < TEAM_SPECTATOR)
				continue;
		}
		if(ent->client->sess.sessionTeam >= TEAM_SPECTATOR)
			color = COLOR_WHITE;

		G_SayTo( ent, other, mode, color, endname, text );
#endif
	}
}


/*
==================
Cmd_Say_f
==================
*/
static void Cmd_Say_f( gentity_t *ent, int mode, qboolean arg0 ) {
	char		*p;

	if ( trap_Argc () < 2 && !arg0 ) {
		return;
	}

	if (arg0)
	{
		p = ConcatArgs( 0 );
	}
	else
	{
		p = ConcatArgs( 1 );
	}

	G_Say( ent, NULL, mode, p );
}

/*
==================
Cmd_Tell_f
==================
*/
static void Cmd_Tell_f( gentity_t *ent ) {
	int			targetNum;
	gentity_t	*target;
	char		*p;
	char		arg[MAX_TOKEN_CHARS];

	if ( trap_Argc () < 2 ) {
		return;
	}

	trap_Argv( 1, arg, sizeof( arg ) );
	targetNum = atoi( arg );
	if ( targetNum < 0 || targetNum >= level.maxclients ) {
		return;
	}

	target = &g_entities[targetNum];
	if ( !target || !target->inuse || !target->client ) {
		return;
	}

	p = ConcatArgs( 2 );

	G_LogPrintf( "tell: %s to %s: %s\n", ent->client->pers.netname, target->client->pers.netname, p );
	G_Say( ent, target, SAY_TELL, p );
	// don't tell to the player self if it was already directed to this player
	// also don't send the chat back to a bot
	if ( ent != target && !(ent->r.svFlags & SVF_BOT)) {
		G_Say( ent, ent, SAY_TELL, p );
	}
}


static void G_VoiceTo( gentity_t *ent, gentity_t *other, int mode, const char *id, qboolean voiceonly ) {
	int color;
	char *cmd;

	if (!other) {
		return;
	}
	if (!other->inuse) {
		return;
	}
	if (!other->client) {
		return;
	}
	if ( mode == SAY_TEAM && !OnSameTeam(ent, other) ) {
		return;
	}
	// no chatting to players in tournements
#ifndef SMOKINGUNS
	if ( (g_gametype.integer == GT_TOURNAMENT )) {
		return;
	}
#endif

	if (mode == SAY_TEAM) {
#ifndef SMOKINGUNS
		color = COLOR_CYAN;
#else
		color = COLOR_YELLOW;
#endif
		cmd = "vtchat";
	}
	else if (mode == SAY_TELL) {
#ifndef SMOKINGUNS
		color = COLOR_MAGENTA;
#else
		color = COLOR_RED;
#endif
		cmd = "vtell";
	}
	else {
		color = COLOR_GREEN;
		cmd = "vchat";
	}

	trap_SendServerCommand( other-g_entities, va("%s %d %d %d %s", cmd, voiceonly, ent->s.number, color, id));
}

void G_Voice( gentity_t *ent, gentity_t *target, int mode, const char *id, qboolean voiceonly ) {
	int			j;
	gentity_t	*other;

	if ( g_gametype.integer < GT_TEAM && mode == SAY_TEAM ) {
		mode = SAY_ALL;
	}

	if ( target ) {
		G_VoiceTo( ent, target, mode, id, voiceonly );
		return;
	}

	// echo the text to the console
	if ( g_dedicated.integer ) {
		G_Printf( "voice: %s %s\n", ent->client->pers.netname, id);
	}

	// send it to all the apropriate clients
	for (j = 0; j < level.maxclients; j++) {
		other = &g_entities[j];
		G_VoiceTo( ent, other, mode, id, voiceonly );
	}
}

/*
==================
Cmd_Voice_f
==================
*/
#ifndef SMOKINGUNS
static void Cmd_Voice_f( gentity_t *ent, int mode, qboolean arg0, qboolean voiceonly ) {
	char		*p;

	if ( trap_Argc () < 2 && !arg0 ) {
		return;
	}

	if (arg0)
	{
		p = ConcatArgs( 0 );
	}
	else
	{
		p = ConcatArgs( 1 );
	}

	G_Voice( ent, NULL, mode, p, voiceonly );
}

/*
==================
Cmd_VoiceTell_f
==================
*/
static void Cmd_VoiceTell_f( gentity_t *ent, qboolean voiceonly ) {
	int			targetNum;
	gentity_t	*target;
	char		*id;
	char		arg[MAX_TOKEN_CHARS];

	if ( trap_Argc () < 2 ) {
		return;
	}

	trap_Argv( 1, arg, sizeof( arg ) );
	targetNum = atoi( arg );
	if ( targetNum < 0 || targetNum >= level.maxclients ) {
		return;
	}

	target = &g_entities[targetNum];
	if ( !target || !target->inuse || !target->client ) {
		return;
	}

	id = ConcatArgs( 2 );

	G_LogPrintf( "vtell: %s to %s: %s\n", ent->client->pers.netname, target->client->pers.netname, id );
	G_Voice( ent, target, SAY_TELL, id, voiceonly );
	// don't tell to the player self if it was already directed to this player
	// also don't send the chat back to a bot
	if ( ent != target && !(ent->r.svFlags & SVF_BOT)) {
		G_Voice( ent, ent, SAY_TELL, id, voiceonly );
	}
}


/*
==================
Cmd_VoiceTaunt_f
==================
*/
static void Cmd_VoiceTaunt_f( gentity_t *ent ) {
	gentity_t *who;
	int i;

	if (!ent->client) {
		return;
	}

	// insult someone who just killed you
	if (ent->enemy && ent->enemy->client && ent->enemy->client->lastkilled_client == ent->s.number) {
		// i am a dead corpse
		if (!(ent->enemy->r.svFlags & SVF_BOT)) {
			G_Voice( ent, ent->enemy, SAY_TELL, VOICECHAT_DEATHINSULT, qfalse );
		}
		if (!(ent->r.svFlags & SVF_BOT)) {
			G_Voice( ent, ent,        SAY_TELL, VOICECHAT_DEATHINSULT, qfalse );
		}
		ent->enemy = NULL;
		return;
	}
	// insult someone you just killed
	if (ent->client->lastkilled_client >= 0 && ent->client->lastkilled_client != ent->s.number) {
		who = g_entities + ent->client->lastkilled_client;
		if (who->client) {
			// who is the person I just killed
			if (who->client->lasthurt_mod == MOD_GAUNTLET) {
				if (!(who->r.svFlags & SVF_BOT)) {
					G_Voice( ent, who, SAY_TELL, VOICECHAT_KILLGAUNTLET, qfalse );	// and I killed them with a gauntlet
				}
				if (!(ent->r.svFlags & SVF_BOT)) {
					G_Voice( ent, ent, SAY_TELL, VOICECHAT_KILLGAUNTLET, qfalse );
				}
			} else {
				if (!(who->r.svFlags & SVF_BOT)) {
					G_Voice( ent, who, SAY_TELL, VOICECHAT_KILLINSULT, qfalse );	// and I killed them with something else
				}
				if (!(ent->r.svFlags & SVF_BOT)) {
					G_Voice( ent, ent, SAY_TELL, VOICECHAT_KILLINSULT, qfalse );
				}
			}
			ent->client->lastkilled_client = -1;
			return;
		}
	}

	if (g_gametype.integer >= GT_TEAM) {
		// praise a team mate who just got a reward
		for(i = 0; i < MAX_CLIENTS; i++) {
			who = g_entities + i;
			if (who->client && who != ent && who->client->sess.sessionTeam == ent->client->sess.sessionTeam) {
				if (who->client->rewardTime > level.time) {
					if (!(who->r.svFlags & SVF_BOT)) {
						G_Voice( ent, who, SAY_TELL, VOICECHAT_PRAISE, qfalse );
					}
					if (!(ent->r.svFlags & SVF_BOT)) {
						G_Voice( ent, ent, SAY_TELL, VOICECHAT_PRAISE, qfalse );
					}
					return;
				}
			}
		}
	}

	// just say something
	G_Voice( ent, NULL, SAY_ALL, VOICECHAT_TAUNT, qfalse );
}
#endif


static char	*gc_orders[] = {
	"hold your position",
	"hold this position",
	"come here",
	"cover me",
	"guard location",
	"search and destroy",
	"report"
};

void Cmd_GameCommand_f( gentity_t *ent ) {
	int		player;
	int		order;
	char	str[MAX_TOKEN_CHARS];

	trap_Argv( 1, str, sizeof( str ) );
	player = atoi( str );
	trap_Argv( 2, str, sizeof( str ) );
	order = atoi( str );

	if ( player < 0 || player >= MAX_CLIENTS ) {
		return;
	}
	if ( order < 0 || order > sizeof(gc_orders)/sizeof(char *) ) {
		return;
	}
	G_Say( ent, &g_entities[player], SAY_TELL, gc_orders[order] );
	G_Say( ent, ent, SAY_TELL, gc_orders[order] );
}

/*
==================
Cmd_Where_f
==================
*/
void Cmd_Where_f( gentity_t *ent ) {
	trap_SendServerCommand( ent-g_entities, va("print \"%s\n\"", vtos( ent->s.origin ) ) );
}

static const char *gameNames[] = {
#ifndef SMOKINGUNS
	"Free For All",
	"Tournament",
	"Single Player",
	"Team Deathmatch",
	"Capture the Flag",
	"One Flag CTF",
	"Overload",
	"Harvester"
#else
	"Deathmatch",
	"Duel",
	"Single Player",
	"Team Deathmatch",
	"Round Teamplay",
	"Bank Robbery",
#endif
};

/*
==================
Cmd_CallVote_f
==================
*/
void Cmd_CallVote_f( gentity_t *ent ) {
	char*	c;
	int		i;
	char	arg1[MAX_STRING_TOKENS];
	char	arg2[MAX_STRING_TOKENS];

	if ( !g_allowVote.integer ) {
		trap_SendServerCommand( ent-g_entities, "print \"Voting not allowed here.\n\"" );
		return;
	}

	if ( level.voteTime ) {
		trap_SendServerCommand( ent-g_entities, "print \"A vote is already in progress.\n\"" );
		return;
	}
	if ( ent->client->pers.voteCount >= MAX_VOTE_COUNT ) {
		trap_SendServerCommand( ent-g_entities, "print \"You have called the maximum number of votes.\n\"" );
		return;
	}
#ifndef SMOKINGUNS
	if ( ent->client->sess.sessionTeam == TEAM_SPECTATOR ) {
#else
	if ( ent->client->sess.sessionTeam == TEAM_SPECTATOR && (g_gametype.integer != GT_DUEL || ent->client->realspec)) {
#endif
		trap_SendServerCommand( ent-g_entities, "print \"Not allowed to call a vote as spectator.\n\"" );
		return;
	}

	// make sure it is a valid command to vote on
	trap_Argv( 1, arg1, sizeof( arg1 ) );
	trap_Argv( 2, arg2, sizeof( arg2 ) );

	// check for command separators in arg2
	for( c = arg2; *c; ++c) {
		switch(*c) {
			case '\n':
			case '\r':
			case ';':
				trap_SendServerCommand( ent-g_entities, "print \"Invalid vote string.\n\"" );
				return;
			break;
		}
	}

	if ( !Q_stricmp( arg1, "map_restart" ) ) {
	} else if ( !Q_stricmp( arg1, "nextmap" ) ) {
	} else if ( !Q_stricmp( arg1, "map" ) ) {
	} else if ( !Q_stricmp( arg1, "g_gametype" ) ) {
	} else if ( !Q_stricmp( arg1, "kick" ) ) {
	} else if ( !Q_stricmp( arg1, "clientkick" ) ) {
	} else if ( !Q_stricmp( arg1, "g_doWarmup" ) ) {
	} else if ( !Q_stricmp( arg1, "timelimit" ) ) {
	} else if ( !Q_stricmp( arg1, "fraglimit" ) ) {
	} else if ( !Q_stricmp( arg1, "mapcycle" ) ) {
	} else {
		trap_SendServerCommand( ent-g_entities, "print \"Invalid vote string.\n\"" );
		trap_SendServerCommand( ent-g_entities, "print \"Vote commands are: map_restart, nextmap, map <mapname>, g_gametype <n>, kick <player>, clientkick <clientnum>, g_doWarmup, timelimit <time>, fraglimit <frags>, mapcycle <mapcyclename>.\n\"" );
		return;
	}

#ifdef SMOKINGUNS
	if ( ( !Q_stricmp( arg1, "kick" ) || !Q_stricmp( arg1, "clientkick" ) )
	&& !g_allowVoteKick.integer ) {
		trap_SendServerCommand( ent-g_entities, "print \"Not allowed to vote kick here.\n\"" );
		return;
	}
#endif

	// if there is still a vote to be executed
	if ( level.voteExecuteTime ) {
		level.voteExecuteTime = 0;
		trap_SendConsoleCommand( EXEC_APPEND, va("%s\n", level.voteString ) );
	}

	// special case for g_gametype, check for bad values
	if ( !Q_stricmp( arg1, "g_gametype" ) ) {
		i = atoi( arg2 );
#ifndef SMOKINGUNS
		if( i == GT_SINGLE_PLAYER || i < GT_FFA || i >= GT_MAX_GAME_TYPE) {
#else
		if( i == GT_SINGLE_PLAYER || i < GT_FFA || i > GT_RTP) {
#endif
			trap_SendServerCommand( ent-g_entities, "print \"Invalid gametype.\n\"" );
			return;
		}

		Com_sprintf( level.voteString, sizeof( level.voteString ), "%s %d", arg1, i );
		Com_sprintf( level.voteDisplayString, sizeof( level.voteDisplayString ), "%s %s", arg1, gameNames[i] );
	} else if ( !Q_stricmp( arg1, "map" ) ) {
		// special case for map changes, we want to reset the nextmap setting
		// this allows a player to change maps, but not upset the map rotation
#ifndef SMOKINGUNS
		char	s[MAX_STRING_CHARS];

		trap_Cvar_VariableStringBuffer( "nextmap", s, sizeof(s) );
		if (*s) {
			Com_sprintf( level.voteString, sizeof( level.voteString ), "%s %s; set nextmap \"%s\"", arg1, arg2, s );
		} else {
			Com_sprintf( level.voteString, sizeof( level.voteString ), "%s %s", arg1, arg2 );
		}
#else
		// Joe Kari : calling a vote for a /map command is not supposed to change 'nextmap' cvar
		// We removed this code, this is not usefull anymore since I have modified server/sv_init.c to never
		// turn 'nextmap' to 'map_restart 0'
		Com_sprintf( level.voteString, sizeof( level.voteString ), "%s %s", arg1, arg2 );
#endif
		Com_sprintf( level.voteDisplayString, sizeof( level.voteDisplayString ), "%s", level.voteString );
#ifdef SMOKINGUNS
	} else if ( !Q_stricmp( arg1, "mapcycle" ) ) {	// Joe Kari: allow player to vote for a mapcycle

		char mapcycles[ MAX_STRING_CHARS ];
		char buf[ 100 ];
		char *token, *p;
		qboolean match = qfalse;

		trap_Cvar_VariableStringBuffer( "g_mapcycles" , mapcycles, sizeof( mapcycles ) ) ;

		p = mapcycles;
		while ( !match && *( token = COM_Parse( &p ) ) ) {
			if ( !Q_stricmp( arg2 , token ) )
				match = qtrue;
		}

		if ( !match ) {
			trap_SendServerCommand( ent-g_entities, "print \"Unknown mapcycle. Available mapcycles are:\n\"" );
			p = mapcycles;
			while ( *( token = COM_Parse( &p ) )  )  {
				Com_sprintf( buf ,  sizeof( buf ) , "print \"^5%s^7\n\"" , token ) ;
				trap_SendServerCommand( ent-g_entities, buf ) ;
			}
			return;
		}

		Com_sprintf( level.voteString, sizeof( level.voteString ), "vstr %s", token );
		Com_sprintf( level.voteDisplayString, sizeof( level.voteDisplayString ), "mapcycle %s", token);

#endif
	} else if ( !Q_stricmp( arg1, "nextmap" ) ) {
		char	s[MAX_STRING_CHARS];

		trap_Cvar_VariableStringBuffer( "nextmap", s, sizeof(s) );
		if (!*s) {
			trap_SendServerCommand( ent-g_entities, "print \"nextmap not set.\n\"" );
			return;
		}
		Com_sprintf( level.voteString, sizeof( level.voteString ), "vstr nextmap");
		Com_sprintf( level.voteDisplayString, sizeof( level.voteDisplayString ), "%s", level.voteString );
	} else {
		Com_sprintf( level.voteString, sizeof( level.voteString ), "%s \"%s\"", arg1, arg2 );
		Com_sprintf( level.voteDisplayString, sizeof( level.voteDisplayString ), "%s", level.voteString );
	}

	trap_SendServerCommand( -1, va("print \"%s called a vote.\n\"", ent->client->pers.netname ) );

	// start the voting, the caller automatically votes yes
	level.voteTime = level.time;
	level.voteYes = 1;
	level.voteNo = 0;

	for ( i = 0 ; i < level.maxclients ; i++ ) {
		level.clients[i].ps.eFlags &= ~EF_VOTED;
	}
	ent->client->ps.eFlags |= EF_VOTED;

	trap_SetConfigstring( CS_VOTE_TIME, va("%i", level.voteTime ) );
	trap_SetConfigstring( CS_VOTE_STRING, level.voteDisplayString );
	trap_SetConfigstring( CS_VOTE_YES, va("%i", level.voteYes ) );
	trap_SetConfigstring( CS_VOTE_NO, va("%i", level.voteNo ) );
}

/*
==================
Cmd_Vote_f
==================
*/
void Cmd_Vote_f( gentity_t *ent ) {
	char		msg[64];

	if ( !level.voteTime ) {
		trap_SendServerCommand( ent-g_entities, "print \"No vote in progress.\n\"" );
		return;
	}
	if ( ent->client->ps.eFlags & EF_VOTED ) {
		trap_SendServerCommand( ent-g_entities, "print \"Vote already cast.\n\"" );
		return;
	}
#ifndef SMOKINGUNS
	if ( ent->client->sess.sessionTeam == TEAM_SPECTATOR ) {
#else
	if ( ent->client->sess.sessionTeam == TEAM_SPECTATOR && (g_gametype.integer != GT_DUEL || ent->client->realspec)) {
#endif
		trap_SendServerCommand( ent-g_entities, "print \"Not allowed to vote as spectator.\n\"" );
		return;
	}

	trap_SendServerCommand( ent-g_entities, "print \"Vote cast.\n\"" );

	ent->client->ps.eFlags |= EF_VOTED;

	trap_Argv( 1, msg, sizeof( msg ) );

	if ( msg[0] == 'y' || msg[1] == 'Y' || msg[1] == '1' ) {
		level.voteYes++;
		trap_SetConfigstring( CS_VOTE_YES, va("%i", level.voteYes ) );
	} else {
		level.voteNo++;
		trap_SetConfigstring( CS_VOTE_NO, va("%i", level.voteNo ) );
	}

	// a majority will be determined in CheckVote, which will also account
	// for players entering or leaving
}

/*
==================
Cmd_CallTeamVote_f
==================
*/
void Cmd_CallTeamVote_f( gentity_t *ent ) {
	int		i, team, cs_offset;
	char	arg1[MAX_STRING_TOKENS];
	char	arg2[MAX_STRING_TOKENS];

	team = ent->client->sess.sessionTeam;
	if ( team == TEAM_RED )
		cs_offset = 0;
	else if ( team == TEAM_BLUE )
		cs_offset = 1;
	else
		return;

	if ( !g_allowVote.integer ) {
		trap_SendServerCommand( ent-g_entities, "print \"Voting not allowed here.\n\"" );
		return;
	}

	if ( level.teamVoteTime[cs_offset] ) {
		trap_SendServerCommand( ent-g_entities, "print \"A team vote is already in progress.\n\"" );
		return;
	}
	if ( ent->client->pers.teamVoteCount >= MAX_VOTE_COUNT ) {
		trap_SendServerCommand( ent-g_entities, "print \"You have called the maximum number of team votes.\n\"" );
		return;
	}
#ifndef SMOKINGUNS
	if ( ent->client->sess.sessionTeam == TEAM_SPECTATOR ) {
#else
	if ( ent->client->sess.sessionTeam == TEAM_SPECTATOR && (g_gametype.integer != GT_DUEL || ent->client->realspec)) {
#endif
		trap_SendServerCommand( ent-g_entities, "print \"Not allowed to call a vote as spectator.\n\"" );
		return;
	}

	// make sure it is a valid command to vote on
	trap_Argv( 1, arg1, sizeof( arg1 ) );
	arg2[0] = '\0';
	for ( i = 2; i < trap_Argc(); i++ ) {
		if (i > 2)
			strcat(arg2, " ");
		trap_Argv( i, &arg2[strlen(arg2)], sizeof( arg2 ) - strlen(arg2) );
	}

	if( strchr( arg1, ';' ) || strchr( arg2, ';' ) ) {
		trap_SendServerCommand( ent-g_entities, "print \"Invalid vote string.\n\"" );
		return;
	}

	if ( !Q_stricmp( arg1, "leader" ) ) {
		char netname[MAX_NETNAME], leader[MAX_NETNAME];

		if ( !arg2[0] ) {
			i = ent->client->ps.clientNum;
		}
		else {
			// numeric values are just slot numbers
			for (i = 0; i < 3; i++) {
				if ( !arg2[i] || arg2[i] < '0' || arg2[i] > '9' )
					break;
			}
			if ( i >= 3 || !arg2[i]) {
				i = atoi( arg2 );
				if ( i < 0 || i >= level.maxclients ) {
					trap_SendServerCommand( ent-g_entities, va("print \"Bad client slot: %i\n\"", i) );
					return;
				}

				if ( !g_entities[i].inuse ) {
					trap_SendServerCommand( ent-g_entities, va("print \"Client %i is not active\n\"", i) );
					return;
				}
			}
			else {
				Q_strncpyz(leader, arg2, sizeof(leader));
				Q_CleanStr(leader);
				for ( i = 0 ; i < level.maxclients ; i++ ) {
					if ( level.clients[i].pers.connected == CON_DISCONNECTED )
						continue;
					if (level.clients[i].sess.sessionTeam != team)
						continue;
					Q_strncpyz(netname, level.clients[i].pers.netname, sizeof(netname));
					Q_CleanStr(netname);
					if ( !Q_stricmp(netname, leader) ) {
						break;
					}
				}
				if ( i >= level.maxclients ) {
					trap_SendServerCommand( ent-g_entities, va("print \"%s is not a valid player on your team.\n\"", arg2) );
					return;
				}
			}
		}
		Com_sprintf(arg2, sizeof(arg2), "%d", i);
	} else {
		trap_SendServerCommand( ent-g_entities, "print \"Invalid vote string.\n\"" );
		trap_SendServerCommand( ent-g_entities, "print \"Team vote commands are: leader <player>.\n\"" );
		return;
	}

	Com_sprintf( level.teamVoteString[cs_offset], sizeof( level.teamVoteString[cs_offset] ), "%s %s", arg1, arg2 );

	for ( i = 0 ; i < level.maxclients ; i++ ) {
		if ( level.clients[i].pers.connected == CON_DISCONNECTED )
			continue;
		if (level.clients[i].sess.sessionTeam == team)
			trap_SendServerCommand( i, va("print \"%s called a team vote.\n\"", ent->client->pers.netname ) );
	}

	// start the voting, the caller autoamtically votes yes
	level.teamVoteTime[cs_offset] = level.time;
	level.teamVoteYes[cs_offset] = 1;
	level.teamVoteNo[cs_offset] = 0;

	for ( i = 0 ; i < level.maxclients ; i++ ) {
		if (level.clients[i].sess.sessionTeam == team)
			level.clients[i].ps.eFlags &= ~EF_TEAMVOTED;
	}
	ent->client->ps.eFlags |= EF_TEAMVOTED;

	trap_SetConfigstring( CS_TEAMVOTE_TIME + cs_offset, va("%i", level.teamVoteTime[cs_offset] ) );
	trap_SetConfigstring( CS_TEAMVOTE_STRING + cs_offset, level.teamVoteString[cs_offset] );
	trap_SetConfigstring( CS_TEAMVOTE_YES + cs_offset, va("%i", level.teamVoteYes[cs_offset] ) );
	trap_SetConfigstring( CS_TEAMVOTE_NO + cs_offset, va("%i", level.teamVoteNo[cs_offset] ) );
}

/*
==================
Cmd_TeamVote_f
==================
*/
void Cmd_TeamVote_f( gentity_t *ent ) {
	int			team, cs_offset;
	char		msg[64];

	team = ent->client->sess.sessionTeam;
	if ( team == TEAM_RED )
		cs_offset = 0;
	else if ( team == TEAM_BLUE )
		cs_offset = 1;
	else
		return;

	if ( !level.teamVoteTime[cs_offset] ) {
		trap_SendServerCommand( ent-g_entities, "print \"No team vote in progress.\n\"" );
		return;
	}
	if ( ent->client->ps.eFlags & EF_TEAMVOTED ) {
		trap_SendServerCommand( ent-g_entities, "print \"Team vote already cast.\n\"" );
		return;
	}
#ifndef SMOKINGUNS
	if ( ent->client->sess.sessionTeam == TEAM_SPECTATOR ) {
#else
	if ( ent->client->sess.sessionTeam == TEAM_SPECTATOR && (g_gametype.integer != GT_DUEL || ent->client->realspec)) {
#endif
		trap_SendServerCommand( ent-g_entities, "print \"Not allowed to vote as spectator.\n\"" );
		return;
	}

	trap_SendServerCommand( ent-g_entities, "print \"Team vote cast.\n\"" );

	ent->client->ps.eFlags |= EF_TEAMVOTED;

	trap_Argv( 1, msg, sizeof( msg ) );

	if ( msg[0] == 'y' || msg[1] == 'Y' || msg[1] == '1' ) {
		level.teamVoteYes[cs_offset]++;
		trap_SetConfigstring( CS_TEAMVOTE_YES + cs_offset, va("%i", level.teamVoteYes[cs_offset] ) );
	} else {
		level.teamVoteNo[cs_offset]++;
		trap_SetConfigstring( CS_TEAMVOTE_NO + cs_offset, va("%i", level.teamVoteNo[cs_offset] ) );
	}

	// a majority will be determined in TeamCheckVote, which will also account
	// for players entering or leaving
}


/*
=================
Cmd_SetViewpos_f
=================
*/
void Cmd_SetViewpos_f( gentity_t *ent ) {
	vec3_t		origin, angles;
	char		buffer[MAX_TOKEN_CHARS];
	int			i;

	if ( !g_cheats.integer ) {
		trap_SendServerCommand( ent-g_entities, va("print \"Cheats are not enabled on this server.\n\""));
		return;
	}
	if ( trap_Argc() != 5 ) {
		trap_SendServerCommand( ent-g_entities, va("print \"usage: setviewpos x y z yaw\n\""));
		return;
	}

	VectorClear( angles );
	for ( i = 0 ; i < 3 ; i++ ) {
		trap_Argv( i + 1, buffer, sizeof( buffer ) );
		origin[i] = atof( buffer );
	}

	trap_Argv( 4, buffer, sizeof( buffer ) );
	angles[YAW] = atof( buffer );

	TeleportPlayer( ent, origin, angles );
}



/*
=================
Cmd_Stats_f
=================
*/
void Cmd_Stats_f( gentity_t *ent ) {
/*
	int max, n, i;

	max = trap_AAS_PointReachabilityAreaIndex( NULL );

	n = 0;
	for ( i = 0; i < max; i++ ) {
		if ( ent->client->areabits[i >> 3] & (1 << (i & 7)) )
			n++;
	}

	//trap_SendServerCommand( ent-g_entities, va("print \"visited %d of %d areas\n\"", n, max));
	trap_SendServerCommand( ent-g_entities, va("print \"%d%% level coverage\n\"", n * 100 / max));
*/
}

/*
=================
Cmd_DropWeapon_f
=================
*/
#ifdef SMOKINGUNS
void Cmd_DropWeapon_f( gentity_t *ent, int weapon ) {

	gclient_t	*client;
	usercmd_t	*ucmd;

	// Don't drop anything if the player is a spectator
	//G_Printf( "ent->client->sess.spectatorState: %d\n" , ent->client->sess.spectatorState ) ;
	// Joe Kari : it looks like using sess.spectatorState is buggy, a playing player have a value of SPECTATOR_FREE instead of SPECTATOR_NOT
	// so I will use sess.sessionTeam instead
	switch ( ent->client->sess.sessionTeam ) {
		case TEAM_SPECTATOR :
		case TEAM_RED_SPECTATOR :
		case TEAM_BLUE_SPECTATOR :
			return ;
		default: // Tequila comment: Avoid a compialer warning
			break ;
	}

	client = ent->client;
	ucmd = &ent->client->pers.cmd;

	if( (ucmd->buttons & BUTTON_ATTACK) ||
		(ucmd->buttons & BUTTON_ALT_ATTACK) ||
		(client->ps.eFlags & EF_RELOAD))
		return;

	/*for ( i=WP_WINCHESTER66 ; i < WP_NUM_WEAPONS; i++){
		if(client->ps.stats[STAT_WEAPONS] & (1 << i)){
			if( i == WP_DYNAMITE)
				continue; // no dropable weapon

			if(i==WP_GATLING && client->ps.stats[STAT_GATLING_MODE])
				continue;

			weapon = i;
			break;
		}
	}*/

	if(!weapon) {
		if(!client->ps.weapon2){
			weapon = client->ps.weapon;
		} else {
			if(client->ps.weapon2 > client->ps.weapon)
				weapon = client->ps.weapon;
			else
				weapon = client->ps.weapon2;
		}

		if(weapon <= 0 || weapon >= WP_NUM_WEAPONS || weapon == WP_KNIFE ||
			weapon == WP_DYNAMITE || weapon == WP_MOLOTOV) //no weapon found
			return;
	}

	// if we're throwing away the gatling and it's being used, abort
	if(weapon == WP_GATLING && ent->client->ps.stats[STAT_GATLING_MODE])
		return;

	// weapon which is in the hand will be dropped
	G_ThrowWeapon( weapon, ent );
}

/*
=================
Cmd_GatlingBuildUp_f
=================
*/
void Cmd_GatlingBuildUp_f( gentity_t *ent ) {
	G_GatlingBuildUp( ent );
}

void Cmd_DuGet_f ( gentity_t *ent){
	char			arg1[MAX_STRING_TOKENS];
	playerState_t	*ps= &ent->client->ps;
	int				key;
	int				weapon1 = 0, weapon2 = 0;

	if(du_introend < level.time)
		return;

	if(g_gametype.integer != GT_DUEL)
		return;

	trap_Argv( 1, arg1, sizeof( arg1 ) );

	if(!arg1[0])
		return;

	key = atoi(arg1);
	ps->stats[STAT_WEAPONS] &= ~(1 << WP_REM58);
	ps->ammo[WP_REM58] = 0;

	switch(key){
	case 1:
		weapon1 = WP_REM58;
		break;
	case 2:
		weapon1 = WP_SCHOFIELD;
		break;
	case 3:
		weapon1 = WP_PEACEMAKER;
		break;
	case 4:
		weapon1 = WP_REM58;
		weapon2 = WP_PEACEMAKER;
		break;
	case 5:
		weapon1 = WP_SCHOFIELD;
		weapon2 = WP_PEACEMAKER;
		break;
	case 6:
		weapon1 = weapon2 = WP_PEACEMAKER;
		break;
	}

	if(weapon1){
		// fill up ammo
		ps->stats[STAT_WEAPONS] |= (1 << weapon1);
		ps->ammo[weapon1] = bg_weaponlist[weapon1].clipAmmo;
		ps->ammo[bg_weaponlist[weapon1].clip] = bg_weaponlist[weapon1].maxAmmo;
	}

	if(weapon2 && weapon1 != weapon2){
		// fill up ammo
		ps->stats[STAT_WEAPONS] |= (1 << weapon2);
		ps->ammo[weapon2] = bg_weaponlist[weapon2].clipAmmo;
		ps->ammo[bg_weaponlist[weapon2].clip] = bg_weaponlist[weapon2].maxAmmo;
	} else if (weapon2){
		ps->stats[STAT_FLAGS] |= SF_SEC_PISTOL;
		ps->ammo[WP_AKIMBO] = bg_weaponlist[weapon2].clipAmmo;
	}
}
/*
=================
Cmd_BuyItem_f
=================
*/
void Cmd_BuyItem_f( gentity_t *ent, qboolean cgame) {
	gitem_t	*item;
	char	arg1[MAX_STRING_TOKENS];
	int		i;
	int		belt = 1;
	int		prize;
	int		weapon;
	playerState_t *ps = &ent->client->ps;
	qboolean gatling = (ent->client->ps.weapon == WP_GATLING &&
		ent->client->ps.stats[STAT_GATLING_MODE]);

	if(ent->client->ps.stats[STAT_HEALTH] <= 0)
		return;

	if(!(ent->client->ps.stats[STAT_FLAGS] & SF_CANBUY) && g_gametype.integer >= GT_RTP){
		if(!cgame)
			trap_SendServerCommand( ent-g_entities, va("print \"You're not in a buy-zone!\n\""));
		return;
	}

	if(level.time - g_roundstarttime > BUY_TIME && g_gametype.integer >= GT_RTP){
		if(!cgame)
			trap_SendServerCommand( ent-g_entities, va("print \"60 seconds have passed ... you can't buy anything\n\""));
		return;
	}

	trap_Argv( 1, arg1, sizeof( arg1 ) );

	item = BG_FindItemForClassname( arg1 );

	if(ent->client->sess.sessionTeam >= TEAM_SPECTATOR){
		if(!cgame)
			trap_SendServerCommand( ent-g_entities, va("print \"You can't buy while in spectator mode!\n\""));
		return;
	}

	if(!item){
		trap_SendServerCommand( ent-g_entities, "print \"Unknown Item\n\"");
		return;
	}

	if(item->prize <= 0)
		return;

	if(ent->client->ps.stats[STAT_MONEY] < item->prize){
		if(!cgame)
			trap_SendServerCommand( ent-g_entities, va("print \"Not Enough Money!\n\""));
		return;
	}

	//trap_SendServerCommand( ent-g_entities, va("print \"debug: %s - %s\n\"",
	//	arg1, item->pickup_name));

	//save because the item might change now
	prize = item->prize;

	if(ps->powerups[PW_BELT])
		belt = 2;

	if(g_gametype.integer == GT_DUEL){
		if(item->giType == IT_WEAPON && bg_weaponlist[item->giTag].wp_sort != WPS_PISTOL)
			return;

		if(item->giType != IT_AMMO && item->giType != IT_WEAPON)
			return;
	}

	//check whether the item has to be changed or not
	switch(item->giType){
	case IT_WEAPON:

		// can't have more than two pistols -> drop bad ones
		if(bg_weaponlist[item->giTag].wp_sort == WPS_PISTOL && CheckPistols(ps, &weapon)){
			if (!(ps->stats[STAT_FLAGS] & SF_SEC_PISTOL) &&
			                             weapon == item->giTag) {
			// The player has two different pistols, wants to buy
			// a pistol of the same type of one already possessed.
			// Drop the one of the different type.
				int weapon_drop = BG_SearchTypeWeapon(WPS_PISTOL, ps->stats[STAT_WEAPONS], weapon);
				if (weapon_drop)
					weapon = weapon_drop;
			}
			G_ThrowWeapon(weapon, ent);
		}

		//if there already is a special weapon: drop old special weapon
		if(bg_weaponlist[item->giTag].wp_sort == WPS_GUN ){
			int drop;

			if(gatling)
				return;

			// throw away all the weapons you can find
			drop = BG_FindPlayerWeapon(WP_WINCHESTER66, WP_DYNAMITE, ps);
			while(drop){
				//Cmd_DropWeapon_f(ent, drop);
				G_ThrowWeapon(drop, ent);
				drop = BG_FindPlayerWeapon(WP_WINCHESTER66, WP_DYNAMITE, ps);
			}
		}

		//check ammo
		if(item->giTag == WP_DYNAMITE || item->giTag == WP_KNIFE || item->giTag == WP_MOLOTOV) {
			if(ps->ammo[item->giTag] >= bg_weaponlist[item->giTag].maxAmmo){
				if(!cgame)
					trap_SendServerCommand( ent-g_entities, va("print \"You can't carry any more!\n\""));
				return;
			}

			item = BG_FindItemForAmmo(item->giTag);
		}

		//give ammo to player
		if(bg_weaponlist[item->giTag].clip && ps->ammo[bg_weaponlist[item->giTag].clip] < bg_weaponlist[item->giTag].maxAmmo){
			ps->ammo[bg_weaponlist[item->giTag].clip] = bg_weaponlist[item->giTag].maxAmmo;
		}
		break;
	case IT_AMMO:
		if (item->giTag == WP_BULLETS_CLIP) {
			item = BG_FindItemForAmmo(WP_BULLETS_CLIP);
			i = WP_PEACEMAKER;
		} else {
			if ( ps->stats[STAT_GATLING_MODE] )
				i = WP_GATLING;
			else
				i = BG_FindPlayerWeapon(WP_WINCHESTER66, WP_DYNAMITE, ps);
			if ( i )
				item = BG_FindItemForAmmo(bg_weaponlist[i].clip);
			else
				return;
		}
		if(ps->ammo[item->giTag] >= belt*bg_weaponlist[i].maxAmmo){
			if(!cgame)
				trap_SendServerCommand( ent-g_entities, va("print \"You can't carry any more!\n\""));
			return;

		}
		break;
	case IT_ARMOR:
		if(ps->stats[STAT_ARMOR] && !cgame){
			trap_SendServerCommand( ent-g_entities, va("print \"You already have a boiler plate!\n\""));
			return;
		}
		break;
	case IT_POWERUP:
		if(item->giTag == PW_BELT){
			if(belt == 2){
				if(!cgame)
					trap_SendServerCommand( ent-g_entities, va("print \"You can't carry any more!\n\""));
				return;
			}
		}
		break;
	default:
		trap_SendServerCommand( ent-g_entities, va("print \"This item is not buyable\n\""));
		return;
	}

	//Spawn the item near the player and immediately pick it up
	{
		gentity_t *it_ent;
		trace_t	trace;

		if (!item) {
			return;
		}

		it_ent = G_Spawn();
		//item is not spawned normally, even not linked to world
		it_ent->flags |= FL_BUY_ITEM;
		VectorCopy( ent->client->ps.origin, it_ent->s.origin );
		it_ent->classname = item->classname;
		G_SpawnItem (it_ent, item);
		FinishSpawningItem(it_ent );
		memset( &trace, 0, sizeof( trace ) );
		Touch_Item (it_ent, ent, &trace);
		if (it_ent->inuse) {
			G_FreeEntity( it_ent );
		}
	}

	// give full ammo to all weapon-clips
	if(item->giType == IT_POWERUP && item->giTag == PW_BELT){
		belt = 2;

		ent->client->ps.ammo[WP_BULLETS_CLIP] = bg_weaponlist[WP_REM58].maxAmmo*belt;
		ent->client->ps.ammo[WP_SHELLS_CLIP] = bg_weaponlist[WP_REMINGTON_GAUGE].maxAmmo*belt;
		ent->client->ps.ammo[WP_CART_CLIP] = bg_weaponlist[WP_WINCHESTER66].maxAmmo*belt;
		ent->client->ps.ammo[WP_GATLING_CLIP] = bg_weaponlist[WP_GATLING].maxAmmo*belt;
		ent->client->ps.ammo[WP_SHARPS_CLIP] = bg_weaponlist[WP_SHARPS].maxAmmo*belt;
	}
	ent->client->ps.stats[STAT_MONEY] -= prize;
}
#endif


/*
=================
ClientCommand
=================
*/
void ClientCommand( int clientNum ) {
	gentity_t *ent;
	char	cmd[MAX_TOKEN_CHARS];

	ent = g_entities + clientNum;
	if ( !ent->client ) {
		return;		// not fully in game yet
	}


	trap_Argv( 0, cmd, sizeof( cmd ) );

	if (Q_stricmp (cmd, "say") == 0) {
		Cmd_Say_f (ent, SAY_ALL, qfalse);
		return;
	}
	if (Q_stricmp (cmd, "say_team") == 0) {
		Cmd_Say_f (ent, SAY_TEAM, qfalse);
		return;
	}
	if (Q_stricmp (cmd, "tell") == 0) {
		Cmd_Tell_f ( ent );
		return;
	}
#ifndef SMOKINGUNS
	if (Q_stricmp (cmd, "vsay") == 0) {
		Cmd_Voice_f (ent, SAY_ALL, qfalse, qfalse);
		return;
	}
	if (Q_stricmp (cmd, "vsay_team") == 0) {
		Cmd_Voice_f (ent, SAY_TEAM, qfalse, qfalse);
		return;
	}
	if (Q_stricmp (cmd, "vtell") == 0) {
		Cmd_VoiceTell_f ( ent, qfalse );
		return;
	}
	if (Q_stricmp (cmd, "vosay") == 0) {
		Cmd_Voice_f (ent, SAY_ALL, qfalse, qtrue);
		return;
	}
	if (Q_stricmp (cmd, "vosay_team") == 0) {
		Cmd_Voice_f (ent, SAY_TEAM, qfalse, qtrue);
		return;
	}
	if (Q_stricmp (cmd, "votell") == 0) {
		Cmd_VoiceTell_f ( ent, qtrue );
		return;
	}
	if (Q_stricmp (cmd, "vtaunt") == 0) {
		Cmd_VoiceTaunt_f ( ent );
		return;
	}
#endif
	if (Q_stricmp (cmd, "score") == 0) {
		Cmd_Score_f (ent);
		return;
	}

	// ignore all other commands when at intermission
	if (level.intermissiontime) {
		Cmd_Say_f (ent, qfalse, qtrue);
		return;
	}

	if (Q_stricmp (cmd, "give") == 0)
		Cmd_Give_f (ent);
	else if (Q_stricmp (cmd, "god") == 0)
		Cmd_God_f (ent);
	else if (Q_stricmp (cmd, "notarget") == 0)
		Cmd_Notarget_f (ent);
	else if (Q_stricmp (cmd, "noclip") == 0)
		Cmd_Noclip_f (ent);
	else if (Q_stricmp (cmd, "kill") == 0)
		Cmd_Kill_f (ent);
	else if (Q_stricmp (cmd, "teamtask") == 0)
		Cmd_TeamTask_f (ent);
	else if (Q_stricmp (cmd, "levelshot") == 0)
		Cmd_LevelShot_f (ent);
	else if (Q_stricmp (cmd, "follow") == 0)
		Cmd_Follow_f (ent);
	else if (Q_stricmp (cmd, "follownext") == 0)
#ifndef SMOKINGUNS
		Cmd_FollowCycle_f (ent, 1);
#else
		ChaseCam_Start(ent, 1);
#endif
	else if (Q_stricmp (cmd, "followprev") == 0)
#ifndef SMOKINGUNS
		Cmd_FollowCycle_f (ent, -1);
#else
		ChaseCam_Start(ent, -1);
#endif
	else if (Q_stricmp (cmd, "team") == 0)
		Cmd_Team_f (ent);
	else if (Q_stricmp (cmd, "where") == 0)
		Cmd_Where_f (ent);
	else if (Q_stricmp (cmd, "callvote") == 0)
		Cmd_CallVote_f (ent);
	else if (Q_stricmp (cmd, "vote") == 0)
		Cmd_Vote_f (ent);
	else if (Q_stricmp (cmd, "callteamvote") == 0)
		Cmd_CallTeamVote_f (ent);
	else if (Q_stricmp (cmd, "teamvote") == 0)
		Cmd_TeamVote_f (ent);
	else if (Q_stricmp (cmd, "gc") == 0)
		Cmd_GameCommand_f( ent );
	else if (Q_stricmp (cmd, "setviewpos") == 0)
		Cmd_SetViewpos_f( ent );
	else if (Q_stricmp (cmd, "stats") == 0)
		Cmd_Stats_f( ent );
#ifdef SMOKINGUNS
	else if (Q_stricmp (cmd, "dropweapon") == 0)
		Cmd_DropWeapon_f( ent, 0 );
	else if (Q_stricmp(cmd, "buy" ) == 0)
		Cmd_BuyItem_f (ent, qfalse);
	else if (Q_stricmp(cmd, "cg_buy" ) == 0)
		Cmd_BuyItem_f (ent, qtrue);
	//developer cheats by spoon, necessary to join in round based gametypes
	else if(!Q_stricmp(cmd, "dev_join_r")){
		if(CheatsOk( ent ))
			SetTeam(ent, "r");
	}
	else if(!Q_stricmp(cmd, "dev_join_b")){
		if(CheatsOk( ent ))
			SetTeam(ent, "b");
	}
#endif
	else
		trap_SendServerCommand( clientNum, va("print \"unknown cmd %s\n\"", cmd ) );
}
