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
#include "g_local.h"

// g_client.c -- client functions that don't happen every frame

#ifndef SMOKINGUNS
static vec3_t	playerMins = {-15, -15, -24};
static vec3_t	playerMaxs = {15, 15, 32};
#endif

/*QUAKED info_player_deathmatch (1 0 1) (-16 -16 -24) (16 16 32) initial
potential spawning position for deathmatch games.
The first time a player enters the game, they will be at an 'initial' spot.
Targets will be fired when someone spawns in on them.
"nobots" will prevent bots from using this spot.
"nohumans" will prevent non-bots from using this spot.
*/
void SP_info_player_deathmatch( gentity_t *ent ) {
	int		i;

	G_SpawnInt( "nobots", "0", &i);
	if ( i ) {
		ent->flags |= FL_NO_BOTS;
	}
	G_SpawnInt( "nohumans", "0", &i );
	if ( i ) {
		ent->flags |= FL_NO_HUMANS;
	}
}

/*QUAKED info_player_start (1 0 0) (-16 -16 -24) (16 16 32)
equivelant to info_player_deathmatch
*/
void SP_info_player_start(gentity_t *ent) {
	ent->classname = "info_player_deathmatch";
	SP_info_player_deathmatch( ent );
}

/*QUAKED info_player_intermission (1 0 1) (-16 -16 -24) (16 16 32)
The intermission will be viewed from this point.  Target an info_notnull for the view direction.
*/
void SP_info_player_intermission( gentity_t *ent ) {

#ifdef SMOKINGUNS
	if(g_gametype.integer != GT_DUEL)
		return;

	// add this so that cgame also knows about this entity (needed for duel camera moves)
	VectorCopy( ent->s.origin, ent->pos1 );

	ent->s.eType = ET_INTERMISSION;
	G_SpawnInt( "part", "0", &ent->s.eventParm);

	trap_LinkEntity(ent);
#endif
}
/*
==========================
G_AnimLength
by Spoon
==========================
*/
#ifdef SMOKINGUNS
int G_AnimLength( int anim, int weapon) {
	int length;

	length = (bg_weaponlist[weapon].animations[anim].numFrames-1)*
		bg_weaponlist[weapon].animations[anim].frameLerp+
		bg_weaponlist[weapon].animations[anim].initialLerp;

	if(bg_weaponlist[weapon].animations[anim].flipflop){
		length *= 2;
		length -= bg_weaponlist[weapon].animations[anim].initialLerp;
	}

	return length;
}
#endif



/*
=======================================================================

  SelectSpawnPoint

=======================================================================
*/

/*
================
SpotWouldTelefrag

================
*/
qboolean SpotWouldTelefrag( gentity_t *spot ) {
	int			i, num;
	int			touch[MAX_GENTITIES];
	gentity_t	*hit;
	vec3_t		mins, maxs;

	VectorAdd( spot->s.origin, playerMins, mins );
	VectorAdd( spot->s.origin, playerMaxs, maxs );
	num = trap_EntitiesInBox( mins, maxs, touch, MAX_GENTITIES );

	for (i=0 ; i<num ; i++) {
		hit = &g_entities[touch[i]];
		//if ( hit->client && hit->client->ps.stats[STAT_HEALTH] > 0 ) {
		if ( hit->client) {
			return qtrue;
		}

	}

	return qfalse;
}

/*
================
SelectNearestDeathmatchSpawnPoint

Find the spot that we DON'T want to use
================
*/
#define	MAX_SPAWN_POINTS	128
gentity_t *SelectNearestDeathmatchSpawnPoint( vec3_t from ) {
	gentity_t	*spot;
	vec3_t		delta;
	float		dist, nearestDist;
	gentity_t	*nearestSpot;

	nearestDist = 999999;
	nearestSpot = NULL;
	spot = NULL;

	while ((spot = G_Find (spot, FOFS(classname), "info_player_deathmatch")) != NULL) {

		VectorSubtract( spot->s.origin, from, delta );
		dist = VectorLength( delta );
		if ( dist < nearestDist ) {
			nearestDist = dist;
			nearestSpot = spot;
		}
	}

	return nearestSpot;
}


/*
================
SelectRandomDeathmatchSpawnPoint

go to a random point that doesn't telefrag
================
*/
#define	MAX_SPAWN_POINTS	128
gentity_t *SelectRandomDeathmatchSpawnPoint(qboolean isbot) {
	gentity_t	*spot;
	int			count;
	int			selection;
	gentity_t	*spots[MAX_SPAWN_POINTS];

	count = 0;
	spot = NULL;

	while((spot = G_Find (spot, FOFS(classname), "info_player_deathmatch")) != NULL && count < MAX_SPAWN_POINTS)
	{
		if(SpotWouldTelefrag(spot))
			continue;

		if(((spot->flags & FL_NO_BOTS) && isbot) ||
		   ((spot->flags & FL_NO_HUMANS) && !isbot))
		{
			// spot is not for this human/bot player
			continue;
		}

		spots[ count ] = spot;
		count++;
	}

	if ( !count ) {	// no spots that won't telefrag
		return G_Find( NULL, FOFS(classname), "info_player_deathmatch");
	}

	selection = rand() % count;
	return spots[ selection ];
}

/*
===========
SelectRandomFurthestSpawnPoint

Chooses a player start, deathmatch start, etc
============
*/
#ifndef SMOKINGUNS
gentity_t *SelectRandomFurthestSpawnPoint ( vec3_t avoidPoint, vec3_t origin, vec3_t angles, qboolean isbot ) {
#else
gentity_t *SelectRandomFurthestSpawnPoint ( vec3_t avoidPoint, vec3_t origin, vec3_t angles, qboolean isbot,
										   int mappart, gclient_t *client) {
#endif
	gentity_t	*spot;
	vec3_t		delta;
	float		dist;
	float		list_dist[MAX_SPAWN_POINTS];
	gentity_t	*list_spot[MAX_SPAWN_POINTS];
#ifndef SMOKINGUNS
	int			numSpots, rnd, i, j;
#else
	int			numSpots, rnd, i, j, ignoreTeam;
	gentity_t	*allSpot[MAX_SPAWN_POINTS];
	qboolean	telefragSpot[MAX_SPAWN_POINTS];
	int			availSpot[MAX_SPAWN_POINTS];
	int			numAllSpots = 0;
	int			numAvailSpots;
	qboolean perfectSpot = qfalse, telefrag;
	int developer;
#endif

	numSpots = 0;
	spot = NULL;

	while ((spot = G_Find (spot, FOFS(classname), "info_player_deathmatch")) != NULL) {
#ifndef SMOKINGUNS
		if ( SpotWouldTelefrag( spot ) ) {
			continue;
		}
#else
		telefrag = SpotWouldTelefrag( spot );
		if (g_gametype.integer != GT_DUEL) {
			if (numAllSpots < MAX_SPAWN_POINTS) {
				allSpot[ numAllSpots ] = spot;
				telefragSpot[ numAllSpots ] = telefrag;
				numAllSpots++;
			}
		}

		if ( telefrag && g_gametype.integer != GT_DUEL) {
			continue;
		}

		if(spot->mappart == mappart && g_gametype.integer == GT_DUEL && !spot->used){
			// if this is a trio player, find a trio-spawnpoint
			if(client && !spot->trio && client->trio)
				continue;

			spot->used = qtrue;
			VectorCopy(spot->s.origin, origin);
			VectorCopy(spot->s.angles, angles);

			// if it has a target, look towards it
			if ( spot->target ) {
				gentity_t *target;
				vec3_t	dir;

				target = G_PickTarget( spot->target );
				if ( target ) {
					VectorSubtract( target->s.origin, origin, dir );
					vectoangles( dir, angles );
				}
			}
			return spot;
			break;
		} else if(g_gametype.integer == GT_DUEL)
			continue;

		// spots are inserted in sorted order, furthest away first
#endif

		if(((spot->flags & FL_NO_BOTS) && isbot) ||
		   ((spot->flags & FL_NO_HUMANS) && !isbot))
		{
			// spot is not for this human/bot player
			continue;
		}

		VectorSubtract( spot->s.origin, avoidPoint, delta );
		dist = VectorLength( delta );

		for (i = 0; i < numSpots; i++)
		{
			if(dist > list_dist[i])
			{
				if (numSpots >= MAX_SPAWN_POINTS)
					numSpots = MAX_SPAWN_POINTS - 1;

				for(j = numSpots; j > i; j--)
				{
					list_dist[j] = list_dist[j-1];
					list_spot[j] = list_spot[j-1];
				}

				list_dist[i] = dist;
				list_spot[i] = spot;

				numSpots++;
				break;
			}
		}

		if(i >= numSpots && numSpots < MAX_SPAWN_POINTS)
		{
			list_dist[numSpots] = dist;
			list_spot[numSpots] = spot;
			numSpots++;
		}
	}

#ifdef SMOKINGUNS
	if(g_gametype.integer == GT_DUEL){
		G_Printf("bug! couldn't find duel-spawnpoint!\n");
		return NULL;
	}
#endif

	if (!numSpots)
	{
		spot = G_Find( NULL, FOFS(classname), "info_player_deathmatch");

		if (!spot)
			G_Error( "Couldn't find a spawn point" );

		VectorCopy (spot->s.origin, origin);
#ifndef SMOKINGUNS
		origin[2] += 9;
#endif
		VectorCopy (spot->s.angles, angles);
		return spot;
	}

#ifndef SMOKINGUNS
	// select a random spot from the spawn points furthest away
	rnd = random() * (numSpots / 2);

	VectorCopy (list_spot[rnd]->s.origin, origin);
	origin[2] += 9;
	VectorCopy (list_spot[rnd]->s.angles, angles);

	return list_spot[rnd];
#else
	ignoreTeam = (g_gametype.integer == GT_TEAM)
	             ? client->ps.persistant[PERS_TEAM] : -1;

	// try find a spawn point that's not too close to another player
	rnd = random() * (numSpots / 2);
	for ( i = rnd; i >= 0; i-- ) {
		spot = list_spot[i];
		if ( !G_IsAnyClientWithinRadius(spot->s.origin, 700, ignoreTeam) ) {
			// Check if the spawn point is appropriate for the bot or human player.
			// If not, try another one.
			if (((spot->flags & FL_NO_BOTS) && isbot) ||
				((spot->flags & FL_NO_HUMANS) && !isbot))
				continue;
			perfectSpot = qtrue;
			rnd = i;
			break;
		}
	}
	if ( i < 0 ) {
		// try the rest, except the worst one
		int highest = numSpots - 2;
		for ( i = rnd+1; i <= highest; i++ ) {
			spot = list_spot[i];
			if ( !G_IsAnyClientWithinRadius(spot->s.origin, 700, ignoreTeam) ) {
				// Check if the spawn point is appropriate for the bot or human player.
				// If not, try another one.
				if (((spot->flags & FL_NO_BOTS) && isbot) ||
					((spot->flags & FL_NO_HUMANS) && !isbot))
					continue;
				perfectSpot = qtrue;
				rnd = i;
				break;
			}
		}
	}

	// No valid spawn point is found.
	// By valid, we mean an allowed spawn point for bot or human AND
	// no client within a radius of 700 units
	if ( !perfectSpot && (((list_spot[rnd]->flags & FL_NO_BOTS) && isbot) ||
		((list_spot[rnd]->flags & FL_NO_HUMANS) && !isbot)) ) {
		developer = trap_Cvar_VariableIntegerValue("developer");

		numAvailSpots = 0;
		for (i = 0; i < numSpots; i++) {
			spot = list_spot[i];
			if (((spot->flags & FL_NO_BOTS) && isbot) ||
				((spot->flags & FL_NO_HUMANS) && !isbot))
				continue;
			availSpot[ numAvailSpots ] = i;
			numAvailSpots++;
		}
		// At least one spot *SHOULD* be available
		if (numAvailSpots > 0) {
			i = random() * (numAvailSpots-1);
			rnd = availSpot[i];
			spot = list_spot[rnd];

			if ( developer ) {
				G_Printf("Error: Invalid %s spawn point\n", isbot ? "bot" : "human");
				G_Printf("Fix: a valid spawn point has been elected. Is any client within radius ? %s\n",
					G_IsAnyClientWithinRadius(spot->s.origin, 700, ignoreTeam) ? "Yes" : "No");
			}
		}
		else {
			// We *SHOULD NOT* be here, or we may experience a real spawn policy problem !
			// Mostly happen, when there is no place to spawn, typically because telefrag would happen.
			// -> Should we spawn in the place with the risk to telefrag somebody else ?
			// -> Yes, as it is better than an infinite loop ;)
			numAvailSpots = 0;
			if ( developer ) {
				G_Printf("Error: Invalid %s spawn point\n", isbot ? "bot" : "human");
				for (i = 0; i < numAllSpots; i++) {
					spot = allSpot[i];
					if ( (spot->flags & FL_NO_BOTS) && isbot )
						G_Printf("DumpInfo: spawnPoint[%i]: not allowed for you, bot, telefrag: %s\n", i, telefragSpot[i] ? "Yes" : "No");
					else if ( (spot->flags & FL_NO_HUMANS) && !isbot )
						G_Printf("DumpInfo: spawnPoint[%i]: not allowed for you, human, telefrag: %s\n", i, telefragSpot[i] ? "Yes" : "No");
					else
						G_Printf("DumpInfo: spawnPoint[%i]: allowed, telefrag: %s\n", i, telefragSpot[i] ? "Yes" : "No");

					if (((spot->flags & FL_NO_BOTS) && isbot) ||
						((spot->flags & FL_NO_HUMANS) && !isbot))
						continue;
					availSpot[ numAvailSpots ] = i;
					numAvailSpots++;
				}
			}
			else {
				for (i = 0; i < numAllSpots; i++) {
					spot = allSpot[i];
					if (((spot->flags & FL_NO_BOTS) && isbot) ||
						((spot->flags & FL_NO_HUMANS) && !isbot))
						continue;
					availSpot[ numAvailSpots ] = i;
					numAvailSpots++;
				}
			}

			if (numAvailSpots > 0) {
				// We get all possible spawn points, even if telefrag would happen.
				// Get a random one
				i = random() * (numAvailSpots-1);
				rnd = availSpot[i];
				spot = allSpot[rnd];

				if ( developer )
					G_Printf("Fix: spawnPoint[%i] has been elected !\n", rnd);
			}
			else {
				// Well, I can't do anything more ... :/.
				// Should be really a spawn problem, and
				// maybe FL_NO_BOTS and FL_NO_HUMANS have to be removed on *ALL* spawn points.
				spot = list_spot[rnd];

				if ( developer )
					G_Printf("Fatal: %s spawn problem ! Maybe FL_NO_BOTS and FL_NO_HUMANS have to be removed on all spawn points ??\n",
						isbot ? "bot" : "human");
			}
		}
	}
	else // Normal process, the random selected spawn point is OK
		spot = list_spot[rnd];

	VectorCopy (spot->s.origin, origin);
	VectorCopy (spot->s.angles, angles);

	// if it has a target, look towards it
	if ( spot->target ) {
		gentity_t *target;
		vec3_t	dir;

		target = G_PickTarget( spot->target );
		if ( target ) {
			VectorSubtract( target->s.origin, origin, dir );
			vectoangles( dir, angles );
		}
	}

	return spot;
#endif
}

/*
===========
SelectSpawnPoint

Chooses a player start, deathmatch start, etc
============
*/
#ifndef SMOKINGUNS
gentity_t *SelectSpawnPoint ( vec3_t avoidPoint, vec3_t origin, vec3_t angles, qboolean isbot ) {
	return SelectRandomFurthestSpawnPoint( avoidPoint, origin, angles, isbot );
#else
gentity_t *SelectSpawnPoint ( vec3_t avoidPoint, vec3_t origin, vec3_t angles, qboolean isbot, int mappart, gclient_t *client ) {
	return SelectRandomFurthestSpawnPoint( avoidPoint, origin, angles, isbot, mappart, client );
#endif

	/*
	gentity_t	*spot;
	gentity_t	*nearestSpot;

	nearestSpot = SelectNearestDeathmatchSpawnPoint( avoidPoint );

	spot = SelectRandomDeathmatchSpawnPoint ( );
	if ( spot == nearestSpot ) {
		// roll again if it would be real close to point of death
		spot = SelectRandomDeathmatchSpawnPoint ( );
		if ( spot == nearestSpot ) {
			// last try
			spot = SelectRandomDeathmatchSpawnPoint ( );
		}
	}

	// find a single player start spot
	if (!spot) {
		G_Error( "Couldn't find a spawn point" );
	}

	VectorCopy (spot->s.origin, origin);
	origin[2] += 9;
	VectorCopy (spot->s.angles, angles);

	return spot;
	*/
}

/*
===========
SelectInitialSpawnPoint

Try to find a spawn point marked 'initial', otherwise
use normal spawn selection.
============
*/
#ifndef SMOKINGUNS
gentity_t *SelectInitialSpawnPoint( vec3_t origin, vec3_t angles, qboolean isbot ) {
#else
gentity_t *SelectInitialSpawnPoint( vec3_t origin, vec3_t angles, qboolean isbot, int mappart, gclient_t *client) {
#endif
	gentity_t	*spot;

	spot = NULL;

	while ((spot = G_Find (spot, FOFS(classname), "info_player_deathmatch")) != NULL)
	{
		if(((spot->flags & FL_NO_BOTS) && isbot) ||
		   ((spot->flags & FL_NO_HUMANS) && !isbot))
		{
			continue;
		}

		if((spot->spawnflags & 0x01))
			break;
	}

	if (!spot || SpotWouldTelefrag(spot))
#ifndef SMOKINGUNS
		return SelectSpawnPoint(vec3_origin, origin, angles, isbot);
#else
		return SelectSpawnPoint( vec3_origin, origin, angles, isbot, mappart, client );
#endif

	VectorCopy (spot->s.origin, origin);
#ifndef SMOKINGUNS
	origin[2] += 9;
#endif
	VectorCopy (spot->s.angles, angles);

	return spot;
}

/*
===========
SelectSpectatorSpawnPoint

============
*/
#ifndef SMOKINGUNS
gentity_t *SelectSpectatorSpawnPoint( vec3_t origin, vec3_t angles ) {
	FindIntermissionPoint();
#else
gentity_t *SelectSpectatorSpawnPoint( vec3_t origin, vec3_t angles, int mappart) {
	FindIntermissionPoint(mappart);
#endif

	VectorCopy( level.intermission_origin, origin );
	VectorCopy( level.intermission_angle, angles );

	return NULL;
}

/*
=======================================================================

BODYQUE

=======================================================================
*/

/*
===============
InitBodyQue
===============
*/
void InitBodyQue (void) {
	int		i;
	gentity_t	*ent;

	level.bodyQueIndex = 0;
	for (i=0; i<BODY_QUEUE_SIZE ; i++) {
		ent = G_Spawn();
		ent->classname = "bodyque";
		ent->neverFree = qtrue;
		level.bodyQue[i] = ent;
	}
}

#ifndef SMOKINGUNS
/*
=============
BodySink

After sitting around for five seconds, fall into the ground and dissapear
=============
*/
void BodySink( gentity_t *ent ) {
	if ( level.time - ent->timestamp > 6500 ) {
		// the body ques are never actually freed, they are just unlinked
		trap_UnlinkEntity( ent );
		ent->physicsObject = qfalse;
		return;
	}
	ent->nextthink = level.time + 100;
	ent->s.pos.trBase[2] -= 1;
}
#else
/*
=============
BodySink

After sitting around for ten seconds, fall into the ground and dissapear
=============
*/
void BodySink( gentity_t *ent ) {
	qboolean duel = (g_gametype.integer == GT_DUEL);

	// make sure no body is left at round restart
	if((g_gametype.integer >= GT_RTP && g_round > ent->round_or_duel) ||
		(g_gametype.integer == GT_DUEL && g_session != ent->round_or_duel)){
		ent->physicsObject = qfalse;
		trap_UnlinkEntity( ent );
		return;
	}

	// don't do anything
	if(level.time - ent->timestamp < 10000 && !duel){
		ent->nextthink = level.time + 50;
		return;
	}

	if ( level.time - ent->timestamp > 15000 && !duel) {
		// the body ques are never actually freed, they are just unlinked
		trap_UnlinkEntity( ent );
		ent->physicsObject = qfalse;
		return;
	}

	ent->nextthink = level.time + 50;

	if(!duel){
		ent->s.pos.trBase[2] -= 1;
	}
}
#endif

/*
=============
CopyToBodyQue

A player is respawning, so make an entity that looks
just like the existing corpse to leave behind.
=============
*/
void CopyToBodyQue( gentity_t *ent ) {
#ifndef SMOKINGUNS
	gentity_t	*e;
	int i;
#endif
	gentity_t		*body;
	int			contents;

	trap_UnlinkEntity (ent);

	// if client is in a nodrop area, don't leave the body
	contents = trap_PointContents( ent->s.origin, -1 );
	if ( contents & CONTENTS_NODROP ) {
		return;
	}

	// grab a body que and cycle to the next one
	body = level.bodyQue[ level.bodyQueIndex ];
	level.bodyQueIndex = (level.bodyQueIndex + 1) % BODY_QUEUE_SIZE;

	body->s = ent->s;
#ifdef SMOKINGUNS
	body->s.eType = ET_PLAYER;
#endif
	body->s.eFlags = EF_DEAD;		// clear EF_TALK, etc
#ifndef SMOKINGUNS
	if ( ent->s.eFlags & EF_KAMIKAZE ) {
		body->s.eFlags |= EF_KAMIKAZE;

		// check if there is a kamikaze timer around for this owner
		for (i = 0; i < level.num_entities; i++) {
			e = &g_entities[i];
			if (!e->inuse)
				continue;
			if (e->activator != ent)
				continue;
			if (strcmp(e->classname, "kamikaze timer"))
				continue;
			e->activator = body;
			break;
		}
	}
#endif
#ifndef SMOKINGUNS
	body->s.powerups = 0;	// clear powerups
#else
	body->s.powerups &= ~(1 << PW_GOLD);
#endif
	body->s.loopSound = 0;	// clear lava burning
	body->s.number = body - g_entities;
	body->timestamp = level.time;
	body->physicsObject = qtrue;
	body->physicsBounce = 0;		// don't bounce
	if ( body->s.groundEntityNum == ENTITYNUM_NONE ) {
		body->s.pos.trType = TR_GRAVITY;
		body->s.pos.trTime = level.time;
		VectorCopy( ent->client->ps.velocity, body->s.pos.trDelta );
	} else {
		body->s.pos.trType = TR_STATIONARY;
	}
	body->s.event = 0;

#ifndef SMOKINGUNS
	// change the animation to the last-frame only, so the sequence
	// doesn't repeat anew for the body
	switch ( body->s.legsAnim & ~ANIM_TOGGLEBIT ) {
	case BOTH_DEATH1:
	case BOTH_DEAD1:
		body->s.torsoAnim = body->s.legsAnim = BOTH_DEAD1;
		break;
	case BOTH_DEATH2:
	case BOTH_DEAD2:
		body->s.torsoAnim = body->s.legsAnim = BOTH_DEAD2;
		break;
	case BOTH_DEATH3:
	case BOTH_DEAD3:
	default:
		body->s.torsoAnim = body->s.legsAnim = BOTH_DEAD3;
		break;
	}
#else
	if(g_gametype.integer != GT_DUEL || !ent->client->specwatch){
		// change the animation to the last-frame only, so the sequence
		// doesn't repeat anew for the body
		switch ( body->s.legsAnim & ~ANIM_TOGGLEBIT ) {
		case BOTH_DEATH_HEAD:
		case BOTH_DEAD_HEAD:
			body->s.torsoAnim = body->s.legsAnim = BOTH_DEAD_HEAD;
			break;
		case BOTH_DEATH_ARM_L:
		case BOTH_DEAD_ARM_L:
			body->s.torsoAnim = body->s.legsAnim = BOTH_DEAD_ARM_L;
			break;
		case BOTH_DEATH_ARM_R:
		case BOTH_DEAD_ARM_R:
			body->s.torsoAnim = body->s.legsAnim = BOTH_DEAD_ARM_R;
			break;
		case BOTH_DEATH_CHEST:
		case BOTH_DEAD_CHEST:
			body->s.torsoAnim = body->s.legsAnim = BOTH_DEAD_CHEST;
			break;
		case BOTH_DEATH_STOMACH:
		case BOTH_DEAD_STOMACH:
			body->s.torsoAnim = body->s.legsAnim = BOTH_DEAD_STOMACH;
			break;
		case BOTH_DEATH_LEG_L:
		case BOTH_DEAD_LEG_L:
			body->s.torsoAnim = body->s.legsAnim = BOTH_DEAD_LEG_L;
			break;
		case BOTH_DEATH_LEG_R:
		case BOTH_DEAD_LEG_R:
			body->s.torsoAnim = body->s.legsAnim = BOTH_DEAD_LEG_R;
			break;
		case BOTH_FALL:
		case BOTH_FALL_BACK:
		case BOTH_DEATH_LAND:
		case BOTH_DEAD_LAND:
			body->s.torsoAnim = body->s.legsAnim = BOTH_DEAD_LAND;
			break;
		default:
			body->s.torsoAnim = body->s.legsAnim = BOTH_DEAD_DEFAULT;
			break;
		}
	} else {

		body->s.torsoAnim = ent->s.torsoAnim;
		body->s.legsAnim = LEGS_IDLE;
	}
#endif

	body->r.svFlags = ent->r.svFlags;
	VectorCopy (ent->r.mins, body->r.mins);
	VectorCopy (ent->r.maxs, body->r.maxs);
	VectorCopy (ent->r.absmin, body->r.absmin);
	VectorCopy (ent->r.absmax, body->r.absmax);

	body->clipmask = CONTENTS_SOLID | CONTENTS_PLAYERCLIP;
	body->r.contents = CONTENTS_CORPSE;
	body->r.ownerNum = ent->s.number;

#ifndef SMOKINGUNS
	body->nextthink = level.time + 5000;
#else
	body->nextthink = level.time + 4000;
#endif
	body->think = BodySink;

#ifndef SMOKINGUNS
	body->die = body_die;
#else
	body->die = 0;
#endif

	// don't take more damage if already gibbed
#ifndef SMOKINGUNS
	if ( ent->health <= GIB_HEALTH ) {
		body->takedamage = qfalse;
	} else {
		body->takedamage = qtrue;
	}
#else
	body->takedamage = qfalse;

	if( g_gametype.integer >= GT_RTP)
		body->round_or_duel = g_round;
	else if( g_gametype.integer == GT_DUEL)
		body->round_or_duel = g_session;
#endif


	VectorCopy ( body->s.pos.trBase, body->r.currentOrigin );
	trap_LinkEntity (body);
}

//======================================================================


/*
==================
SetClientViewAngle

==================
*/
void SetClientViewAngle( gentity_t *ent, vec3_t angle ) {
	int			i;

	// set the delta angle
	for (i=0 ; i<3 ; i++) {
		int		cmdAngle;

		cmdAngle = ANGLE2SHORT(angle[i]);
		ent->client->ps.delta_angles[i] = cmdAngle - ent->client->pers.cmd.angles[i];
	}
	VectorCopy( angle, ent->s.angles );
	VectorCopy (ent->s.angles, ent->client->ps.viewangles);
}

/*
================
ClientRespawn
================
*/
void ClientRespawn( gentity_t *ent ) {

	CopyToBodyQue (ent);
	ClientSpawn(ent);
}

/*
================
TeamCount

Returns number of players on a team
================
*/
int TeamCount( int ignoreClientNum, team_t team ) {
	int		i;
	int		count = 0;

	for ( i = 0 ; i < level.maxclients ; i++ ) {
		if ( i == ignoreClientNum ) {
			continue;
		}
		if ( level.clients[i].pers.connected == CON_DISCONNECTED ) {
			continue;
		}

#ifdef SMOKINGUNS
		if ( (g_gametype.integer >= GT_RTP || g_gametype.integer == GT_DUEL) &&
			level.clients[i].pers.connected != CON_CONNECTED )
			continue;

		if(g_gametype.integer == GT_DUEL && team == TEAM_SPECTATOR &&
			g_entities[i].client->realspec)
			continue;
#endif

		if ( level.clients[i].sess.sessionTeam == team ) {
			count++;
		}
	}

	return count;
}

#ifdef SMOKINGUNS
int MappartCount( int ignoreClientNum, int team ) {
	int		i;
	int		count = 0;

	for ( i = 0 ; i < level.maxclients ; i++ ) {
		if ( i == ignoreClientNum ) {
			continue;
		}
		if ( level.clients[i].pers.connected == CON_DISCONNECTED ) {
			continue;
		}

		if ( (g_gametype.integer >= GT_RTP || g_gametype.integer == GT_DUEL) &&
			level.clients[i].pers.connected != CON_CONNECTED )
			continue;

		if(g_gametype.integer == GT_DUEL && team == TEAM_SPECTATOR &&
			g_entities[i].client->realspec)
			continue;

		if ( level.clients[i].sess.sessionTeam == team ) {
			count++;
		}
	}

	return count;
}
#endif

/*
================
TeamLeader

Returns the client number of the team leader
================
*/
int TeamLeader( int team ) {
	int		i;

	for ( i = 0 ; i < level.maxclients ; i++ ) {
		if ( level.clients[i].pers.connected == CON_DISCONNECTED ) {
			continue;
		}
		if ( level.clients[i].sess.sessionTeam == team ) {
			if ( level.clients[i].sess.teamLeader )
				return i;
		}
	}

	return -1;
}


/*
================
PickTeam

================
*/
team_t PickTeam( int ignoreClientNum ) {
	int		counts[TEAM_NUM_TEAMS];

#ifndef SMOKINGUNS
	counts[TEAM_BLUE] = TeamCount( ignoreClientNum, TEAM_BLUE );
	counts[TEAM_RED] = TeamCount( ignoreClientNum, TEAM_RED );
#else
	// Joe Kari: TeamCount ignore TEAM_X_SPECTATOR if called for TEAM_X
	counts[TEAM_BLUE] = TeamCount( ignoreClientNum, TEAM_BLUE ) + TeamCount( ignoreClientNum, TEAM_BLUE_SPECTATOR ) ;
	counts[TEAM_RED] = TeamCount( ignoreClientNum, TEAM_RED ) + TeamCount( ignoreClientNum, TEAM_RED_SPECTATOR ) ;
#endif
	
	if ( counts[TEAM_BLUE] > counts[TEAM_RED] ) {
		return TEAM_RED;
	}
	if ( counts[TEAM_RED] > counts[TEAM_BLUE] ) {
		return TEAM_BLUE;
	}
	// equal team count, so join the team with the lowest score
	if ( level.teamScores[TEAM_BLUE] > level.teamScores[TEAM_RED] ) {
		return TEAM_RED;
	}
	return TEAM_BLUE;
}

/*
===========
ForceUniqueName

Tequila: Forces client's name to be unique
===========
*/
#ifdef SMOKINGUNS
static qboolean ForceUniqueName( int clientNum ) {
	int	i, star=0, len;
	char tag[MAX_NAME_LENGTH];
	char name[MAX_NAME_LENGTH];

	Q_strncpyz( name, level.clients[clientNum].pers.netname, sizeof(name) );
check_again:
	Q_strncpyz( level.clients[clientNum].pers.cleanname, level.clients[clientNum].pers.netname, sizeof(level.clients[clientNum].pers.cleanname) );
	Q_CleanStr( level.clients[clientNum].pers.cleanname );
	len = strlen(name);

	for ( i = 0 ; i < level.maxclients ; i++ ) {
		if ( i == clientNum ) {
			continue;
		}
		if ( level.clients[i].pers.connected == CON_DISCONNECTED ) {
			continue;
		}
		// We compare on the cleanname as this is a guarantee to avoid confusion between players
		if (Q_stricmp(level.clients[i].pers.cleanname,level.clients[clientNum].pers.cleanname)) {
			continue;
		}

		// Prepare the tag
		if (star<sizeof(tag)-2){ // Keep at least one letter from name
			tag[star]='*';
			tag[star+1]='\0';
		} else
			return qfalse;

		// Check if we need to truncate the name
		if (len+star+2>sizeof(name)-1) {
			len=sizeof(name)-star-3 ;
			if (len<0)
				return qfalse;
			name[len]='\0';
		}

		// Tag the name
		Com_sprintf(level.clients[clientNum].pers.netname,sizeof(level.clients[clientNum].pers.netname),"%s %s",name,tag);

		// Then we need to scan again all the names, eventually adding a new star if this tagged name is still used
		star++;
		goto check_again ;
	}
	return qtrue;
}

/*
===========
ForceClientSkin

Forces a client's skin (for teamplay)
===========
*/
static void ForceClientSkin( gclient_t *client, char *model, const char *skin ) {
	char *p;

	if ((p = strrchr(model, '/')) != 0) {
		*p = 0;
	}

	Q_strcat(model, MAX_QPATH, "/");
	Q_strcat(model, MAX_QPATH, skin);
}
#endif

/*
===========
ClientCleanName
============
*/
static void ClientCleanName(const char *in, char *out, int outSize)
{
	int outpos = 0, colorlessLen = 0, spaces = 0;

	// discard leading spaces
	for(; *in == ' '; in++);

	for(; *in && outpos < outSize - 1; in++)
	{
		out[outpos] = *in;

		if(*in == ' ')
		{
			// don't allow too many consecutive spaces
			if(spaces > 2)
				continue;

			spaces++;
		}
		else if(outpos > 0 && out[outpos - 1] == Q_COLOR_ESCAPE)
		{
			if(Q_IsColorString(&out[outpos - 1]))
			{
				colorlessLen--;

				if(ColorIndex(*in) == 0)
				{
					// Disallow color black in names to prevent players
					// from getting advantage playing in front of black backgrounds
					outpos--;
					continue;
				}
			}
			else
			{
				spaces = 0;
				colorlessLen++;
			}
		}
		else
		{
			spaces = 0;
			colorlessLen++;
		}

		outpos++;
	}

#ifdef SMOKINGUNS
	// Tequila: Discard trailing spaces and stars (stars are reserved at this position)
	while ( outpos && ( (out[outpos-1] == ' ') || (out[outpos-1] == '*') ) )
		outpos--;
#endif

	out[outpos] = '\0';

#ifndef SMOKINGUNS
	// don't allow empty names
	if( *out == '\0' || colorlessLen == 0)
#else
	// Tequila: Don't allow leading numbers as it can confuse ClientForString() in g_svcmds.c
	while ( *out != '\0' && *out >= '0' && *out <= '9' )
		Q_strncpyz( out, out+1, outSize );

	// Tequila: Don't allow empty names and reserved word
	if( *out == '\0' || colorlessLen == 0 || !Q_stricmp(out,"all") || !Q_stricmp(out,"allbots") )
#endif
		Q_strncpyz(out, "UnnamedPlayer", outSize );
}


/*
===========
ClientUserInfoChanged

Called from ClientConnect when the player first connects and
directly by the server system when the player updates a userinfo variable.

The game can override any of the settings and call trap_SetUserinfo
if desired.
============
*/
void ClientUserinfoChanged( int clientNum ) {
	gentity_t *ent;
	int		teamTask, teamLeader, team, health;
	char	*s;
	char	model[MAX_QPATH];
	char	headModel[MAX_QPATH];
	char	oldname[MAX_STRING_CHARS];
	gclient_t	*client;
#ifndef SMOKINGUNS
	char	c1[MAX_INFO_STRING];
	char	c2[MAX_INFO_STRING];
#endif
	char	redTeam[MAX_INFO_STRING];
	char	blueTeam[MAX_INFO_STRING];
	char	userinfo[MAX_INFO_STRING];
#ifdef SMOKINGUNS
	char	version[MAX_INFO_STRING];
	char	md5[MAX_INFO_STRING];
#endif

	ent = g_entities + clientNum;
	client = ent->client;

	trap_GetUserinfo( clientNum, userinfo, sizeof( userinfo ) );

	// check for malformed or illegal info strings
	if ( !Info_Validate(userinfo) ) {
		strcpy (userinfo, "\\name\\badinfo");
		// don't keep those clients and userinfo
		trap_DropClient(clientNum, "Invalid userinfo");
	}

	// check for local client
	s = Info_ValueForKey( userinfo, "ip" );
	if ( !strcmp( s, "localhost" ) ) {
		client->pers.localClient = qtrue;
	}

	// check the item prediction
	s = Info_ValueForKey( userinfo, "cg_predictItems" );
	if ( !atoi( s ) ) {
		client->pers.predictItemPickup = qfalse;
	} else {
		client->pers.predictItemPickup = qtrue;
	}

#ifdef SMOKINGUNS
//unlagged - client options
	// see if the player has opted out
	s = Info_ValueForKey( userinfo, "cg_delag" );
	if ( !atoi( s ) ) {
		client->pers.delag = 0;
	} else {
		client->pers.delag = atoi( s );
	}

	// see if the player is nudging his shots
	s = Info_ValueForKey( userinfo, "cg_cmdTimeNudge" );
	client->pers.cmdTimeNudge = atoi( s );

	// see if the player wants to debug the backward reconciliation
	s = Info_ValueForKey( userinfo, "cg_debugDelag" );
	if ( !atoi( s ) || !g_cheats.integer ) {
		client->pers.debugDelag = qfalse;
	}
	else {
		client->pers.debugDelag = qtrue;
	}

	// see if the player is simulating incoming latency
	s = Info_ValueForKey( userinfo, "cg_latentSnaps" );
	client->pers.latentSnaps = g_cheats.integer ? atoi( s ) : 0 ;

	// see if the player is simulating outgoing latency
	s = Info_ValueForKey( userinfo, "cg_latentCmds" );
	client->pers.latentCmds = g_cheats.integer ? atoi( s ) : 0 ;

	// see if the player is simulating outgoing packet loss
	s = Info_ValueForKey( userinfo, "cg_plOut" );
	client->pers.plOut = g_cheats.integer ? atoi( s ) : 0 ;
//unlagged - client options
#endif

	// set name
	Q_strncpyz ( oldname, client->pers.netname, sizeof( oldname ) );
	s = Info_ValueForKey (userinfo, "name");
	ClientCleanName( s, client->pers.netname, sizeof(client->pers.netname) );

#ifndef SMOKINGUNS
	if ( client->sess.sessionTeam == TEAM_SPECTATOR ) {
#else
	// Tequila: Be sure the name is really unique or drop the client
	if (!ForceUniqueName( clientNum )) {
		// We should only came here in extremely rare case and surely not with fair cowboys...
		if (client->pers.connected == CON_CONNECTING)
			// When connecting, not setting cleanname will discard the client with a suitable message about an invalid name
			client->pers.cleanname[0]='\0';
		else
			trap_DropClient( clientNum, "Dropped due to invalid player name" );
		return;
	}

	if ( client->sess.sessionTeam >= TEAM_SPECTATOR ) {
#endif
		if ( client->sess.spectatorState == SPECTATOR_SCOREBOARD ) {
			Q_strncpyz( client->pers.netname, "scoreboard", sizeof(client->pers.netname) );
		}
	}

	if ( client->pers.connected == CON_CONNECTED ) {
#ifndef SMOKINGUNS
		if ( strcmp( oldname, client->pers.netname ) ) {
			trap_SendServerCommand( -1, va("print \"%s" S_COLOR_WHITE " renamed to %s\n\"", oldname,
				client->pers.netname) );
		}
	}
#else
		// Tequila: Handle case player has set an empty name (see ClientCleanName)
		if ( !strcmp("UnnamedPlayer",client->pers.netname) && oldname[0] ) {
			// Don't allow rename to empty one
			Q_strncpyz( client->pers.netname, oldname, sizeof( client->pers.netname ) );
			Q_strncpyz( client->pers.cleanname, oldname, sizeof(client->pers.cleanname) );
			Q_CleanStr( client->pers.cleanname );
		}

		// Tequila: if there is a vote kick on the oldname, cancel renaming and
		// consider this is a mandatory vote to be kicked
		if ( level.voteTime && !strcmp(level.voteString,va("sendaway \"%s\"",oldname)) ) {
			Q_strncpyz ( client->pers.netname, oldname, sizeof( client->pers.netname ) );
			trap_SendServerCommand( -1, va("print \"%s" S_COLOR_WHITE " tried to rename\n\"", oldname) );
			trap_SendConsoleCommand( EXEC_APPEND, va("%s;cancelvote\n", level.voteString ) );
		}

		if ( strcmp( oldname, client->pers.netname ) ) {
			/* Tequila: Conditions to delay renaming
			 * 1. Player still has renamed himself
			 * 2. g_delayedRenaming if set
			 * 3. renameTime is older then the g_delayedRenaming specified time in seconds
			 * 4. g_splitChat is set, gametype is round base and player is a spectator
			 */
			if ( !client->pers.renameTime || !g_delayedRenaming.integer ||
				( level.time - client->pers.renameTime >= g_delayedRenaming.integer*1000 ) ||
				!( g_splitChat.integer && g_gametype.integer >= GT_RTP && client->sess.sessionTeam >= TEAM_SPECTATOR ) )
			{
				PushMinilogf( "RENAME: %i > %s", clientNum, client->pers.netname ) ;
				trap_SendServerCommand( -1, va("print \"%s" S_COLOR_WHITE " renamed to %s\n\"", oldname,
					client->pers.netname) );
				// Store when last renaming occured only if needed
				if ( g_delayedRenaming.integer && g_splitChat.integer && g_gametype.integer >= GT_RTP && client->sess.sessionTeam >= TEAM_SPECTATOR )
					client->pers.renameTime = level.time;

			} else {
				// Set next rename time
				if (!client->pers.renameTime)
					client->pers.renameTime = level.time;
				client->pers.renameTime += g_delayedRenaming.integer*1000 ;

				// Rename at last at the end of the current round
				if (g_roundendtime>level.time && client->pers.renameTime>g_roundendtime)
					client->pers.renameTime=g_roundendtime;

				// Keep requested name
				Q_strncpyz ( client->pers.renameName, client->pers.netname, sizeof( client->pers.renameName ) );

				// Tequila: Keep oldname and inform renaming was delayed
				G_LogPrintf( "ClientUserinfoChanged: Delayed \"%s" S_COLOR_WHITE
					"\" renaming to \"%s" S_COLOR_WHITE "\"\n",oldname,client->pers.netname);
				Q_strncpyz ( client->pers.netname, oldname, sizeof( client->pers.netname ) );
				trap_SendServerCommand( clientNum, va("print \"Renaming delayed to %i seconds, %s" S_COLOR_WHITE "...\n\"",
					(client->pers.renameTime-level.time)/1000, client->pers.netname) );
			}
		}

		// Tequila: Advertize client if we have changed significantly his player name to a suitable one
		Q_CleanStr( s ); // Here s is a requested name backup, clean it and compare it to the cleanname set
		if ( Q_stricmp( s, client->pers.cleanname ) )
			trap_SendServerCommand( clientNum, va("cp \"Your name is %s\"", client->pers.netname) );
	}

	// Tequila: Synchronize anyway the set netname with the server known name
	Info_SetValueForKey(userinfo, "name", client->pers.netname);
	trap_SetUserinfo( clientNum, userinfo );
#endif

	// set max health
#ifndef SMOKINGUNS
	if (client->ps.powerups[PW_GUARD]) {
		client->pers.maxHealth = 200;
	} else {
		health = atoi( Info_ValueForKey( userinfo, "handicap" ) );
		client->pers.maxHealth = health;
		if ( client->pers.maxHealth < 1 || client->pers.maxHealth > 100 ) {
			client->pers.maxHealth = 100;
		}
	}
#else
	health = atoi( Info_ValueForKey( userinfo, "handicap" ) );
	client->pers.maxHealth = health;
	if ( client->pers.maxHealth < 1 || client->pers.maxHealth > 100 ) {
		client->pers.maxHealth = 100;
	}
#endif
	client->ps.stats[STAT_MAX_HEALTH] = client->pers.maxHealth;

	// set model
	if( g_gametype.integer >= GT_TEAM ) {
		Q_strncpyz( model, Info_ValueForKey (userinfo, "team_model"), sizeof( model ) );
		Q_strncpyz( headModel, Info_ValueForKey (userinfo, "team_headmodel"), sizeof( headModel ) );
	} else {
		Q_strncpyz( model, Info_ValueForKey (userinfo, "model"), sizeof( model ) );
		Q_strncpyz( headModel, Info_ValueForKey (userinfo, "headmodel"), sizeof( headModel ) );
	}

	// bots set their team a few frames later
#ifndef SMOKINGUNS
	if (g_gametype.integer >= GT_TEAM && g_entities[clientNum].r.svFlags & SVF_BOT) {
		s = Info_ValueForKey( userinfo, "team" );
		if ( !Q_stricmp( s, "red" ) || !Q_stricmp( s, "r" ) ) {
			team = TEAM_RED;
		} else if ( !Q_stricmp( s, "blue" ) || !Q_stricmp( s, "b" ) ) {
			team = TEAM_BLUE;
		} else {
#else
	if (g_gametype.integer >= GT_TEAM && ent->r.svFlags & SVF_BOT) {
		s = Info_ValueForKey( userinfo, "team" );
		if ( !Q_stricmp( s, "red" ) || !Q_stricmp( s, "r" ) || !Q_stricmp( s, "redspec" )) {
			if(g_gametype.integer >= GT_RTP)
				team = TEAM_RED_SPECTATOR;
			else
				team = TEAM_RED;
		} else if ( !Q_stricmp( s, "blue" ) || !Q_stricmp( s, "b" ) || !Q_stricmp( s, "bluespec" )) {
			if(g_gametype.integer >= GT_RTP)
				team = TEAM_BLUE_SPECTATOR;
			else
				team = TEAM_BLUE;
#endif
		} else {
			// pick the team with the least number of players
			team = PickTeam( clientNum );
		}

#ifdef SMOKINGUNS
		if(client->sess.sessionTeam == TEAM_SPECTATOR) {
			client->sess.sessionTeam = team;
			switch ( team ) {
				case TEAM_RED : 
				case TEAM_RED_SPECTATOR : 
					PushMinilogf( "JOIN: %i => lawmen" , clientNum ) ;
					break ;
				case TEAM_BLUE :
				case TEAM_BLUE_SPECTATOR :
					PushMinilogf( "JOIN: %i => outlaws" , clientNum ) ;
					break ;
				case TEAM_FREE :
					PushMinilogf( "JOIN: %i => free" , clientNum ) ; 
					break ;
			}
		}
#endif
	}
	else {
		team = client->sess.sessionTeam;
	}

/*	NOTE: all client side now

	// team
	switch( team ) {
	case TEAM_RED:
		ForceClientSkin(client, model, "red");
//		ForceClientSkin(client, headModel, "red");
		break;
	case TEAM_BLUE:
		ForceClientSkin(client, model, "blue");
//		ForceClientSkin(client, headModel, "blue");
		break;
	}
	// don't ever use a default skin in teamplay, it would just waste memory
	// however bots will always join a team but they spawn in as spectator
	if ( g_gametype.integer >= GT_TEAM && team == TEAM_SPECTATOR) {
		ForceClientSkin(client, model, "red");
//		ForceClientSkin(client, headModel, "red");
	}
*/

#ifdef SMOKINGUNS
	// team skin in Smokin' Guns
	switch( team ) {
	case TEAM_RED:
	case TEAM_RED_SPECTATOR:
		ForceClientSkin(client, model, "red");
		ForceClientSkin(client, headModel, "red");
		break;
	case TEAM_BLUE:
	case TEAM_BLUE_SPECTATOR:
		ForceClientSkin(client, model, "blue");
		ForceClientSkin(client, headModel, "blue");
		break;
	}
#endif

#ifndef SMOKINGUNS
	if (g_gametype.integer >= GT_TEAM) {
		client->pers.teamInfo = qtrue;
	} else {
		s = Info_ValueForKey( userinfo, "teamoverlay" );
		if ( ! *s || atoi( s ) != 0 ) {
			client->pers.teamInfo = qtrue;
		} else {
			client->pers.teamInfo = qfalse;
		}
	}
#else
	// teamInfo
	s = Info_ValueForKey( userinfo, "teamoverlay" );
	if ( ! *s || atoi( s ) != 0 ) {
		client->pers.teamInfo = qtrue;
	} else {
		client->pers.teamInfo = qfalse;
	}
#endif
	/*
	s = Info_ValueForKey( userinfo, "cg_pmove_fixed" );
	if ( !*s || atoi( s ) == 0 ) {
		client->pers.pmoveFixed = qfalse;
	}
	else {
		client->pers.pmoveFixed = qtrue;
	}
	*/

	// team task (0 = none, 1 = offence, 2 = defence)
	teamTask = atoi(Info_ValueForKey(userinfo, "teamtask"));
	// team Leader (1 = leader, 0 is normal player)
	teamLeader = client->sess.teamLeader;

	// colors
#ifndef SMOKINGUNS
	strcpy(c1, Info_ValueForKey( userinfo, "color1" ));
	strcpy(c2, Info_ValueForKey( userinfo, "color2" ));
#else
	strcpy(version, Info_ValueForKey( userinfo, "cl_version" ));
	strcpy(md5, Info_ValueForKey( userinfo, "cl_md5" ));
#endif
	strcpy(redTeam, Info_ValueForKey( userinfo, "g_redteam" ));
	strcpy(blueTeam, Info_ValueForKey( userinfo, "g_blueteam" ));

	// send over a subset of the userinfo keys so other clients can
	// print scoreboards, display models, and play custom sounds
	if (ent->r.svFlags & SVF_BOT)
	{
#ifndef SMOKINGUNS
		s = va("n\\%s\\t\\%i\\model\\%s\\hmodel\\%s\\c1\\%s\\c2\\%s\\hc\\%i\\w\\%i\\l\\%i\\skill\\%s\\tt\\%d\\tl\\%d",
			client->pers.netname, team, model, headModel, c1, c2,
			client->pers.maxHealth, client->sess.wins, client->sess.losses,
			Info_ValueForKey( userinfo, "skill" ), teamTask, teamLeader );
#else
		s = va("n\\%s\\t\\%i\\model\\%s\\hc\\%i\\w\\%i\\l\\%i\\skill\\%s\\tt\\%d\\tl\\%d",
			client->pers.netname, client->sess.sessionTeam, model,
			client->pers.maxHealth, client->sess.wins, client->sess.losses,
			Info_ValueForKey( userinfo, "skill" ), teamTask, teamLeader);
#endif
	}
	else
	{
#ifndef SMOKINGUNS
		s = va("n\\%s\\t\\%i\\model\\%s\\hmodel\\%s\\g_redteam\\%s\\g_blueteam\\%s\\c1\\%s\\c2\\%s\\hc\\%i\\w\\%i\\l\\%i\\tt\\%d\\tl\\%d",
			client->pers.netname, client->sess.sessionTeam, model, headModel, redTeam, blueTeam, c1, c2,
			client->pers.maxHealth, client->sess.wins, client->sess.losses, teamTask, teamLeader);
#else
		s = va("\\n\\%s\\t\\%i\\model\\%s\\g_redteam\\%s\\g_blueteam\\%s\\hc\\%i\\w\\%i\\l\\%i\\tt\\%d\\tl\\%d\\v\\%s\\md5\\%s",
			client->pers.netname, client->sess.sessionTeam, model, redTeam, blueTeam,
			client->pers.maxHealth, client->sess.wins, client->sess.losses, teamTask, teamLeader, version, md5);
#endif
	}

	trap_SetConfigstring( CS_PLAYERS+clientNum, s );

	// this is not the userinfo, more like the configstring actually
	G_LogPrintf( "ClientUserinfoChanged: %i %s\n", clientNum, s );
}


/*
===========
ClientConnect

Called when a player begins connecting to the server.
Called again for every map change or tournement restart.

The session information will be valid after exit.

Return NULL if the client should be allowed, otherwise return
a string with the reason for denial.

Otherwise, the client will be sent the current gamestate
and will eventually get to ClientBegin.

firstTime will be qtrue the very first time a client connects
to the server machine, but qfalse on map changes and tournement
restarts.
============
*/
char *ClientConnect( int clientNum, qboolean firstTime, qboolean isBot ) {
	char		*value;
//	char		*areabits;
	gclient_t	*client;
	char		userinfo[MAX_INFO_STRING];
	gentity_t	*ent;

	ent = &g_entities[ clientNum ];

	trap_GetUserinfo( clientNum, userinfo, sizeof( userinfo ) );

	// IP filtering
	// https://zerowing.idsoftware.com/bugzilla/show_bug.cgi?id=500
	// recommanding PB based IP / GUID banning, the builtin system is pretty limited
	// check to see if they are on the banned IP list
	value = Info_ValueForKey (userinfo, "ip");
	if ( G_FilterPacket( value ) ) {
		return "You are banned from this server.";
	}

	// we don't check password for bots and local client
	// NOTE: local client <-> "ip" "localhost"
	//   this means this client is not running in our current process
	if ( !isBot && (strcmp(value, "localhost") != 0)) {
		// check for a password
		value = Info_ValueForKey (userinfo, "password");
		if ( g_password.string[0] && Q_stricmp( g_password.string, "none" ) &&
			strcmp( g_password.string, value) != 0) {
			return "Invalid password";
		}
	}

#ifdef SMOKINGUNS
	// Check clients is valid. If g_checkClients is set, quake3 client will be
	// discarded with a fair message immediately
	if ( !isBot && g_checkClients.integer ) {
		value = Info_ValueForKey (userinfo, "cl_version");
		if (!*value)
			return "Please download " PRODUCT_NAME " " PRODUCT_VERSION " from " PRODUCT_URL ;
	}
#endif

	// if a player reconnects quickly after a disconnect, the client disconnect may never be called, thus flag can get lost in the ether
	if (ent->inuse) {
		G_LogPrintf("Forcing disconnect on active client: %i\n", clientNum);
		// so lets just fix up anything that should happen on a disconnect
		ClientDisconnect(clientNum);
	}
	// they can connect
	ent->client = level.clients + clientNum;
	client = ent->client;

//	areabits = client->areabits;

	memset( client, 0, sizeof(*client) );

	client->pers.connected = CON_CONNECTING;

#ifdef SMOKINGUNS
	PushMinilogf( "CONNECT: %i" , clientNum ) ;
#endif
	// read or initialize the session data
	if ( firstTime || level.newSession ) {
#ifndef SMOKINGUNS
		G_InitSessionData( client, userinfo );
#else
		G_InitSessionData( client, userinfo, isBot );
#endif
	}
	G_ReadSessionData( client );

	if( isBot ) {
		ent->r.svFlags |= SVF_BOT;
		ent->inuse = qtrue;
		if( !G_BotConnect( clientNum, !firstTime ) ) {
			return "BotConnectfailed";
		}
#ifdef SMOKINGUNS
		if(g_gametype.integer == GT_DUEL){
			ent->client->realspec = qfalse;
		}
#endif
	}

	// get and distribute relevent parameters
	G_LogPrintf( "ClientConnect: %i\n", clientNum );
	ClientUserinfoChanged( clientNum );
#ifdef SMOKINGUNS
	// Tequila: cleanname must has been set if given name is valid
	if(!client->pers.cleanname[0])
		return "Invalid player name";
#endif

	// don't do the "xxx connected" messages if they were carried over from previous level
	if ( firstTime ) {
		trap_SendServerCommand( -1, va("print \"%s" S_COLOR_WHITE " connected\n\"", client->pers.netname) );
	}

	if ( g_gametype.integer >= GT_TEAM &&
		client->sess.sessionTeam != TEAM_SPECTATOR ) {
		BroadcastTeamChange( client, -1 );
	}

	// count current clients and rank for scoreboard
	CalculateRanks();

	// for statistics
//	client->areabits = areabits;
//	if ( !client->areabits )
//		client->areabits = G_Alloc( (trap_AAS_PointReachabilityAreaIndex( NULL ) + 7) / 8 );

#ifdef SMOKINGUNS
//unlagged - backward reconciliation #5
	// announce it
	if ( g_delagHitscan.integer ) {
		trap_SendServerCommand( clientNum, "print \"This server is Unlagged: full lag compensation is ON!\n\"" );
	}
	else {
		trap_SendServerCommand( clientNum, "print \"This server is Unlagged: full lag compensation is OFF!\n\"" );
	}
//unlagged - backward reconciliation #5

	// Joe Kari: exec content of the onEvent_playerConnect cvar
	if ( firstTime && !isBot )
	{
		trap_SendConsoleCommand( EXEC_APPEND, "vstr onEvent_playerConnect\n" ) ;
		trap_SendConsoleCommand( EXEC_APPEND , va( "vstr onEvent_playerUpTo%i\n" , g_humancount ) ) ;
	}
#endif

	return NULL;
}

/*
===========
ClientBegin

called when a client has finished connecting, and is ready
to be placed into the level.  This will happen every level load,
and on transition between teams, but doesn't happen on respawns
============
*/
void ClientBegin( int clientNum ) {
	gentity_t	*ent;
	gclient_t	*client;
#ifdef SMOKINGUNS
	vec3_t		viewangles;
	vec3_t		origin;
#endif
	int			flags;

	ent = g_entities + clientNum;

	client = level.clients + clientNum;

	if ( ent->r.linked ) {
		trap_UnlinkEntity( ent );
	}
	G_InitGentity( ent );
	ent->touch = 0;
	ent->pain = 0;
	ent->client = client;

	client->pers.connected = CON_CONNECTED;
	client->pers.enterTime = level.time;
	client->pers.teamState.state = TEAM_BEGIN;

	// save eflags around this, because changing teams will
	// cause this to happen with a valid entity, and we
	// want to make sure the teleport bit is set right
	// so the viewpoint doesn't interpolate through the
	// world to the new position
#ifdef SMOKINGUNS
	VectorCopy(client->ps.viewangles, viewangles);
	VectorCopy(client->ps.origin, origin);
#endif

	flags = client->ps.eFlags;
	memset( &client->ps, 0, sizeof( client->ps ) );
	client->ps.eFlags = flags;

#ifdef SMOKINGUNS
	VectorCopy(viewangles, client->ps.viewangles);
	VectorCopy(origin, client->ps.origin);
#endif

	// locate ent at a spawn point
	ClientSpawn( ent );

#ifndef SMOKINGUNS
	if ( client->sess.sessionTeam != TEAM_SPECTATOR ) {
		if ( g_gametype.integer != GT_TOURNAMENT  ) {
			trap_SendServerCommand( -1, va("print \"%s" S_COLOR_WHITE " entered the game\n\"", client->pers.netname) );
		}
#else
	if ( client->sess.sessionTeam < TEAM_SPECTATOR ) {
		if ( g_gametype.integer != GT_DUEL  ) {
			trap_SendServerCommand( -1, va("print \"%s" S_COLOR_WHITE " entered the game\n\"", client->pers.netname) );
		}
#endif
	}
	G_LogPrintf( "ClientBegin: %i\n", clientNum );

	// count current clients and rank for scoreboard
	CalculateRanks();

	//start into game: receive money (by Spoon)
#ifdef SMOKINGUNS
	ent->client->ps.stats[STAT_MONEY] = g_startingMoney.integer;

	// Send round time for round based games
	if ( g_gametype.integer >= GT_RTP && !( ent->r.svFlags & SVF_BOT ) ) {
		gentity_t	*te = G_TempEntity( vec3_origin, EV_ROUND_TIME );

		te->s.eventParm = 10;

		//sets cg.roundendtime
		te->s.time = g_roundstarttime;
		te->s.time2 = g_roundendtime;
		te->r.svFlags |= SVF_SINGLECLIENT;
		te->r.singleClient = client->ps.clientNum ;
	}
#endif
}

/*
===========
G_RestorePlayerStats
restore the playerstats on Clienspawn
===========
*/
#ifdef SMOKINGUNS
static void G_RestorePlayerStats(gentity_t *ent, qboolean save){
	int		i;
	clientPersistant_t	saved;
	clientSession_t		savedSess;
	int		persistant[MAX_PERSISTANT];
	int		savedPing, savedMoney;
	int		accuracy_hits, accuracy_shots;
	int		savedEvents[MAX_PS_EVENTS];
	int		eventSequence;
	qboolean	realspec;
	qboolean	won;
	gclient_t	*client = ent->client;
	int			wins = client->ps.stats[STAT_WINS];


	// clear everything but the persistant data
	// but only if player didn'T die when playing a round-based-mode
	saved = client->pers;
	savedSess = client->sess;
	savedPing = client->ps.ping;
	savedMoney = client->ps.stats[STAT_MONEY];
	realspec = client->realspec;
	won = client->won;

	accuracy_hits = client->accuracy_hits;
	accuracy_shots = client->accuracy_shots;
	for ( i = 0 ; i < MAX_PERSISTANT ; i++ ) {
		persistant[i] = client->ps.persistant[i];
	}
	// also save the predictable events otherwise we might get double or dropped events
	for (i = 0; i < MAX_PS_EVENTS; i++) {
		savedEvents[i] = client->ps.events[i];
	}
	eventSequence = client->ps.eventSequence;

	// save important data
	if(save){
		int ammo[MAX_WEAPONS];
		int stats[MAX_STATS];
		int powerups[MAX_POWERUPS];
		int weapon = client->ps.weapon;
		int weapon2 = client->ps.weapon2;

		// make copies
		for(i = 0; i < MAX_WEAPONS; i++){
			ammo[i] = client->ps.ammo[i];
		}
		for(i = 0; i < MAX_STATS; i++){
			stats[i] = client->ps.stats[i];
		}
		for(i = 0; i < MAX_POWERUPS; i++){
			powerups[i] = client->ps.powerups[i];
		}

		// reset client
		memset (client, 0, sizeof(*client));

		// use copies
		for(i = 0; i < MAX_WEAPONS; i++){
			client->ps.ammo[i] = ammo[i];
		}
		for(i = 0; i < MAX_STATS; i++){
			client->ps.stats[i] = stats[i];
		}
		for(i = 0; i < MAX_POWERUPS; i++){
			client->ps.powerups[i] = powerups[i];
		}
		client->ps.weapon = weapon;
		client->ps.weapon2 = weapon2;

	} else {
		memset (client, 0, sizeof(*client));
	}

	client->pers = saved;
	client->sess = savedSess;
	client->ps.ping = savedPing;
	client->ps.stats[STAT_MONEY] = savedMoney;
	client->accuracy_hits = accuracy_hits;
	client->accuracy_shots = accuracy_shots;
	client->lastkilled_client = -1;
	client->realspec = realspec;
	client->won = won;
	client->ps.stats[STAT_WINS] = wins;

	for ( i = 0 ; i < MAX_PERSISTANT ; i++ ) {
		client->ps.persistant[i] = persistant[i];
	}
	for (i = 0; i < MAX_PS_EVENTS; i++) {
		client->ps.events[i] = savedEvents[i];
	}
	client->ps.eventSequence = eventSequence;
}

/*
===========
AddDefaultWeapons

Adds the default weapon or weapons that a player starts with, with ammo.
Returns the best of the added weapons.
============
*/
static int AddDefaultWeapons( playerState_t *ps ) {
	int		startingWeapon;

	if ( g_gametype.integer == GT_DUEL ) {
		startingWeapon = WP_REM58;
	} else {
		startingWeapon = g_startingWeapon.integer;
	}

	if ( startingWeapon < 0 || startingWeapon >= WP_NUM_WEAPONS ) {
		startingWeapon = 0;
	}

	// always start with at least a knife
	ps->stats[STAT_WEAPONS] |= ( 1 << WP_KNIFE );
	ps->ammo[WP_KNIFE] = 1;

	// optionally add another weapon, with ammo
	if ( startingWeapon ) {
		int clip = bg_weaponlist[startingWeapon].clip;

		if ( clip ) {
			ps->stats[STAT_WEAPONS] |= ( 1 << startingWeapon );
			ps->ammo[startingWeapon] = bg_weaponlist[startingWeapon].clipAmmo;
			ps->ammo[clip] = bg_weaponlist[startingWeapon].maxAmmo;
		} else {
			// add ammo for weapons that don't use clip ammo
			gitem_t *item = BG_FindItemForAmmo( startingWeapon );
			ps->ammo[startingWeapon] = item->quantity;
		}
	}

	return startingWeapon ? startingWeapon : WP_KNIFE;
}
#endif

/*
===========
ClientSpawn

Called every time a client is placed fresh in the world:
after the first ClientBegin, and after each respawn
Initializes all non-persistant parts of playerState
============
*/
void ClientSpawn(gentity_t *ent) {
	int		index;
	vec3_t	spawn_origin, spawn_angles;
	gclient_t	*client;
	int		i;
#ifndef SMOKINGUNS
	clientPersistant_t	saved;
	clientSession_t		savedSess;
	int		persistant[MAX_PERSISTANT];
	gentity_t	*spawnPoint;
	gentity_t *tent;
	int		flags;
	int		savedPing;
//	char	*savedAreaBits;
	int		accuracy_hits, accuracy_shots;
	int		eventSequence;
	char	userinfo[MAX_INFO_STRING];
#else
	gentity_t	*spawnPoint;
	gentity_t *tent;
	char		userinfo[MAX_INFO_STRING];
	int		flags;
	int		min_money = 0;
	int		defaultWeapon =	WP_NONE;
	qboolean	player_died;
	qboolean	specwatch = ent->client->specwatch;
#endif

	index = ent - g_entities;
	client = ent->client;

	VectorClear(spawn_origin);

#ifdef SMOKINGUNS
	if (g_gametype.integer != GT_DUEL)
		// hika comments: client->ps.pm_type is set to PM_DEAD in player_die() at g_combat.c
		player_died = client->player_died || (client->ps.pm_type & PM_DEAD);
	else
		player_died = client->player_died;

	if(!ent->mappart && g_gametype.integer == GT_DUEL){
		ent->mappart = rand()%g_maxmapparts+1;
	}
#endif

	// find a spawn point
	// do it before setting health back up, so farthest
	// ranging doesn't count this client
#ifndef SMOKINGUNS
	if ( client->sess.sessionTeam == TEAM_SPECTATOR ) {
		spawnPoint = SelectSpectatorSpawnPoint (
						spawn_origin, spawn_angles);
	} else if (g_gametype.integer >= GT_CTF ) {
		// all base oriented team games use the CTF spawn points
		spawnPoint = SelectCTFSpawnPoint (
						client->sess.sessionTeam,
						client->pers.teamState.state,
						spawn_origin, spawn_angles,
						!!(ent->r.svFlags & SVF_BOT));
	}
#else
	if ( client->sess.sessionTeam == TEAM_SPECTATOR && (g_gametype.integer != GT_DUEL ||
		client->realspec)) {
		spawnPoint = SelectSpectatorSpawnPoint (
						spawn_origin, spawn_angles, ent->mappart);

	} else if (g_gametype.integer >= GT_RTP ) {
		if (client->sess.sessionTeam >= TEAM_SPECTATOR) {
			spawnPoint = NULL;
			VectorCopy(ent->client->ps.viewangles, spawn_angles);
			VectorCopy(ent->client->ps.origin, spawn_origin);
		} else {
			spawnPoint = SelectCTFSpawnPoint (
				client->sess.sessionTeam,
				client->pers.teamState.state,
				spawn_origin, spawn_angles,
				!!(ent->r.svFlags & SVF_BOT),
				ent->mappart);
		}
	} else if (g_gametype.integer == GT_DUEL){
		if (client->sess.sessionTeam == TEAM_SPECTATOR) {
			spawnPoint = NULL;
			VectorCopy(ent->client->ps.viewangles, spawn_angles);
			VectorCopy(ent->client->ps.origin, spawn_origin);
		} else {
			spawnPoint = SelectSpawnPoint (
						client->ps.origin,
						spawn_origin, spawn_angles,
						!!(ent->r.svFlags & SVF_BOT),
						ent->mappart, ent->client);
		}
#endif
	}
	else
	{
			// the first spawn should be at a good looking spot
		if ( !client->pers.initialSpawn && client->pers.localClient )
		{
				client->pers.initialSpawn = qtrue;
#ifndef SMOKINGUNS
				spawnPoint = SelectInitialSpawnPoint( spawn_origin, spawn_angles,
							     !!(ent->r.svFlags & SVF_BOT));
#else
				spawnPoint = SelectInitialSpawnPoint( spawn_origin, spawn_angles,
							     !!(ent->r.svFlags & SVF_BOT), ent->mappart, ent->client );
#endif
			}
			else
			{
				// don't spawn near existing origin if possible
				spawnPoint = SelectSpawnPoint (
					client->ps.origin,
#ifndef SMOKINGUNS
					spawn_origin, spawn_angles, !!(ent->r.svFlags & SVF_BOT));
#else
					spawn_origin, spawn_angles, !!(ent->r.svFlags & SVF_BOT), ent->mappart, client);
#endif
			}
	}
	client->pers.teamState.state = TEAM_ACTIVE;

	// always clear the kamikaze flag
	ent->s.eFlags &= ~EF_KAMIKAZE;

	// toggle the teleport bit so the client knows to not lerp
	// and never clear the voted flag
	flags = ent->client->ps.eFlags & (EF_TELEPORT_BIT | EF_VOTED | EF_TEAMVOTED);
	flags ^= EF_TELEPORT_BIT;

#ifndef SMOKINGUNS
	// clear everything but the persistant data

	saved = client->pers;
	savedSess = client->sess;
	savedPing = client->ps.ping;
//	savedAreaBits = client->areabits;
	accuracy_hits = client->accuracy_hits;
	accuracy_shots = client->accuracy_shots;
	for ( i = 0 ; i < MAX_PERSISTANT ; i++ ) {
		persistant[i] = client->ps.persistant[i];
	}
	eventSequence = client->ps.eventSequence;

	Com_Memset (client, 0, sizeof(*client));

	client->pers = saved;
	client->sess = savedSess;
	client->ps.ping = savedPing;
//	client->areabits = savedAreaBits;
	client->accuracy_hits = accuracy_hits;
	client->accuracy_shots = accuracy_shots;
	client->lastkilled_client = -1;

	for ( i = 0 ; i < MAX_PERSISTANT ; i++ ) {
		client->ps.persistant[i] = persistant[i];
	}
	client->ps.eventSequence = eventSequence;
#else
//unlagged - backward reconciliation #3
	// we don't want players being backward-reconciled to the place they died
	G_ResetHistory( ent );
	// and this is as good a time as any to clear the saved state
	ent->client->saved.leveltime = 0;
//unlagged - backward reconciliation #3

	if((g_gametype.integer < GT_RTP && g_gametype.integer != GT_DUEL)|| player_died){
		G_RestorePlayerStats(ent, qfalse);
	} else {
		// player stats stay(weapons, ammo, items, etc)
		VectorClear(client->ps.velocity);
		client->ps.stats[STAT_OLDWEAPON] = 0;
		client->ps.stats[STAT_WP_MODE] = 0;
		client->ps.stats[STAT_GATLING_MODE] = 0;
	}

	//goldbags will be deleted in every case
	client->ps.powerups[PW_GOLD] = 0;
#endif

	// increment the spawncount so the client will detect the respawn
	client->ps.persistant[PERS_SPAWN_COUNT]++;
	client->ps.persistant[PERS_TEAM] = client->sess.sessionTeam;

	client->airOutTime = level.time + 12000;

	trap_GetUserinfo( index, userinfo, sizeof(userinfo) );
	// set max health
	client->pers.maxHealth = atoi( Info_ValueForKey( userinfo, "handicap" ) );
	if ( client->pers.maxHealth < 1 || client->pers.maxHealth > 100 ) {
		client->pers.maxHealth = 100;
	}
	// clear entity values
	client->ps.stats[STAT_MAX_HEALTH] = client->pers.maxHealth;
	client->ps.eFlags = flags;

#ifdef SMOKINGUNS
	for(i=DM_HEAD_1;i<=DM_LEGS_2;i++){
		client->ps.powerups[i] = 0;
	}
#endif

	ent->s.groundEntityNum = ENTITYNUM_NONE;
	ent->client = &level.clients[index];
	ent->takedamage = qtrue;
	ent->inuse = qtrue;
	ent->classname = "player";
	ent->r.contents = CONTENTS_BODY;
	ent->clipmask = MASK_PLAYERSOLID;
	ent->die = player_die;
	ent->waterlevel = 0;
	ent->watertype = 0;
	ent->flags = 0;

	VectorCopy (playerMins, ent->r.mins);
	VectorCopy (playerMaxs, ent->r.maxs);

	client->ps.clientNum = index;

#ifndef SMOKINGUNS
	client->ps.stats[STAT_WEAPONS] = ( 1 << WP_MACHINEGUN );
	if ( g_gametype.integer == GT_TEAM ) {
		client->ps.ammo[WP_MACHINEGUN] = 50;
	} else {
		client->ps.ammo[WP_MACHINEGUN] = 100;
	}

	client->ps.stats[STAT_WEAPONS] |= ( 1 << WP_GAUNTLET );
	client->ps.ammo[WP_GAUNTLET] = -1;
	client->ps.ammo[WP_GRAPPLING_HOOK] = -1;

	// health will count down towards max_health
	ent->health = client->ps.stats[STAT_HEALTH] = client->ps.stats[STAT_MAX_HEALTH] + 25;
#else
	//weapons
	if((g_gametype.integer < GT_RTP && g_gametype.integer != GT_DUEL) || player_died){
		defaultWeapon = AddDefaultWeapons( &client->ps );
	} else {
		int i;

		// fill up ammo in weapon
		for ( i=WP_REM58; i<WP_DYNAMITE; i++){
			if(client->ps.stats[STAT_WEAPONS] & (1<<i)){
				client->ps.ammo[i] = bg_weaponlist[i].clipAmmo;

				// also fill up clip in duel mode
				if(g_gametype.integer == GT_DUEL)
					client->ps.ammo[bg_weaponlist[i].clip] = bg_weaponlist[i].maxAmmo;

				if(client->ps.ammo[bg_weaponlist[i].clip] < bg_weaponlist[i].maxAmmo/2 &&
					bg_weaponlist[i].clip)
					client->ps.ammo[bg_weaponlist[i].clip] = bg_weaponlist[i].maxAmmo/2;
			}
		}

		if(client->ps.stats[STAT_FLAGS] & SF_SEC_PISTOL){
			client->ps.ammo[WP_AKIMBO] = bg_weaponlist[WP_PEACEMAKER].clipAmmo;
		}
	}

	// give dynamite in BR
	if(g_gametype.integer == GT_BR && client->sess.sessionTeam == g_robteam){
		if(client->ps.ammo[WP_DYNAMITE] < 2)
			client->ps.ammo[WP_DYNAMITE] = 2;
	}

	ent->health = client->ps.stats[STAT_HEALTH] = client->ps.stats[STAT_MAX_HEALTH];
#endif

	G_SetOrigin( ent, spawn_origin );
	VectorCopy( spawn_origin, client->ps.origin );

	// the respawned flag will be cleared after the attack and jump keys come up
	client->ps.pm_flags |= PMF_RESPAWNED;

	trap_GetUsercmd( client - level.clients, &ent->client->pers.cmd );
	SetClientViewAngle( ent, spawn_angles );

#ifdef SMOKINGUNS
	if ( ent->client->sess.sessionTeam < TEAM_SPECTATOR ) {
		if ( g_gametype.integer == GT_DUEL )  {
			min_money = DU_MIN_MONEY ;
		}
		else  {
			if ( g_moneyRespawn.integer == 1 ) {
				// Joe Kari: new minimum money at respawn formula //
				if ( ( g_gametype.integer == GT_BR ) || ( g_gametype.integer == GT_RTP ) ) {
					min_money = g_startingMoney.integer + ( client->ps.stats[ STAT_MONEY ] - m_teamlose.integer ) / 4;
				} else {
					min_money = g_startingMoney.integer + client->ps.stats[ STAT_MONEY ] / 4;
				}
			}
			if ( min_money < g_startingMoney.integer ) {
				// former minimum money at respawn formula : //
				min_money = g_startingMoney.integer;
			}
		}

		// don't apply this in duel mode
		if(g_gametype.integer != GT_DUEL){
			// make sure the player doesn't interfere with KillBox
			trap_UnlinkEntity (ent);
			G_KillBox( ent );
			// Tequila comment: G_KillBox will set dontTelefrag as needed
			if (client->dontTelefrag) {
#ifdef DEBUG_TELEFRAG_CASE
				G_Printf(S_COLOR_MAGENTA "ClientSpawn: Telefrag case delayed at respawn for %s...\n",client->pers.netname);
#endif
				trap_SendServerCommand( index, va("print \"Go away %s\n\"",client->pers.netname) );
				// So we will link the player entity later with normal content
				ent->r.contents = 0;
			}
			trap_LinkEntity (ent);
		}

		// force the base weapon up
		if(g_gametype.integer == GT_DUEL){
			// in duel mode the player has to draw
			client->ps.weapon = WP_NONE;
			client->ps.weapon2 = 0;
			client->pers.cmd.weapon = WP_NONE;
		} else if(g_gametype.integer < GT_RTP || player_died){
			client->ps.weapon = defaultWeapon;
			client->ps.weapon2 = 0;
		}

		if(client->ps.stats[STAT_MONEY] < min_money)
			client->ps.stats[STAT_MONEY] = min_money;

		client->ps.weaponstate = WEAPON_READY;
		//do idle animation
		client->ps.weaponAnim = WP_ANIM_IDLE;
		client->ps.weapon2Anim = WP_ANIM_IDLE;
	}
#endif

	// don't allow full run speed for a bit
	client->ps.pm_flags |= PMF_TIME_KNOCKBACK;
	client->ps.pm_time = 100;

	client->respawnTime = level.time;
	client->inactivityTime = level.time + g_inactivity.integer * 1000;
	client->latched_buttons = 0;

	// set default animations
#ifndef SMOKINGUNS
	client->ps.torsoAnim = TORSO_STAND;
#else
	client->ps.torsoAnim = TORSO_PISTOL_STAND;
#endif
	client->ps.legsAnim = LEGS_IDLE;

	if (!level.intermissiontime) {
		if (ent->client->sess.sessionTeam != TEAM_SPECTATOR) {
			G_KillBox(ent);
			// force the base weapon up
#ifndef SMOKINGUNS
			client->ps.weapon = WP_MACHINEGUN;
#endif
			client->ps.weaponstate = WEAPON_READY;
			// fire the targets of the spawn point
#ifdef SMOKINGUNS
			if(spawnPoint)
				G_UseTargets(spawnPoint, ent);
#else
			G_UseTargets(spawnPoint, ent);
#endif
			// select the highest weapon number available, after any spawn given items have fired
#ifndef SMOKINGUNS
			client->ps.weapon = 1;

			for (i = WP_NUM_WEAPONS - 1 ; i > 0 ; i--) {
				if (client->ps.stats[STAT_WEAPONS] & (1 << i)) {
					client->ps.weapon = i;
					break;
				}
			}
#endif
			// positively link the client, even if the command times are weird
			VectorCopy(ent->client->ps.origin, ent->r.currentOrigin);

			tent = G_TempEntity(ent->client->ps.origin, EV_PLAYER_TELEPORT_IN);
			tent->s.clientNum = ent->s.clientNum;

			trap_LinkEntity (ent);
		}
	} else {
		// move players to intermission
		MoveClientToIntermission(ent);
	}
	// run a client frame to drop exactly to the floor,
	// initialize animations and other things
	client->ps.commandTime = level.time - 100;
	ent->client->pers.cmd.serverTime = level.time;
	ClientThink( ent-g_entities );

#ifdef SMOKINGUNS
	client->player_died = qtrue;
	client->specwatch = specwatch;
#endif

	// run the presend to set anything else, follow spectators wait
	// until all clients have been reconnected after map_restart
	if ( ent->client->sess.spectatorState != SPECTATOR_FOLLOW ) {
		ClientEndFrame( ent );
	}

	// clear entity state values
	BG_PlayerStateToEntityState( &client->ps, &ent->s, qtrue );

#ifdef SMOKINGUNS
	G_ClearLerpFrame( &client->torso, TORSO_PISTOL_STAND);
	G_ClearLerpFrame( &client->legs, LEGS_IDLE);

	if(g_gametype.integer == GT_DUEL && client->sess.sessionTeam == TEAM_SPECTATOR)
		client->specmappart = ent->mappart;

	// remove this in every case
	client->ps.pm_flags &= ~PMF_FOLLOW;
	client->ps.pm_flags &= ~PMF_SUICIDE;
#endif
}


/*
===========
ClientDisconnect

Called when a player drops from the server.
Will not be called between levels.

This should NOT be called directly by any game logic,
call trap_DropClient(), which will call this and do
server system housekeeping.
============
*/
void ClientDisconnect( int clientNum ) {
	gentity_t	*ent;
	gentity_t	*tent;
	int			i;

	// cleanup if we are kicking a bot that
	// hasn't spawned yet
	G_RemoveQueuedBotBegin( clientNum );

	ent = g_entities + clientNum;
	if (!ent->client || ent->client->pers.connected == CON_DISCONNECTED) {
		return;
	}

	// stop any following clients
	for ( i = 0 ; i < level.maxclients ; i++ ) {
#ifndef SMOKINGUNS
		if ( level.clients[i].sess.sessionTeam == TEAM_SPECTATOR
			&& level.clients[i].sess.spectatorState == SPECTATOR_FOLLOW
#else
		if ( level.clients[i].sess.sessionTeam >= TEAM_SPECTATOR
			&& ( level.clients[i].sess.spectatorState == SPECTATOR_FOLLOW
			|| level.clients[i].sess.spectatorState == SPECTATOR_CHASECAM
			|| level.clients[i].sess.spectatorState == SPECTATOR_FIXEDCAM )
#endif
			&& level.clients[i].sess.spectatorClient == clientNum ) {
			StopFollowing( &g_entities[i] );
		}
	}

	// send effect if they were completely connected
	if ( ent->client->pers.connected == CON_CONNECTED
#ifndef SMOKINGUNS
		&& ent->client->sess.sessionTeam != TEAM_SPECTATOR ) {
#else
		&& ent->client->sess.sessionTeam < TEAM_SPECTATOR ) {
#endif
		tent = G_TempEntity( ent->client->ps.origin, EV_PLAYER_TELEPORT_OUT );
		tent->s.clientNum = ent->s.clientNum;

		// They don't get to take powerups with them!
		// Especially important for stuff like CTF flags
		TossClientItems( ent );
#ifndef SMOKINGUNS
		TossClientPersistantPowerups( ent );
		if( g_gametype.integer == GT_HARVESTER ) {
			TossClientCubes( ent );
		}
#endif

	}

	G_LogPrintf( "ClientDisconnect: %i\n", clientNum );

	// if we are playing in tourney mode and losing, give a win to the other player
#ifndef SMOKINGUNS
	if ( (g_gametype.integer == GT_TOURNAMENT )
		&& !level.intermissiontime
		&& !level.warmupTime && level.sortedClients[1] == clientNum ) {
		level.clients[ level.sortedClients[0] ].sess.wins++;
		ClientUserinfoChanged( level.sortedClients[0] );
	}

	if( g_gametype.integer == GT_TOURNAMENT &&
		ent->client->sess.sessionTeam == TEAM_FREE &&
		level.intermissiontime ) {

		trap_SendConsoleCommand( EXEC_APPEND, "map_restart 0\n" );
		level.restarted = qtrue;
		level.changemap = NULL;
		level.intermissiontime = 0;
	}
#endif

	trap_UnlinkEntity (ent);
	ent->s.modelindex = 0;
	ent->inuse = qfalse;
	ent->classname = "disconnected";
	ent->client->pers.connected = CON_DISCONNECTED;
	ent->client->ps.persistant[PERS_TEAM] = TEAM_FREE;
	ent->client->sess.sessionTeam = TEAM_FREE;

	trap_SetConfigstring( CS_PLAYERS + clientNum, "");

	CalculateRanks();

	if ( ent->r.svFlags & SVF_BOT ) {
		BotAIShutdownClient( clientNum, qfalse );
	}

	// Joe Kari: exec content of the onEvent_playerDisconnect cvar
#ifdef SMOKINGUNS
	else
	{
		trap_SendConsoleCommand( EXEC_APPEND , "vstr onEvent_playerDisconnect\n" ) ;
		trap_SendConsoleCommand( EXEC_APPEND , va( "vstr onEvent_playerDownTo%i\n" , g_humancount ) ) ;
	}
	
	PushMinilogf( "DISCONNECT: %i" , clientNum ) ;
#endif
}


