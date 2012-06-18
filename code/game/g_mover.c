/*
===========================================================================
Copyright (C) 1999-2005 Id Software, Inc.
Copyright (C) 2000-2003 Iron Claw Interactive
Copyright (C) 2005-2012 Smokin' Guns

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

// spawnflags for doors, dont forget to change for ClearItems
#ifdef SMOKINGUNS
#define DOOR_RETURN 8
#define TRIGGER_DOOR 16

// only for rotating doors
#define DOOR_ROTATING_X_AXIS 32
#define DOOR_ROTATING_Y_AXIS 64
#define DOOR_ROTATING_ONE_WAY 128

#endif

/*
===============================================================================

PUSHMOVE

===============================================================================
*/

typedef struct {
	gentity_t	*ent;
	vec3_t	origin;
	vec3_t	angles;
	float	deltayaw;
} pushed_t;
pushed_t	pushed[MAX_GENTITIES], *pushed_p;


/*
============
G_TestEntityPosition

============
*/
gentity_t	*G_TestEntityPosition( gentity_t *ent ) {
	trace_t	tr;
	int		mask;

	if ( ent->clipmask ) {
		mask = ent->clipmask;
	} else {
		mask = MASK_SOLID;
	}
	if ( ent->client ) {
		trap_Trace( &tr, ent->client->ps.origin, ent->r.mins, ent->r.maxs, ent->client->ps.origin, ent->s.number, mask );
	} else {
		trap_Trace( &tr, ent->s.pos.trBase, ent->r.mins, ent->r.maxs, ent->s.pos.trBase, ent->s.number, mask );
	}

	if (tr.startsolid)
		return &g_entities[ tr.entityNum ];

	return NULL;
}

/*
================
G_CreateRotationMatrix
================
*/
void G_CreateRotationMatrix(vec3_t angles, vec3_t matrix[3]) {
	AngleVectors(angles, matrix[0], matrix[1], matrix[2]);
	VectorInverse(matrix[1]);
}

/*
================
G_TransposeMatrix
================
*/
void G_TransposeMatrix(vec3_t matrix[3], vec3_t transpose[3]) {
	int i, j;
	for (i = 0; i < 3; i++) {
		for (j = 0; j < 3; j++) {
			transpose[i][j] = matrix[j][i];
		}
	}
}

/*
================
G_RotatePoint
================
*/
void G_RotatePoint(vec3_t point, vec3_t matrix[3]) {
	vec3_t tvec;

	VectorCopy(point, tvec);
	point[0] = DotProduct(matrix[0], tvec);
	point[1] = DotProduct(matrix[1], tvec);
	point[2] = DotProduct(matrix[2], tvec);
}

/*
==================
G_TryPushingEntity

Returns qfalse if the move is blocked
==================
*/
qboolean	G_TryPushingEntity( gentity_t *check, gentity_t *pusher, vec3_t move, vec3_t amove ) {
	vec3_t		matrix[3], transpose[3];
	vec3_t		org, org2, move2;
	gentity_t	*block;

	// EF_MOVER_STOP will just stop when contacting another entity
	// instead of pushing it, but entities can still ride on top of it
	if ( ( pusher->s.eFlags & EF_MOVER_STOP ) &&
		check->s.groundEntityNum != pusher->s.number ) {
		return qfalse;
	}

	// save off the old position
	if (pushed_p > &pushed[MAX_GENTITIES]) {
		G_Error( "pushed_p > &pushed[MAX_GENTITIES]" );
	}
	pushed_p->ent = check;
	VectorCopy (check->s.pos.trBase, pushed_p->origin);
	VectorCopy (check->s.apos.trBase, pushed_p->angles);
	if ( check->client ) {
		pushed_p->deltayaw = check->client->ps.delta_angles[YAW];
		VectorCopy (check->client->ps.origin, pushed_p->origin);
	}
	pushed_p++;

	// try moving the contacted entity
	// figure movement due to the pusher's amove
	G_CreateRotationMatrix( amove, transpose );
	G_TransposeMatrix( transpose, matrix );
	if ( check->client ) {
		VectorSubtract (check->client->ps.origin, pusher->r.currentOrigin, org);
	}
	else {
		VectorSubtract (check->s.pos.trBase, pusher->r.currentOrigin, org);
	}
	VectorCopy( org, org2 );
	G_RotatePoint( org2, matrix );
	VectorSubtract (org2, org, move2);
	// add movement
	VectorAdd (check->s.pos.trBase, move, check->s.pos.trBase);
	VectorAdd (check->s.pos.trBase, move2, check->s.pos.trBase);
	if ( check->client ) {
		VectorAdd (check->client->ps.origin, move, check->client->ps.origin);
		VectorAdd (check->client->ps.origin, move2, check->client->ps.origin);
		// make sure the client's view rotates when on a rotating mover
		check->client->ps.delta_angles[YAW] += ANGLE2SHORT(amove[YAW]);
	}

	// may have pushed them off an edge
	if ( check->s.groundEntityNum != pusher->s.number ) {
		check->s.groundEntityNum = ENTITYNUM_NONE;
	}

	block = G_TestEntityPosition( check );
	if (!block) {
		// pushed ok
		if ( check->client ) {
			VectorCopy( check->client->ps.origin, check->r.currentOrigin );
		} else {
			VectorCopy( check->s.pos.trBase, check->r.currentOrigin );
		}
		trap_LinkEntity (check);
		return qtrue;
	}

	// if it is ok to leave in the old position, do it
	// this is only relevent for riding entities, not pushed
	// Sliding trapdoors can cause this.
	VectorCopy( (pushed_p-1)->origin, check->s.pos.trBase);
	if ( check->client ) {
		VectorCopy( (pushed_p-1)->origin, check->client->ps.origin);
	}
	VectorCopy( (pushed_p-1)->angles, check->s.apos.trBase );
	block = G_TestEntityPosition (check);
	if ( !block ) {
		check->s.groundEntityNum = ENTITYNUM_NONE;
		pushed_p--;
		return qtrue;
	}

	// blocked
	return qfalse;
}

/*
==================
G_CheckProxMinePosition
==================
*/
#ifndef SMOKINGUNS
qboolean G_CheckProxMinePosition( gentity_t *check ) {
	vec3_t		start, end;
	trace_t	tr;

	VectorMA(check->s.pos.trBase, 0.125, check->movedir, start);
	VectorMA(check->s.pos.trBase, 2, check->movedir, end);
	trap_Trace( &tr, start, NULL, NULL, end, check->s.number, MASK_SOLID );

	if (tr.startsolid || tr.fraction < 1)
		return qfalse;

	return qtrue;
}

/*
==================
G_TryPushingProxMine
==================
*/
qboolean G_TryPushingProxMine( gentity_t *check, gentity_t *pusher, vec3_t move, vec3_t amove ) {
	vec3_t		forward, right, up;
	vec3_t		org, org2, move2;
	int ret;

	// we need this for pushing things later
	VectorSubtract (vec3_origin, amove, org);
	AngleVectors (org, forward, right, up);

	// try moving the contacted entity
	VectorAdd (check->s.pos.trBase, move, check->s.pos.trBase);

	// figure movement due to the pusher's amove
	VectorSubtract (check->s.pos.trBase, pusher->r.currentOrigin, org);
	org2[0] = DotProduct (org, forward);
	org2[1] = -DotProduct (org, right);
	org2[2] = DotProduct (org, up);
	VectorSubtract (org2, org, move2);
	VectorAdd (check->s.pos.trBase, move2, check->s.pos.trBase);

	ret = G_CheckProxMinePosition( check );
	if (ret) {
		VectorCopy( check->s.pos.trBase, check->r.currentOrigin );
		trap_LinkEntity (check);
	}
	return ret;
}

void G_ExplodeMissile( gentity_t *ent );
#endif

/*
============
G_MoverPush

Objects need to be moved back on a failed push,
otherwise riders would continue to slide.
If qfalse is returned, *obstacle will be the blocking entity
============
*/
qboolean G_MoverPush( gentity_t *pusher, vec3_t move, vec3_t amove, gentity_t **obstacle ) {
	int			i, e;
	gentity_t	*check;
	vec3_t		mins, maxs;
	pushed_t	*p;
	int			entityList[MAX_GENTITIES];
	int			listedEntities;
	vec3_t		totalMins, totalMaxs;

	*obstacle = NULL;

	// mins/maxs are the bounds at the destination
	// totalMins / totalMaxs are the bounds for the entire move
	if ( pusher->r.currentAngles[0] || pusher->r.currentAngles[1] || pusher->r.currentAngles[2]
		|| amove[0] || amove[1] || amove[2] ) {
		float		radius;

		radius = RadiusFromBounds( pusher->r.mins, pusher->r.maxs );
		for ( i = 0 ; i < 3 ; i++ ) {
			mins[i] = pusher->r.currentOrigin[i] + move[i] - radius;
			maxs[i] = pusher->r.currentOrigin[i] + move[i] + radius;
			totalMins[i] = mins[i] - move[i];
			totalMaxs[i] = maxs[i] - move[i];
		}
	} else {
		for (i=0 ; i<3 ; i++) {
			mins[i] = pusher->r.absmin[i] + move[i];
			maxs[i] = pusher->r.absmax[i] + move[i];
		}

		VectorCopy( pusher->r.absmin, totalMins );
		VectorCopy( pusher->r.absmax, totalMaxs );
		for (i=0 ; i<3 ; i++) {
			if ( move[i] > 0 ) {
				totalMaxs[i] += move[i];
			} else {
				totalMins[i] += move[i];
			}
		}
	}

	// unlink the pusher so we don't get it in the entityList
	trap_UnlinkEntity( pusher );

	listedEntities = trap_EntitiesInBox( totalMins, totalMaxs, entityList, MAX_GENTITIES );

	// move the pusher to its final position
	VectorAdd( pusher->r.currentOrigin, move, pusher->r.currentOrigin );
	VectorAdd( pusher->r.currentAngles, amove, pusher->r.currentAngles );

	// Tequila comment: Fix "CM_AdjustAreaPortalState: negative reference count" case in SteamBoat
	// Binary mod don't round float in the same way than QVM mod
#ifdef SMOKINGUNS
	SnapVector( pusher->r.currentAngles );
#endif

	trap_LinkEntity( pusher );

	// see if any solid entities are inside the final position
	for ( e = 0 ; e < listedEntities ; e++ ) {
		check = &g_entities[ entityList[ e ] ];

#ifndef SMOKINGUNS
		if ( check->s.eType == ET_MISSILE ) {
			// if it is a prox mine
			if ( !strcmp(check->classname, "prox mine") ) {
				// if this prox mine is attached to this mover try to move it with the pusher
				if ( check->enemy == pusher ) {
					if (!G_TryPushingProxMine( check, pusher, move, amove )) {
						//explode
						check->s.loopSound = 0;
						G_AddEvent( check, EV_PROXIMITY_MINE_TRIGGER, 0 );
						G_ExplodeMissile(check);
						if (check->activator) {
							G_FreeEntity(check->activator);
							check->activator = NULL;
						}
						//G_Printf("prox mine explodes\n");
					}
				}
				else {
					//check if the prox mine is crushed by the mover
					if (!G_CheckProxMinePosition( check )) {
						//explode
						check->s.loopSound = 0;
						G_AddEvent( check, EV_PROXIMITY_MINE_TRIGGER, 0 );
						G_ExplodeMissile(check);
						if (check->activator) {
							G_FreeEntity(check->activator);
							check->activator = NULL;
						}
						//G_Printf("prox mine explodes\n");
					}
				}
				continue;
			}
		}
#endif
		// only push items and players
#ifndef SMOKINGUNS
		if ( check->s.eType != ET_ITEM && check->s.eType != ET_PLAYER && !check->physicsObject ) {
#else
		if ( (check->s.eType != ET_ITEM && check->s.eType != ET_PLAYER && check->s.eType != ET_TURRET
			&& !check->physicsObject ) ) {
#endif
			continue;
		}

		// if the entity is standing on the pusher, it will definitely be moved
		if ( check->s.groundEntityNum != pusher->s.number ) {
			// see if the ent needs to be tested
			if ( check->r.absmin[0] >= maxs[0]
			|| check->r.absmin[1] >= maxs[1]
			|| check->r.absmin[2] >= maxs[2]
			|| check->r.absmax[0] <= mins[0]
			|| check->r.absmax[1] <= mins[1]
			|| check->r.absmax[2] <= mins[2] ) {
				continue;
			}
			// see if the ent's bbox is inside the pusher's final position
			// this does allow a fast moving object to pass through a thin entity...
			if (!G_TestEntityPosition (check)) {
				continue;
			}
		}

		// the entity needs to be pushed
		if ( G_TryPushingEntity( check, pusher, move, amove ) ) {
			continue;
		}

		// the move was blocked an entity

		// bobbing entities are instant-kill and never get blocked
		if ( pusher->s.pos.trType == TR_SINE || pusher->s.apos.trType == TR_SINE ) {
#ifndef SMOKINGUNS
			G_Damage( check, pusher, pusher, NULL, NULL, 99999, 0, MOD_CRUSH );
			continue;
#else
			if(pusher->damage > 0){
				G_Damage( check, pusher, pusher, NULL, NULL, 99999, 0, MOD_CRUSH );
				continue;
			}
#endif
		}


		// save off the obstacle so we can call the block function (crush, etc)
		*obstacle = check;

		// move back any entities we already moved
		// go backwards, so if the same entity was pushed
		// twice, it goes back to the original position
		for ( p=pushed_p-1 ; p>=pushed ; p-- ) {
			VectorCopy (p->origin, p->ent->s.pos.trBase);
			VectorCopy (p->angles, p->ent->s.apos.trBase);
			if ( p->ent->client ) {
				p->ent->client->ps.delta_angles[YAW] = p->deltayaw;
				VectorCopy (p->origin, p->ent->client->ps.origin);
			}
			trap_LinkEntity (p->ent);
		}
		return qfalse;
	}

	return qtrue;
}


/*
=================
G_MoverTeam
=================
*/
void G_MoverTeam( gentity_t *ent ) {
	vec3_t		move, amove;
	gentity_t	*part, *obstacle;
	vec3_t		origin, angles;

	obstacle = NULL;

	// make sure all team slaves can move before commiting
	// any moves or calling any think functions
	// if the move is blocked, all moved objects will be backed out
	pushed_p = pushed;
	for (part = ent ; part ; part=part->teamchain) {
		// get current position
		BG_EvaluateTrajectory( &part->s.pos, level.time, origin );
		BG_EvaluateTrajectory( &part->s.apos, level.time, angles );
		VectorSubtract( origin, part->r.currentOrigin, move );
		VectorSubtract( angles, part->r.currentAngles, amove );
		if ( !G_MoverPush( part, move, amove, &obstacle ) ) {
			break;	// move was blocked
		}
	}

	if (part) {
		// go back to the previous position
		for ( part = ent ; part ; part = part->teamchain ) {
			part->s.pos.trTime += level.time - level.previousTime;
			part->s.apos.trTime += level.time - level.previousTime;
			BG_EvaluateTrajectory( &part->s.pos, level.time, part->r.currentOrigin );
			BG_EvaluateTrajectory( &part->s.apos, level.time, part->r.currentAngles );
			trap_LinkEntity( part );
		}

		// if the pusher has a "blocked" function, call it
		if (ent->blocked) {
			ent->blocked( ent, obstacle );
		}
		return;
	}

	// the move succeeded
	for ( part = ent ; part ; part = part->teamchain ) {
		// call the reached function if time is at or past end point
#ifdef SMOKINGUNS
		if ( ent->s.eFlags & EF_ROTATING_DOOR ) {
			if ( part->s.apos.trType == TR_LINEAR_STOP ) {
				if ( level.time >= part->s.apos.trTime + part->s.apos.trDuration ) {
					if ( part->reached ) {
						part->reached( part );
					}
				}
			}
			continue ;
		}
#endif
		if ( part->s.pos.trType == TR_LINEAR_STOP ) {
			if ( level.time >= part->s.pos.trTime + part->s.pos.trDuration ) {
				if ( part->reached ) {
					part->reached( part );
				}
			}
		}
	}
}

/*
================
G_RunMover

================
*/
void G_RunMover( gentity_t *ent ) {
	// if not a team captain, don't do anything, because
	// the captain will handle everything
	if ( ent->flags & FL_TEAMSLAVE ) {
		return;
	}

	// if stationary at one of the positions, don't move anything
	if ( ent->s.pos.trType != TR_STATIONARY || ent->s.apos.trType != TR_STATIONARY ) {
		G_MoverTeam( ent );
	}

	// check think function
	G_RunThink( ent );
}

/*
============================================================================

GENERAL MOVERS

Doors, plats, and buttons are all binary (two position) movers
Pos1 is "at rest", pos2 is "activated"
============================================================================
*/

/*
===============
SetMoverState
===============
*/
void SetMoverState( gentity_t *ent, moverState_t moverState, int time ) {
	vec3_t			delta;
	float			f;

#ifdef SMOKINGUNS
	if ( ent->moverState == MOVER_STATIC )  return ;
#endif

	ent->moverState = moverState;

	ent->s.pos.trTime = time;
	ent->s.apos.trTime = time;
	
	switch( moverState ) {
		
		case MOVER_POS1:
			VectorCopy( ent->pos1, ent->s.pos.trBase );
			ent->s.pos.trType = TR_STATIONARY;
#ifdef SMOKINGUNS
			VectorCopy( ent->apos1, ent->s.apos.trBase );
			ent->s.apos.trType = TR_STATIONARY;
#endif
			break;
		case MOVER_POS2:
			VectorCopy( ent->pos2, ent->s.pos.trBase );
			ent->s.pos.trType = TR_STATIONARY;
#ifdef SMOKINGUNS
			VectorCopy( ent->apos2, ent->s.apos.trBase );
			ent->s.apos.trType = TR_STATIONARY;
#endif
			break;
		case MOVER_1TO2:
			VectorCopy( ent->pos1, ent->s.pos.trBase );
			VectorSubtract( ent->pos2, ent->pos1, delta );
			f = 1000.0 / ent->s.pos.trDuration;
			VectorScale( delta, f, ent->s.pos.trDelta );
			ent->s.pos.trType = TR_LINEAR_STOP;
#ifdef SMOKINGUNS
			VectorCopy( ent->apos1, ent->s.apos.trBase );
			AnglesSubtract( ent->apos2, ent->apos1, delta );
			f = 1000.0 / ent->s.apos.trDuration;
			VectorScale( delta, f, ent->s.apos.trDelta );
			ent->s.apos.trType = TR_LINEAR_STOP;
#endif
			break;
		case MOVER_2TO1:
			VectorCopy( ent->pos2, ent->s.pos.trBase );
			VectorSubtract( ent->pos1, ent->pos2, delta );
			f = 1000.0 / ent->s.pos.trDuration;
			VectorScale( delta, f, ent->s.pos.trDelta );
			ent->s.pos.trType = TR_LINEAR_STOP;
#ifdef SMOKINGUNS
			VectorCopy( ent->apos2, ent->s.apos.trBase );
			AnglesSubtract( ent->apos1, ent->apos2, delta );
			f = 1000.0 / ent->s.apos.trDuration;
			VectorScale( delta, f, ent->s.apos.trDelta );
			ent->s.apos.trType = TR_LINEAR_STOP;
#endif
			break;
#ifdef SMOKINGUNS
		default: // Tequila comment: Avoid a compilation warning about MOVER_STATIC
			break;
	}
#endif

	BG_EvaluateTrajectory( &ent->s.pos, level.time, ent->r.currentOrigin );
	trap_LinkEntity( ent );
}

/*
================
MatchTeam

All entities in a mover team will move from pos1 to pos2
in the same amount of time
================
*/
void MatchTeam( gentity_t *teamLeader, int moverState, int time ) {
	gentity_t		*slave;

	for ( slave = teamLeader ; slave ; slave = slave->teamchain ) {
		SetMoverState( slave, moverState, time );
	}
}



/*
================
ReturnToPos1
================
*/
void ReturnToPos1( gentity_t *ent ) {
	MatchTeam( ent, MOVER_2TO1, level.time );

	// looping sound
	ent->s.loopSound = ent->soundLoop;

	// starting sound
	if ( ent->sound2to1 ) {
		G_AddEvent( ent, EV_GENERAL_SOUND, ent->sound2to1 );
	}
}


/*
================
Reached_BinaryMover
================
*/
void Reached_BinaryMover( gentity_t *ent ) {

	// stop the looping sound
	ent->s.loopSound = ent->soundLoop;

	if ( ent->moverState == MOVER_1TO2 ) {
		// reached pos2
		SetMoverState( ent, MOVER_POS2, level.time );

		// play sound
		if ( ent->soundPos2 ) {
			G_AddEvent( ent, EV_GENERAL_SOUND, ent->soundPos2 );
		}

		// return to pos1 after a delay
#ifndef SMOKINGUNS
		ent->think = ReturnToPos1;
		ent->nextthink = level.time + ent->wait;
#else
		if(ent->s.angles2[0] != -1000 ||
			(ent->spawnflags & DOOR_RETURN)){
			ent->think = ReturnToPos1;
			ent->nextthink = level.time + ent->wait;
		}
#endif

		// fire targets
		if ( !ent->activator ) {
			ent->activator = ent;
		}
		G_UseTargets( ent, ent->activator );
	} else if ( ent->moverState == MOVER_2TO1 ) {
		// reached pos1
		SetMoverState( ent, MOVER_POS1, level.time );

		// play sound
		if ( ent->soundPos1 ) {
			G_AddEvent( ent, EV_GENERAL_SOUND, ent->soundPos1 );
		}

		// close areaportals
		if ( ent->teammaster == ent || !ent->teammaster ) {
			trap_AdjustAreaPortalState( ent, qfalse );
		}
	} else {
		G_Error( "Reached_BinaryMover: bad moverState" );
	}
}


/*
================
Use_BinaryMover
================
*/
void Use_BinaryMover( gentity_t *ent, gentity_t *other, gentity_t *activator ) {
	int		total;
	int		partial;

	// only the master should be used
	if ( ent->flags & FL_TEAMSLAVE ) {
		Use_BinaryMover( ent->teammaster, other, activator );
		return;
	}

	ent->activator = activator;

	if ( ent->moverState == MOVER_POS1 ) {
		//calculate the side to which the door should open
#ifdef SMOKINGUNS
		if( ent->s.eFlags & EF_ROTATING_DOOR ) {
			int		axis;

			if (ent->spawnflags & DOOR_ROTATING_X_AXIS) {
				axis = 2;
			} else if (ent->spawnflags & DOOR_ROTATING_Y_AXIS) {
				axis = 0;
			} else {
				axis = 1;
			}

			VectorCopy(ent->apos1, ent->apos2);

			if (ent->spawnflags & DOOR_ROTATING_ONE_WAY) {
				ent->apos2[axis] += ent->count;
			} else {
				int len, apos;
				vec3_t	space;

				VectorSubtract(ent->r.maxs, ent->r.mins, space);

				//from which side??
				if(space[1] < space[0]){
					len = 1;
					apos = 0;
				} else {
					len = 0;
					apos = 1;
				}
				if (len){
					if(activator->client->ps.origin[len] > ent->r.currentOrigin[len]){
						if(ent->r.maxs[apos] <
							-ent->r.mins[apos]){
							ent->apos2[axis] += ent->count;
						} else {
							ent->apos2[axis] -= ent->count;
						}
					} else  {
						if(ent->r.maxs[apos] <
							-ent->r.mins[apos]) {
							ent->apos2[axis] -= ent->count;
						} else {
							ent->apos2[axis] += ent->count;
						}
					}
				} else {
					if(activator->client->ps.origin[len] > ent->r.currentOrigin[len]){
						if(ent->r.maxs[apos] <
							-ent->r.mins[apos]){
							ent->apos2[axis] -= ent->count;
						} else {
							ent->apos2[axis] += ent->count;
						}
					} else  {
						if(ent->r.maxs[apos] <
							-ent->r.mins[apos]) {
							ent->apos2[axis] += ent->count;
						} else {
							ent->apos2[axis] -= ent->count;
						}
					}
				}
			}
		}
#endif
		// start moving 50 msec later, because if this was player
		// triggered, level.time hasn't been advanced yet
		MatchTeam( ent, MOVER_1TO2, level.time + 50 );

		// starting sound
		if ( ent->sound1to2 ) {
			G_AddEvent( ent, EV_GENERAL_SOUND, ent->sound1to2 );
		}

		// looping sound
		ent->s.loopSound = ent->soundLoop;

		// open areaportal
		if ( ent->teammaster == ent || !ent->teammaster ) {
			trap_AdjustAreaPortalState( ent, qtrue );
		}
		return;
	}

	// if all the way up, just delay before coming down
	if ( ent->moverState == MOVER_POS2 ) {
#ifdef SMOKINGUNS
		if(ent->s.angles2[0] == -1000 && !(ent->spawnflags & DOOR_RETURN)){
			ReturnToPos1(ent);
			return;
		}
#endif
		ent->nextthink = level.time + ent->wait;
		return;
	}

	// only partway down before reversing
	if ( ent->moverState == MOVER_2TO1 ) {
#ifndef SMOKINGUNS
		total = ent->s.pos.trDuration;
		partial = level.time - ent->s.pos.trTime;
#else
		// Tequila: Handle the partial rotation of rotating doors
		if( ent->s.eFlags & EF_ROTATING_DOOR ) {
			total = ent->s.apos.trDuration;
			partial = level.time - ent->s.apos.trTime;
		} else {
			total = ent->s.pos.trDuration;
			partial = level.time - ent->s.pos.trTime;
		}
#endif
		if ( partial > total ) {
			partial = total;
		}

		MatchTeam( ent, MOVER_1TO2, level.time - ( total - partial ) );

		if ( ent->sound1to2 ) {
			G_AddEvent( ent, EV_GENERAL_SOUND, ent->sound1to2 );
		}
		return;
	}

	// only partway up before reversing
	if ( ent->moverState == MOVER_1TO2 ) {
#ifndef SMOKINGUNS
		total = ent->s.pos.trDuration;
		partial = level.time - ent->s.pos.trTime;
#else
		// Tequila: Handle the partial rotation of rotating doors
		if( ent->s.eFlags & EF_ROTATING_DOOR ) {
			total = ent->s.apos.trDuration;
			partial = level.time - ent->s.apos.trTime;
		} else {
			total = ent->s.pos.trDuration;
			partial = level.time - ent->s.pos.trTime;
		}
#endif
		if ( partial > total ) {
			partial = total;
		}

		MatchTeam( ent, MOVER_2TO1, level.time - ( total - partial ) );

		if ( ent->sound2to1 ) {
			G_AddEvent( ent, EV_GENERAL_SOUND, ent->sound2to1 );
		}
		return;
	}

	// now look for other doors which have the same targetname
#ifdef SMOKINGUNS
	if(ent->s.angles2[0] == -1000 && ent->s.angles2[1] == 0 &&
		ent->targetname && ent->targetname[0]){
		int i;

		for(i=0;i<MAX_GENTITIES;i++){
			gentity_t *t = &g_entities[i];

			if(ent->s.number != i && t->s.eType == ET_MOVER &&
				t->s.angles2[0] == -1000 &&
				// FIXME: String comparison are BAD: too SLOW
				!Q_stricmp(ent->targetname, t->targetname)){

				// mark the door as being mastered
				t->s.angles2[1] = -1000;
				t->use(t, other, activator);
				t->s.angles2[1] = 0;
			}
		}
	}
#endif
}



/*
================
InitMover

"pos1", "pos2", and "speed" should be set before calling,
so the movement delta can be calculated
================
*/
void InitMover( gentity_t *ent ) {
	vec3_t		move;
	vec3_t		amove;
	float		distance;
	float		adistance;
	float		light;
	vec3_t		color;
	qboolean	lightSet, colorSet;
	char		*sound;

	// if the "model2" key is set, use a seperate model
	// for drawing, but clip against the brushes
	if ( ent->model2 ) {
		ent->s.modelindex2 = G_ModelIndex( ent->model2 );
	}

	// if the "loopsound" key is set, use a constant looping sound when moving
	if ( G_SpawnString( "noise", "100", &sound ) ) {
		ent->s.loopSound = G_SoundIndex( sound );
	}
	
#ifdef SMOKINGUNS
	// Joe Kari: this prevent from collision with func_static far clipping
	ent->s.powerups = FARCLIP_NONE ;
#endif

	// if the "color" or "light" keys are set, setup constantLight
	lightSet = G_SpawnFloat( "light", "100", &light );
	colorSet = G_SpawnVector( "color", "1 1 1", color );
	if ( lightSet || colorSet ) {
		int		r, g, b, i;

		r = color[0] * 255;
		if ( r > 255 ) {
			r = 255;
		}
		g = color[1] * 255;
		if ( g > 255 ) {
			g = 255;
		}
		b = color[2] * 255;
		if ( b > 255 ) {
			b = 255;
		}
		i = light / 4;
		if ( i > 255 ) {
			i = 255;
		}
		ent->s.constantLight = r | ( g << 8 ) | ( b << 16 ) | ( i << 24 );
	}


	ent->use = Use_BinaryMover;
	ent->reached = Reached_BinaryMover;

	ent->moverState = MOVER_POS1;
	ent->r.svFlags = SVF_USE_CURRENT_ORIGIN;
	ent->s.eType = ET_MOVER;
	VectorCopy (ent->pos1, ent->r.currentOrigin);
	VectorCopy (ent->apos1, ent->r.currentAngles);
	trap_LinkEntity (ent);

	ent->s.pos.trType = TR_STATIONARY;
	ent->s.apos.trType = TR_STATIONARY;
	VectorCopy( ent->pos1, ent->s.pos.trBase );
	VectorCopy( ent->apos1, ent->s.apos.trBase );

	// calculate time to reach second position from speed
#ifdef SMOKINGUNS
	ent->r.contents = CONTENTS_BODY;

	if ( ent->s.eFlags & EF_ROTATING_DOOR ){
		ent->s.apos.trType = TR_STATIONARY;
		VectorCopy( ent->apos1, ent->s.apos.trBase );

		AnglesSubtract( ent->apos2, ent->apos1, amove );
		distance = VectorLength( amove );
		if ( ! ent->speed ) {
			ent->speed = 100;
		}
		VectorScale( amove, ent->speed, ent->s.apos.trDelta );
		ent->s.apos.trDuration = distance * 1000 / ent->speed;
		if ( ent->s.apos.trDuration <= 0 ) {
			ent->s.apos.trDuration = 1;
		}
		
		// For God only know the reason, if ent->s.pos.trDuration is not set, it causes trouble...
		ent->s.pos.trDuration = ent->s.apos.trDuration ;
		
		return ;
	}
#endif
	VectorSubtract( ent->pos2, ent->pos1, move );
	distance = VectorLength( move );
	if ( ! ent->speed ) {
		ent->speed = 100;
	}
	VectorScale( move, ent->speed, ent->s.pos.trDelta );
	ent->s.pos.trDuration = distance * 1000 / ent->speed;
	if ( ent->s.pos.trDuration <= 0 ) {
		ent->s.pos.trDuration = 1;
	}
	
#ifdef SMOKINGUNS
	AnglesSubtract( ent->apos2, ent->apos1, amove ) ;
	adistance = VectorLength( amove );
	VectorScale( amove, ent->speed, ent->s.apos.trDelta );
	
	//ent->s.apos.trDuration = adistance * 1000 / ent->speed;
	ent->s.apos.trDuration = ent->s.pos.trDuration;
	
	if ( ent->s.apos.trDuration <= 0 ) {
		ent->s.apos.trDuration = 1;
	}
#endif
}


/*
===============================================================================

DOOR

A use can be triggered either by a touch function, by being shot, or by being
targeted by another entity.

===============================================================================
*/

/*
================
Blocked_Door
================
*/
void Blocked_Door( gentity_t *ent, gentity_t *other ) {
	// remove anything other than a client
	if ( !other->client ) {
		// except CTF flags!!!!
#ifndef SMOKINGUNS
		if( other->s.eType == ET_ITEM && other->item->giType == IT_TEAM ) {
			Team_DroppedFlagThink( other );
			return;
		}
#endif
		G_TempEntity( other->s.origin, EV_ITEM_POP );
		G_FreeEntity( other );
		return;
	}

	if ( ent->damage ) {
		G_Damage( other, ent, ent, NULL, NULL, ent->damage, 0, MOD_CRUSH );
	}
	if ( ent->spawnflags & 4 ) {
		return;		// crushers don't reverse
	}

	// reverse direction
	Use_BinaryMover( ent, ent, other );
}

/*
================
Touch_DoorTriggerSpectator
================
*/
#ifndef SMOKINGUNS
static void Touch_DoorTriggerSpectator( gentity_t *ent, gentity_t *other, trace_t *trace ) {
	int axis;
	float doorMin, doorMax;
	vec3_t origin;

	axis = ent->count;
	// the constants below relate to constants in Think_SpawnNewDoorTrigger()
	doorMin = ent->r.absmin[axis] + 100;
	doorMax = ent->r.absmax[axis] - 100;

	VectorCopy(other->client->ps.origin, origin);

	if (origin[axis] < doorMin || origin[axis] > doorMax) return;

	if (fabs(origin[axis] - doorMax) < fabs(origin[axis] - doorMin)) {
		origin[axis] = doorMin - 10;
	} else {
		origin[axis] = doorMax + 10;
	}

	TeleportPlayer(other, origin, tv(10000000.0, 0, 0));
}
#endif

/*
================
Touch_DoorTrigger
================
*/
void Touch_DoorTrigger( gentity_t *ent, gentity_t *other, trace_t *trace ) {
#ifndef SMOKINGUNS
	if ( other->client && other->client->sess.sessionTeam == TEAM_SPECTATOR ) {
		// if the door is not open and not opening
		if ( ent->parent->moverState != MOVER_1TO2 &&
			ent->parent->moverState != MOVER_POS2) {
			Touch_DoorTriggerSpectator( ent, other, trace );
		}
	}
	else if ( ent->parent->moverState != MOVER_1TO2 ) {
#else
	if ( ent->parent->moverState != MOVER_1TO2 ) {
#endif
		Use_BinaryMover( ent->parent, ent, other );
	}
}


/*
======================
Think_SpawnNewDoorTrigger

All of the parts of a door have been spawned, so create
a trigger that encloses all of them
======================
*/
void Think_SpawnNewDoorTrigger( gentity_t *ent ) {
	gentity_t		*other;
	vec3_t		mins, maxs;
	int			i, best;

	// set all of the slaves as shootable
	for ( other = ent ; other ; other = other->teamchain ) {
		other->takedamage = qtrue;
	}

	// find the bounds of everything on the team
	VectorCopy (ent->r.absmin, mins);
	VectorCopy (ent->r.absmax, maxs);

	for (other = ent->teamchain ; other ; other=other->teamchain) {
		AddPointToBounds (other->r.absmin, mins, maxs);
		AddPointToBounds (other->r.absmax, mins, maxs);
	}

	// find the thinnest axis, which will be the one we expand
	best = 0;
	for ( i = 1 ; i < 3 ; i++ ) {
		if ( maxs[i] - mins[i] < maxs[best] - mins[best] ) {
			best = i;
		}
	}
	maxs[best] += 120;
	mins[best] -= 120;

	// create a trigger with this size
	other = G_Spawn ();
	other->classname = "door_trigger";
	VectorCopy (mins, other->r.mins);
	VectorCopy (maxs, other->r.maxs);
	other->parent = ent;
	other->r.contents = CONTENTS_TRIGGER;
	other->touch = Touch_DoorTrigger;
	// remember the thinnest axis
	other->count = best;
	trap_LinkEntity (other);

	MatchTeam( ent, ent->moverState, level.time );
}

void Think_MatchTeam( gentity_t *ent ) {
	MatchTeam( ent, ent->moverState, level.time );
}


/*QUAKED func_door (0 .5 .8) ? START_OPEN x CRUSHER
TOGGLE		wait in both the start and end states for a trigger event.
START_OPEN	the door to moves to its destination when spawned, and operate in reverse.  It is used to temporarily or permanently close off an area when triggered (not useful for touch or takedamage doors).
NOMONSTER	monsters will not trigger this door
SILENT		supresses the default door sounds.

"model2"	.md3 model to also draw
"angle"		determines the opening direction
"targetname" if set, no touch field will be spawned and a remote button or trigger field activates the door.
"speed"		movement speed (100 default)
"wait"		wait before returning (3 default, -1 = never return)
"lip"		lip remaining at end of move (8 default)
"dmg"		damage to inflict when blocked (2 default)
"color"		constantLight color
"light"		constantLight radius
"health"	if set, the door must be shot open
*/
void SP_func_door (gentity_t *ent) {
	vec3_t	abs_movedir;
	float	distance;
	vec3_t	size;
	float	lip;
	//Pardner: if SILENT flag not set, use the default door sounds. 
	//Use_BinaryMover() already handles entities without set sounds.
#ifdef SMOKINGUNS
	if ( !(ent->spawnflags & 32) ) {
#endif

	ent->sound1to2 = ent->sound2to1 = G_SoundIndex("sound/movers/doors/dr1_strt.wav");
	ent->soundPos1 = ent->soundPos2 = G_SoundIndex("sound/movers/doors/dr1_end.wav");

#ifdef SMOKINGUNS
	}
#endif
	ent->blocked = Blocked_Door;

	// default speed of 400
	if (!ent->speed)
		ent->speed = 400;

	// default wait of 2 seconds
	if (!ent->wait)
		ent->wait = 2;
	ent->wait *= 1000;

	// default lip of 8 units
	G_SpawnFloat( "lip", "8", &lip );

#ifndef SMOKINGUNS
	// default damage of 2 points
	G_SpawnInt( "dmg", "2", &ent->damage );
#else
	// default damage of 0 points
	G_SpawnInt( "dmg", "0", &ent->damage );
#endif

	// first position at start
	VectorCopy( ent->s.origin, ent->pos1 );

	// calculate second position
	trap_SetBrushModel( ent, ent->model );
#ifndef SMOKINGUNS
	G_SetMovedir (ent->s.angles, ent->movedir);
	abs_movedir[0] = fabs(ent->movedir[0]);
	abs_movedir[1] = fabs(ent->movedir[1]);
	abs_movedir[2] = fabs(ent->movedir[2]);
	VectorSubtract( ent->r.maxs, ent->r.mins, size );
	distance = DotProduct( abs_movedir, size ) - lip;
	VectorMA( ent->pos1, distance, ent->movedir, ent->pos2 );

	// if "start_open", reverse position 1 and 2
	if ( ent->spawnflags & 1 ) {
		vec3_t	temp;

		VectorCopy( ent->pos2, temp );
		VectorCopy( ent->s.origin, ent->pos2 );
		VectorCopy( temp, ent->pos1 );
	}
#else
	if(!ent->movedir[0] && !ent->movedir[1] &&!ent->movedir[2]){
		G_SetMovedir (ent->s.angles, ent->movedir);

		abs_movedir[0] = fabs(ent->movedir[0]);
		abs_movedir[1] = fabs(ent->movedir[1]);
		abs_movedir[2] = fabs(ent->movedir[2]);
		VectorSubtract( ent->r.maxs, ent->r.mins, size );
		distance = DotProduct( abs_movedir, size ) - lip;
		VectorMA( ent->pos1, distance, ent->movedir, ent->pos2 );

		// if "start_open", reverse position 1 and 2
		if ( ent->spawnflags & 1 ) {
			vec3_t	temp;

			VectorCopy( ent->pos2, temp );
			VectorCopy( ent->s.origin, ent->pos2 );
			VectorCopy( temp, ent->pos1 );
		}
	}
#endif

	InitMover( ent );

	ent->nextthink = level.time + FRAMETIME;

#ifndef SMOKINGUNS
	if ( ! (ent->flags & FL_TEAMSLAVE ) ) {
		int health;

		G_SpawnInt( "health", "0", &health );
		if ( health ) {
			ent->takedamage = qtrue;
		}
		if ( ent->targetname || health ) {
			// non touch/shoot doors
			ent->think = Think_MatchTeam;
		} else {
			ent->think = Think_SpawnNewDoorTrigger;
		}
	}

#else
	if ( ! (ent->flags & FL_TEAMSLAVE ) ) {
		ent->think = Think_MatchTeam;
	}

	// marks the door as being usable
	if(!(ent->spawnflags & TRIGGER_DOOR)){
		VectorClear(ent->s.angles2);
		ent->s.angles2[0] = -1000;
	}

	// Mark the door to block on entities
	ent->s.eFlags |= EF_MOVER_STOP ;
#endif

}

#ifdef SMOKINGUNS
/*QUAKED func_door_rotating (0 .5 .8) ? START_OPEN - CRUSHER RETURN TRIGGER_DOOR
START_OPEN	the door to moves to its destination when spawned, and operate in reverse.  It is used to temporarily or permanently close off an area when triggered (not useful for touch or takedamage doors).
CRUSHER : door will not reverse direction when blocked and will keep damaging player until he dies or gets out of the way.
RETURN: door will return automatically after having been opened
TRIGGER_DOOR: door can't be used by an activate button but a trigger
X_AXIS : open on the X-axis instead of the Z-axis
Y_AXIS : open on the Y-axis instead of the Z-axis
ONE_WAY : always open the same way, set negative distance to reverse
SILENT: supresses the default door sounds. 

"model2"	.md3 model to also draw
"targetname" if set, no touch field will be spawned and a remote button or trigger field activates the door.
"speed"		movement speed (130 default)
"distance"	how many degrees the door will open, negative to reverse direction (default 90)
"wait"		wait before returning (3 default, -1 = never return)
"lip"		lip remaining at end of move (8 default)
"dmg"		damage to inflict when blocked (2 default)
"color"		constantLight color
"light"		constantLight radius
"health"	if set, the door must be shot open
*/
void SP_func_door_rotating (gentity_t *ent) {
	int	distance;

	ent->s.pos.trType = TR_STATIONARY;
	ent->s.apos.trType = TR_STATIONARY;
	//Pardner: if SILENT flag not set, use the default door sounds. 
	//Use_BinaryMover() already handles entities without set sounds.
#ifdef SMOKINGUNS
	if ( !(ent->spawnflags & 256) ) {
#endif

	ent->sound1to2 = ent->sound2to1 = G_SoundIndex("sound/movers/doors/dr1_strt.wav");
	ent->soundPos1 = ent->soundPos2 = G_SoundIndex("sound/movers/doors/dr1_end.wav");

#ifdef SMOKINGUNS
	}
#endif
	ent->blocked = Blocked_Door;

	// default speed of 130
	if (!ent->speed)
		ent->speed = 130;

	// only initialize the first time
	if(!ent->count){
		G_SpawnInt( "distance", "90", &distance);

		if(!distance)
			distance = 90;

		ent->count = distance;
		
		VectorCopy( ent->s.pos.trBase, ent->pos1);
		VectorCopy( ent->s.pos.trBase, ent->pos2);
		VectorCopy( ent->s.apos.trBase, ent->apos1);
		VectorCopy( ent->s.apos.trBase, ent->apos2);
		
		ent->apos2[1] += ent->count;

		// default wait of 4 seconds
		if (!ent->wait)
			ent->wait = 4;

		ent->wait *= 1000;

		// default damage of 0 points
		G_SpawnInt( "dmg", "0", &ent->damage );

		// calculate second position
		trap_SetBrushModel( ent, ent->model );

		// if "start_open", reverse position 1 and 2
		if ( ent->spawnflags & 1 ) {
			vec3_t	temp;

			VectorCopy( ent->apos2, temp );
			//VectorCopy( ent->s.origin, ent->apos2 );
			VectorCopy( ent->s.angles, ent->apos2 );
			VectorCopy( temp, ent->apos1 );
		}
	}

	// Mark entity as a rotating door blocking on other entities
	ent->s.eFlags |= (EF_MOVER_STOP|EF_ROTATING_DOOR) ;

	InitMover( ent );

	ent->nextthink = level.time + FRAMETIME;

	if ( ! (ent->flags & FL_TEAMSLAVE ) ) {
		ent->think = Think_MatchTeam;
	}
	VectorCopy( ent->s.origin, ent->s.pos.trBase );
	VectorCopy( ent->s.pos.trBase, ent->r.currentOrigin );
	VectorCopy( ent->s.apos.trBase, ent->r.currentAngles );

	// marks the door as being usable
	if(!(ent->spawnflags & TRIGGER_DOOR)){
		VectorClear(ent->s.angles2);
		ent->s.angles2[0] = -1000;
	}
}

#define BREAK_RESPAWN_TIME 45000

void func_breakable_die( gentity_t *self, gentity_t *inflictor, gentity_t *attacker, int damage, int meansOfDeath ) {

	gentity_t	*temp;

	//remove from world
	self->r.contents = CONTENTS_MOVER;
	self->takedamage = qfalse;
	self->flags |= EF_BROKEN;
	self->s.powerups = FARCLIP_NONE ;
	// respawn breakable in deathmatch
	if (g_breakspawndelay.integer > 0)
		self->wait = level.time + g_breakspawndelay.integer * 1000;
	else
		self->wait = level.time + BREAK_RESPAWN_TIME;

	G_UseTargets(self, attacker);

	trap_UnlinkEntity(self);

	//launch particles
	temp = G_TempEntity(self->s.pos.trBase, EV_FUNCBREAKABLE);

	VectorCopy(self->r.maxs, temp->s.origin);
	VectorCopy(self->r.mins, temp->s.origin2);
	temp->r.svFlags |= SVF_BROADCAST;

	// set surfaceType // with the torsoAnim set to 1 it transfers a surfaceFlag
	if(self->trio){
		// save in float because of more memory
		temp->s.angles2[0] = self->trio;
		temp->s.torsoAnim = 1;
	} else {
		temp->s.eventParm = self->count;
		temp->s.torsoAnim = 0;
	}

//	G_LogPrintf("breakable die\n");
}

//QUAKED func_breakable (0 .5 .8) ?

// spawnflags, manually set breakables types
#define TYPE_METAL	4
#define TYPE_WOOD	8
#define	TYPE_CLOTH	16
#define TYPE_DIRT	32
#define	TYPE_GLASS	64
#define	TYPE_STONE	128
void SP_func_breakable (gentity_t *ent){
	float	size;
	char	*s;
	gitem_t	*item;

	// look for a manually-set-type
	if(ent->spawnflags){

		if(ent->spawnflags & TYPE_METAL){
			ent->trio = SURF_METAL;
		} else if(ent->spawnflags & TYPE_WOOD){
			ent->trio = SURF_WOOD;
		} else if(ent->spawnflags & TYPE_CLOTH){
			ent->trio = SURF_CLOTH;
		} else if(ent->spawnflags & TYPE_DIRT){
			ent->trio = SURF_DIRT;
		} else if(ent->spawnflags & TYPE_GLASS){
			ent->trio = SURF_GLASS;
		} else if(ent->spawnflags & TYPE_STONE){
			ent->trio = SURF_STONE;
		} else
			ent->trio = 0;
	} else {
		ent->trio = 0;
	}

	trap_SetBrushModel( ent, ent->model );
	
	// Joe Kari: this prevent from collision with func_static far clipping
	ent->s.powerups = FARCLIP_NONE ;

	//set weapon by which it can be destroyed, 0 = every weapon
	if(!ent->s.weapon){
		G_SpawnString( "weapon", "", &s );
		item = BG_FindItemForClassname(s);

		if(item){
			if(item->giType == IT_WEAPON)
				ent->s.weapon = item->giTag;
		} else {
			ent->s.weapon = 0;
		}
	}

	size = Distance(ent->r.absmin, ent->r.absmax);
	size = fabs(size)/3;

	if (ent->s.angles2[0])
		ent->health = ent->s.angles2[0];
	else if (!ent->health)
		ent->health = size;

	ent->s.angles2[0] = ent->health;

	// Tequila comment: set breakable center as origin for G_BreakableRespawn needs
	VectorSubtract(ent->r.absmax, ent->r.absmin, ent->pos1);
	VectorScale(ent->pos1,0.5f,ent->pos2);
	VectorAdd(ent->pos2,ent->r.absmin,ent->s.origin);
	
	VectorCopy( ent->s.origin, ent->pos1 );
	VectorCopy( ent->s.origin, ent->pos2 );

	ent->die = func_breakable_die;
	ent->takedamage = qtrue;
	ent->s.eType = ET_BREAKABLE;
	ent->r.svFlags &= ~SVF_NOCLIENT;
	ent->s.eFlags &= ~EF_NODRAW;
	ent->flags = 0;

	// initialize it, if nessecary
	G_LookForBreakableType(ent);

	ent->r.contents = (CONTENTS_BODY|CONTENTS_MOVER);
	ent->wait = 0;

	trap_LinkEntity(ent);
}

#define REPORTED_RESPAWN_TIME 1000
void G_BreakableRespawn( gentity_t *ent){
	int i;

	// cycle through all players and see if the breakable respawn would be visible to them
	for(i = 0; i < level.maxclients; i++){
		gentity_t *player = &g_entities[i];
		gclient_t *client = &level.clients[i];
		float diff, angle;
		vec3_t angles, dir;
		vec3_t eye;
		const float fov = 100;
		qboolean cont = qfalse;
		int j;

		if (client->pers.connected != CON_CONNECTED)
			continue;

		// if it's too near abort
		if(Distance(client->ps.origin, ent->pos2) < 300){
			return;
		}

		// first check if player could be stuck in the breakable
		if ( player->r.absmin[0] >= ent->r.absmax[0]
			|| player->r.absmin[1] >= ent->r.absmax[1]
			|| player->r.absmin[2] >= ent->r.absmax[2]
			|| player->r.absmax[0] <= ent->r.absmin[0]
			|| player->r.absmax[1] <= ent->r.absmin[1]
			|| player->r.absmax[2] <= ent->r.absmin[2] ) {
		} else {
			return;
		}

		// Don't process field of vision tests
		if (g_forcebreakrespawn.integer)
			continue;

		// Tequila comment: Minor server optimization, don't check if breakable is in a bot FOV
		// They really don't care to "see" a breakable respawn, so we won't delay the respawn
		// because of bot proximity.
		if (player->r.svFlags & SVF_BOT) {
			continue;
		}

		// check if its in field of vision
		VectorCopy(client->ps.origin, eye);
		eye[2] += client->ps.viewheight;

		VectorSubtract(ent->pos2, eye, dir);
		vectoangles(dir, angles);

		for (j = 0; j < 2; j++) {
			angle = AngleMod(client->ps.viewangles[j]);
			angles[j] = AngleMod(angles[j]);
			diff = fabs(angles[j] - angle);

			if (diff > 180.0)
				diff -= 360.0;

			// if not in field of vision continue;
			if ( fabs(diff) > fov/2 ) {
				cont = qtrue;
				break;
			}
		}

		// it's in the field of vision
		if(!cont){
			// Tequila comment: Delay the next breakable respawn check
			ent->wait = level.time + REPORTED_RESPAWN_TIME;
			return;
		}
	}

	// now while nobody can see it respawn the breakable
	VectorCopy(ent->pos1, ent->s.origin);
	SP_func_breakable(ent);
}

void G_MoverContents(qboolean change){
	int i;
	gentity_t *ent;

	for(i=0; i<MAX_GENTITIES; i++){
		ent = &g_entities[i];

		if(ent->s.eType == ET_MOVER &&
			(ent->r.contents & CONTENTS_BODY)){

			if(change)
				ent->r.contents |= CONTENTS_SOLID;
			else
				ent->r.contents &= ~CONTENTS_SOLID;
		}
	}
}

void Think_SmokeInit( gentity_t *ent ){
	// if we have a wind direction
	if( ent->target){
		gentity_t *target = G_PickTarget( ent->target );
		vec3_t dir;

		if(target){
			VectorSubtract(target->r.currentOrigin, ent->s.origin, dir);
			VectorNormalize(dir);

			VectorCopy(dir, ent->s.apos.trBase);
		}
	}
	ent->think = 0;
	ent->nextthink = 0;
}

/*QUAKED func_smoke (0 .5 .8) ?
Generates Smoke
-------- KEYS --------
size : size of the smoke particles (default 50) (time)
trans : transparency of the smoke (default 0.8, max 1.0) (angles[0])
color : color of the smoke ( default 1 1 1) (angles2)
life : lifetime of the smoke (default 5000 (msecs)) (time2)
rate : generation rate of the smoke-sprites (default 3 (sprites per second)) (frame)
target : sets the direction of the wind (direction: apos.trBase)
wind : strength of the wind (default 5) (torsoAnim)
boost : rise-up-velocity of the sprites (default 5) (legsAnim)
*/
void SP_func_smoke (gentity_t *ent) {
	ent->s.eType = ET_SMOKE;
	ent->flags = ent->spawnflags;
	ent->s.eFlags = ent->spawnflags;

	trap_SetBrushModel( ent, ent->model );

	ent->r.contents = 0;

	G_SpawnInt( "size", "50", &ent->s.time );
	G_SpawnFloat( "trans", "0.6", &ent->s.angles[0]);
	G_SpawnVector( "color", "1 1 1", ent->s.angles2);
	G_SpawnInt( "lifetime", "5000", &ent->s.time2);
	G_SpawnInt( "rate", "3", &ent->s.frame);
	G_SpawnInt( "wind", "5", &ent->s.torsoAnim);
	G_SpawnInt( "boost", "5", &ent->s.legsAnim);

	VectorCopy(ent->r.absmax, ent->s.apos.trDelta);
	VectorCopy(ent->r.absmin, ent->s.pos.trDelta);
	VectorAdd(ent->r.absmax, ent->r.absmin, ent->s.origin);
	VectorScale(ent->s.origin, 0.5, ent->s.origin);
	VectorCopy( ent->s.origin, ent->pos1 );

	VectorClear(ent->s.apos.trBase);
	ent->think = Think_SmokeInit;
	ent->nextthink = 1;

	trap_LinkEntity(ent);
}

#define NO_GLOW	1
#define	NO_LENSFLARES	2
void SP_func_flare (gentity_t *ent) {
	vec3_t origin;

	VectorCopy( ent->s.origin, ent->pos1 );

	ent->s.eType = ET_FLARE;
	ent->flags = ent->spawnflags;
	ent->s.eFlags = ent->spawnflags;

	trap_SetBrushModel( ent, ent->model );

	ent->r.contents = CONTENTS_DETAIL;

	G_SpawnInt( "maxsize", "120", &ent->s.time );
	G_SpawnInt( "minsize", "30", &ent->s.time2);
	G_SpawnInt( "radius", "20", &ent->s.frame);
	G_SpawnFloat( "trans", "0.6", &ent->s.angles[0]);
	G_SpawnVector( "color", "1 1 1", ent->s.angles2);

	VectorAdd(ent->r.absmax, ent->r.absmin, origin);
	VectorScale(origin, 0.5, origin);
	VectorCopy(origin, ent->s.origin);

	trap_LinkEntity(ent);
}
#endif

/*
===============================================================================

PLAT

===============================================================================
*/

/*
==============
Touch_Plat

Don't allow decent if a living player is on it
===============
*/
void Touch_Plat( gentity_t *ent, gentity_t *other, trace_t *trace ) {
	if ( !other->client || other->client->ps.stats[STAT_HEALTH] <= 0 ) {
		return;
	}

	// delay return-to-pos1 by one second
	if ( ent->moverState == MOVER_POS2 ) {
		ent->nextthink = level.time + 1000;
	}
}

/*
==============
Touch_PlatCenterTrigger

If the plat is at the bottom position, start it going up
===============
*/
void Touch_PlatCenterTrigger(gentity_t *ent, gentity_t *other, trace_t *trace ) {
	if ( !other->client ) {
		return;
	}

	if ( ent->parent->moverState == MOVER_POS1 ) {
		Use_BinaryMover( ent->parent, ent, other );
	}
}


/*
================
SpawnPlatTrigger

Spawn a trigger in the middle of the plat's low position
Elevator cars require that the trigger extend through the entire low position,
not just sit on top of it.
================
*/
void SpawnPlatTrigger( gentity_t *ent ) {
	gentity_t	*trigger;
	vec3_t	tmin, tmax;

	// the middle trigger will be a thin trigger just
	// above the starting position
	trigger = G_Spawn();
	trigger->classname = "plat_trigger";
	trigger->touch = Touch_PlatCenterTrigger;
	trigger->r.contents = CONTENTS_TRIGGER;
	trigger->parent = ent;

	tmin[0] = ent->pos1[0] + ent->r.mins[0] + 33;
	tmin[1] = ent->pos1[1] + ent->r.mins[1] + 33;
	tmin[2] = ent->pos1[2] + ent->r.mins[2];

	tmax[0] = ent->pos1[0] + ent->r.maxs[0] - 33;
	tmax[1] = ent->pos1[1] + ent->r.maxs[1] - 33;
	tmax[2] = ent->pos1[2] + ent->r.maxs[2] + 8;

	if ( tmax[0] <= tmin[0] ) {
		tmin[0] = ent->pos1[0] + (ent->r.mins[0] + ent->r.maxs[0]) *0.5;
		tmax[0] = tmin[0] + 1;
	}
	if ( tmax[1] <= tmin[1] ) {
		tmin[1] = ent->pos1[1] + (ent->r.mins[1] + ent->r.maxs[1]) *0.5;
		tmax[1] = tmin[1] + 1;
	}

	VectorCopy (tmin, trigger->r.mins);
	VectorCopy (tmax, trigger->r.maxs);

	trap_LinkEntity (trigger);
}


/*QUAKED func_plat (0 .5 .8) ?
Plats are always drawn in the extended position so they will light correctly.

SILENT: supresses the default plat sounds. 

"lip"		default 8, protrusion above rest position
"height"	total height of movement, defaults to model height
"speed"		overrides default 200.
"dmg"		overrides default 2
"model2"	.md3 model to also draw
"color"		constantLight color
"light"		constantLight radius
*/
void SP_func_plat (gentity_t *ent) {
	float		lip, height;
	//Pardner: if SILENT flag not set, use the default plat sounds. 
	//Use_BinaryMover() already handles entities without set sounds.
#ifdef SMOKINGUNS
	if ( !(ent->spawnflags & 1) ) {
#endif

	ent->sound1to2 = ent->sound2to1 = G_SoundIndex("sound/movers/plats/pt1_strt.wav");
	ent->soundPos1 = ent->soundPos2 = G_SoundIndex("sound/movers/plats/pt1_end.wav");

#ifdef SMOKINGUNS
	}
#endif
	VectorClear (ent->s.angles);

	G_SpawnFloat( "speed", "200", &ent->speed );
	G_SpawnInt( "dmg", "2", &ent->damage );
	G_SpawnFloat( "wait", "1", &ent->wait );
	G_SpawnFloat( "lip", "8", &lip );

	ent->wait = 1000;

	// create second position
	trap_SetBrushModel( ent, ent->model );

	if ( !G_SpawnFloat( "height", "0", &height ) ) {
		height = (ent->r.maxs[2] - ent->r.mins[2]) - lip;
	}

	// pos1 is the rest (bottom) position, pos2 is the top
	VectorCopy( ent->s.origin, ent->pos2 );
	VectorCopy( ent->pos2, ent->pos1 );
	ent->pos1[2] -= height;

	InitMover( ent );

	// touch function keeps the plat from returning while
	// a live player is standing on it
	ent->touch = Touch_Plat;

	ent->blocked = Blocked_Door;

	ent->parent = ent;	// so it can be treated as a door

	// spawn the trigger if one hasn't been custom made
	if ( !ent->targetname ) {
		SpawnPlatTrigger(ent);
	}
}


/*
===============================================================================

BUTTON

===============================================================================
*/

/*
==============
Touch_Button

===============
*/
void Touch_Button(gentity_t *ent, gentity_t *other, trace_t *trace ) {
	if ( !other->client ) {
		return;
	}

	if ( ent->moverState == MOVER_POS1 ) {
		Use_BinaryMover( ent, other, other );
	}
}


/*QUAKED func_button (0 .5 .8) ?
When a button is touched, it moves some distance in the direction of its angle, triggers all of its targets, waits some time, then returns to its original position where it can be triggered again.

"model2"	.md3 model to also draw
"angle"		determines the opening direction
"target"	all entities with a matching targetname will be used
"speed"		override the default 40 speed
"wait"		override the default 1 second wait (-1 = never return)
"lip"		override the default 4 pixel lip remaining at end of move
"health"	if set, the button must be killed instead of touched
"color"		constantLight color
"light"		constantLight radius
*/
void SP_func_button( gentity_t *ent ) {
	vec3_t		abs_movedir;
	float		distance;
	vec3_t		size;
	float		lip;

	ent->sound1to2 = G_SoundIndex("sound/movers/switches/butn2.wav");

	if ( !ent->speed ) {
		ent->speed = 40;
	}

	if ( !ent->wait ) {
		ent->wait = 1;
	}
	ent->wait *= 1000;

	// first position
	VectorCopy( ent->s.origin, ent->pos1 );

	// calculate second position
	trap_SetBrushModel( ent, ent->model );

	G_SpawnFloat( "lip", "4", &lip );

	G_SetMovedir( ent->s.angles, ent->movedir );
	abs_movedir[0] = fabs(ent->movedir[0]);
	abs_movedir[1] = fabs(ent->movedir[1]);
	abs_movedir[2] = fabs(ent->movedir[2]);
	VectorSubtract( ent->r.maxs, ent->r.mins, size );
	distance = abs_movedir[0] * size[0] + abs_movedir[1] * size[1] + abs_movedir[2] * size[2] - lip;
	VectorMA (ent->pos1, distance, ent->movedir, ent->pos2);

	if (ent->health) {
		// shootable button
		ent->takedamage = qtrue;
	} else {
		// touchable button
		ent->touch = Touch_Button;
	}

	InitMover( ent );
}



/*
===============================================================================

TRAIN

===============================================================================
*/


#define TRAIN_START_ON		1
#define TRAIN_TOGGLE		2
#define TRAIN_BLOCK_STOPS	4

#ifdef SMOKINGUNS
#define TRAIN_MOV_SYNC_SPEEDUP 8
#define TRAIN_MOV_SYNC_SLOWDOWN 16
#define TRAIN_ROT_SYNC_SPEEDUP 32
#define TRAIN_ROT_SYNC_SLOWDOWN 64
#define TRAIN_ROTATION_ONLY 128
#define TRAIN_RELATIVE_MOVE 256
#endif

/*
===============
Think_BeginMoving

The wait time at a corner has completed, so start moving again
===============
*/
void Think_BeginMoving( gentity_t *ent ) {
	ent->s.pos.trTime = level.time;
	ent->s.pos.trType = TR_LINEAR_STOP;
	ent->s.apos.trTime = level.time;
	ent->s.apos.trType = TR_LINEAR_STOP;
}

/*
===============
Reached_Train
===============
*/
void Reached_Train( gentity_t *ent ) {
	gentity_t		*next;
	float			speed;
	float			aspeed;
	vec3_t			tmp_vector;
	vec3_t			move;
	vec3_t			amove;
	float			length;
	float			alength;
	qboolean		rotation_only;
	qboolean		relative_move;
	qboolean		rot_sync_speedup;
	qboolean		rot_sync_slowdown;
	qboolean		mov_sync_speedup;
	qboolean		mov_sync_slowdown;

	// copy the apropriate values
	next = ent->nextTrain;
	if ( !next || !next->nextTrain ) {
		return;		// just stop
	}

	// fire all other targets
	G_UseTargets( next, NULL );
	
	// set the new trajectory
	ent->nextTrain = next->nextTrain;
	
#ifdef SMOKINGUNS
	// preliminaries...
	relative_move = ent->spawnflags & TRAIN_RELATIVE_MOVE || next->spawnflags & TRAIN_RELATIVE_MOVE ;
	rotation_only = ent->spawnflags & TRAIN_ROTATION_ONLY || next->spawnflags & TRAIN_ROTATION_ONLY ;
	rot_sync_speedup = ent->spawnflags & TRAIN_ROT_SYNC_SPEEDUP || next->spawnflags & TRAIN_ROT_SYNC_SPEEDUP ;
	rot_sync_slowdown = ent->spawnflags & TRAIN_ROT_SYNC_SLOWDOWN || next->spawnflags & TRAIN_ROT_SYNC_SLOWDOWN ;
	mov_sync_speedup = ent->spawnflags & TRAIN_MOV_SYNC_SPEEDUP || next->spawnflags & TRAIN_MOV_SYNC_SPEEDUP ;
	mov_sync_slowdown = ent->spawnflags & TRAIN_MOV_SYNC_SLOWDOWN || next->spawnflags & TRAIN_MOV_SYNC_SLOWDOWN ;
	
	if ( rotation_only ) {
		// The train coordinate never change
		VectorCopy( ent->s.origin, ent->pos1 );
		VectorCopy( ent->s.origin, ent->pos2 );
	} else if ( relative_move ) {
		// The train start from its own location and move relatively to it
		VectorSubtract( ent->s.origin, ent->firstTrain->s.origin, tmp_vector );
		VectorAdd( next->s.origin, tmp_vector, ent->pos1 );
		VectorAdd( next->nextTrain->s.origin, tmp_vector, ent->pos2 );
	} else {
		// Standard case, coordinate are taken from the current path_corner origin
		VectorCopy( next->s.origin, ent->pos1 );
		VectorCopy( next->nextTrain->s.origin, ent->pos2 );
	}
	VectorCopy( next->s.angles, ent->apos1 );
	VectorCopy( next->nextTrain->s.angles, ent->apos2 );
#else
	VectorCopy( next->s.origin, ent->pos1 );
	VectorCopy( next->nextTrain->s.origin, ent->pos2 );
#endif
	
	
	// if the path_corner has a speed, use that
	if ( next->speed ) {
		speed = next->speed;
	} else {
		// otherwise use the train's speed
		speed = ent->speed;
	}
	if ( speed < 1 ) {
		speed = 1;
	}
	
#ifdef SMOKINGUNS
	// if the path_corner has a "aspeed", use that
	if ( next->aspeed ) {
		aspeed = next->aspeed;
	} else {
		// otherwise use the train's "aspeed"
		aspeed = ent->aspeed;
	}
	if ( aspeed < 1 ) {
		aspeed = 1;
	}
#endif
	
	// calculate duration
	VectorSubtract( ent->pos2, ent->pos1, move );
	length = VectorLength( move );
	ent->s.pos.trDuration = length * 1000 / speed;

#ifdef SMOKINGUNS
	AnglesSubtract( ent->apos2, ent->apos1, amove );
	alength = VectorLength( amove );
	ent->s.apos.trDuration = alength * 1000 / aspeed;

	// Tequila comment: Be sure to send to clients after any fast move case
	ent->r.svFlags &= ~SVF_NOCLIENT;

	// Tequila comment: Fast move case
	if(ent->s.pos.trDuration<1) {
		// Tequila comment: As trDuration is used later in a division, we need to avoid that case now
		// With null trDuration,
		// the calculated rocks bounding box becomes infinite and the engine think for a short time
		// any entity is riding that mover but not the world entity... In rare case, I found it
		// can also stuck every map entities after func_door are used.
		// The desired effect with very very big speed is to have instant move, so any not null duration
		// lower than a frame duration should be sufficient.
		// Afaik, the negative case don't have to be supported.
		ent->s.pos.trDuration=1;

		// Tequila comment: Don't send entity to clients so it becomes really invisible 
		// Joe Kari: can cause bugs, commented out... (especially it limits/bugs rotating train)
		//ent->r.svFlags |= SVF_NOCLIENT;
	}
	
	// Joe Kari: same for fast angles move...
	if(ent->s.apos.trDuration<1) {
		ent->s.apos.trDuration=1;
	}
	
	
	// Here we are computing translation/rotation synchronization, if needed
	
	if ( ent->s.apos.trDuration > ent->s.pos.trDuration ) {
		// Rotation is slower than translation
		
		if ( rot_sync_speedup && mov_sync_slowdown ) {
			// Both synchronize, so we take the average duration...
			ent->s.pos.trDuration = ( ent->s.pos.trDuration + ent->s.apos.trDuration ) / 2 ;
			ent->s.apos.trDuration = ent->s.pos.trDuration ;
		}
		else if ( rot_sync_speedup ) {
			// Rotation can speed up to synchronize...
			ent->s.apos.trDuration = ent->s.pos.trDuration ;
		}
		else if ( mov_sync_slowdown ) {
			// Translation can slow down to synchronize...
			ent->s.pos.trDuration = ent->s.apos.trDuration ;
		}
	}
	else if ( ent->s.pos.trDuration > ent->s.apos.trDuration ) {
		// Translation is slower than rotation
		
		if ( rot_sync_slowdown && mov_sync_speedup ) {
			// Both synchronize, so we take the average duration...
			ent->s.pos.trDuration = ( ent->s.pos.trDuration + ent->s.apos.trDuration ) / 2 ;
			ent->s.apos.trDuration = ent->s.pos.trDuration ;
		}
		else if ( rot_sync_slowdown ) {
			// Rotation can slow down to synchronize...
			ent->s.apos.trDuration = ent->s.pos.trDuration ;
		}
		else if ( mov_sync_speedup ) {
			// Translation can speed up to synchronize...
			ent->s.pos.trDuration = ent->s.apos.trDuration ;
		}
	}
#endif

	// looping sound
	ent->s.loopSound = next->soundLoop;

	// start it going
	SetMoverState( ent, MOVER_1TO2, level.time );

	// if there is a "wait" value on the target, don't start moving yet
	if ( next->wait ) {
		ent->nextthink = level.time + next->wait * 1000;
		ent->think = Think_BeginMoving;
		ent->s.pos.trType = TR_STATIONARY;
#ifdef SMOKINGUNS
		ent->s.apos.trType = TR_STATIONARY;
#endif
	}
}


/*
===============
Think_SetupTrainTargets

Link all the corners together
===============
*/
void Think_SetupTrainTargets( gentity_t *ent ) {
	gentity_t		*path, *next, *start;
	
	ent->nextTrain = G_Find( NULL, FOFS(targetname), ent->target );
#ifdef SMOKINGUNS
	// Joe Kari: usefull for the TRAIN_RELATIVE_MOVE spawnflags
	ent->firstTrain = ent->nextTrain ;
#endif
	if ( !ent->nextTrain ) {
		G_Printf( "func_train at %s with an unfound target\n",
			vtos(ent->r.absmin) );
		return;
	}

	start = NULL;
	for ( path = ent->nextTrain ; path != start ; path = next ) {
		if ( !start ) {
			start = path;
		}

		if ( !path->target ) {
			G_Printf( "Train corner at %s without a target\n",
				vtos(path->s.origin) );
			return;
		}

		// find a path_corner among the targets
		// there may also be other targets that get fired when the corner
		// is reached
		next = NULL;
		do {
			next = G_Find( next, FOFS(targetname), path->target );
			if ( !next ) {
				G_Printf( "Train corner at %s without a target path_corner\n",
					vtos(path->s.origin) );
				return;
			}
		} while ( strcmp( next->classname, "path_corner" ) );

		path->nextTrain = next;
	}

	// start the train moving from the first corner
	Reached_Train( ent );
}



/*QUAKED path_corner (.5 .3 0) (-8 -8 -8) (8 8 8)
Train path corners.
Target: next path corner and other targets to fire
"speed" speed to move to the next corner
"aspeed" "angular speed" to move to the next corner
"wait" seconds to wait before behining move to next corner
*/
void SP_path_corner( gentity_t *self ) {
	if ( !self->targetname ) {
		G_Printf ("path_corner with no targetname at %s\n", vtos(self->s.origin));
		G_FreeEntity( self );
		return;
	}
	
#ifdef SMOKINGUNS
	G_SpawnFloat( "aspeed", "0", &self->aspeed );
#endif

	// path corners don't need to be linked in
}



/*QUAKED func_train (0 .5 .8) ? START_ON TOGGLE BLOCK_STOPS
A train is a mover that moves between path_corner target points.
Trains MUST HAVE AN ORIGIN BRUSH.
The train spawns at the first target it is pointing at.
"model2"	.md3 model to also draw
"speed"		default 100
"aspeed"	"angular speed", default 90
"dmg"		default	2
"noise"		looping sound to play when the train is in motion
"target"	next path corner
"color"		constantLight color
"light"		constantLight radius
*/
void SP_func_train (gentity_t *self) {
	VectorClear (self->s.angles);

	if (self->spawnflags & TRAIN_BLOCK_STOPS) {
		self->damage = 0;
	} else {
		if (!self->damage) {
			self->damage = 2;
		}
	}

	if ( !self->speed ) {
		self->speed = 100;
	}
	
#ifdef SMOKINGUNS
	G_SpawnFloat( "aspeed", "90", &self->aspeed );
#endif
	
	if ( !self->target ) {
		G_Printf ("func_train without a target at %s\n", vtos(self->r.absmin));
		G_FreeEntity( self );
		return;
	}

	trap_SetBrushModel( self, self->model );
	
	InitMover( self );

	self->reached = Reached_Train;

	// start trains on the second frame, to make sure their targets have had
	// a chance to spawn
	self->nextthink = level.time + FRAMETIME;
	self->think = Think_SetupTrainTargets;
}

/*
===============================================================================

STATIC

===============================================================================
*/


/*QUAKED func_static (0 .5 .8) ?
A bmodel that just sits there, doing nothing.  Can be used for conditional walls and models.
"model2"	.md3 model to also draw
"color"		constantLight color
"light"		constantLight radius
*/
#ifndef SMOKINGUNS
void SP_func_static( gentity_t *ent ) {
	trap_SetBrushModel( ent, ent->model );
	InitMover( ent );
	VectorCopy( ent->s.origin, ent->s.pos.trBase );
	VectorCopy( ent->s.origin, ent->r.currentOrigin );
}
#else
void SP_func_static( gentity_t *ent )
{
	
	// Joe Kari: new func_static (totally rewritten) with far-clipping and level of detail support
	// with remainder-style comment
	
	float		light ;
	vec3_t		color ;
	qboolean	lightSet , colorSet ;
	char		*value ;
	char		tmp_char[32] ;
	int		tmp_int ;
	
	// register brush model to physical engine
	trap_SetBrushModel( ent , ent->model ) ;
	
	// because a func_static could have is own origin, this is needed to replace its coord in world coord
	// (automatically made for game, but not for cgame)
	VectorCopy( ent->r.currentOrigin , ent->s.origin ) ;
	
	// this is needed for CG_Mover() to display the element (yes func_static was used like a mover)
	ent->s.eType = ET_MOVER ;
	
	// just a new value to ensure that the game will never perform "think" for this entity
	ent->moverState = MOVER_STATIC;
	
	// if the "color" or "light" keys are set, setup constantLight
	lightSet = G_SpawnFloat( "light" , "100" , &light ) ;
	colorSet = G_SpawnVector( "color" , "1 1 1" , color ) ;
	if ( lightSet || colorSet )
	{
		int r , g , b , i ;
		r = color[0] * 255 ;
		if ( r > 255 )  r = 255 ;
		g = color[1] * 255 ;
		if ( g > 255 )  g = 255 ;
		b = color[2] * 255 ;
		if ( b > 255 )  b = 255 ;
		i = light / 4 ;
		if ( i > 255 )  i = 255 ;
		ent->s.constantLight = r | ( g << 8 ) | ( b << 16 ) | ( i << 24 ) ;
	}
	
	// if the "model2" key is set, use a seperate model
	// for drawing, but clip against the brushes
	if ( ent->model2 )  ent->s.modelindex2 = G_ModelIndex( ent->model2 ) ;

	
	// get the value for the key "farclip" and copy it to powerups (7 lower bits), legsAnim and torsoAnim field
	
	G_SpawnString( "farclip" , "none" , &value ) ;
	sscanf( value , "%31s %i %i" , tmp_char , &ent->s.legsAnim , &ent->s.torsoAnim ) ;
	
	ent->s.powerups = FARCLIP_NONE ;
	
	if ( !Q_stricmp( "none" , tmp_char ) )  ent->s.powerups = FARCLIP_NONE ;
	else if ( !Q_stricmp( "always" , tmp_char ) )  ent->s.powerups = FARCLIP_ALWAYS ;
	else if ( !Q_stricmp( "sphere" , tmp_char ) )  ent->s.powerups = FARCLIP_SPHERE ;
	else if ( !Q_stricmp( "cube" , tmp_char ) )  ent->s.powerups = FARCLIP_CUBE ;
	else if ( !Q_stricmp( "ellipse_z" , tmp_char ) )  ent->s.powerups = FARCLIP_ELLIPSE_Z ;
	else if ( !Q_stricmp( "cylinder_z" , tmp_char ) )  ent->s.powerups = FARCLIP_CYLINDER_Z ;
	else if ( !Q_stricmp( "box_z" , tmp_char ) )  ent->s.powerups = FARCLIP_BOX_Z ;
	else if ( !Q_stricmp( "cone_z" , tmp_char ) )  ent->s.powerups = FARCLIP_CONE_Z ;
	else if ( !Q_stricmp( "pyramid_z" , tmp_char ) )  ent->s.powerups = FARCLIP_PYRAMID_Z ;
	else if ( !Q_stricmp( "circle_infinite_z" , tmp_char ) )  ent->s.powerups = FARCLIP_CIRCLE_INFINITE_Z ;
	else if ( !Q_stricmp( "square_infinite_z" , tmp_char ) )  ent->s.powerups = FARCLIP_SQUARE_INFINITE_Z ;
	else if ( !Q_stricmp( "ellipse_x" , tmp_char ) )  ent->s.powerups = FARCLIP_ELLIPSE_X ;
	else if ( !Q_stricmp( "cylinder_x" , tmp_char ) )  ent->s.powerups = FARCLIP_CYLINDER_X ;
	else if ( !Q_stricmp( "box_x" , tmp_char ) )  ent->s.powerups = FARCLIP_BOX_X ;
	else if ( !Q_stricmp( "cone_x" , tmp_char ) )  ent->s.powerups = FARCLIP_CONE_X ;
	else if ( !Q_stricmp( "pyramid_x" , tmp_char ) )  ent->s.powerups = FARCLIP_PYRAMID_X ;
	else if ( !Q_stricmp( "circle_infinite_x" , tmp_char ) )  ent->s.powerups = FARCLIP_CIRCLE_INFINITE_X ;
	else if ( !Q_stricmp( "square_infinite_x" , tmp_char ) )  ent->s.powerups = FARCLIP_SQUARE_INFINITE_X ;
	else if ( !Q_stricmp( "ellipse_y" , tmp_char ) )  ent->s.powerups = FARCLIP_ELLIPSE_Y ;
	else if ( !Q_stricmp( "cylinder_y" , tmp_char ) )  ent->s.powerups = FARCLIP_CYLINDER_Y ;
	else if ( !Q_stricmp( "box_y" , tmp_char ) )  ent->s.powerups = FARCLIP_BOX_Y ;
	else if ( !Q_stricmp( "cone_y" , tmp_char ) )  ent->s.powerups = FARCLIP_CONE_Y ;
	else if ( !Q_stricmp( "pyramid_y" , tmp_char ) )  ent->s.powerups = FARCLIP_PYRAMID_Y ;
	else if ( !Q_stricmp( "circle_infinite_y" , tmp_char ) )  ent->s.powerups = FARCLIP_CIRCLE_INFINITE_Y ;
	else if ( !Q_stricmp( "square_infinite_y" , tmp_char ) )  ent->s.powerups = FARCLIP_SQUARE_INFINITE_Y ;
	
	
	// get the value for the key "closeclip" and copy it to powerups (7 lower bits), legsAnim and torsoAnim field
	// farclip has priority over closeclip, we only try it if there is no farclip
	
	if ( ent->s.powerups == FARCLIP_NONE )
	{
		G_SpawnString( "closeclip" , "none" , &value ) ;
		sscanf( value , "%31s %i %i" , tmp_char , &ent->s.legsAnim , &ent->s.torsoAnim ) ;
		
		if ( !Q_stricmp( "none" , tmp_char ) )  ent->s.powerups = FARCLIP_NONE ;
		else if ( !Q_stricmp( "always" , tmp_char ) )  ent->s.powerups = FARCLIP_ALWAYS ;
		else if ( !Q_stricmp( "sphere" , tmp_char ) )  ent->s.powerups = FARCLIP_SPHERE ;
		else if ( !Q_stricmp( "cube" , tmp_char ) )  ent->s.powerups = FARCLIP_CUBE ;
		else if ( !Q_stricmp( "ellipse_z" , tmp_char ) )  ent->s.powerups = FARCLIP_ELLIPSE_Z ;
		else if ( !Q_stricmp( "cylinder_z" , tmp_char ) )  ent->s.powerups = FARCLIP_CYLINDER_Z ;
		else if ( !Q_stricmp( "box_z" , tmp_char ) )  ent->s.powerups = FARCLIP_BOX_Z ;
		else if ( !Q_stricmp( "cone_z" , tmp_char ) )  ent->s.powerups = FARCLIP_CONE_Z ;
		else if ( !Q_stricmp( "pyramid_z" , tmp_char ) )  ent->s.powerups = FARCLIP_PYRAMID_Z ;
		else if ( !Q_stricmp( "circle_infinite_z" , tmp_char ) )  ent->s.powerups = FARCLIP_CIRCLE_INFINITE_Z ;
		else if ( !Q_stricmp( "square_infinite_z" , tmp_char ) )  ent->s.powerups = FARCLIP_SQUARE_INFINITE_Z ;
		else if ( !Q_stricmp( "ellipse_x" , tmp_char ) )  ent->s.powerups = FARCLIP_ELLIPSE_X ;
		else if ( !Q_stricmp( "cylinder_x" , tmp_char ) )  ent->s.powerups = FARCLIP_CYLINDER_X ;
		else if ( !Q_stricmp( "box_x" , tmp_char ) )  ent->s.powerups = FARCLIP_BOX_X ;
		else if ( !Q_stricmp( "cone_x" , tmp_char ) )  ent->s.powerups = FARCLIP_CONE_X ;
		else if ( !Q_stricmp( "pyramid_x" , tmp_char ) )  ent->s.powerups = FARCLIP_PYRAMID_X ;
		else if ( !Q_stricmp( "circle_infinite_x" , tmp_char ) )  ent->s.powerups = FARCLIP_CIRCLE_INFINITE_X ;
		else if ( !Q_stricmp( "square_infinite_x" , tmp_char ) )  ent->s.powerups = FARCLIP_SQUARE_INFINITE_X ;
		else if ( !Q_stricmp( "ellipse_y" , tmp_char ) )  ent->s.powerups = FARCLIP_ELLIPSE_Y ;
		else if ( !Q_stricmp( "cylinder_y" , tmp_char ) )  ent->s.powerups = FARCLIP_CYLINDER_Y ;
		else if ( !Q_stricmp( "box_y" , tmp_char ) )  ent->s.powerups = FARCLIP_BOX_Y ;
		else if ( !Q_stricmp( "cone_y" , tmp_char ) )  ent->s.powerups = FARCLIP_CONE_Y ;
		else if ( !Q_stricmp( "pyramid_y" , tmp_char ) )  ent->s.powerups = FARCLIP_PYRAMID_Y ;
		else if ( !Q_stricmp( "circle_infinite_y" , tmp_char ) )  ent->s.powerups = FARCLIP_CIRCLE_INFINITE_Y ;
		else if ( !Q_stricmp( "square_infinite_y" , tmp_char ) )  ent->s.powerups = FARCLIP_SQUARE_INFINITE_Y ;
		
		if ( ent->s.powerups )  ent->s.powerups |= CLOSECLIP_BINARY_MASK ;
		
	}
	
	if ( ent->s.powerups )
	{
		// "legsAnim" has only 8 bits over network, so the far clip value is rounded to multiple of 64
		ent->s.legsAnim = ( ent->s.legsAnim + 32 ) / 64 ;
		if ( ent->s.legsAnim < 1 )  ent->s.legsAnim = 1 ;
		
		// "torsoAnim" has only 8 bits over network, so the far clip value is rounded to multiple of 64
		ent->s.torsoAnim = ( ent->s.torsoAnim + 32 ) / 64 ;
		if ( ent->s.torsoAnim < 1 )  ent->s.torsoAnim = 1 ;
	}
	
	//G_Printf( "^4%s (%i) : %i - %i\n" , tmp_char , ent->s.powerups , ent->s.legsAnim , ent->s.torsoAnim ) ;
	
	
	// get the value for the key "lod" (Level Of Detail) and copy it to upper bits of powerups (bits 9 to 12)
	
	G_SpawnString( "lod" , "none" , &value ) ;
	sscanf( value , "%5s %i" , tmp_char , &tmp_int ) ;
	
	if ( !Q_stricmp( "min" , tmp_char ) )  ent->s.powerups |= MAPLOD_BINARY_MASK | MAPLOD_GTE_BINARY_MASK | ( ( tmp_int & 3 ) << 8 ) ;
	else if ( !Q_stricmp( "max" , tmp_char ) )  ent->s.powerups |= MAPLOD_BINARY_MASK | ( ( tmp_int & 3 ) << 8 ) ;
	
	
	//G_Printf( "^5%s (%i) : %i - %i\n" , tmp_char , ent->s.powerups , ent->s.legsAnim , ent->s.torsoAnim ) ;
	
	trap_LinkEntity( ent ) ;
	
}
#endif


/*
===============================================================================

ROTATING

===============================================================================
*/


/*QUAKED func_rotating (0 .5 .8) ? START_ON - X_AXIS Y_AXIS
You need to have an origin brush as part of this entity.  The center of that brush will be
the point around which it is rotated. It will rotate around the Z axis by default.  You can
check either the X_AXIS or Y_AXIS box to change that.

"model2"	.md3 model to also draw
"speed"		determines how fast it moves; default value is 100.
"dmg"		damage to inflict when blocked (0 default)
"func"		type of func: linear, sin (default is "linear")
"delta"		angle's delta in degree around the origine angle (only if func = sin)
"freq"		frequency of the movement (only if func = sin)
"color"		constantLight color
"light"		constantLight radius
*/

// Joe Kari : New func_rotating allowing the "sin" function (greater flexibility than pendulum)

#ifdef SMOKINGUNS
void SP_func_rotating (gentity_t *ent) {
	float		freq ;
	float		phase ;
	float		delta ;
	float		speed ;
	char            *tmp_char ;
	char            func_char[32] ;
	float		func_val1 , func_val2 ;
	
	G_SpawnFloat( "speed", "30", &speed ) ;
	G_SpawnInt( "dmg", "0", &ent->damage ) ;
	G_SpawnFloat( "phase", "0", &phase ) ;
	G_SpawnString( "func" , "linear" , &tmp_char ) ;
	sscanf( tmp_char , "%31s %f %f" , func_char , &func_val1 , &func_val2 ) ;
	G_SpawnFloat( "delta", "0", &delta ) ;
	G_SpawnFloat( "freq", "1000", &freq ) ;
        
	trap_SetBrushModel( ent, ent->model );
	
	if ( !Q_stricmp( "sin" , func_char ) )
	{
		if ( freq < 0.001 ) { freq = 0.001 ; }
		else if ( freq > 20 ) { freq = 20 ; }
		
		ent->s.pos.trDuration = 1000 / freq ;
		
		InitMover( ent );
		
		VectorCopy( ent->s.origin, ent->s.pos.trBase ) ;
		VectorCopy( ent->s.origin, ent->r.currentOrigin ) ;
		VectorCopy( ent->s.angles, ent->s.apos.trBase ) ;
		
		ent->s.apos.trDuration = 1000 / freq ;
		ent->s.apos.trTime = ent->s.apos.trDuration * phase ;
		ent->s.apos.trType = TR_SINE ;
		
		// set the axis of rotation
		if ( ent->spawnflags & 4 ) {
			ent->s.apos.trDelta[2] = delta ;
		} else if ( ent->spawnflags & 8 ) {
			ent->s.apos.trDelta[0] = delta ;
		} else {
			ent->s.apos.trDelta[1] = delta ;
		}
	}
	else /* if ( !Q_stricmp( "linear" , func_char ) ) */
	{
		// standard func_rotating
		
		if ( !ent->speed ) {
			ent->speed = 100;
		}
		
		InitMover( ent );
		
		VectorCopy( ent->s.origin, ent->s.pos.trBase );
		VectorCopy( ent->s.pos.trBase, ent->r.currentOrigin );
		VectorCopy( ent->s.apos.trBase, ent->r.currentAngles );
		
		// set the axis of rotation
		ent->s.apos.trType = TR_LINEAR;
		if ( ent->spawnflags & 4 ) {
			ent->s.apos.trDelta[2] = ent->speed;
		} else if ( ent->spawnflags & 8 ) {
			ent->s.apos.trDelta[0] = ent->speed;
		} else {
			ent->s.apos.trDelta[1] = ent->speed;
		}
	}
	
	trap_LinkEntity( ent );
}

#else

void SP_func_rotating (gentity_t *ent) {
	if ( !ent->speed ) {
		ent->speed = 100;
	}

	// set the axis of rotation
	ent->s.apos.trType = TR_LINEAR;
	if ( ent->spawnflags & 4 ) {
		ent->s.apos.trDelta[2] = ent->speed;
	} else if ( ent->spawnflags & 8 ) {
		ent->s.apos.trDelta[0] = ent->speed;
	} else {
		ent->s.apos.trDelta[1] = ent->speed;
	}

	if (!ent->damage) {
		ent->damage = 2;
	}

	trap_SetBrushModel( ent, ent->model );
	InitMover( ent );

	VectorCopy( ent->s.origin, ent->s.pos.trBase );
	VectorCopy( ent->s.pos.trBase, ent->r.currentOrigin );
	VectorCopy( ent->s.apos.trBase, ent->r.currentAngles );

	trap_LinkEntity( ent );
}

#endif

/*
===============================================================================

BOBBING

===============================================================================
*/


/*QUAKED func_bobbing (0 .5 .8) ? X_AXIS Y_AXIS
Normally bobs on the Z axis
"model2"	.md3 model to also draw
"height"	amplitude of bob (32 default)
"speed"		seconds to complete a bob cycle (4 default)
"phase"		the 0.0 to 1.0 offset in the cycle to start at
"dmg"		damage to inflict when blocked (2 default)
"color"		constantLight color
"light"		constantLight radius
*/
void SP_func_bobbing (gentity_t *ent) {
	float		height;
	float		phase;

	G_SpawnFloat( "speed", "4", &ent->speed );
	G_SpawnFloat( "height", "32", &height );
	G_SpawnInt( "dmg", "2", &ent->damage );
	G_SpawnFloat( "phase", "0", &phase );

	trap_SetBrushModel( ent, ent->model );
	InitMover( ent );

	VectorCopy( ent->s.origin, ent->s.pos.trBase );
	VectorCopy( ent->s.origin, ent->r.currentOrigin );

	ent->s.pos.trDuration = ent->speed * 1000;
	ent->s.pos.trTime = ent->s.pos.trDuration * phase;
	ent->s.pos.trType = TR_SINE;

	// set the axis of bobbing
	if ( ent->spawnflags & 1 ) {
		ent->s.pos.trDelta[0] = height;
	} else if ( ent->spawnflags & 2 ) {
		ent->s.pos.trDelta[1] = height;
	} else {
		ent->s.pos.trDelta[2] = height;
	}
}

/*
===============================================================================

PENDULUM

===============================================================================
*/


/*QUAKED func_pendulum (0 .5 .8) ?
You need to have an origin brush as part of this entity.
Pendulums always swing north / south on unrotated models.  Add an angles field to the model to allow rotation in other directions.
Pendulum frequency is a physical constant based on the length of the beam and gravity.
"model2"	.md3 model to also draw
"speed"		the number of degrees each way the pendulum swings, (30 default)
"phase"		the 0.0 to 1.0 offset in the cycle to start at
"dmg"		damage to inflict when blocked (2 default)
"color"		constantLight color
"light"		constantLight radius
*/
void SP_func_pendulum(gentity_t *ent) {
	float		freq;
	float		length;
	float		phase;
	float		speed;

	G_SpawnFloat( "speed", "30", &speed );
#ifndef SMOKINGUNS
	G_SpawnInt( "dmg", "2", &ent->damage );
#else
	G_SpawnInt( "dmg", "0", &ent->damage );
#endif
	G_SpawnFloat( "phase", "0", &phase );

	trap_SetBrushModel( ent, ent->model );

	// find pendulum length
	length = fabs( ent->r.mins[2] );
	if ( length < 8 ) {
		length = 8;
	}

	freq = 1 / ( M_PI * 2 ) * sqrt( g_gravity.value / ( 3 * length ) );

	ent->s.pos.trDuration = ( 1000 / freq );

	InitMover( ent );

	VectorCopy( ent->s.origin, ent->s.pos.trBase );
	VectorCopy( ent->s.origin, ent->r.currentOrigin );

	VectorCopy( ent->s.angles, ent->s.apos.trBase );

	ent->s.apos.trDuration = 1000 / freq;
	ent->s.apos.trTime = ent->s.apos.trDuration * phase;
	ent->s.apos.trType = TR_SINE;
	ent->s.apos.trDelta[2] = speed;
}
