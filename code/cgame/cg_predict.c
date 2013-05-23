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
// cg_predict.c -- this file generates cg.predictedPlayerState by either
// interpolating between snapshots from the server or locally predicting
// ahead the client's movement.
// It also handles local physics interaction, like fragments bouncing off walls

#include "cg_local.h"

static	pmove_t		cg_pmove;

static	int			cg_numSolidEntities;
static	centity_t	*cg_solidEntities[MAX_ENTITIES_IN_SNAPSHOT];
static	int			cg_numTriggerEntities;
static	centity_t	*cg_triggerEntities[MAX_ENTITIES_IN_SNAPSHOT];
#ifdef SMOKINGUNS
static	int			cg_numVisibleSolidEntities;
static	centity_t	*cg_VisibleSolidEntities[MAX_ENTITIES_IN_SNAPSHOT];
#endif

/*
====================
CG_BuildSolidList

When a new cg.snap has been set, this function builds a sublist
of the entities that are actually solid, to make for more
efficient collision detection
====================
*/
#ifndef SMOKINGUNS
void CG_BuildSolidList( void ) {
#else
// if light is set to 1, movers will be taken into recognition when being a spectator
void CG_BuildSolidList( qboolean light ) {
#endif
	int			i;
	centity_t	*cent;
	snapshot_t	*snap;
	entityState_t	*ent;

#ifdef SMOKINGUNS
	if (cg_boostfps.integer) {
// FPS Optimization ?
		CG_BuildVisibleSolidList(light);
		return;
// FPS Optimization ?
	}
#endif

	cg_numSolidEntities = 0;
	cg_numTriggerEntities = 0;

	if ( cg.nextSnap && !cg.nextFrameTeleport && !cg.thisFrameTeleport ) {
		snap = cg.nextSnap;
	} else {
		snap = cg.snap;
	}

	for ( i = 0 ; i < snap->numEntities ; i++ ) {
		cent = &cg_entities[ snap->entities[ i ].number ];
		ent = &cent->currentState;

#ifndef SMOKINGUNS
		if ( ent->eType == ET_ITEM || ent->eType == ET_PUSH_TRIGGER || ent->eType == ET_TELEPORT_TRIGGER ) {
#else
		if ( ent->eType == ET_ITEM || ent->eType == ET_PUSH_TRIGGER || ent->eType == ET_TELEPORT_TRIGGER
			|| ent->eType == ET_ESCAPE ||
			// also disable doors and breakables if the player is spectator
			((ent->eType == ET_MOVER || ent->eType == ET_BREAKABLE) &&
			cg.snap->ps.persistant[PERS_TEAM] >= TEAM_SPECTATOR && !light)) {
#endif
			cg_triggerEntities[cg_numTriggerEntities] = cent;
			cg_numTriggerEntities++;
			continue;
		}

		if ( cent->nextState.solid ) {
			cg_solidEntities[cg_numSolidEntities] = cent;
			cg_numSolidEntities++;
			continue;
		}
	}
}

/*
====================
CG_BuildVisibleSolidList
by: hika
2006-12-27

Same as CG_BuildSolidList, except that only visible entities are computed.
====================
*/
// if light is set to 1, movers will be taken into recognition when being a spectator
#ifdef SMOKINGUNS
void CG_BuildVisibleSolidList( qboolean light ) {
	int			i;
	centity_t	*cent;
	snapshot_t	*snap;
	entityState_t	*ent;

	cg_numVisibleSolidEntities = 0;
	cg_numTriggerEntities = 0;

	if ( cg.nextSnap && !cg.nextFrameTeleport && !cg.thisFrameTeleport ) {
		snap = cg.nextSnap;
	} else {
		snap = cg.snap;
	}

	for ( i = 0 ; i < snap->numEntities ; i++ ) {
		cent = &cg_entities[ snap->entities[ i ].number ];
		ent = &cent->currentState;

		if (ent->eType > ET_EVENTS)	// Ignore event entities
			continue;

		if ( ent->eType == ET_ITEM || ent->eType == ET_PUSH_TRIGGER || ent->eType == ET_TELEPORT_TRIGGER
			|| ent->eType == ET_ESCAPE ||
			// also disable doors and breakables if the player is spectator
			((ent->eType == ET_MOVER || ent->eType == ET_BREAKABLE) &&
			cg.snap->ps.persistant[PERS_TEAM] >= TEAM_SPECTATOR && !light)) {
			cg_triggerEntities[cg_numTriggerEntities] = cent;
			cg_numTriggerEntities++;
			continue;
		}

		if ( cent->nextState.solid
		&& CG_IsEntityVisible(cent, 0) ) {
			cg_VisibleSolidEntities[cg_numVisibleSolidEntities] = cent;
			cg_numVisibleSolidEntities++;
		}
	}
}

/*
====================
CG_ClipMoveEntity
by: hika
2007-01-22

Check collision against one solid entity
====================
*/
static void CG_ClipMoveEntity( centity_t *cent, const vec3_t start, const vec3_t mins,
		const vec3_t maxs, const vec3_t end, int mask, trace_t *tr ) {
	int			x, zd, zu;
	entityState_t	*ent;
	clipHandle_t 	cmodel;
	vec3_t		bmins, bmaxs;
	vec3_t		origin, angles;

	ent = &cent->currentState;

	if ( ent->solid == SOLID_BMODEL ) {

		// special value for bmodel
		cmodel = trap_CM_InlineModel( ent->modelindex );
		VectorCopy( cent->lerpAngles, angles );
		BG_EvaluateTrajectory( &cent->currentState.pos, cg.physicsTime, origin );
	} else {

		// encoded bbox
		x = (ent->solid & 255);
		zd = ((ent->solid>>8) & 255);
		zu = ((ent->solid>>16) & 255) - 32;

		bmins[0] = bmins[1] = -x;
		bmaxs[0] = bmaxs[1] = x;
		bmins[2] = -zd;
		bmaxs[2] = zu;

		cmodel = trap_CM_TempBoxModel( bmins, bmaxs );
		VectorCopy( vec3_origin, angles );
		VectorCopy( cent->lerpOrigin, origin );
	}

	trap_CM_TransformedBoxTrace_New( tr, start, end,
		mins, maxs, cmodel, mask, origin, angles);

}

/*
====================
CG_ClipMoveToVisibleEntities
by: hika
2006-12-27

Same as CG_ClipMoveToEntities, except that only visible solids are computed
Ignore all solid entities that are farther than the given "max_distance".
If "max_distance" == 0.0, the parameter is ignored.
====================
*/
static void CG_ClipMoveToVisibleEntities( const vec3_t start, const vec3_t mins, const vec3_t maxs, const vec3_t end,
							int skipNumber, int mask, trace_t *tr, vec_t max_distance ) {
	int			i;
	trace_t		trace;
	entityState_t	*ent;
	centity_t	*cent;

	for ( i = 0 ; i < cg_numVisibleSolidEntities ; i++ ) {
		cent = cg_VisibleSolidEntities[ i ];
		ent = &cent->currentState;

		if ( ent->number == skipNumber ) {
			continue;
		}

		if ( max_distance > 0.0 && cent->distFromCamera > max_distance ) {
			// The solid is too far from the vec3_t "start"
			continue;
		}

		CG_ClipMoveEntity( cent, start, mins, maxs, end, mask, &trace );

		if (trace.allsolid || trace.fraction < tr->fraction) {
			trace.entityNum = ent->number;
			*tr = trace;
		} else if (trace.startsolid) {
			tr->startsolid = qtrue;
		}
		if ( tr->allsolid ) {
			return;
		}
	}
}

/*
====================
CG_ClipMoveToNearEntities
by: hika
2007-01-22

Same as CG_ClipMoveToVisibleEntities.
Only used by CG_Trace_Fragment.
Ignore solid entities if their bounding sphere does not contain "start" and "end".
====================
*/
static void CG_ClipMoveToNearEntities( const vec3_t start, const vec3_t mins, const vec3_t maxs, const vec3_t end,
							int skipNumber, int mask, trace_t *tr ) {
	int			i;
	trace_t		trace;
	entityState_t	*ent;
	centity_t	*cent;

	for ( i = 0 ; i < cg_numVisibleSolidEntities ; i++ ) {
		cent = cg_VisibleSolidEntities[ i ];
		ent = &cent->currentState;

		if ( ent->number == skipNumber ) {
			continue;
		}

		if (Distance( cent->centerOrigin, start ) > cent->radius
		&& Distance( cent->centerOrigin, end ) > cent->radius) {
		// "start" and "end" are not in the entity's bounding sphere
			continue;
		}

		CG_ClipMoveEntity( cent, start, mins, maxs, end, mask, &trace );

		if (trace.allsolid || trace.fraction < tr->fraction) {
			trace.entityNum = ent->number;
			*tr = trace;
		} else if (trace.startsolid) {
			tr->startsolid = qtrue;
		}
		if ( tr->allsolid ) {
			return;
		}
	}
}
#endif

/*
====================
CG_ClipMoveToEntities

====================
*/
static void CG_ClipMoveToEntities ( const vec3_t start, const vec3_t mins, const vec3_t maxs, const vec3_t end,
							int skipNumber, int mask, trace_t *tr ) {
	int			i, x, zd, zu;
	trace_t		trace;
	entityState_t	*ent;
	clipHandle_t 	cmodel;
	vec3_t		bmins, bmaxs;
	vec3_t		origin, angles;
	centity_t	*cent;


#ifdef SMOKINGUNS
	if (cg_boostfps.integer) {
// FPS Optimization ?
		CG_ClipMoveToVisibleEntities(start, mins, maxs, end, skipNumber, mask, tr,	0.0);
		return;
// FPS Optimization ?
	}
#endif

	for ( i = 0 ; i < cg_numSolidEntities ; i++ ) {
		cent = cg_solidEntities[ i ];
		ent = &cent->currentState;

		if ( ent->number == skipNumber ) {
			continue;
		}

		if ( ent->solid == SOLID_BMODEL ) {
			// special value for bmodel
			cmodel = trap_CM_InlineModel( ent->modelindex );
			VectorCopy( cent->lerpAngles, angles );
			BG_EvaluateTrajectory( &cent->currentState.pos, cg.physicsTime, origin );
		} else {
			// encoded bbox
			x = (ent->solid & 255);
			zd = ((ent->solid>>8) & 255);
			zu = ((ent->solid>>16) & 255) - 32;

			bmins[0] = bmins[1] = -x;
			bmaxs[0] = bmaxs[1] = x;
			bmins[2] = -zd;
			bmaxs[2] = zu;

			cmodel = trap_CM_TempBoxModel( bmins, bmaxs );
			VectorCopy( vec3_origin, angles );
			VectorCopy( cent->lerpOrigin, origin );
		}


#ifndef SMOKINGUNS
		trap_CM_TransformedBoxTrace ( &trace, start, end,
#else
		trap_CM_TransformedBoxTrace_New ( &trace, start, end,
#endif
			mins, maxs, cmodel,  mask, origin, angles);

		if (trace.allsolid || trace.fraction < tr->fraction) {
			trace.entityNum = ent->number;
			*tr = trace;
		} else if (trace.startsolid) {
			tr->startsolid = qtrue;
		}
		if ( tr->allsolid ) {
			return;
		}
	}
}


/*
================
CG_Trace_New
================
*/
#ifdef SMOKINGUNS
int	CG_Trace_New( trace_t *result, const vec3_t start, const vec3_t mins, const vec3_t maxs, const vec3_t end,
					 int skipNumber, int mask ) {
	trace_t	t;
	int	shaderNum;

	shaderNum = trap_CM_BoxTrace_New ( &t, start, end, mins, maxs, 0, mask);
	t.entityNum = t.fraction != 1.0 ? ENTITYNUM_WORLD : ENTITYNUM_NONE;
	// check all other solid models
	CG_ClipMoveToEntities (start, mins, maxs, end, skipNumber, mask, &t);

	*result = t;

	return shaderNum;
}
#endif

/*
================
CG_Trace
================
*/
void	CG_Trace( trace_t *result, const vec3_t start, const vec3_t mins, const vec3_t maxs, const vec3_t end,
					 int skipNumber, int mask ) {
	trace_t	t;

#ifndef SMOKINGUNS
	trap_CM_BoxTrace ( &t, start, end, mins, maxs, 0, mask);
#else
	trap_CM_BoxTrace_New ( &t, start, end, mins, maxs, 0, mask);
#endif
	t.entityNum = t.fraction != 1.0 ? ENTITYNUM_WORLD : ENTITYNUM_NONE;
	// check all other solid models
	CG_ClipMoveToEntities (start, mins, maxs, end, skipNumber, mask, &t);

	*result = t;
}

/*
================
CG_Trace_Visible
by: hika
2006-12-27

Same as CG_Trace_New, except that only visible/solid entities are traced.
Ignore all solid entities that are farther than the given "max_distance".
If "max_distance" == 0, the parameter is ignored
================
*/
#ifdef SMOKINGUNS
int CG_Trace_Visible( trace_t *result, const vec3_t start, const vec3_t mins, const vec3_t maxs, const vec3_t end,
					 int skipNumber, int mask, vec_t max_distance ) {
	trace_t	t;
	int	shaderNum;

	shaderNum = trap_CM_BoxTrace_New( &t, start, end, mins, maxs, 0, mask);
	t.entityNum = t.fraction != 1.0 ? ENTITYNUM_WORLD : ENTITYNUM_NONE;

	// check all other visible solid models
	CG_ClipMoveToVisibleEntities(start, mins, maxs, end, skipNumber, mask, &t, max_distance);

	*result = t;

	return shaderNum;
}

/*
================
CG_Trace_Flare
by: hika
2007-01-12

Same as CG_Trace_Visible.
If the first BoxTrace hit something solid in the world,
don't trace to any solid entities.
================
*/
int CG_Trace_Flare( trace_t *result, const vec3_t start, const vec3_t mins, const vec3_t maxs, const vec3_t end,
					 int skipNumber, int mask, vec_t max_distance ) {
	trace_t	t;
	int	shaderNum;

	shaderNum = trap_CM_BoxTrace_New( &t, start, end, mins, maxs, 0, mask);
	t.entityNum = t.fraction != 1.0 ? ENTITYNUM_WORLD : ENTITYNUM_NONE;

	if (t.fraction == 1.0) { // Means that we do not hit anything
		// check all other visible solid models
		CG_ClipMoveToVisibleEntities(start, mins, maxs, end, skipNumber, mask, &t, max_distance );
	}

	*result = t;

	return shaderNum;
}

/*
================
CG_Trace_Fragment
by: hika
2007-01-22

Same as CG_Trace_Visible.
If vec3_t "start" and vec3_t "end" are both outside a solid entity bounding sphere,
then ignore this entity.

We can do this kind of test, because "start" and "end" are (should be ?) very closed,
when this function is called.
================
*/
int CG_Trace_Fragment( trace_t *result, const vec3_t start, const vec3_t mins, const vec3_t maxs, const vec3_t end,
					 int skipNumber, int mask ) {
	trace_t	t;
	int	shaderNum;

	shaderNum = trap_CM_BoxTrace_New( &t, start, end, mins, maxs, 0, mask);
	t.entityNum = t.fraction != 1.0 ? ENTITYNUM_WORLD : ENTITYNUM_NONE;

	// check all other visible and very closed solid models
	CG_ClipMoveToNearEntities(start, mins, maxs, end, skipNumber, mask, &t);

	*result = t;

	return shaderNum;
}

/*
================
CG_PointContentsVisible
by: hika
2007-01-01

Same as CG_PointContents, except that only visible/solid entities are checked
================
*/
int		CG_PointContentsVisible( const vec3_t point, int passEntityNum ) {
	int			i;
	entityState_t	*ent;
	centity_t	*cent;
	clipHandle_t cmodel;
	int			contents;

	contents = trap_CM_PointContents (point, 0);

	for ( i = 0 ; i < cg_numVisibleSolidEntities ; i++ ) {
		cent = cg_VisibleSolidEntities[ i ];

		ent = &cent->currentState;

		if ( ent->number == passEntityNum ) {
			continue;
		}

		if (ent->solid != SOLID_BMODEL) { // special value for bmodel
			continue;
		}

		cmodel = trap_CM_InlineModel( ent->modelindex );
		if ( !cmodel ) {
			continue;
		}

		contents |= trap_CM_TransformedPointContents( point, cmodel, ent->origin, ent->angles );
	}

	return contents;
}
#endif

/*
================
CG_PointContents
================
*/
int		CG_PointContents( const vec3_t point, int passEntityNum ) {
	int			i;
	entityState_t	*ent;
	centity_t	*cent;
	clipHandle_t cmodel;
	int			contents;

#ifdef SMOKINGUNS
	if (cg_boostfps.integer) {
// FPS Optimization ?
		return CG_PointContentsVisible(point, passEntityNum);
// FPS Optimization ?
	}
#endif

	contents = trap_CM_PointContents (point, 0);

	for ( i = 0 ; i < cg_numSolidEntities ; i++ ) {
		cent = cg_solidEntities[ i ];

		ent = &cent->currentState;

		if ( ent->number == passEntityNum ) {
			continue;
		}

		if (ent->solid != SOLID_BMODEL) { // special value for bmodel
			continue;
		}

		cmodel = trap_CM_InlineModel( ent->modelindex );
		if ( !cmodel ) {
			continue;
		}

		contents |= trap_CM_TransformedPointContents( point, cmodel, cent->lerpOrigin, cent->lerpAngles );
	}

	return contents;
}


/*
========================
CG_InterpolatePlayerState

Generates cg.predictedPlayerState by interpolating between
cg.snap->player_state and cg.nextFrame->player_state
========================
*/
static void CG_InterpolatePlayerState( qboolean grabAngles ) {
	float			f;
	int				i;
	playerState_t	*out;
	snapshot_t		*prev, *next;

	out = &cg.predictedPlayerState;
	prev = cg.snap;
	next = cg.nextSnap;

	*out = cg.snap->ps;

	// if we are still allowing local input, short circuit the view angles
	if ( grabAngles ) {
		usercmd_t	cmd;
		int			cmdNum;

		cmdNum = trap_GetCurrentCmdNumber();
		trap_GetUserCmd( cmdNum, &cmd );

		PM_UpdateViewAngles( out, &cmd );
	}

	// if the next frame is a teleport, we can't lerp to it
	if ( cg.nextFrameTeleport ) {
		return;
	}

	if ( !next || next->serverTime <= prev->serverTime ) {
		return;
	}

	f = (float)( cg.time - prev->serverTime ) / ( next->serverTime - prev->serverTime );

	i = next->ps.bobCycle;
	if ( i < prev->ps.bobCycle ) {
		i += 256;		// handle wraparound
	}
	out->bobCycle = prev->ps.bobCycle + f * ( i - prev->ps.bobCycle );

	for ( i = 0 ; i < 3 ; i++ ) {
		out->origin[i] = prev->ps.origin[i] + f * (next->ps.origin[i] - prev->ps.origin[i] );
		if ( !grabAngles ) {
			out->viewangles[i] = LerpAngle(
				prev->ps.viewangles[i], next->ps.viewangles[i], f );
		}
		out->velocity[i] = prev->ps.velocity[i] +
			f * (next->ps.velocity[i] - prev->ps.velocity[i] );
	}

}

/*
===================
CG_TouchItem
===================
*/
static void CG_TouchItem( centity_t *cent ) {
	gitem_t		*item;

	if ( !cg_predictItems.integer ) {
		return;
	}
	if ( !BG_PlayerTouchesItem( &cg.predictedPlayerState, &cent->currentState, cg.time ) ) {
		return;
	}

	// never pick an item up twice in a prediction
	if ( cent->miscTime == cg.time ) {
		return;
	}

	if ( !BG_CanItemBeGrabbed( cgs.gametype, &cent->currentState, &cg.predictedPlayerState ) ) {
		return;		// can't hold it
	}

	item = &bg_itemlist[ cent->currentState.modelindex ];

#ifndef SMOKINGUNS
	// Special case for flags.  
	// We don't predict touching our own flag
#ifdef MISSIONPACK
	if( cgs.gametype == GT_1FCTF ) {
		if( item->giType == IT_TEAM && item->giTag != PW_NEUTRALFLAG ) {
			return;
		}
	}
#endif
	if( cgs.gametype == GT_CTF ) {
		if (cg.predictedPlayerState.persistant[PERS_TEAM] == TEAM_RED &&
			item->giType == IT_TEAM && item->giTag == PW_REDFLAG)
			return;
		if (cg.predictedPlayerState.persistant[PERS_TEAM] == TEAM_BLUE &&
			item->giType == IT_TEAM && item->giTag == PW_BLUEFLAG)
			return;
	}
#else
	if(item->giType == IT_POWERUP && item->giTag == PW_GOLD){
		float dist1, dist2;
		vec3_t		muzzle;
		trace_t		tr;

		VectorCopy( cg.predictedPlayerState.origin, muzzle );
		muzzle[2] = cent->lerpOrigin[2];

		//find the nearest obstacle
		CG_Trace_New (&tr, muzzle, NULL, NULL, cent->lerpOrigin, cg.predictedPlayerState.clientNum, MASK_SHOT);

		dist1 = Distance(muzzle, tr.endpos);
		dist2 = Distance(muzzle, cent->lerpOrigin);

		if(dist2>dist1)
			return;
	}
#endif

	// grab it
#ifndef SMOKINGUNS
	BG_AddPredictableEventToPlayerstate( EV_ITEM_PICKUP, cent->currentState.modelindex , &cg.predictedPlayerState);
#else
	if(Q_stricmp(item->classname, "pickup_money")){
		BG_AddPredictableEventToPlayerstate( EV_ITEM_PICKUP, cent->currentState.modelindex , &cg.predictedPlayerState);
	} else {
		BG_AddPredictableEventToPlayerstate( EV_MONEY_PICKUP, cent->currentState.time2 , &cg.predictedPlayerState);
	}
#endif

	// remove it from the frame so it won't be drawn
	cent->currentState.eFlags |= EF_NODRAW;

	// don't touch it again this prediction
	cent->miscTime = cg.time;

	// if it's a weapon, give them some predicted ammo so the autoswitch will work
	if ( item->giType == IT_WEAPON ) {
		cg.predictedPlayerState.stats[ STAT_WEAPONS ] |= 1 << item->giTag;
		if ( !cg.predictedPlayerState.ammo[ item->giTag ] ) {
			cg.predictedPlayerState.ammo[ item->giTag ] = 1;
		}
	}
}


/*
=========================
CG_TouchTriggerPrediction

Predict push triggers and items
=========================
*/
static void CG_TouchTriggerPrediction( void ) {
	int			i;
	trace_t		trace;
	entityState_t	*ent;
	clipHandle_t cmodel;
	centity_t	*cent;
	qboolean	spectator;

	// dead clients don't activate triggers
	if ( cg.predictedPlayerState.stats[STAT_HEALTH] <= 0 ) {
		return;
	}

	spectator = ( cg.predictedPlayerState.pm_type == PM_SPECTATOR );

	if ( cg.predictedPlayerState.pm_type != PM_NORMAL && !spectator ) {
		return;
	}

	for ( i = 0 ; i < cg_numTriggerEntities ; i++ ) {
		cent = cg_triggerEntities[ i ];
		ent = &cent->currentState;

		if ( ent->eType == ET_ITEM && !spectator ) {
			CG_TouchItem( cent );
			continue;
		}

		if ( ent->solid != SOLID_BMODEL ) {
			continue;
		}

		cmodel = trap_CM_InlineModel( ent->modelindex );
		if ( !cmodel ) {
			continue;
		}

#ifndef SMOKINGUNS
		trap_CM_BoxTrace( &trace, cg.predictedPlayerState.origin, cg.predictedPlayerState.origin, 
#else
		trap_CM_BoxTrace_New( &trace, cg.predictedPlayerState.origin, cg.predictedPlayerState.origin,
#endif
			cg_pmove.mins, cg_pmove.maxs, cmodel, -1 );

		if ( !trace.startsolid ) {
			continue;
		}

#ifndef SMOKINGUNS
		if ( ent->eType == ET_TELEPORT_TRIGGER ) {
			cg.hyperspace = qtrue;
		} else if ( ent->eType == ET_PUSH_TRIGGER ) {
			BG_TouchJumpPad( &cg.predictedPlayerState, ent );
		}
#endif
	}

	// if we didn't touch a jump pad this pmove frame
#ifndef SMOKINGUNS
	if ( cg.predictedPlayerState.jumppad_frame != cg.predictedPlayerState.pmove_framecount ) {
		cg.predictedPlayerState.jumppad_frame = 0;
		cg.predictedPlayerState.jumppad_ent = 0;
	}
#endif
}

#ifdef SMOKINGUNS
//unlagged - optimized prediction
#define ABS(x) ((x) < 0 ? (-(x)) : (x))

static int IsUnacceptableError( playerState_t *ps, playerState_t *pps ) {
	vec3_t delta;
	int i;

	if ( pps->pm_type != ps->pm_type ||
			pps->pm_flags != ps->pm_flags ||
			pps->pm_time != ps->pm_time ) {
		return 1;
	}

	VectorSubtract( pps->origin, ps->origin, delta );
	if ( VectorLengthSquared( delta ) > 0.1f * 0.1f ) {
		if ( cg_showmiss.integer ) {
			CG_Printf("delta: %.2f  ", VectorLength(delta) );
		}
		return 2;
	}

	VectorSubtract( pps->velocity, ps->velocity, delta );
	if ( VectorLengthSquared( delta ) > 0.1f * 0.1f ) {
		if ( cg_showmiss.integer ) {
			CG_Printf("delta: %.2f  ", VectorLength(delta) );
		}
		return 3;
	}

	if ( pps->weaponTime != ps->weaponTime ||
			pps->gravity != ps->gravity ||
			pps->speed != ps->speed ||
			pps->delta_angles[0] != ps->delta_angles[0] ||
			pps->delta_angles[1] != ps->delta_angles[1] ||
			pps->delta_angles[2] != ps->delta_angles[2] ||
			pps->groundEntityNum != ps->groundEntityNum ) {
		return 4;
	}

	if ( pps->legsTimer != ps->legsTimer ||
			pps->legsAnim != ps->legsAnim ||
			pps->torsoTimer != ps->torsoTimer ||
			pps->torsoAnim != ps->torsoAnim ||
			pps->movementDir != ps->movementDir ) {
		return 5;
	}

	VectorSubtract( pps->grapplePoint, ps->grapplePoint, delta );
	if ( VectorLengthSquared( delta ) > 0.1f * 0.1f ) {
		return 6;
	}

	if ( pps->eFlags != ps->eFlags ) {
		return 7;
	}

	if ( pps->eventSequence != ps->eventSequence ) {
		return 8;
	}

	for ( i = 0; i < MAX_PS_EVENTS; i++ ) {
		if ( pps->events[i] != ps->events[i] ||
				pps->eventParms[i] != ps->eventParms[i] ) {
			return 9;
		}
	}

	if ( pps->externalEvent != ps->externalEvent ||
			pps->externalEventParm != ps->externalEventParm ||
			pps->externalEventTime != ps->externalEventTime ) {
		return 10;
	}

	if ( pps->clientNum != ps->clientNum ||
			pps->weapon != ps->weapon ||
			pps->weaponstate != ps->weaponstate ) {
		return 11;
	}

	if ( ABS(pps->viewangles[0] - ps->viewangles[0]) > 1.0f ||
			ABS(pps->viewangles[1] - ps->viewangles[1]) > 1.0f ||
			ABS(pps->viewangles[2] - ps->viewangles[2]) > 1.0f ) {
		return 12;
	}

	if ( pps->viewheight != ps->viewheight ) {
		return 13;
	}

	if ( pps->damageEvent != ps->damageEvent ||
			pps->damageYaw != ps->damageYaw ||
			pps->damagePitch != ps->damagePitch ||
			pps->damageCount != ps->damageCount ) {
		return 14;
	}

	for ( i = 0; i < MAX_STATS; i++ ) {
		if ( pps->stats[i] != ps->stats[i] ) {
			// Tequila comment:
			// IDLE_TIMER is only used to handle WP_ANIM_IDLE, so that's acceptable
			if ( i != IDLE_TIMER )
				return 15;
		}
	}

	for ( i = 0; i < MAX_PERSISTANT; i++ ) {
		if ( pps->persistant[i] != ps->persistant[i] ) {
			return 16;
		}
	}

	for ( i = 0; i < MAX_POWERUPS; i++ ) {
		if ( pps->powerups[i] != ps->powerups[i] ) {
			return 17;
		}
	}

	for ( i = 0; i < MAX_WEAPONS; i++ ) {
		if ( pps->ammo[i] != ps->ammo[i] ) {
			return 18;
		}
	}
/*
	if ( pps->generic1 != ps->generic1 ||
			pps->loopSound != ps->loopSound ||
			pps->jumppad_ent != ps->jumppad_ent ) {
		return 19;
	}
*/
	return 0;
}
//unlagged - optimized prediction
#endif

/*
=================
CG_PredictPlayerState

Generates cg.predictedPlayerState for the current cg.time
cg.predictedPlayerState is guaranteed to be valid after exiting.

For demo playback, this will be an interpolation between two valid
playerState_t.

For normal gameplay, it will be the result of predicted usercmd_t on
top of the most recent playerState_t received from the server.

Each new snapshot will usually have one or more new usercmd over the last,
but we simulate all unacknowledged commands each time, not just the new ones.
This means that on an internet connection, quite a few pmoves may be issued
each frame.

OPTIMIZE: don't re-simulate unless the newly arrived snapshot playerState_t
differs from the predicted one.  Would require saving all intermediate
playerState_t during prediction.

We detect prediction errors and allow them to be decayed off over several frames
to ease the jerk.
=================
*/
void CG_PredictPlayerState( void ) {
	int			cmdNum, current;
	playerState_t	oldPlayerState;
	qboolean	moved;
	usercmd_t	oldestCmd;
	usercmd_t	latestCmd;

#ifdef SMOKINGUNS
//unlagged - optimized prediction
	int stateIndex = 0, predictCmd;
	int numPredicted = 0, numPlayedBack = 0; // debug code
//unlagged - optimized prediction
#endif

	cg.hyperspace = qfalse;	// will be set if touching a trigger_teleport

	// if this is the first frame we must guarantee
	// predictedPlayerState is valid even if there is some
	// other error condition
	if ( !cg.validPPS ) {
		cg.validPPS = qtrue;
		cg.predictedPlayerState = cg.snap->ps;
	}


	// demo playback just copies the moves
#ifndef SMOKINGUNS
	if ( cg.demoPlayback || (cg.snap->ps.pm_flags & PMF_FOLLOW) ) {
#else
	if ( cg.demoPlayback || (cg.snap->ps.pm_flags & PMF_SUICIDE)
	            || ((cg.snap->ps.pm_flags & PMF_FOLLOW)  && !(cg.snap->ps.pm_type == PM_CHASECAM)) ) {
#endif
		CG_InterpolatePlayerState( qfalse );
		return;
	}

	// non-predicting local movement will grab the latest angles
	if ( cg_nopredict.integer || cg_synchronousClients.integer ) {
		CG_InterpolatePlayerState( qtrue );
		return;
	}

	// prepare for pmove
	cg_pmove.ps = &cg.predictedPlayerState;
	cg_pmove.trace = CG_Trace;
	cg_pmove.pointcontents = CG_PointContents;
	if ( cg_pmove.ps->pm_type == PM_DEAD ) {
		cg_pmove.tracemask = MASK_PLAYERSOLID & ~CONTENTS_BODY;
	}
	else {
		cg_pmove.tracemask = MASK_PLAYERSOLID;
	}
#ifndef SMOKINGUNS
	if ( cg.snap->ps.persistant[PERS_TEAM] == TEAM_SPECTATOR ) {
#else
	if ( cg.snap->ps.persistant[PERS_TEAM] >= TEAM_SPECTATOR ) {
#endif
		cg_pmove.tracemask &= ~CONTENTS_BODY;	// spectators can fly through bodies
	}
	cg_pmove.noFootsteps = ( cgs.dmflags & DF_NO_FOOTSTEPS ) > 0;

	// save the state before the pmove so we can detect transitions
	oldPlayerState = cg.predictedPlayerState;

	predictCmd = current = trap_GetCurrentCmdNumber();

	// if we don't have the commands right after the snapshot, we
	// can't accurately predict a current position, so just freeze at
	// the last good position we had
	cmdNum = current - CMD_BACKUP + 1;
	trap_GetUserCmd( cmdNum, &oldestCmd );
	if ( oldestCmd.serverTime > cg.snap->ps.commandTime
		&& oldestCmd.serverTime < cg.time ) {	// special check for map_restart
		if ( cg_showmiss.integer ) {
			CG_Printf ("exceeded PACKET_BACKUP on commands\n");
		}
		return;
	}

	// get the latest command so we can know which commands are from previous map_restarts
	trap_GetUserCmd( current, &latestCmd );

	// get the most recent information we have, even if
	// the server time is beyond our current cg.time,
	// because predicted player positions are going to
	// be ahead of everything else anyway
	if ( cg.nextSnap && !cg.nextFrameTeleport && !cg.thisFrameTeleport ) {
		cg.predictedPlayerState = cg.nextSnap->ps;
		cg.physicsTime = cg.nextSnap->serverTime;
	} else {
		cg.predictedPlayerState = cg.snap->ps;
		cg.physicsTime = cg.snap->serverTime;
	}

	if ( pmove_msec.integer < 8 ) {
		trap_Cvar_Set("pmove_msec", "8");
	}
	else if (pmove_msec.integer > 33) {
		trap_Cvar_Set("pmove_msec", "33");
	}

	cg_pmove.pmove_fixed = pmove_fixed.integer;// | cg_pmove_fixed.integer;
	cg_pmove.pmove_msec = pmove_msec.integer;

#ifdef SMOKINGUNS
//unlagged - optimized prediction
	// Like the comments described above, a player's state is entirely
	// re-predicted from the last valid snapshot every client frame, which
	// can be really, really, really slow.  Every old command has to be
	// run again.  For every client frame that is *not* directly after a
	// snapshot, this is unnecessary, since we have no new information.
	// For those, we'll play back the predictions from the last frame and
	// predict only the newest commands.  Essentially, we'll be doing
	// an incremental predict instead of a full predict.
	//
	// If we have a new snapshot, we can compare its player state's command
	// time to the command times in the queue to find a match.  If we find
	// a matching state, and the predicted version has not deviated, we can
	// use the predicted state as a base - and also do an incremental predict.
	//
	// With this method, we get incremental predicts on every client frame
	// except a frame following a new snapshot in which there was a prediction
	// error.  This yeilds anywhere from a 15% to 40% performance increase,
	// depending on how much of a bottleneck the CPU is.

	// we check for cg_latentCmds because it'll mess up the optimization
	// FIXME: make cg_latentCmds work with cg_optimizePrediction?
	if ( cg_optimizePrediction.integer && !cg_latentCmds.integer ) {
		if ( cg.nextFrameTeleport || cg.thisFrameTeleport ) {
			// do a full predict
			cg.lastPredictedCommand = 0;
			cg.stateTail = cg.stateHead;
			predictCmd = current - CMD_BACKUP + 1;
		}
		// cg.physicsTime is the current snapshot's serverTime
		// if it's the same as the last one
		else if ( cg.physicsTime == cg.lastServerTime ) {
			// we have no new information, so do an incremental predict
			predictCmd = cg.lastPredictedCommand + 1;
		}
		else {
			// we have a new snapshot

			int i;
			qboolean error = qtrue;

			// loop through the saved states queue
			for ( i = cg.stateHead; i != cg.stateTail; i = (i + 1) % NUM_SAVED_STATES ) {
				// if we find a predicted state whose commandTime matches the snapshot player state's commandTime
				if ( cg.savedPmoveStates[i].commandTime == cg.predictedPlayerState.commandTime ) {
					// make sure the state differences are acceptable
					int errorcode = IsUnacceptableError( &cg.predictedPlayerState, &cg.savedPmoveStates[i] );

					// too much change?
					if ( errorcode ) {
						if ( cg_showmiss.integer ) {
							CG_Printf("errorcode %d at %d\n", errorcode, cg.time);
						}
						// yeah, so do a full predict
						break;
					}

					// this one is almost exact, so we'll copy it in as the starting point
					*cg_pmove.ps = cg.savedPmoveStates[i];
					// advance the head
					cg.stateHead = (i + 1) % NUM_SAVED_STATES;

					// set the next command to predict
					predictCmd = cg.lastPredictedCommand + 1;

					// a saved state matched, so flag it
					error = qfalse;
					break;
				}
			}

			// if no saved states matched
			if ( error ) {
				// do a full predict
				cg.lastPredictedCommand = 0;
				cg.stateTail = cg.stateHead;
				predictCmd = current - CMD_BACKUP + 1;
			}
		}

		// keep track of the server time of the last snapshot so we
		// know when we're starting from a new one in future calls
		cg.lastServerTime = cg.physicsTime;
		stateIndex = cg.stateHead;
	}
//unlagged - optimized prediction
#endif

	// run cmds
	moved = qfalse;
	for ( cmdNum = current - CMD_BACKUP + 1 ; cmdNum <= current ; cmdNum++ ) {

#ifdef SMOKINGUNS
		cg_pmove.ps->oldbuttons = cg_pmove.cmd.buttons;
#endif

		// get the command
		trap_GetUserCmd( cmdNum, &cg_pmove.cmd );

		if ( cg_pmove.pmove_fixed ) {
			PM_UpdateViewAngles( cg_pmove.ps, &cg_pmove.cmd );
		}

		// don't do anything if the time is before the snapshot player time
		if ( cg_pmove.cmd.serverTime <= cg.predictedPlayerState.commandTime ) {
			continue;
		}

		// don't do anything if the command was from a previous map_restart
		if ( cg_pmove.cmd.serverTime > latestCmd.serverTime ) {
			continue;
		}

		// check for a prediction error from last frame
		// on a lan, this will often be the exact value
		// from the snapshot, but on a wan we will have
		// to predict several commands to get to the point
		// we want to compare
		if ( cg.predictedPlayerState.commandTime == oldPlayerState.commandTime ) {
			vec3_t	delta;
			float	len;

			if ( cg.thisFrameTeleport ) {
				// a teleport will not cause an error decay
				VectorClear( cg.predictedError );
				if ( cg_showmiss.integer ) {
					CG_Printf( "PredictionTeleport\n" );
				}
				cg.thisFrameTeleport = qfalse;
			} else {
				vec3_t adjusted, new_angles;
				CG_AdjustPositionForMover( cg.predictedPlayerState.origin, 
				cg.predictedPlayerState.groundEntityNum, cg.physicsTime, cg.oldTime, adjusted, cg.predictedPlayerState.viewangles, new_angles);

#ifdef SMOKINGUNS
				// If ground entity has changed and cg_boostfps is set, be sure it is in visible entity list
				if ( cg_boostfps.integer && oldPlayerState.groundEntityNum != cg.predictedPlayerState.groundEntityNum ) {
					centity_t *cent = &cg_entities[ cg.predictedPlayerState.groundEntityNum ];
					if (!cent->visible) {
						cg_VisibleSolidEntities[cg_numVisibleSolidEntities] = cent;
						cg_numVisibleSolidEntities++;
					}
				}
#endif

				if ( cg_showmiss.integer ) {
					if (!VectorCompare( oldPlayerState.origin, adjusted )) {
						CG_Printf("prediction error\n");
					}
				}
				VectorSubtract( oldPlayerState.origin, adjusted, delta );
				len = VectorLength( delta );
				if ( len > 0.1 ) {
					if ( cg_showmiss.integer ) {
						CG_Printf("Prediction miss: %f\n", len);
					}
					if ( cg_errorDecay.integer ) {
						int		t;
						float	f;

						t = cg.time - cg.predictedErrorTime;
						f = ( cg_errorDecay.value - t ) / cg_errorDecay.value;
						if ( f < 0 ) {
							f = 0;
						}
						if ( f > 0 && cg_showmiss.integer ) {
							CG_Printf("Double prediction decay: %f\n", f);
						}
						VectorScale( cg.predictedError, f, cg.predictedError );
					} else {
						VectorClear( cg.predictedError );
					}
					VectorAdd( delta, cg.predictedError, cg.predictedError );
					cg.predictedErrorTime = cg.oldTime;
				}
			}
		}

		// don't predict gauntlet firing, which is only supposed to happen
		// when it actually inflicts damage
		cg_pmove.gauntletHit = qfalse;

		if ( cg_pmove.pmove_fixed ) {
			cg_pmove.cmd.serverTime = ((cg_pmove.cmd.serverTime + pmove_msec.integer-1) / pmove_msec.integer) * pmove_msec.integer;
		}

#ifdef SMOKINGUNS
		if(cg.introend >= cg.time && cgs.gametype == GT_DUEL &&
			cg.predictedPlayerState.persistant[PERS_TEAM] == TEAM_FREE){
			// delete all movement and cmd stats
			cg_pmove.ps->speed = 0;
			cg_pmove.cmd.forwardmove = 0;
			cg_pmove.cmd.rightmove = 0;
			cg_pmove.cmd.upmove = 0;
			cg_pmove.cmd.weapon = WP_NONE;
			cg_pmove.cmd.buttons = 0;
			cg_pmove.ps->weapon = WP_NONE;
			cg_pmove.ps->weapon2 = WP_NONE;
			cg_pmove.xyspeed = 0;
			cg.weaponSelect = WP_NONE;
			cg.markedweapon = 0;
			cg_pmove.ps->stats[STAT_OLDWEAPON] = 0;
		}
		else if ( ( cgs.gametype >= GT_RTP ) && ( cg.time < cg.roundstarttime + cgs.roundNoMoveTime ) 
			&& (cg.predictedPlayerState.persistant[PERS_TEAM] < TEAM_SPECTATOR ) )  {
			// added by Joe Kari: delete all movement and cmd stats until the end of the countdown in RTP and BR gametype
			cg_pmove.ps->speed = 0;
			cg_pmove.cmd.forwardmove = 0;
			cg_pmove.cmd.rightmove = 0;
			cg_pmove.cmd.upmove = 0;
			cg_pmove.cmd.buttons = 0;
			cg_pmove.xyspeed = 0;
		}

		// mouseangles can't be moved during camera move
		if(cg.introend - DU_INTRO_DRAW >= cg.time &&
			cg.predictedPlayerState.persistant[PERS_TEAM] == TEAM_FREE) {
			cg_pmove.ps->stats[STAT_FLAGS] |= SF_DU_INTRO;
		} else {
			cg_pmove.ps->stats[STAT_FLAGS] &= ~SF_DU_INTRO;
		}

		// if player is able to change to the spectators in duel mode remove the attack button
		if(cgs.gametype == GT_DUEL && cg_pmove.ps->stats[STAT_FLAGS] & SF_DU_WON)
			cg_pmove.cmd.buttons &= ~BUTTON_ATTACK;

		if(cg_pmove.ps->stats[STAT_GATLING_MODE]){
			cg_pmove.ps->speed = 0;
			cg_pmove.cmd.forwardmove = 0;
			cg_pmove.cmd.rightmove = 0;
			cg_pmove.cmd.upmove = 0;
		}

		// needs to be set to start the idle animation
		if(cg.predictedPlayerState.persistant[PERS_SPAWN_COUNT] != oldPlayerState.persistant[PERS_SPAWN_COUNT]){
			cg_pmove.ps->pm_flags |= PMF_RESPAWNED;
		}
#endif

#ifndef SMOKINGUNS
		Pmove (&cg_pmove);
#else
//unlagged - optimized prediction
		// we check for cg_latentCmds because it'll mess up the optimization
		if ( cg_optimizePrediction.integer && !cg_latentCmds.integer ) {
			// if we need to predict this command, or we've run out of space in the saved states queue
			if ( cmdNum >= predictCmd || (stateIndex + 1) % NUM_SAVED_STATES == cg.stateHead ) {
				// run the Pmove
				Pmove (&cg_pmove);

				numPredicted++; // debug code

				// record the last predicted command
				cg.lastPredictedCommand = cmdNum;

				// if we haven't run out of space in the saved states queue
				if ( (stateIndex + 1) % NUM_SAVED_STATES != cg.stateHead ) {
					// save the state for the false case (of cmdNum >= predictCmd)
					// in later calls to this function
					cg.savedPmoveStates[stateIndex] = *cg_pmove.ps;
					stateIndex = (stateIndex + 1) % NUM_SAVED_STATES;
					cg.stateTail = stateIndex;
				}
			}
			else {
				numPlayedBack++; // debug code

				if ( cg_showmiss.integer &&
						cg.savedPmoveStates[stateIndex].commandTime != cg_pmove.cmd.serverTime) {
					// this should ONLY happen just after changing the value of pmove_fixed
					CG_Printf( "saved state miss\n" );
				}

				// play back the command from the saved states
				*cg_pmove.ps = cg.savedPmoveStates[stateIndex];

				// go to the next element in the saved states array
				stateIndex = (stateIndex + 1) % NUM_SAVED_STATES;
			}
		}
		else {
			// run the Pmove
			Pmove (&cg_pmove);

			numPredicted++; // debug code
		}
//unlagged - optimized prediction
#endif

		moved = qtrue;

		// add push trigger movement effects
		CG_TouchTriggerPrediction();

		// check for predictable events that changed from previous predictions
		//CG_CheckChangedPredictableEvents(&cg.predictedPlayerState);
	}

#ifdef SMOKINGUNS
//unlagged - optimized prediction
	// do a /condump after a few seconds of this
	//CG_Printf("cg.time: %d, numPredicted: %d, numPlayedBack: %d\n", cg.time, numPredicted, numPlayedBack); // debug code
	// if everything is working right, numPredicted should be 1 more than 98%
	// of the time, meaning only ONE predicted move was done in the frame
	// you should see other values for numPredicted after IsUnacceptableError
	// returns nonzero, and that's it
//unlagged - optimized prediction
#endif

	if ( cg_showmiss.integer > 1 ) {
		CG_Printf( "[%i : %i] ", cg_pmove.cmd.serverTime, cg.time );
	}

	if ( !moved ) {
		if ( cg_showmiss.integer ) {
			CG_Printf( "not moved\n" );
		}
		return;
	}

	// adjust for the movement of the groundentity
	CG_AdjustPositionForMover( cg.predictedPlayerState.origin, 
		cg.predictedPlayerState.groundEntityNum, 
		cg.physicsTime, cg.time, cg.predictedPlayerState.origin, cg.predictedPlayerState.viewangles, cg.predictedPlayerState.viewangles);

	if ( cg_showmiss.integer ) {
		if (cg.predictedPlayerState.eventSequence > oldPlayerState.eventSequence + MAX_PS_EVENTS) {
			CG_Printf("WARNING: dropped event\n");
		}
	}

	// fire events and other transition triggered things
	CG_TransitionPlayerState( &cg.predictedPlayerState, &oldPlayerState );

	if ( cg_showmiss.integer ) {
		if (cg.eventSequence > cg.predictedPlayerState.eventSequence) {
			CG_Printf("WARNING: double event\n");
			cg.eventSequence = cg.predictedPlayerState.eventSequence;
		}
	}

	// predict weaponanims
#ifdef SMOKINGUNS
	cg.weapon2AnimOld = cg.weapon2Anim;
	cg.weaponAnimOld = cg.weaponAnim;
	cg.weaponold = cg.weapon;
	cg.weapon2old = cg.weapon2;

	cg.weapon2Anim = cg.predictedPlayerState.weapon2Anim;
	cg.weaponAnim = cg.predictedPlayerState.weaponAnim;
	cg.weapon = cg.predictedPlayerState.weapon;
	cg.weapon2 = cg.predictedPlayerState.weapon2;
	cg.animtime = cg.time;

	if(cg.gatlingmodeOld && !cg.gatlingmode && cg.weapon == WP_GATLING){
	} else {
		cg.gatlingmodeOld = cg.gatlingmode;
		cg.gatlingmode = cg.predictedPlayerState.stats[STAT_GATLING_MODE];
	}
#endif
}


