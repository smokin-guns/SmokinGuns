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
// cg_ents.c -- present snapshot entities, happens every single frame

#include "cg_local.h"


/*
======================
CG_PositionEntityOnTag

Modifies the entities position and axis by the given
tag location
======================
*/
void CG_PositionEntityOnTag( refEntity_t *entity, const refEntity_t *parent,
							qhandle_t parentModel, char *tagName ) {
	int				i;
	orientation_t	lerped;

	// lerp the tag
	trap_R_LerpTag( &lerped, parentModel, parent->oldframe, parent->frame,
		1.0 - parent->backlerp, tagName );

	// FIXME: allow origin offsets along tag?
	VectorCopy( parent->origin, entity->origin );
	for ( i = 0 ; i < 3 ; i++ ) {
		VectorMA( entity->origin, lerped.origin[i], parent->axis[i], entity->origin );
	}

	// had to cast away the const to avoid compiler problems...
	MatrixMultiply( lerped.axis, ((refEntity_t *)parent)->axis, entity->axis );
	entity->backlerp = parent->backlerp;
}

#ifdef SMOKINGUNS
void CG_PositionViewWeaponOnTag( refEntity_t *entity, const refEntity_t *parent,
							qhandle_t parentModel, char *tagName ) {
	int				i;
	orientation_t	lerped;
	//vec3_t			tempAxis[3];

	// lerp the tag
	trap_R_LerpTag( &lerped, parentModel, parent->oldframe, parent->frame,
		1.0 - parent->backlerp, tagName );

	// FIXME: allow origin offsets along tag?
	VectorCopy( parent->origin, entity->origin );
	for ( i = 0 ; i < 3 ; i++ ) {
		VectorMA( entity->origin, lerped.origin[i], parent->axis[i], entity->origin );
	}

	// had to cast away the const to avoid compiler problems...
	MatrixMultiply( lerped.axis, ((refEntity_t *)parent)->axis, entity->axis );
	//MatrixMultiply( entity->axis, lerped.axis, tempAxis );
	//MatrixMultiply( tempAxis, ((refEntity_t *)parent)->axis, entity->axis );
}
#endif

/*
======================
CG_PositionRotatedEntityOnTag

Modifies the entities position and axis by the given
tag location
======================
*/
void CG_PositionRotatedEntityOnTag( refEntity_t *entity, const refEntity_t *parent,
							qhandle_t parentModel, char *tagName ) {
	int				i;
	orientation_t	lerped;
	vec3_t			tempAxis[3];

	//AxisClear( entity->axis );
	// lerp the tag
	trap_R_LerpTag( &lerped, parentModel, parent->oldframe, parent->frame,
		1.0 - parent->backlerp, tagName );

	// FIXME: allow origin offsets along tag?
	VectorCopy( parent->origin, entity->origin );
	for ( i = 0 ; i < 3 ; i++ ) {
		VectorMA( entity->origin, lerped.origin[i], parent->axis[i], entity->origin );
	}

	// had to cast away the const to avoid compiler problems...
	MatrixMultiply( entity->axis, lerped.axis, tempAxis );
	MatrixMultiply( tempAxis, ((refEntity_t *)parent)->axis, entity->axis );
}



/*
==========================================================================

FUNCTIONS CALLED EACH FRAME

==========================================================================
*/

/*
======================
CG_SetEntitySoundPosition

Also called by event processing code
======================
*/
void CG_SetEntitySoundPosition( centity_t *cent ) {
	if ( cent->currentState.solid == SOLID_BMODEL ) {
		vec3_t	origin;
		float	*v;

		v = cgs.inlineModelMidpoints[ cent->currentState.modelindex ];
		VectorAdd( cent->lerpOrigin, v, origin );
		trap_S_UpdateEntityPosition( cent->currentState.number, origin );
	} else {
		trap_S_UpdateEntityPosition( cent->currentState.number, cent->lerpOrigin );
	}
}

/*
==================
CG_EntityEffects

Add continuous entity effects, like local entity emission and lighting
==================
*/
static void CG_EntityEffects( centity_t *cent ) {

	// update sound origins
	CG_SetEntitySoundPosition( cent );

	// add loop sound
	if ( cent->currentState.loopSound ) {
		if (cent->currentState.eType != ET_SPEAKER) {
			trap_S_AddLoopingSound( cent->currentState.number, cent->lerpOrigin, vec3_origin,
				cgs.gameSounds[ cent->currentState.loopSound ] );
		} else {
			trap_S_AddRealLoopingSound( cent->currentState.number, cent->lerpOrigin, vec3_origin,
				cgs.gameSounds[ cent->currentState.loopSound ] );
		}
	}


	// constant light glow
	if(cent->currentState.constantLight)
	{
		int		cl;
		float		i, r, g, b;

		cl = cent->currentState.constantLight;
		r = (float) (cl & 0xFF) / 255.0;
		g = (float) ((cl >> 8) & 0xFF) / 255.0;
		b = (float) ((cl >> 16) & 0xFF) / 255.0;
		i = (float) ((cl >> 24) & 0xFF) * 4.0;
		trap_R_AddLightToScene(cent->lerpOrigin, i, r, g, b);
	}

}


/*
==================
CG_General
==================
*/
static void CG_General( centity_t *cent ) {
	refEntity_t			ent;
	entityState_t		*s1;

	s1 = &cent->currentState;

	// if set to invisible, skip
	if (!s1->modelindex) {
		return;
	}

	memset (&ent, 0, sizeof(ent));

	// set frame

	ent.frame = s1->frame;
	ent.oldframe = ent.frame;
	ent.backlerp = 0;

	VectorCopy( cent->lerpOrigin, ent.origin);
	VectorCopy( cent->lerpOrigin, ent.oldorigin);

	ent.hModel = cgs.gameModels[s1->modelindex];

	// player model
	if (s1->number == cg.snap->ps.clientNum) {
		ent.renderfx |= RF_THIRD_PERSON;	// only draw from mirrors
	}

	// convert angles to axis
	AnglesToAxis( cent->lerpAngles, ent.axis );

	// add to refresh list
	trap_R_AddRefEntityToScene (&ent);
}

/*
==================
CG_Speaker

Speaker entities can automatically play sounds
==================
*/
static void CG_Speaker( centity_t *cent ) {
	if ( ! cent->currentState.clientNum ) {	// FIXME: use something other than clientNum...
		return;		// not auto triggering
	}

	if ( cg.time < cent->miscTime ) {
		return;
	}

	trap_S_StartSound (NULL, cent->currentState.number, CHAN_ITEM, cgs.gameSounds[cent->currentState.eventParm] );

	//	ent->s.frame = ent->wait * 10;
	//	ent->s.clientNum = ent->random * 10;
	cent->miscTime = cg.time + cent->currentState.frame * 100 + cent->currentState.clientNum * 100 * crandom();
}

/*
==================
CG_Item
==================
*/
static void CG_Item( centity_t *cent ) {
	refEntity_t		ent;
	entityState_t	*es;
	gitem_t			*item;
	int				msec;
	float			frac;
#ifndef SMOKINGUNS
	float			scale;
#endif
	weaponInfo_t	*wi;

	es = &cent->currentState;
	if ( es->modelindex >= bg_numItems ) {
		CG_Error( "Bad item index %i on entity", es->modelindex );
	}

	// if set to invisible, skip
	if ( !es->modelindex || ( es->eFlags & EF_NODRAW ) ) {
		return;
	}

	item = &bg_itemlist[ es->modelindex ];
	if ( cg_simpleItems.integer && item->giType != IT_TEAM ) {
		memset( &ent, 0, sizeof( ent ) );
		ent.reType = RT_SPRITE;
		VectorCopy( cent->lerpOrigin, ent.origin );
#ifndef SMOKINGUNS
		ent.radius = 14;
#else
		ent.radius = 7;
#endif
		ent.customShader = cg_items[es->modelindex].icon;
#ifdef SMOKINGUNS
		if(!Q_stricmp(item->classname, "pickup_money")){
			if(cent->currentState.time2 < COINS)
				ent.customShader = cgs.media.coins_pic;
			else if(cent->currentState.time2 < BILLS)
				ent.customShader = cgs.media.bills_pic;
		}
#endif
		ent.shaderRGBA[0] = 255;
		ent.shaderRGBA[1] = 255;
		ent.shaderRGBA[2] = 255;
		ent.shaderRGBA[3] = 255;
		trap_R_AddRefEntityToScene(&ent);
		return;
	}

	// items bob up and down continuously
#ifndef SMOKINGUNS
	scale = 0.005 + cent->currentState.number * 0.00001;
	cent->lerpOrigin[2] += 4 + cos( ( cg.time + 1000 ) *  scale ) * 4;
#endif

	memset (&ent, 0, sizeof(ent));

#ifndef SMOKINGUNS
	// autorotate at one of two speeds
	if ( item->giType == IT_HEALTH ) {
		VectorCopy( cg.autoAnglesFast, cent->lerpAngles );
		AxisCopy( cg.autoAxisFast, ent.axis );
	} else {
		VectorCopy( cg.autoAngles, cent->lerpAngles );
		AxisCopy( cg.autoAxis, ent.axis );
	}
#else
	//angle assignment, new by Spoon
	AnglesToAxis( cent->lerpAngles, ent.axis );
#endif

	wi = NULL;
	// the weapons have their origin where they attatch to player
	// models, so we need to offset them or they will rotate
	// eccentricly
	if ( item->giType == IT_WEAPON ) {
#ifdef SMOKINGUNS
		//make the dynamite a bit smaller
		if(item->giTag == WP_DYNAMITE){
			VectorScale(ent.axis[0], 0.7f, ent.axis[0]);
			VectorScale(ent.axis[1], 0.7f, ent.axis[1]);
			VectorScale(ent.axis[2], 0.7f, ent.axis[2]);
		} else if(item->giTag == WP_MOLOTOV){
			VectorScale(ent.axis[0], 0.65f, ent.axis[0]);
			VectorScale(ent.axis[1], 0.65f, ent.axis[1]);
			VectorScale(ent.axis[2], 0.65f, ent.axis[2]);
		}
#endif
		wi = &cg_weapons[item->giTag];
		cent->lerpOrigin[0] -=
			wi->weaponMidpoint[0] * ent.axis[0][0] +
			wi->weaponMidpoint[1] * ent.axis[1][0] +
			wi->weaponMidpoint[2] * ent.axis[2][0];
		cent->lerpOrigin[1] -=
			wi->weaponMidpoint[0] * ent.axis[0][1] +
			wi->weaponMidpoint[1] * ent.axis[1][1] +
			wi->weaponMidpoint[2] * ent.axis[2][1];
		cent->lerpOrigin[2] -=
			wi->weaponMidpoint[0] * ent.axis[0][2] +
			wi->weaponMidpoint[1] * ent.axis[1][2] +
			wi->weaponMidpoint[2] * ent.axis[2][2];

#ifndef SMOKINGUNS
		cent->lerpOrigin[2] += 8;	// an extra height boost
#endif
	}

#ifndef SMOKINGUNS
	if( item->giType == IT_WEAPON && item->giTag == WP_RAILGUN ) {
		clientInfo_t *ci = &cgs.clientinfo[cg.snap->ps.clientNum];
		Byte4Copy( ci->c1RGBA, ent.shaderRGBA );
	}
#endif

	ent.hModel = cg_items[es->modelindex].models[0];
#ifdef SMOKINGUNS
	if(!Q_stricmp(item->classname, "pickup_money")){
		if(cent->currentState.time2 < COINS)
			ent.hModel = cgs.media.coins;
		else if(cent->currentState.time2 < BILLS)
			ent.hModel = cgs.media.bills;
	}
#endif

	VectorCopy( cent->lerpOrigin, ent.origin);
	VectorCopy( cent->lerpOrigin, ent.oldorigin);

	ent.nonNormalizedAxes = qfalse;

	// if just respawned, slowly scale up
	msec = cg.time - cent->miscTime;
	if ( msec >= 0 && msec < ITEM_SCALEUP_TIME ) {
		frac = (float)msec / ITEM_SCALEUP_TIME;
		VectorScale( ent.axis[0], frac, ent.axis[0] );
		VectorScale( ent.axis[1], frac, ent.axis[1] );
		VectorScale( ent.axis[2], frac, ent.axis[2] );
		ent.nonNormalizedAxes = qtrue;
	} else {
		frac = 1.0;
	}

	// items without glow textures need to keep a minimum light value
	// so they are always visible
	if ( ( item->giType == IT_WEAPON ) ||
		 ( item->giType == IT_ARMOR ) ) {
		ent.renderfx |= RF_MINLIGHT;
	}

	// increase the size of the weapons when they are presented as items
	if ( item->giType == IT_WEAPON ) {
		VectorScale( ent.axis[0], 1.5, ent.axis[0] );
		VectorScale( ent.axis[1], 1.5, ent.axis[1] );
		VectorScale( ent.axis[2], 1.5, ent.axis[2] );
		ent.nonNormalizedAxes = qtrue;
#ifndef SMOKINGUNS
		trap_S_AddLoopingSound( cent->currentState.number, cent->lerpOrigin, vec3_origin, cgs.media.weaponHoverSound );
#endif
	}

#ifndef SMOKINGUNS
	if ( item->giType == IT_HOLDABLE && item->giTag == HI_KAMIKAZE ) {
		VectorScale( ent.axis[0], 2, ent.axis[0] );
		VectorScale( ent.axis[1], 2, ent.axis[1] );
		VectorScale( ent.axis[2], 2, ent.axis[2] );
		ent.nonNormalizedAxes = qtrue;
	}
#endif

	// add to refresh list
	trap_R_AddRefEntityToScene(&ent);

#ifdef SMOKINGUNS
	if ( item->giType == IT_WEAPON && wi->barrelModel ) {
		refEntity_t	barrel;

		memset( &barrel, 0, sizeof( barrel ) );

		barrel.hModel = wi->barrelModel;

		VectorCopy( ent.lightingOrigin, barrel.lightingOrigin );
		barrel.shadowPlane = ent.shadowPlane;
		barrel.renderfx = ent.renderfx;

		CG_PositionRotatedEntityOnTag( &barrel, &ent, wi->weaponModel, "tag_barrel" );

		AxisCopy( ent.axis, barrel.axis );
		barrel.nonNormalizedAxes = ent.nonNormalizedAxes;

		trap_R_AddRefEntityToScene( &barrel );
	}
#endif

	// accompanying rings / spheres for powerups
	if ( !cg_simpleItems.integer )
	{
		vec3_t spinAngles;

		VectorClear( spinAngles );

		if ( item->giType == IT_HEALTH || item->giType == IT_POWERUP )
		{
			if ( ( ent.hModel = cg_items[es->modelindex].models[1] ) != 0 )
			{
				if ( item->giType == IT_POWERUP )
				{
					ent.origin[2] += 12;
					spinAngles[1] = ( cg.time & 1023 ) * 360 / -1024.0f;
				}
				AnglesToAxis( spinAngles, ent.axis );

				// scale up if respawning
				if ( frac != 1.0 ) {
					VectorScale( ent.axis[0], frac, ent.axis[0] );
					VectorScale( ent.axis[1], frac, ent.axis[1] );
					VectorScale( ent.axis[2], frac, ent.axis[2] );
					ent.nonNormalizedAxes = qtrue;
				}
				trap_R_AddRefEntityToScene( &ent );
			}
		}
	}
}

/*
===============
CG_Missile
===============
*/
static void CG_Missile( centity_t *cent ) {
	refEntity_t			ent;
	entityState_t		*s1;
	const weaponInfo_t		*weapon;
//	int	col;

	s1 = &cent->currentState;
	if ( s1->weapon >= WP_NUM_WEAPONS ) {
		s1->weapon = 0;
	}
	weapon = &cg_weapons[s1->weapon];

	// calculate the axis
	VectorCopy( s1->angles, cent->lerpAngles);

	// add trails
	if ( weapon->missileTrailFunc )
	{
		weapon->missileTrailFunc( cent, weapon );
	}
/*
	if ( cent->currentState.modelindex == TEAM_RED ) {
		col = 1;
	}
	else if ( cent->currentState.modelindex == TEAM_BLUE ) {
		col = 2;
	}
	else {
		col = 0;
	}

	// add dynamic light
	if ( weapon->missileDlight ) {
		trap_R_AddLightToScene(cent->lerpOrigin, weapon->missileDlight, 
			weapon->missileDlightColor[col][0], weapon->missileDlightColor[col][1], weapon->missileDlightColor[col][2] );
	}
*/

#ifdef SMOKINGUNS
	// draw nothing if this is an alcohol drop(already predicted)
	if(s1->weapon == WP_MOLOTOV && s1->apos.trDelta[1]){
		return;
	}
#endif

	// add dynamic light
#ifndef SMOKINGUNS
	if ( weapon->missileDlight ) {
#else
	if ( weapon->missileDlight && s1->weapon != WP_DYNAMITE && s1->weapon != WP_MOLOTOV) {
#endif
		trap_R_AddLightToScene(cent->lerpOrigin, weapon->missileDlight,
			weapon->missileDlightColor[0], weapon->missileDlightColor[1], weapon->missileDlightColor[2] );
	}

	// add missile sound
#ifndef SMOKINGUNS
	if ( weapon->missileSound ) {
#else
	if ( weapon->missileSound && (s1->weapon != WP_DYNAMITE  ||
		(s1->weapon == WP_DYNAMITE && (int)s1->apos.trDelta[0]))) {
#endif
		vec3_t	velocity;

		BG_EvaluateTrajectoryDelta( &cent->currentState.pos, cg.time, velocity );

		trap_S_AddLoopingSound( cent->currentState.number, cent->lerpOrigin, velocity, weapon->missileSound );
	}

	// create the render entity
	memset (&ent, 0, sizeof(ent));
	VectorCopy( cent->lerpOrigin, ent.origin);
	VectorCopy( cent->lerpOrigin, ent.oldorigin);

#ifndef SMOKINGUNS
	if ( cent->currentState.weapon == WP_PLASMAGUN ) {
		ent.reType = RT_SPRITE;
		ent.radius = 16;
		ent.rotation = 0;
		ent.customShader = cgs.media.plasmaBallShader;
		trap_R_AddRefEntityToScene( &ent );
		return;
	}
#endif

	// flicker between two skins
	ent.skinNum = cg.clientFrame & 1;
	ent.hModel = weapon->missileModel;
	ent.renderfx = weapon->missileRenderfx | RF_NOSHADOW;

#ifndef SMOKINGUNS
	if ( cent->currentState.weapon == WP_PROX_LAUNCHER ) {
		if (s1->generic1 == TEAM_BLUE) {
			ent.hModel = cgs.media.blueProxMine;
		}
	}
#endif

	// convert direction of travel into axis
	if ( VectorNormalize2( s1->pos.trDelta, ent.axis[0] ) == 0 ) {
		ent.axis[0][2] = 1;
	}

	// spin as it moves
	if ( s1->pos.trType != TR_STATIONARY ) {
#ifndef SMOKINGUNS
		RotateAroundDirection( ent.axis, cg.time / 4 );
#else
		RotateAroundDirection( ent.axis, cg.time / 6 );
#endif
	} else {
#ifndef SMOKINGUNS
		if ( s1->weapon == WP_PROX_LAUNCHER ) {
			AnglesToAxis( cent->lerpAngles, ent.axis );
		}
		else
#endif
		{
			RotateAroundDirection( ent.axis, s1->time );
		}
	}

#ifdef SMOKINGUNS
	if(s1->weapon == WP_DYNAMITE && (int)s1->apos.trDelta[0]){
		const int time	= BG_AnimLength(WP_ANIM_SPECIAL, WP_DYNAMITE);
		const int num	= bg_weaponlist[WP_DYNAMITE].animations[WP_ANIM_SPECIAL].numFrames;
		const float delta = cg.time - s1->apos.trDelta[0] + time;
		const float pos = delta/(time/num);

		//animation
		ent.frame = (int)pos;
		if((float)ent.frame < pos)
			ent.frame++;

		if(ent.frame > num){
			return;
		}

		ent.oldframe = ent.frame -1;
		ent.backlerp = ent.frame - pos;
	}
#endif

	// add to refresh list, possibly with quad glow
	CG_AddRefEntityWithPowerups( &ent, s1, TEAM_FREE );

#ifdef SMOKINGUNS
	//add the sparks to the dynamite
	if(s1->weapon == WP_DYNAMITE && (int)s1->apos.trDelta[0]){
		refEntity_t		sparks;
		vec3_t			origin;

		memset( &sparks, 0, sizeof( sparks ) );
		sparks.reType = RT_SPRITE;
		sparks.customShader = cgs.media.wqfx_sparks;
		sparks.radius = 20;
		sparks.renderfx = 0;

		AnglesToAxis( vec3_origin, sparks.axis );

		CG_PositionRotatedEntityOnTag( &sparks, &ent, ent.hModel, "tag_sparks");
		trap_R_AddRefEntityToScene( &sparks );

		VectorCopy(sparks.origin, origin);

		origin[2] += 20;

		// add dynamic light
		trap_R_AddLightToScene(origin, weapon->missileDlight,
			weapon->missileDlightColor[0], weapon->missileDlightColor[1], weapon->missileDlightColor[2] );
	}

	if(s1->weapon == WP_MOLOTOV && s1->apos.trDelta[0] != 0.0f){
		refEntity_t		fire;
		vec3_t			origin;

		memset( &fire, 0, sizeof( fire ) );
		fire.reType = RT_SPRITE;
		fire.customShader = cgs.media.wqfx_matchfire;
		fire.radius = 2;
		fire.renderfx = 0;

		AnglesToAxis( vec3_origin, fire.axis );

		CG_PositionRotatedEntityOnTag( &fire, &ent, ent.hModel, "tag_fire");
		trap_R_AddRefEntityToScene( &fire );

		VectorCopy(fire.origin, origin);

		origin[2] += 20;
		trap_R_AddLightToScene(origin, weapon->missileDlight,
			weapon->missileDlightColor[0], weapon->missileDlightColor[1], weapon->missileDlightColor[2] );
	}
#endif
}

/*
===============
CG_Grapple

This is called when the grapple is sitting up against the wall
===============
*/
#ifndef SMOKINGUNS
static void CG_Grapple( centity_t *cent ) {
	refEntity_t			ent;
	entityState_t		*s1;
	const weaponInfo_t		*weapon;

	s1 = &cent->currentState;
	if ( s1->weapon >= WP_NUM_WEAPONS ) {
		s1->weapon = 0;
	}
	weapon = &cg_weapons[s1->weapon];

	// calculate the axis
	VectorCopy( s1->angles, cent->lerpAngles);

#if 0 // FIXME add grapple pull sound here..?
	// add missile sound
	if ( weapon->missileSound ) {
		trap_S_AddLoopingSound( cent->currentState.number, cent->lerpOrigin, vec3_origin, weapon->missileSound );
	}
#endif

	// Will draw cable if needed
	CG_GrappleTrail ( cent, weapon );

	// create the render entity
	memset (&ent, 0, sizeof(ent));
	VectorCopy( cent->lerpOrigin, ent.origin);
	VectorCopy( cent->lerpOrigin, ent.oldorigin);

	// flicker between two skins
	ent.skinNum = cg.clientFrame & 1;
	ent.hModel = weapon->missileModel;
	ent.renderfx = weapon->missileRenderfx | RF_NOSHADOW;

	// convert direction of travel into axis
	if ( VectorNormalize2( s1->pos.trDelta, ent.axis[0] ) == 0 ) {
		ent.axis[0][2] = 1;
	}

	trap_R_AddRefEntityToScene( &ent );
}
#endif

#ifdef SMOKINGUNS
/*QUAKED func_smoke (0 .5 .8) ?
Generates Smoke
-------- KEYS --------
size : size of the smoke particles (default 50) (time)
trans : transparency of the smoke (default 0.8, max 1.0) (angles[0])
color : color of the smoke ( default 1 1 1) (angles2)
lifetime : lifetime of the smoke (default 5000 (msecs)) (time2)
rate : generation rate of the smoke-sprites (default 3 (sprites per second)) (frame)
target : sets the direction of the wind (direction: apos.trBase)
wind : strength of the wind (default 5) (torsoAnim)
boost : rise-up-velocity of the sprites (default 5) (legsAnim)
*/
/*
===============
CG_Smoke
by: SPoon
24/9/2002
Generates Smoke
===============
*/
#define SMOKE_SIZE 50
static void CG_Smoke( centity_t *cent){
	// initalizations
	const int size = cent->currentState.time;
	const float trans = cent->currentState.angles[0];
	const int lifetime = cent->currentState.time2;
	const int rate = cent->currentState.frame;
	const int wind = cent->currentState.torsoAnim;
	const int boost = cent->currentState.legsAnim*2;
	vec3_t winddir;
	vec3_t origin, maxs, mins, len;
	int	x_num, y_num;
	float x_dist, y_dist;
	int x, y;
	vec4_t color;

	// see if we have to generate sprites this time
	if(!rate || cent->lastSmokeTime + (float)(1000/rate) > cg.time)
		return;

	VectorCopy(cent->currentState.apos.trBase, winddir);
	color[0] = cent->currentState.angles2[0] * 255;
	color[1] = cent->currentState.angles2[1] * 255;
	color[2] = cent->currentState.angles2[2] * 255;
	color[3] = trans * 255;

	// First get position infos
	VectorCopy(cent->currentState.apos.trDelta, maxs);
	VectorCopy(cent->currentState.pos.trDelta, mins);
	VectorSubtract(maxs, mins, len);

	if( (float)(len[0]/SMOKE_SIZE) > (int)(len[0]/SMOKE_SIZE))
		x_num = (int)(len[0]/SMOKE_SIZE)+1;
	else
		x_num = (int)(len[0]/SMOKE_SIZE);

	if( (float)(len[1]/SMOKE_SIZE) > (int)(len[1]/SMOKE_SIZE))
		y_num = (int)(len[1]/SMOKE_SIZE)+1;
	else
		y_num = (int)(len[1]/SMOKE_SIZE);

	x_num -= 1; if(x_num <= 0) x_num = 1;
	y_num -= 1; if(y_num <= 0) y_num = 1;

	x_dist = len[0]/x_num;
	y_dist = len[1]/y_num;

	// begin generating the sprites
	for(x = 0; x < x_num; x++){
		float random;

		VectorCopy(mins, origin);
		origin[0] += x*x_dist;

		// add random factor
		random = ((float)(rand()%(int)(x_dist*10)))/10;
		origin[0] += random;

		for(y = 0; y < y_num; y++){
			localEntity_t *le;
			origin[1] = mins[1] + y*y_dist;

			// add a random factor
			random = ((float)(rand()%(int)(y_dist*10)))/10;
			origin[1] += random;

			le = CG_AddSmokeParticle(origin, size, size/4, lifetime, boost, wind,
				winddir);
			Vector4Copy(color, le->color);
			le->refEntity.customShader = cgs.media.ent_smoke;
		}
	}

	cent->lastSmokeTime = cg.time;
}

/*
===============
CG_Flare
by: Spoon
28.6.2001
Generates a flare
===============
*/
#define NO_GLOW	1
#define	NO_LENSFLARES	2
static void CG_Flare( centity_t *cent ){
	refEntity_t			ent;
	entityState_t		*s1;

	s1 = &cent->currentState;

	// create the render entity
	memset (&ent, 0, sizeof(ent));
	VectorCopy( cent->lerpOrigin, ent.origin);
	VectorCopy( cent->lerpOrigin, ent.oldorigin);
	AnglesToAxis( cent->lerpAngles, ent.axis );

	ent.renderfx = RF_NOSHADOW;

	// flicker between two skins (FIXME?)
	ent.skinNum = ( cg.time >> 6 ) & 1;

	// get the model, either as a bmodel or a modelindex
	if ( s1->solid == SOLID_BMODEL ) {
		ent.hModel = cgs.inlineDrawModel[s1->modelindex];
	} else {
		ent.hModel = cgs.gameModels[s1->modelindex];
	}

	// add to refresh list
	trap_R_AddRefEntityToScene(&ent);

	// add the flare
	if(cgs.media.flare){
		float	distance;
		trace_t	trace;

		if (!cg_glowflares.integer)
			return;

		if (cg_boostfps.integer) {
// FPS Optimization ?
			if (!CG_IsEntityVisible(cent, 0))
				return;
			// Here, the flare is visible

			// Ignore all solids that are farther than the flare.
			CG_Trace_Flare( &trace, cg.refdef.vieworg, vec3_origin, vec3_origin, s1->origin,
				cg.snap->ps.clientNum, MASK_SHOT, cent->distFromCamera );
// FPS Optimization ?
		}
		else {
			CG_BuildSolidList(qtrue);

			CG_Trace( &trace, cg.refdef.vieworg, vec3_origin, vec3_origin, cent->currentState.origin,
				cg.snap->ps.clientNum, MASK_SHOT );

			CG_BuildSolidList(qfalse);
		}

		if(trace.entityNum != cent->currentState.number)
			return;

		distance = Distance(s1->origin, cg.refdef.vieworg);

		if(!(cent->currentState.eFlags & NO_GLOW)){
			refEntity_t	ent2;

			memset( &ent2, 0, sizeof( ent2 ) );
			VectorCopy( s1->origin, ent2.origin );
			ent2.reType = RT_SPRITE;
			ent2.customShader = cgs.media.flare;

			ent2.radius = distance/(64/s1->frame);

			if(ent2.radius < s1->time2)
				ent2.radius = s1->time2;
			else if(ent2.radius > s1->time)
				ent2.radius = s1->time;

			ent2.rotation = cg.refdefViewAngles[YAW];
			ent2.shaderRGBA[0] = 255*s1->angles2[0];
			ent2.shaderRGBA[1] = 255*s1->angles2[1];
			ent2.shaderRGBA[2] = 255*s1->angles2[2];
			ent2.shaderRGBA[3] = 255*s1->angles[0];
			trap_R_AddRefEntityToScene( &ent2 );
		}
	}
}
#endif

/*
===============
CG_Mover
===============
*/
static void CG_Mover( centity_t *cent ) {
	refEntity_t		ent;
	entityState_t		*s1;
	
#ifdef SMOKINGUNS
	// Joe Kari: farclip_type and farclip_dist :
	
	int farclip_type = cent->currentState.powerups & 127 ;	// only 7 lower bits of powerups is used for farclipping
	int invert_clip = cent->currentState.powerups & 128 ;	// if it's a close clipping
	// fast multiply by 64:
	float farclip_dist = (float)( cent->currentState.legsAnim << 6 ) ;
	float farclip_alt_dist = (float)( cent->currentState.torsoAnim << 6 ) ;
	float farclip_factor ;
	
	if ( !cg_farclip.integer || farclip_dist < 1 )
	{
		farclip_type = FARCLIP_NONE ;
	}
	else
	{
		if ( cg.zoomed )  farclip_factor = cg_farclipZoomValue.value ;
		else  farclip_factor = cg_farclipValue.value ;
		if ( farclip_factor < 1 )  farclip_factor = 1 ;
		farclip_dist *= farclip_factor ;
		farclip_alt_dist *= farclip_factor ;
	}
#endif
	
	s1 = &cent->currentState;
	
	// create the render entity
	memset (&ent, 0, sizeof(ent));
	VectorCopy( cent->lerpOrigin, ent.origin);
	VectorCopy( cent->lerpOrigin, ent.oldorigin);
	AnglesToAxis( cent->lerpAngles, ent.axis );

	ent.renderfx = RF_NOSHADOW;
	
#ifdef SMOKINGUNS
	// Joe Kari: for func_static (yes there are classified as mover)
	// if a LOD is specified for this entity, then try to clip it !!!
	if ( cent->currentState.powerups & MAPLOD_BINARY_MASK ) {
		int objectLOD = ( cent->currentState.powerups >> 8 ) & 3 ;
		if ( cent->currentState.powerups & MAPLOD_GTE_BINARY_MASK ) {
			if ( cg_mapLOD.integer < objectLOD )  return;
		}
		else if ( cg_mapLOD.integer > objectLOD )  return;
	}
	
	// Joe Kari: for func_static
	// if a farclip is specified for this entity, then try to clip it !!!
	
	if ( ( farclip_type + invert_clip > FARCLIP_NONE ) && ( farclip_type < CG_Farclip_Tester_Table_Size ) )
	{
		if ( !invert_clip && CG_Farclip_Tester[farclip_type]( s1->origin , cg.refdef.vieworg , farclip_dist , farclip_alt_dist ) )  return;
		else if ( invert_clip && !CG_Farclip_Tester[farclip_type]( s1->origin , cg.refdef.vieworg , farclip_dist , farclip_alt_dist ) )  return;
	}
#endif
	
	// flicker between two skins (FIXME?)
	ent.skinNum = ( cg.time >> 6 ) & 1;
	
	// get the model, either as a bmodel or a modelindex
	if ( s1->solid == SOLID_BMODEL ) {
		ent.hModel = cgs.inlineDrawModel[s1->modelindex];
	} else {
		ent.hModel = cgs.gameModels[s1->modelindex];
	}

	// add to refresh list
	trap_R_AddRefEntityToScene(&ent);

	// add the secondary model
	if ( s1->modelindex2 ) {
		ent.skinNum = 0;
		ent.hModel = cgs.gameModels[s1->modelindex2];
		trap_R_AddRefEntityToScene(&ent);
	}

}

/*
===============
CG_Beam

Also called as an event
===============
*/
void CG_Beam( centity_t *cent ) {
	refEntity_t			ent;
	entityState_t		*s1;

	s1 = &cent->currentState;

	// create the render entity
	memset (&ent, 0, sizeof(ent));
	VectorCopy( s1->pos.trBase, ent.origin );
	VectorCopy( s1->origin2, ent.oldorigin );
	AxisClear( ent.axis );
	ent.reType = RT_BEAM;

	ent.renderfx = RF_NOSHADOW;

	// add to refresh list
	trap_R_AddRefEntityToScene(&ent);
}


/*
===============
CG_Portal
===============
*/
static void CG_Portal( centity_t *cent ) {
	refEntity_t			ent;
	entityState_t		*s1;

	s1 = &cent->currentState;

	// create the render entity
	memset (&ent, 0, sizeof(ent));
	VectorCopy( cent->lerpOrigin, ent.origin );
	VectorCopy( s1->origin2, ent.oldorigin );
	ByteToDir( s1->eventParm, ent.axis[0] );
	PerpendicularVector( ent.axis[1], ent.axis[0] );

	// negating this tends to get the directions like they want
	// we really should have a camera roll value
	VectorSubtract( vec3_origin, ent.axis[1], ent.axis[1] );

	CrossProduct( ent.axis[0], ent.axis[1], ent.axis[2] );
	ent.reType = RT_PORTALSURFACE;
	ent.oldframe = s1->powerups;
	ent.frame = s1->frame;		// rotation speed
	ent.skinNum = s1->clientNum/256.0 * 360;	// roll offset

	// add to refresh list
	trap_R_AddRefEntityToScene(&ent);
}


/*
=========================
CG_AdjustPositionForMover

Also called by client movement prediction code
=========================
*/
void CG_AdjustPositionForMover(const vec3_t in, int moverNum, int fromTime, int toTime, vec3_t out, vec3_t angles_in, vec3_t angles_out) {
	centity_t	*cent;
	vec3_t	oldOrigin, origin, deltaOrigin;
	vec3_t	oldAngles, angles, deltaAngles;

	if ( moverNum <= 0 || moverNum >= ENTITYNUM_MAX_NORMAL ) {
		VectorCopy( in, out );
		VectorCopy(angles_in, angles_out);
		return;
	}

	cent = &cg_entities[ moverNum ];
	if ( cent->currentState.eType != ET_MOVER ) {
		VectorCopy( in, out );
		VectorCopy(angles_in, angles_out);
		return;
	}

	BG_EvaluateTrajectory( &cent->currentState.pos, fromTime, oldOrigin );
	BG_EvaluateTrajectory( &cent->currentState.apos, fromTime, oldAngles );

	BG_EvaluateTrajectory( &cent->currentState.pos, toTime, origin );
	BG_EvaluateTrajectory( &cent->currentState.apos, toTime, angles );

	VectorSubtract( origin, oldOrigin, deltaOrigin );
	VectorSubtract( angles, oldAngles, deltaAngles );

	VectorAdd( in, deltaOrigin, out );
	VectorAdd( angles_in, deltaAngles, angles_out );
	// FIXME: origin change when on a rotating object
}


/*
=============================
CG_InterpolateEntityPosition
=============================
*/
static void CG_InterpolateEntityPosition( centity_t *cent ) {
	vec3_t		current, next;
	float		f;

	// it would be an internal error to find an entity that interpolates without
	// a snapshot ahead of the current one
	if ( cg.nextSnap == NULL ) {
		CG_Error( "CG_InterpoateEntityPosition: cg.nextSnap == NULL" );
	}

	f = cg.frameInterpolation;

	// this will linearize a sine or parabolic curve, but it is important
	// to not extrapolate player positions if more recent data is available
	BG_EvaluateTrajectory( &cent->currentState.pos, cg.snap->serverTime, current );
	BG_EvaluateTrajectory( &cent->nextState.pos, cg.nextSnap->serverTime, next );

	cent->lerpOrigin[0] = current[0] + f * ( next[0] - current[0] );
	cent->lerpOrigin[1] = current[1] + f * ( next[1] - current[1] );
	cent->lerpOrigin[2] = current[2] + f * ( next[2] - current[2] );

	BG_EvaluateTrajectory( &cent->currentState.apos, cg.snap->serverTime, current );
	BG_EvaluateTrajectory( &cent->nextState.apos, cg.nextSnap->serverTime, next );

	cent->lerpAngles[0] = LerpAngle( current[0], next[0], f );
	cent->lerpAngles[1] = LerpAngle( current[1], next[1], f );
	cent->lerpAngles[2] = LerpAngle( current[2], next[2], f );

}

/*
===============
CG_CalcEntityLerpPositions

===============
*/
static void CG_CalcEntityLerpPositions( centity_t *cent ) {
#ifndef SMOKINGUNS
	// if this player does not want to see extrapolated players
	if ( !cg_smoothClients.integer ) {
		// make sure the clients use TR_INTERPOLATE
		if ( cent->currentState.number < MAX_CLIENTS ) {
			cent->currentState.pos.trType = TR_INTERPOLATE;
			cent->nextState.pos.trType = TR_INTERPOLATE;
		}
	}
#else
//unlagged - projectile nudge
	// this will be set to how far forward projectiles will be extrapolated
	int timeshift = 0;
//unlagged - projectile nudge

//unlagged - smooth clients #2
	// this is done server-side now - cg_smoothClients is undefined
	// players will always be TR_INTERPOLATE
#endif
//unlagged - smooth clients #2

	if ( cent->interpolate && cent->currentState.pos.trType == TR_INTERPOLATE ) {
		CG_InterpolateEntityPosition( cent );
		return;
	}

	// first see if we can interpolate between two snaps for
	// linear extrapolated clients
	if ( cent->interpolate && cent->currentState.pos.trType == TR_LINEAR_STOP &&
											cent->currentState.number < MAX_CLIENTS) {
		CG_InterpolateEntityPosition( cent );
		return;
	}

#ifdef SMOKINGUNS
//unlagged - timenudge extrapolation
	// interpolating failed (probably no nextSnap), so extrapolate
	// this can also happen if the teleport bit is flipped, but that
	// won't be noticeable
	if ( cent->currentState.number < MAX_CLIENTS &&
			cent->currentState.clientNum != cg.predictedPlayerState.clientNum ) {
		cent->currentState.pos.trType = TR_LINEAR_STOP;
		cent->currentState.pos.trTime = cg.snap->serverTime;
		cent->currentState.pos.trDuration = 1000 / sv_fps.integer;
	}
//unlagged - timenudge extrapolation

//unlagged - projectile nudge
	// if it's a missile but not a grappling hook
	if ( cent->currentState.eType == ET_MISSILE /*&& cent->currentState.weapon != WP_GRAPPLING_HOOK*/ ) {
		// if it's one of ours
		if ( cent->currentState.otherEntityNum == cg.clientNum ) {
			// extrapolate one server frame's worth - this will correct for tiny
			// visual inconsistencies introduced by backward-reconciling all players
			// one server frame before running projectiles
			timeshift = 1000 / sv_fps.integer;
		}
		// if it's not, and it's not a grenade launcher
		//else if ( cent->currentState.weapon != WP_GRENADE_LAUNCHER ) {
		else if ( cent->currentState.weapon != WP_DYNAMITE && cent->currentState.weapon != WP_MOLOTOV) {
			// extrapolate based on cg_projectileNudge
			timeshift = cg_projectileNudge.integer + 1000 / sv_fps.integer;
		}
	}
#endif

	// just use the current frame and evaluate as best we can
#ifndef SMOKINGUNS
	BG_EvaluateTrajectory( &cent->currentState.pos, cg.time, cent->lerpOrigin );
	BG_EvaluateTrajectory( &cent->currentState.apos, cg.time, cent->lerpAngles );
#else
	BG_EvaluateTrajectory( &cent->currentState.pos, cg.time + timeshift, cent->lerpOrigin );
	BG_EvaluateTrajectory( &cent->currentState.apos, cg.time + timeshift, cent->lerpAngles );

	// if there's a time shift
	if ( timeshift != 0 ) {
		trace_t tr;
		vec3_t lastOrigin;

		BG_EvaluateTrajectory( &cent->currentState.pos, cg.time, lastOrigin );

		CG_Trace( &tr, lastOrigin, vec3_origin, vec3_origin, cent->lerpOrigin, cent->currentState.number, MASK_SHOT );

		// don't let the projectile go through the floor
		if ( tr.fraction < 1.0f ) {
			cent->lerpOrigin[0] = lastOrigin[0] + tr.fraction * ( cent->lerpOrigin[0] - lastOrigin[0] );
			cent->lerpOrigin[1] = lastOrigin[1] + tr.fraction * ( cent->lerpOrigin[1] - lastOrigin[1] );
			cent->lerpOrigin[2] = lastOrigin[2] + tr.fraction * ( cent->lerpOrigin[2] - lastOrigin[2] );
		}
	}
//unlagged - projectile nudge
#endif


	// adjust for riding a mover if it wasn't rolled into the predicted
	// player state
	if ( cent != &cg.predictedPlayerEntity ) {
		CG_AdjustPositionForMover( cent->lerpOrigin, cent->currentState.groundEntityNum, 
		cg.snap->serverTime, cg.time, cent->lerpOrigin, cent->lerpAngles, cent->lerpAngles);
	}
}

/*
===============
CG_TeamBase
===============
*/
#ifndef SMOKINGUNS
static void CG_TeamBase( centity_t *cent ) {
	refEntity_t model;
#ifdef MISSIONPACK
	vec3_t angles;
	int t, h;
	float c;

	if ( cgs.gametype == GT_CTF || cgs.gametype == GT_1FCTF ) {
#else
	if ( cgs.gametype == GT_CTF) {
#endif
		// show the flag base
		memset(&model, 0, sizeof(model));
		model.reType = RT_MODEL;
		VectorCopy( cent->lerpOrigin, model.lightingOrigin );
		VectorCopy( cent->lerpOrigin, model.origin );
		AnglesToAxis( cent->currentState.angles, model.axis );
		if ( cent->currentState.modelindex == TEAM_RED ) {
			model.hModel = cgs.media.redFlagBaseModel;
		}
		else if ( cent->currentState.modelindex == TEAM_BLUE ) {
			model.hModel = cgs.media.blueFlagBaseModel;
		}
		else {
			model.hModel = cgs.media.neutralFlagBaseModel;
		}
		trap_R_AddRefEntityToScene( &model );
	}
#ifdef MISSIONPACK
	else if ( cgs.gametype == GT_OBELISK ) {
		// show the obelisk
		memset(&model, 0, sizeof(model));
		model.reType = RT_MODEL;
		VectorCopy( cent->lerpOrigin, model.lightingOrigin );
		VectorCopy( cent->lerpOrigin, model.origin );
		AnglesToAxis( cent->currentState.angles, model.axis );

		model.hModel = cgs.media.overloadBaseModel;
		trap_R_AddRefEntityToScene( &model );
		// if hit
		if ( cent->currentState.frame == 1) {
			// show hit model
			// modelindex2 is the health value of the obelisk
			c = cent->currentState.modelindex2;
			model.shaderRGBA[0] = 0xff;
			model.shaderRGBA[1] = c;
			model.shaderRGBA[2] = c;
			model.shaderRGBA[3] = 0xff;
			//
			model.hModel = cgs.media.overloadEnergyModel;
			trap_R_AddRefEntityToScene( &model );
		}
		// if respawning
		if ( cent->currentState.frame == 2) {
			if ( !cent->miscTime ) {
				cent->miscTime = cg.time;
			}
			t = cg.time - cent->miscTime;
			h = (cg_obeliskRespawnDelay.integer - 5) * 1000;
			//
			if (t > h) {
				c = (float) (t - h) / h;
				if (c > 1)
					c = 1;
			}
			else {
				c = 0;
			}
			// show the lights
			AnglesToAxis( cent->currentState.angles, model.axis );
			//
			model.shaderRGBA[0] = c * 0xff;
			model.shaderRGBA[1] = c * 0xff;
			model.shaderRGBA[2] = c * 0xff;
			model.shaderRGBA[3] = c * 0xff;

			model.hModel = cgs.media.overloadLightsModel;
			trap_R_AddRefEntityToScene( &model );
			// show the target
			if (t > h) {
				if ( !cent->muzzleFlashTime ) {
					trap_S_StartSound (cent->lerpOrigin, ENTITYNUM_NONE, CHAN_BODY,  cgs.media.obeliskRespawnSound);
					cent->muzzleFlashTime = 1;
				}
				VectorCopy(cent->currentState.angles, angles);
				angles[YAW] += (float) 16 * acos(1-c) * 180 / M_PI;
				AnglesToAxis( angles, model.axis );

				VectorScale( model.axis[0], c, model.axis[0]);
				VectorScale( model.axis[1], c, model.axis[1]);
				VectorScale( model.axis[2], c, model.axis[2]);

				model.shaderRGBA[0] = 0xff;
				model.shaderRGBA[1] = 0xff;
				model.shaderRGBA[2] = 0xff;
				model.shaderRGBA[3] = 0xff;
				//
				model.origin[2] += 56;
				model.hModel = cgs.media.overloadTargetModel;
				trap_R_AddRefEntityToScene( &model );
			}
			else {
				//FIXME: show animated smoke
			}
		}
		else {
			cent->miscTime = 0;
			cent->muzzleFlashTime = 0;
			// modelindex2 is the health value of the obelisk
			c = cent->currentState.modelindex2;
			model.shaderRGBA[0] = 0xff;
			model.shaderRGBA[1] = c;
			model.shaderRGBA[2] = c;
			model.shaderRGBA[3] = 0xff;
			// show the lights
			model.hModel = cgs.media.overloadLightsModel;
			trap_R_AddRefEntityToScene( &model );
			// show the target
			model.origin[2] += 56;
			model.hModel = cgs.media.overloadTargetModel;
			trap_R_AddRefEntityToScene( &model );
		}
	}
	else if ( cgs.gametype == GT_HARVESTER ) {
		// show harvester model
		memset(&model, 0, sizeof(model));
		model.reType = RT_MODEL;
		VectorCopy( cent->lerpOrigin, model.lightingOrigin );
		VectorCopy( cent->lerpOrigin, model.origin );
		AnglesToAxis( cent->currentState.angles, model.axis );

		if ( cent->currentState.modelindex == TEAM_RED ) {
			model.hModel = cgs.media.harvesterModel;
			model.customSkin = cgs.media.harvesterRedSkin;
		}
		else if ( cent->currentState.modelindex == TEAM_BLUE ) {
			model.hModel = cgs.media.harvesterModel;
			model.customSkin = cgs.media.harvesterBlueSkin;
		}
		else {
			model.hModel = cgs.media.harvesterNeutralModel;
			model.customSkin = 0;
		}
		trap_R_AddRefEntityToScene( &model );
	}
#endif
}
#endif

/*
===============
CG_Intermission
===============
*/
#ifdef SMOKINGUNS
static void CG_Intermission( centity_t *cent){

	if(cgs.gametype != GT_DUEL)
		return;

	VectorCopy(cent->lerpOrigin,
		cg_intermission_origin[cent->currentState.eventParm-1]);

	VectorCopy(cent->currentState.angles,
		cg_intermission_angles[cent->currentState.eventParm-1]);

	//CG_Printf("%f %f %f\n", cg_entities[i].lerpOrigin[0], cg_entities[i].lerpOrigin[1],
	//	cg_entities[i].lerpOrigin[2]);
}
#endif

/*
===============
CG_AddCEntity

===============
*/
static void CG_AddCEntity( centity_t *cent ) {
	// event-only entities will have been dealt with already
	if ( cent->currentState.eType >= ET_EVENTS ) {
		return;
	}

	// calculate the current origin
	CG_CalcEntityLerpPositions( cent );

	// add automatic effects
	CG_EntityEffects( cent );

	switch ( cent->currentState.eType ) {
	default:
		CG_Error( "Bad entity type: %i", cent->currentState.eType );
		break;
#ifdef SMOKINGUNS
	case ET_FLY:
		CG_Fly(cent);
		break;
#endif
	case ET_INVISIBLE:
	case ET_PUSH_TRIGGER:
	case ET_TELEPORT_TRIGGER:
		break;
	case ET_GENERAL:
		CG_General( cent );
		break;
	case ET_PLAYER:
		CG_Player( cent );
		break;
	case ET_ITEM:
		CG_Item( cent );
		break;
	case ET_MISSILE:
		CG_Missile( cent );
		break;
	case ET_MOVER:
#ifdef SMOKINGUNS
	case ET_BREAKABLE:
#endif
		CG_Mover( cent );
		break;
	case ET_BEAM:
		CG_Beam( cent );
		break;
	case ET_PORTAL:
		CG_Portal( cent );
		break;
	case ET_SPEAKER:
		CG_Speaker( cent );
		break;
#ifndef SMOKINGUNS
	case ET_GRAPPLE:
		CG_Grapple( cent );
		break;
	case ET_TEAM:
		CG_TeamBase( cent );
		break;
#else
		break;
	//gatling gun
	case ET_TURRET:
		CG_Turret( cent );
		break;
	case ET_FLARE:
		CG_Flare( cent);
		break;
	case ET_SMOKE:
		CG_Smoke( cent);
		break;
	case ET_ESCAPE:
		VectorCopy(cent->currentState.angles2, cg.escapePoint);
		break;
	case ET_INTERMISSION:
		CG_Intermission (cent);
		break;
#endif
	}
#ifdef SMOKINGUNS
	CG_AddBoundingBoxEntity( cent );
#endif
}

/*
===============
CG_AddPacketEntities

===============
*/
void CG_AddPacketEntities( void ) {
	int					num;
	centity_t			*cent;
	playerState_t		*ps;

	// set cg.frameInterpolation
	if ( cg.nextSnap ) {
		int		delta;

		delta = (cg.nextSnap->serverTime - cg.snap->serverTime);
		if ( delta == 0 ) {
			cg.frameInterpolation = 0;
		} else {
			cg.frameInterpolation = (float)( cg.time - cg.snap->serverTime ) / delta;
		}
	} else {
		cg.frameInterpolation = 0;	// actually, it should never be used, because
									// no entities should be marked as interpolating
	}

	// the auto-rotating items will all have the same axis
	cg.autoAngles[0] = 0;
	cg.autoAngles[1] = ( cg.time & 2047 ) * 360 / 2048.0;
	cg.autoAngles[2] = 0;

	cg.autoAnglesFast[0] = 0;
	cg.autoAnglesFast[1] = ( cg.time & 1023 ) * 360 / 1024.0f;
	cg.autoAnglesFast[2] = 0;

	AnglesToAxis( cg.autoAngles, cg.autoAxis );
	AnglesToAxis( cg.autoAnglesFast, cg.autoAxisFast );

	// generate and add the entity from the playerstate
	ps = &cg.predictedPlayerState;
	BG_PlayerStateToEntityState( ps, &cg.predictedPlayerEntity.currentState, qfalse );
	CG_AddCEntity( &cg.predictedPlayerEntity );

	// lerp the non-predicted value for lightning gun origins
	CG_CalcEntityLerpPositions( &cg_entities[ cg.snap->ps.clientNum ] );

#ifdef SMOKINGUNS
//unlagged - early transitioning
	if ( cg.nextSnap ) {
		// pre-add some of the entities sent over by the server
		// we have data for them and they don't need to interpolate
		for ( num = 0 ; num < cg.nextSnap->numEntities ; num++ ) {
			cent = &cg_entities[ cg.nextSnap->entities[ num ].number ];
			if ( cent->nextState.eType == ET_MISSILE || cent->nextState.eType == ET_GENERAL ) {
				// transition it immediately and add it
				CG_TransitionEntity( cent );
				cent->interpolate = qtrue;
				CG_AddCEntity( cent );
			}
		}
	}
//unlagged - early transitioning
#endif

	// add each entity sent over by the server
	for ( num = 0 ; num < cg.snap->numEntities ; num++ ) {
		cent = &cg_entities[ cg.snap->entities[ num ].number ];
#ifdef SMOKINGUNS
//unlagged - early transitioning
		if ( !cg.nextSnap || (cent->nextState.eType != ET_MISSILE && cent->nextState.eType != ET_GENERAL) ) {
//unlagged - early transitioning
			CG_AddCEntity( cent );
		}
#else
		CG_AddCEntity( cent );
#endif
	}
}

