/*
===========================================================================
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
// cg_sg_utils.c -- SG functions

#include "cg_local.h"

#define	MARK_TOTAL_TIME		20000

/*
typedef enum {
	RT_MODEL,
	RT_POLY,
	RT_SPRITE,
	RT_BEAM,
	RT_RAIL_CORE,
	RT_RAIL_RINGS,
	RT_LIGHTNING,
	RT_PORTALSURFACE,		// doesn't draw anything, just info for portals

	RT_MAX_REF_ENTITY_TYPE
} refEntityType_t;*/

localEntity_t *CG_CreateBulletHole( vec3_t origin, vec3_t dir, int surfaceFlags,
								   int entityNum){

	localEntity_t	*le;
	refEntity_t		*re;
	int num=0, j;
	vec3_t angles;

	for(j=0; j<NUM_PREFIXINFO; j++){
		if((surfaceFlags & prefixInfo[j].surfaceFlags) ||
			!prefixInfo[j].surfaceFlags){
			num = j;
			break;
		}
	}

	le = CG_AllocLocalEntity();
	re = &le->refEntity;

	le->leType = LE_FRAGMENT;
	le->startTime = cg.time;
	le->endTime = cg.time + MARK_TOTAL_TIME;

	VectorCopy( origin, re->origin);
	VectorCopy( origin, re->oldorigin);
	VectorCopy( origin, re->lightingOrigin);
	vectoangles(dir, angles);
	AnglesToAxis(angles, le->refEntity.axis);
	re->reType = RT_MODEL;
	re->radius = 3;
	re->customShader = cgs.media.breakmarks[num];
	re->hModel = cgs.media.quad;

	VectorScale(le->refEntity.axis[0], le->refEntity.radius, le->refEntity.axis[0]);
	VectorScale(le->refEntity.axis[1], le->refEntity.radius, le->refEntity.axis[1]);
	VectorScale(le->refEntity.axis[2], le->refEntity.radius, le->refEntity.axis[2]);

	le->pos.trType = TR_STATIONARY;
	VectorCopy(origin, le->pos.trBase );
	le->pos.trTime = cg.time;
	le->leFlags = LEF_TUMBLE;
	le->leFlags |= LEF_MARK;
	le->angles.trType = TR_STATIONARY;
	VectorCopy(origin, le->refEntity.lightingOrigin);
	le->leMarkType = LEMT_NONE;

	// bind the mark to an entitynum
	le->refEntity.radius = entityNum;
	return le;
}

/*
========================
SG CG_LaunchParticle
by: dX
date: 6.feb.2000

function: Generates One particle (model) with given values

=========================
*/
localEntity_t *CG_LaunchParticle( vec3_t origin, vec3_t dir, qhandle_t hModel, int snd,
						int bouncefactor, int duration,
						int leFlags){

	localEntity_t	*le;
	refEntity_t		*re;

	le = CG_AllocLocalEntity();
	re = &le->refEntity;

	le->leType = LE_FRAGMENT;
	le->startTime = cg.time;
	le->endTime = le->startTime + duration;

	VectorCopy( origin, re->origin);
	VectorCopy( origin, re->oldorigin);
	AxisCopy( axisDefault, re->axis);
	re->hModel = hModel;

	le->pos.trType = TR_GRAVITY;
	VectorCopy(origin, le->pos.trBase );
	le->pos.trTime = cg.time;
	le->leFlags = LEF_TUMBLE;
	le->leFlags |= leFlags;
	le->angles.trType = TR_LINEAR;
	le->angles.trTime = cg.time;
	le->angles.trBase[0] = rand()%360;
	le->angles.trBase[1] = rand()%360;
	le->angles.trBase[2] = rand()%360;
	le->angles.trDelta[0] = -le->angles.trBase[0];
	le->angles.trDelta[1] = -le->angles.trBase[1];
	le->angles.trDelta[2] = -le->angles.trBase[2];

	VectorScale(dir, 100, le->pos.trDelta);

	le->leBounceSoundType = snd;

	switch(bouncefactor){
	case BOUNCE_LIGHT:
		le->bounceFactor = 0.15f;
		break;
	case BOUNCE_MEDIUM:
		le->bounceFactor = 0.3f;
		break;
	case BOUNCE_HEAVY:
		le->bounceFactor = 0.6f;
		break;
	}

	le->leMarkType = LEMT_NONE;

	return le;
}


localEntity_t *CG_LaunchSpriteParticle( vec3_t origin, vec3_t dir,
						int bouncefactor, int duration, int leFlags){

	localEntity_t	*le;
	refEntity_t		*re;

	le = CG_AllocLocalEntity();
	re = &le->refEntity;

	le->leType = LE_FRAGMENT;
	le->startTime = cg.time;
	le->endTime = le->startTime + duration + rand()%500;

	VectorCopy( origin, re->origin);
	VectorCopy( origin, re->oldorigin);
	AxisCopy( axisDefault, re->axis);
	re->reType = RT_SPRITE;
	re->radius = 3;

	le->pos.trType = TR_GRAVITY_LOW2;
	VectorCopy(origin, le->pos.trBase );
	le->pos.trTime = cg.time;
	le->leFlags = LEF_TUMBLE;
	le->leFlags |= leFlags;
	le->angles.trType = TR_LINEAR;
	le->angles.trTime = cg.time;
	le->angles.trBase[0] = rand()%(2*360)-360;
	le->angles.trBase[1] = rand()%(2*360)-360;
	le->angles.trBase[2] = rand()%(2*360)-360;
	le->angles.trDelta[0] = -le->angles.trBase[0];
	le->angles.trDelta[1] = -le->angles.trBase[1];
	le->angles.trDelta[2] = -le->angles.trBase[2];
	VectorCopy(origin, le->refEntity.lightingOrigin);

	VectorScale(dir, 100, le->pos.trDelta);

	switch(bouncefactor){
	case BOUNCE_LIGHT:
		le->bounceFactor = 0.15f;
		break;
	case BOUNCE_MEDIUM:
		le->bounceFactor = 0.3f;
		break;
	case BOUNCE_HEAVY:
		le->bounceFactor = 0.6f;
		break;
	}

	le->leMarkType = LEMT_NONE;
	return le;
}


/*
=================
cgame breakable effects
=================
*/

void CG_LaunchGlass(vec3_t mins, vec3_t maxs, int surfaceFlags){
	vec3_t length;
	int cols, rows;
	int side;
	float col_width, row_width;
	int starttime,i,c,r;
	int num_gibs, quad_size;

	if(surfaceFlags & SURF_CLOTH){
		num_gibs = 5;
		quad_size = 60;
	}else{
		num_gibs = NUM_GIBS;
		quad_size = 40;
	}

	VectorSubtract(maxs, mins, length);

	// calculate how much rows and columns
	if(length[1] > length[0])
		side = 1;
	else
		side = 0;

	// columns
	if((length[side]/quad_size) >
		((int)length[side]/quad_size))
		cols = (int)length[side]/quad_size + 1;
	else
		cols = (int)length[side]/quad_size + 1;

	// rows
	if((length[2]/quad_size) >
		((int)length[2]/quad_size))
		rows = (int)length[2]/quad_size + 1;
	else
		rows = (int)length[2]/quad_size + 1;

	col_width = length[side]/cols+1;
	row_width = length[2]/rows+1;

	// now start the creation
	for(c=cols-1; c >= 0; c--){
		starttime = cg.time;

		for(r=rows-1; r >= 0; r--){
			vec3_t origin;
			const int roll = (rand()%4)*90;

			if(r<rows-1)
				starttime += rand()%200;

			VectorCopy(mins, origin);
			origin[side] += c*col_width+col_width/2;
			origin[2] += r*row_width+row_width/2;

			if(length[1] > length[0])
				origin[!side] += 3*length[!side]/8;
			else
				origin[!side] += 6*length[!side]/8;

			for(i=0; i < num_gibs; i++){
				localEntity_t	*gib;
				qhandle_t model;
				float real_row, real_col;
				vec3_t factor;
				vec3_t dir;
				int snd = LEBS_DEFAULT;

				dir[0] = (rand()%360)-180;
				dir[1] = (rand()%360)-180;
				dir[2] = (rand()%180)-90;

				VectorNormalize(dir);

				if(surfaceFlags & SURF_CLOTH)
					model = cgs.media.clothgib[i];
				else {
					model = cgs.media.glassgib[i];

					if(surfaceFlags & SURF_GLASS)
						snd = LEBS_GLASS;
					else if(surfaceFlags & SURF_METAL)
						snd = LEBS_METAL;
				}
				if(rand()%6)
					snd = LEBS_NONE;

				gib = CG_LaunchParticle(origin, dir, model, snd,
					BOUNCE_LIGHT, 6000 + random()*4000, LEF_BREAKS);

				if(surfaceFlags & SURF_METAL)
					gib->refEntity.customShader = cgs.media.metalgibshader;

				gib->fadeInTime = side;
				gib->pos.trType = TR_STATIONARY;
				gib->startTime =  starttime + rand()%300;

				VectorClear(gib->angles.trBase);

				if(length[1] > length[0])
					gib->angles.trBase[YAW] = 90;
				else
					gib->angles.trBase[YAW] = 0;

				gib->angles.trBase[0] = roll;
				AnglesToAxis(gib->angles.trBase, gib->refEntity.axis);

				if(roll%180){
					real_col = row_width;
					real_row = col_width;
				}else{
					real_row = row_width;
					real_col = col_width;
				}

				// scale it so that it fits the brush
				factor[0] = (float)(real_col/10);
				if(length[1] > length[0])
					factor[1] = length[0]/2;
				else
					factor[1] = length[1]/2;

				factor[2] = (float)(real_row/10);

				VectorCopy(factor, gib->vector);
				VectorScale(gib->refEntity.axis[0], factor[0], gib->refEntity.axis[0]);
				VectorScale(gib->refEntity.axis[1], factor[1], gib->refEntity.axis[1]);
				VectorScale(gib->refEntity.axis[2], factor[2], gib->refEntity.axis[2]);
			}
		}
	}
}

#define	DIR_FACTOR	   0.8f//0.6f
void CG_LaunchDefault( vec3_t mins, vec3_t maxs, int surfaceFlags, int shaderNum) {
	vec3_t dir, origin, length;
	float *xpos, *ypos, miny;
	int height, width;
	int i, border;
	const int step = 4;
	qhandle_t model;
	int bouncefactor;
	int	pos;

	VectorSubtract( maxs, mins, origin);
	VectorSubtract(maxs, mins, length);

	height = (int)origin[2];

	if(origin[0] > origin[1]){
		pos = 1;
		xpos = &origin[0];
		ypos = &origin[1];

		width = (int)origin[1];
		miny = mins[1];

		border = *xpos/step;

		VectorScale(origin, 0.5, origin);
		VectorAdd(mins, origin, origin);

		*xpos = mins[0];
	} else {
		pos = 0;
		xpos = &origin[1];
		ypos = &origin[0];

		width = (int)origin[0];
		miny = mins[0];

		border = *xpos/step;

		VectorScale(origin, 0.5, origin);
		VectorAdd(mins, origin, origin);

		*xpos = mins[1];

	}


	for(i=0; i < border || !i; i++){
		localEntity_t	*gib;
		float factor;
		int snd = LEBS_DEFAULT;

		VectorClear(dir);
		dir[0] = (rand()%360)-180;
		dir[1] = (rand()%360)-180;
		dir[2] = (rand()%360)-180;

		VectorNormalize(dir);

		*xpos += step;
		*ypos = miny + rand()%width;
		origin[2] = mins[2] + rand()%height;

		if(surfaceFlags & SURF_WOOD){
			model = cgs.media.woodgib[rand()%3];
			factor = 2;
			snd = LEBS_WOOD;

		} else {
			model = cgs.media.dirtgib[rand()%3];
			factor = 1.5f;

			// additonally launch some particles
			if((surfaceFlags & SURF_GRASS) ||
				(surfaceFlags & SURF_SNOW) ||
				(surfaceFlags & SURF_SAND) ||
				(surfaceFlags & SURF_PLANT)){
				qhandle_t shader;
				int surfaceNum=0;
				float radius=0.0f, weight=0.0f;
				int j;
				vec3_t newdir;
				localEntity_t *le;

				for(j=0; j<NUM_PREFIXINFO; j++){
					if((surfaceFlags & prefixInfo[j].surfaceFlags) ||
						!prefixInfo[j].surfaceFlags){
						radius = prefixInfo[j].radius;
						weight = prefixInfo[j].weight;
						surfaceNum = j;
						break;
					}
				}

				shader = cgs.media.particles[surfaceNum][rand()%3];
				VectorCopy(dir, newdir);
				VectorNormalize(newdir);

				for(j=0; j<3; j++){
					float	temp;

					temp = (rand()%20)-10;
					temp /= 10;

					newdir[j] += temp;
				}

				VectorNormalize(newdir);
				VectorScale( newdir, DIR_FACTOR*1/weight, newdir );

				if(newdir[2] > 0)
					newdir[2] *= 2.4f;
				else
					newdir[2] *= 0.5;

				le = CG_LaunchSpriteParticle(origin, newdir, BOUNCE_MEDIUM, 2000*1/weight, LEF_PARTICLE);

				le->refEntity.customShader = shader;
				le->refEntity.radius = radius*3;

				if(shaderNum != -1){

					Vector4Copy(shaderInfo[shaderNum].color, le->color);
				}
			}

		}

		if(VectorLength(length) > 100){
			float add = VectorLength(length)/100;
			if(add > 4)
				add = 4;
			factor *= add;
		} else if( VectorLength(length) < 80){
			float add = VectorLength(length)/80;
			if(add < 0.05f)
				add = 0.05f;
			factor *= add;
		}

		bouncefactor = BOUNCE_LIGHT;
		if(rand()%6)
			snd = LEBS_NONE;

		gib = CG_LaunchParticle(origin, dir, model, snd, bouncefactor,
			5000 + random()*4000, LEF_BREAKS_DEF);

		if(surfaceFlags & SURF_STONE)
			gib->refEntity.customShader = cgs.media.stonegibshader;

		VectorScale(gib->refEntity.axis[0], factor, gib->refEntity.axis[0]);
		VectorScale(gib->refEntity.axis[1], factor, gib->refEntity.axis[1]);
		VectorScale(gib->refEntity.axis[2], factor, gib->refEntity.axis[2]);
		VectorSet(gib->vector, factor, factor, factor);
	}
}

void CG_LaunchFuncBreakable( centity_t *cent) {
	int surfaceFlags;
	int shaderNum;

	if(cent->currentState.torsoAnim == 1){
		surfaceFlags = (int)cent->currentState.angles2[0];
		shaderNum = -1;
	} else {
		surfaceFlags = shaderInfo[cent->currentState.eventParm].surfaceFlags;
		shaderNum = cent->currentState.eventParm;
	}

	if((surfaceFlags & SURF_GLASS)||
		(surfaceFlags & SURF_CLOTH)||
		(surfaceFlags & SURF_METAL))
		CG_LaunchGlass(cent->currentState.origin2, cent->currentState.origin, surfaceFlags);
	else
		CG_LaunchDefault(cent->currentState.origin2, cent->currentState.origin, surfaceFlags, shaderNum);

	if(surfaceFlags & SURF_GLASS) {
		trap_S_StartSound(cent->currentState.origin,
		ENTITYNUM_WORLD, CHAN_AUTO, cgs.media.sfxglassgib[rand()%2]);
	} else if((surfaceFlags & SURF_STONE) ||
		(surfaceFlags & SURF_METAL) ||
		(surfaceFlags & SURF_DIRT) ||
		(surfaceFlags & SURF_SAND)){
		trap_S_StartSound(cent->currentState.origin,
		ENTITYNUM_WORLD, CHAN_AUTO, cgs.media.sfxstonegib);
	}
}

/*
========================
CG_CreateSparkTrail
by: Spoon
17.6.2001

Generates a Spark Trail
========================
*/
#define SPARKTRAIL_COUNT	20
#define	SPARKTRAIL_TIME		500
#define SPARK_TIME	1
localEntity_t *CG_CreateSparkTrail( vec3_t origin, vec3_t dir){
	localEntity_t	*le;
	refEntity_t		*re;

	le = CG_AllocLocalEntity();
	re= &le->refEntity;

	le->leType = LE_FRAGMENT;
	//le->leType = LE_SPRITE_EXPLOSION;
	le->startTime = cg.time;
	le->endTime = le->startTime + SPARKTRAIL_TIME;

	AxisCopy( axisDefault, re->axis);
	re->reType = RT_SPRITE;
	re->radius = 1;

	le->pos.trType = TR_GRAVITY_LOW2;
	le->pos.trTime = cg.time;
	le->leFlags = LEF_TUMBLE;
	le->leFlags |= LEF_SPARKS;
	le->angles.trType = TR_LINEAR;
	le->angles.trTime = cg.time;
	/*le->angles.trBase[0] = rand()%(2*360)-360;
	le->angles.trBase[1] = rand()%(2*360)-360;
	le->angles.trBase[2] = rand()%(2*360)-360;
	le->angles.trDelta[0] = -le->angles.trBase[0];
	le->angles.trDelta[1] = -le->angles.trBase[1];
	le->angles.trDelta[2] = -le->angles.trBase[2];*/

	le->color[0] = 255;
	le->color[1] = 255;
	le->color[2] = 255;
	le->color[3] = 255;

	le->bounceFactor = 0.10f;
	le->leMarkType = LEMT_NONE;

	re->customShader = cgs.media.backTileShader;//cgs.media.sparktrail;

	VectorCopy( origin, re->origin);
	VectorCopy( origin, le->pos.trBase );
	VectorScale(dir, 100, le->pos.trDelta);

	//VectorAdd(re->origin, dir, re->origin);

	VectorCopy( origin, re->oldorigin);
	VectorCopy( dir, re->lightingOrigin);

	//marks number of spark-paricle
	le->refEntity.frame = 1;

	return le;
}

/*
========================
CG_CheckSparkGen
by: Spoon

26.6.2001
========================
*/
void CG_CheckSparkGen(localEntity_t	*le){
	localEntity_t	*le2[SPARKTRAIL_COUNT];
	int i = 1;

	le2[0] = le;

	if(le->refEntity.frame > SPARKTRAIL_COUNT)
		return;

	while((le2[i-1]->startTime + le2[i-1]->refEntity.frame*SPARK_TIME) < cg.time &&
		le2[i-1]->refEntity.frame){
		const int	deltaTime = cg.time - (le2[i-1]->startTime + le2[i-1]->refEntity.frame*SPARK_TIME);

		CG_Printf("%i (%i, %i)\n", deltaTime, cg.time, (le2[i-1]->startTime + le2[i-1]->refEntity.frame*SPARK_TIME));

		le2[i] = CG_CreateSparkTrail(le2[i-1]->refEntity.oldorigin, le2[i-1]->refEntity.lightingOrigin);

		le2[i]->pos.trTime -= deltaTime;
		le2[i]->startTime = le2[i-1]->startTime;

		//marks number of spark-paricle
		le2[i]->refEntity.frame = le2[i-1]->refEntity.frame+1;

		le2[i]->refEntity.frame = 0;
		VectorCopy( le2[i-1]->refEntity.oldorigin, le2[i]->refEntity.oldorigin);
		VectorCopy( le2[i-1]->refEntity.lightingOrigin, le2[i]->refEntity.lightingOrigin);

		BG_EvaluateTrajectory(&le2[i]->pos, cg.time, le2[i]->pos.trBase);
		VectorCopy(le2[i]->pos.trBase, le2[i]->refEntity.origin);

		i++;
		if(i==SPARKTRAIL_COUNT)
			return;
	}
}

/*
========================
CG_LightForPoint
by: Spoon
18.6.2001

returns the light value of a point
========================
*/
void CG_LightForPoint(vec3_t origin, vec3_t normal, vec4_t color){
	int				j;
	float			incoming;
	vec3_t			ambientLight;
	vec3_t			lightDir;
	vec3_t			directedLight;

	trap_R_LightForPoint( origin, ambientLight, directedLight, lightDir );

	incoming = DotProduct (normal, lightDir);
	if ( incoming <= 0 ) {
		color[0] = ambientLight[0];
		color[1] = ambientLight[1];
		color[2] = ambientLight[2];
		color[3] = 255;
		return;
	}
	j = ( ambientLight[0] + incoming * directedLight[0] );
	if ( j > 255 ) {
		j = 255;
	}
	color[0] = j;

	j = ( ambientLight[1] + incoming * directedLight[1] );
	if ( j > 255 ) {
		j = 255;
	}
	color[1] = j;

	j = ( ambientLight[2] + incoming * directedLight[2] );
	if ( j > 255 ) {
		j = 255;
	}
	color[2] = j;

	color[3] = 255;

	return;
}

/*
========================
CG_LaunchImpactParticle
by: Spoon
9.6.2001

Generates Particles which will spawn if a bullet hits a surface
========================
*/
void CG_LaunchImpactParticle( vec3_t origin, vec3_t dir, int surfaceFlags
							 , int shaderNum, int weapon, qboolean flesh){
	localEntity_t	*le;
	int i,j;
	vec3_t			spawnpoint;
	qhandle_t		shader;
	float			radius=0.0f;
	float			weight=0.0f;
	int				num=0;
	int				surfaceNum=0;
	vec3_t			newdir;

	//impactsound
	if(!flesh && ((weapon != WP_SAWEDOFF && weapon != WP_REMINGTON_GAUGE &&
		weapon != WP_WINCH97) || !(rand()%3)) ){
		if(surfaceFlags & SURF_WOOD){
			trap_S_StartSound( origin, ENTITYNUM_WORLD, CHAN_AUTO,cgs.media.impact[IMPACT_WOOD][rand()%3]);
		} else if(surfaceFlags & SURF_METAL){
			trap_S_StartSound( origin, ENTITYNUM_WORLD, CHAN_AUTO,cgs.media.impact[IMPACT_METAL][rand()%3]);
		} else if(surfaceFlags & SURF_WATER){
			trap_S_StartSound( origin, ENTITYNUM_WORLD, CHAN_AUTO,cgs.media.impact[IMPACT_WATER][rand()%3]);
		} else if(surfaceFlags & SURF_GLASS){
			trap_S_StartSound( origin, ENTITYNUM_WORLD, CHAN_AUTO,cgs.media.impact[IMPACT_GLASS][rand()%3]);
		} else if(surfaceFlags & SURF_STONE){
			trap_S_StartSound( origin, ENTITYNUM_WORLD, CHAN_AUTO,cgs.media.impact[IMPACT_STONE][rand()%3]);
		} else if((surfaceFlags & SURF_SAND) || (surfaceFlags & SURF_SNOW)){
			trap_S_StartSound( origin, ENTITYNUM_WORLD, CHAN_AUTO,cgs.media.impact[IMPACT_SAND][rand()%3]);
		} else if(surfaceFlags & SURF_GRASS){
			trap_S_StartSound( origin, ENTITYNUM_WORLD, CHAN_AUTO,cgs.media.impact[IMPACT_GRASS][rand()%3]);
		} else
			trap_S_StartSound( origin, ENTITYNUM_WORLD, CHAN_AUTO,cgs.media.impact[IMPACT_DEFAULT][rand()%3]);
	}

	if(!cg_impactparticles.integer)
		return;

	//blood particles
	if(flesh){
		for(i=0; i<5;i++){
			float	temp;

			VectorCopy(dir, newdir);
			VectorNormalize(newdir);

			for(j=0; j<3; j++){
				temp = (rand()%40)-20;
				temp /= 10;

				newdir[j] += temp;
			}

			VectorNormalize(newdir);

			le = CG_LaunchSpriteParticle(origin, newdir, BOUNCE_LIGHT, 2000, (LEF_PARTICLE|LEF_BLOOD));
			le->refEntity.customShader = cgs.media.blood_particles[rand()%6];
			le->refEntity.radius = 1;

			le->color[0] = 1;
			le->color[1] = 1;
			le->color[2] = 1;
			le->color[3] = 1;

			if(i){
				le->pos.trDelta[0] /= sqrt(i);
				le->pos.trDelta[1] /= sqrt(i);
				le->pos.trDelta[2] /= sqrt(i);
			}
		}
		return;
	}

	VectorCopy(origin, spawnpoint);

	for(i=0; i<NUM_PREFIXINFO; i++){
		if((surfaceFlags & prefixInfo[i].surfaceFlags) ||
			!prefixInfo[i].surfaceFlags/*other*/){
			radius = prefixInfo[i].radius;
			weight = prefixInfo[i].weight;
			num	   = prefixInfo[i].numParticles;
			surfaceNum = i;

			if(weapon == WP_REMINGTON_GAUGE){
				num /= 3;
			} else if(weapon == WP_SAWEDOFF){
				num /= 4;
			} else if(weapon == WP_KNIFE){
				num /= 1.5f;
				weight *= 2.0f;
			}

			// set detail
			if(cg_impactparticles.integer == 1)
				num /= 2;

			break;
		}
	}

	for(i=0; i<num; i++){
		float	temp;
		shader = cgs.media.particles[surfaceNum][rand()%3];
		VectorCopy(dir, newdir);
		VectorNormalize(newdir);

		for(j=0; j<3; j++){
			temp = (rand()%20)-10;
			temp /= 10;

			newdir[j] += temp;
		}

		VectorNormalize(newdir);
		VectorScale( newdir, DIR_FACTOR*1/weight, newdir );

		if(newdir[2] > 0)
			newdir[2] *= 2.4f;
		else
			newdir[2] *= 0.5;

		le = CG_LaunchSpriteParticle(spawnpoint, newdir, BOUNCE_MEDIUM, 2000*1/weight, LEF_PARTICLE);

		le->refEntity.customShader = shader;
		le->refEntity.radius = radius;

		if(shaderNum != -1){

			Vector4Copy(shaderInfo[shaderNum].color, le->color);

			if(shaderInfo[shaderNum].surfaceFlags & SURF_WATER)
				le->color[3] = 0.5f;

		}
	}

	//additional specific surface-spawns
	/*if(prefixInfo[surfaceNum].surfaceFlags == SURF_METAL){
		CG_CreateSparkTrail(spawnpoint, dir);
	} else*/ if(prefixInfo[surfaceNum].surfaceFlags == SURF_WATER){
		localEntity_t *le;
		vec4_t	light, color;
		int		i;
		vec3_t	normal;

		le = CG_MakeExplosion( origin, dir,
			cgs.media.bulletFlashModel, cgs.media.bulletExplosionShader,
			375, qfalse, 8, qfalse);

		Vector4Copy(shaderInfo[shaderNum].color, color);
		VectorSet(normal, 0, 0, 1);

		CG_LightForPoint(origin, dir, light);

		for(i=0;i<3;i++){
			float temp = light[i]/255;
			temp = sqrt(temp);
			temp *= 255;

			color[i] *= temp*0.87f + 255*0.13f;
		}
		color[3] *= 255;

		Vector4Copy(color, le->refEntity.shaderRGBA);
	}

#if 0
	for(i=0; i<NUM_PREFIXINFO; i++){
		if(surfaceFlags & prefixInfo[i].surfaceFlags){
			Com_Printf("surface: %s\n", prefixInfo[i].name);
			return;
		}
	}

	CG_Printf("surface: other\n");
#endif
}

/*
==============================
These new Trace-functions
"decompress" the new surfaceFlag-
integers
==============================
*/
int	trap_CM_TransformedBoxTrace_New( trace_t *results, const vec3_t start, const vec3_t end,
						  const vec3_t mins, const vec3_t maxs,
						  clipHandle_t model, int brushmask,
						  const vec3_t origin, const vec3_t angles) {
	int shaderNum;

	trap_CM_TransformedBoxTrace( results, start, end, mins, maxs, model, brushmask,
						  origin, angles);

	//added by Spoon to decompress surfaceFlags
	if((results->contents & CONTENTS_SOLID) || (results->contents & CONTENTS_PLAYERCLIP)){
		shaderNum = results->surfaceFlags;
		results->surfaceFlags = shaderInfo[shaderNum].surfaceFlags;
	} else {
		shaderNum = -1;
	}

	return shaderNum;
}

int	trap_CM_BoxTrace_New( trace_t *results, const vec3_t start, const vec3_t end,
						  const vec3_t mins, const vec3_t maxs,
						  clipHandle_t model, int brushmask ) {
	int shaderNum;

	trap_CM_BoxTrace( results, start, end,
						  mins, maxs,
						  model, brushmask );

	//added by Spoon to decompress surfaceFlags
	if((results->contents & CONTENTS_SOLID) || (results->contents & CONTENTS_PLAYERCLIP)){
		shaderNum = results->surfaceFlags;
		results->surfaceFlags = shaderInfo[shaderNum].surfaceFlags;
	} else
		shaderNum = -1;

	return shaderNum;
}

#if 0
static void CreateTriangle(vec3_t vert1, vec3_t vert2, vec3_t vert3){
	polyVert_t	verts[3];

	VectorCopy(vert1, verts[0].xyz);
	verts[0].st[0] = 0;
	verts[0].st[1] = 0;
	verts[0].modulate[0] = 255;
	verts[0].modulate[1] = 255;
	verts[0].modulate[2] = 255;
	verts[0].modulate[3] = 255;

	VectorCopy(vert2, verts[1].xyz);
	verts[1].st[0] = 0;
	verts[1].st[1] = 1;
	verts[1].modulate[0] = 255;
	verts[1].modulate[1] = 255;
	verts[1].modulate[2] = 255;
	verts[1].modulate[3] = 255;

	VectorCopy(vert3, verts[2].xyz);
	verts[2].st[0] = 1;
	verts[2].st[1] = 1;
	verts[2].modulate[0] = 255;
	verts[2].modulate[1] = 255;
	verts[2].modulate[2] = 255;
	verts[2].modulate[3] = 255;

	trap_R_AddPolyToScene( cgs.media.menu_top, 3, verts );
}

void BuildHitPart(vec3_t offset_origin, vec3_t offset_angles, surf_data_t *surfData,
				  int frame, int oldframe, float backlerp, int numSurfaces){
	int j, i, k, l;
	vec3_t	maxs, mins;
	vec3_t	left, right;
	VectorSet(maxs, -999, -999, -999);
	VectorSet(mins, 999, 999, 999);
	VectorSet(right, -999, -999, -999);
	VectorSet(left, 999, 999, 999);

	//build the model
	for(j = 0; j < numSurfaces; j++){

		for(i = 0; i < surfData[j].header.numTriangles; i++){

			int	vec[3];
			vec3_t	vert[3];
			vec3_t	oldvert[3];

			if(i%2)
				continue;

			//prepare each vertex(k)
			for(k=0;k<3;k++){
				vec[k] = surfData[j].triangles[i][k];

				VectorCopy(surfData[j].verts[frame][vec[k]], vert[k]);
				VectorCopy(surfData[j].verts[oldframe][vec[k]], oldvert[k]);

				//do the lerp on the axes(l)
				for(l=0;l<3;l++){
					vert[k][l] = oldvert[k][l] + (vert[k][l]-oldvert[k][l])*(1-backlerp);
					vert[k][l] /= 64;
				}
				RotateVectorAroundVector(vec3_origin, offset_angles, vert[k]);
				VectorAdd(vert[k], offset_origin, vert[k]);

				if(maxs[2] < vert[k][2])
					VectorCopy(vert[k], maxs);
				if(mins[2] > vert[k][2])
					VectorCopy(vert[k], mins);

				if(vert[k][0] < left[0])
					VectorCopy(vert[k], left);
				if(vert[k][0] > right[0])
					VectorCopy(vert[k], right);
			}

			CreateTriangle(vert[0], vert[1], vert[2]);
		}
	}

	CG_Printf("maxs  %f %f %f\n", maxs[0], maxs[1], maxs[2]);
			CG_Printf("mins  %f %f %f\n", mins[0], mins[1], mins[2]);
			CG_Printf("left  %f %f %f\n", left[0], left[1], left[2]);
			CG_Printf("right %f %f %f\n\n", right[0], right[1], right[2]);
}

//calculates position and angles of tag
void TagOffset(vec3_t offset_origin, vec3_t offset_angles,
			   hit_tag_t *ptag, int frame, int oldframe, int backlerp){
	vec3_t	tag;
	vec3_t	angles;
	int		i;

	//calculate pos & angles of
	for(i=0;i<3;i++){
		tag[i] = ptag[oldframe].origin[i] +
			(ptag[frame].origin[i]-ptag[oldframe].origin[i])*(1-backlerp);

		angles[i] = ptag[oldframe].angles[i] +
			(ptag[frame].angles[i]-ptag[oldframe].angles[i])*(1-backlerp);
	}

	RotateVectorAroundVector(vec3_origin, offset_angles, tag);
	VectorAdd(tag, offset_origin, offset_origin);

	VectorAdd(angles, offset_angles, offset_angles);
}

void BuildHitModel(hit_data_t *data, vec3_t origin,
				   vec3_t angles, vec3_t torso_angles, vec3_t head_angles,
				   int frame_upper, int oldframe_upper, float backlerp_upper,
				   int frame_lower, int oldframe_lower, float backlerp_lower){

	vec3_t	offset_origin, offset_angles;

	if(!hit_model.integer)
		return;

	//CG_Printf("cg %f %f %f, %f %f %f\n", origin[0], origin[1], origin[2],
	//	head_angles[0], head_angles[1], head_angles[2]);

	VectorCopy(origin, offset_origin);
	VectorCopy(angles, offset_angles);

	//build the model
	//legs
	BuildHitPart(offset_origin, offset_angles, data->lower.surfData, frame_lower,
		oldframe_lower, backlerp_lower, data->lower.header.numSurfaces);

	//calculate pos of tag_torso
	TagOffset(offset_origin, offset_angles, data->lower.tag, frame_lower,
		oldframe_lower, backlerp_lower);

	//torso
	VectorAdd(offset_angles, torso_angles, offset_angles);
	BuildHitPart(offset_origin, offset_angles, data->upper.surfData, frame_upper,
		oldframe_upper, backlerp_upper, data->upper.header.numSurfaces);

	//calculate pos of tag_head
	TagOffset(offset_origin, offset_angles, data->upper.tag, frame_upper,
		oldframe_upper, backlerp_upper);

	//head
	VectorCopy(head_angles, offset_angles);
	BuildHitPart(offset_origin, offset_angles, data->head.surfData, 0,
		0, 0, data->head.header.numSurfaces);
}
#endif


#define SCREEN_SIZE 100
#define PLAYER_SIZE_RIGHT 13
#define PLAYER_SIZE_UP 70
#define	STEP		   3
qboolean CG_CheckPlayerVisible(vec3_t view, vec3_t player){
	vec3_t	start;
//	vec3_t	end;
	int		i, j;

	for( i = 0 ; i <= STEP; i++){
		trace_t	trace;
		vec3_t temp;

		//left/right
		VectorMA(view, (SCREEN_SIZE/STEP)*(i)-(SCREEN_SIZE/2), cg.refdef.viewaxis[1], start);

		CG_Trace( &trace, start, vec3_origin, vec3_origin, player,
			-1, (CONTENTS_SOLID|CONTENTS_BODY|CONTENTS_CORPSE) );

		if ( trace.entityNum < MAX_CLIENTS)
			return qfalse;

		if(Distance(start, player) < 100)
			return qfalse;

		VectorCopy(start, temp);

		for(j = 0; j <= STEP; j++){
			//up/down
			VectorMA(temp, (SCREEN_SIZE/STEP)*(j)-(SCREEN_SIZE/2), cg.refdef.viewaxis[2], start);

			CG_Trace( &trace, start, vec3_origin, vec3_origin, player,
				-1, (CONTENTS_SOLID|CONTENTS_BODY|CONTENTS_CORPSE) );

			if ( trace.entityNum < MAX_CLIENTS)
				return qfalse;

			if(Distance(start, player) < 100)
				return qfalse;
		}
	}

	return qtrue;
}

void CG_PlayReloadSound(int weapon, centity_t *cent, qboolean sec){
	sfxHandle_t sfx=0;

	//Com_Printf("reloading %s\n", bg_weaponlist[weapon].name);

	if(cg_weapons[weapon].reloadSound && !sec){
		sfx = cg_weapons[weapon].reloadSound;
	} else {
		switch(weapon){
		case WP_PEACEMAKER:
			sfx = cgs.media.peacem_reload2;
			break;
		case WP_REMINGTON_GAUGE:
		case WP_SAWEDOFF:
			sfx = cgs.media.reloadShotgun;
			break;
		case WP_WINCHESTER66:
		case WP_LIGHTNING:
			sfx = cgs.media.rifle_reload2;
			break;
		case WP_WINCH97:
			sfx = cgs.media.winch97_reload2;
			break;
		}
	}

	if(sfx){
		if(cg.snap->ps.clientNum == cent->currentState.clientNum)
			trap_S_StartSound(NULL, cent->currentState.number, CHAN_WEAPON, sfx);
		else
			trap_S_StartSound(cent->lerpOrigin, cent->currentState.number, CHAN_WEAPON, sfx);
	}
}

/*
===============
G_CopyLerpFrame
by: Spoon
19.10.2001
copies the data of one lerpframe-structure to another
===============
*/
void BG_CopyLerpFrame(lerpFrame_t *org, lerpFrame_t *targ){

	targ->animation = org->animation;
	targ->animationNumber = org->animationNumber;
	targ->animationTime = org->animationTime;
	targ->backlerp = org->backlerp;
	targ->frame = org->frame;
	targ->frameTime = org->frameTime;
	targ->oldFrame = org->oldFrame;
	targ->oldFrameTime = org->oldFrameTime;
	targ->pitchAngle = org->pitchAngle;
	targ->pitching = org->pitching;
	targ->weaponAnim = org->weaponAnim;
	targ->yawAngle = org->yawAngle;
	targ->yawing = org->yawing;
}

localEntity_t *CG_AddSmokeParticle(vec3_t origin, int radius, int randomradius, int lifetime,
								   int boost, int wind, vec3_t winddir){
	localEntity_t	*le;
	refEntity_t		*re;

	le = CG_AllocLocalEntity();
	re= &le->refEntity;

	le->leType = LE_SMOKE;
	le->startTime = cg.time;
	le->endTime = le->startTime + lifetime;

	re->reType = RT_SPRITE;
	re->radius = radius + rand()%randomradius;
	re->rotation = rand()%360;
	le->boost = boost;
	le->wind = wind;
	VectorCopy(winddir, le->vector);

	le->color[0] = 255;
	le->color[1] = 255;
	le->color[2] = 255;
	le->color[3] = 225;

	re->customShader = cgs.media.pistol_smoke;

	VectorCopy( origin, re->origin);
	VectorCopy( re->origin, le->pos.trBase );

	return le;
}

/*
===============
CG_PlayTalkSound
by: hika
27.10.2006

Function that triggers the talk sound when chatting/voting etc .
Does nothing if cg_talksound cvar is set to "0".
===============
*/
void CG_PlayTalkSound( void ) {

	switch (cg_talksound.integer) {
	case 0: /* Do nothing */
		break;
	case 1: /* Play the default sound */
		trap_S_StartLocalSound( cgs.media.talkSound, CHAN_LOCAL_SOUND );
		break;
	case 2:
		trap_S_StartLocalSound( cgs.media.talkSound1, CHAN_LOCAL_SOUND );
		break;
	case 3:
		trap_S_StartLocalSound( cgs.media.talkSound2, CHAN_LOCAL_SOUND );
		break;
	case 4:
		trap_S_StartLocalSound( cgs.media.talkSound3, CHAN_LOCAL_SOUND );
		break;
	default:
		trap_S_StartLocalSound( cgs.media.talkSound, CHAN_LOCAL_SOUND );
		break;
	}

}

/*
===============
CG_Cvar_VariableInteger
by: hika
2007-02-28

Do the same as CG_Cvar_Get() in cg_main.c
===============
*/
int CG_Cvar_VariableInteger(const char *cvar) {
	char buff[128];
	memset(buff, 0, sizeof(buff));
	trap_Cvar_VariableStringBuffer(cvar, buff, sizeof(buff));
	return atoi(buff);
}

/*
===============
CG_SetupFrustum
by: hika
2007-01-06

Imported from Q3 Engine ./code/renderer/tr_main.c
cg_frustum[0] : right plane
cg_frustum[1] : left plane
cg_frustum[2] : bottom plane
cg_frustum[3] : top plane
===============
*/
void CG_SetupFrustum( void ) {
	int		i;
	float	xs, xc;
	float	ang;

	if (sa_engine_inuse) {
		trap_R_GetFrustumPlane( cg_frustum );
		return;
	}

	ang = cg.refdef.fov_x / 180 * M_PI * 0.5f;
	xs = sin( ang );
	xc = cos( ang );

	VectorScale( cg.refdef.viewaxis[0], xs, cg_frustum[0].normal );
	VectorMA( cg_frustum[0].normal, xc, cg.refdef.viewaxis[1], cg_frustum[0].normal );

	VectorScale( cg.refdef.viewaxis[0], xs, cg_frustum[1].normal );
	VectorMA( cg_frustum[1].normal, -xc, cg.refdef.viewaxis[1], cg_frustum[1].normal );

	ang = cg.refdef.fov_y / 180 * M_PI * 0.5f;
	xs = sin( ang );
	xc = cos( ang );

	VectorScale( cg.refdef.viewaxis[0], xs, cg_frustum[2].normal );
	VectorMA( cg_frustum[2].normal, xc, cg.refdef.viewaxis[2], cg_frustum[2].normal );

	VectorScale( cg.refdef.viewaxis[0], xs, cg_frustum[3].normal );
	VectorMA( cg_frustum[3].normal, -xc, cg.refdef.viewaxis[2], cg_frustum[3].normal );

	for ( i = 0; i < 4; i++ ) {
		cg_frustum[i].type = PLANE_NON_AXIAL;
		cg_frustum[i].dist = DotProduct(cg.refdef.vieworg, cg_frustum[i].normal);
		SetPlaneSignbits( &cg_frustum[i] );
	}
}

/*
=================
CG_CullPointAndRadius
by: hika
2007-01-06

Imported from Q3 Engine ./code/renderer/tr_main.c
Returns CULL_IN, CULL_CLIP, or CULL_OUT
=================
*/
static int CG_CullPointAndRadius(vec3_t pt, float radius) {
	int		i;
	float	dist;
	cplane_t	*frust;
	qboolean mightBeClipped = qfalse;

	if (sa_engine_inuse)
		return trap_R_CullPointAndRadius( pt, radius );

	// check against frustum planes
	for (i = 0 ; i < 4 ; i++)	{
		frust = &cg_frustum[i];

		dist = DotProduct( pt, frust->normal) - frust->dist;
		if ( dist < -radius )
			return CULL_OUT;
		else if ( dist <= radius )
			mightBeClipped = qtrue;
	}

	if ( mightBeClipped )
		return CULL_CLIP;

	return CULL_IN;		// completely inside frustum
}

/*
=================
CG_CullBoundingBox
by: hika
2007-01-06

Imported from Q3 Engine ./code/renderer/tr_main.c
Returns CULL_IN, CULL_CLIP, or CULL_OUT
=================
*/
static int CG_CullBoundingBox(vec3_t box_vertex[8]) {
	int		i, j;
	float	dists[8];
	cplane_t	*frust;
	int			anyBack;
	int			front, back;

	if (sa_engine_inuse)
		return trap_R_CullBoundingBox( box_vertex );

	// check against frustum planes
	anyBack = 0;
	for (i = 0 ; i < 4 ; i++) {
		frust = &cg_frustum[i];

		front = back = 0;
		for (j = 0 ; j < 8 ; j++) {
			dists[j] = DotProduct(box_vertex[j], frust->normal);
			if ( dists[j] > frust->dist ) {
				front = 1;
				if ( back ) {
					break;		// a point is in front
				}
			} else {
				back = 1;
			}
		}
		if ( !front ) {
			// all points were behind one of the planes
			return CULL_OUT;
		}
		anyBack |= back;
	}

	if ( !anyBack ) {
		return CULL_IN;		// completely inside frustum
	}

	return CULL_CLIP;		// partially clipped
}

/*
===============
CG_IsSpotVisible
by: hika
2006-12-27

Check if the vec3_t "spot" is visible from the camera point of view.

NOTES:
- if max_sight <= 0, this parameter is ignored
===============
*//*
qboolean CG_IsSpotVisible(vec3_t spot, vec_t max_sight) {

	vec3_t v, v_playerAxis;
	vec2_t verticalPlane, horizontalPlane;

	// First, get a line from viewPos to spot.
	VectorSubtract(spot, cg.refdef.vieworg, v);

	if (max_sight > 0	&& VectorLength(v) > max_sight)
		return qfalse;	// The spot is too far away

	// Set the vector, according to the camera's view direction
	VectorRotate(v, cg.refdef.viewaxis, v_playerAxis);

	// After the rotate, from the camera's view direction,
	//                   ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	// Take the following considerations :
	// [0] is the front axe (Back pedal is negative value)
	// [1] is the width axe (Left side is positive value)
	// [2] is still the height (Top is positive value)

	if (v_playerAxis[0] < 0)
		return qfalse;

	// Positive all values, so that we compare only unsigned values
	if (v_playerAxis[0] < 0)
		v_playerAxis[0] *= -1;
	if (v_playerAxis[1] < 0)
		v_playerAxis[1] *= -1;
	if (v_playerAxis[2] < 0)
		v_playerAxis[2] *= -1;

	// Horizontal plane with the given angle, to the right camera's point of view (positive axis)
	horizontalPlane[0] = v_playerAxis[0] * tan(cg.refdef.fov_x * M_PI / 360);
	horizontalPlane[1] = v_playerAxis[0];
	// Vectical plane with the given angle, to the top camera's point of view (positive axis)
	verticalPlane[0] = v_playerAxis[0];
	verticalPlane[1] = v_playerAxis[0] * tan(cg.refdef.fov_y * M_PI / 360);

	if (v_playerAxis[1] > horizontalPlane[0])
		return qfalse;
	if (v_playerAxis[2] > verticalPlane[1])
		return qfalse;

	return qtrue;
}*/

/*
===============
CG_IsEntityVisible
by: hika
2007-01-06

Use of cg.refdef
Perform frustum culling on bounding box

Only keep last result when max_sight == 0.0
===============
*/
qboolean CG_IsEntityVisible(centity_t *cent, vec_t max_sight) {
	qboolean hasBounds;
	vec3_t origin;
	vec3_t box_vertex[8];
	vec_t distance;
	qboolean visible=qtrue;
	vec_t radius;

	if (cent->visibleTime == cg.time && !max_sight)
		return cent->visible;	// Return the previous result

	hasBounds = CG_EntityVectors(cent, origin, box_vertex);
	// Keep the bounding box entity origin.
	VectorCopy( origin, cent->centerOrigin );

	// Get the distance, between entity origin and the camera
	// !! This is not really accurate, because we use the entity origin !!
	// But, when we want to compare distances and check what entities are farther
	// than the given distance, it works in most of the case.
	distance = Distance(cg.refdef.vieworg, origin);
	if (max_sight > 0.0 && distance > max_sight)
		return qfalse;	// The spot is too far away

	if (cent->currentState.number == cg.predictedPlayerState.groundEntityNum)
		// Keep current player ground entity visible to avoid player fall prediction
		// even if it is not really visible as player ground entity can only be
		// well predicted if it is visible (traces will only detect visible entities
		// when cg_boostfps is set)
		visible = qtrue;
	else if (distance < FORCE_VISIBILITY_RANGE)
		// Btw keep too closed entities as visible to avoid prediction error
		// when moving over it and not being in the fov
		visible = qtrue;
	else if (hasBounds) {
		radius = Distance(box_vertex[5], box_vertex[3]) * 0.5f;
		cent->radius = radius;	// Keep its radius

		// First check against a bounding sphere
		switch (CG_CullPointAndRadius(origin, radius)) {
		case CULL_IN:
			visible = qtrue;
			break;
		case CULL_CLIP:
			switch (cent->currentState.eType) {
			case ET_FLARE:
				// Only frustum check against sphere for flares.
				visible = qtrue;
				break;
			default:
				// Then, against a bounding box
				switch (CG_CullBoundingBox(box_vertex)) {
				case CULL_IN:
				case CULL_CLIP:
					visible = qtrue;
					break;
				default:
					visible = qfalse;
					break;
				}
				break;
			}
			break;
		case CULL_OUT:
			visible = qfalse;
			break;
		}
	}
	else
		visible = CG_CullPointAndRadius(origin, 0.0f) != CULL_OUT;

	if (!max_sight) {
	// Keep result
		cent->visible = visible;
		cent->visibleTime = cg.time;
		cent->distFromCamera = distance;
	}
	return visible;
}

/*
===============
CG_IsLocalEntityVisible
by: hika
2007-01-01

For instance, only check its origin visibility
** TODO **: have to get its bounds ?

Only keep last result when max_sight == 0.0
===============
*/
qboolean CG_IsLocalEntityVisible(localEntity_t *lent, vec_t max_sight) {
	qboolean visible;
	vec_t distance;

	if (lent->visibleTime == cg.time && !max_sight)
		return lent->visible;	// Return the previous result

	// Get the distance, between entity origin and the camera
	// !! This is not really accurate, because we use the entity origin !!
	// But, when we want to compare distances and check what entities are farther
	// than the given distance, it works in most of the case.
	distance = Distance(cg.refdef.vieworg, lent->refEntity.origin);
	if (max_sight > 0.0 && distance > max_sight)
		return qfalse;	// The spot is too far away

	visible = CG_CullPointAndRadius(lent->refEntity.origin, 0.0f) != CULL_OUT;
	if (!max_sight) {
		// Keep result
		lent->visible = visible;
		lent->visibleTime = cg.time;
		lent->distFromCamera = distance;
	}
	return visible;
}

/*
===============
CG_ColorByEntityType
by: hika
2006-12-28

Only used by CG_AddBoundingBoxEntity.
Don't question color choices.
It is completely arbitrary ;)
===============
*/
static void CG_ColorByEntityType(int eType, byte colors[4]) {

	// Default
	colors[0] = 220;
	colors[1] = 220;
	colors[2] = 100;
	colors[3] = 192;

	switch (eType) {
	case ET_GENERAL:
		colors[0] = 255;
		colors[1] = 0;
		colors[2] = 0;
		colors[3] = 255;
		break;
	case ET_PLAYER:
		colors[0] = 192;
		colors[1] = 0;
		colors[2] = 0;
		colors[3] = 255;
		break;
	case ET_ITEM:
		colors[0] = 128;
		colors[1] = 0;
		colors[2] = 0;
		colors[3] = 255;
		break;
	case ET_MISSILE:
		colors[0] = 64;
		colors[1] = 0;
		colors[2] = 0;
		colors[3] = 255;
		break;
	case ET_MOVER:
		colors[0] = 0;
		colors[1] = 255;
		colors[2] = 0;
		colors[3] = 255;
		break;
	case ET_BEAM:
		colors[0] = 0;
		colors[1] = 192;
		colors[2] = 0;
		colors[3] = 255;
		break;
	case ET_PORTAL:
		colors[0] = 0;
		colors[1] = 128;
		colors[2] = 0;
		colors[3] = 255;
		break;
	case ET_SPEAKER:
		colors[0] = 0;
		colors[1] = 64;
		colors[2] = 0;
		colors[3] = 255;
		break;
	case ET_PUSH_TRIGGER:
		colors[0] = 0;
		colors[1] = 0;
		colors[2] = 255;
		colors[3] = 255;
		break;
	case ET_TELEPORT_TRIGGER:
		colors[0] = 0;
		colors[1] = 0;
		colors[2] = 192;
		colors[3] = 255;
		break;
	case ET_INVISIBLE:
		colors[0] = 0;
		colors[1] = 0;
		colors[2] = 128;
		colors[3] = 255;
		break;
	case ET_FLY:
		colors[0] = 0;
		colors[1] = 0;
		colors[2] = 64;
		colors[3] = 255;
		break;
	case ET_BREAKABLE:
		colors[0] = 128;
		colors[1] = 128;
		colors[2] = 0;
		colors[3] = 255;
		break;
	case ET_INTERMISSION:
		colors[0] = 64;
		colors[1] = 64;
		colors[2] = 0;
		colors[3] = 255;
		break;
	case ET_FLARE:
		colors[0] = 0;
		colors[1] = 255;
		colors[2] = 255;
		colors[3] = 255;
		break;
	case ET_SMOKE:
		colors[0] = 0;
		colors[1] = 192;
		colors[2] = 192;
		colors[3] = 255;
		break;
	case ET_TURRET:
		colors[0] = 0;
		colors[1] = 128;
		colors[2] = 128;
		colors[3] = 255;
		break;
	case ET_ESCAPE:
		colors[0] = 0;
		colors[1] = 64;
		colors[2] = 64;
		colors[3] = 255;
		break;
	}
}

/*
=================
TransposeMatrix
by: hika
2006-12-30

Imported from Q3 Engine ./code/qcommon/cm_trace.c
=================
*/
static void TransposeMatrix(vec3_t matrix[3], vec3_t transpose[3]) {
	int i, j;
	for (i = 0; i < 3; i++) {
		for (j = 0; j < 3; j++) {
			transpose[i][j] = matrix[j][i];
		}
	}
}

/*
=================
CG_EntityVectors
by: hika
2006-12-30

Get origin vector and bounding box vextex vectors from an entity.
If box_vertex is NULL, it will be ignored.
=> qfalse, if the entity has no bounds vertex.

The bounds index are in the following order.

    2+------+3     ^ z
    /|     /|      |  y
  1/ |   0/ |      |/
  +------+  |      +--> x
  | 6+---|--+7
  | /    | /
  |/     |/
 5+------+4

=================
*/
qboolean CG_EntityVectors(centity_t *cent, vec3_t origin, vec3_t box_vertex[8]) {

	float *vmodel;
	vec3_t bmaxs, bmins;
	entityState_t *ent;
	clipHandle_t cmodel;
	vec3_t angles, r, vertex, rotate_origin={ 0.0f, 0.0f, 0.0f }, median, ext_xyz;
	vec3_t matrix[3], transpose[3];
	int i, x, zd, zu;
	qboolean rotate = qfalse;
	qboolean hasBounds = qtrue;

	if (!origin)	// Output parameter origin have to be set
		return qfalse;

	ent = &cent->currentState;

	if (ent->solid == SOLID_BMODEL) {

		vmodel = cgs.inlineModelMidpoints[ ent->modelindex ];
		VectorAdd( cent->lerpOrigin, vmodel, origin );

		if (!box_vertex)
			return qtrue;		// Has bounds

		cmodel = cgs.inlineDrawModel[ ent->modelindex ];
		trap_R_ModelBounds( cmodel, bmins, bmaxs );

		for ( i = 0; i < 3; i++ ) {
			// Store a vector for one box side
			ext_xyz[i] = bmaxs[i] - bmins[i];

			// And store a *positive* vector, from origin to one vertex corner.
			median[i] = bmaxs[i] - ( bmaxs[i] + bmins[i] ) * 0.5;
		}

		VectorCopy( cent->lerpAngles, angles );
		// Rotate only solid models ?
		if (angles[0] || angles[1] || angles[2]) {
			rotate = qtrue;
			VectorCopy( cent->lerpOrigin, rotate_origin );
		}
	}
	else if (ent->solid) {
		VectorCopy( cent->lerpOrigin, origin );

		if (!box_vertex)
			return qtrue;		// Has bounds

		// encoded bbox
		x = (ent->solid & 255);
		zd = ((ent->solid >> 8) & 255);
		zu = ((ent->solid >> 16) & 255) - 32;

		median[0] = x;
		median[1] = x;
		median[2] = zu;

		ext_xyz[0] = 2 * x;
		ext_xyz[1] = 2 * x;
		ext_xyz[2] = zu + zd;
	}
	else {
		// Not a solid
		VectorCopy( cent->lerpOrigin, origin );

		if (!box_vertex)
			return qfalse;	// No bounds

		// Default values
		median[0] = 3;
		median[1] = 3;
		median[2] = 3;

		ext_xyz[0] = 6;
		ext_xyz[1] = 6;
		ext_xyz[2] = 6;

		hasBounds = qfalse;
	}

	// Vertex bounds for the "horizontal" top box.
	VectorAdd( origin, median, box_vertex[3] );	// Top forward right

	VectorCopy( box_vertex[3], box_vertex[2] );	// Top forward left
	box_vertex[2][0] -= ext_xyz[0];

	VectorCopy( box_vertex[2], box_vertex[1] );	// Top backward left
	box_vertex[1][1] -= ext_xyz[1];

	VectorCopy( box_vertex[1], box_vertex[0] );	// Top backward right
	box_vertex[0][0] += ext_xyz[0];

	// Vertex bounds for the "horizontal" bottom box.
	for ( i = 0; i < 4; i++ ) {
		VectorCopy( box_vertex[i], box_vertex[i + 4] );
		box_vertex[i + 4][2] -= ext_xyz[2];
	}

	// Rotate all the bounds, if needed
	if (rotate) {
		// Get the entity axis's point of view.
		AnglesToAxis( angles, matrix );
		// Transpose matrix as it is the entity axis point of view, which rotate.
		// Consider that it is not each bounds that rotate, but the entity axis.
		// So, if the entity axis rotate in one direction, the bounds will be rotate in the opposite,
		// from the entity axis point of view.
		// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
		TransposeMatrix( matrix, transpose );
		for ( i = 0; i < 8; i++ ) {
			// Rotate should be done from the rotation origin, not the entity origin
			VectorSubtract( box_vertex[i], rotate_origin, vertex );
			VectorRotate( vertex, transpose, r );

			// Apply the rotation result.
			VectorAdd( rotate_origin, r, box_vertex[i] );
		}
	}

	return hasBounds;
}

/*
=================
CG_AddBoundingBoxEntity
by: hika
2006-12-29

Same as CG_AddBoundingBox in cg_unlagged, except that it can be used for an
entity other than a player.
Call it in CG_AddCEntity, if needed.
=================
*/
void CG_AddBoundingBoxEntity(centity_t *cent) {
	polyVert_t verts[4];
	int i;
	vec3_t box_vertex[8];
	qhandle_t bboxShader, bboxShader_nocull;
	entityState_t *ent;
	vec3_t origin;

	ent = &cent->currentState;

	if ( !cg_drawBBox.integer )
		return;

	// get the shader handles
	bboxShader = trap_R_RegisterShader( "bbox" );
	bboxShader_nocull = trap_R_RegisterShader( "bbox_nocull" );

	// if they don't exist, forget it
	if ( !bboxShader || !bboxShader_nocull ) {
		return;
	}

	CG_EntityVectors(cent, origin, box_vertex);

	// set the polygon's texture coordinates
	verts[0].st[0] = 0;
	verts[0].st[1] = 0;
	verts[1].st[0] = 0;
	verts[1].st[1] = 1;
	verts[2].st[0] = 1;
	verts[2].st[1] = 1;
	verts[3].st[0] = 1;
	verts[3].st[1] = 0;

	// Set the bounding box color, according to the entity type
	for ( i = 0; i < 4; i++ ) {
		CG_ColorByEntityType( ent->eType, verts[i].modulate );
	}

	// top
	VectorCopy( box_vertex[0], verts[0].xyz );
	VectorCopy( box_vertex[1], verts[1].xyz );
	VectorCopy( box_vertex[2], verts[2].xyz );
	VectorCopy( box_vertex[3], verts[3].xyz );
	trap_R_AddPolyToScene( bboxShader, 4, verts );

	// bottom
	VectorCopy( box_vertex[7], verts[0].xyz );
	VectorCopy( box_vertex[6], verts[1].xyz );
	VectorCopy( box_vertex[5], verts[2].xyz );
	VectorCopy( box_vertex[4], verts[3].xyz );
	trap_R_AddPolyToScene( bboxShader, 4, verts );

	// top side
	VectorCopy( box_vertex[3], verts[0].xyz );
	VectorCopy( box_vertex[2], verts[1].xyz );
	VectorCopy( box_vertex[6], verts[2].xyz );
	VectorCopy( box_vertex[7], verts[3].xyz );
	trap_R_AddPolyToScene( bboxShader_nocull, 4, verts );

	// left side
	VectorCopy( box_vertex[2], verts[0].xyz );
	VectorCopy( box_vertex[1], verts[1].xyz );
	VectorCopy( box_vertex[5], verts[2].xyz );
	VectorCopy( box_vertex[6], verts[3].xyz );
	trap_R_AddPolyToScene( bboxShader_nocull, 4, verts );

	// right side
	VectorCopy( box_vertex[0], verts[0].xyz );
	VectorCopy( box_vertex[3], verts[1].xyz );
	VectorCopy( box_vertex[7], verts[2].xyz );
	VectorCopy( box_vertex[4], verts[3].xyz );
	trap_R_AddPolyToScene( bboxShader_nocull, 4, verts );

	// bottom side
	VectorCopy( box_vertex[1], verts[0].xyz );
	VectorCopy( box_vertex[0], verts[1].xyz );
	VectorCopy( box_vertex[4], verts[2].xyz );
	VectorCopy( box_vertex[5], verts[3].xyz );
	trap_R_AddPolyToScene( bboxShader_nocull, 4, verts );
}



/*
=================
CG_Farclip_*
by: Joe Kari
2009-1-12

Function that match farclip_t type defined in bg_public.h, 
returning qtrue if the entity should be clipped
or qfalse if the entity should be add to the scene
=================
*/

qboolean CG_Farclip_None( vec3_t entite_vec3 , vec3_t camera_vec3 , float farclip_dist , float farclip_alt_dist ) {
	return qfalse ;
}

qboolean CG_Farclip_Always( vec3_t entite_vec3 , vec3_t camera_vec3 , float farclip_dist , float farclip_alt_dist ) {
	return qtrue ;
}

qboolean CG_Farclip_Sphere( vec3_t entite_vec3 , vec3_t camera_vec3 , float farclip_dist , float farclip_alt_dist )
{
	float dx , dy , dz ;
	dx = entite_vec3[0] - camera_vec3[0] ;
	dy = entite_vec3[1] - camera_vec3[1] ;
	dz = entite_vec3[2] - camera_vec3[2] ;
	return ( dx * dx + dy * dy + dz * dz ) > ( farclip_dist * farclip_dist ) ;
}

qboolean CG_Farclip_Cube( vec3_t entite_vec3 , vec3_t camera_vec3 , float farclip_dist , float farclip_alt_dist )
{
	if ( abs( entite_vec3[0] - camera_vec3[0] ) > farclip_dist )  return qtrue ;
	if ( abs( entite_vec3[1] - camera_vec3[1] ) > farclip_dist )  return qtrue ;
	if ( abs( entite_vec3[2] - camera_vec3[2] ) > farclip_dist )  return qtrue ;
	return qfalse ;
}

qboolean CG_Farclip_Ellipse_Z( vec3_t entite_vec3 , vec3_t camera_vec3 , float farclip_dist , float farclip_alt_dist )
{
	float dx , dy , dz ;
	if ( ( farclip_alt_dist < 1 ) || ( farclip_dist < 1 ) )  return qtrue ;
	dx = entite_vec3[0] - camera_vec3[0] ;
	dy = entite_vec3[1] - camera_vec3[1] ;
	dz = ( entite_vec3[2] - camera_vec3[2] ) * farclip_dist / farclip_alt_dist ;
	return ( dx * dx + dy * dy + dz * dz ) > ( farclip_dist * farclip_dist ) ;
}

qboolean CG_Farclip_Cylinder_Z( vec3_t entite_vec3 , vec3_t camera_vec3 , float farclip_dist , float farclip_alt_dist )
{
	float dx , dy ;
	if ( abs( entite_vec3[2] - camera_vec3[2] ) > farclip_alt_dist )  return qtrue ;
	dx = entite_vec3[0] - camera_vec3[0] ;
	dy = entite_vec3[1] - camera_vec3[1] ;
	return ( dx * dx + dy * dy ) > ( farclip_dist * farclip_dist ) ;
}

qboolean CG_Farclip_Box_Z( vec3_t entite_vec3 , vec3_t camera_vec3 , float farclip_dist , float farclip_alt_dist )
{
	if ( abs( entite_vec3[0] - camera_vec3[0] ) > farclip_dist )  return qtrue ;
	if ( abs( entite_vec3[1] - camera_vec3[1] ) > farclip_dist )  return qtrue ;
	if ( abs( entite_vec3[2] - camera_vec3[2] ) > farclip_alt_dist )  return qtrue ;
	return qfalse ;
}

qboolean CG_Farclip_Circle_Infinite_Z( vec3_t entite_vec3 , vec3_t camera_vec3 , float farclip_dist , float farclip_alt_dist )
{
	float dx , dy ;
	dx = entite_vec3[0] - camera_vec3[0] ;
	dy = entite_vec3[1] - camera_vec3[1] ;
	return ( dx * dx + dy * dy ) > ( farclip_dist * farclip_dist ) ;
}

qboolean CG_Farclip_Square_Infinite_Z( vec3_t entite_vec3 , vec3_t camera_vec3 , float farclip_dist , float farclip_alt_dist )
{
	if ( abs( entite_vec3[0] - camera_vec3[0] ) > farclip_dist )  return qtrue ;
	if ( abs( entite_vec3[1] - camera_vec3[1] ) > farclip_dist )  return qtrue ;
	return qfalse ;
}

qboolean CG_Farclip_Cone_Z( vec3_t entite_vec3 , vec3_t camera_vec3 , float farclip_dist , float farclip_alt_dist )
{
	float dx , dy , dz ;
	dx = entite_vec3[0] - camera_vec3[0] ;
	dy = entite_vec3[1] - camera_vec3[1] ;
	// not usefull to test if farclip_dist > 0, cause it have been tested before calling this function
	dz = abs( entite_vec3[2] - camera_vec3[2] ) * farclip_alt_dist / farclip_dist + farclip_dist ;
	return ( dx * dx + dy * dy ) > ( dz * dz ) ;
}

qboolean CG_Farclip_Pyramid_Z( vec3_t entite_vec3 , vec3_t camera_vec3 , float farclip_dist , float farclip_alt_dist )
{
	float dz ;
	// not usefull to test if farclip_dist > 0, cause it have been tested before calling this function
	dz = abs( entite_vec3[2] - camera_vec3[2] ) * farclip_alt_dist / farclip_dist + farclip_dist ;
	if ( abs( entite_vec3[0] - camera_vec3[0] ) > dz )  return qtrue ;
	if ( abs( entite_vec3[1] - camera_vec3[1] ) > dz )  return qtrue ;
	return qfalse ;
}




qboolean CG_Farclip_Ellipse_X( vec3_t entite_vec3 , vec3_t camera_vec3 , float farclip_dist , float farclip_alt_dist )
{
	float dx , dy , dz ;
	if ( ( farclip_alt_dist < 1 ) || ( farclip_dist < 1 ) )  return qtrue ;
	dx = entite_vec3[0] - camera_vec3[0] * farclip_dist / farclip_alt_dist ;
	dy = entite_vec3[1] - camera_vec3[1] ;
	dz = ( entite_vec3[2] - camera_vec3[2] ) ;
	return ( dx * dx + dy * dy + dz * dz ) > ( farclip_dist * farclip_dist ) ;
}

qboolean CG_Farclip_Cylinder_X( vec3_t entite_vec3 , vec3_t camera_vec3 , float farclip_dist , float farclip_alt_dist )
{
	float dy , dz ;
	if ( abs( entite_vec3[0] - camera_vec3[0] ) > farclip_alt_dist )  return qtrue ;
	dy = entite_vec3[1] - camera_vec3[1] ;
	dz = entite_vec3[2] - camera_vec3[2] ;
	return ( dy * dy + dz * dz ) > ( farclip_dist * farclip_dist ) ;
}

qboolean CG_Farclip_Box_X( vec3_t entite_vec3 , vec3_t camera_vec3 , float farclip_dist , float farclip_alt_dist )
{
	if ( abs( entite_vec3[0] - camera_vec3[0] ) > farclip_alt_dist )  return qtrue ;
	if ( abs( entite_vec3[1] - camera_vec3[1] ) > farclip_dist )  return qtrue ;
	if ( abs( entite_vec3[2] - camera_vec3[2] ) > farclip_dist )  return qtrue ;
	return qfalse ;
}

qboolean CG_Farclip_Circle_Infinite_X( vec3_t entite_vec3 , vec3_t camera_vec3 , float farclip_dist , float farclip_alt_dist )
{
	float dy , dz ;
	dy = entite_vec3[1] - camera_vec3[1] ;
	dz = entite_vec3[2] - camera_vec3[2] ;
	return ( dy * dy + dz * dz ) > ( farclip_dist * farclip_dist ) ;
}

qboolean CG_Farclip_Square_Infinite_X( vec3_t entite_vec3 , vec3_t camera_vec3 , float farclip_dist , float farclip_alt_dist )
{
	if ( abs( entite_vec3[1] - camera_vec3[1] ) > farclip_dist )  return qtrue ;
	if ( abs( entite_vec3[2] - camera_vec3[2] ) > farclip_dist )  return qtrue ;
	return qfalse ;
}

qboolean CG_Farclip_Cone_X( vec3_t entite_vec3 , vec3_t camera_vec3 , float farclip_dist , float farclip_alt_dist )
{
	float dx , dy , dz ;
	// not usefull to test if farclip_dist > 0, cause it have been tested before calling this function
	dx = abs( entite_vec3[0] - camera_vec3[0] ) * farclip_alt_dist / farclip_dist + farclip_dist ;
	dy = entite_vec3[1] - camera_vec3[1] ;
	dz = entite_vec3[2] - camera_vec3[2] ;
	return ( dy * dy + dz * dz ) > ( dx * dx ) ;
}

qboolean CG_Farclip_Pyramid_X( vec3_t entite_vec3 , vec3_t camera_vec3 , float farclip_dist , float farclip_alt_dist )
{
	float dx ;
	// not usefull to test if farclip_dist > 0, cause it have been tested before calling this function
	dx = abs( entite_vec3[0] - camera_vec3[0] ) * farclip_alt_dist / farclip_dist + farclip_dist ;
	if ( abs( entite_vec3[1] - camera_vec3[1] ) > dx )  return qtrue ;
	if ( abs( entite_vec3[2] - camera_vec3[2] ) > dx )  return qtrue ;
	return qfalse ;
}




qboolean CG_Farclip_Ellipse_Y( vec3_t entite_vec3 , vec3_t camera_vec3 , float farclip_dist , float farclip_alt_dist )
{
	float dx , dy , dz ;
	if ( ( farclip_alt_dist < 1 ) || ( farclip_dist < 1 ) )  return qtrue ;
	dx = entite_vec3[0] - camera_vec3[0] ;
	dy = entite_vec3[1] - camera_vec3[1] * farclip_dist / farclip_alt_dist ;
	dz = ( entite_vec3[2] - camera_vec3[2] ) ;
	return ( dx * dx + dy * dy + dz * dz ) > ( farclip_dist * farclip_dist ) ;
}

qboolean CG_Farclip_Cylinder_Y( vec3_t entite_vec3 , vec3_t camera_vec3 , float farclip_dist , float farclip_alt_dist )
{
	float dx , dz ;
	if ( abs( entite_vec3[1] - camera_vec3[1] ) > farclip_alt_dist )  return qtrue ;
	dx = entite_vec3[0] - camera_vec3[0] ;
	dz = entite_vec3[2] - camera_vec3[2] ;
	return ( dx * dx + dz * dz ) > ( farclip_dist * farclip_dist ) ;
}

qboolean CG_Farclip_Box_Y( vec3_t entite_vec3 , vec3_t camera_vec3 , float farclip_dist , float farclip_alt_dist )
{
	if ( abs( entite_vec3[0] - camera_vec3[0] ) > farclip_dist )  return qtrue ;
	if ( abs( entite_vec3[1] - camera_vec3[1] ) > farclip_alt_dist )  return qtrue ;
	if ( abs( entite_vec3[2] - camera_vec3[2] ) > farclip_dist )  return qtrue ;
	return qfalse ;
}

qboolean CG_Farclip_Circle_Infinite_Y( vec3_t entite_vec3 , vec3_t camera_vec3 , float farclip_dist , float farclip_alt_dist )
{
	float dx , dz ;
	dx = entite_vec3[0] - camera_vec3[0] ;
	dz = entite_vec3[2] - camera_vec3[2] ;
	return ( dx * dx + dz * dz ) > ( farclip_dist * farclip_dist ) ;
}

qboolean CG_Farclip_Square_Infinite_Y( vec3_t entite_vec3 , vec3_t camera_vec3 , float farclip_dist , float farclip_alt_dist )
{
	if ( abs( entite_vec3[0] - camera_vec3[0] ) > farclip_dist )  return qtrue ;
	if ( abs( entite_vec3[2] - camera_vec3[2] ) > farclip_dist )  return qtrue ;
	return qfalse ;
}

qboolean CG_Farclip_Cone_Y( vec3_t entite_vec3 , vec3_t camera_vec3 , float farclip_dist , float farclip_alt_dist )
{
	float dx , dy , dz ;
	dx = entite_vec3[0] - camera_vec3[0] ;
	// not usefull to test if farclip_dist > 0, cause it have been tested before calling this function
	dy = abs( entite_vec3[1] - camera_vec3[1] ) * farclip_alt_dist / farclip_dist + farclip_dist ;
	dz = entite_vec3[2] - camera_vec3[2] ;
	return ( dx * dx + dz * dz ) > ( dy * dy ) ;
}

qboolean CG_Farclip_Pyramid_Y( vec3_t entite_vec3 , vec3_t camera_vec3 , float farclip_dist , float farclip_alt_dist )
{
	float dy ;
	// not usefull to test if farclip_dist > 0, cause it have been tested before calling this function
	dy = abs( entite_vec3[1] - camera_vec3[1] ) * farclip_alt_dist / farclip_dist + farclip_dist ;
	if ( abs( entite_vec3[0] - camera_vec3[0] ) > dy )  return qtrue ;
	if ( abs( entite_vec3[2] - camera_vec3[2] ) > dy )  return qtrue ;
	return qfalse ;
}

// function pointer array

qboolean ( * CG_Farclip_Tester[] )( vec3_t , vec3_t , float , float ) = {
        CG_Farclip_None ,
        CG_Farclip_Always ,
        CG_Farclip_Sphere ,
        CG_Farclip_Ellipse_X ,
        CG_Farclip_Ellipse_Y ,
        CG_Farclip_Ellipse_Z ,
        CG_Farclip_Cylinder_X ,
        CG_Farclip_Cylinder_Y ,
        CG_Farclip_Cylinder_Z ,
        CG_Farclip_Cube ,
        CG_Farclip_Box_X ,
        CG_Farclip_Box_Y ,
        CG_Farclip_Box_Z ,
        CG_Farclip_Cone_X ,
        CG_Farclip_Cone_Y ,
        CG_Farclip_Cone_Z ,
        CG_Farclip_Pyramid_X ,
        CG_Farclip_Pyramid_Y ,
        CG_Farclip_Pyramid_Z ,
        CG_Farclip_Circle_Infinite_X ,
        CG_Farclip_Circle_Infinite_Y ,
        CG_Farclip_Circle_Infinite_Z ,
        CG_Farclip_Square_Infinite_X ,
        CG_Farclip_Square_Infinite_Y ,
        CG_Farclip_Square_Infinite_Z
} ;

int CG_Farclip_Tester_Table_Size = ARRAY_LEN( CG_Farclip_Tester );
