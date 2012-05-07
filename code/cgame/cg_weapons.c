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
#ifdef SMOKINGUNS
	vec3_t			angles;
#endif

	if ( cg_brassTime.integer <= 0 ) {
		return;
	}

	le = CG_AllocLocalEntity();
	re = &le->refEntity;

	velocity[0] = 0;
	velocity[1] = -50 + 40 * crandom();
#ifndef SMOKINGUNS
	velocity[2] = 100 + 50 * crandom();
#else
	velocity[2] = 0;
#endif

	le->leType = LE_FRAGMENT;
	le->startTime = cg.time;
	le->endTime = le->startTime + cg_brassTime.integer + ( cg_brassTime.integer / 4 ) * random();

	le->pos.trType = TR_GRAVITY;
	le->pos.trTime = cg.time - (rand()&15);

	AnglesToAxis( cent->lerpAngles, v );

#ifndef SMOKINGUNS
	offset[0] = 8;
	offset[1] = -4;
	offset[2] = 24;
#else
	offset[0] = 20;
	offset[1] = 0;
	offset[2] = 8;
#endif

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

#ifndef SMOKINGUNS
	AxisCopy( axisDefault, re->axis );
#else
	VectorClear(angles);
	angles[YAW] = 90;

	AnglesToAxis(angles, re->axis);
#endif
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
#ifndef SMOKINGUNS
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
#endif

#ifndef SMOKINGUNS
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
#ifndef SMOKINGUNS
void CG_RailTrail (clientInfo_t *ci, vec3_t start, vec3_t end) {
	vec3_t axis[36], move, move2, vec, temp;
	float  len;
	int    i, j, skip;

	localEntity_t *le;
	refEntity_t   *re;

#define RADIUS   4
#define ROTATION 1
#define SPACING  5
 
	start[2] -= 4;
 
	le = CG_AllocLocalEntity();
	re = &le->refEntity;
 
	le->leType = LE_FADE_RGB;
	le->startTime = cg.time;
	le->endTime = cg.time + cg_railTrailTime.value;
	le->lifeRate = 1.0 / (le->endTime - le->startTime);
 
	re->shaderTime = cg.time / 1000.0f;
	re->reType = RT_RAIL_CORE;
	re->customShader = cgs.media.railCoreShader;
 
	VectorCopy(start, re->origin);
	VectorCopy(end, re->oldorigin);
 
	re->shaderRGBA[0] = ci->color1[0] * 255;
	re->shaderRGBA[1] = ci->color1[1] * 255;
	re->shaderRGBA[2] = ci->color1[2] * 255;
	re->shaderRGBA[3] = 255;

	le->color[0] = ci->color1[0] * 0.75;
	le->color[1] = ci->color1[1] * 0.75;
	le->color[2] = ci->color1[2] * 0.75;
	le->color[3] = 1.0f;

	AxisClear( re->axis );
 
	if (cg_oldRail.integer)
	{
		// nudge down a bit so it isn't exactly in center
		re->origin[2] -= 8;
		re->oldorigin[2] -= 8;
		return;
	}

	VectorCopy (start, move);
	VectorSubtract (end, start, vec);
	len = VectorNormalize (vec);
	PerpendicularVector(temp, vec);
	for (i = 0 ; i < 36; i++)
	{
		RotatePointAroundVector(axis[i], vec, temp, i * 10);//banshee 2.4 was 10
	}

	VectorMA(move, 20, vec, move);
	VectorScale (vec, SPACING, vec);

	skip = -1;

	j = 18;
	for (i = 0; i < len; i += SPACING)
	{
		if (i != skip)
		{
			skip = i + SPACING;
			le = CG_AllocLocalEntity();
			re = &le->refEntity;
			le->leFlags = LEF_PUFF_DONT_SCALE;
			le->leType = LE_MOVE_SCALE_FADE;
			le->startTime = cg.time;
			le->endTime = cg.time + (i>>1) + 600;
			le->lifeRate = 1.0 / (le->endTime - le->startTime);

			re->shaderTime = cg.time / 1000.0f;
			re->reType = RT_SPRITE;
			re->radius = 1.1f;
			re->customShader = cgs.media.railRingsShader;

			re->shaderRGBA[0] = ci->color2[0] * 255;
			re->shaderRGBA[1] = ci->color2[1] * 255;
			re->shaderRGBA[2] = ci->color2[2] * 255;
			re->shaderRGBA[3] = 255;

			le->color[0] = ci->color2[0] * 0.75;
			le->color[1] = ci->color2[1] * 0.75;
			le->color[2] = ci->color2[2] * 0.75;
			le->color[3] = 1.0f;

			le->pos.trType = TR_LINEAR;
			le->pos.trTime = cg.time;

			VectorCopy( move, move2);
			VectorMA(move2, RADIUS , axis[j], move2);
			VectorCopy(move2, le->pos.trBase);

			le->pos.trDelta[0] = axis[j][0]*6;
			le->pos.trDelta[1] = axis[j][1]*6;
			le->pos.trDelta[2] = axis[j][2]*6;
		}

		VectorAdd (move, vec, move);

		j = (j + ROTATION) % 36;
	}
}
#endif

/*
==========================
CG_RocketTrail
==========================
*/
#ifndef SMOKINGUNS
static void CG_RocketTrail( centity_t *ent, const weaponInfo_t *wi ) {
	int		step;
	vec3_t	origin, lastPos;
	int		t;
	int		startTime, contents;
	int		lastContents;
	entityState_t	*es;
	vec3_t	up;
	localEntity_t	*smoke;

	if ( cg_noProjectileTrail.integer ) {
		return;
	}

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
#endif

#ifndef SMOKINGUNS
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

	if ( cg_noProjectileTrail.integer ) {
		return;
	}

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
CG_PlasmaTrail
==========================
*/
#ifndef SMOKINGUNS
static void CG_PlasmaTrail( centity_t *cent, const weaponInfo_t *wi ) {
	localEntity_t	*le;
	refEntity_t		*re;
	entityState_t	*es;
	vec3_t			velocity, xvelocity, origin;
	vec3_t			offset, xoffset;
	vec3_t			v[3];

	float	waterScale = 1.0f;

	if ( cg_noProjectileTrail.integer || cg_oldPlasma.integer ) {
		return;
	}

	es = &cent->currentState;

	BG_EvaluateTrajectory( &es->pos, cg.time, origin );

	le = CG_AllocLocalEntity();
	re = &le->refEntity;

	velocity[0] = 60 - 120 * crandom();
	velocity[1] = 40 - 80 * crandom();
	velocity[2] = 100 - 200 * crandom();

	le->leType = LE_MOVE_SCALE_FADE;
	le->leFlags = LEF_TUMBLE;
	le->leBounceSoundType = LEBS_NONE;
	le->leMarkType = LEMT_NONE;

	le->startTime = cg.time;
	le->endTime = le->startTime + 600;

	le->pos.trType = TR_GRAVITY;
	le->pos.trTime = cg.time;

	AnglesToAxis( cent->lerpAngles, v );

	offset[0] = 2;
	offset[1] = 2;
	offset[2] = 2;

	xoffset[0] = offset[0] * v[0][0] + offset[1] * v[1][0] + offset[2] * v[2][0];
	xoffset[1] = offset[0] * v[0][1] + offset[1] * v[1][1] + offset[2] * v[2][1];
	xoffset[2] = offset[0] * v[0][2] + offset[1] * v[1][2] + offset[2] * v[2][2];

	VectorAdd( origin, xoffset, re->origin );
	VectorCopy( re->origin, le->pos.trBase );

	if ( CG_PointContents( re->origin, -1 ) & CONTENTS_WATER ) {
		waterScale = 0.10f;
	}

	xvelocity[0] = velocity[0] * v[0][0] + velocity[1] * v[1][0] + velocity[2] * v[2][0];
	xvelocity[1] = velocity[0] * v[0][1] + velocity[1] * v[1][1] + velocity[2] * v[2][1];
	xvelocity[2] = velocity[0] * v[0][2] + velocity[1] * v[1][2] + velocity[2] * v[2][2];
	VectorScale( xvelocity, waterScale, le->pos.trDelta );

	AxisCopy( axisDefault, re->axis );
	re->shaderTime = cg.time / 1000.0f;
	re->reType = RT_SPRITE;
	re->radius = 0.25f;
	re->customShader = cgs.media.railRingsShader;
	le->bounceFactor = 0.3f;

	re->shaderRGBA[0] = wi->flashDlightColor[0] * 63;
	re->shaderRGBA[1] = wi->flashDlightColor[1] * 63;
	re->shaderRGBA[2] = wi->flashDlightColor[2] * 63;
	re->shaderRGBA[3] = 63;

	le->color[0] = wi->flashDlightColor[0] * 0.2;
	le->color[1] = wi->flashDlightColor[1] * 0.2;
	le->color[2] = wi->flashDlightColor[2] * 0.2;
	le->color[3] = 0.25f;

	le->angles.trType = TR_LINEAR;
	le->angles.trTime = cg.time;
	le->angles.trBase[0] = rand()&31;
	le->angles.trBase[1] = rand()&31;
	le->angles.trBase[2] = rand()&31;
	le->angles.trDelta[0] = 1;
	le->angles.trDelta[1] = 0.5;
	le->angles.trDelta[2] = 0;

}
#endif

/*
==========================
CG_GrappleTrail
==========================
*/
#ifndef SMOKINGUNS
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
#endif

/*
==========================
CG_GrenadeTrail
==========================
*/
#ifndef SMOKINGUNS
static void CG_GrenadeTrail( centity_t *ent, const weaponInfo_t *wi ) {
	CG_RocketTrail( ent, wi );
}
#endif


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

#ifndef SMOKINGUNS
	strcpy( path, item->world_model[0] );
#else
	strcpy( path, bg_weaponlist[weaponNum].path );
#endif
	COM_StripExtension(path, path, sizeof(path));
#ifndef SMOKINGUNS
	strcat( path, "_flash.md3" );
#else
	strcat( path, "flash.md3" );
#endif
	weaponInfo->flashModel = trap_R_RegisterModel( path );

	strcpy( path, item->world_model[0] );
	COM_StripExtension(path, path, sizeof(path));
	strcat( path, "_barrel.md3" );
	weaponInfo->barrelModel = trap_R_RegisterModel( path );

	strcpy( path, item->world_model[0] );
	COM_StripExtension(path, path, sizeof(path));
	strcat( path, "_hand.md3" );
	weaponInfo->handsModel = trap_R_RegisterModel( path );

	if ( !weaponInfo->handsModel ) {
		weaponInfo->handsModel = trap_R_RegisterModel( "models/weapons2/shotgun/shotgun_hand.md3" );
	}

#ifdef SMOKINGUNS
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
#endif

	switch ( weaponNum ) {
#ifndef SMOKINGUNS
	case WP_GAUNTLET:
		MAKERGB( weaponInfo->flashDlightColor, 0.6f, 0.6f, 1.0f );
		weaponInfo->firingSound = trap_S_RegisterSound( "sound/weapons/melee/fstrun.wav", qfalse );
		weaponInfo->flashSound[0] = trap_S_RegisterSound( "sound/weapons/melee/fstatck.wav", qfalse );
#else
	case WP_KNIFE:
		weaponInfo->missileModel = trap_R_RegisterModel( "models/weapons2/knife/e_knife.md3");
		break;

	case WP_PEACEMAKER:
	case WP_REM58:
	case WP_SCHOFIELD:
		MAKERGB( weaponInfo->flashDlightColor, 1, 0.8f, 0.6f );
		cgs.media.bulletExplosionShader = trap_R_RegisterShader( "waterExplosion" );
		cgs.media.bulletPuffShader = trap_R_RegisterShader("smokePuff");
#endif
		break;

	case WP_LIGHTNING:
#ifndef SMOKINGUNS
		MAKERGB( weaponInfo->flashDlightColor, 0.6f, 0.6f, 1.0f );
		weaponInfo->readySound = trap_S_RegisterSound( "sound/weapons/melee/fsthum.wav", qfalse );
		weaponInfo->firingSound = trap_S_RegisterSound( "sound/weapons/lightning/lg_hum.wav", qfalse );

		weaponInfo->flashSound[0] = trap_S_RegisterSound( "sound/weapons/lightning/lg_fire.wav", qfalse );
		cgs.media.lightningShader = trap_R_RegisterShader( "lightningBoltNew");
		cgs.media.lightningExplosionModel = trap_R_RegisterModel( "models/weaphits/crackle.md3" );
		cgs.media.sfx_lghit1 = trap_S_RegisterSound( "sound/weapons/lightning/lg_hit.wav", qfalse );
		cgs.media.sfx_lghit2 = trap_S_RegisterSound( "sound/weapons/lightning/lg_hit2.wav", qfalse );
		cgs.media.sfx_lghit3 = trap_S_RegisterSound( "sound/weapons/lightning/lg_hit3.wav", qfalse );

#else
	case WP_WINCHESTER66:
	case WP_SHARPS:
		MAKERGB( weaponInfo->flashDlightColor, 1, 0.8f, 0.6f );
#endif
		break;

#ifndef SMOKINGUNS
	case WP_GRAPPLING_HOOK:
		MAKERGB( weaponInfo->flashDlightColor, 0.6f, 0.6f, 1.0f );
		weaponInfo->missileModel = trap_R_RegisterModel( "models/ammo/rocket/rocket.md3" );
		weaponInfo->missileTrailFunc = CG_GrappleTrail;
		weaponInfo->missileDlight = 200;
		MAKERGB( weaponInfo->missileDlightColor, 1, 0.75f, 0 );
		weaponInfo->readySound = trap_S_RegisterSound( "sound/weapons/melee/fsthum.wav", qfalse );
		weaponInfo->firingSound = trap_S_RegisterSound( "sound/weapons/melee/fstrun.wav", qfalse );
		cgs.media.lightningShader = trap_R_RegisterShader( "lightningBoltNew");
		break;

#ifdef MISSIONPACK
	case WP_CHAINGUN:
		weaponInfo->firingSound = trap_S_RegisterSound( "sound/weapons/vulcan/wvulfire.wav", qfalse );
		MAKERGB( weaponInfo->flashDlightColor, 1, 1, 0 );
		weaponInfo->flashSound[0] = trap_S_RegisterSound( "sound/weapons/vulcan/vulcanf1b.wav", qfalse );
		weaponInfo->flashSound[1] = trap_S_RegisterSound( "sound/weapons/vulcan/vulcanf2b.wav", qfalse );
		weaponInfo->flashSound[2] = trap_S_RegisterSound( "sound/weapons/vulcan/vulcanf3b.wav", qfalse );
		weaponInfo->flashSound[3] = trap_S_RegisterSound( "sound/weapons/vulcan/vulcanf4b.wav", qfalse );
		weaponInfo->ejectBrassFunc = CG_MachineGunEjectBrass;
		cgs.media.bulletExplosionShader = trap_R_RegisterShader( "bulletExplosion" );
		break;
#endif

	case WP_MACHINEGUN:
		MAKERGB( weaponInfo->flashDlightColor, 1, 1, 0 );
		weaponInfo->flashSound[0] = trap_S_RegisterSound( "sound/weapons/machinegun/machgf1b.wav", qfalse );
		weaponInfo->flashSound[1] = trap_S_RegisterSound( "sound/weapons/machinegun/machgf2b.wav", qfalse );
		weaponInfo->flashSound[2] = trap_S_RegisterSound( "sound/weapons/machinegun/machgf3b.wav", qfalse );
		weaponInfo->flashSound[3] = trap_S_RegisterSound( "sound/weapons/machinegun/machgf4b.wav", qfalse );
		weaponInfo->ejectBrassFunc = CG_MachineGunEjectBrass;
		cgs.media.bulletExplosionShader = trap_R_RegisterShader( "bulletExplosion" );
		break;
#else
	case WP_GATLING:
		weaponInfo->ejectBrassFunc = CG_MachineGunEjectBrass;
		MAKERGB( weaponInfo->flashDlightColor, 1, 0.8f, 0.6f );
		break;
#endif

#ifndef SMOKINGUNS
	case WP_SHOTGUN:
		MAKERGB( weaponInfo->flashDlightColor, 1, 1, 0 );
		weaponInfo->flashSound[0] = trap_S_RegisterSound( "sound/weapons/shotgun/sshotf1b.wav", qfalse );
		weaponInfo->ejectBrassFunc = CG_ShotgunEjectBrass;
#else
	case WP_REMINGTON_GAUGE:
	case WP_SAWEDOFF:
	case WP_WINCH97:
		MAKERGB( weaponInfo->flashDlightColor, 1, 0.8f, 0.6f );
#endif
		break;

#ifndef SMOKINGUNS
	case WP_ROCKET_LAUNCHER:
		weaponInfo->missileModel = trap_R_RegisterModel( "models/ammo/rocket/rocket.md3" );
		weaponInfo->missileSound = trap_S_RegisterSound( "sound/weapons/rocket/rockfly.wav", qfalse );
		weaponInfo->missileTrailFunc = CG_RocketTrail;
		weaponInfo->missileDlight = 200;
		weaponInfo->wiTrailTime = 2000;
		weaponInfo->trailRadius = 64;
		
		MAKERGB( weaponInfo->missileDlightColor, 1, 0.75f, 0 );
		MAKERGB( weaponInfo->flashDlightColor, 1, 0.75f, 0 );

		weaponInfo->flashSound[0] = trap_S_RegisterSound( "sound/weapons/rocket/rocklf1a.wav", qfalse );
		cgs.media.rocketExplosionShader = trap_R_RegisterShader( "rocketExplosion" );
#else
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
#endif
		break;

#ifndef SMOKINGUNS
#ifdef MISSIONPACK
	case WP_PROX_LAUNCHER:
		weaponInfo->missileModel = trap_R_RegisterModel( "models/weaphits/proxmine.md3" );
		weaponInfo->missileTrailFunc = CG_GrenadeTrail;
		weaponInfo->wiTrailTime = 700;
		weaponInfo->trailRadius = 32;
		MAKERGB( weaponInfo->flashDlightColor, 1, 0.70f, 0 );
		weaponInfo->flashSound[0] = trap_S_RegisterSound( "sound/weapons/proxmine/wstbfire.wav", qfalse );
		cgs.media.grenadeExplosionShader = trap_R_RegisterShader( "grenadeExplosion" );
		break;
#endif

	case WP_GRENADE_LAUNCHER:
		weaponInfo->missileModel = trap_R_RegisterModel( "models/ammo/grenade1.md3" );
		weaponInfo->missileTrailFunc = CG_GrenadeTrail;
		weaponInfo->wiTrailTime = 700;
		weaponInfo->trailRadius = 32;
		MAKERGB( weaponInfo->flashDlightColor, 1, 0.70f, 0 );
		weaponInfo->flashSound[0] = trap_S_RegisterSound( "sound/weapons/grenade/grenlf1a.wav", qfalse );
		cgs.media.grenadeExplosionShader = trap_R_RegisterShader( "grenadeExplosion" );
		break;

#ifdef MISSIONPACK
	case WP_NAILGUN:
		weaponInfo->ejectBrassFunc = CG_NailgunEjectBrass;
		weaponInfo->missileTrailFunc = CG_NailTrail;
//		weaponInfo->missileSound = trap_S_RegisterSound( "sound/weapons/nailgun/wnalflit.wav", qfalse );
		weaponInfo->trailRadius = 16;
		weaponInfo->wiTrailTime = 250;
		weaponInfo->missileModel = trap_R_RegisterModel( "models/weaphits/nail.md3" );
		MAKERGB( weaponInfo->flashDlightColor, 1, 0.75f, 0 );
		weaponInfo->flashSound[0] = trap_S_RegisterSound( "sound/weapons/nailgun/wnalfire.wav", qfalse );
		break;
#endif

	case WP_PLASMAGUN:
//		weaponInfo->missileModel = cgs.media.invulnerabilityPowerupModel;
		weaponInfo->missileTrailFunc = CG_PlasmaTrail;
		weaponInfo->missileSound = trap_S_RegisterSound( "sound/weapons/plasma/lasfly.wav", qfalse );
		MAKERGB( weaponInfo->flashDlightColor, 0.6f, 0.6f, 1.0f );
		weaponInfo->flashSound[0] = trap_S_RegisterSound( "sound/weapons/plasma/hyprbf1a.wav", qfalse );
		cgs.media.plasmaExplosionShader = trap_R_RegisterShader( "plasmaExplosion" );
		cgs.media.railRingsShader = trap_R_RegisterShader( "railDisc" );
		break;

	case WP_RAILGUN:
		weaponInfo->readySound = trap_S_RegisterSound( "sound/weapons/railgun/rg_hum.wav", qfalse );
		MAKERGB( weaponInfo->flashDlightColor, 1, 0.5f, 0 );
		weaponInfo->flashSound[0] = trap_S_RegisterSound( "sound/weapons/railgun/railgf1a.wav", qfalse );
		cgs.media.railExplosionShader = trap_R_RegisterShader( "railExplosion" );
		cgs.media.railRingsShader = trap_R_RegisterShader( "railDisc" );
		cgs.media.railCoreShader = trap_R_RegisterShader( "railCore" );
		break;

	case WP_BFG:
		weaponInfo->readySound = trap_S_RegisterSound( "sound/weapons/bfg/bfg_hum.wav", qfalse );
		MAKERGB( weaponInfo->flashDlightColor, 1, 0.7f, 1 );
		weaponInfo->flashSound[0] = trap_S_RegisterSound( "sound/weapons/bfg/bfg_fire.wav", qfalse );
		cgs.media.bfgExplosionShader = trap_R_RegisterShader( "bfgExplosion" );
		weaponInfo->missileModel = trap_R_RegisterModel( "models/weaphits/bfg.md3" );
		weaponInfo->missileSound = trap_S_RegisterSound( "sound/weapons/rocket/rockfly.wav", qfalse );
		break;
#endif

	 default:
#ifndef SMOKINGUNS
		MAKERGB( weaponInfo->flashDlightColor, 1, 1, 1 );
		weaponInfo->flashSound[0] = trap_S_RegisterSound( "sound/weapons/rocket/rocklf1a.wav", qfalse );
#else
		MAKERGB( weaponInfo->flashDlightColor, 0, 0, 0 );
#endif
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

	if ( itemNum < 0 || itemNum >= bg_numItems ) {
		CG_Error( "CG_RegisterItemVisuals: itemNum %d out of range [0-%d]", itemNum, bg_numItems-1 );
	}

	itemInfo = &cg_items[ itemNum ];
	if ( itemInfo->registered ) {
		return;
	}

	item = &bg_itemlist[ itemNum ];

	memset( itemInfo, 0, sizeof( *itemInfo ) );
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
#ifndef SMOKINGUNS
static int CG_MapTorsoToWeaponFrame( clientInfo_t *ci, int frame ) {

	// change weapon
	if ( frame >= ci->animations[TORSO_DROP].firstFrame
		&& frame < ci->animations[TORSO_DROP].firstFrame + 9 ) {
		return frame - ci->animations[TORSO_DROP].firstFrame + 6;
	}

	// stand attack
	if ( frame >= ci->animations[TORSO_ATTACK].firstFrame
		&& frame < ci->animations[TORSO_ATTACK].firstFrame + 6 ) {
		return 1 + frame - ci->animations[TORSO_ATTACK].firstFrame;
	}

	// stand attack 2
	if ( frame >= ci->animations[TORSO_ATTACK2].firstFrame
		&& frame < ci->animations[TORSO_ATTACK2].firstFrame + 6 ) {
		return 1 + frame - ci->animations[TORSO_ATTACK2].firstFrame;
	}

	return 0;
}
#endif


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
#ifdef SMOKINGUNS
	vec3_t	forward;
	float	pitch = cg.refdefViewAngles[PITCH];
	float	value;
#endif

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

#ifdef SMOKINGUNS
	VectorCopy(cg_entities[cg.snap->ps.clientNum].currentState.pos.trDelta, forward);
	VectorNormalize(forward);
	VectorMA( origin, cg.bobfracsin*0.5f, forward, origin);
#endif

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
#ifndef SMOKINGUNS
	angles[PITCH] += scale * fracsin * 0.01;
#else
	value = pitch/ANGLE_REDUCTION;

	angles[PITCH] += scale * fracsin * 0.01 + value;

	//angle: look up and down
	if(pitch < 0)
		pitch *= -1;

	AngleVectors( cg.refdefViewAngles, forward, NULL, NULL );
	VectorMA( origin, MAX_WP_DISTANCE*(pitch/90.0f), forward, origin );
#endif
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
#ifndef SMOKINGUNS
static void CG_LightningBolt( centity_t *cent, vec3_t origin ) {
	trace_t  trace;
	refEntity_t  beam;
	vec3_t   forward;
	vec3_t   muzzlePoint, endPoint;
	int      anim;

	if (cent->currentState.weapon != WP_LIGHTNING) {
		return;
	}

	memset( &beam, 0, sizeof( beam ) );

	// CPMA  "true" lightning
	if ((cent->currentState.number == cg.predictedPlayerState.clientNum) && (cg_trueLightning.value != 0)) {
		vec3_t angle;
		int i;

		for (i = 0; i < 3; i++) {
			float a = cent->lerpAngles[i] - cg.refdefViewAngles[i];
			if (a > 180) {
				a -= 360;
			}
			if (a < -180) {
				a += 360;
			}

			angle[i] = cg.refdefViewAngles[i] + a * (1.0 - cg_trueLightning.value);
			if (angle[i] < 0) {
				angle[i] += 360;
			}
			if (angle[i] > 360) {
				angle[i] -= 360;
			}
		}

		AngleVectors(angle, forward, NULL, NULL );
		VectorCopy(cent->lerpOrigin, muzzlePoint );
//		VectorCopy(cg.refdef.vieworg, muzzlePoint );
	} else {
		// !CPMA
		AngleVectors( cent->lerpAngles, forward, NULL, NULL );
		VectorCopy(cent->lerpOrigin, muzzlePoint );
	}

	anim = cent->currentState.legsAnim & ~ANIM_TOGGLEBIT;
	if ( anim == LEGS_WALKCR || anim == LEGS_IDLECR ) {
		muzzlePoint[2] += CROUCH_VIEWHEIGHT;
	} else {
		muzzlePoint[2] += DEFAULT_VIEWHEIGHT;
	}

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

static void CG_LightningBolt( centity_t *cent, vec3_t origin ) {
	trace_t		trace;
	refEntity_t		beam;
	vec3_t			forward;
	vec3_t			muzzlePoint, endPoint;

	if ( cent->currentState.weapon != WP_LIGHTNING ) {
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
*/
#endif

/*
======================
CG_MachinegunSpinAngle
======================
*/
#ifndef SMOKINGUNS
#define		SPIN_SPEED	0.9
#define		COAST_TIME	1000
static float	CG_MachinegunSpinAngle( centity_t *cent ) {
#else
#define		SPIN_SPEED	0.3
#define		COAST_TIME	500
float	CG_MachinegunSpinAngle( centity_t *cent ) {
#endif
	int		delta;
	float	angle;
	float	speed;
#ifdef SMOKINGUNS
	qboolean firing = ((cent->currentState.eFlags & EF_FIRING) /*&& !reloading*/);

	// if this is the player which usese the gatling do prediction
	if(cg.snap->ps.clientNum == cent->currentState.clientNum &&
		!cg_nopredict.integer && !cg_synchronousClients.integer){
		firing = ( cg.predictedPlayerState.eFlags & EF_FIRING);
	}
#endif

	delta = cg.time - cent->pe.barrelTime;
	if ( cent->pe.barrelSpinning ) {
		angle = cent->pe.barrelAngle + delta * SPIN_SPEED;

#ifdef SMOKINGUNS
		trap_S_AddLoopingSound( cent->currentState.number, cent->lerpOrigin, vec3_origin, cgs.media.sfx_gatling_loop );
#endif
	} else {
		if ( delta > COAST_TIME ) {
			delta = COAST_TIME;
		}

		speed = 0.5 * ( SPIN_SPEED + (float)( COAST_TIME - delta ) / COAST_TIME );

#ifdef SMOKINGUNS
		if(COAST_TIME - delta)
			trap_S_AddLoopingSound( cent->currentState.number, cent->lerpOrigin, vec3_origin, cgs.media.sfx_gatling_loop );
#endif
		angle = cent->pe.barrelAngle + delta * speed;
	}

#ifndef SMOKINGUNS
	if ( cent->pe.barrelSpinning == !(cent->currentState.eFlags & EF_FIRING) ) {
#else
	if ( cent->pe.barrelSpinning == !firing) {
#endif
		cent->pe.barrelTime = cg.time;
		cent->pe.barrelAngle = AngleMod( angle );
#ifndef SMOKINGUNS
		cent->pe.barrelSpinning = !!(cent->currentState.eFlags & EF_FIRING);
#ifdef MISSIONPACK
		if ( cent->currentState.weapon == WP_CHAINGUN && !cent->pe.barrelSpinning ) {
			trap_S_StartSound( NULL, cent->currentState.number, CHAN_WEAPON, trap_S_RegisterSound( "sound/weapons/vulcan/wvulwind.wav", qfalse ) );
		}
#endif
#else
		cent->pe.barrelSpinning = !!firing;
#endif
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
#ifndef SMOKINGUNS
	if ( powerups & ( 1 << PW_INVIS ) ) {
		gun->customShader = cgs.media.invisShader;
		trap_R_AddRefEntityToScene( gun );
	} else {
		trap_R_AddRefEntityToScene( gun );

		if ( powerups & ( 1 << PW_BATTLESUIT ) ) {
			gun->customShader = cgs.media.battleWeaponShader;
			trap_R_AddRefEntityToScene( gun );
		}
		if ( powerups & ( 1 << PW_QUAD ) ) {
			gun->customShader = cgs.media.quadWeaponShader;
			trap_R_AddRefEntityToScene( gun );
		}
	}
#else
	trap_R_AddRefEntityToScene( gun );
#endif
}

/*
=============
CG_InitWeaponAnim
=============
*/
#ifdef SMOKINGUNS
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
#endif

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
#ifndef SMOKINGUNS
	refEntity_t	barrel;
#else
	refEntity_t	gun2;
#endif
	refEntity_t	flash;
	vec3_t		angles;
#ifndef SMOKINGUNS
	weapon_t	weaponNum;
	weaponInfo_t	*weapon;
#else
	weapon_t	weaponNum, weapon2Num;
	weaponInfo_t	*weapon, *weapon2=NULL;
	int i;
	qboolean render = qtrue;
#endif
	centity_t	*nonPredictedCent;
#ifndef SMOKINGUNS
	orientation_t	lerped;
#endif

#ifdef SMOKINGUNS
	if(ps && ps->weapon == WP_SHARPS && cg.snap->ps.stats[STAT_WP_MODE]==1)
		render = qfalse;
#endif

	weaponNum = cent->currentState.weapon;
#ifdef SMOKINGUNS
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
#endif

	CG_RegisterWeapon( weaponNum );
	weapon = &cg_weapons[weaponNum];

#ifdef SMOKINGUNS
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
#endif

	// add the weapon
	memset( &gun, 0, sizeof( gun ) );
	VectorCopy( parent->lightingOrigin, gun.lightingOrigin );
	gun.shadowPlane = parent->shadowPlane;
	gun.renderfx = parent->renderfx;

#ifndef SMOKINGUNS
	// set custom shading for railgun refire rate
	if( weaponNum == WP_RAILGUN ) {
		clientInfo_t *ci = &cgs.clientinfo[cent->currentState.clientNum];
		if( cent->pe.railFireTime + 1500 > cg.time ) {
			int scale = 255 * ( cg.time - cent->pe.railFireTime ) / 1500;
			gun.shaderRGBA[0] = ( ci->c1RGBA[0] * scale ) >> 8;
			gun.shaderRGBA[1] = ( ci->c1RGBA[1] * scale ) >> 8;
			gun.shaderRGBA[2] = ( ci->c1RGBA[2] * scale ) >> 8;
			gun.shaderRGBA[3] = 255;
		}
		else {
			Byte4Copy( ci->c1RGBA, gun.shaderRGBA );
		}
	}

	gun.hModel = weapon->weaponModel;
#else
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
#endif

	if (!gun.hModel) {
		return;
	}

	if ( !ps ) {
		// add weapon ready sound
		cent->pe.lightningFiring = qfalse;
#ifndef SMOKINGUNS
		if ( ( cent->currentState.eFlags & EF_FIRING ) && weapon->firingSound ) {
			// lightning gun and guantlet make a different sound when fire is held down
			trap_S_AddLoopingSound( cent->currentState.number, cent->lerpOrigin, vec3_origin, weapon->firingSound );
			cent->pe.lightningFiring = qtrue;
		} else if ( weapon->readySound ) {
			trap_S_AddLoopingSound( cent->currentState.number, cent->lerpOrigin, vec3_origin, weapon->readySound );
		}
#endif
	}

#ifndef SMOKINGUNS
	trap_R_LerpTag(&lerped, parent->hModel, parent->oldframe, parent->frame,
		1.0 - parent->backlerp, "tag_weapon");
	VectorCopy(parent->origin, gun.origin);

	VectorMA(gun.origin, lerped.origin[0], parent->axis[0], gun.origin);

	// Make weapon appear left-handed for 2 and centered for 3
	if(ps && cg_drawGun.integer == 2)
		VectorMA(gun.origin, -lerped.origin[1], parent->axis[1], gun.origin);
	else if(!ps || cg_drawGun.integer != 3)
	       	VectorMA(gun.origin, lerped.origin[1], parent->axis[1], gun.origin);

	VectorMA(gun.origin, lerped.origin[2], parent->axis[2], gun.origin);

	MatrixMultiply(lerped.axis, ((refEntity_t *)parent)->axis, gun.axis);
	gun.backlerp = parent->backlerp;

	CG_AddWeaponWithPowerups( &gun, cent->currentState.powerups );
#else
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
#endif


	// add the spinning barrel
#ifndef SMOKINGUNS
	if ( weapon->barrelModel ) {
		memset( &barrel, 0, sizeof( barrel ) );
		VectorCopy( parent->lightingOrigin, barrel.lightingOrigin );
		barrel.shadowPlane = parent->shadowPlane;
		barrel.renderfx = parent->renderfx;

		barrel.hModel = weapon->barrelModel;
		angles[YAW] = 0;
		angles[PITCH] = 0;
		angles[ROLL] = CG_MachinegunSpinAngle( cent );
		AnglesToAxis( angles, barrel.axis );

		CG_PositionRotatedEntityOnTag( &barrel, &gun, weapon->weaponModel, "tag_barrel" );

		CG_AddWeaponWithPowerups( &barrel, cent->currentState.powerups );
	}
#endif

	// make sure we aren't looking at cg.predictedPlayerEntity for LG
	nonPredictedCent = &cg_entities[cent->currentState.clientNum];

	// if the index of the nonPredictedCent is not the same as the clientNum
	// then this is a fake player (like on teh single player podiums), so
	// go ahead and use the cent
	if( ( nonPredictedCent - cg_entities ) != cent->currentState.clientNum ) {
		nonPredictedCent = cent;
	}

	// add sparks to the burning dynamite
#ifdef SMOKINGUNS
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
#endif

	// add the flash
#ifndef SMOKINGUNS
	if ( ( weaponNum == WP_LIGHTNING || weaponNum == WP_GAUNTLET || weaponNum == WP_GRAPPLING_HOOK )
		&& ( nonPredictedCent->currentState.eFlags & EF_FIRING ) ) 
	{
		// continuous flash
	} else {
		// impulse flash
		if ( cg.time - cent->muzzleFlashTime > MUZZLE_FLASH_TIME ) {
			return;
		}
	}
#else
	if ( (cg.time - cent->muzzleFlashTime > MUZZLE_FLASH_TIME) &&
		(cg.time - cent->muzzleFlashTime2 > MUZZLE_FLASH_TIME)) {
		return;
	}
#endif

#ifndef SMOKINGUNS
	memset( &flash, 0, sizeof( flash ) );
	VectorCopy( parent->lightingOrigin, flash.lightingOrigin );
	flash.shadowPlane = parent->shadowPlane;
	flash.renderfx = parent->renderfx;

	flash.hModel = weapon->flashModel;
	if (!flash.hModel) {
		return;
	}
	angles[YAW] = 0;
	angles[PITCH] = 0;
	angles[ROLL] = crandom() * 10;
	AnglesToAxis( angles, flash.axis );

	// colorize the railgun blast
	if ( weaponNum == WP_RAILGUN ) {
		clientInfo_t	*ci;

		ci = &cgs.clientinfo[ cent->currentState.clientNum ];
		flash.shaderRGBA[0] = 255 * ci->color1[0];
		flash.shaderRGBA[1] = 255 * ci->color1[1];
		flash.shaderRGBA[2] = 255 * ci->color1[2];
	}

	CG_PositionRotatedEntityOnTag( &flash, &gun, weapon->weaponModel, "tag_flash");
	trap_R_AddRefEntityToScene( &flash );

	if ( ps || cg.renderingThirdPerson ||
		cent->currentState.number != cg.predictedPlayerState.clientNum ) {
		// add lightning bolt
		CG_LightningBolt( nonPredictedCent, flash.origin );

		if ( weapon->flashDlightColor[0] || weapon->flashDlightColor[1] || weapon->flashDlightColor[2] ) {
			trap_R_AddLightToScene( flash.origin, 300 + (rand()&31), weapon->flashDlightColor[0],
				weapon->flashDlightColor[1], weapon->flashDlightColor[2] );
		}
	}
#else
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

			if(i == 2){
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
#endif
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

#ifndef SMOKINGUNS
	if ( ps->persistant[PERS_TEAM] == TEAM_SPECTATOR ) {
#else
	if ( ps->persistant[PERS_TEAM] >= TEAM_SPECTATOR ) {
#endif
		return;
	}

	if ( ps->pm_type == PM_INTERMISSION ) {
		return;
	}

	// no gun if in third person view or a camera is active
	//if ( cg.renderingThirdPerson || cg.cameraMode) {
	if ( cg.renderingThirdPerson ) {
		return;
	}


	// allow the gun to be completely removed
	if ( !cg_drawGun.integer ) {
#ifndef SMOKINGUNS
		vec3_t		origin;

		if ( cg.predictedPlayerState.eFlags & EF_FIRING ) {
			// special hack for lightning gun...
			VectorCopy( cg.refdef.vieworg, origin );
			VectorMA( origin, -8, cg.refdef.viewaxis[2], origin );
			CG_LightningBolt( &cg_entities[ps->clientNum], origin );
		}
#endif
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
#ifndef SMOKINGUNS
		hand.frame = CG_MapTorsoToWeaponFrame( ci, cent->pe.torso.frame );
		hand.oldframe = CG_MapTorsoToWeaponFrame( ci, cent->pe.torso.oldFrame );
#else
		hand.frame = 0;
		hand.oldframe = 0;
#endif
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

/*
===================
CG_DrawWeaponSelect
===================
*/

#ifdef SMOKINGUNS
#define FIELD_WIDTH		135
#define	FIELD_HEIGHT	47
#define	TOP_HEIGHT		24

#define AMMO_BULLET_WIDTH	15
#define AMMO_CART_WIDTH		19
#define AMMO_SHARP_WIDTH	24
#define AMMO_SHELL_WIDTH	14.5
#define AMMO_HEIGHT				4.5
#endif

void CG_DrawWeaponSelect( void ) {

#ifdef SMOKINGUNS
	vec4_t	color2[4] = {
		{1.0f, 1.0f, 1.0f, 0.5f},
		{0.0f, 0.0f, 0.0f, 0.75f},
		{0.4f, 0.4f, 0.4f, 0.75f},
		{1.0f, 0.8f, 0.0f, 1.0f},
	};
#endif
	int		i;
	int		bits;
#ifndef SMOKINGUNS
	int		count;
#endif
	int		x, y, w;
	char	*name;
#ifndef SMOKINGUNS
	float	*color;
#else
	int		wp_sort;
	qboolean akimbo, singlepistolSelect;
	int		ammo, weapon;
	float	ammowidth;
	qhandle_t	clip;

	// in duel mode don't let them choose the weapon before the intro ends
	if(cgs.gametype == GT_DUEL && cg.introend && (cg.introend-DU_INTRO_DRAW) >= cg.time)
		return;
#endif

	// don't display if dead
	if ( cg.predictedPlayerState.stats[STAT_HEALTH] <= 0 ) {
		return;
	}

#ifndef SMOKINGUNS
	color = CG_FadeColor( cg.weaponSelectTime, WEAPON_SELECT_TIME );
	if ( !color ) {
		return;
	}
	trap_R_SetColor( color );

	// showing weapon select clears pickup item display, but not the blend blob
	cg.itemPickupTime = 0;
#else
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
#endif

	// count the number of weapons owned
	bits = cg.snap->ps.stats[ STAT_WEAPONS ];
#ifndef SMOKINGUNS
	count = 0;
	for ( i = 1 ; i < MAX_WEAPONS ; i++ ) {
		if ( bits & ( 1 << i ) ) {
			count++;
		}
	}

	x = 320 - count * 20;
	y = 380;

	for ( i = 1 ; i < MAX_WEAPONS ; i++ ) {
		if ( !( bits & ( 1 << i ) ) ) {
			continue;
		}

		CG_RegisterWeapon( i );

		// draw weapon icon
		CG_DrawPic( x, y, 32, 32, cg_weapons[i].weaponIcon );

		// draw selection marker
		if ( i == cg.weaponSelect ) {
			CG_DrawPic( x-4, y-4, 40, 40, cgs.media.selectShader );
		}

		// no ammo cross on top
		if ( !cg.snap->ps.ammo[ i ] ) {
			CG_DrawPic( x, y, 32, 32, cgs.media.noammoShader );
		}

		x += 40;
	}
#else
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
#endif

	// draw the selected name
#ifndef SMOKINGUNS
	if ( cg_weapons[ cg.weaponSelect ].item ) {
		name = cg_weapons[ cg.weaponSelect ].item->pickup_name;
		if ( name ) {
			w = CG_DrawStrlen( name ) * BIGCHAR_WIDTH;
			x = ( SCREEN_WIDTH - w ) / 2;
			CG_DrawBigStringColor(x, y - 22, name, color);
		}
	}

	trap_R_SetColor( NULL );
#else
	if ( cg_weapons[ cg.markedweapon ].item || cg.markedweapon == WP_AKIMBO) {

		if(cg.markedweapon == WP_AKIMBO)
			name = "Double Pistols";
		else
			name = cg_weapons[ cg.markedweapon ].item->pickup_name;

		if ( name ) {
			w = CG_DrawStrlen( name ) * BIGCHAR_WIDTH;
			CG_DrawBigStringColor(x, y, name, color2[0]);
		}
	}
#endif
}


/*
===============
CG_WeaponSelectable
===============
*/
static qboolean CG_WeaponSelectable( int i ) {
#ifdef SMOKINGUNS
	if (bg_weaponlist[i].clip){
		if ( !cg.snap->ps.ammo[i] && !cg.snap->ps.ammo[bg_weaponlist[i].clip]) {
			return qfalse;
		}
	} else {
#endif
		if ( !cg.snap->ps.ammo[i]) {
			return qfalse;
		}
#ifdef SMOKINGUNS
	}
#endif

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
#ifndef SMOKINGUNS
	int		i;
	int		original;
#else
	int		num;
	int		i, j, weapon;
	int startnum;
	// is it the first time weapon_f was called since the last wp-change?
	qboolean first_time = !cg.markedweapon;

	// has the player got one pistol in hand ?
	qboolean singlepistolSelect = cg.weaponSelect != WP_AKIMBO
		&& bg_weaponlist[ cg.weaponSelect ].wp_sort == WPS_PISTOL;
#endif

	if ( !cg.snap ) {
		return;
	}
	if ( cg.snap->ps.pm_flags & PMF_FOLLOW ) {
		return;
	}

#ifndef SMOKINGUNS
	cg.weaponSelectTime = cg.time;
	original = cg.weaponSelect;

	for ( i = 0 ; i < MAX_WEAPONS ; i++ ) {
		cg.weaponSelect++;
		if ( cg.weaponSelect == MAX_WEAPONS ) {
			cg.weaponSelect = 0;
		}
		if ( cg.weaponSelect == WP_GAUNTLET ) {
			continue;		// never cycle to gauntlet
		}
		if ( CG_WeaponSelectable( cg.weaponSelect ) ) {
			break;
		}
	}
	if ( i == MAX_WEAPONS ) {
		cg.weaponSelect = original;
	}
#else
	if( cg.introend > cg.time)
		return;

	if(cg.snap->ps.stats[STAT_GATLING_MODE])
		return;

	if(first_time) { // signal BUTTON_CHOOSE_MODE to the server
		// Tequila comment: By torhu, see http://www.quake3world.com/forum/viewtopic.php?f=16&t=17815
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

		for(i = 1, weapon = 0; i <= WP_NUM_WEAPONS; i++){

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
#endif
}

/*
===============
CG_PrevWeapon_f
===============
*/
void CG_PrevWeapon_f( void ) {
#ifndef SMOKINGUNS
	int		i;
	int		original;
#else
	int		num = 0;
	int		i, j, weapon;
	int startnum;
	// is it the first time weapon_f was called since the last wp-change?
	qboolean first_time = !cg.markedweapon;

	// has the player got one pistol in hand ?
	qboolean singlepistolSelect = cg.weaponSelect != WP_AKIMBO
		&& bg_weaponlist[ cg.weaponSelect ].wp_sort == WPS_PISTOL;
#endif

	if ( !cg.snap ) {
		return;
	}
	if ( cg.snap->ps.pm_flags & PMF_FOLLOW ) {
		return;
	}

#ifndef SMOKINGUNS
	cg.weaponSelectTime = cg.time;
	original = cg.weaponSelect;

	for ( i = 0 ; i < MAX_WEAPONS ; i++ ) {
		cg.weaponSelect--;
		if ( cg.weaponSelect == -1 ) {
			cg.weaponSelect = MAX_WEAPONS - 1;
		}
		if ( cg.weaponSelect == WP_GAUNTLET ) {
			continue;		// never cycle to gauntlet
		}
		if ( CG_WeaponSelectable( cg.weaponSelect ) ) {
			break;
		}
	}
	if ( i == MAX_WEAPONS ) {
		cg.weaponSelect = original;
	}
#else
	if( cg.introend > cg.time)
		return;

	if(cg.snap->ps.stats[STAT_GATLING_MODE])
		return;

	if(first_time) { // signal BUTTON_CHOOSE_MODE to the server
		// Tequila comment: By torhu, see http://www.quake3world.com/forum/viewtopic.php?f=16&t=17815
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

		for(i = 1, weapon = 0; i <= WP_NUM_WEAPONS; i++){

			// check if akimbo has to be set
			if(num == WPS_GUN && weapon == 1 &&
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
#endif
}

/*
===============
CG_Weapon_f
===============
*/
void CG_Weapon_f( void ) {
	int		num;
#ifdef SMOKINGUNS
	int		i, weapon;
	int		startnum;
	// is it the first time weapon_f was called since the last wp-change?
	qboolean first_time = !cg.markedweapon;

	// has the player got one pistol in hand ?
	qboolean singlepistolSelect = cg.weaponSelect != WP_AKIMBO
		&& bg_weaponlist[ cg.weaponSelect ].wp_sort == WPS_PISTOL;
#endif

	if ( !cg.snap ) {
		return;
	}
	if ( cg.snap->ps.pm_flags & PMF_FOLLOW ) {
		return;
	}

#ifdef SMOKINGUNS
	if( cg.introend > cg.time)
		return;

	if(cg.snap->ps.stats[STAT_GATLING_MODE])
		return;
#endif

	num = atoi( CG_Argv( 1 ) );

#ifndef SMOKINGUNS
	if ( num < 1 || num > MAX_WEAPONS-1 ) {
		return;
	}

	cg.weaponSelectTime = cg.time;

	if ( ! ( cg.snap->ps.stats[STAT_WEAPONS] & ( 1 << num ) ) ) {
		return;		// don't have the weapon
	}

	cg.weaponSelect = num;
#else
	if ( num < 1 || num > 4 ) {
		return;
	}

	if(first_time) { // signal BUTTON_CHOOSE_MODE to the server
		// Tequila comment: By torhu, see http://www.quake3world.com/forum/viewtopic.php?f=16&t=17815
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

	for(i = 1, weapon = 0; i <= WP_NUM_WEAPONS; i++){

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
#endif
}

/*
===============
CG_LastUsedWeapon_f
===============
*/
#ifdef SMOKINGUNS
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
#endif

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

#ifndef SMOKINGUNS
	for ( i = MAX_WEAPONS-1 ; i > 0 ; i-- ) {
#else
	for ( i = WP_NUM_WEAPONS -1 ; i > 0 ; i-- ) {
#endif
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
#ifndef SMOKINGUNS
void CG_FireWeapon( centity_t *cent ) {
#else
void CG_FireWeapon( centity_t *cent, qboolean altfire, int weapon ) {
#endif
	entityState_t *ent;
	int				c;
	weaponInfo_t	*weap;

	ent = &cent->currentState;
#ifndef SMOKINGUNS
	if ( ent->weapon == WP_NONE ) {
#else
	if ( weapon == WP_NONE ) {
#endif
		return;
	}
#ifndef SMOKINGUNS
	if ( ent->weapon >= WP_NUM_WEAPONS ) {
		CG_Error( "CG_FireWeapon: ent->weapon >= WP_NUM_WEAPONS" );
#else
	if ( weapon >= WP_NUM_WEAPONS ) {
		CG_Error( "CG_FireWeapon: weapon >= WP_NUM_WEAPONS" );
#endif
		return;
	}
#ifndef SMOKINGUNS
	weap = &cg_weapons[ ent->weapon ];
#else
	weap = &cg_weapons[ weapon ];
#endif

	// mark the entity as muzzle flashing, so when it is added it will
	// append the flash to the weapon model
#ifdef SMOKINGUNS
	if(!altfire || !cent->currentState.frame){
#endif
		cent->muzzleFlashTime = cg.time;

#ifndef SMOKINGUNS
	// lightning gun only does this this on initial press
	if ( ent->weapon == WP_LIGHTNING ) {
		if ( cent->pe.lightningFiring ) {
			return;
		}
	}

	if( ent->weapon == WP_RAILGUN ) {
		cent->pe.railFireTime = cg.time;
	}

	// play quad sound if needed
	if ( cent->currentState.powerups & ( 1 << PW_QUAD ) ) {
		trap_S_StartSound (NULL, cent->currentState.number, CHAN_ITEM, cgs.media.quadSound );
	}
#else
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
#endif

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
#ifdef SMOKINGUNS
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
#endif

/*
=================
CG_MissileHitWall

Caused by an EV_MISSILE_MISS event, or directly by local bullet tracing
=================
*/
#ifndef SMOKINGUNS
void CG_MissileHitWall( int weapon, int clientNum, vec3_t origin, vec3_t dir, impactSound_t soundType ) {
#else
#define SMOKEPUFF_SIZE 6
void CG_MissileHitWall( int weapon, int clientNum, vec3_t origin, vec3_t dir,
					   impactSound_t soundType,
					   int surfaceFlags, int shaderNum,
					   qboolean fire, vec3_t bottledirs[ALC_COUNT],
					   int entityNum) {
#endif
	qhandle_t		mod;
	qhandle_t		mark;
	qhandle_t		shader;
	sfxHandle_t		sfx;
#ifndef SMOKINGUNS
	float			radius;
#else
	float			radius_mrk;
	float			radius_exp;
#endif
	float			light;
	vec3_t			lightColor;
	localEntity_t	*le;
#ifndef SMOKINGUNS
	int				r;
#endif
	qboolean		alphaFade;
	qboolean		isSprite;
	int				duration;
#ifndef SMOKINGUNS
	vec3_t			sprOrg;
	vec3_t			sprVel;
#else
	int				i;
#endif

	// set defaults
	mark = 0;
#ifndef SMOKINGUNS
	radius = 32;
#else
	radius_mrk = 32;
	radius_exp = 32;
#endif
	sfx = 0;
	mod = 0;
	shader = 0;
	light = 0;
#ifndef SMOKINGUNS
	lightColor[0] = 1;
	lightColor[1] = 1;
	lightColor[2] = 0;

	// set defaults
#else
	lightColor[0] = 0.8f;
	lightColor[1] = 0.8f;
	lightColor[2] = 0.6f;
#endif
	isSprite = qfalse;
	duration = 600;

	switch ( weapon ) {
#ifndef SMOKINGUNS
	default:
#ifdef MISSIONPACK
	case WP_NAILGUN:
		if( soundType == IMPACTSOUND_FLESH ) {
			sfx = cgs.media.sfx_nghitflesh;
		} else if( soundType == IMPACTSOUND_METAL ) {
			sfx = cgs.media.sfx_nghitmetal;
		} else {
			sfx = cgs.media.sfx_nghit;
		}
		mark = cgs.media.holeMarkShader;
		radius = 12;
		break;
#endif
	case WP_LIGHTNING:
		// no explosion at LG impact, it is added with the beam
		r = rand() & 3;
		if ( r < 2 ) {
			sfx = cgs.media.sfx_lghit2;
		} else if ( r == 2 ) {
			sfx = cgs.media.sfx_lghit1;
		} else {
			sfx = cgs.media.sfx_lghit3;
		}
		mark = cgs.media.holeMarkShader;
		radius = 12;
		break;
#ifdef MISSIONPACK
	case WP_PROX_LAUNCHER:
		mod = cgs.media.dishFlashModel;
		shader = cgs.media.grenadeExplosionShader;
		sfx = cgs.media.sfx_proxexp;
		mark = cgs.media.burnMarkShader;
		radius = 64;
		light = 300;
		isSprite = qtrue;
		break;
#endif
	case WP_GRENADE_LAUNCHER:
		mod = cgs.media.dishFlashModel;
		shader = cgs.media.grenadeExplosionShader;
		sfx = cgs.media.sfx_rockexp;
		mark = cgs.media.burnMarkShader;
		radius = 64;
		light = 300;
		isSprite = qtrue;
		break;
	case WP_ROCKET_LAUNCHER:
		mod = cgs.media.dishFlashModel;
		shader = cgs.media.rocketExplosionShader;
		sfx = cgs.media.sfx_rockexp;
		mark = cgs.media.burnMarkShader;
		radius = 64;
		light = 300;
		isSprite = qtrue;
		duration = 1000;
		lightColor[0] = 1;
		lightColor[1] = 0.75;
		lightColor[2] = 0.0;
		if (cg_oldRocket.integer == 0) {
			// explosion sprite animation
			VectorMA( origin, 24, dir, sprOrg );
			VectorScale( dir, 64, sprVel );

			CG_ParticleExplosion( "explode1", sprOrg, sprVel, 1400, 20, 30 );
		}
		break;
	case WP_RAILGUN:
		mod = cgs.media.ringFlashModel;
		shader = cgs.media.railExplosionShader;
		//sfx = cgs.media.sfx_railg;
		sfx = cgs.media.sfx_plasmaexp;
		mark = cgs.media.energyMarkShader;
		radius = 24;
		break;
	case WP_PLASMAGUN:
		mod = cgs.media.ringFlashModel;
		shader = cgs.media.plasmaExplosionShader;
		sfx = cgs.media.sfx_plasmaexp;
		mark = cgs.media.energyMarkShader;
		radius = 16;
		break;
	case WP_BFG:
		mod = cgs.media.dishFlashModel;
		shader = cgs.media.bfgExplosionShader;
		sfx = cgs.media.sfx_rockexp;
		mark = cgs.media.burnMarkShader;
		radius = 32;
		isSprite = qtrue;
		break;
	case WP_SHOTGUN:
		mod = cgs.media.bulletFlashModel;
		shader = cgs.media.bulletExplosionShader;
		mark = cgs.media.bulletMarkShader;
		sfx = 0;
		radius = 4;
		break;

#ifdef MISSIONPACK
	case WP_CHAINGUN:
		mod = cgs.media.bulletFlashModel;
		if( soundType == IMPACTSOUND_FLESH ) {
			sfx = cgs.media.sfx_chghitflesh;
		} else if( soundType == IMPACTSOUND_METAL ) {
			sfx = cgs.media.sfx_chghitmetal;
		} else {
			sfx = cgs.media.sfx_chghit;
		}
		mark = cgs.media.bulletMarkShader;

		radius = 8;
		break;
#endif

	case WP_MACHINEGUN:
		mod = cgs.media.bulletFlashModel;
		shader = cgs.media.bulletExplosionShader;
		mark = cgs.media.bulletMarkShader;

		r = rand() & 3;
		if ( r == 0 ) {
			sfx = cgs.media.sfx_ric1;
		} else if ( r == 1 ) {
			sfx = cgs.media.sfx_ric2;
		} else {
			sfx = cgs.media.sfx_ric3;
		}

		radius = 8;
		break;
#else
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
		sfx = cgs.media.impact[IMPACT_GLASS][rand()%3];
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
#endif
	}


#ifndef SMOKINGUNS
	if ( sfx ) {
#else
	if ( sfx && weapon != WP_SAWEDOFF && weapon != WP_REMINGTON_GAUGE) {
		trap_S_StartSound( origin, ENTITYNUM_WORLD, CHAN_AUTO, sfx );
	} else if(sfx){
		if(Distance(origin, cg.refdef.vieworg) > 650)
#endif
			trap_S_StartSound( origin, ENTITYNUM_WORLD, CHAN_AUTO, sfx );
	}

	//
	// create the explosion
	//
#ifndef SMOKINGUNS
	if ( mod ) {
#else
	if ( mod || shader) {
#endif
		le = CG_MakeExplosion( origin, dir,
							   mod,	shader,
#ifndef SMOKINGUNS
							   duration, isSprite );
#else
							   duration, isSprite , radius_exp, (weapon == WP_DYNAMITE));
#endif
		le->light = light;
		VectorCopy( lightColor, le->lightColor );
#ifndef SMOKINGUNS
		if ( weapon == WP_RAILGUN ) {
			// colorize with client color
			VectorCopy( cgs.clientinfo[clientNum].color1, le->color );
			le->refEntity.shaderRGBA[0] = le->color[0] * 0xff;
			le->refEntity.shaderRGBA[1] = le->color[1] * 0xff;
			le->refEntity.shaderRGBA[2] = le->color[2] * 0xff;
			le->refEntity.shaderRGBA[3] = 0xff;
		}
#endif
	}

	//
	// impact mark
	//
#ifndef SMOKINGUNS
	alphaFade = (mark == cgs.media.energyMarkShader);	// plasma fades alpha, all others fade color
	if ( weapon == WP_RAILGUN ) {
		float	*color;

		// colorize with client color
		color = cgs.clientinfo[clientNum].color1;
		CG_ImpactMark( mark, origin, dir, random()*360, color[0],color[1], color[2],1, alphaFade, radius, qfalse );
	} else {
		CG_ImpactMark( mark, origin, dir, random()*360, 1,1,1,1, alphaFade, radius, qfalse );
	}
#else
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

				color[i] *= temp;
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
#endif
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
#ifndef SMOKINGUNS
	case WP_GRENADE_LAUNCHER:
	case WP_ROCKET_LAUNCHER:
	case WP_PLASMAGUN:
	case WP_BFG:
#ifdef MISSIONPACK
	case WP_NAILGUN:
	case WP_CHAINGUN:
	case WP_PROX_LAUNCHER:
#endif
		CG_MissileHitWall( weapon, 0, origin, dir, IMPACTSOUND_FLESH );
#else
	case WP_DYNAMITE:
		CG_MissileHitWall( weapon, 0, origin, dir, IMPACTSOUND_FLESH, -1, -1, qfalse, NULL, -1 );
		break;
	case WP_MOLOTOV:
		CG_MissileHitWall( weapon, 0, origin, dir, IMPACTSOUND_FLESH, -1, -1, qfalse, NULL, -1 );
#endif
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
#ifndef SMOKINGUNS
static void CG_ShotgunPellet( vec3_t start, vec3_t end, int skipNum ) {
	trace_t		tr;
	int sourceContentType, destContentType;

	CG_Trace( &tr, start, NULL, NULL, end, skipNum, MASK_SHOT );

	sourceContentType = CG_PointContents( start, 0 );
	destContentType = CG_PointContents( tr.endpos, 0 );

	// FIXME: should probably move this cruft into CG_BubbleTrail
	if ( sourceContentType == destContentType ) {
		if ( sourceContentType & CONTENTS_WATER ) {
			CG_BubbleTrail( start, tr.endpos, 32 );
		}
	} else if ( sourceContentType & CONTENTS_WATER ) {
		trace_t trace;

		trap_CM_BoxTrace( &trace, end, start, NULL, NULL, 0, CONTENTS_WATER );
		CG_BubbleTrail( start, trace.endpos, 32 );
	} else if ( destContentType & CONTENTS_WATER ) {
		trace_t trace;

		trap_CM_BoxTrace( &trace, start, end, NULL, NULL, 0, CONTENTS_WATER );
		CG_BubbleTrail( tr.endpos, trace.endpos, 32 );
	}

	if (  tr.surfaceFlags & SURF_NOIMPACT ) {
		return;
	}

	if ( cg_entities[tr.entityNum].currentState.eType == ET_PLAYER ) {
		CG_MissileHitPlayer( WP_SHOTGUN, tr.endpos, tr.plane.normal, tr.entityNum );
	} else {
		if ( tr.surfaceFlags & SURF_NOIMPACT ) {
			// SURF_NOIMPACT will not make a flame puff or a mark
			return;
		}
		if ( tr.surfaceFlags & SURF_METALSTEPS ) {
			CG_MissileHitWall( WP_SHOTGUN, 0, tr.endpos, tr.plane.normal, IMPACTSOUND_METAL );
		} else {
			CG_MissileHitWall( WP_SHOTGUN, 0, tr.endpos, tr.plane.normal, IMPACTSOUND_DEFAULT );
		}
	}
}
#else
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

		shaderNum = CG_Trace_New( &tr, tr_start, NULL, NULL, end, ENTITYNUM_NONE, MASK_SHOT );

		//player is already dead
		if(tr.entityNum >= MAX_CLIENTS){
			VectorCopy(player, tr.endpos);
			tr.entityNum = playerhit;
		}

	} else {
		vec3_t dir;

		VectorSubtract(end, tr_start, dir);
		VectorNormalize(dir);

		shaderNum = CG_Trace_New( &tr, tr_start, NULL, NULL, end, ENTITYNUM_NONE, MASK_SHOT );

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
			shaderNum = CG_Trace_New( &tr, endpos, NULL, NULL, tr_start, ENTITYNUM_NONE, (MASK_SOLID|CONTENTS_BODY) );

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
#endif

/*
================
CG_ShotgunPattern

Perform the same traces the server did to locate the
hit splashes
================
*/
#ifndef SMOKINGUNS
static void CG_ShotgunPattern( vec3_t origin, vec3_t origin2, int seed, int otherEntNum ) {
	int			i;
	float		r, u;
	vec3_t		end;
	vec3_t		forward, right, up;

	// derive the right and up vectors from the forward vector, because
	// the client won't have any other information
	VectorNormalize2( origin2, forward );
	PerpendicularVector( right, forward );
	CrossProduct( forward, right, up );

	// generate the "random" spread pattern
	for ( i = 0 ; i < DEFAULT_SHOTGUN_COUNT ; i++ ) {
		r = Q_crandom( &seed ) * DEFAULT_SHOTGUN_SPREAD * 16;
		u = Q_crandom( &seed ) * DEFAULT_SHOTGUN_SPREAD * 16;
		VectorMA( origin, 8192 * 16, forward, end);
		VectorMA (end, r, right, end);
		VectorMA (end, u, up, end);

		CG_ShotgunPellet( origin, end, otherEntNum );
	}
}
#else
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
	
	if ( cgs.newShotgunPattern ) {
			
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
			// spread_dist *= 1.4f ; // 1.4 only adjust from homogenous to radial homogenous
			// Adding an extra 0.05 because the spread is now circular instead of square
			spread_dist *= 1.45f ;
			
			spread_angle = current_angle_shift + ( (float)current_spread_cell + Q_random( &seed ) ) * M_PI * 2.0f / (float)current_pellet_per_circle ;

			r = sin( spread_angle ) * spread_dist ;
			u = cos( spread_angle ) * spread_dist ;

			VectorMA( origin, 8192 * 16, forward, end);
			VectorMA (end, r, right, end);
			VectorMA (end, u, up, end);

			if ( ( i + extra_center_pellet + 1 ) < 16 && ( (int)es->angles[1] & ( 1 << ( i + extra_center_pellet + 1 ) ) ) )
			{
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
#endif

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

		contents = CG_PointContents( es->pos.trBase, 0 );
		if ( !( contents & CONTENTS_WATER ) && (cg.snap->ps.clientNum != es->otherEntityNum ||
			cg.renderingThirdPerson || !cg_gunsmoke.integer)) {
			VectorSet( up, 0, 0, 8 );
			CG_SmokePuff( v, up, 32, 1, 1, 1, 0.33f, 900, cg.time, 0, LEF_PUFF_DONT_SCALE, cgs.media.shotgunSmokePuffShader );
		}
	}
#ifndef SMOKINGUNS
	CG_ShotgunPattern( es->pos.trBase, es->origin2, es->eventParm, es->otherEntityNum );
#else
	CG_ShotgunPattern( es->pos.trBase, es->origin2, es->otherEntityNum, es );
#endif
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

	// tracer
	VectorSubtract( dest, source, forward );
	len = VectorNormalize( forward );

	// start at least a little ways from the muzzle
	if ( len < 100 ) {
		return;
	}
	begin = 50 + random() * (len - 60);
	end = begin + cg_tracerLength.value;
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

	VectorMA( finish, cg_tracerWidth.value, right, verts[0].xyz );
	verts[0].st[0] = 0;
	verts[0].st[1] = 1;
	verts[0].modulate[0] = 255;
	verts[0].modulate[1] = 255;
	verts[0].modulate[2] = 255;
	verts[0].modulate[3] = 255;

	VectorMA( finish, -cg_tracerWidth.value, right, verts[1].xyz );
	verts[1].st[0] = 1;
	verts[1].st[1] = 0;
	verts[1].modulate[0] = 255;
	verts[1].modulate[1] = 255;
	verts[1].modulate[2] = 255;
	verts[1].modulate[3] = 255;

	VectorMA( start, -cg_tracerWidth.value, right, verts[2].xyz );
	verts[2].st[0] = 1;
	verts[2].st[1] = 1;
	verts[2].modulate[0] = 255;
	verts[2].modulate[1] = 255;
	verts[2].modulate[2] = 255;
	verts[2].modulate[3] = 255;

	VectorMA( start, cg_tracerWidth.value, right, verts[3].xyz );
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
#ifdef SMOKINGUNS
	if ( random()*2 < cg_tracerChance.value )
#endif
		trap_S_StartSound( midpoint, ENTITYNUM_WORLD, CHAN_AUTO, cgs.media.tracerSound );

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
#ifndef SMOKINGUNS
	if ( anim == LEGS_WALKCR || anim == LEGS_IDLECR ) {
#else
	if ( anim == LEGS_CROUCH_WALK || anim == LEGS_CROUCHED_IDLE ) {
#endif
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
#ifndef SMOKINGUNS
void CG_Bullet( vec3_t end, int sourceEntityNum, vec3_t normal, qboolean flesh, int fleshEntityNum ) {
#else
void CG_Bullet( vec3_t end, int sourceEntityNum, vec3_t normal, qboolean flesh, int fleshEntityNum,
			   int weapon, int surfaceFlags, int shaderNum, int entityNum ) {
#endif
	trace_t trace;
	int sourceContentType, destContentType;
	vec3_t		start;

	// if the shooter is currently valid, calc a source point and possibly
	// do trail effects
	if ( sourceEntityNum >= 0 && cg_tracerChance.value > 0 ) {
		if ( CG_CalcMuzzlePoint( sourceEntityNum, start ) ) {
			sourceContentType = CG_PointContents( start, 0 );
			destContentType = CG_PointContents( end, 0 );

			// do a complete bubble trail if necessary
			if ( ( sourceContentType == destContentType ) && ( sourceContentType & CONTENTS_WATER ) ) {
				CG_BubbleTrail( start, end, 32 );
			}
			// bubble trail from water into air
			else if ( ( sourceContentType & CONTENTS_WATER ) ) {
#ifndef SMOKINGUNS
				trap_CM_BoxTrace( &trace, end, start, NULL, NULL, 0, CONTENTS_WATER );
#else
				shaderNum = trap_CM_BoxTrace_New( &trace, end, start, NULL, NULL, 0, CONTENTS_WATER );
#endif
				CG_BubbleTrail( start, trace.endpos, 32 );
			}
			// bubble trail from air into water
			else if ( ( destContentType & CONTENTS_WATER ) ) {
#ifndef SMOKINGUNS
				trap_CM_BoxTrace( &trace, start, end, NULL, NULL, 0, CONTENTS_WATER );
#else
				shaderNum = trap_CM_BoxTrace_New( &trace, start, end, NULL, NULL, 0, CONTENTS_WATER );
#endif
				CG_BubbleTrail( trace.endpos, end, 32 );
				//water particles
#ifdef SMOKINGUNS
				CG_LaunchImpactParticle( trace.endpos, trace.plane.normal, SURF_WATER, shaderNum, weapon, qfalse);
#endif
			}

			// draw a tracer
			if ( random() < cg_tracerChance.value ) {
				CG_Tracer( start, end );
			}
		}
	}

	// impact splash and mark
	if ( flesh ) {
#ifdef SMOKINGUNS
		if ( !cg_blood.integer ) {
			return;
		}
#endif
		CG_Bleed( end, fleshEntityNum );
#ifdef SMOKINGUNS
		CG_LaunchImpactParticle(end, normal, -1, -1, -1, qtrue);
#endif
	} else {
#ifndef SMOKINGUNS
		CG_MissileHitWall( WP_MACHINEGUN, 0, end, normal, IMPACTSOUND_DEFAULT );
#else
		if(weapon)
			CG_MissileHitWall( weapon, 0, end, normal, IMPACTSOUND_DEFAULT, surfaceFlags, shaderNum, qfalse, NULL, entityNum );
		else
			CG_MissileHitWall( WP_PEACEMAKER, 0, end, normal, IMPACTSOUND_DEFAULT, surfaceFlags , shaderNum, qfalse, NULL, entityNum);
#endif
	}

}

#ifdef SMOKINGUNS
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

void CG_BulletTracer( vec3_t start, vec3_t end, int number, int entityNum ) {
	vec3_t forward;

	localEntity_t *le;
	refEntity_t   *re;

	VectorSubtract( end, start, forward );
	VectorNormalize( forward );

	// Draw the trace line
	le = CG_AllocLocalEntity();
	re = &le->refEntity;

	le->leType = LE_FADE_RGB;
	le->startTime = cg.time;
	le->endTime = cg.time + 30000 ;
	le->lifeRate = 1.0 / (le->endTime - le->startTime);

	re->shaderTime = cg.time / 1000.0f;
	re->reType = RT_RAIL_CORE;
	re->customShader = trap_R_RegisterShader( "weapon_trace" );

	VectorCopy(start, re->origin);
	VectorCopy(end, re->oldorigin);

	re->shaderRGBA[0] = 0;
	re->shaderRGBA[1] = 0;
	re->shaderRGBA[2] = 255;
	re->shaderRGBA[3] = 128;

	le->color[0] = 0.0f;
	le->color[1] = 0.0f;
	le->color[2] = 1.0f;
	le->color[3] = 0.5f;

	// Draw the trace number at the line begin
	le = CG_AllocLocalEntity();
	re = &le->refEntity;

	le->leType = LE_SHOWREFENTITY;
	le->startTime = cg.time;
	le->endTime = cg.time + 30000 ;
	le->lifeRate = 1.0 / (le->endTime - le->startTime);

	re->reType = RT_SPRITE;
	re->radius = 0.5f;
	re->customShader = cgs.media.numberShaders[number%10];
	le->pos.trType = TR_STATIONARY;

	VectorCopy(start, re->origin);
	re->origin[2] += 2 ;
}
#endif
