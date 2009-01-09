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
// cg_weapons.c -- events and effects dealing with weapons
#include "cg_local.h"

/*
==========================
CG_MachineGunEjectBrass
==========================
*/
static void CG_MachineGunEjectBrass( centity_t *cent ) {
	localEntity_t	*le;
	refEntity_t		*re;
	vec3_t			velocity, xvelocity;
	vec3_t			offset, xoffset;
	float			waterScale = 1.0f;
	vec3_t			v[3];
	vec3_t			angles;

	if ( cg_brassTime.integer <= 0 ) {
		return;
	}

	le = CG_AllocLocalEntity();
	re = &le->refEntity;

	velocity[0] = 0;
	velocity[1] = -50 + 40 * crandom();
	velocity[2] = 0;//100 + 50 * crandom();

	le->leType = LE_FRAGMENT;
	le->startTime = cg.time;
	le->endTime = le->startTime + cg_brassTime.integer + ( cg_brassTime.integer / 4 ) * random();

	le->pos.trType = TR_GRAVITY;
	le->pos.trTime = cg.time - (rand()&15);

	AnglesToAxis( cent->lerpAngles, v );

	offset[0] = 20;
	offset[1] = 0;
	offset[2] = 8;

	xoffset[0] = offset[0] * v[0][0] + offset[1] * v[1][0] + offset[2] * v[2][0];
	xoffset[1] = offset[0] * v[0][1] + offset[1] * v[1][1] + offset[2] * v[2][1];
	xoffset[2] = offset[0] * v[0][2] + offset[1] * v[1][2] + offset[2] * v[2][2];
	VectorAdd( cent->lerpOrigin, xoffset, re->origin );

	VectorCopy( re->origin, le->pos.trBase );

	if ( CG_PointContents( re->origin, -1 ) & CONTENTS_WATER ) {
		waterScale = 0.10f;
	}

	xvelocity[0] = velocity[0] * v[0][0] + velocity[1] * v[1][0] + velocity[2] * v[2][0];
	xvelocity[1] = velocity[0] * v[0][1] + velocity[1] * v[1][1] + velocity[2] * v[2][1];
	xvelocity[2] = velocity[0] * v[0][2] + velocity[1] * v[1][2] + velocity[2] * v[2][2];
	VectorScale( xvelocity, waterScale, le->pos.trDelta );

	VectorClear(angles);
	angles[YAW] = 90;

	AnglesToAxis(angles, re->axis);

	//AxisCopy( axisDefault, re->axis );
	re->hModel = cgs.media.machinegunBrassModel;

	le->bounceFactor = 0.4 * waterScale;

	le->angles.trType = TR_LINEAR;
	le->angles.trTime = cg.time;
	le->angles.trBase[0] = rand()&31;
	le->angles.trBase[1] = rand()&31;
	le->angles.trBase[2] = rand()&31;
	le->angles.trDelta[0] = 2;
	le->angles.trDelta[1] = 1;
	le->angles.trDelta[2] = 0;

	le->leFlags = LEF_TUMBLE;
	le->leBounceSoundType = LEBS_BRASS;
	le->leMarkType = LEMT_NONE;
}

/*
==========================
CG_ShotgunEjectBrass
==========================
*/
static void CG_ShotgunEjectBrass( centity_t *cent ) {
	localEntity_t	*le;
	refEntity_t		*re;
	vec3_t			velocity, xvelocity;
	vec3_t			offset, xoffset;
	vec3_t			v[3];
	int				i;

	if ( cg_brassTime.integer <= 0 ) {
		return;
	}

	for ( i = 0; i < 2; i++ ) {
		float	waterScale = 1.0f;

		le = CG_AllocLocalEntity();
		re = &le->refEntity;

		velocity[0] = 60 + 60 * crandom();
		if ( i == 0 ) {
			velocity[1] = 40 + 10 * crandom();
		} else {
			velocity[1] = -40 + 10 * crandom();
		}
		velocity[2] = 100 + 50 * crandom();

		le->leType = LE_FRAGMENT;
		le->startTime = cg.time;
		le->endTime = le->startTime + cg_brassTime.integer*3 + cg_brassTime.integer * random();

		le->pos.trType = TR_GRAVITY;
		le->pos.trTime = cg.time;

		AnglesToAxis( cent->lerpAngles, v );

		offset[0] = 8;
		offset[1] = 0;
		offset[2] = 24;

		xoffset[0] = offset[0] * v[0][0] + offset[1] * v[1][0] + offset[2] * v[2][0];
		xoffset[1] = offset[0] * v[0][1] + offset[1] * v[1][1] + offset[2] * v[2][1];
		xoffset[2] = offset[0] * v[0][2] + offset[1] * v[1][2] + offset[2] * v[2][2];
		VectorAdd( cent->lerpOrigin, xoffset, re->origin );
		VectorCopy( re->origin, le->pos.trBase );
		if ( CG_PointContents( re->origin, -1 ) & CONTENTS_WATER ) {
			waterScale = 0.10f;
		}

		xvelocity[0] = velocity[0] * v[0][0] + velocity[1] * v[1][0] + velocity[2] * v[2][0];
		xvelocity[1] = velocity[0] * v[0][1] + velocity[1] * v[1][1] + velocity[2] * v[2][1];
		xvelocity[2] = velocity[0] * v[0][2] + velocity[1] * v[1][2] + velocity[2] * v[2][2];
		VectorScale( xvelocity, waterScale, le->pos.trDelta );

		AxisCopy( axisDefault, re->axis );
		re->hModel = cgs.media.shotgunBrassModel;
		le->bounceFactor = 0.3f;

		le->angles.trType = TR_LINEAR;
		le->angles.trTime = cg.time;
		le->angles.trBase[0] = rand()&31;
		le->angles.trBase[1] = rand()&31;
		le->angles.trBase[2] = rand()&31;
		le->angles.trDelta[0] = 1;
		le->angles.trDelta[1] = 0.5;
		le->angles.trDelta[2] = 0;

		le->leFlags = LEF_TUMBLE;
		le->leBounceSoundType = LEBS_BRASS;
		le->leMarkType = LEMT_NONE;
	}
}


#ifdef MISSIONPACK
/*
==========================
CG_NailgunEjectBrass
==========================
*/
static void CG_NailgunEjectBrass( centity_t *cent ) {
	localEntity_t	*smoke;
	vec3_t			origin;
	vec3_t			v[3];
	vec3_t			offset;
	vec3_t			xoffset;
	vec3_t			up;

	AnglesToAxis( cent->lerpAngles, v );

	offset[0] = 0;
	offset[1] = -12;
	offset[2] = 24;

	xoffset[0] = offset[0] * v[0][0] + offset[1] * v[1][0] + offset[2] * v[2][0];
	xoffset[1] = offset[0] * v[0][1] + offset[1] * v[1][1] + offset[2] * v[2][1];
	xoffset[2] = offset[0] * v[0][2] + offset[1] * v[1][2] + offset[2] * v[2][2];
	VectorAdd( cent->lerpOrigin, xoffset, origin );

	VectorSet( up, 0, 0, 64 );

	smoke = CG_SmokePuff( origin, up, 32, 1, 1, 1, 0.33f, 700, cg.time, 0, 0, cgs.media.smokePuffShader );
	// use the optimized local entity add
	smoke->leType = LE_SCALE_FADE;
}
#endif


/*
==========================
CG_RailTrail
==========================
*/
void CG_RailTrail( clientInfo_t *ci, vec3_t start, vec3_t end ) {
	localEntity_t	*le;
	refEntity_t		*re;

	//
	// rings
	//
	le = CG_AllocLocalEntity();
	re = &le->refEntity;

	le->leType = LE_FADE_RGB;
	le->startTime = cg.time;
	le->endTime = cg.time + cg_railTrailTime.value;
	le->lifeRate = 1.0 / ( le->endTime - le->startTime );

	re->shaderTime = cg.time / 1000.0f;
	re->reType = RT_RAIL_RINGS;
	re->customShader = cgs.media.railRingsShader;

	VectorCopy( start, re->origin );
	VectorCopy( end, re->oldorigin );

	// nudge down a bit so it isn't exactly in center
	re->origin[2] -= 8;
	re->oldorigin[2] -= 8;

	le->color[0] = ci->color[0] * 0.75;
	le->color[1] = ci->color[1] * 0.75;
	le->color[2] = ci->color[2] * 0.75;
	le->color[3] = 1.0f;

	AxisClear( re->axis );

	//
	// core
	//
	le = CG_AllocLocalEntity();
	re = &le->refEntity;

	le->leType = LE_FADE_RGB;
	le->startTime = cg.time;
	le->endTime = cg.time + cg_railTrailTime.value;
	le->lifeRate = 1.0 / ( le->endTime - le->startTime );

	re->shaderTime = cg.time / 1000.0f;
	re->reType = RT_RAIL_CORE;
	re->customShader = cgs.media.railCoreShader;

	VectorCopy( start, re->origin );
	VectorCopy( end, re->oldorigin );

	// nudge down a bit so it isn't exactly in center
	re->origin[2] -= 8;
	re->oldorigin[2] -= 8;

	le->color[0] = ci->color[0] * 0.75;
	le->color[1] = ci->color[1] * 0.75;
	le->color[2] = ci->color[2] * 0.75;
	le->color[3] = 1.0f;

	AxisClear( re->axis );
}

/*
==========================
CG_RocketTrail
==========================
*/
static void CG_RocketTrail( centity_t *ent, const weaponInfo_t *wi ) {
	int		step;
	vec3_t	origin, lastPos;
	int		t;
	int		startTime, contents;
	int		lastContents;
	entityState_t	*es;
	vec3_t	up;
	localEntity_t	*smoke;

	up[0] = 0;
	up[1] = 0;
	up[2] = 0;

	step = 50;

	es = &ent->currentState;
	startTime = ent->trailTime;
	t = step * ( (startTime + step) / step );

	BG_EvaluateTrajectory( &es->pos, cg.time, origin );
	contents = CG_PointContents( origin, -1 );

	// if object (e.g. grenade) is stationary, don't toss up smoke
	if ( es->pos.trType == TR_STATIONARY ) {
		ent->trailTime = cg.time;
		return;
	}

	BG_EvaluateTrajectory( &es->pos, ent->trailTime, lastPos );
	lastContents = CG_PointContents( lastPos, -1 );

	ent->trailTime = cg.time;

	if ( contents & ( CONTENTS_WATER | CONTENTS_SLIME | CONTENTS_LAVA ) ) {
		if ( contents & lastContents & CONTENTS_WATER ) {
			CG_BubbleTrail( lastPos, origin, 8 );
		}
		return;
	}

	for ( ; t <= ent->trailTime ; t += step ) {
		BG_EvaluateTrajectory( &es->pos, t, lastPos );

		smoke = CG_SmokePuff( lastPos, up,
					  wi->trailRadius,
					  1, 1, 1, 0.33f,
					  wi->wiTrailTime,
					  t,
					  0,
					  0,
					  cgs.media.smokePuffShader );
		// use the optimized local entity add
		smoke->leType = LE_SCALE_FADE;
	}

}

#ifdef MISSIONPACK
/*
==========================
CG_NailTrail
==========================
*/
static void CG_NailTrail( centity_t *ent, const weaponInfo_t *wi ) {
	int		step;
	vec3_t	origin, lastPos;
	int		t;
	int		startTime, contents;
	int		lastContents;
	entityState_t	*es;
	vec3_t	up;
	localEntity_t	*smoke;

	up[0] = 0;
	up[1] = 0;
	up[2] = 0;

	step = 50;

	es = &ent->currentState;
	startTime = ent->trailTime;
	t = step * ( (startTime + step) / step );

	BG_EvaluateTrajectory( &es->pos, cg.time, origin );
	contents = CG_PointContents( origin, -1 );

	// if object (e.g. grenade) is stationary, don't toss up smoke
	if ( es->pos.trType == TR_STATIONARY ) {
		ent->trailTime = cg.time;
		return;
	}

	BG_EvaluateTrajectory( &es->pos, ent->trailTime, lastPos );
	lastContents = CG_PointContents( lastPos, -1 );

	ent->trailTime = cg.time;

	if ( contents & ( CONTENTS_WATER | CONTENTS_SLIME | CONTENTS_LAVA ) ) {
		if ( contents & lastContents & CONTENTS_WATER ) {
			CG_BubbleTrail( lastPos, origin, 8 );
		}
		return;
	}

	for ( ; t <= ent->trailTime ; t += step ) {
		BG_EvaluateTrajectory( &es->pos, t, lastPos );

		smoke = CG_SmokePuff( lastPos, up,
					  wi->trailRadius,
					  1, 1, 1, 0.33f,
					  wi->wiTrailTime,
					  t,
					  0,
					  0,
					  cgs.media.nailPuffShader );
		// use the optimized local entity add
		smoke->leType = LE_SCALE_FADE;
	}

}
#endif

/*
==========================
CG_GrappleTrail
==========================
*/
void CG_GrappleTrail( centity_t *ent, const weaponInfo_t *wi ) {
	vec3_t	origin;
	entityState_t	*es;
	vec3_t			forward, up;
	refEntity_t		beam;

	es = &ent->currentState;

	BG_EvaluateTrajectory( &es->pos, cg.time, origin );
	ent->trailTime = cg.time;

	memset( &beam, 0, sizeof( beam ) );
	//FIXME adjust for muzzle position
	VectorCopy ( cg_entities[ ent->currentState.otherEntityNum ].lerpOrigin, beam.origin );
	beam.origin[2] += 26;
	AngleVectors( cg_entities[ ent->currentState.otherEntityNum ].lerpAngles, forward, NULL, up );
	VectorMA( beam.origin, -6, up, beam.origin );
	VectorCopy( origin, beam.oldorigin );

	if (Distance( beam.origin, beam.oldorigin ) < 64 )
		return; // Don't draw if close

	beam.reType = RT_LIGHTNING;
	beam.customShader = cgs.media.lightningShader;

	AxisClear( beam.axis );
	beam.shaderRGBA[0] = 0xff;
	beam.shaderRGBA[1] = 0xff;
	beam.shaderRGBA[2] = 0xff;
	beam.shaderRGBA[3] = 0xff;
	trap_R_AddRefEntityToScene( &beam );
}

/*
==========================
CG_GrenadeTrail
==========================
*/
static void CG_GrenadeTrail( centity_t *ent, const weaponInfo_t *wi ) {
	CG_RocketTrail( ent, wi );
}


/*
=================
CG_RegisterWeapon

The server says this item is used on this level
=================
*/
void CG_RegisterWeapon( int weaponNum ) {
	weaponInfo_t	*weaponInfo;
	gitem_t			*item, *ammo;
	char			path[MAX_QPATH];
	vec3_t			mins, maxs;
	int				i;

	weaponInfo = &cg_weapons[weaponNum];

	if ( weaponNum == 0 ) {
		return;
	}

	if ( weaponInfo->registered ) {
		return;
	}

	memset( weaponInfo, 0, sizeof( *weaponInfo ) );
	weaponInfo->registered = qtrue;

	for ( item = bg_itemlist + 1 ; item->classname ; item++ ) {
		if ( item->giType == IT_WEAPON && item->giTag == weaponNum ) {
			weaponInfo->item = item;
			break;
		}
	}
	if ( !item->classname ) {
		CG_Error( "Couldn't find weapon %i", weaponNum );
	}
	CG_RegisterItemVisuals( item - bg_itemlist );

	// load cmodel before model so filecache works
	weaponInfo->weaponModel = trap_R_RegisterModel( item->world_model[0] );

	// calc midpoint for rotation
	trap_R_ModelBounds( weaponInfo->weaponModel, mins, maxs );
	for ( i = 0 ; i < 3 ; i++ ) {
		weaponInfo->weaponMidpoint[i] = mins[i] + 0.5 * ( maxs[i] - mins[i] );
	}

	weaponInfo->weaponIcon = trap_R_RegisterShader( item->icon );
	weaponInfo->ammoIcon = trap_R_RegisterShader( item->icon );

	for ( ammo = bg_itemlist + 1 ; ammo->classname ; ammo++ ) {
		if ( ammo->giType == IT_AMMO && ammo->giTag == weaponNum ) {
			break;
		}
	}
	if ( ammo->classname && ammo->world_model[0] ) {
		weaponInfo->ammoModel = trap_R_RegisterModel( ammo->world_model[0] );
	}

	strcpy(path, bg_weaponlist[weaponNum].path );
	COM_StripExtension( path, path, sizeof(path) );
	strcat( path, "flash.md3" );
	weaponInfo->flashModel = trap_R_RegisterModel( path );

	strcpy( path, item->world_model[0] );
	COM_StripExtension( path, path, sizeof(path) );
	strcat( path, "_barrel.md3" );
	weaponInfo->barrelModel = trap_R_RegisterModel( path );

	strcpy( path, item->world_model[0] );
	COM_StripExtension( path, path, sizeof(path) );
	strcat( path, "_hand.md3" );
	weaponInfo->handsModel = trap_R_RegisterModel( path );

	if ( !weaponInfo->handsModel ) {
		weaponInfo->handsModel = trap_R_RegisterModel( "models/weapons2/shotgun/shotgun_hand.md3" );
	}

	weaponInfo->loopFireSound = qfalse;

	// first person view weaponmodel
	//right handed
	if(bg_weaponlist[weaponNum].v_model){
		strcpy( path, bg_weaponlist[weaponNum].v_model);
		weaponInfo->r_weaponModel = trap_R_RegisterModel( path);

		// left handed
#ifdef AKIMBO
		if(bg_weaponlist[weaponNum].wp_sort == WPS_PISTOL){
			COM_StripExtension( path, path, sizeof(path) );
			strcat( path, "_l.md3" );
			weaponInfo->l_weaponModel = trap_R_RegisterModel( path);
		}
#endif
	}

	if(bg_weaponlist[weaponNum].snd_fire && weaponNum >= WP_REM58
		&& weaponNum < WP_GATLING){
		for(i = 0; i < 3; i++){
			char name[MAX_QPATH];

			Com_sprintf (name, sizeof(name), "%s%i.wav", bg_weaponlist[weaponNum].snd_fire, i+1);
			weaponInfo->flashSound[i] = trap_S_RegisterSound( name, qfalse );
		}
	} else if(weaponNum == WP_GATLING){
		for(i = 0; i < 4; i++){
			char name[MAX_QPATH];

			Com_sprintf (name, sizeof(name), "%s%i.wav", bg_weaponlist[weaponNum].snd_fire, i+1);
			weaponInfo->flashSound[i] = trap_S_RegisterSound( name, qfalse );
		}
	} else {
		weaponInfo->flashSound[0] = trap_S_RegisterSound( bg_weaponlist[weaponNum].snd_fire, qfalse );
	}

	if(bg_weaponlist[weaponNum].snd_reload)
		weaponInfo->reloadSound = trap_S_RegisterSound( bg_weaponlist[weaponNum].snd_reload, qfalse );

	switch ( weaponNum ) {
	case WP_KNIFE:
		weaponInfo->missileModel = trap_R_RegisterModel( "models/weapons2/knife/e_knife.md3");
		/*weaponInfo->missileTrailFunc = CG_RocketTrail;
		weaponInfo->missileDlight = 0;
		weaponInfo->wiTrailTime = 0;
		weaponInfo->trailRadius = 0;*/
		break;

	case WP_PEACEMAKER:
	case WP_REM58:
	case WP_SCHOFIELD:
		MAKERGB( weaponInfo->flashDlightColor, 1, 0.8f, 0.6f );
		cgs.media.bulletExplosionShader = /*trap_R_RegisterShader("smokePuff");*/trap_R_RegisterShader( "waterExplosion" );
		cgs.media.bulletPuffShader = trap_R_RegisterShader("smokePuff");
		break;

	case WP_WINCHESTER66:
	case WP_LIGHTNING:
	case WP_SHARPS:
		MAKERGB( weaponInfo->flashDlightColor, 1, 0.8f, 0.6f );
		break;

	case WP_GATLING:
		weaponInfo->firingSound = cgs.media.sfx_gatling_loop;
		weaponInfo->ejectBrassFunc = CG_MachineGunEjectBrass;
		MAKERGB( weaponInfo->flashDlightColor, 1, 0.8f, 0.6f );
		break;

	case WP_REMINGTON_GAUGE:
	case WP_SAWEDOFF:
	case WP_WINCH97:
		MAKERGB( weaponInfo->flashDlightColor, 1, 0.8f, 0.6f );
		break;

	case WP_DYNAMITE:
		weaponInfo->missileModel = trap_R_RegisterModel( "models/weapons2/dyn/e_dynamite.md3" ); // Spoon
		cgs.media.dynamiteExplosionShader = trap_R_RegisterShader( "wqfx/dynexp");
		weaponInfo->missileSound = cgs.media.dynamiteburn;
		weaponInfo->missileDlight = 30;
		VectorSet(weaponInfo->missileDlightColor, 1, 0.75f, 0.4f);
		break;

	case WP_MOLOTOV:
		weaponInfo->missileModel = trap_R_RegisterModel( "models/weapons2/molotov/e_molotov_missile.md3" ); // Spoon
		weaponInfo->missileDlight = 30;
		VectorSet(weaponInfo->missileDlightColor, 1, 0.75f, 0.4f);
		break;

	 default:
		MAKERGB( weaponInfo->flashDlightColor, 0, 0, 0 );
		break;
	}
}

/*
=================
CG_RegisterItemVisuals

The server says this item is used on this level
=================
*/
void CG_RegisterItemVisuals( int itemNum ) {
	itemInfo_t		*itemInfo;
	gitem_t			*item;

	itemInfo = &cg_items[ itemNum ];
	if ( itemInfo->registered ) {
		return;
	}

	item = &bg_itemlist[ itemNum ];

	memset( itemInfo, 0, sizeof( &itemInfo ) );
	itemInfo->registered = qtrue;

	itemInfo->models[0] = trap_R_RegisterModel( item->world_model[0] );

	itemInfo->icon = trap_R_RegisterShader( item->icon );

	if ( item->giType == IT_WEAPON ) {
		CG_RegisterWeapon( item->giTag );
	}

	//
	// powerups have an accompanying ring or sphere
	//
	if ( item->giType == IT_POWERUP || item->giType == IT_HEALTH ||
		item->giType == IT_ARMOR || item->giType == IT_HOLDABLE ) {
		if ( item->world_model[1] ) {
			itemInfo->models[1] = trap_R_RegisterModel( item->world_model[1] );
		}
	}
}


/*
========================================================================================

VIEW WEAPON

========================================================================================
*/

/*
=================
CG_MapTorsoToWeaponFrame

=================
*/
static int CG_MapTorsoToWeaponFrame( clientInfo_t *ci, int frame ) {

	// change weapon
	/*if ( frame >= ci->animations[TORSO_DROP].firstFrame
		&& frame < ci->animations[TORSO_DROP].firstFrame + 9 ) {
		return frame - ci->animations[TORSO_DROP].firstFrame + 6;
	}*/

	// stand attack
	/*if ( frame >= ci->animations[TORSO_ATTACK].firstFrame
		&& frame < ci->animations[TORSO_ATTACK].firstFrame + 6 ) {
		return 1 + frame - ci->animations[TORSO_ATTACK].firstFrame;
	}

	// stand attack 2
	if ( frame >= ci->animations[TORSO_ATTACK2].firstFrame
		&& frame < ci->animations[TORSO_ATTACK2].firstFrame + 6 ) {
		return 1 + frame - ci->animations[TORSO_ATTACK2].firstFrame;
	}*/ // own animation of the weapon

	return 0;
}


/*
==============
CG_CalculateWeaponPosition
==============
*/
#define MAX_WP_DISTANCE	0.75f
#define	ANGLE_REDUCTION	30
static void CG_CalculateWeaponPosition( vec3_t origin, vec3_t angles ) {
	float	scale;
	int		delta;
	float	fracsin;
	vec3_t	forward;
	float	pitch = cg.refdefViewAngles[PITCH];
	float	value;

	VectorCopy( cg.refdef.vieworg, origin );
	VectorCopy( cg.refdefViewAngles, angles );

	// on odd legs, invert some angles
	if ( cg.bobcycle & 1 ) {
		scale = -cg.xyspeed;
	} else {
		scale = cg.xyspeed;
	}

	// gun angles from bobbing
	angles[ROLL] += scale * cg.bobfracsin * 0.005;
	angles[YAW] += scale * cg.bobfracsin * 0.01;
	angles[PITCH] += cg.xyspeed * cg.bobfracsin * 0.005;

	VectorCopy(cg_entities[cg.snap->ps.clientNum].currentState.pos.trDelta, forward);
	VectorNormalize(forward);
	VectorMA( origin, cg.bobfracsin*0.5f, forward, origin);

	// drop the weapon when landing
	delta = cg.time - cg.landTime;
	if ( delta < LAND_DEFLECT_TIME ) {
		origin[2] += cg.landChange*0.25 * delta / LAND_DEFLECT_TIME;
	} else if ( delta < LAND_DEFLECT_TIME + LAND_RETURN_TIME ) {
		origin[2] += cg.landChange*0.25 *
			(LAND_DEFLECT_TIME + LAND_RETURN_TIME - delta) / LAND_RETURN_TIME;
	}

#if 0
	// drop the weapon when stair climbing
	delta = cg.time - cg.stepTime;
	if ( delta < STEP_TIME/2 ) {
		origin[2] -= cg.stepChange*0.25 * delta / (STEP_TIME/2);
	} else if ( delta < STEP_TIME ) {
		origin[2] -= cg.stepChange*0.25 * (STEP_TIME - delta) / (STEP_TIME/2);
	}
#endif

	// idle drift
	scale = cg.xyspeed + 40;
	fracsin = sin( cg.time * 0.001 );
	angles[ROLL] += scale * fracsin * 0.01;
	angles[YAW] += scale * fracsin * 0.01;

	value = pitch/ANGLE_REDUCTION;

	angles[PITCH] += scale * fracsin * 0.01 + value;

	//angle: look up and down
	if(pitch < 0)
		pitch *= -1;

	AngleVectors( cg.refdefViewAngles, forward, NULL, NULL );
	VectorMA( origin, MAX_WP_DISTANCE*(pitch/90.0f), forward, origin );
}

/*
===============
CG_LightningBolt

Origin will be the exact tag point, which is slightly
different than the muzzle point used for determining hits.
The cent should be the non-predicted cent if it is from the player,
so the endpoint will reflect the simulated strike (lagging the predicted
angle)
===============
*/
static void CG_LightningBolt( centity_t *cent, vec3_t origin ) {
	trace_t		trace;
	refEntity_t		beam;
	vec3_t			forward;
	vec3_t			muzzlePoint, endPoint;

	if ( cent->currentState.weapon != WP_SAWEDOFF ) {
		return;
	}

	memset( &beam, 0, sizeof( beam ) );

	// find muzzle point for this frame
	VectorCopy( cent->lerpOrigin, muzzlePoint );
	AngleVectors( cent->lerpAngles, forward, NULL, NULL );

	// FIXME: crouch
	muzzlePoint[2] += DEFAULT_VIEWHEIGHT;

	VectorMA( muzzlePoint, 14, forward, muzzlePoint );

	// project forward by the lightning range
	VectorMA( muzzlePoint, LIGHTNING_RANGE, forward, endPoint );

	// see if it hit a wall
	CG_Trace( &trace, muzzlePoint, vec3_origin, vec3_origin, endPoint,
		cent->currentState.number, MASK_SHOT );

	// this is the endpoint
	VectorCopy( trace.endpos, beam.oldorigin );

	// use the provided origin, even though it may be slightly
	// different than the muzzle origin
	VectorCopy( origin, beam.origin );

	beam.reType = RT_LIGHTNING;
	beam.customShader = cgs.media.lightningShader;
	trap_R_AddRefEntityToScene( &beam );

	// add the impact flare if it hit something
	if ( trace.fraction < 1.0 ) {
		vec3_t	angles;
		vec3_t	dir;

		VectorSubtract( beam.oldorigin, beam.origin, dir );
		VectorNormalize( dir );

		memset( &beam, 0, sizeof( beam ) );
		beam.hModel = cgs.media.lightningExplosionModel;

		VectorMA( trace.endpos, -16, dir, beam.origin );

		// make a random orientation
		angles[0] = rand() % 360;
		angles[1] = rand() % 360;
		angles[2] = rand() % 360;
		AnglesToAxis( angles, beam.axis );
		trap_R_AddRefEntityToScene( &beam );
	}
}


/*
===============
CG_SpawnRailTrail

Origin will be the exact tag point, which is slightly
different than the muzzle point used for determining hits.
===============
*/
#if 0
static void CG_SpawnRailTrail( centity_t *cent, vec3_t origin ) {
	clientInfo_t	*ci;

	if ( cent->currentState.weapon != WP_RAILGUN ) {
		return;
	}
	if ( !cent->pe.railgunFlash ) {
		return;
	}
	cent->pe.railgunFlash = qtrue;
	ci = &cgs.clientinfo[ cent->currentState.clientNum ];
	CG_RailTrail( ci, origin, cent->pe.railgunImpact );
}
#endif


/*
======================
CG_MachinegunSpinAngle
======================
*/
#define		SPIN_SPEED	0.3//0.9
#define		COAST_TIME	500//1000
float	CG_MachinegunSpinAngle( centity_t *cent/*, qboolean reloading*/ ) {
	int		delta;
	float	angle;
	float	speed;
	qboolean firing = ((cent->currentState.eFlags & EF_FIRING) /*&& !reloading*/);

	// if this is the player which usese the gatling do prediction
	if(cg.snap->ps.clientNum == cent->currentState.clientNum &&
		!cg_nopredict.integer && !cg_synchronousClients.integer){
		firing = ( cg.predictedPlayerState.eFlags & EF_FIRING);
	}

	delta = cg.time - cent->pe.barrelTime;
	if ( cent->pe.barrelSpinning ) {
		angle = cent->pe.barrelAngle + delta * SPIN_SPEED;

		trap_S_AddLoopingSound( cent->currentState.number, cent->lerpOrigin, vec3_origin, cgs.media.sfx_gatling_loop );

	} else {
		if ( delta > COAST_TIME ) {
			delta = COAST_TIME;
		}

		speed = 0.5 * ( SPIN_SPEED + (float)( COAST_TIME - delta ) / COAST_TIME );

		if(COAST_TIME - delta)
			trap_S_AddLoopingSound( cent->currentState.number, cent->lerpOrigin, vec3_origin, cgs.media.sfx_gatling_loop );

		angle = cent->pe.barrelAngle + delta * speed;
	}

	if ( cent->pe.barrelSpinning == !firing) {
		cent->pe.barrelTime = cg.time;
		cent->pe.barrelAngle = AngleMod( angle );
		cent->pe.barrelSpinning = !!firing;
	}

	return angle;
}


/*
========================
CG_AddWeaponWithPowerups
========================
*/
static void CG_AddWeaponWithPowerups( refEntity_t *gun, int powerups ) {
	// add powerup effects
	/*if ( powerups & ( 1 << PW_INVIS ) ) {
		gun->customShader = cgs.media.invisShader;
		trap_R_AddRefEntityToScene( gun );
	} else {*/
		trap_R_AddRefEntityToScene( gun );

		/*if ( powerups & ( 1 << PW_BATTLESUIT ) ) {
			gun->customShader = cgs.media.battleWeaponShader;
			trap_R_AddRefEntityToScene( gun );
		}
		if ( powerups & ( 1 << PW_QUAD ) ) {
			gun->customShader = cgs.media.quadWeaponShader;
			trap_R_AddRefEntityToScene( gun );
		}
	}*/
}

/*
=============
CG_InitWeaponAnim
=============
*/
void CG_InitWeaponAnim(centity_t *cent, int weapon, int weaponAnim, lerpFrame_t *anim,
					   qboolean weapon2){

	int realanim = weaponAnim;
	realanim &= ~ANIM_TOGGLEBIT;

	if(!cg.snap->ps.ammo[WP_REMINGTON_GAUGE])
		bg_weaponlist[WP_REMINGTON_GAUGE].animations[WP_ANIM_CHANGE].numFrames = 20;
	else
		bg_weaponlist[WP_REMINGTON_GAUGE].animations[WP_ANIM_CHANGE].numFrames = 49;

	//special functions
	if(realanim == WP_ANIM_SPECIAL && weapon == WP_WINCHESTER66){
		if(cg.snap->ps.powerups[PW_SCOPE] == 2)
			bg_weaponlist[WP_WINCHESTER66].animations[WP_ANIM_SPECIAL].reversed = qtrue;
		else
			bg_weaponlist[WP_WINCHESTER66].animations[WP_ANIM_SPECIAL].reversed = qfalse;
	}

	//set the animation
	anim->weaponAnim = weaponAnim;
}

/*
=============
CG_AddPlayerWeapon

Used for both the view weapon (ps is valid) and the world modelother character models (ps is NULL)
The main player will have this called for BOTH cases, so effects like light and
sound should only be done on the world model case.
=============
*/
void CG_AddPlayerWeapon( refEntity_t *parent, playerState_t *ps, centity_t *cent, int team ) {
	refEntity_t	gun;
	refEntity_t	gun2;

//	refEntity_t	barrel;
	refEntity_t	flash;
	vec3_t		angles;
	weapon_t	weaponNum, weapon2Num;
	weaponInfo_t	*weapon, *weapon2;
	centity_t	*nonPredictedCent;
	int i;
	qboolean render = qtrue;

	if(ps && ps->weapon == WP_SHARPS && cg.snap->ps.stats[STAT_WP_MODE]==1)
		render = qfalse;

	weaponNum = cent->currentState.weapon;
	weapon2Num = cent->currentState.frame;

	// set weapon prediction
	if(ps){
		weaponNum = cg.weapon;
		weapon2Num = cg.weapon2;
	}

	//Animation of weapons
	if(ps){
		clientInfo_t	*ci;

		ci = &cgs.clientinfo[ cent->currentState.clientNum];

		// no prediction
		if(cg.animtime < cg.time){
			cg.weaponAnim = ps->weaponAnim;
			cg.weapon2Anim = ps->weapon2Anim;
			weaponNum = ps->weapon;
			weapon2Num = ps->weapon2;
		}

		// 2nd weapon
		if(ps->weapon2){
			// copy the animation
			if(cg.weapon != cg.weaponold){
				/*int anim = cg.weapon2Anim;
				anim &= ~ANIM_TOGGLEBIT;*/

				BG_CopyLerpFrame(&cent->pe.weapon, &cent->pe.weapon2);

				/*if(bg_weaponlist[cg.weaponold].wp_sort == WPS_PISTOL &&
					(anim == WP_ANIM_RELOAD ||
					anim == WP_ANIM_SPECIAL2) )
					cent->pe.weapon2.animationNumber = cg.weapon2Anim;*/

				if(!cg.weaponold && cg.weapon){
					cg.weapon2Anim = WP_ANIM_CHANGE;
				}
			}

			CG_InitWeaponAnim(cent, weapon2Num, cg.weapon2Anim, &cent->pe.weapon2, qtrue);
			CG_RunLerpFrame( ci, &cent->pe.weapon2, cent->pe.weapon2.weaponAnim, 1.0, weapon2Num);
		}

		// 1st weapon
		CG_InitWeaponAnim(cent, weaponNum, cg.weaponAnim, &cent->pe.weapon, qfalse);
		CG_RunLerpFrame(ci, &cent->pe.weapon, cent->pe.weapon.weaponAnim, 1.0, weaponNum);
	}

	CG_RegisterWeapon( weaponNum );
	weapon = &cg_weapons[weaponNum];

	if(weaponNum == WP_GATLING){ //doesn't have a weapon, "tagged" on the player

		int anim = cg.weaponAnim;
		anim &= ~ANIM_TOGGLEBIT;

		if(ps && cg.gatlingmodeOld && !cg.gatlingmode &&
			!(ps->stats[STAT_WEAPONS] & (1 << WP_GATLING))){
			if(anim != WP_ANIM_DROP &&
				anim != WP_ANIM_CHANGE){
				return;
			}
		}

		if(ps && ps->stats[STAT_GATLING_MODE]){

			if(cent->pe.weapon.frame == bg_weaponlist[WP_GATLING].animations[anim].numFrames){
				return;
			}

			if(anim != WP_ANIM_DROP &&
				anim != WP_ANIM_CHANGE){
				return;
			}
		}
		if(!ps){
			if(cent->currentState.powerups & (1 << PW_GATLING))
				return;
		}
	}

	// add the weapon
	memset( &gun, 0, sizeof( gun ) );
	VectorCopy( parent->lightingOrigin, gun.lightingOrigin );
	gun.shadowPlane = parent->shadowPlane;
	gun.renderfx = parent->renderfx;

	//2nd weapon if akimbo
	if((ps && ps->weapon2) || weapon2Num){

		memset( &gun2, 0, sizeof( gun2 ) );
		VectorCopy( parent->lightingOrigin, gun2.lightingOrigin );
		//VectorNegate(gun2.lightingOrigin, gun2.lightingOrigin);
		gun2.shadowPlane = parent->shadowPlane;
		gun2.renderfx = parent->renderfx;
	}

	if(ps && bg_weaponlist[weaponNum].v_model){
		clientInfo_t	*ci;

		ci = &cgs.clientinfo[ cent->currentState.clientNum];

		//2nd Pistol
		if(ps->weapon2){
			weapon2 = &cg_weapons[ps->weapon2];

			//assign
			gun2.frame = cent->pe.weapon2.frame;
			gun2.oldframe = cent->pe.weapon2.oldFrame;
			gun2.backlerp = cent->pe.weapon2.backlerp;

			gun2.hModel = weapon2->r_weaponModel;
		}

		// first person view
		//assign
		gun.frame = cent->pe.weapon.frame;
		gun.oldframe = cent->pe.weapon.oldFrame;
		gun.backlerp = cent->pe.weapon.backlerp;

		if(ps->weapon2)
			gun.hModel = weapon->l_weaponModel;
		else
			gun.hModel = weapon->r_weaponModel;

	} else {
		//if the player has the scope on the weapon
		if(weaponNum == WP_DYNAMITE){
			gun.hModel = cgs.media.e_dynamite;
		} else if(weaponNum == WP_KNIFE)
			gun.hModel = cgs.media.e_knife;
		else
			gun.hModel = weapon->weaponModel;

		if(weapon2Num){
			CG_RegisterWeapon(weapon2Num);
			weapon2 = &cg_weapons[weapon2Num];
			gun2.hModel = weapon2->weaponModel;
		}
	}

	if (!gun.hModel) {
		return;
	}

	if ( !ps ) {
		// add weapon ready sound
		cent->pe.lightningFiring = qfalse;
	}

	// first weapon
	if(!ps){
		// The player hold two guns.
		// weaponNum is the one on the left hand
		// weapon2Num is the one on the right hand
		if (weapon2Num) // left hand
			CG_PositionEntityOnTag( &gun, parent, parent->hModel, "tag_weapon2");
		else
			CG_PositionEntityOnTag( &gun, parent, parent->hModel, "tag_weapon");
		if(weaponNum ==  WP_DYNAMITE){
			VectorScale(gun.axis[0], 0.5, gun.axis[0]);
			VectorScale(gun.axis[1], 0.5, gun.axis[1]);
			VectorScale(gun.axis[2], 0.5, gun.axis[2]);
		} else if(weaponNum == WP_MOLOTOV){
			VectorScale(gun.axis[0], 0.65, gun.axis[0]);
			VectorScale(gun.axis[1], 0.65, gun.axis[1]);
			VectorScale(gun.axis[2], 0.65, gun.axis[2]);
		}
	} else {
		vec3_t forward;

		CG_PositionViewWeaponOnTag( &gun, parent, gun.hModel, "tag_weapon");
		// we'll use the gun.hModel as the parent model here, doesn't matter

		// go backwards some units
		AngleVectors(cg.refdefViewAngles, forward, NULL, NULL);
		VectorMA(gun.origin, -2, forward, gun.origin);

		if(ps->weapon2)
			VectorNegate(gun.axis[1], gun.axis[1]);
	}

	// add scope if nessecary
	if(ps && ps->weapon == WP_SHARPS &&
		(ps->powerups[PW_SCOPE] == 2 || ps->powerups[PW_SCOPE] == -1 )){
		refEntity_t scope;

		// make a move from the scope to the eye
		if(cg.scopetime + SCOPE_TIME > cg.time && cg.scopetime){
			refEntity_t temp;
			vec3_t	dist;
			int		delta;
			float factor;

			delta = cg.time - cg.scopetime;
			factor = (float)delta/(float)SCOPE_TIME;
			factor = sqrt(factor);

			if(cg.scopedeltatime < 0){
				factor = (float)(SCOPE_TIME-delta)/(float)SCOPE_TIME;
				factor = factor*factor;
			}
			memset( &temp, 0, sizeof( temp ) );
			CG_PositionViewWeaponOnTag( &temp, &gun, gun.hModel, "tag_eye");

			VectorSubtract(cg.refdef.vieworg, temp.origin, dist);
			//VectorMA(cg.refdef.vieworg, factor, dist, cg.refdef.vieworg);
			// now move the weapon
			VectorMA(gun.origin, factor, dist, gun.origin);
		}

		memset( &scope, 0, sizeof( scope ) );
		VectorCopy( gun.lightingOrigin, scope.lightingOrigin );
		scope.shadowPlane = gun.shadowPlane;
		scope.renderfx = gun.renderfx;

		scope.hModel = cgs.media.model_scope;
		CG_PositionViewWeaponOnTag( &scope, &gun, gun.hModel, "tag_scope");

		if(render)
			trap_R_AddRefEntityToScene( &scope );
	}

	if(render)
		CG_AddWeaponWithPowerups( &gun, cent->currentState.powerups );

	// 2nd weapon
	if(ps && ps->weapon2){

		CG_PositionViewWeaponOnTag( &gun2, parent, gun2.hModel, "tag_weapon");

		if(render)
		CG_AddWeaponWithPowerups( &gun2, cent->currentState.powerups );

	} else if(weapon2Num){

		// The player hold two guns.
		// weapon2Num is the one on the right hand
		CG_PositionEntityOnTag( &gun2, parent, parent->hModel, "tag_weapon");

		if(render)
			CG_AddWeaponWithPowerups( &gun2, cent->currentState.powerups );
	}


	// add the spinning barrel
	/*if ( weapon->barrelModel && ps) {
		memset( &barrel, 0, sizeof( barrel ) );
		VectorCopy( parent->lightingOrigin, barrel.lightingOrigin );
		barrel.shadowPlane = parent->shadowPlane;
		barrel.renderfx = parent->renderfx;

		barrel.hModel = weapon->barrelModel;
		angles[YAW] = 0;
		angles[PITCH] = 0;
		angles[ROLL] = CG_MachinegunSpinAngle( cent, (cent->pe.weapon.weaponAnim == WP_ANIM_RELOAD) );
		AnglesToAxis( angles, barrel.axis );

		CG_PositionRotatedEntityOnTag( &barrel, &gun, gun.hModel, "tag_barrel" );

		CG_AddWeaponWithPowerups( &barrel, cent->currentState.powerups );

		// add animations
		barrel.frame = gun.frame;
		barrel.oldframe = gun.oldframe;
		barrel.backlerp = gun.backlerp;
	}*/

	// make sure we aren't looking at cg.predictedPlayerEntity for LG
	nonPredictedCent = &cg_entities[cent->currentState.clientNum];

	// if the index of the nonPredictedCent is not the same as the clientNum
	// then this is a fake player (like on teh single player podiums), so
	// go ahead and use the cent
	if( ( nonPredictedCent - cg_entities ) != cent->currentState.clientNum ) {
		nonPredictedCent = cent;
	}

	// add sparks to the burning dynamite
	if ((cent->currentState.powerups & (1 << PW_BURN)) &&
		(cg.snap->ps.clientNum != cent->currentState.clientNum ||
		cg_thirdPerson.integer)){

		if(weaponNum == WP_DYNAMITE){
			refEntity_t		sparks;
			vec3_t			origin;

			memset( &sparks, 0, sizeof( sparks ) );
			sparks.reType = RT_SPRITE;
			sparks.customShader = cgs.media.wqfx_sparks;
			sparks.radius = 10;
			sparks.renderfx = 0;

			AnglesToAxis( vec3_origin, sparks.axis );

			CG_PositionRotatedEntityOnTag( &sparks, &gun, gun.hModel, "tag_sparks");
			trap_R_AddRefEntityToScene( &sparks );

			VectorCopy(sparks.origin, origin);

			origin[2] += 20;

			// add dynamic light
			trap_R_AddLightToScene(origin, weapon->missileDlight,
				weapon->missileDlightColor[0], weapon->missileDlightColor[1], weapon->missileDlightColor[2] );

		// add burning flame to the molotov
		} else if(weaponNum == WP_MOLOTOV){
			refEntity_t		fire;
			vec3_t			origin;

			memset( &fire, 0, sizeof( fire ) );
			fire.reType = RT_SPRITE;
			fire.customShader = cgs.media.wqfx_matchfire;
			fire.radius = 2;
			fire.renderfx = parent->renderfx;

			AnglesToAxis( vec3_origin, fire.axis );

			CG_PositionRotatedEntityOnTag( &fire, &gun, gun.hModel, "tag_fire");
			trap_R_AddRefEntityToScene( &fire );

			VectorCopy(fire.origin, origin);

			origin[2] += 20;
			trap_R_AddLightToScene(origin, weapon->missileDlight,
				weapon->missileDlightColor[0], weapon->missileDlightColor[1], weapon->missileDlightColor[2] );
		}
	}

	if(weaponNum == WP_GATLING)
		return;

#define SMOKE_TIME	1000
#define DELTA_TIME	75
#define LIFE_TIME	4000

	// add smoke if nessecary
	if(cg.time >= cent->lastSmokeTime && cent->lastSmokeTime && cg_gunsmoke.integer){
		refEntity_t		re;
		int contents;

		if(!cg_thirdPerson.integer && !ps)
			goto muzzle;

		if(cent->lastSmokeTime - cent->smokeTime > SMOKE_TIME){
			cent->lastSmokeTime = 0;
			cent->smokeTime = 0;
		} else
			cent->lastSmokeTime += DELTA_TIME;

		if(ps){

			// check if player is in the water
			contents = trap_CM_PointContents( cg.refdef.vieworg, 0 );
			if ( contents & CONTENTS_WATER ) {
				goto muzzle;
			}

			// get position of tag_flash
			CG_PositionRotatedEntityOnTag( &re, &gun, gun.hModel, "tag_flash");

			CG_AddSmokeParticle(re.origin, 4, 2, LIFE_TIME, 5, 0, vec3_origin);
		}
	}

	if(	cg.time >= cent->lastSmokeTime2 && cent->lastSmokeTime2 && cg_gunsmoke.integer){
		refEntity_t		re;
		int contents;

		if(!cg_thirdPerson.integer && !ps)
			goto muzzle;

		if(cent->lastSmokeTime2 - cent->smokeTime2 > SMOKE_TIME){
			cent->lastSmokeTime2 = 0;
			cent->smokeTime2 = 0;
		} else
			cent->lastSmokeTime2 += DELTA_TIME;

		if(ps){
			// check if player is in the water
			contents = trap_CM_PointContents( cg.refdef.vieworg, 0 );
			if ( contents & CONTENTS_WATER ) {
				goto muzzle;
			}

			// get position of tag_flash
			CG_PositionRotatedEntityOnTag( &re, &gun2, gun2.hModel, "tag_flash");

			CG_AddSmokeParticle(re.origin, 4, 2, LIFE_TIME, 5, 0, vec3_origin);
		}
	}

muzzle:

	// add the flash
	if ( (cg.time - cent->muzzleFlashTime > MUZZLE_FLASH_TIME) &&
		(cg.time - cent->muzzleFlashTime2 > MUZZLE_FLASH_TIME)) {
		return;
	}

	// decide which weapon to add the flash
	for(i = 0; i < 2; i++){
		memset( &flash, 0, sizeof( flash ) );
		VectorCopy( parent->lightingOrigin, flash.lightingOrigin );
		flash.shadowPlane = parent->shadowPlane;
		flash.renderfx = parent->renderfx;

		if((!(cg.time - cent->muzzleFlashTime > MUZZLE_FLASH_TIME)) && !i){

			flash.hModel = weapon->flashModel;

		} else if(!(cg.time - cent->muzzleFlashTime2 > MUZZLE_FLASH_TIME) && cent->currentState.frame){

			flash.hModel = weapon2->flashModel;
			i = 2;
		}
		if (!flash.hModel) {
			if(!i)
				return;
			else
				break;
		}

		angles[YAW] = 0;
		angles[PITCH] = 0;
		angles[ROLL] = crandom() * 10;
		AnglesToAxis( angles, flash.axis );

		if(i == 2){
			CG_PositionRotatedEntityOnTag( &flash, &gun2, gun2.hModel, "tag_flash");
		} else {
			CG_PositionRotatedEntityOnTag( &flash, &gun, gun.hModel, "tag_flash");
		}
		trap_R_AddRefEntityToScene( &flash );

		if ( ps || cg.renderingThirdPerson ||
			cent->currentState.number != cg.predictedPlayerState.clientNum ) {

			if( 1 == 2){
				if ( weapon2->flashDlightColor[0] || weapon2->flashDlightColor[1] || weapon2->flashDlightColor[2] ) {
					trap_R_AddLightToScene( flash.origin, 100 + (rand()&31), weapon2->flashDlightColor[0],
						weapon2->flashDlightColor[1], weapon2->flashDlightColor[2] );
				}

			} else {
				if ( weapon->flashDlightColor[0] || weapon->flashDlightColor[1] || weapon->flashDlightColor[2] ) {
					trap_R_AddLightToScene( flash.origin, 100 + (rand()&31), weapon->flashDlightColor[0],
						weapon->flashDlightColor[1], weapon->flashDlightColor[2] );
				}
			}
		}
	}
}

/*
==============
CG_AddViewWeapon

Add the weapon, and flash for the player's view
==============
*/
void CG_AddViewWeapon( playerState_t *ps ) {
	refEntity_t	hand;
	centity_t	*cent;
	clientInfo_t	*ci;
	float		fovOffset;
	vec3_t		angles;
	weaponInfo_t	*weapon;

	if ( ps->persistant[PERS_TEAM] >= TEAM_SPECTATOR ) {
		return;
	}

	if ( ps->pm_type == PM_INTERMISSION ) {
		return;
	}

	// no gun if in third person view
	if ( cg.renderingThirdPerson ) {
		return;
	}

	// allow the gun to be completely removed
	if ( !cg_drawGun.integer ) {
		return;
	}

	// don't draw if testing a gun model
	if ( cg.testGun ) {
		return;
	}


	// drop gun lower at higher fov
	if ( cg_fov.integer > 90 ) {
		fovOffset = -0.2 * ( cg_fov.integer - 90 );
	} else {
		fovOffset = 0;
	}

	cent = &cg.predictedPlayerEntity;	// &cg_entities[cg.snap->ps.clientNum];
	CG_RegisterWeapon( ps->weapon );
	weapon = &cg_weapons[ ps->weapon ];

	memset (&hand, 0, sizeof(hand));

	// set up gun position
	CG_CalculateWeaponPosition( hand.origin, angles );

	VectorMA( hand.origin, cg_gun_x.value, cg.refdef.viewaxis[0], hand.origin );
	VectorMA( hand.origin, cg_gun_y.value, cg.refdef.viewaxis[1], hand.origin );
	VectorMA( hand.origin, (cg_gun_z.value+fovOffset), cg.refdef.viewaxis[2], hand.origin );

	AnglesToAxis( angles, hand.axis );

	// map torso animations to weapon animations
	if ( cg_gun_frame.integer ) {
		// development tool
		hand.frame = hand.oldframe = cg_gun_frame.integer;
		hand.backlerp = 0;
	} else {
		// get clientinfo for animation map
		ci = &cgs.clientinfo[ cent->currentState.clientNum ];
		hand.frame = 0;// CG_MapTorsoToWeaponFrame( ci, cent->pe.torso.frame );
		hand.oldframe = 0;//CG_MapTorsoToWeaponFrame( ci, cent->pe.torso.oldFrame );
		hand.backlerp = cent->pe.torso.backlerp;
	}

	hand.hModel = weapon->handsModel;
	hand.renderfx = RF_DEPTHHACK | RF_FIRST_PERSON | RF_MINLIGHT;

	// add everything onto the hand
	CG_AddPlayerWeapon( &hand, ps, &cg.predictedPlayerEntity, ps->persistant[PERS_TEAM] );
}

/*
==============================================================================

WEAPON SELECTION

==============================================================================
*/

#define FIELD_WIDTH		135
#define	FIELD_HEIGHT	47
#define	TOP_HEIGHT		24

#define AMMO_BULLET_WIDTH	15
#define AMMO_CART_WIDTH		19
#define AMMO_SHARP_WIDTH	24
#define AMMO_SHELL_WIDTH	14.5
#define AMMO_HEIGHT				4.5

void CG_DrawWeaponSelect( void ) {

	vec4_t	color2[4] = {
		{1.0f, 1.0f, 1.0f, 0.5f},
		{0.0f, 0.0f, 0.0f, 0.75f},
		{0.4f, 0.4f, 0.4f, 0.75f},
		{1.0f, 0.8f, 0.0f, 1.0f},
	};
	int		i;
	int		bits;
	int		x, y, w;
	char	*name;
	int		wp_sort;
	qboolean akimbo, singlepistolSelect;
	int		ammo, weapon;
	float	ammowidth;
	qhandle_t	clip;

	// in duel mode don't let them choose the weapon before the intro ends
	if(cgs.gametype == GT_DUEL && cg.introend && (cg.introend-DU_INTRO_DRAW) >= cg.time)
		return;

	// don't display if dead
	if ( cg.predictedPlayerState.stats[STAT_HEALTH] <= 0 ) {
		return;
	}

	if( !cg.markedweapon)
		return;

	if( cg.menu ){
		cg.markedweapon = 0;
		return;
	}

	if( cg.markedweapon != WP_AKIMBO &&
		!(cg.snap->ps.stats[STAT_WEAPONS] & (1 << cg.markedweapon))){
		cg.markedweapon = 0;
		return;
	}

	// count the number of weapons owned
	bits = cg.snap->ps.stats[ STAT_WEAPONS ];

	// draw the category-numbers
	for( i = 1; i < WPS_NUM_ITEMS; i++){
		CG_DrawPic((i-1)*FIELD_WIDTH, 0, FIELD_WIDTH, TOP_HEIGHT, cgs.media.hud_wp_top);
		//draw the number
		trap_R_SetColor(color2[0]);
		CG_DrawPic((i-1)*FIELD_WIDTH + FIELD_WIDTH/2 -15/2, (TOP_HEIGHT-15)/2, 15, 15, cgs.media.numbermoneyShaders[i]);
		trap_R_SetColor(NULL);
	}

	if(cg.markedweapon == WP_AKIMBO)
		wp_sort = WPS_PISTOL;
	else
		wp_sort = bg_weaponlist[cg.markedweapon].wp_sort;

	akimbo = (wp_sort == WPS_PISTOL &&
		(BG_CountTypeWeapons(WPS_PISTOL, cg.snap->ps.stats[STAT_WEAPONS]) >= 2 ||
		(cg.snap->ps.stats[STAT_FLAGS] & SF_SEC_PISTOL) ) );

	// has the player got one pistol in hand ?
	singlepistolSelect = cg.weaponSelect != WP_AKIMBO
		&& bg_weaponlist[ cg.weaponSelect ].wp_sort == WPS_PISTOL;

	x = (wp_sort-1)*FIELD_WIDTH;
	y = TOP_HEIGHT;

	for ( i = WP_KNIFE ; i < WP_NUM_WEAPONS ; i++ ) {
		gitem_t	*item = BG_FindItemForWeapon(i);
		qhandle_t	shader;

		if ( !( bits & ( 1 << i ) ) ) {
			continue;
		}

		if(bg_weaponlist[i].wp_sort != wp_sort)
			continue;

		CG_RegisterWeapon( i );
		shader = cg_weapons[i].weaponIcon;

		if( i == WP_SHARPS && cg.snap->ps.powerups[PW_SCOPE] == 2)
			shader = cgs.media.sharps_scope;
		else if(i == WP_SHARPS && cg.snap->ps.powerups[PW_SCOPE] )
			shader = cgs.media.scope_sharps;

		if(item->xyrelation == 1){

			//selection marker
			if( i == cg.markedweapon){
				CG_FillRect(x + 2, y+2, FIELD_WIDTH - 4, FIELD_WIDTH-4, color2[2]);
			} else {
				CG_FillRect(x + 2, y+2, FIELD_WIDTH - 4, FIELD_WIDTH-4, color2[1]);
			}

			// draw weapon icon
			CG_DrawPic( x + 2, y+4, FIELD_WIDTH-4 , FIELD_WIDTH - 8, shader );

			//draw field
			CG_DrawPic( x, y, FIELD_WIDTH, FIELD_WIDTH, cgs.media.hud_wp_box_quad);

			weapon = 0;
			switch (bg_weaponlist[ i ].clip) {
			case WP_GATLING_CLIP:
				weapon = i;
				ammowidth = AMMO_SHARP_WIDTH;
				clip = cgs.media.hud_ammo_sharps;
			}

			// Draw the ammo amount with ammo pics
			if (weapon && cg.snap->ps.ammo[ weapon ] > 0) {
				for (ammo = 0; ammo < cg.snap->ps.ammo[ weapon ]; ammo++) {
					CG_DrawPic( x+4 + FIELD_WIDTH, y+4 + (ammo * (AMMO_HEIGHT + 1)), ammowidth, AMMO_HEIGHT, clip );
				}
			}

			y += FIELD_WIDTH;

		} else {
			//selection marker
			if ( i == cg.markedweapon
				&& (bg_weaponlist[i].wp_sort != WPS_PISTOL
				|| (bg_weaponlist[i].wp_sort == WPS_PISTOL
					&& (
						!( cg.snap->ps.stats[STAT_FLAGS] & SF_SEC_PISTOL )
						|| (( cg.snap->ps.stats[STAT_FLAGS] & SF_SEC_PISTOL ) && cg.markedfirstpistol)
						)
					)
				)
			) {
				CG_FillRect(x + 2, y+2, FIELD_WIDTH - 4, FIELD_HEIGHT-4, color2[2]);
			} else {
				CG_FillRect(x + 2, y+2, FIELD_WIDTH - 4, FIELD_HEIGHT-4, color2[1]);
			}

			// draw weapon icon
			CG_DrawPic( x + 2, y+4, FIELD_WIDTH-4 , FIELD_HEIGHT - 8, shader );

			//draw field
			CG_DrawPic( x, y, FIELD_WIDTH, FIELD_HEIGHT, cgs.media.hud_wp_box);

			weapon = 0;
			switch (bg_weaponlist[ i ].clip) {
			case WP_BULLETS_CLIP:
				weapon = i;
				ammowidth = AMMO_BULLET_WIDTH;
				clip = cgs.media.hud_ammo_bullet;

				if (cg.snap->ps.stats[STAT_FLAGS] & SF_SEC_PISTOL)
				// The top pistol is the left one when the player has two pistols
					weapon = WP_AKIMBO;
				break;
			case WP_CART_CLIP:
				weapon = i;
				ammowidth = AMMO_CART_WIDTH;
				clip = cgs.media.hud_ammo_cart;
				break;
			case WP_SHARPS_CLIP:
				weapon = i;
				ammowidth = AMMO_SHARP_WIDTH;
				clip = cgs.media.hud_ammo_sharps;
				break;
			case WP_SHELLS_CLIP:
				weapon = i;
				ammowidth = AMMO_SHELL_WIDTH;
				clip = cgs.media.hud_ammo_shell;
				break;
			}

			// Draw the ammo amount with ammo pics
			if (weapon && cg.snap->ps.ammo[ weapon ] > 0) {
				for (ammo = 0; ammo < cg.snap->ps.ammo[ weapon ]; ammo++) {
					CG_DrawPic( x+4 + FIELD_WIDTH, y+4 + (ammo * (AMMO_HEIGHT + 1)), ammowidth, AMMO_HEIGHT, clip );
				}
			}
			else if ( ( i == WP_KNIFE || i == WP_DYNAMITE || i == WP_MOLOTOV )
				&& cg.snap->ps.ammo[ i ] > 1) {
			// Display number of knives, dynamites and molotoves
				CG_DrawSmallStringColor(x+4 + FIELD_WIDTH, y+6 + FIELD_HEIGHT / 2, va("x%i", cg.snap->ps.ammo[ i ]), color2[0]);
			}

			// if player has pistol: give option to combine it with another weapon
			/*if(bg_weaponlist[cg.snap->ps.weapon].wp_sort == WPS_PISTOL &&
				bg_weaponlist[i].wp_sort == WPS_PISTOL &&
				i != cg.snap->ps.weapon && !cg.snap->ps.weapon2) {
				CG_DrawStringExt( x+66, y+24, "FIRE2: Add", color2[3], qtrue, qfalse, 6, 15, -1);
			}*/

			y += FIELD_HEIGHT;
		}
	}

	// add akimbo pistol
	if(akimbo){
		int weapon1, weapon2;
		int leftweapon, rightweapon;

		weapon1 = BG_SearchTypeWeapon(WPS_PISTOL, cg.snap->ps.stats[STAT_WEAPONS], 0);
		if (weapon1 < WP_PEACEMAKER && cg.weaponSelect == WP_PEACEMAKER
		                    && cg.snap->ps.stats[STAT_WEAPONS] & (1 << WP_PEACEMAKER))
		// Player holds a Peacemaker and has a Remington58 or a Schofield.
		// The Peacemaker will be the one in the right hand.
			weapon2 = WP_PEACEMAKER;
		else if (weapon1 < WP_SCHOFIELD && cg.weaponSelect == WP_SCHOFIELD
		                     && cg.snap->ps.stats[STAT_WEAPONS] & (1 << WP_SCHOFIELD))
		// Player holds a Schofield and has a Remington58.
		// The Schofield will be the one in the right hand.
			weapon2 = WP_SCHOFIELD;
		else
			weapon2 = BG_SearchTypeWeapon(WPS_PISTOL, cg.snap->ps.stats[STAT_WEAPONS], weapon1);

		if (cg.snap->ps.stats[STAT_FLAGS] & SF_SEC_PISTOL) {
		// Player has two equal pistols
			leftweapon = WP_AKIMBO;
			rightweapon = weapon1;
		}
		else {
		// The player has two different pistols
		// Try to figure out what the left one is, and what the right one is
			if (cg.weaponSelect == weapon1) {
			// Player holds the weakest pistol, so the left one is the strongest
				leftweapon = weapon2;
				rightweapon = weapon1;
			}
			else if (cg.weaponSelect == weapon2) {
			// Player holds the strongest pistol, so the left one is the weakest
				leftweapon = weapon1;
				rightweapon = weapon2;
			}
			else if (cg.weaponSelect == WP_AKIMBO) {
			// Player holds both
				leftweapon = cg.snap->ps.weapon;
				rightweapon = cg.snap->ps.weapon2;
			}
			else {
			// Player doesn't hold any pistols
				leftweapon = weapon1;		// The weakest one
				rightweapon = weapon2;	// The strongest one
			}
		}

		if (cg.snap->ps.stats[STAT_FLAGS] & SF_SEC_PISTOL
			&& !singlepistolSelect) {
		// Player has two equal pistols.  Draw the second one here,
		// but *ONLY* if the player is not already in single pistol mode
			if( weapon1 == cg.markedweapon && !cg.markedfirstpistol){
			// Second pistol is marked
				CG_FillRect(x + 2, y+2, FIELD_WIDTH - 4, FIELD_HEIGHT-4, color2[2]);
			} else {
				CG_FillRect(x + 2, y+2, FIELD_WIDTH - 4, FIELD_HEIGHT-4, color2[1]);
			}

			// draw weapon icon
			CG_DrawPic( x + 2, y+4, FIELD_WIDTH-4 , FIELD_HEIGHT - 8, cg_weapons[weapon1].weaponIcon );

			//draw field
			CG_DrawPic( x, y, FIELD_WIDTH, FIELD_HEIGHT, cgs.media.hud_wp_box);

			// Draw the ammo amount with the bullet pic
			if (cg.snap->ps.ammo[ weapon1 ] > 0) {
				for (ammo = 0; ammo < cg.snap->ps.ammo[ weapon1 ]; ammo++) {
					CG_DrawPic( x+4 + FIELD_WIDTH, y+4 + (ammo * (AMMO_HEIGHT + 1)), AMMO_BULLET_WIDTH, AMMO_HEIGHT, cgs.media.hud_ammo_bullet );
				}
			}

			y += FIELD_HEIGHT;
		}

		//selection marker
		if( WP_AKIMBO == cg.markedweapon){
			CG_FillRect(x + 2, y+2, FIELD_WIDTH - 4, FIELD_HEIGHT-4, color2[2]);
		} else {
			CG_FillRect(x + 2, y+2, FIELD_WIDTH - 4, FIELD_HEIGHT-4, color2[1]);
		}

		// draw weapon icon
		if(cg.snap->ps.stats[STAT_FLAGS] & SF_SEC_PISTOL){
			CG_DrawPic( x + 2, y+4, FIELD_WIDTH-4 , FIELD_HEIGHT - 8, cg_weapons[weapon1].weaponIcon );
		} else {
			// Draw the weapon icon on the left side
			CG_DrawPic( x + 2, y+4, FIELD_WIDTH-4 , FIELD_HEIGHT - 8, cg_weapons[leftweapon].weaponIcon );
		}
		// Draw the weapon icon on the right side
		CG_DrawPic( x-2+FIELD_WIDTH, y-4+FIELD_HEIGHT, -(FIELD_WIDTH-4) , -(FIELD_HEIGHT - 8), cg_weapons[rightweapon].weaponIcon );

		//draw field
		CG_DrawPic( x, y, FIELD_WIDTH, FIELD_HEIGHT, cgs.media.hud_wp_box);

		// Draw the ammo amount with the bullet image
		// on both the left side and the right side
		if (cg.snap->ps.ammo[ leftweapon ] > 0) {
			for (ammo = 0; ammo < cg.snap->ps.ammo[ leftweapon ]; ammo++) {
				CG_DrawPic( x-4 - AMMO_BULLET_WIDTH, y+4 + (ammo * (AMMO_HEIGHT + 1)), AMMO_BULLET_WIDTH, AMMO_HEIGHT, cgs.media.hud_ammo_bullet );
			}
		}
		if (cg.snap->ps.ammo[ rightweapon ] > 0) {
			for (ammo = 0; ammo < cg.snap->ps.ammo[ rightweapon ]; ammo++) {
				CG_DrawPic( x+4 + FIELD_WIDTH, y+4 + (ammo * (AMMO_HEIGHT + 1)), AMMO_BULLET_WIDTH, AMMO_HEIGHT, cgs.media.hud_ammo_bullet );
			}
		}

		y += FIELD_HEIGHT;
	}



	// draw the selected name
	if ( cg_weapons[ cg.markedweapon ].item || cg.markedweapon == WP_AKIMBO) {

		if(cg.markedweapon == WP_AKIMBO)
			name = "Double Pistols";
		else
			name = cg_weapons[ cg.markedweapon ].item->pickup_name;

		if ( name ) {
			w = CG_DrawStrlen( name ) * BIGCHAR_WIDTH;
			//x = ( SCREEN_WIDTH - w ) / 2;
			CG_DrawBigStringColor(x, y, name, color2[0]);
		}
	}

	//trap_R_SetColor( NULL );
}


/*
===============
CG_WeaponSelectable
===============
*/
static qboolean CG_WeaponSelectable( int i ) {
	if (bg_weaponlist[i].clip){
		if ( !cg.snap->ps.ammo[i] && !cg.snap->ps.ammo[bg_weaponlist[i].clip]) {
			return qfalse;
		}
	} else {
		if ( !cg.snap->ps.ammo[i]) {
			return qfalse;
		}
	}

	if ( ! (cg.snap->ps.stats[ STAT_WEAPONS ] & ( 1 << i ) ) ) {
		return qfalse;
	}

	return qtrue;
}

/*
===============
CG_NextWeapon_f
===============
*/
void CG_NextWeapon_f( void ) {
	int		num;
	int		i, j;
	int startnum;
	// is it the first time weapon_f was called since the last wp-change?
	qboolean first_time = !cg.markedweapon;

	// has the player got one pistol in hand ?
	qboolean singlepistolSelect = cg.weaponSelect != WP_AKIMBO
		&& bg_weaponlist[ cg.weaponSelect ].wp_sort == WPS_PISTOL;

	if ( !cg.snap ) {
		return;
	}
	if ( cg.snap->ps.pm_flags & PMF_FOLLOW ) {
		return;
	}

	if( cg.introend > cg.time)
		return;

	if(cg.snap->ps.stats[STAT_GATLING_MODE])
		return;

	if(first_time) { // signal BUTTON_CHOOSE_MODE to the server
		trap_SendConsoleCommand("+button14; wait; -button14");
	}

	if(!cg.markedweapon)
		startnum = cg.weaponSelect;
	else
		startnum = cg.markedweapon;

	for(j = 0; j < WPS_NUM_ITEMS; j++){

		if(startnum == WP_NONE){
			num = WPS_PISTOL + j;
		} else if(startnum == WP_AKIMBO){
			num = WPS_PISTOL + 1 + j;
		} else {
			num = bg_weaponlist[startnum].wp_sort + j;
		}

		if(num >= WPS_NUM_ITEMS){
			num = WPS_MELEE;
			cg.markedweapon = WP_KNIFE;
			break;
		}

		// Player has two equal pistols.
		// Detect a change from knife to a single pistol.
		// Weapon is set to the first pistol.
		if ( ( cg.snap->ps.stats[STAT_FLAGS] & SF_SEC_PISTOL )
			&& num == WPS_PISTOL && j > 0)
			cg.markedfirstpistol = qtrue;

		for(i = 1; i <= WP_NUM_WEAPONS; i++){
			int weapon;

			// check if akimbo has to be set
			if(num == WPS_PISTOL && weapon+1 == WP_NUM_WEAPONS &&
				(BG_CountTypeWeapons(WPS_PISTOL, cg.snap->ps.stats[STAT_WEAPONS]) >= 2 ||
				( ( cg.snap->ps.stats[STAT_FLAGS] & SF_SEC_PISTOL )
					&& ( !cg.markedfirstpistol || singlepistolSelect )))) {
					// One of the following is true :
					// 1) Player has two different pistols.
					// 2) Player has two equal pistols and the second pistol is selected.
					// 3) Player has two equal pistols and has already got one pistol in hand.
				cg.markedweapon = WP_AKIMBO;
				goto end;
				break;

			}
			else if (num == WPS_PISTOL
				&& ( cg.snap->ps.stats[STAT_FLAGS] & SF_SEC_PISTOL )
				&& cg.markedfirstpistol && !singlepistolSelect
				&& startnum != WP_AKIMBO && bg_weaponlist[startnum].wp_sort == WPS_PISTOL) {
			// Player has two equal pistols.
			// Change from the first pistol to the second one.

			// This must happen *ONLY* if the player is not already in single pistol mode
				cg.markedfirstpistol = qfalse;
				cg.markedweapon = startnum;
				goto end;
				break;

			} else if(!cg.markedweapon){
				if(cg.weaponSelect == WP_AKIMBO)
					weapon = WP_PEACEMAKER+i;
				else
					weapon = cg.weaponSelect+i;

				if(weapon == cg.weaponSelect)
					continue;

				if(weapon >= WP_NUM_WEAPONS)
					weapon -= WP_NUM_WEAPONS + 1;

			} else if(cg.markedweapon == WP_AKIMBO){
				cg.markedweapon = BG_SearchTypeWeapon(num, cg.snap->ps.stats[STAT_WEAPONS], 0);

				if(!cg.markedweapon)
					continue;

				goto end;
				break;
			} else {
				weapon = cg.markedweapon+i;
			}

			if(bg_weaponlist[weapon].wp_sort == num &&
				( cg.snap->ps.stats[STAT_WEAPONS] & ( 1 << weapon ))){

				cg.markedweapon = weapon;
				goto end;
				break;
			}
		}
	}
end:

	if(first_time)
		trap_Cvar_Set("cl_menu", "1");
}

/*
===============
CG_PrevWeapon_f
===============
*/
void CG_PrevWeapon_f( void ) {
	int		num;
	int		i, j;
	int startnum;
	// is it the first time weapon_f was called since the last wp-change?
	qboolean first_time = !cg.markedweapon;

	// has the player got one pistol in hand ?
	qboolean singlepistolSelect = cg.weaponSelect != WP_AKIMBO
		&& bg_weaponlist[ cg.weaponSelect ].wp_sort == WPS_PISTOL;

	if ( !cg.snap ) {
		return;
	}
	if ( cg.snap->ps.pm_flags & PMF_FOLLOW ) {
		return;
	}

	if( cg.introend > cg.time)
		return;

	if(cg.snap->ps.stats[STAT_GATLING_MODE])
		return;

	if(first_time) { // signal BUTTON_CHOOSE_MODE to the server
		trap_SendConsoleCommand("+button14; wait; -button14");
	}

	if(!cg.markedweapon)
		startnum = cg.weaponSelect;
	else
		startnum = cg.markedweapon;

	for(j = 0; j < WPS_NUM_ITEMS; j++){

		if(startnum != WP_AKIMBO && startnum != WP_NONE){
			num = bg_weaponlist[startnum].wp_sort - j;
		} else if(startnum == WP_NONE){
			num = WPS_OTHER - j;
		} else if(startnum == WP_AKIMBO){
			num = WPS_PISTOL - j;
		}

		if(num <= 0){
			num = WPS_OTHER - j + 1;
			cg.markedweapon = WP_NUM_WEAPONS;
			//break;
		}

		// Player has two equal pistols.
		// Detect a change from akimbo to a single pistol.
		if ( ( cg.snap->ps.stats[STAT_FLAGS] & SF_SEC_PISTOL )
			&& num == WPS_PISTOL && startnum == WP_AKIMBO ) {
		// If the player hasn't already got a pistol in hand,
		// set to the second one from akimbo.
			if (!singlepistolSelect)
				cg.markedfirstpistol = qfalse;
			else // else, set to the first one from akimbo.
				cg.markedfirstpistol = qtrue;
		}

		for(i = 1; i <= WP_NUM_WEAPONS; i++){
			int weapon;

			// check if akimbo has to be set
			if(num == WPS_GUN && weapon-1 == 0 &&
				(BG_CountTypeWeapons(WPS_PISTOL, cg.snap->ps.stats[STAT_WEAPONS]) >= 2 ||
				(cg.snap->ps.stats[STAT_FLAGS] & SF_SEC_PISTOL))){

				cg.markedweapon = WP_AKIMBO;
				goto end;
				break;

			}
			else if (num == WPS_PISTOL
				&& ( cg.snap->ps.stats[STAT_FLAGS] & SF_SEC_PISTOL )
				&& !cg.markedfirstpistol && startnum != WP_AKIMBO
				&& bg_weaponlist[startnum].wp_sort == WPS_PISTOL) {
			// Player has two equal pistols.
			// Change from the second pistol to the first one.
				cg.markedfirstpistol = qtrue;
				cg.markedweapon = startnum;
				goto end;
				break;

			} else if(!cg.markedweapon){
				weapon = cg.weaponSelect-i;

				if(weapon == WP_NUM_WEAPONS)
					weapon--;

				if(weapon == cg.weaponSelect)
					continue;

				if(weapon == 0)
					weapon = WP_NUM_WEAPONS-1;

			} else if(cg.markedweapon == WP_AKIMBO){
				weapon = WP_NUM_WEAPONS;
				cg.markedweapon = weapon;
			} else {
				weapon = cg.markedweapon-i;
			}

			if(bg_weaponlist[weapon].wp_sort == num &&
				( cg.snap->ps.stats[STAT_WEAPONS] & ( 1 << weapon ))){

				cg.markedweapon = weapon;
				goto end;
				break;
			}
		}
	}
end:

	if(first_time)
		trap_Cvar_Set("cl_menu", "1");
}

/*
===============
CG_Weapon_f
===============
*/
void CG_Weapon_f( void ) {
	int		num;
	int		i;
	int		startnum;
	// is it the first time weapon_f was called since the last wp-change?
	qboolean first_time = !cg.markedweapon;

	// has the player got one pistol in hand ?
	qboolean singlepistolSelect = cg.weaponSelect != WP_AKIMBO
		&& bg_weaponlist[ cg.weaponSelect ].wp_sort == WPS_PISTOL;

	if ( !cg.snap ) {
		return;
	}
	if ( cg.snap->ps.pm_flags & PMF_FOLLOW ) {
		return;
	}

	if( cg.introend > cg.time)
		return;

	if(cg.snap->ps.stats[STAT_GATLING_MODE])
		return;

	num = atoi( CG_Argv( 1 ) );

	if ( num < 1 || num > 4 ) {
		return;
	}

	if(first_time) { // signal BUTTON_CHOOSE_MODE to the server
		trap_SendConsoleCommand("+button14; wait; -button14");
	}

	if(!cg.markedweapon)
		startnum = cg.weaponSelect;
	else
		startnum = cg.markedweapon;

	// Player has two equal pistols.
	// Change from another weapon type to pistol weapon type
	// The first pistol must be marked
	if ( ( cg.snap->ps.stats[STAT_FLAGS] & SF_SEC_PISTOL )
		&& num == WPS_PISTOL
		&& startnum < WP_NUM_WEAPONS && bg_weaponlist[ startnum ].wp_sort != WPS_PISTOL) {
		cg.markedfirstpistol = qtrue;
	}

	for(i = 1; i <= WP_NUM_WEAPONS; i++){
		int weapon;

		// check if akimbo has to be set
		if(num == WPS_PISTOL && weapon+1 == WP_NUM_WEAPONS &&
			(BG_CountTypeWeapons(num, cg.snap->ps.stats[STAT_WEAPONS]) >= 2 ||
			( ( cg.snap->ps.stats[STAT_FLAGS] & SF_SEC_PISTOL )
					&& ( !cg.markedfirstpistol || singlepistolSelect )))) {
					// One of the following is true :
					// 1) Player has two different pistols.
					// 2) Player has two equal pistols and the second pistol is selected.
					// 3) Player has two equal pistols and has already got one pistol in hand.

			cg.markedweapon = WP_AKIMBO;
			break;

		}
		else if (num == WPS_PISTOL
			&& ( cg.snap->ps.stats[STAT_FLAGS] & SF_SEC_PISTOL )
			&& cg.markedfirstpistol && !singlepistolSelect
			&& startnum != WP_AKIMBO && bg_weaponlist[startnum].wp_sort == WPS_PISTOL) {
		// Player has two equal pistols.
		// Change from the first pistol to the second one.

		// This must happen *ONLY* if the player is not already in single pistol mode
			cg.markedfirstpistol = qfalse;
			if (!cg.markedweapon)
				cg.markedweapon = cg.weaponSelect;
			break;

		} else if(!cg.markedweapon){
			int delta;

			if(cg.weaponSelect == WP_AKIMBO && num == WPS_PISTOL){
				cg.markedweapon = BG_SearchTypeWeapon(num, cg.snap->ps.stats[STAT_WEAPONS], 0);

				// Player has two equal pistols.
				// Switch from akimbo to the first pistol
				if (cg.snap->ps.stats[STAT_FLAGS] & SF_SEC_PISTOL)
					cg.markedfirstpistol = qtrue;
				break;
			}

			if(bg_weaponlist[cg.weaponSelect].wp_sort != num)
				weapon = WP_KNIFE + i;
			else
				weapon = cg.weaponSelect+i;

			if(weapon >= WP_NUM_WEAPONS){
				delta = weapon - WP_NUM_WEAPONS;
				weapon = WP_KNIFE + delta;
			}

			//if(weapon == cg.weaponSelect)
			//	continue;

		} else if(cg.markedweapon == WP_AKIMBO){
			cg.markedweapon = BG_SearchTypeWeapon(num, cg.snap->ps.stats[STAT_WEAPONS], 0);

			// Player has two equal pistols.
			// Switch from akimbo to the first pistol
			if (cg.snap->ps.stats[STAT_FLAGS] & SF_SEC_PISTOL)
				cg.markedfirstpistol = qtrue;
			break;
		} else {
			// always begin at the top of the list when changing the wp_sort
			if(bg_weaponlist[cg.markedweapon].wp_sort != num)
				weapon = WP_KNIFE+i;
			else
				weapon = cg.markedweapon+i;
		}

		if(weapon >= WP_NUM_WEAPONS)
			weapon = weapon - WP_NUM_WEAPONS +1;

		if(bg_weaponlist[weapon].wp_sort == num &&
			( cg.snap->ps.stats[STAT_WEAPONS] & ( 1 << weapon ))){

			cg.markedweapon = weapon;
			break;
		}
	}

	if(first_time)
		trap_Cvar_Set("cl_menu", "1");
}

/*
===============
CG_LastUsedWeapon_f
===============
*/
void CG_LastUsedWeapon_f(void) {
	int honeymug;

	if ( !cg.snap ) {
		return;
	}
	if ( cg.snap->ps.pm_flags & PMF_FOLLOW ) {
		return;
	}
	if( cg.introend > cg.time)
		return;

	if(cg.snap->ps.stats[STAT_GATLING_MODE])
		return;

	//don't change if lastusedweapon is null
    if (!cg.lastusedweapon) {
		return;
	}
	//return if player doesn't have two pistols when using akimbo
	if (cg.lastusedweapon == WP_AKIMBO
	    && BG_CountTypeWeapons(WPS_PISTOL, cg.snap->ps.stats[STAT_WEAPONS]) < 2
	    && !(cg.snap->ps.stats[STAT_FLAGS] & SF_SEC_PISTOL)) {
		return;
	}
	//return if player doesn't has exactly one more pistol of the same kind.
	//NB : WP_SEC_PISTOL is only used when the player has two equal pistols.
	if (cg.lastusedweapon == WP_SEC_PISTOL
		&& ( BG_CountTypeWeapons(WPS_PISTOL, cg.snap->ps.stats[STAT_WEAPONS]) != 1
			|| !(cg.snap->ps.stats[STAT_FLAGS] & SF_SEC_PISTOL) )) {
		return;
	}
	//return if player doesn't have the previously used weapon
	if (cg.lastusedweapon != WP_AKIMBO
		&& cg.lastusedweapon != WP_SEC_PISTOL
		&& !(cg.snap->ps.stats[STAT_WEAPONS] & (1 << cg.lastusedweapon)) ) {
		return;
	}

	honeymug = cg.lastusedweapon;
	// Keep cg.lastusedweapon to WP_SEC_PISTOL if the current selected weapon
	// is the other single same type pistol, because it will become the last used
	// weapon as the secondary same type pistol.
	if (honeymug == WP_SEC_PISTOL
		&& cg.weaponSelect != WP_AKIMBO
		&& bg_weaponlist[ cg.weaponSelect ].wp_sort == WPS_PISTOL) {
		// cg.weaponSelect will be restored in
		// -> ./cgame/cg_view.c, CG_DrawActiveFrame()
		cg._weaponSelect = cg.weaponSelect;
	}
	else
		cg.lastusedweapon = cg.weaponSelect;
	cg.weaponSelect = honeymug;
	cg.weaponSelectTime = cg.time;
}


/*
===================
CG_OutOfAmmoChange

The current weapon has just run out of ammo

***not used anymore***
===================
*/
void CG_OutOfAmmoChange( void ) {
	int		i;

	cg.weaponSelectTime = cg.time;

	for ( i = WP_NUM_WEAPONS -1 ; i > 0 ; i-- ) {
		if ( CG_WeaponSelectable( i ) ) {
			cg.weaponSelect = i;
			break;
		}
	}
}



/*
===================================================================================================

WEAPON EVENTS

===================================================================================================
*/

/*
================
CG_FireWeapon

Caused by an EV_FIRE_WEAPON event
================
*/
void CG_FireWeapon( centity_t *cent, qboolean altfire, int weapon ) {
	entityState_t *ent;
	int				c;
	weaponInfo_t	*weap;

	ent = &cent->currentState;
	if ( weapon == WP_NONE ) {
		return;
	}
	if ( weapon >= WP_NUM_WEAPONS ) {
		CG_Error( "CG_FireWeapon: weapon >= WP_NUM_WEAPONS" );
		return;
	}
	weap = &cg_weapons[ weapon ];

	// mark the entity as muzzle flashing, so when it is added it will
	// append the flash to the weapon model
	if(!altfire || !cent->currentState.frame){
		cent->muzzleFlashTime = cg.time;

		if((bg_weaponlist[weapon].wp_sort == WPS_PISTOL ||
			bg_weaponlist[weapon].wp_sort == WPS_GUN) && cg_gunsmoke.integer){

			if(!cent->smokeTime)
				cent->smokeTime = cg.time + 500;
			else
				cent->smokeTime = cg.time;

			cent->lastSmokeTime = cent->smokeTime;
		}
	} else {
		cent->muzzleFlashTime2 = cg.time;

		if((bg_weaponlist[weapon].wp_sort == WPS_PISTOL ||
			bg_weaponlist[weapon].wp_sort == WPS_GUN) && cg_gunsmoke.integer){

			if(!cent->smokeTime2)
				cent->smokeTime2 = cg.time + 500;
			else
				cent->smokeTime2 = cg.time;

			cent->lastSmokeTime2 = cent->smokeTime2;
		}
	}

	// lightning gun only does this this on initial press
	/*if ( ent->weapon == WP_SAWEDOFF ) {
		if ( cent->pe.lightningFiring ) {
			return;
		}
	}*/

	// play quad sound if needed
	/*if ( cent->currentState.powerups & ( 1 << PW_QUAD ) ) {
		trap_S_StartSound (NULL, cent->currentState.number, CHAN_ITEM, cgs.media.quadSound );
	}*/

	// play a sound
	for ( c = 0 ; c < 4 ; c++ ) {
		if ( !weap->flashSound[c] ) {
			break;
		}
	}
	if ( c > 0 ) {
		c = rand() % c;
		if ( weap->flashSound[c] )
		{
			trap_S_StartSound( NULL, ent->number, CHAN_WEAPON, weap->flashSound[c] );
		}
	}

	// do brass ejection
	if ( weap->ejectBrassFunc && cg_brassTime.integer > 0 ) {
		weap->ejectBrassFunc( cent );
	}
}

/*
==============
CG_BottleBreak

creates 4 gibs of the bottle
==============
*/
void CG_BottleBreak(vec3_t org, vec3_t bottledir, qboolean fire, vec3_t dirs[ALC_COUNT]){
	int i;
	vec3_t dir, origin;
	vec3_t	temp;

	for(i = 0; i < 4; i++){
		VectorMA(org, 8, bottledir, origin);

		VectorSet(dir, ((rand()%10)/5)-1, ((rand()%10)/5)-1, 1);
		CG_LaunchParticle(origin, dir, cgs.media.molotovgib[i], LEBS_NONE, BOUNCE_LIGHT, 4000 + rand()%2000, /*LEF_BREAKS*/0);
	}

	// launch the predicted alcohol drops
	for(i=0; i<8;i++){
		localEntity_t *le;

		VectorCopy(dirs[i], dir);

		// just set the origin first
		VectorCopy(org, temp);
		BG_SetWhiskeyDrop(NULL, org, bottledir, dir);
		VectorCopy(org, origin);

		le = CG_LaunchSpriteParticle(origin, dir, BOUNCE_LIGHT, 4000, (LEF_PARTICLE|LEF_BLOOD));
		le->refEntity.customShader = cgs.media.whiskey_drops[rand()%6];
		le->refEntity.radius = 1.0 + (rand()%10)/6;
		le->leFlags |= LEF_WHISKEY; // this marks a whiskey drop
		le->leFlags |= LEF_REMOVE;

		if(fire){
			le->leFlags |= LEF_FIRE;
			le->lightColor[0] = 1;
		} else
			le->lightColor[0] = 0;

		le->color[0] = 1;
		le->color[1] = 1;
		le->color[2] = 1;
		le->color[3] = 1;

		// now really set up the drop
		VectorCopy(temp, org);
		BG_SetWhiskeyDrop(&le->pos, org, bottledir, dir);
		VectorCopy(le->pos.trDelta, temp);
	}
}

/*
=================
CG_MissileHitWall

Caused by an EV_MISSILE_MISS event, or directly by local bullet tracing
=================
*/
#define SMOKEPUFF_SIZE 6
void CG_MissileHitWall( int weapon, int clientNum, vec3_t origin, vec3_t dir,
					   impactSound_t soundType,
					   int surfaceFlags, int shaderNum,
					   qboolean fire, vec3_t bottledirs[ALC_COUNT],
					   int entityNum) {
	qhandle_t		mod;
	qhandle_t		mark;
	qhandle_t		shader;
	sfxHandle_t		sfx;
	float			radius_mrk;
	float			radius_exp;
	float			light;
	vec3_t			lightColor;
	localEntity_t	*le;
	//int				r;
	qboolean		alphaFade;
	qboolean		isSprite;
	int				duration;
	int				i;

	// set defaults
	mark = 0;
	radius_mrk = 32;
	radius_exp = 32;
	sfx = 0;
	mod = 0;
	shader = 0;
	light = 0;
	lightColor[0] = 0.8f;
	lightColor[1] = 0.8f;
	lightColor[2] = 0.6f;
	isSprite = qfalse;
	duration = 600;

	switch ( weapon ) {
	//default:
	case WP_KNIFE:
		sfx = cgs.media.knifehit2;
		break;
	case WP_DYNAMITE:
		mod = cgs.media.dishFlashModel;
		shader = cgs.media.dynamiteExplosionShader;
		sfx = cgs.media.sfx_rockexp;
		mark = cgs.media.burnMarkShader;
		duration = 1000;
		radius_mrk = radius_exp = 64*2.5 + rand()%30; //old 64
		light = 150;//300
		isSprite = qtrue;
		break;
	case WP_MOLOTOV:
		//mod = cgs.media.dishFlashModel;
		//shader = cgs.media.dynamiteExplosionShader;
		sfx = cgs.media.impact[3][rand()%3];
		//mark = cgs.media.burnMarkShader;
		//duration = 1000;
		//radius_mrk = radius_exp = 64;
		//light = 150;
		//isSprite = qtrue;
		break;
	case WP_REMINGTON_GAUGE:
	case WP_SAWEDOFF:
	case WP_WINCH97:
	case WP_SHARPS:
	case WP_WINCHESTER66:
	case WP_LIGHTNING:
	case WP_PEACEMAKER:
	case WP_REM58:
	case WP_SCHOFIELD:
	case WP_GATLING:
		shader = cgs.media.bulletPuffShader;
		isSprite = qtrue;

		if(weapon == WP_GATLING){
			if(!(rand()%4))
				sfx = cgs.media.sfx_ric[rand()%6];
		} else
			sfx = cgs.media.sfx_ric[rand()%6];

		radius_mrk = (rand()%1) + 2;
		radius_exp = SMOKEPUFF_SIZE;
		break;
	}

	if(surfaceFlags != -1){
		for(i=0; i<NUM_PREFIXINFO; i++){
			if((surfaceFlags & prefixInfo[i].surfaceFlags) ||
				!prefixInfo[i].surfaceFlags){
				mark = cgs.media.marks[i][rand()%2];
				radius_mrk = 3;
				break;
			}
		}
	}


	if ( sfx && weapon != WP_SAWEDOFF && weapon != WP_REMINGTON_GAUGE) {
		trap_S_StartSound( origin, ENTITYNUM_WORLD, CHAN_AUTO, sfx );
	} else if(sfx){
		if(Distance(origin, cg.refdef.vieworg) > 650)
			trap_S_StartSound( origin, ENTITYNUM_WORLD, CHAN_AUTO, sfx );
	}


	//
	// create the explosion
	//
	if ( mod || shader) {
		le = CG_MakeExplosion( origin, dir,
							   mod,	shader,
							   duration, isSprite , radius_exp, (weapon == WP_DYNAMITE));
		le->light = light;
		VectorCopy( lightColor, le->lightColor );
	}

	//
	// impact mark
	//
	if(mark){
		float red, green, blue;
		alphaFade = qtrue;

		if(shaderNum > -1){
			vec4_t	light, color;
			int		i;

			Vector4Copy(shaderInfo[shaderNum].color, color);
			CG_LightForPoint(origin, dir, light);

			for(i=0;i<3;i++){
				float temp = light[i]/255;
				temp = sqrt(temp);

				color[i] *= temp;//*0.87f + 0.13f;
			}

			red = color[0];
			green = color[1];
			blue = color[2];

		} else {
			red = 1;
			green = 1;
			blue = 1;
		}
		if((surfaceFlags & SURF_BREAKABLE) &&
			(bg_weaponlist[weapon].wp_sort == WPS_PISTOL ||
			bg_weaponlist[weapon].wp_sort == WPS_GUN))
			CG_CreateBulletHole(origin, dir, surfaceFlags, entityNum);
		else {
			CG_ImpactMark( mark, origin, dir, random()*360,red, green, blue, 1, alphaFade, radius_mrk, qfalse, -1 );
		}
	}

	if(surfaceFlags != -1){
		CG_LaunchImpactParticle( origin, dir, surfaceFlags, shaderNum, weapon, qfalse);
	}

	// break the bottle
	if(weapon == WP_MOLOTOV){
		CG_BottleBreak(origin, dir, fire, bottledirs);
	}

}


/*
=================
CG_MissileHitPlayer
=================
*/
void CG_MissileHitPlayer( int weapon, vec3_t origin, vec3_t dir, int entityNum ) {
	CG_Bleed( origin, entityNum );

	// some weapons will make an explosion with the blood, while
	// others will just make the blood
	switch ( weapon ) {
	case WP_DYNAMITE:
		CG_MissileHitWall( weapon, 0, origin, dir, IMPACTSOUND_FLESH, -1, -1, qfalse, NULL, -1 );
		break;
	case WP_MOLOTOV:
		CG_MissileHitWall( weapon, 0, origin, dir, IMPACTSOUND_FLESH, -1, -1, qfalse, NULL, -1 );
		break;
	default:
		break;
	}
}



/*
============================================================================

SHOTGUN TRACING

============================================================================
*/

/*
================
CG_ShotgunPellet
================
*/
static void CG_ShotgunPellet( vec3_t start, vec3_t origin2, vec3_t end, int skipNum, int playerhit,
							 vec3_t player, float r, float u, int weapon) {
	trace_t		tr;
	int sourceContentType, destContentType;
	int	shaderNum;
	float		damage = bg_weaponlist[weapon].damage;
	qboolean	shootthru;
	vec3_t		tr_start;
	int shootcount = 0;

	VectorCopy(start, tr_start);

cg_shotgunfire:
	shootthru = qfalse;

	if(playerhit != -1){

		shaderNum = CG_Trace_New( &tr, tr_start, NULL, NULL, end, ENTITYNUM_NONE/*skipNum*/, MASK_SHOT );

		//player is already dead
		if(tr.entityNum >= MAX_CLIENTS){
			VectorCopy(player, tr.endpos);
			tr.entityNum = playerhit;
		}

	} else {
		vec3_t dir;

		VectorSubtract(end, tr_start, dir);
		VectorNormalize(dir);

		shaderNum = CG_Trace_New( &tr, tr_start, NULL, NULL, end, ENTITYNUM_NONE/*skipNum*/, MASK_SHOT );

		while(tr.entityNum < MAX_CLIENTS){
			VectorMA( tr.endpos, 8192 * 16, dir, end);

			shaderNum = CG_Trace_New( &tr, tr.endpos, NULL, NULL, end, tr.entityNum, MASK_SHOT );
		}
	}

	sourceContentType = trap_CM_PointContents( tr_start, 0 );
	destContentType = trap_CM_PointContents( tr.endpos, 0 );

	// FIXME: should probably move this cruft into CG_BubbleTrail
	if ( sourceContentType == destContentType ) {
		if ( sourceContentType & CONTENTS_WATER ) {
			CG_BubbleTrail( tr_start, tr.endpos, 32 );
		}
	} else if ( sourceContentType & CONTENTS_WATER ) {
		trace_t trace;

		trap_CM_BoxTrace_New( &trace, end, tr_start, NULL, NULL, 0, CONTENTS_WATER );
		CG_BubbleTrail( tr_start, trace.endpos, 32 );
	} else if ( destContentType & CONTENTS_WATER ) {
		trace_t trace;

		shaderNum = trap_CM_BoxTrace_New( &trace, tr_start, end, NULL, NULL, 0, CONTENTS_WATER );
		CG_BubbleTrail( tr.endpos, trace.endpos, 32 );

		//water particles
		CG_LaunchImpactParticle( trace.endpos, trace.plane.normal, SURF_WATER, shaderNum, WP_SAWEDOFF, qfalse);
	}

	if (  tr.surfaceFlags & SURF_NOIMPACT ) {
		return;
	}

	if ( cg_entities[tr.entityNum].currentState.eType == ET_PLAYER) {
		vec3_t dir;

		VectorAdd(cg_entities[tr.entityNum].currentState.pos.trDelta, tr.plane.normal, dir);
		VectorNormalize(dir);

		CG_LaunchImpactParticle(tr.endpos, dir, -1, -1, -1, qtrue);
		//CG_MissileHitPlayer( WP_REMINGTON_GAUGE, tr.endpos, tr.plane.normal, tr.entityNum );
		if(tr.entityNum == cg.snap->ps.clientNum){
			//"shake" the view
			cg.landAngleChange = (rand()%30)-15;
			cg.landTime = cg.time;
			cg.landChange = 0;
		}
	} else {
		vec3_t	endpos;
		vec3_t		forward, right, up;

		if ( tr.surfaceFlags & SURF_NOIMPACT ) {
			// SURF_NOIMPACT will not make a flame puff or a mark
			return;
		}

		if(cg_entities[tr.entityNum].currentState.eType == ET_BREAKABLE)
			tr.surfaceFlags |= SURF_BREAKABLE;

		if ( tr.surfaceFlags & SURF_METAL ) {
			CG_MissileHitWall( WP_REMINGTON_GAUGE, 0, tr.endpos, tr.plane.normal, IMPACTSOUND_METAL, tr.surfaceFlags, shaderNum, qfalse, NULL, tr.entityNum );
		} else {
			CG_MissileHitWall( WP_REMINGTON_GAUGE, 0, tr.endpos, tr.plane.normal, IMPACTSOUND_DEFAULT, tr.surfaceFlags, shaderNum, qfalse, NULL, tr.entityNum );
		}

		// check if bullets are gone thru
#define DAM_FACTOR 0.5f
		// modify damage for a short time, cause shotguns have particles with low damage
		damage *= DAM_FACTOR;

		// look if the weapon is able to shoot through the wall
		shootthru = BG_ShootThruWall(&damage, tr.endpos, tr_start, tr.surfaceFlags, endpos,
			CG_Trace);

		damage /= DAM_FACTOR;

		// derive the right and up vectors from the forward vector, because
		// the client won't have any other information
		VectorNormalize2( origin2, forward );
		PerpendicularVector( right, forward );
		CrossProduct( forward, right, up );

		if(shootthru){

			shootcount++;
			if(shootcount >= 10)
				return;

			// first make a hole on the other side of the wall
			shaderNum = CG_Trace_New( &tr, endpos, NULL, NULL, tr_start, ENTITYNUM_NONE/*skipNum*/, (MASK_SOLID|CONTENTS_BODY) );

			if(cg_entities[tr.entityNum].currentState.eType == ET_BREAKABLE)
				tr.surfaceFlags |= SURF_BREAKABLE;

			if ( tr.surfaceFlags & SURF_METAL ) {
				CG_MissileHitWall( WP_REMINGTON_GAUGE, 0, tr.endpos, tr.plane.normal, IMPACTSOUND_METAL, tr.surfaceFlags, shaderNum, qfalse, NULL, tr.entityNum );
			} else {
				CG_MissileHitWall( WP_REMINGTON_GAUGE, 0, tr.endpos, tr.plane.normal, IMPACTSOUND_DEFAULT, tr.surfaceFlags, shaderNum, qfalse, NULL,tr.entityNum );
			}

			VectorCopy(endpos, tr_start);
			VectorMA( tr_start, 8192 * 16, forward, end);
			VectorMA (end, r, right, end);
			VectorMA (end, u, up, end);
			goto cg_shotgunfire;
		}

	}
}

/*
================
CG_ShotgunPattern

Perform the same traces the server did to locate the
hit splashes (FIXME: ranom seed isn't synce anymore)
================
*/
static void CG_ShotgunPattern( vec3_t origin, vec3_t origin2, int otherEntNum, entityState_t *es ) {
	int		i;
	float		r, u;
	float           spread_dist , spread_angle , angle_shift , current_angle_shift ;
	float           max_spread_circle , current_spread_circle , extra_circle ;
	int             current_spread_cell , pellet_per_circle , extra_center_pellet , current_pellet_per_circle ;
	vec3_t		end;
	vec3_t		forward, right, up;
	int count = bg_weaponlist[es->weapon].count;
	int			seed = es->eventParm;

	if(es->angles[0])
		count *=2;

	// derive the right and up vectors from the forward vector, because
	// the client won't have any other information
	VectorNormalize2( origin2, forward );
	PerpendicularVector( right, forward );
	CrossProduct( forward, right, up );
	
	if ( cg_exp_shotgunpattern.integer ) {
			
		// Joe Kari: new experimental shotgun pattern //

		// generate the "random" spread pattern

		switch ( count )  {
			case 14 :
			case 28 :
				pellet_per_circle = 7 ;
				extra_center_pellet = 0 ;
				break ;
			case 6 :
				pellet_per_circle = 5 ;
				extra_center_pellet = 1 ;
				break ;
			default :
				pellet_per_circle = 6 ;
				extra_center_pellet = 0 ;
		}
		max_spread_circle = count / pellet_per_circle ;
		if ( max_spread_circle < 1 )  max_spread_circle = 1 ;
		angle_shift = Q_random( &seed ) * M_PI * 2.0f ;
		if ( extra_center_pellet > 0 )  {
			extra_circle = (float)extra_center_pellet / (float)pellet_per_circle ;
			max_spread_circle += extra_circle ;
		}

		for ( i = - extra_center_pellet ; i < count - extra_center_pellet ; i++ ) {

			if ( extra_center_pellet > 0 )  {
				if ( i < 0 )  {
					current_spread_circle = 0 ;
					current_pellet_per_circle = extra_center_pellet ;
				}
				else  {
					current_spread_circle = extra_circle + i / pellet_per_circle ;
					current_pellet_per_circle = pellet_per_circle ;
				}
				current_spread_cell = i - current_spread_circle * current_pellet_per_circle ;
			}
			else  {
				current_spread_circle = i / pellet_per_circle ;
				current_pellet_per_circle = pellet_per_circle ;
				current_spread_cell = i - current_spread_circle * current_pellet_per_circle ;
			}
			current_angle_shift = angle_shift + current_spread_circle * M_PI / (float)current_pellet_per_circle ;

			spread_dist = ( current_spread_circle + Q_random( &seed ) ) / max_spread_circle * bg_weaponlist[es->weapon].spread * 16 ;
			// spread adjustement to keep the same spread feeling:
			spread_dist *= 1.4 ;
			
			spread_angle = current_angle_shift + ( (float)current_spread_cell + Q_random( &seed ) ) * M_PI * 2.0f / (float)current_pellet_per_circle ;

			r = sin( spread_angle ) * spread_dist ;
			u = cos( spread_angle ) * spread_dist ;

			VectorMA( origin, 8192 * 16, forward, end);
			VectorMA (end, r, right, end);
			VectorMA (end, u, up, end);

			if((i+1) < 16 && ((int)es->angles[1] & (1 << (i+1)))){
				vec3_t player;

				VectorCopy(cg_entities[es->clientNum].lerpOrigin, player);

				CG_ShotgunPellet( origin, origin2, end, otherEntNum, es->clientNum, player, r, u, es->weapon );
			} else {
				CG_ShotgunPellet( origin, origin2, end, otherEntNum, -1, origin, r, u, es->weapon );
			}
		
		}

		// End (Joe Kari) //

			
	} else {
		
		// generate the "random" spread pattern
		for ( i = 0 ; i < count ; i++ ) {
			r = Q_crandom( &seed ) * bg_weaponlist[es->weapon].spread * 16;
			u = Q_crandom( &seed ) * bg_weaponlist[es->weapon].spread * 16;
			VectorMA( origin, 8192 * 16, forward, end);
			VectorMA (end, r, right, end);
			VectorMA (end, u, up, end);

			if((i+1) < 16 && ((int)es->angles[1] & (1 << (i+1)))){
				vec3_t player;

				VectorCopy(cg_entities[es->clientNum].lerpOrigin, player);

				CG_ShotgunPellet( origin, origin2, end, otherEntNum, es->clientNum, player, r, u, es->weapon );
			} else {
				CG_ShotgunPellet( origin, origin2, end, otherEntNum, -1, origin, r, u, es->weapon );
			}
		}
	}
}

/*
==============
CG_ShotgunFire
==============
*/
void CG_ShotgunFire( entityState_t *es ) {
	vec3_t	v;
	int		contents;

	VectorSubtract( es->origin2, es->pos.trBase, v );
	VectorNormalize( v );
	VectorScale( v, 32, v );
	VectorAdd( es->pos.trBase, v, v );

	if ( cgs.glconfig.hardwareType != GLHW_RAGEPRO ) {
		// ragepro can't alpha fade, so don't even bother with smoke
		vec3_t			up;

		contents = trap_CM_PointContents( es->pos.trBase, 0 );
		if ( !( contents & CONTENTS_WATER ) && (cg.snap->ps.clientNum != es->otherEntityNum ||
			cg.renderingThirdPerson || !cg_gunsmoke.integer)) {
			VectorSet( up, 0, 0, 8 );
			CG_SmokePuff( v, up, 32, 1, 1, 1, 0.33f, 900, cg.time, 0, LEF_PUFF_DONT_SCALE, cgs.media.shotgunSmokePuffShader );
		}
	}

	CG_ShotgunPattern( es->pos.trBase, es->origin2, es->otherEntityNum, es );
}

/*
============================================================================

BULLETS

============================================================================
*/


/*
===============
CG_Tracer
===============
*/
void CG_Tracer( vec3_t source, vec3_t dest ) {
	vec3_t		forward, right;
	polyVert_t	verts[4];
	vec3_t		line;
	float		len, begin, end;
	vec3_t		start, finish;
	vec3_t		midpoint;
	int			tracerWidth = 7, tracerLength = 500;

	// tracer
	VectorSubtract( dest, source, forward );
	len = VectorNormalize( forward );

	// start at least a little ways from the muzzle
	if ( len < 100 ) {
		return;
	}
	begin = 50 + random() * (len - 60);
	end = begin + tracerLength;
	if ( end > len ) {
		end = len;
	}
	VectorMA( source, begin, forward, start );
	VectorMA( source, end, forward, finish );

	line[0] = DotProduct( forward, cg.refdef.viewaxis[1] );
	line[1] = DotProduct( forward, cg.refdef.viewaxis[2] );

	VectorScale( cg.refdef.viewaxis[1], line[1], right );
	VectorMA( right, -line[0], cg.refdef.viewaxis[2], right );
	VectorNormalize( right );

	VectorMA( finish, tracerWidth, right, verts[0].xyz );
	verts[0].st[0] = 0;
	verts[0].st[1] = 1;
	verts[0].modulate[0] = 255;
	verts[0].modulate[1] = 255;
	verts[0].modulate[2] = 255;
	verts[0].modulate[3] = 255;

	VectorMA( finish, -tracerWidth, right, verts[1].xyz );
	verts[1].st[0] = 1;
	verts[1].st[1] = 0;
	verts[1].modulate[0] = 255;
	verts[1].modulate[1] = 255;
	verts[1].modulate[2] = 255;
	verts[1].modulate[3] = 255;

	VectorMA( start, -tracerWidth, right, verts[2].xyz );
	verts[2].st[0] = 1;
	verts[2].st[1] = 1;
	verts[2].modulate[0] = 255;
	verts[2].modulate[1] = 255;
	verts[2].modulate[2] = 255;
	verts[2].modulate[3] = 255;

	VectorMA( start, tracerWidth, right, verts[3].xyz );
	verts[3].st[0] = 0;
	verts[3].st[1] = 0;
	verts[3].modulate[0] = 255;
	verts[3].modulate[1] = 255;
	verts[3].modulate[2] = 255;
	verts[3].modulate[3] = 255;

	trap_R_AddPolyToScene( cgs.media.tracerShader, 4, verts );

	midpoint[0] = ( start[0] + finish[0] ) * 0.5;
	midpoint[1] = ( start[1] + finish[1] ) * 0.5;
	midpoint[2] = ( start[2] + finish[2] ) * 0.5;

	// add the tracer sound
	if ( random() < cg_tracerChance.value ) {
		trap_S_StartSound( midpoint, ENTITYNUM_WORLD, CHAN_AUTO, cgs.media.tracerSound );
	}

}


/*
======================
CG_CalcMuzzlePoint
======================
*/
static qboolean	CG_CalcMuzzlePoint( int entityNum, vec3_t muzzle ) {
	vec3_t		forward;
	centity_t	*cent;
	int			anim;

	if ( entityNum == cg.snap->ps.clientNum ) {
		VectorCopy( cg.snap->ps.origin, muzzle );
		muzzle[2] += cg.snap->ps.viewheight;
		AngleVectors( cg.snap->ps.viewangles, forward, NULL, NULL );
		VectorMA( muzzle, 14, forward, muzzle );
		return qtrue;
	}

	cent = &cg_entities[entityNum];
	if ( !cent->currentValid ) {
		return qfalse;
	}

	VectorCopy( cent->currentState.pos.trBase, muzzle );

	AngleVectors( cent->currentState.apos.trBase, forward, NULL, NULL );
	anim = cent->currentState.legsAnim & ~ANIM_TOGGLEBIT;
	if ( anim == LEGS_CROUCH_WALK || anim == LEGS_CROUCHED_IDLE ) {
		muzzle[2] += CROUCH_VIEWHEIGHT;
	} else {
		muzzle[2] += DEFAULT_VIEWHEIGHT;
	}

	VectorMA( muzzle, 14, forward, muzzle );

	return qtrue;

}

/*
======================
CG_Bullet

Renders bullet effects.
======================
*/
void CG_Bullet( vec3_t end, int sourceEntityNum, vec3_t normal, qboolean flesh, int fleshEntityNum,
			   int weapon, int surfaceFlags, int shaderNum, int entityNum ) {
	trace_t trace;
	int sourceContentType, destContentType;
	vec3_t		start;

	// if the shooter is currently valid, calc a source point and possibly
	// do trail effects
	if ( sourceEntityNum >= 0/* && cg_tracerChance.value > 0*/ ) {
		if ( CG_CalcMuzzlePoint( sourceEntityNum, start ) ) {
			int shaderNum;

			sourceContentType = trap_CM_PointContents( start, 0 );
			destContentType = trap_CM_PointContents( end, 0 );

			// do a complete bubble trail if necessary
			if ( ( sourceContentType == destContentType ) && ( sourceContentType & CONTENTS_WATER ) ) {
				CG_BubbleTrail( start, end, 32 );
			}
			// bubble trail from water into air
			else if ( ( sourceContentType & CONTENTS_WATER ) ) {
				shaderNum = trap_CM_BoxTrace_New( &trace, end, start, NULL, NULL, 0, CONTENTS_WATER );
				CG_BubbleTrail( start, trace.endpos, 32 );
			}
			// bubble trail from air into water
			else if ( ( destContentType & CONTENTS_WATER ) ) {
				shaderNum = trap_CM_BoxTrace_New( &trace, start, end, NULL, NULL, 0, CONTENTS_WATER );
				CG_BubbleTrail( trace.endpos, end, 32 );
				//water particles
				CG_LaunchImpactParticle( trace.endpos, trace.plane.normal, SURF_WATER, shaderNum, weapon, qfalse);
			}

			// draw a tracer
			//if ( random() < cg_tracerChance.value ) {
			CG_Tracer( start, end );

			//}
		}
	}

	// impact splash and mark
	if ( flesh ) {
		if ( !cg_blood.integer ) {
			return;
		}
		CG_Bleed( end, fleshEntityNum );
		CG_LaunchImpactParticle(end, normal, -1, -1, -1, qtrue);
	} else {
		if(weapon)
			CG_MissileHitWall( weapon, 0, end, normal, IMPACTSOUND_DEFAULT, surfaceFlags, shaderNum, qfalse, NULL, entityNum );
		else
			CG_MissileHitWall( WP_PEACEMAKER, 0, end, normal, IMPACTSOUND_DEFAULT, surfaceFlags , shaderNum, qfalse, NULL, entityNum);
	}

}

void CG_MakeSmokePuff(entityState_t *ent){
	vec3_t	v;
	int		contents;

	//make smoke puff
	VectorSubtract( ent->origin2, ent->pos.trBase, v );
	VectorNormalize( v );
	VectorScale( v, 32, v );
	VectorAdd( ent->pos.trBase, v, v );
	if ( cgs.glconfig.hardwareType != GLHW_RAGEPRO ) {
		// ragepro can't alpha fade, so don't even bother with smoke
		vec3_t			up;

		contents = trap_CM_PointContents( ent->pos.trBase, 0 );
		if ( !( contents & CONTENTS_WATER ) ) {
			VectorSet( up, 0, 0, 8 );
			CG_SmokePuff( v, up, 24, 1, 1, 0.8f, 0.40f, 400, cg.time, 0, LEF_PUFF_DONT_SCALE, cgs.media.shotgunSmokePuffShader );
		}
	}
}
