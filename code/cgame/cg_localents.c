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

// cg_localents.c -- every frame, generate renderer commands for locally
// processed entities, like smoke puffs, gibs, shells, etc.

#include "cg_local.h"

#define	MAX_LOCAL_ENTITIES	512
localEntity_t	cg_localEntities[MAX_LOCAL_ENTITIES];
localEntity_t	cg_activeLocalEntities;		// double linked list
localEntity_t	*cg_freeLocalEntities;		// single linked list
#ifdef SMOKINGUNS
// assigned entity if there are no entites, which could be freed(fire)
localEntity_t le_pretend;

/*
===================
CG_GetActiveLocalEntities
by; hika
2007-01-01

Get current active local entities.
Only used in ./code/cgame/cg_draw.c:CG_DrawTraceRate()
===================
*/
localEntity_t *CG_GetActiveLocalEntities( void ) {
	return &cg_activeLocalEntities;
}
#endif

/*
===================
CG_InitLocalEntities

This is called at startup and for tournement restarts
===================
*/
void	CG_InitLocalEntities( void ) {
	int		i;

	memset( cg_localEntities, 0, sizeof( cg_localEntities ) );
	cg_activeLocalEntities.next = &cg_activeLocalEntities;
	cg_activeLocalEntities.prev = &cg_activeLocalEntities;
	cg_freeLocalEntities = cg_localEntities;
	for ( i = 0 ; i < MAX_LOCAL_ENTITIES - 1 ; i++ ) {
		cg_localEntities[i].next = &cg_localEntities[i+1];
	}
}


/*
==================
CG_FreeLocalEntity
==================
*/
#ifndef SMOKINGUNS
void CG_FreeLocalEntity( localEntity_t *le ) {
#else
qboolean CG_FreeLocalEntity( localEntity_t *le ) {
#endif
	if ( !le->prev ) {
		CG_Error( "CG_FreeLocalEntity: not active" );
	}

	// these entites can't be removed
#ifdef SMOKINGUNS
	if((le->leFlags & LEF_FIRE) && le->endTime > cg.time)
		return qfalse;
#endif

	// remove from the doubly linked active list
	le->prev->next = le->next;
	le->next->prev = le->prev;

	// the free list is only singly linked
	le->next = cg_freeLocalEntities;
	cg_freeLocalEntities = le;

#ifdef SMOKINGUNS
	return qtrue;
#endif
}

#ifdef SMOKINGUNS
qboolean CG_FreeLocalFire( localEntity_t *le ) {
	if ( !le->prev ) {
		CG_Error( "CG_FreeLocalEntity: not active" );
	}

	// remove from the doubly linked active list
	le->prev->next = le->next;
	le->next->prev = le->prev;

	// the free list is only singly linked
	le->next = cg_freeLocalEntities;
	cg_freeLocalEntities = le;

	return qtrue;
}
#endif
/*
===================
CG_AllocLocalEntity

Will always succeed, even if it requires freeing an old active entity
===================
*/
localEntity_t	*CG_AllocLocalEntity( void ) {
	localEntity_t	*le;

	if ( !cg_freeLocalEntities ) {
		// no free entities, so free the one at the end of the chain
		// cycle through the entities and remove the oldest active valid entity
#ifndef SMOKINGUNS
		CG_FreeLocalEntity( cg_activeLocalEntities.prev );
#else
		int i;
		le = cg_activeLocalEntities.prev;
		for(i=0; i<MAX_LOCAL_ENTITIES && !CG_FreeLocalEntity( le ); i++){
			le = le->prev;
		}

		if(i == MAX_LOCAL_ENTITIES)
			return &le_pretend;
#endif
	}

	le = cg_freeLocalEntities;
	cg_freeLocalEntities = cg_freeLocalEntities->next;

	memset( le, 0, sizeof( *le ) );

	// link into the active list
	le->next = cg_activeLocalEntities.next;
	le->prev = &cg_activeLocalEntities;
	cg_activeLocalEntities.next->prev = le;
	cg_activeLocalEntities.next = le;
	return le;
}


/*
====================================================================================

FRAGMENT PROCESSING

A fragment localentity interacts with the environment in some way (hitting walls),
or generates more localentities along a trail.

====================================================================================
*/

/*
================
CG_BloodTrail

Leave expanding blood puffs behind gibs
================
*/
void CG_BloodTrail( localEntity_t *le ) {
	int		t;
#ifndef SMOKINGUNS
	int		t2;
	int		step;
	vec3_t	newOrigin;
	localEntity_t	*blood;

	step = 150;
	t = step * ( (cg.time - cg.frametime + step ) / step );
	t2 = step * ( cg.time / step );

	for ( ; t <= t2; t += step ) {
		BG_EvaluateTrajectory( &le->pos, t, newOrigin );

		blood = CG_SmokePuff( newOrigin, vec3_origin, 
					  20,		// radius
					  1, 1, 1, 1,	// color
					  2000,		// trailTime
					  t,		// startTime
					  0,		// fadeInTime
					  0,		// flags
					  cgs.media.bloodTrailShader );
		// use the optimized version
		blood->leType = LE_FALL_SCALE_FADE;
		// drop a total of 40 units over its lifetime
		blood->pos.trDelta[2] = 40;
	}
#else
	int		step;
	vec3_t	newOrigin;
	vec3_t	newDelta;

	localEntity_t	*blood;
	qboolean whiskey = qfalse; // is it really blood or is it whiskey?
	int				spawnTime = 250;

	if(le->leFlags & LEF_WHISKEY){
		whiskey = qtrue;
		spawnTime = 500;
	}

	if((cg.time - le->startTime) > spawnTime)
		return;

	if(whiskey){
		if(le->leFlags & LEF_FIRE)
			step = 30;
		else
			step = 20;
	} else
		step = 35;

	//t = step * ( (cg.time - cg.frametime + step ) / step );
	//t2 = step * ( cg.time / step );
	t = (cg.time - le->startTime)/step;

	if(t*step + le->startTime > cg.time)
		t--;

	t *= step;
	t += le->startTime;

	//for ( ; t <= t2; t += step ) {
	if ( t <= cg.time && t > cg.oldTime) {
		int time = cg.time - step;
		if(time < le->startTime)
			time = le->startTime;

		BG_EvaluateTrajectory( &le->pos, time, newOrigin );
		BG_EvaluateTrajectoryDelta( &le->pos, time, newDelta);
		VectorNormalize(newDelta);

		blood = CG_LaunchSpriteParticle(newOrigin, newDelta, 0, 2000, (LEF_PARTICLE|LEF_TRAIL));

		if(!whiskey){
			blood->refEntity.customShader = cgs.media.blood_particles[rand()%6];
			blood->refEntity.radius = 1;
		} else {
			blood->refEntity.customShader = cgs.media.whiskey_drops[rand()%6];
			blood->leFlags |= LEF_WHISKEY;
			blood->refEntity.radius = 1.0 + (rand()%10)/6;

			if(le->leFlags & LEF_FIRE)
				blood->leFlags |= LEF_FIRE;
		}

		blood->color[0] = 0.6f;
		blood->color[1] = 0.6f;
		blood->color[2] = 0.6f;
		blood->color[3] = 0.5f;
	}
#endif
}


/*
================
CG_FragmentBounceMark
================
*/
void CG_FragmentBounceMark( localEntity_t *le, trace_t *trace ) {
	int			radius;

	if ( le->leMarkType == LEMT_BLOOD ) {

		radius = 16 + (rand()&31);
		CG_ImpactMark( cgs.media.bloodMarkShader, trace->endpos, trace->plane.normal, random()*360,
#ifndef SMOKINGUNS
			1,1,1,1, qtrue, radius, qfalse );
#else
			1,1,1,1, qtrue, radius, qfalse, -1 );
#endif
	} else if ( le->leMarkType == LEMT_BURN ) {

		radius = 8 + (rand()&15);
		CG_ImpactMark( cgs.media.burnMarkShader, trace->endpos, trace->plane.normal, random()*360,
#ifndef SMOKINGUNS
			1,1,1,1, qtrue, radius, qfalse );
#else
			1,1,1,1, qtrue, radius, qfalse, -1 );
#endif
	}


	// don't allow a fragment to make multiple marks, or they
	// pile up while settling
	le->leMarkType = LEMT_NONE;
}

/*
================
CG_FragmentBounceSound
================
*/
static void CG_FragmentBounceSound( localEntity_t *le, trace_t *trace ) {
	if ( le->leBounceSoundType == LEBS_BLOOD ) {
		// half the gibs will make splat sounds
#ifndef SMOKINGUNS
		if ( rand() & 1 ) {
			int r = rand()&3;
			sfxHandle_t	s;

			if ( r == 0 ) {
				s = cgs.media.gibBounce1Sound;
			} else if ( r == 1 ) {
				s = cgs.media.gibBounce2Sound;
			} else {
				s = cgs.media.gibBounce3Sound;
			}
			trap_S_StartSound( trace->endpos, ENTITYNUM_WORLD, CHAN_AUTO, s );
		}
#endif
	} else if ( le->leBounceSoundType == LEBS_BRASS ) {

#ifdef SMOKINGUNS
	} else if (le->leBounceSoundType == LEBS_GLASS){
		trap_S_StartSound( trace->endpos, ENTITYNUM_WORLD, CHAN_AUTO,
			cgs.media.impact[IMPACT_GLASS][rand()%3] );
	} else if (le->leBounceSoundType == LEBS_METAL){
		trap_S_StartSound( trace->endpos, ENTITYNUM_WORLD, CHAN_AUTO,
			cgs.media.impact[IMPACT_METAL][rand()%3] );
	} else if (le->leBounceSoundType == LEBS_WOOD){
		trap_S_StartSound( trace->endpos, ENTITYNUM_WORLD, CHAN_AUTO,
			cgs.media.impact[IMPACT_WOOD][rand()%3] );
	} else if (le->leBounceSoundType == LEBS_DEFAULT){
		trap_S_StartSound( trace->endpos, ENTITYNUM_WORLD, CHAN_AUTO,
			cgs.media.impact[IMPACT_DEFAULT][rand()%3] );
#endif
	}

	// don't allow a fragment to make multiple bounce sounds,
	// or it gets too noisy as they settle
	le->leBounceSoundType = LEBS_NONE;
}


/*
================
CG_ReflectVelocity
================
*/
void CG_ReflectVelocity( localEntity_t *le, trace_t *trace ) {
	vec3_t	velocity;
	float	dot;
	int		hitTime;

	// reflect the velocity on the trace plane
	hitTime = cg.time - cg.frametime + cg.frametime * trace->fraction;
	BG_EvaluateTrajectoryDelta( &le->pos, hitTime, velocity );
	dot = DotProduct( velocity, trace->plane.normal );
	VectorMA( velocity, -2*dot, trace->plane.normal, le->pos.trDelta );

	VectorScale( le->pos.trDelta, le->bounceFactor, le->pos.trDelta );

	VectorCopy( trace->endpos, le->pos.trBase );
	le->pos.trTime = cg.time;


	// check for stop, making sure that even on low FPS systems it doesn't bobble
	if ( trace->allsolid ||
		( trace->plane.normal[2] > 0 &&
		( le->pos.trDelta[2] < 40 || le->pos.trDelta[2] < -cg.frametime * le->pos.trDelta[2] ) ) ) {
		le->pos.trType = TR_STATIONARY;
	} else {

	}
}

/*
==============
CG_DeleteRoundEntities
==============
*/
#ifdef SMOKINGUNS
void CG_DeleteRoundEntities(void){
	localEntity_t *le, *next;

	le = cg_activeLocalEntities.prev;
	for ( ; le != &cg_activeLocalEntities ; le = next ) {
		next = le->prev;

		if(le->leFlags & LEF_REMOVE){
			CG_FreeLocalFire(le);
		}
	}
}

/*
================
CG_CreateFire
================
*/
void CG_CreateFire(vec3_t origin, vec3_t normal){
	vec4_t	light;
	float	red, green, blue, factor;
	localEntity_t	*le;
	refEntity_t		*re;

	// make burning entity
	le = CG_AllocLocalEntity();
	re = &le->refEntity;

	le->leType = LE_FRAGMENT;

	VectorCopy( origin, re->origin);
	VectorCopy( origin, re->oldorigin);
	AxisCopy( axisDefault, re->axis);

	le->pos.trType = TR_GRAVITY;
	VectorCopy(origin, le->pos.trBase );
	le->pos.trTime = cg.time;
	le->leFlags = (LEF_WHISKEY|LEF_FIRE|LEF_REMOVE);

	le->lightColor[0] = 2;
	le->startTime = cg.time;
	le->refEntity.rotation = rand()%360;
	le->endTime = cg.time + WHISKEY_BURNTIME + rand()%5000;
	le->refEntity.radius = 25 + rand()%16;

	// calculate lighting
	CG_LightForPoint(le->refEntity.origin, normal, light);

	factor = ((light[0]+light[1]+light[2])/3)/255;
	factor *= 1.3f;

	if(factor > 1)
		factor = 1;

	factor = 1-factor;
	factor = factor*factor*factor;
	factor = 1-factor;

	red = blue = green = factor;

	// male a black burn shader
	CG_ImpactMark(cgs.media.burnMarkShader, origin, normal,
		random()*360, red, green, blue, 1, qtrue, (rand()%8)+25, qfalse,
		WHISKEY_SICKERTIME);
}
#endif

/*
================
CG_AddFragment
================
*/
void CG_AddFragment( localEntity_t *le ) {
	vec3_t	newOrigin;
	trace_t	trace;
#ifndef SMOKINGUNS
	if ( le->pos.trType == TR_STATIONARY ) {
		// sink into the ground if near the removal time
		int		t;
		float	oldZ;
		
		t = le->endTime - cg.time;
		if ( t < SINK_TIME ) {
			// we must use an explicit lighting origin, otherwise the
			// lighting would be lost as soon as the origin went
			// into the ground
			VectorCopy( le->refEntity.origin, le->refEntity.lightingOrigin );
			le->refEntity.renderfx |= RF_LIGHTING_ORIGIN;
			oldZ = le->refEntity.origin[2];
			le->refEntity.origin[2] -= 16 * ( 1.0 - (float)t / SINK_TIME );
			trap_R_AddRefEntityToScene( &le->refEntity );
			le->refEntity.origin[2] = oldZ;
		} else {
			trap_R_AddRefEntityToScene( &le->refEntity );
		}

		return;
	}
#else
	qboolean visible = qtrue;

	// check if we hit water
	if(le->leFlags & LEF_WHISKEY){
		int contents = trap_CM_PointContents(le->refEntity.origin, 0);
		if(contents & CONTENTS_WATER){
			le->endTime = cg.time - 100;
			CG_FreeLocalEntity(le);
			return;
		}
	}

	// make it burning on the floor
	if((le->leFlags & LEF_WHISKEY) && (le->leFlags & LEF_FIRE) &&
		le->lightColor[0] == 2){
		refEntity_t		fire;
		//vec3_t			origin;

		memset( &fire, 0, sizeof( fire ) );
		fire.reType = RT_SPRITE;
		fire.customShader = cgs.media.wqfx_fire;
		fire.rotation = le->refEntity.rotation;
		fire.radius = le->refEntity.radius;
		fire.renderfx = 0;
		fire.shaderTime = le->startTime / 1000.0f;

		AnglesToAxis( vec3_origin, fire.axis );
		VectorCopy(le->refEntity.origin, fire.origin);

		// get bigger the 1st quarter second
		if(le->startTime + 250 > cg.time){
			float t = (float)(cg.time - le->startTime);

			fire.radius = sqrt(t/250)*le->refEntity.radius;
			fire.origin[2] -= sqrt((250-t)/250)*5;

		// get smaller in the last 5 seconds
		} else if(le->endTime - cg.time <= 5000){
			float t = (float)(le->endTime - cg.time);

			fire.radius = sqrt(t/5000)*le->refEntity.radius;
			fire.origin[2] -= sqrt((5000-t)/5000)*5;
		}
		VectorCopy(fire.origin, fire.oldorigin);

		trap_R_AddRefEntityToScene( &fire );

		/*VectorCopy(fire.origin, origin);

		origin[2] += 20;
		trap_R_AddLightToScene(origin, 20,
			cg_weapons[WP_MOLOTOV].missileDlightColor[0],
			cg_weapons[WP_MOLOTOV].missileDlightColor[1],
			cg_weapons[WP_MOLOTOV].missileDlightColor[2] );*/

		trap_S_AddLoopingSound( ENTITYNUM_NONE, fire.origin, vec3_origin,
				cgs.media.firesound );
		return;
	}

	if(le->refEntity.reType == RT_SPRITE && (le->leFlags & LEF_PARTICLE)){
		vec4_t	light, color;
		int		i;
		vec3_t	normal;

		// first check if particle is too far away
		if(Distance(le->refEntity.origin, cg.refdef.vieworg) > 500)
			visible = qfalse;

		Vector4Copy(le->color, color);
		VectorSet(normal, 0, 0, 1);

		CG_LightForPoint(le->refEntity.origin, normal, light);

		for(i=0;i<3;i++){
			float temp = light[i]/255;
			temp = sqrt(temp);
			temp *= 255;

			color[i] *= temp*0.87f + 255*0.13f;
		}
		color[3] *= 255;

		Vector4Copy(color, le->refEntity.shaderRGBA);

	} else if (le->leFlags & LEF_COLOR) {

		Vector4Copy(le->color, le->refEntity.shaderRGBA);
	}

	if(le->pos.trType == TR_STATIONARY && (le->leFlags & LEF_BREAKS) &&
		le->startTime <= cg.time)
		le->pos.trType = TR_GRAVITY_LOW2;

	if ( le->pos.trType == TR_STATIONARY && le->refEntity.reType != RT_SPRITE) {
		// sink into the ground if near the removal time
		int		t;
		float	oldZ;

		// breakable doesn't exist anymore
		if((le->leFlags & LEF_MARK) && le->refEntity.radius != -1 &&
			!cg_entities[(int)le->refEntity.radius].currentValid){
			CG_FreeLocalEntity( le );
			return;
		}

		t = le->endTime - cg.time;
		if ( t < SINK_TIME) {

			if(le->leFlags & LEF_MARK){
				//le->refEntity.shaderRGBA[3] = 255*((float)t/SINK_TIME);
				if(visible)
					trap_R_AddRefEntityToScene( &le->refEntity );
				return;
			}
			// we must use an explicit lighting origin, otherwise the
			// lighting would be lost as soon as the origin went
			// into the ground
			VectorCopy( le->refEntity.origin, le->refEntity.lightingOrigin );
			le->refEntity.renderfx |= RF_LIGHTING_ORIGIN;
			oldZ = le->refEntity.origin[2];
			le->refEntity.origin[2] -= 16 * ( 1.0 - (float)t / SINK_TIME );
			if(visible)
				trap_R_AddRefEntityToScene( &le->refEntity );
			le->refEntity.origin[2] = oldZ;
		} else {

			if(visible)
				trap_R_AddRefEntityToScene( &le->refEntity );
		}
		return;
	}
#endif

	// calculate new position
	BG_EvaluateTrajectory( &le->pos, cg.time, newOrigin );
#ifdef SMOKINGUNS
	if (cg_boostfps.integer) {
// FPS Optimization ?
		// Ignore check with all solid entities, their bounding sphere does not
		// contain the current local entity position and the next position.
		CG_Trace_Fragment( &trace, le->refEntity.origin, NULL, NULL, newOrigin,	-1, CONTENTS_SOLID );
// FPS Optimization ?
	}
	else
#endif
		// trace a line from previous position to new position
		CG_Trace( &trace, le->refEntity.origin, NULL, NULL, newOrigin, -1, CONTENTS_SOLID );

#ifdef SMOKINGUNS
	//check if new spark-particle has to be spawned
	if(le->leFlags & LEF_SPARKS){
		CG_CheckSparkGen(le);
	}

	if (  trace.fraction == 1.0 || (le->leFlags & LEF_SPARKS)) {
#else
	if ( trace.fraction == 1.0 ) {
#endif
		// still in free fall
		VectorCopy( newOrigin, le->refEntity.origin );

		if ( le->leFlags & LEF_TUMBLE ) {
			vec3_t angles;

			BG_EvaluateTrajectory( &le->angles, cg.time, angles );
			AnglesToAxis( angles, le->refEntity.axis );
#ifdef SMOKINGUNS
			if((le->leFlags & LEF_BREAKS) ||
				(le->leFlags & LEF_BREAKS_DEF)) {
				VectorScale(le->refEntity.axis[0], le->vector[0], le->refEntity.axis[0]);
				VectorScale(le->refEntity.axis[1], le->vector[1], le->refEntity.axis[1]);
				VectorScale(le->refEntity.axis[2], le->vector[2], le->refEntity.axis[2]);
			}
			le->refEntity.rotation = angles[YAW];
#endif
		}

#ifdef SMOKINGUNS
		if(visible && !(le->leFlags & LEF_FIRE))
#endif
			trap_R_AddRefEntityToScene( &le->refEntity );

		// add a blood trail
#ifndef SMOKINGUNS
		if ( le->leBounceSoundType == LEBS_BLOOD ) {
#else
		if ( le->leFlags & LEF_BLOOD ){
#endif
			CG_BloodTrail( le );
		}

#ifdef SMOKINGUNS
		// make it burning
		if (le->leFlags & LEF_FIRE){
			refEntity_t		fire;
			vec3_t			origin;

			memset( &fire, 0, sizeof( fire ) );
			fire.reType = RT_SPRITE;
			fire.customShader = cgs.media.wqfx_firedrop;
			fire.radius = 3;
			fire.renderfx = 0;

			AnglesToAxis( vec3_origin, fire.axis );
			VectorCopy(le->refEntity.origin, fire.origin);
			VectorCopy(fire.origin, fire.oldorigin);

			trap_R_AddRefEntityToScene( &fire );

			if(le->lightColor[0] == 1){
				VectorCopy(fire.origin, origin);

				origin[2] += 20;
				trap_R_AddLightToScene(origin, 50,
					cg_weapons[WP_MOLOTOV].missileDlightColor[0],
					cg_weapons[WP_MOLOTOV].missileDlightColor[1],
					cg_weapons[WP_MOLOTOV].missileDlightColor[2] );
			}
		}
#endif
		return;
	}

#ifdef SMOKINGUNS
	if(le->leFlags & LEF_TRAIL)
		return;

	//delete blood particle and make a mark on the wall/floor
	if(le->leFlags & LEF_BLOOD){
		vec4_t	light;
		float	red, green, blue, factor;
		qhandle_t shader;
		float	radius;
		int		time = -1;

		CG_LightForPoint(le->refEntity.origin, trace.plane.normal, light);

		factor = ((light[0]+light[1]+light[2])/3)/255;
		factor *= 1.3f;

		if(factor > 1)
			factor = 1;

		factor = 1-factor;
		factor = factor*factor*factor;
		factor = 1-factor;
		factor *= 0.6f;

		red = blue = green = factor;

		if(le->leFlags & LEF_WHISKEY){
			shader = cgs.media.whiskey_pools[rand()%3];
			radius = (rand()%3)+12;

			time = WHISKEY_SICKERTIME;
		} else {
			shader = cgs.media.blood_marks[rand()%3];
			radius = (rand()%5)+2;
		}

		// don't delete it, if burning
		if((le->leFlags & LEF_WHISKEY) && (le->leFlags & LEF_FIRE)){
			le->lightColor[0] = 2;
			le->refEntity.rotation = rand()%360;
			le->startTime = cg.time;
			le->endTime = cg.time + WHISKEY_BURNTIME + rand()%5000;
			le->refEntity.radius = 25 + rand()%16;

			// male a black burn shader
			CG_ImpactMark(cgs.media.burnMarkShader, trace.endpos, trace.plane.normal,
				random()*360, red, green, blue, 1, qtrue, (rand()%8)+25, qfalse,
				time);
		} else  {
			CG_ImpactMark(shader, trace.endpos, trace.plane.normal,
				random()*360, red, green, blue, 1, qtrue, radius, qfalse,
				time);
				CG_FreeLocalEntity( le );
		}
		return;
	}

	if(le->refEntity.reType == RT_SPRITE){
		if(cg.time - le->startTime < 200)
			return;
		VectorClear(le->angles.trDelta);
	}

	//make the glassgibs lying on the floor with the right angles
	if(((le->leFlags & LEF_BREAKS) || (le->leFlags & LEF_BREAKS_DEF))
		&& trace.fraction < 1.0){
		le->angles.trBase[0] = 0.0;
		le->angles.trBase[2] = 90.0;

		VectorClear(le->angles.trDelta);
	}
#endif

	// if it is in a nodrop zone, remove it
	// this keeps gibs from waiting at the bottom of pits of death
	// and floating levels
	if ( CG_PointContents( trace.endpos, 0 ) & CONTENTS_NODROP ) {
		CG_FreeLocalEntity( le );
		return;
	}

	// leave a mark
	CG_FragmentBounceMark( le, &trace );

	// do a bouncy sound
	CG_FragmentBounceSound( le, &trace );

	// reflect the velocity on the trace plane
	CG_ReflectVelocity( le, &trace );

#ifdef SMOKINGUNS
	if(visible)
#endif
		trap_R_AddRefEntityToScene( &le->refEntity );
}

/*
================
CG_AddSmoke by Spoon
Adds smoke which will rise to the sky
================
*/
#ifdef SMOKINGUNS
#define SPEED		0.015f
#define FADE_IN		750.0f
#define FADE_OUT	2000.0f
void CG_AddSmoke( localEntity_t *le ) {

	refEntity_t	re;
	int fade_in, fade_out;
	float boost = SPEED * (float)(le->boost/5);
	float wind = SPEED * (float)(le->wind/5);
	float windfactor = 1;
	int temp;

	re = le->refEntity;

	fade_in = cg.time - le->startTime;
	fade_out = le->endTime - cg.time;

	re.rotation += fade_in/20;

	Vector4Copy(le->color, re.shaderRGBA);
	temp = le->color[3];

	if(fade_in <= FADE_IN){
		temp = (float)le->color[3] * ((float)fade_in/FADE_IN);
		re.radius *= ((float)fade_in/FADE_IN);
	} else if( fade_out <= FADE_OUT){
		temp = (float)le->color[3] * ((float)fade_out/FADE_OUT);
		re.radius *= 1 + ((float)(FADE_OUT-fade_out)/750);
	}

	re.shaderRGBA[3] = (int)temp;

	re.reType = RT_SPRITE;

	// move up slowly
	re.origin[2] += fade_in*boost;

	// if there's wind
	if((le->vector[0] || le->vector[1] || le->vector[2]) &&
		le->wind){
		// the first seconds let the wind fade in
		if(fade_in < FADE_IN*2){
			windfactor = fade_in/(FADE_IN*2);
		}
		VectorMA(re.origin, fade_in*wind*windfactor, le->vector, re.origin);
	}

	trap_R_AddRefEntityToScene( &re );
}
#endif

/*
=====================================================================

TRIVIAL LOCAL ENTITIES

These only do simple scaling or modulation before passing to the renderer
=====================================================================
*/

/*
====================
CG_AddFadeRGB
====================
*/
void CG_AddFadeRGB( localEntity_t *le ) {
	refEntity_t *re;
	float c;

	re = &le->refEntity;

	c = ( le->endTime - cg.time ) * le->lifeRate;
	c *= 0xff;

	re->shaderRGBA[0] = le->color[0] * c;
	re->shaderRGBA[1] = le->color[1] * c;
	re->shaderRGBA[2] = le->color[2] * c;
	re->shaderRGBA[3] = le->color[3] * c;

	trap_R_AddRefEntityToScene( re );
}

/*
==================
CG_AddMoveScaleFade
==================
*/
static void CG_AddMoveScaleFade( localEntity_t *le ) {
	refEntity_t	*re;
	float		c;
	vec3_t		delta;
	float		len;

	re = &le->refEntity;

	if ( le->fadeInTime > le->startTime && cg.time < le->fadeInTime ) {
		// fade / grow time
		c = 1.0 - (float) ( le->fadeInTime - cg.time ) / ( le->fadeInTime - le->startTime );
	}
	else {
		// fade / grow time
		c = ( le->endTime - cg.time ) * le->lifeRate;
	}

	re->shaderRGBA[3] = 0xff * c * le->color[3];

	if ( !( le->leFlags & LEF_PUFF_DONT_SCALE ) ) {
		re->radius = le->radius * ( 1.0 - c ) + 8;
	}

	BG_EvaluateTrajectory( &le->pos, cg.time, re->origin );

	// if the view would be "inside" the sprite, kill the sprite
	// so it doesn't add too much overdraw
	VectorSubtract( re->origin, cg.refdef.vieworg, delta );
	len = VectorLength( delta );
	if ( len < le->radius ) {
		CG_FreeLocalEntity( le );
		return;
	}

	trap_R_AddRefEntityToScene( re );
}


/*
===================
CG_AddScaleFade

For rocket smokes that hang in place, fade out, and are
removed if the view passes through them.
There are often many of these, so it needs to be simple.
===================
*/
static void CG_AddScaleFade( localEntity_t *le ) {
	refEntity_t	*re;
	float		c;
	vec3_t		delta;
	float		len;

	re = &le->refEntity;

	// fade / grow time
	c = ( le->endTime - cg.time ) * le->lifeRate;

	re->shaderRGBA[3] = 0xff * c * le->color[3];
	re->radius = le->radius * ( 1.0 - c ) + 8;

	// if the view would be "inside" the sprite, kill the sprite
	// so it doesn't add too much overdraw
	VectorSubtract( re->origin, cg.refdef.vieworg, delta );
	len = VectorLength( delta );
	if ( len < le->radius ) {
		CG_FreeLocalEntity( le );
		return;
	}

	trap_R_AddRefEntityToScene( re );
}


/*
=================
CG_AddFallScaleFade

This is just an optimized CG_AddMoveScaleFade
For blood mists that drift down, fade out, and are
removed if the view passes through them.
There are often 100+ of these, so it needs to be simple.
=================
*/
static void CG_AddFallScaleFade( localEntity_t *le ) {
	refEntity_t	*re;
	float		c;
	vec3_t		delta;
	float		len;

	re = &le->refEntity;

	// fade time
	c = ( le->endTime - cg.time ) * le->lifeRate;

	re->shaderRGBA[3] = 0xff * c * le->color[3];

	re->origin[2] = le->pos.trBase[2] - ( 1.0 - c ) * le->pos.trDelta[2];

	re->radius = le->radius * ( 1.0 - c ) + 16;

	// if the view would be "inside" the sprite, kill the sprite
	// so it doesn't add too much overdraw
	VectorSubtract( re->origin, cg.refdef.vieworg, delta );
	len = VectorLength( delta );
	if ( len < le->radius ) {
		CG_FreeLocalEntity( le );
		return;
	}

	trap_R_AddRefEntityToScene( re );
}



/*
================
CG_AddExplosion
================
*/
static void CG_AddExplosion( localEntity_t *ex ) {
	refEntity_t	*ent;

	ent = &ex->refEntity;

	// add the entity
	trap_R_AddRefEntityToScene(ent);

	// add the dlight
	if ( ex->light ) {
		float		light;

		light = (float)( cg.time - ex->startTime ) / ( ex->endTime - ex->startTime );
		if ( light < 0.5 ) {
			light = 1.0;
		} else {
			light = 1.0 - ( light - 0.5 ) * 2;
		}
		light = ex->light * light;
		trap_R_AddLightToScene(ent->origin, light, ex->lightColor[0], ex->lightColor[1], ex->lightColor[2] );
	}
}

/*
================
CG_AddSpriteExplosion
================
*/
static void CG_AddSpriteExplosion( localEntity_t *le ) {
	refEntity_t	re;
	float c;

	re = le->refEntity;

	c = ( le->endTime - cg.time ) / ( float ) ( le->endTime - le->startTime );
	if ( c > 1 ) {
		c = 1.0;	// can happen during connection problems
	}

	re.shaderRGBA[0] = 0xff;
	re.shaderRGBA[1] = 0xff;
	re.shaderRGBA[2] = 0xff;
	re.shaderRGBA[3] = 0xff * c * 0.33;

	re.reType = RT_SPRITE;
#ifndef SMOKINGUNS
	re.radius = 42 * ( 1.0 - c ) + 30;
#endif

	trap_R_AddRefEntityToScene( &re );

	// add the dlight
	if ( le->light ) {
		float		light;

		light = (float)( cg.time - le->startTime ) / ( le->endTime - le->startTime );
		if ( light < 0.5 ) {
			light = 1.0;
		} else {
			light = 1.0 - ( light - 0.5 ) * 2;
		}
		light = le->light * light;
		trap_R_AddLightToScene(re.origin, light, le->lightColor[0], le->lightColor[1], le->lightColor[2] );
	}
}


#ifndef SMOKINGUNS
/*
====================
CG_AddKamikaze
====================
*/
void CG_AddKamikaze( localEntity_t *le ) {
	refEntity_t	*re;
	refEntity_t shockwave;
	float		c;
	vec3_t		test, axis[3];
	int			t;

	re = &le->refEntity;

	t = cg.time - le->startTime;
	VectorClear( test );
	AnglesToAxis( test, axis );

	if (t > KAMI_SHOCKWAVE_STARTTIME && t < KAMI_SHOCKWAVE_ENDTIME) {

		if (!(le->leFlags & LEF_SOUND1)) {
//			trap_S_StartSound (re->origin, ENTITYNUM_WORLD, CHAN_AUTO, cgs.media.kamikazeExplodeSound );
			trap_S_StartLocalSound(cgs.media.kamikazeExplodeSound, CHAN_AUTO);
			le->leFlags |= LEF_SOUND1;
		}
		// 1st kamikaze shockwave
		memset(&shockwave, 0, sizeof(shockwave));
		shockwave.hModel = cgs.media.kamikazeShockWave;
		shockwave.reType = RT_MODEL;
		shockwave.shaderTime = re->shaderTime;
		VectorCopy(re->origin, shockwave.origin);

		c = (float)(t - KAMI_SHOCKWAVE_STARTTIME) / (float)(KAMI_SHOCKWAVE_ENDTIME - KAMI_SHOCKWAVE_STARTTIME);
		VectorScale( axis[0], c * KAMI_SHOCKWAVE_MAXRADIUS / KAMI_SHOCKWAVEMODEL_RADIUS, shockwave.axis[0] );
		VectorScale( axis[1], c * KAMI_SHOCKWAVE_MAXRADIUS / KAMI_SHOCKWAVEMODEL_RADIUS, shockwave.axis[1] );
		VectorScale( axis[2], c * KAMI_SHOCKWAVE_MAXRADIUS / KAMI_SHOCKWAVEMODEL_RADIUS, shockwave.axis[2] );
		shockwave.nonNormalizedAxes = qtrue;

		if (t > KAMI_SHOCKWAVEFADE_STARTTIME) {
			c = (float)(t - KAMI_SHOCKWAVEFADE_STARTTIME) / (float)(KAMI_SHOCKWAVE_ENDTIME - KAMI_SHOCKWAVEFADE_STARTTIME);
		}
		else {
			c = 0;
		}
		c *= 0xff;
		shockwave.shaderRGBA[0] = 0xff - c;
		shockwave.shaderRGBA[1] = 0xff - c;
		shockwave.shaderRGBA[2] = 0xff - c;
		shockwave.shaderRGBA[3] = 0xff - c;

		trap_R_AddRefEntityToScene( &shockwave );
	}

	if (t > KAMI_EXPLODE_STARTTIME && t < KAMI_IMPLODE_ENDTIME) {
		// explosion and implosion
		c = ( le->endTime - cg.time ) * le->lifeRate;
		c *= 0xff;
		re->shaderRGBA[0] = le->color[0] * c;
		re->shaderRGBA[1] = le->color[1] * c;
		re->shaderRGBA[2] = le->color[2] * c;
		re->shaderRGBA[3] = le->color[3] * c;

		if( t < KAMI_IMPLODE_STARTTIME ) {
			c = (float)(t - KAMI_EXPLODE_STARTTIME) / (float)(KAMI_IMPLODE_STARTTIME - KAMI_EXPLODE_STARTTIME);
		}
		else {
			if (!(le->leFlags & LEF_SOUND2)) {
//				trap_S_StartSound (re->origin, ENTITYNUM_WORLD, CHAN_AUTO, cgs.media.kamikazeImplodeSound );
				trap_S_StartLocalSound(cgs.media.kamikazeImplodeSound, CHAN_AUTO);
				le->leFlags |= LEF_SOUND2;
			}
			c = (float)(KAMI_IMPLODE_ENDTIME - t) / (float) (KAMI_IMPLODE_ENDTIME - KAMI_IMPLODE_STARTTIME);
		}
		VectorScale( axis[0], c * KAMI_BOOMSPHERE_MAXRADIUS / KAMI_BOOMSPHEREMODEL_RADIUS, re->axis[0] );
		VectorScale( axis[1], c * KAMI_BOOMSPHERE_MAXRADIUS / KAMI_BOOMSPHEREMODEL_RADIUS, re->axis[1] );
		VectorScale( axis[2], c * KAMI_BOOMSPHERE_MAXRADIUS / KAMI_BOOMSPHEREMODEL_RADIUS, re->axis[2] );
		re->nonNormalizedAxes = qtrue;

		trap_R_AddRefEntityToScene( re );
		// add the dlight
		trap_R_AddLightToScene( re->origin, c * 1000.0, 1.0, 1.0, c );
	}

	if (t > KAMI_SHOCKWAVE2_STARTTIME && t < KAMI_SHOCKWAVE2_ENDTIME) {
		// 2nd kamikaze shockwave
		if (le->angles.trBase[0] == 0 &&
			le->angles.trBase[1] == 0 &&
			le->angles.trBase[2] == 0) {
			le->angles.trBase[0] = random() * 360;
			le->angles.trBase[1] = random() * 360;
			le->angles.trBase[2] = random() * 360;
		}
		memset(&shockwave, 0, sizeof(shockwave));
		shockwave.hModel = cgs.media.kamikazeShockWave;
		shockwave.reType = RT_MODEL;
		shockwave.shaderTime = re->shaderTime;
		VectorCopy(re->origin, shockwave.origin);

		test[0] = le->angles.trBase[0];
		test[1] = le->angles.trBase[1];
		test[2] = le->angles.trBase[2];
		AnglesToAxis( test, axis );

		c = (float)(t - KAMI_SHOCKWAVE2_STARTTIME) / (float)(KAMI_SHOCKWAVE2_ENDTIME - KAMI_SHOCKWAVE2_STARTTIME);
		VectorScale( axis[0], c * KAMI_SHOCKWAVE2_MAXRADIUS / KAMI_SHOCKWAVEMODEL_RADIUS, shockwave.axis[0] );
		VectorScale( axis[1], c * KAMI_SHOCKWAVE2_MAXRADIUS / KAMI_SHOCKWAVEMODEL_RADIUS, shockwave.axis[1] );
		VectorScale( axis[2], c * KAMI_SHOCKWAVE2_MAXRADIUS / KAMI_SHOCKWAVEMODEL_RADIUS, shockwave.axis[2] );
		shockwave.nonNormalizedAxes = qtrue;

		if (t > KAMI_SHOCKWAVE2FADE_STARTTIME) {
			c = (float)(t - KAMI_SHOCKWAVE2FADE_STARTTIME) / (float)(KAMI_SHOCKWAVE2_ENDTIME - KAMI_SHOCKWAVE2FADE_STARTTIME);
		}
		else {
			c = 0;
		}
		c *= 0xff;
		shockwave.shaderRGBA[0] = 0xff - c;
		shockwave.shaderRGBA[1] = 0xff - c;
		shockwave.shaderRGBA[2] = 0xff - c;
		shockwave.shaderRGBA[3] = 0xff - c;

		trap_R_AddRefEntityToScene( &shockwave );
	}
}

/*
===================
CG_AddInvulnerabilityImpact
===================
*/
void CG_AddInvulnerabilityImpact( localEntity_t *le ) {
	trap_R_AddRefEntityToScene( &le->refEntity );
}

/*
===================
CG_AddInvulnerabilityJuiced
===================
*/
void CG_AddInvulnerabilityJuiced( localEntity_t *le ) {
	int t;

	t = cg.time - le->startTime;
	if ( t > 3000 ) {
		le->refEntity.axis[0][0] = (float) 1.0 + 0.3 * (t - 3000) / 2000;
		le->refEntity.axis[1][1] = (float) 1.0 + 0.3 * (t - 3000) / 2000;
		le->refEntity.axis[2][2] = (float) 0.7 + 0.3 * (2000 - (t - 3000)) / 2000;
	}
	if ( t > 5000 ) {
		le->endTime = 0;
		CG_GibPlayer( le->refEntity.origin );
	}
	else {
		trap_R_AddRefEntityToScene( &le->refEntity );
	}
}
#else

/*
===================
CG_AddRefEntity
===================
*/
void CG_AddRefEntity( localEntity_t *le ) {
	if (le->endTime < cg.time) {
		CG_FreeLocalEntity( le );
		return;
	}
	trap_R_AddRefEntityToScene( &le->refEntity );
}

#endif
/*
===================
CG_AddScorePlum
===================
*/
#ifndef SMOKINGUNS
#define NUMBER_SIZE		8

void CG_AddScorePlum( localEntity_t *le ) {
	refEntity_t	*re;
	vec3_t		origin, delta, dir, vec, up = {0, 0, 1};
	float		c, len;
	int			i, score, digits[10], numdigits, negative;

	re = &le->refEntity;

	c = ( le->endTime - cg.time ) * le->lifeRate;

	score = le->radius;
	if (score < 0) {
		re->shaderRGBA[0] = 0xff;
		re->shaderRGBA[1] = 0x11;
		re->shaderRGBA[2] = 0x11;
	}
	else {
		re->shaderRGBA[0] = 0xff;
		re->shaderRGBA[1] = 0xff;
		re->shaderRGBA[2] = 0xff;
		if (score >= 50) {
			re->shaderRGBA[1] = 0;
		} else if (score >= 20) {
			re->shaderRGBA[0] = re->shaderRGBA[1] = 0;
		} else if (score >= 10) {
			re->shaderRGBA[2] = 0;
		} else if (score >= 2) {
			re->shaderRGBA[0] = re->shaderRGBA[2] = 0;
		}

	}
	if (c < 0.25)
		re->shaderRGBA[3] = 0xff * 4 * c;
	else
		re->shaderRGBA[3] = 0xff;

	re->radius = NUMBER_SIZE / 2;

	VectorCopy(le->pos.trBase, origin);
	origin[2] += 110 - c * 100;

	VectorSubtract(cg.refdef.vieworg, origin, dir);
	CrossProduct(dir, up, vec);
	VectorNormalize(vec);

	VectorMA(origin, -10 + 20 * sin(c * 2 * M_PI), vec, origin);

	// if the view would be "inside" the sprite, kill the sprite
	// so it doesn't add too much overdraw
	VectorSubtract( origin, cg.refdef.vieworg, delta );
	len = VectorLength( delta );
	if ( len < 20 ) {
		CG_FreeLocalEntity( le );
		return;
	}

	negative = qfalse;
	if (score < 0) {
		negative = qtrue;
		score = -score;
	}

	for (numdigits = 0; !(numdigits && !score); numdigits++) {
		digits[numdigits] = score % 10;
		score = score / 10;
	}

	if (negative) {
		digits[numdigits] = 10;
		numdigits++;
	}

	for (i = 0; i < numdigits; i++) {
		VectorMA(origin, (float) (((float) numdigits / 2) - i) * NUMBER_SIZE, vec, re->origin);
		re->customShader = cgs.media.numberShaders[digits[numdigits-1-i]];
		trap_R_AddRefEntityToScene( re );
	}
}
#endif




//==============================================================================

/*
===================
CG_AddLocalEntities

===================
*/
void CG_AddLocalEntities( void ) {
	localEntity_t	*le, *next;

	// walk the list backwards, so any new local entities generated
	// (trails, marks, etc) will be present this frame
	le = cg_activeLocalEntities.prev;
	for ( ; le != &cg_activeLocalEntities ; le = next ) {
		// grab next now, so if the local entity is freed we
		// still have it
		next = le->prev;

		if ( cg.time >= le->endTime ) {
			CG_FreeLocalEntity( le );
			continue;
		}
		switch ( le->leType ) {
		default:
			CG_Error( "Bad leType: %i", le->leType );
			break;

		case LE_MARK:
			break;

#ifdef SMOKINGUNS
		case LE_SMOKE:
			CG_AddSmoke( le );
			break;
#endif

		case LE_SPRITE_EXPLOSION:
			CG_AddSpriteExplosion( le );
			break;

		case LE_EXPLOSION:
			CG_AddExplosion( le );
			break;

		case LE_FRAGMENT:			// gibs and brass
			CG_AddFragment( le );
			break;

		case LE_MOVE_SCALE_FADE:		// water bubbles
			CG_AddMoveScaleFade( le );
			break;

		case LE_FADE_RGB:				// teleporters, railtrails
			CG_AddFadeRGB( le );
			break;

		case LE_FALL_SCALE_FADE: // gib blood trails
			CG_AddFallScaleFade( le );
			break;

		case LE_SCALE_FADE:		// rocket trails
			CG_AddScaleFade( le );
			break;

#ifndef SMOKINGUNS
		case LE_SCOREPLUM:
			CG_AddScorePlum( le );
			break;
		case LE_KAMIKAZE:
			CG_AddKamikaze( le );
			break;
		case LE_INVULIMPACT:
			CG_AddInvulnerabilityImpact( le );
			break;
		case LE_INVULJUICED:
			CG_AddInvulnerabilityJuiced( le );
			break;
#else
		case LE_SHOWREFENTITY:
			CG_AddRefEntity( le );
			break;
#endif
		}
	}
}




