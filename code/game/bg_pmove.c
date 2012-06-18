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
// bg_pmove.c -- both games player movement code
// takes a playerstate and a usercmd as input and returns a modifed playerstate

#include "../qcommon/q_shared.h"
#include "bg_public.h"
#include "bg_local.h"

pmove_t		*pm;
pml_t		pml;

// movement parameters
#ifndef SMOKINGUNS
float	pm_stopspeed = 100.0f;
float	pm_duckScale = 0.25f;
float	pm_swimScale = 0.50f;

float	pm_accelerate = 10.0f;
float	pm_airaccelerate = 1.0f;
float	pm_wateraccelerate = 4.0f;
float	pm_flyaccelerate = 8.0f;

float	pm_friction = 6.0f;
float	pm_waterfriction = 1.0f;
float	pm_flightfriction = 3.0f;
float	pm_spectatorfriction = 5.0f;
#else
float	pm_stopspeed = 20.0f;
float	pm_duckScale = 0.50f;
float	pm_swimScale = 0.60f;
float	pm_ladderScale = 0.80f;  // Set the max movement speed to 80% of normal
float	pm_reloadScale = 0.75f;

float	pm_accelerate = 8.4f;
float	pm_airaccelerate = 1.0f;
float	pm_wateraccelerate = 5.6f;
float	pm_flyaccelerate = 8.0f;
float	pm_ladderAccelerate = 3000;  // The acceleration to friction ratio is 1:1

float	pm_friction = 6.0f;
float	pm_waterfriction = 1.2f;
float	pm_flightfriction = 3.0f;
float	pm_spectatorfriction = 5.0f;
float	pm_ladderfriction = 3000;  // Friction is high enough so you don't slip down
#endif

int		c_pmove = 0;


/*
===============
PM_AddEvent

===============
*/
#ifdef SMOKINGUNS
void PM_AddEvent2(int newEvent, int param){
	BG_AddPredictableEventToPlayerstate( newEvent, param, pm->ps );
}
#endif

void PM_AddEvent( int newEvent ) {
	BG_AddPredictableEventToPlayerstate( newEvent, 0, pm->ps );
}

#ifdef SMOKINGUNS
void PM_FireWeapon(int anim, qboolean weapon2, qboolean altfire, qboolean delay){

	if(weapon2){
		BG_AddPredictableEventToPlayerstate( EV_FIRE_WEAPON2, anim, pm->ps );
	} else if(altfire){
		BG_AddPredictableEventToPlayerstate( EV_ALT_FIRE_WEAPON, anim, pm->ps );
	} else if (delay){
		BG_AddPredictableEventToPlayerstate( EV_FIRE_WEAPON_DELAY, anim, pm->ps );
	} else {
		BG_AddPredictableEventToPlayerstate( EV_FIRE_WEAPON, anim, pm->ps );
	}

	if(anim){
		if(weapon2)
			pm->ps->weapon2Anim = ( ( pm->ps->weapon2Anim & ANIM_TOGGLEBIT ) ^ ANIM_TOGGLEBIT )
				| anim;
		else
			pm->ps->weaponAnim = ( ( pm->ps->weaponAnim & ANIM_TOGGLEBIT ) ^ ANIM_TOGGLEBIT )
				| anim;
	}
}

//Spoon functions START
void PM_ChangeWeapon( int weapon ) {
	BG_AddPredictableEventToPlayerstate( EV_CHANGE_TO_WEAPON, weapon, pm->ps );
}

int PM_AnimLength( int anim, qboolean weapon2) {
	int length;
	int weapon = weapon2 ? pm->ps->weapon2 : pm->ps->weapon;

	length = (bg_weaponlist[weapon].animations[anim].numFrames-1)*
		bg_weaponlist[weapon].animations[anim].frameLerp+
		bg_weaponlist[weapon].animations[anim].initialLerp;

	if(bg_weaponlist[weapon].animations[anim].flipflop){
		length *= 2;
		length -= bg_weaponlist[weapon].animations[anim].initialLerp;
	}

	return length;
}
//Spoon functions END
#endif

/*
===============
PM_AddTouchEnt
===============
*/
void PM_AddTouchEnt( int entityNum ) {
	int		i;

	if ( entityNum == ENTITYNUM_WORLD ) {
		return;
	}
	if ( pm->numtouch == MAXTOUCH ) {
		return;
	}

	// see if it is already added
	for ( i = 0 ; i < pm->numtouch ; i++ ) {
		if ( pm->touchents[ i ] == entityNum ) {
			return;
		}
	}

	// add it
	pm->touchents[pm->numtouch] = entityNum;
	pm->numtouch++;
}

/*
===================
PM_StartWeaponAnim
===================
*/
#ifdef SMOKINGUNS
static void PM_StartWeaponAnim( int anim, qboolean weapon2, qboolean reload, qboolean sec) {
	int *weaponAnim;
	int weaponNum;

	// if weapon2 has to be animated
	if(weapon2){
		weaponAnim = &pm->ps->weapon2Anim ;
		weaponNum = pm->ps->weapon2 ;
	} else {
		weaponAnim = &pm->ps->weaponAnim ;
		weaponNum = pm->ps->weapon ;
	}

	if(reload){
		if(!sec)
			BG_AddPredictableEventToPlayerstate( EV_RELOAD, weaponNum, pm->ps );
		else
			BG_AddPredictableEventToPlayerstate( EV_RELOAD2, weaponNum, pm->ps );
	}

	*weaponAnim = ( ( *weaponAnim & ANIM_TOGGLEBIT ) ^ ANIM_TOGGLEBIT ) | anim;
}

static void PM_ContinueWeaponAnim( int anim , qboolean weapon2) {
	int *weaponAnim = weapon2 ? &pm->ps->weapon2Anim : &pm->ps->weaponAnim;

	if((*weaponAnim &~ ANIM_TOGGLEBIT) == anim)
		return;

	*weaponAnim = ( ( *weaponAnim & ANIM_TOGGLEBIT ) ^ ANIM_TOGGLEBIT ) | anim;
}
#endif

/*
===================
PM_StartTorsoAnim
===================
*/
static void PM_StartTorsoAnim( int anim ) {
	if ( pm->ps->pm_type >= PM_DEAD ) {
		return;
	}
	pm->ps->torsoAnim = ( ( pm->ps->torsoAnim & ANIM_TOGGLEBIT ) ^ ANIM_TOGGLEBIT )
		| anim;
}
/*
===================
PM_StartLegsAnim
===================
*/
static void PM_StartLegsAnim( int anim ) {
	if ( pm->ps->pm_type >= PM_DEAD ) {
		return;
	}
	if ( pm->ps->legsTimer > 0 ) {
		return;		// a high priority animation is running
	}
	pm->ps->legsAnim = ( ( pm->ps->legsAnim & ANIM_TOGGLEBIT ) ^ ANIM_TOGGLEBIT )
		| anim;
}

static void PM_ContinueLegsAnim( int anim ) {
	if ( ( pm->ps->legsAnim & ~ANIM_TOGGLEBIT ) == anim ) {
		return;
	}
	if ( pm->ps->legsTimer > 0 ) {
		return;		// a high priority animation is running
	}
	PM_StartLegsAnim( anim );
}

static void PM_ContinueTorsoAnim( int anim ) {
	if ( ( pm->ps->torsoAnim & ~ANIM_TOGGLEBIT ) == anim ) {
		return;
	}
	if ( pm->ps->torsoTimer > 0 ) {
		return;		// a high priority animation is running
	}
	PM_StartTorsoAnim( anim );
}

static void PM_ForceLegsAnim( int anim ) {

#ifdef SMOKINGUNS
	if(pm->ps->weaponstate == WEAPON_JUMPING)
		pm->ps->weaponstate = WEAPON_READY;
#endif

	pm->ps->legsTimer = 0;
	PM_StartLegsAnim( anim );
}


/*
==================
PM_ClipVelocity

Slide off of the impacting surface
==================
*/
void PM_ClipVelocity( vec3_t in, vec3_t normal, vec3_t out, float overbounce ) {
	float	backoff;
	float	change;
	int		i;

	backoff = DotProduct (in, normal);

	if ( backoff < 0 ) {
		backoff *= overbounce;
	} else {
		backoff /= overbounce;
	}

	for ( i=0 ; i<3 ; i++ ) {
		change = normal[i]*backoff;
		out[i] = in[i] - change;
	}
}


/*
==================
PM_Friction

Handles both ground friction and water friction
==================
*/
static void PM_Friction( void ) {
	vec3_t	vec;
	float	*vel;
	float	speed, newspeed, control;
	float	drop;

	vel = pm->ps->velocity;

	VectorCopy( vel, vec );
	if ( pml.walking ) {
		vec[2] = 0;	// ignore slope movement
	}

	speed = VectorLength(vec);
	if (speed < 1) {
		vel[0] = 0;
		vel[1] = 0;		// allow sinking underwater
		// FIXME: still have z friction underwater?
		return;
	}

	drop = 0;

	// apply ground friction
	if ( pm->waterlevel <= 1 ) {
		if ( pml.walking && !(pml.groundTrace.surfaceFlags & SURF_SLICK) ) {
			// if getting knocked back, no friction
			if ( ! (pm->ps->pm_flags & PMF_TIME_KNOCKBACK) ) {
				control = speed < pm_stopspeed ? pm_stopspeed : speed;
				drop += control*pm_friction*pml.frametime;
			}
		}
	}

	// apply water friction even if just wading
	if ( pm->waterlevel ) {
		drop += speed*pm_waterfriction*pm->waterlevel*pml.frametime;
	}

	// apply flying friction
#ifndef SMOKINGUNS
	if ( pm->ps->powerups[PW_FLIGHT]) {
		drop += speed*pm_flightfriction*pml.frametime;
	}
#else
	if ( pml.ladder ) // If they're on a ladder...
	{
		drop += speed*pm_ladderfriction*pml.frametime;  // Add ladder friction!
	}
#endif

	if ( pm->ps->pm_type == PM_SPECTATOR) {
		drop += speed*pm_spectatorfriction*pml.frametime;
	}

	// scale the velocity
	newspeed = speed - drop;
	if (newspeed < 0) {
		newspeed = 0;
	}
	newspeed /= speed;

	vel[0] = vel[0] * newspeed;
	vel[1] = vel[1] * newspeed;
	vel[2] = vel[2] * newspeed;
}


/*
==============
PM_Accelerate

Handles user intended acceleration
==============
*/
static void PM_Accelerate( vec3_t wishdir, float wishspeed, float accel ) {
#ifndef SMOKINGUNS
	// q2 style
	int			i;
	float		addspeed, accelspeed, currentspeed;

	currentspeed = DotProduct (pm->ps->velocity, wishdir);
	addspeed = wishspeed - currentspeed;
	if (addspeed <= 0) {
		return;
	}
	accelspeed = accel*pml.frametime*wishspeed;
	if (accelspeed > addspeed) {
		accelspeed = addspeed;
	}

	for (i=0 ; i<3 ; i++) {
		pm->ps->velocity[i] += accelspeed*wishdir[i];
	}
#else
	// proper way (avoids strafe jump maxspeed bug), but feels bad,
	// does it at all? ;) I changed it a bit
	vec3_t		wishVelocity;
	vec3_t		pushDir;
	float		pushLen;
	float		canPush;
//	float		currentspeed = DotProduct (pm->ps->velocity, wishdir);
#ifdef SMOKINGUNS
	float oldaccel = accel;
	float vel_z_save = pm->ps->velocity[2];

	//change acceleration
	if(pm->ps->stats[STAT_KNOCKTIME]){
		accel *= 0.25f;
	}
#endif

	VectorScale( wishdir, wishspeed, wishVelocity );
	VectorSubtract( wishVelocity, pm->ps->velocity, pushDir );
	pushLen = VectorNormalize( pushDir );

	canPush = accel*pml.frametime*wishspeed;
	if (canPush > pushLen) {
		canPush = pushLen;
	}

#ifdef SMOKINGUNS
	if(pushLen && canPush){

		accel *= sqrt(canPush/pushLen)*1.5f;

		if(oldaccel < accel) accel = oldaccel;
		//Com_Printf("%f %f\n", accel, sqrt(canPush/pushLen));
		canPush = accel*pml.frametime*wishspeed;

		if (canPush > pushLen) {
			canPush = pushLen;
		}
	}
#endif

	VectorMA( pm->ps->velocity, canPush, pushDir, pm->ps->velocity );

#ifdef SMOKINGUNS
	// use other acceleration when falling down/jumping
	if(pushLen && canPush){

		accel = oldaccel * sqrt(canPush/pushLen)*1.6f;
		if(oldaccel < accel) accel = oldaccel;

		canPush = accel*pml.frametime*wishspeed;

		if(canPush > pushLen) {
			canPush = pushLen;
		}

		pm->ps->velocity[2] = vel_z_save + canPush*pushDir[2];
	}
#endif
#endif
}



/*
============
PM_CmdScale

Returns the scale factor to apply to cmd movements
This allows the clients to use axial -127 to 127 values for all directions
without getting a sqrt(2) distortion in speed.
============
*/
static float PM_CmdScale( usercmd_t *cmd ) {
	int		max;
	float	total;
	float	scale;

	max = abs( cmd->forwardmove );
	if ( abs( cmd->rightmove ) > max ) {
		max = abs( cmd->rightmove );
	}
	if ( abs( cmd->upmove ) > max ) {
		max = abs( cmd->upmove );
	}
	if ( !max ) {
		return 0;
	}

	total = sqrt( cmd->forwardmove * cmd->forwardmove
		+ cmd->rightmove * cmd->rightmove + cmd->upmove * cmd->upmove );
	scale = (float)pm->ps->speed * max / ( 127.0 * total );

	return scale;
}


/*
================
PM_SetMovementDir

Determine the rotation of the legs relative
to the facing dir
================
*/
static void PM_SetMovementDir( void ) {
	if ( pm->cmd.forwardmove || pm->cmd.rightmove ) {
		if ( pm->cmd.rightmove == 0 && pm->cmd.forwardmove > 0 ) {
			pm->ps->movementDir = 0;
		} else if ( pm->cmd.rightmove < 0 && pm->cmd.forwardmove > 0 ) {
			pm->ps->movementDir = 1;
		} else if ( pm->cmd.rightmove < 0 && pm->cmd.forwardmove == 0 ) {
#ifdef SMOKINGUNS
			if(!( pm->cmd.buttons & BUTTON_WALKING ))
				pm->ps->movementDir = 0; // we now have a slide anim
			else
#endif
				pm->ps->movementDir = 2;
		} else if ( pm->cmd.rightmove < 0 && pm->cmd.forwardmove < 0 ) {
			pm->ps->movementDir = 3;
		} else if ( pm->cmd.rightmove == 0 && pm->cmd.forwardmove < 0 ) {
			pm->ps->movementDir = 4;
		} else if ( pm->cmd.rightmove > 0 && pm->cmd.forwardmove < 0 ) {
			pm->ps->movementDir = 5;
		} else if ( pm->cmd.rightmove > 0 && pm->cmd.forwardmove == 0 ) {
#ifdef SMOKINGUNS
			if(!( pm->cmd.buttons & BUTTON_WALKING ))
				pm->ps->movementDir = 0;// we now have a slide anim
			else
#endif
				pm->ps->movementDir = 6;
		} else if ( pm->cmd.rightmove > 0 && pm->cmd.forwardmove > 0 ) {
			pm->ps->movementDir = 7;
		}
	} else {
		// if they aren't actively going directly sideways,
		// change the animation to the diagonal so they
		// don't stop too crooked
		if ( pm->ps->movementDir == 2 ) {
			pm->ps->movementDir = 1;
		} else if ( pm->ps->movementDir == 6 ) {
			pm->ps->movementDir = 7;
		}
	}
}


/*
=============
PM_CheckJump
=============
*/
static qboolean PM_CheckJump( void ) {
	if ( pm->ps->pm_flags & PMF_RESPAWNED ) {
		return qfalse;		// don't allow jump until all buttons are up
	}

	if ( pm->cmd.upmove < 10 ) {
		// not holding jump
		return qfalse;
	}

	// must wait for jump to be released
	if ( pm->ps->pm_flags & PMF_JUMP_HELD ) {
		// clear upmove so cmdscale doesn't lower running speed
		pm->cmd.upmove = 0;
		return qfalse;
	}

	pml.groundPlane = qfalse;		// jumping away
	pml.walking = qfalse;
	pm->ps->pm_flags |= PMF_JUMP_HELD;

	pm->ps->groundEntityNum = ENTITYNUM_NONE;
	pm->ps->velocity[2] = JUMP_VELOCITY;
#ifndef SMOKINGUNS
	PM_AddEvent( EV_JUMP );
#else
	pm->ps->velocity[1] *= 0.75f; //reduce jump-verlocity
	pm->ps->velocity[0] *= 0.75f;
#endif

#ifndef SMOKINGUNS
	if ( pm->cmd.forwardmove >= 0 ) {
		PM_ForceLegsAnim( LEGS_JUMP );
		pm->ps->pm_flags &= ~PMF_BACKWARDS_JUMP;
	} else {
		PM_ForceLegsAnim( LEGS_JUMPB );
		pm->ps->pm_flags |= PMF_BACKWARDS_JUMP;
	}
#else
	PM_ForceLegsAnim( BOTH_JUMP );
	pm->ps->pm_flags &= ~PMF_BACKWARDS_JUMP;

	if((pm->ps->weaponstate == WEAPON_READY
		&& pm->ps->weapon2state == WEAPON_READY)
		|| pm->ps->weaponstate == WEAPON_JUMPING){
		pm->ps->weaponstate = WEAPON_JUMPING;
		PM_StartTorsoAnim( BOTH_JUMP );
	}
#endif

	return qtrue;
}

/*
=============
PM_CheckWaterJump
=============
*/
static qboolean	PM_CheckWaterJump( void ) {
	vec3_t	spot;
	int		cont;
	vec3_t	flatforward;

	if (pm->ps->pm_time) {
		return qfalse;
	}

	// check for water jump
	if ( pm->waterlevel != 2 ) {
		return qfalse;
	}

	flatforward[0] = pml.forward[0];
	flatforward[1] = pml.forward[1];
	flatforward[2] = 0;
	VectorNormalize (flatforward);

	VectorMA (pm->ps->origin, 30, flatforward, spot);
	spot[2] += 4;
	cont = pm->pointcontents (spot, pm->ps->clientNum );
	if ( !(cont & CONTENTS_SOLID) ) {
		return qfalse;
	}

	spot[2] += 16;
	cont = pm->pointcontents (spot, pm->ps->clientNum );
	if ( cont & (CONTENTS_SOLID|CONTENTS_PLAYERCLIP|CONTENTS_BODY) ) {
		return qfalse;
	}

	// jump out of water
	VectorScale (pml.forward, 200, pm->ps->velocity);
	pm->ps->velocity[2] = 350;

	pm->ps->pm_flags |= PMF_TIME_WATERJUMP;
	pm->ps->pm_time = 2000;

	return qtrue;
}

//============================================================================


/*
===================
PM_WaterJumpMove

Flying out of the water
===================
*/
static void PM_WaterJumpMove( void ) {
	// waterjump has no control, but falls

	PM_StepSlideMove( qtrue );

	pm->ps->velocity[2] -= pm->ps->gravity * pml.frametime;
	if (pm->ps->velocity[2] < 0) {
		// cancel as soon as we are falling down again
		pm->ps->pm_flags &= ~PMF_ALL_TIMES;
		pm->ps->pm_time = 0;
	}
}

/*
===================
PM_WaterMove

===================
*/
static void PM_WaterMove( void ) {
	int		i;
	vec3_t	wishvel;
	float	wishspeed;
	vec3_t	wishdir;
	float	scale;
	float	vel;

	if ( PM_CheckWaterJump() ) {
		PM_WaterJumpMove();
		return;
	}
#if 0
	// jump = head for surface
	if ( pm->cmd.upmove >= 10 ) {
		if (pm->ps->velocity[2] > -300) {
			if ( pm->watertype == CONTENTS_WATER ) {
				pm->ps->velocity[2] = 100;
			} else if (pm->watertype == CONTENTS_SLIME) {
				pm->ps->velocity[2] = 80;
			} else {
				pm->ps->velocity[2] = 50;
			}
		}
	}
#endif
	PM_Friction ();

	scale = PM_CmdScale( &pm->cmd );
	//
	// user intentions
	//
	if ( !scale ) {
		wishvel[0] = 0;
		wishvel[1] = 0;
		wishvel[2] = -60;		// sink towards bottom
	} else {
		for (i=0 ; i<3 ; i++)
			wishvel[i] = scale * pml.forward[i]*pm->cmd.forwardmove + scale * pml.right[i]*pm->cmd.rightmove;

		wishvel[2] += scale * pm->cmd.upmove;
	}

	VectorCopy (wishvel, wishdir);
	wishspeed = VectorNormalize(wishdir);

	if ( wishspeed > pm->ps->speed * pm_swimScale ) {
		wishspeed = pm->ps->speed * pm_swimScale;
	}

	PM_Accelerate (wishdir, wishspeed, pm_wateraccelerate);

	// make sure we can go up slopes easily under water
	if ( pml.groundPlane && DotProduct( pm->ps->velocity, pml.groundTrace.plane.normal ) < 0 ) {
		vel = VectorLength(pm->ps->velocity);
		// slide along the ground plane
		PM_ClipVelocity (pm->ps->velocity, pml.groundTrace.plane.normal,
			pm->ps->velocity, OVERCLIP );

		VectorNormalize(pm->ps->velocity);
		VectorScale(pm->ps->velocity, vel, pm->ps->velocity);
	}

	PM_SlideMove( qfalse );
}

#ifndef SMOKINGUNS
/*
===================
PM_InvulnerabilityMove

Only with the invulnerability powerup
===================
*/
static void PM_InvulnerabilityMove( void ) {
	pm->cmd.forwardmove = 0;
	pm->cmd.rightmove = 0;
	pm->cmd.upmove = 0;
	VectorClear(pm->ps->velocity);
}
#endif

/*
===================
PM_FlyMove

Only with the flight powerup
===================
*/
static void PM_FlyMove( void ) {
	int		i;
	vec3_t	wishvel;
	float	wishspeed;
	vec3_t	wishdir;
	float	scale;

	// normal slowdown
	PM_Friction ();

	scale = PM_CmdScale( &pm->cmd );
	//
	// user intentions
	//
	if ( !scale ) {
		wishvel[0] = 0;
		wishvel[1] = 0;
		wishvel[2] = 0;
	} else {
		for (i=0 ; i<3 ; i++) {
			wishvel[i] = scale * pml.forward[i]*pm->cmd.forwardmove + scale * pml.right[i]*pm->cmd.rightmove;
		}

		wishvel[2] += scale * pm->cmd.upmove;
	}

	VectorCopy (wishvel, wishdir);
	wishspeed = VectorNormalize(wishdir);

	PM_Accelerate (wishdir, wishspeed, pm_flyaccelerate);

	PM_StepSlideMove( qfalse );
}


/*
===================
PM_AirMove

===================
*/
static void PM_AirMove( void ) {
	int			i;
	vec3_t		wishvel;
	float		fmove, smove;
	vec3_t		wishdir;
	float		wishspeed;
	float		scale;
	usercmd_t	cmd;

	PM_Friction();

	fmove = pm->cmd.forwardmove;
	smove = pm->cmd.rightmove;

	cmd = pm->cmd;
	scale = PM_CmdScale( &cmd );

	// set the movementDir so clients can rotate the legs for strafing
	PM_SetMovementDir();

	// project moves down to flat plane
	pml.forward[2] = 0;
	pml.right[2] = 0;
	VectorNormalize (pml.forward);
	VectorNormalize (pml.right);

	for ( i = 0 ; i < 2 ; i++ ) {
		wishvel[i] = pml.forward[i]*fmove + pml.right[i]*smove;
	}
	wishvel[2] = 0;

	VectorCopy (wishvel, wishdir);
	wishspeed = VectorNormalize(wishdir);
	wishspeed *= scale;

	// not on ground, so little effect on velocity
	PM_Accelerate (wishdir, wishspeed, pm_airaccelerate);

	// we may have a ground plane that is very steep, even
	// though we don't have a groundentity
	// slide along the steep plane
	if ( pml.groundPlane ) {
		PM_ClipVelocity (pm->ps->velocity, pml.groundTrace.plane.normal,
			pm->ps->velocity, OVERCLIP );
	}

#if 0
	//ZOID:  If we are on the grapple, try stair-stepping
	//this allows a player to use the grapple to pull himself
	//over a ledge
	if (pm->ps->pm_flags & PMF_GRAPPLE_PULL)
		PM_StepSlideMove ( qtrue );
	else
		PM_SlideMove ( qtrue );
#endif

	PM_StepSlideMove ( qtrue );
}

/*
===================
PM_GrappleMove

===================
*/
#ifndef SMOKINGUNS
static void PM_GrappleMove( void ) {
	vec3_t vel, v;
	float vlen;

	VectorScale(pml.forward, -16, v);
#ifndef SMOKINGUNS
	VectorAdd(pm->ps->grapplePoint, v, v);
#endif
	VectorSubtract(v, pm->ps->origin, vel);
	vlen = VectorLength(vel);
	VectorNormalize( vel );

	if (vlen <= 100)
		VectorScale(vel, 10 * vlen, vel);
	else
		VectorScale(vel, 800, vel);

	VectorCopy(vel, pm->ps->velocity);

	pml.groundPlane = qfalse;
}
#endif

/*
===================
PM_WalkMove

===================
*/
static void PM_WalkMove( void ) {
	int			i;
	vec3_t		wishvel;
	float		fmove, smove;
	vec3_t		wishdir;
	float		wishspeed;
	float		scale;
	usercmd_t	cmd;
	float		accelerate;
	float		vel;

	if ( pm->waterlevel > 2 && DotProduct( pml.forward, pml.groundTrace.plane.normal ) > 0 ) {
		// begin swimming
		PM_WaterMove();
		return;
	}


	if ( PM_CheckJump () ) {
		// jumped away
		if ( pm->waterlevel > 1 ) {
			PM_WaterMove();
		} else {
			PM_AirMove();
		}
		return;
	}

	PM_Friction ();

	fmove = pm->cmd.forwardmove;
	smove = pm->cmd.rightmove;

	cmd = pm->cmd;
	scale = PM_CmdScale( &cmd );

	// set the movementDir so clients can rotate the legs for strafing
	PM_SetMovementDir();

	// project moves down to flat plane
	pml.forward[2] = 0;
	pml.right[2] = 0;

	// project the forward and right directions onto the ground plane
	PM_ClipVelocity (pml.forward, pml.groundTrace.plane.normal, pml.forward, OVERCLIP );
	PM_ClipVelocity (pml.right, pml.groundTrace.plane.normal, pml.right, OVERCLIP );
	//
	VectorNormalize (pml.forward);
	VectorNormalize (pml.right);

	for ( i = 0 ; i < 3 ; i++ ) {
		wishvel[i] = pml.forward[i]*fmove + pml.right[i]*smove;
	}
	// when going up or down slopes the wish velocity should Not be zero
//	wishvel[2] = 0;

	VectorCopy (wishvel, wishdir);
	wishspeed = VectorNormalize(wishdir);
	wishspeed *= scale;

	// clamp the speed lower if ducking
	if ( pm->ps->pm_flags & PMF_DUCKED ) {
		if ( wishspeed > pm->ps->speed * pm_duckScale ) {
			wishspeed = pm->ps->speed * pm_duckScale;
		}
	}

	// clamp the speed lower while reloading
#ifdef SMOKINGUNS
	if ( pm->ps->weaponstate == WEAPON_RELOADING ||
	                     pm->ps->weapon2state == WEAPON_RELOADING) {
		if ( wishspeed > pm->ps->speed * pm_reloadScale ) {
			wishspeed = pm->ps->speed * pm_reloadScale;
		}
	}
#endif

	// clamp the speed lower if wading or walking on the bottom
	if ( pm->waterlevel ) {
		float	waterScale;

		waterScale = pm->waterlevel / 3.0;
		waterScale = 1.0 - ( 1.0 - pm_swimScale ) * waterScale;
		if ( wishspeed > pm->ps->speed * waterScale ) {
			wishspeed = pm->ps->speed * waterScale;
		}
	}

	// when a player gets hit, they temporarily lose
	// full control, which allows them to be moved a bit
	if ( ( pml.groundTrace.surfaceFlags & SURF_SLICK ) || pm->ps->pm_flags & PMF_TIME_KNOCKBACK ) {
		accelerate = pm_airaccelerate;
	} else {
		accelerate = pm_accelerate;
	}

	PM_Accelerate (wishdir, wishspeed, accelerate);

	//Com_Printf("velocity = %1.1f %1.1f %1.1f\n", pm->ps->velocity[0], pm->ps->velocity[1], pm->ps->velocity[2]);
	//Com_Printf("velocity1 = %1.1f\n", VectorLength(pm->ps->velocity));

	if ( ( pml.groundTrace.surfaceFlags & SURF_SLICK ) || pm->ps->pm_flags & PMF_TIME_KNOCKBACK ) {
		pm->ps->velocity[2] -= pm->ps->gravity * pml.frametime;
	} else {
		// don't reset the z velocity for slopes
//		pm->ps->velocity[2] = 0;
	}

	vel = VectorLength(pm->ps->velocity);

	// slide along the ground plane
	PM_ClipVelocity (pm->ps->velocity, pml.groundTrace.plane.normal,
		pm->ps->velocity, OVERCLIP );

	// don't decrease velocity when going up or down a slope
	VectorNormalize(pm->ps->velocity);
	VectorScale(pm->ps->velocity, vel, pm->ps->velocity);

	// don't do anything if standing still
	if (!pm->ps->velocity[0] && !pm->ps->velocity[1]) {
		return;
	}

	PM_StepSlideMove( qfalse );

	//Com_Printf("velocity2 = %1.1f\n", VectorLength(pm->ps->velocity));

}


/*
==============
PM_DeadMove
==============
*/
static void PM_DeadMove( void ) {
	float	forward;

	if ( !pml.walking ) {
		return;
	}

	// extra friction

	forward = VectorLength (pm->ps->velocity);
	forward -= 20;
	if ( forward <= 0 ) {
		VectorClear (pm->ps->velocity);
	} else {
		VectorNormalize (pm->ps->velocity);
		VectorScale (pm->ps->velocity, forward, pm->ps->velocity);
	}
}


/*
===============
PM_NoclipMove
===============
*/
static void PM_NoclipMove( void ) {
	float	speed, drop, friction, control, newspeed;
	int			i;
	vec3_t		wishvel;
	float		fmove, smove;
	vec3_t		wishdir;
	float		wishspeed;
	float		scale;

	pm->ps->viewheight = DEFAULT_VIEWHEIGHT;

	// friction

	speed = VectorLength (pm->ps->velocity);
	if (speed < 1)
	{
		VectorCopy (vec3_origin, pm->ps->velocity);
	}
	else
	{
		drop = 0;

		friction = pm_friction*1.5;	// extra friction
		control = speed < pm_stopspeed ? pm_stopspeed : speed;
		drop += control*friction*pml.frametime;

		// scale the velocity
		newspeed = speed - drop;
		if (newspeed < 0)
			newspeed = 0;
		newspeed /= speed;

		VectorScale (pm->ps->velocity, newspeed, pm->ps->velocity);
	}

	// accelerate
	scale = PM_CmdScale( &pm->cmd );

	fmove = pm->cmd.forwardmove;
	smove = pm->cmd.rightmove;

	for (i=0 ; i<3 ; i++)
		wishvel[i] = pml.forward[i]*fmove + pml.right[i]*smove;
	wishvel[2] += pm->cmd.upmove;

	VectorCopy (wishvel, wishdir);
	wishspeed = VectorNormalize(wishdir);
	wishspeed *= scale;

	PM_Accelerate( wishdir, wishspeed, pm_accelerate );

	// move
	VectorMA (pm->ps->origin, pml.frametime, pm->ps->velocity, pm->ps->origin);
}

//============================================================================

/*
================
PM_FootstepForSurface

Returns an event number apropriate for the groundsurface
================
*/
static int PM_FootstepForSurface( void ) {
	if ( pml.groundTrace.surfaceFlags & SURF_NOSTEPS ) {
		return 0;
	}
#ifndef SMOKINGUNS
	if ( pml.groundTrace.surfaceFlags & SURF_METALSTEPS ) {
		return EV_FOOTSTEP_METAL;
	}
#else
	if ( pml.groundTrace.surfaceFlags & SURF_METAL ) {
		return EV_FOOTSTEP_METAL;
	}
	if( pml.groundTrace.surfaceFlags & SURF_SNOW) {
		return EV_FOOTSTEP_SNOW;
	}
	if(	pml.groundTrace.surfaceFlags & SURF_SAND) {
		return EV_FOOTSTEP_SAND;
	}
	if( pml.groundTrace.surfaceFlags & SURF_GRASS) {
		return EV_FOOTSTEP_GRASS;
	}
	if( pml.groundTrace.surfaceFlags & SURF_CLOTH) {
		return EV_FOOTSTEP_CLOTH;
	}
#endif
	return EV_FOOTSTEP;
}


/*
=================
PM_CrashLand

Check for hard landings that generate sound events
=================
*/
static void PM_CrashLand( void ) {
	float		delta;
	float		dist;
	float		vel, acc;
	float		t;
	float		a, b, c, den;
#ifdef SMOKINGUNS
	int		damage;
#endif

#ifndef SMOKINGUNS
	// decide which landing animation to use
	if ( pm->ps->pm_flags & PMF_BACKWARDS_JUMP ) {
		PM_ForceLegsAnim( LEGS_LANDB );
	} else {
		PM_ForceLegsAnim( LEGS_LAND );
	}

	pm->ps->legsTimer = TIMER_LAND;
#else
	if(pm->ps->weaponstate == WEAPON_JUMPING){
		pm->ps->weaponstate = WEAPON_READY;
		pm->ps->weapon2state = WEAPON_READY;
	}
#endif

	// calculate the exact velocity on landing
	dist = pm->ps->origin[2] - pml.previous_origin[2];
	vel = pml.previous_velocity[2];
	acc = -pm->ps->gravity;

	a = acc / 2;
	b = vel;
	c = -dist;

#ifdef SMOKINGUNS
	if( a == 0)
		return;
#endif
	den =  b * b - 4 * a * c;
	if ( den < 0 ) {
		return;
	}
	t = (-b - sqrt( den ) ) / ( 2 * a );

	delta = vel + t * acc;
#ifndef SMOKINGUNS
	delta = delta*delta * 0.0001;
#else
	delta = delta*delta * 0.00013;
#endif

#ifndef SMOKINGUNS
	// ducking while falling doubles damage
	if ( pm->ps->pm_flags & PMF_DUCKED ) {
		delta *= 2;
	}
#else
	// ducking while falling 0.8*damage
	if ( pm->ps->pm_flags & PMF_DUCKED ) {
		// The new falling damage needs to raise a bit this value
		//delta *= 0.8f;
		delta *= 0.9f;
	}

	if( delta > 12){
		// decide which landing animation to use
		/*if ( pm->ps->pm_flags & PMF_BACKWARDS_JUMP ) {
			PM_ForceLegsAnim( LEGS_LANDB );
		} else {*/
			PM_ForceLegsAnim( BOTH_LAND );

			if((pm->ps->weaponstate == WEAPON_READY
				&& pm->ps->weapon2state == WEAPON_READY)
				|| pm->ps->weaponstate == WEAPON_JUMPING){
				pm->ps->weaponstate = WEAPON_JUMPING;
				PM_StartTorsoAnim( BOTH_LAND );
			}

		//}

		pm->ps->legsTimer = TIMER_LAND;
	}
#endif

	// never take falling damage if completely underwater
	if ( pm->waterlevel == 3 ) {
		return;
	}

	// reduce falling damage if there is standing water
	if ( pm->waterlevel == 2 ) {
		delta *= 0.25;
	}
	if ( pm->waterlevel == 1 ) {
		delta *= 0.5;
	}

	if ( delta < 1 ) {
		return;
	}

	// create a local entity event to play the sound

	// SURF_NODAMAGE is used for bounce pads where you don't ever
	// want to take damage or play a crunch sound
	if ( !(pml.groundTrace.surfaceFlags & SURF_NODAMAGE) )  {
#ifndef SMOKINGUNS
		if ( delta > 60 ) {
			PM_AddEvent( EV_FALL_FAR );
		} else if ( delta > 40 ) {
			// this is a pain grunt, so don't play it if dead
			if ( pm->ps->stats[STAT_HEALTH] > 0 ) {
				PM_AddEvent( EV_FALL_MEDIUM );
			}
		} else if ( delta > 7 ) {
			PM_AddEvent( EV_FALL_SHORT );
		} else {
			PM_AddEvent( PM_FootstepForSurface() );
		}
#else
		// Joe Kari: using PM_AddEvent2 to produce linear damage calculated here
		damage = (int)( ( delta - 37 ) * 2 ) ;
		
		if ( delta > 60 ) {
			PM_AddEvent2( EV_FALL_VERY_FAR , damage );
			//Com_Printf("FALLING_DAMAGE: very far\n");
		} else if ( delta > 50 ) {
			//if ( pm->ps->stats[STAT_HEALTH] > 0 ) {
				PM_AddEvent2( EV_FALL_FAR , damage );
				//Printf("FALLING_DAMAGE: far\n");
			//}
		} else if ( delta > 39 ) {
			// this is a pain grunt, so don't play it if dead
			if ( pm->ps->stats[STAT_HEALTH] > 0 ) {
				PM_AddEvent2( EV_FALL_MEDIUM , damage );
				//Printf("FALLING_DAMAGE: medium\n");
			}
		} else if ( delta > 12 ) {
			if ( pm->ps->stats[STAT_HEALTH] > 0 ) {
				PM_AddEvent( EV_FALL_SHORT );
				//Printf("FALLING_DAMAGE: short\n");
			}
		} else if ( delta > 6 ) {
			if ( pm->ps->stats[STAT_HEALTH] > 0 ) {
				PM_AddEvent( EV_FALL_VERY_SHORT );
				//Printf("FALLING_DAMAGE: short\n");
			}
		} else if ( delta > 4 ) {
			PM_AddEvent( PM_FootstepForSurface() );
		} else {
			// Allow for moving down stairs silently
			if ( !(pm->cmd.buttons & BUTTON_WALKING)
				&& !(pm->ps->pm_flags & PMF_DUCKED) )
				PM_AddEvent( PM_FootstepForSurface() );
		}
#endif
	}

	// start footstep cycle over
	pm->ps->bobCycle = 0;
}

/*
=============
PM_CheckStuck
=============
*/
/*
void PM_CheckStuck(void) {
	trace_t trace;

	pm->trace (&trace, pm->ps->origin, pm->mins, pm->maxs, pm->ps->origin, pm->ps->clientNum, pm->tracemask);
	if (trace.allsolid) {
		//int shit = qtrue;
	}
}
*/

/*
=============
PM_CorrectAllSolid
=============
*/
static int PM_CorrectAllSolid( trace_t *trace ) {
	int			i, j, k;
	vec3_t		point;

	if ( pm->debugLevel ) {
		Com_Printf("%i:allsolid\n", c_pmove);
	}

	// jitter around
	for (i = -1; i <= 1; i++) {
		for (j = -1; j <= 1; j++) {
			for (k = -1; k <= 1; k++) {
				VectorCopy(pm->ps->origin, point);
				point[0] += (float) i;
				point[1] += (float) j;
				point[2] += (float) k;
				pm->trace (trace, point, pm->mins, pm->maxs, point, pm->ps->clientNum, pm->tracemask);
				if ( !trace->allsolid ) {
					point[0] = pm->ps->origin[0];
					point[1] = pm->ps->origin[1];
					point[2] = pm->ps->origin[2] - 0.25;

					pm->trace (trace, pm->ps->origin, pm->mins, pm->maxs, point, pm->ps->clientNum, pm->tracemask);
					pml.groundTrace = *trace;
					return qtrue;
				}
			}
		}
	}

	pm->ps->groundEntityNum = ENTITYNUM_NONE;
	pml.groundPlane = qfalse;
	pml.walking = qfalse;

	return qfalse;
}


/*
=============
PM_GroundTraceMissed

The ground trace didn't hit a surface, so we are in freefall
=============
*/
static void PM_GroundTraceMissed( void ) {
	trace_t		trace;
	vec3_t		point;

	if ( pm->ps->groundEntityNum != ENTITYNUM_NONE ) {
		// we just transitioned into freefall
		if ( pm->debugLevel ) {
			Com_Printf("%i:lift\n", c_pmove);
		}

		// if they aren't in a jumping animation and the ground is a ways away, force into it
		// if we didn't do the trace, the player would be backflipping down staircases
		VectorCopy( pm->ps->origin, point );
		point[2] -= 64;

		pm->trace (&trace, pm->ps->origin, pm->mins, pm->maxs, point, pm->ps->clientNum, pm->tracemask);
		if ( trace.fraction == 1.0 ) {
#ifndef SMOKINGUNS
			if ( pm->cmd.forwardmove >= 0 ) {
				PM_ForceLegsAnim( LEGS_JUMP );
				pm->ps->pm_flags &= ~PMF_BACKWARDS_JUMP;
			} else {
				PM_ForceLegsAnim( LEGS_JUMPB );
				pm->ps->pm_flags |= PMF_BACKWARDS_JUMP;
			}
#else
			PM_ForceLegsAnim( BOTH_JUMP );
			pm->ps->pm_flags &= ~PMF_BACKWARDS_JUMP;

			if((pm->ps->weaponstate == WEAPON_READY
				&& pm->ps->weapon2state == WEAPON_READY)
				|| pm->ps->weaponstate == WEAPON_JUMPING){
				pm->ps->weaponstate = WEAPON_JUMPING;
				PM_StartTorsoAnim( BOTH_JUMP );
			}
#endif
		}
	}

	pm->ps->groundEntityNum = ENTITYNUM_NONE;
	pml.groundPlane = qfalse;
	pml.walking = qfalse;
}

#ifdef SMOKINGUNS
void	trap_Cvar_VariableStringBuffer( const char *var_name, char *buffer, int bufsize );

void PM_FindGround( void ){
	vec3_t		point, end;
	trace_t		trace;
	vec3_t		mins, maxs;

	VectorSet(mins, -2, -2, MINS_Z);
	VectorSet(maxs, 2, 2, 2);

	point[0] = pm->ps->origin[0];
	point[1] = pm->ps->origin[1];
	point[2] = pm->ps->grapplePoint[2] + 20;
	VectorCopy(point, end);
	end[2] -= 200;

	pm->trace (&trace, point, mins, maxs, end, pm->ps->clientNum, pm->tracemask);

	// make no changes
	if(trace.fraction == 1.0 || trace.startsolid)
		return;

	pm->ps->origin[2] = trace.endpos[2];
}
#endif

/*
=============
PM_GroundTrace
=============
*/
static void PM_GroundTrace( void ) {
	vec3_t		point;
	trace_t		trace;

	point[0] = pm->ps->origin[0];
	point[1] = pm->ps->origin[1];
	point[2] = pm->ps->origin[2] - 0.25f;

	pm->trace (&trace, pm->ps->origin, pm->mins, pm->maxs, point, pm->ps->clientNum, pm->tracemask);
	pml.groundTrace = trace;

	// do something corrective if the trace starts in a solid...
	if ( trace.allsolid ) {
		if ( !PM_CorrectAllSolid(&trace) )
			return;
	}

	// if the trace didn't hit anything, we are in free fall
	if ( trace.fraction == 1.0 ) {
		PM_GroundTraceMissed();
		pml.groundPlane = qfalse;
		pml.walking = qfalse;
		return;
	}

	// check if getting thrown off the ground
	if ( pm->ps->velocity[2] > 0 && DotProduct( pm->ps->velocity, trace.plane.normal ) > 10 ) {
		if ( pm->debugLevel ) {
			Com_Printf("%i:kickoff\n", c_pmove);
		}
		// go into jump animation
#ifndef SMOKINGUNS
		if ( pm->cmd.forwardmove >= 0 ) {
			PM_ForceLegsAnim( LEGS_JUMP );
			pm->ps->pm_flags &= ~PMF_BACKWARDS_JUMP;
		} else {
			PM_ForceLegsAnim( LEGS_JUMPB );
			pm->ps->pm_flags |= PMF_BACKWARDS_JUMP;
		}
#else
		PM_ForceLegsAnim( BOTH_JUMP );
		pm->ps->pm_flags &= ~PMF_BACKWARDS_JUMP;

		if((pm->ps->weaponstate == WEAPON_READY
			&& pm->ps->weapon2state == WEAPON_READY)
			|| pm->ps->weaponstate == WEAPON_JUMPING){
			pm->ps->weaponstate = WEAPON_JUMPING;
			PM_StartTorsoAnim( BOTH_JUMP );
		}
#endif

		pm->ps->groundEntityNum = ENTITYNUM_NONE;
		pml.groundPlane = qfalse;
		pml.walking = qfalse;
		return;
	}

	// slopes that are too steep will not be considered onground
	if ( trace.plane.normal[2] < MIN_WALK_NORMAL ) {
		if ( pm->debugLevel ) {
			Com_Printf("%i:steep\n", c_pmove);
		}
		// FIXME: if they can't slide down the slope, let them
		// walk (sharp crevices)
		pm->ps->groundEntityNum = ENTITYNUM_NONE;
		pml.groundPlane = qtrue;
		pml.walking = qfalse;
		return;
	}

	pml.groundPlane = qtrue;
	pml.walking = qtrue;

	// hitting solid ground will end a waterjump
	if (pm->ps->pm_flags & PMF_TIME_WATERJUMP)
	{
		pm->ps->pm_flags &= ~(PMF_TIME_WATERJUMP | PMF_TIME_LAND);
		pm->ps->pm_time = 0;
	}

	if ( pm->ps->groundEntityNum == ENTITYNUM_NONE ) {
		// just hit the ground
		if ( pm->debugLevel ) {
			Com_Printf("%i:Land\n", c_pmove);
		}

		PM_CrashLand();

		// don't do landing time if we were just going down a slope
		if ( pml.previous_velocity[2] < -200 ) {
			// don't allow another jump for a little while
			pm->ps->pm_flags |= PMF_TIME_LAND;
			pm->ps->pm_time = 250;
		}
	}

	pm->ps->groundEntityNum = trace.entityNum;

	// don't reset the z velocity for slopes
//	pm->ps->velocity[2] = 0;

	PM_AddTouchEnt( trace.entityNum );
}


/*
=============
PM_SetWaterLevel	FIXME: avoid this twice?  certainly if not moving
=============
*/
static void PM_SetWaterLevel( void ) {
	vec3_t		point;
	int			cont;
	int			sample1;
	int			sample2;

	//
	// get waterlevel, accounting for ducking
	//
	pm->waterlevel = 0;
	pm->watertype = 0;

	point[0] = pm->ps->origin[0];
	point[1] = pm->ps->origin[1];
	point[2] = pm->ps->origin[2] + MINS_Z + 1;
	cont = pm->pointcontents( point, pm->ps->clientNum );

	if ( cont & MASK_WATER ) {
		sample2 = pm->ps->viewheight - MINS_Z;
		sample1 = sample2 / 2;

		pm->watertype = cont;
		pm->waterlevel = 1;
		point[2] = pm->ps->origin[2] + MINS_Z + sample1;
		cont = pm->pointcontents (point, pm->ps->clientNum );
		if ( cont & MASK_WATER ) {
			pm->waterlevel = 2;
			point[2] = pm->ps->origin[2] + MINS_Z + sample2;
			cont = pm->pointcontents (point, pm->ps->clientNum );
			if ( cont & MASK_WATER ){
				pm->waterlevel = 3;
#ifdef SMOKINGUNS
				PM_AddEvent(EV_UNDERWATER);
#endif
			}
		}
	}

}

/*
==============
PM_CheckDuck

Sets mins, maxs, and pm->ps->viewheight
==============
*/
static void PM_CheckDuck (void)
{
	trace_t	trace;

#ifndef SMOKINGUNS
	if ( pm->ps->powerups[PW_INVULNERABILITY] ) {
		if ( pm->ps->pm_flags & PMF_INVULEXPAND ) {
			// invulnerability sphere has a 42 units radius
			VectorSet( pm->mins, -42, -42, -42 );
			VectorSet( pm->maxs, 42, 42, 42 );
		}
		else {
			VectorSet( pm->mins, -15, -15, MINS_Z );
			VectorSet( pm->maxs, 15, 15, 16 );
		}
		pm->ps->pm_flags |= PMF_DUCKED;
		pm->ps->viewheight = CROUCH_VIEWHEIGHT;
		return;
	}
#endif
	pm->ps->pm_flags &= ~PMF_INVULEXPAND;

#ifndef SMOKINGUNS
	pm->mins[0] = -15;
	pm->mins[1] = -15;

	pm->maxs[0] = 15;
	pm->maxs[1] = 15;
#else
	VectorCopy(playerMins, pm->mins);
	VectorCopy(playerMaxs, pm->maxs);
#endif

	pm->mins[2] = MINS_Z;

	if (pm->ps->pm_type == PM_DEAD)
	{
		pm->maxs[2] = -8;
		pm->ps->viewheight = DEAD_VIEWHEIGHT;
		return;
	}

	if (pm->cmd.upmove < 0)
	{	// duck
		pm->ps->pm_flags |= PMF_DUCKED;
	}
	else
	{	// stand up if possible
		if (pm->ps->pm_flags & PMF_DUCKED)
		{
			// try to stand up
#ifndef SMOKINGUNS
			pm->maxs[2] = 32;
#else
			pm->maxs[2] = MAXS_Z;
#endif
			pm->trace (&trace, pm->ps->origin, pm->mins, pm->maxs, pm->ps->origin, pm->ps->clientNum, pm->tracemask );
			if (!trace.allsolid)
				pm->ps->pm_flags &= ~PMF_DUCKED;
		}
	}

	if (pm->ps->pm_flags & PMF_DUCKED)
	{
#ifndef SMOKINGUNS
		pm->maxs[2] = 16;
#else
		pm->maxs[2] = MAXS_Z - 16;
#endif
		pm->ps->viewheight = CROUCH_VIEWHEIGHT;
	}
	else
	{
#ifndef SMOKINGUNS
		pm->maxs[2] = 32;
#else
		pm->maxs[2] = MAXS_Z;
#endif
		pm->ps->viewheight = DEFAULT_VIEWHEIGHT;
	}
}



//===================================================================


/*
===============
PM_Footsteps
===============
*/
static void PM_Footsteps( void ) {
	float		bobmove;
	int			old;
	qboolean	footstep;

	//
	// calculate speed and cycle to be used for
	// all cyclic walking effects
	//
	pm->xyspeed = sqrt( pm->ps->velocity[0] * pm->ps->velocity[0]
		+  pm->ps->velocity[1] * pm->ps->velocity[1] );

#ifndef SMOKINGUNS
	if ( pm->ps->groundEntityNum == ENTITYNUM_NONE ) {

		if ( pm->ps->powerups[PW_INVULNERABILITY] ) {
			PM_ContinueLegsAnim( LEGS_IDLECR );
		}
#else
	if ( pm->ps->groundEntityNum == ENTITYNUM_NONE  && !pml.ladder) {
#endif
		// airborne leaves position in cycle intact, but doesn't advance
		if ( pm->waterlevel > 1 ) {
			PM_ContinueLegsAnim( LEGS_SWIM );
		}
		return;
	}

	// if not trying to move
#ifndef SMOKINGUNS
	if ( !pm->cmd.forwardmove && !pm->cmd.rightmove ) {
#else
	if( (pml.ladder && !pm->cmd.forwardmove && !pm->cmd.upmove && !pm->cmd.rightmove) ||//if on ladder
		(!pm->cmd.forwardmove && !pm->cmd.rightmove && !pml.ladder)) { //if not on ladder
#endif
		if (  pm->xyspeed < 5 ) {
			pm->ps->bobCycle = 0;	// start at beginning of cycle again
			if ( pm->ps->pm_flags & PMF_DUCKED ) {
#ifndef SMOKINGUNS
				PM_ContinueLegsAnim( LEGS_IDLECR );
#else
				PM_ContinueLegsAnim( LEGS_CROUCHED_IDLE );
#endif
			} else {
				PM_ContinueLegsAnim( LEGS_IDLE );
			}
		}
#ifdef SMOKINGUNS
		if(pml.ladder){
			PM_StartTorsoAnim(BOTH_LADDER_STAND);
			PM_StartLegsAnim(BOTH_LADDER_STAND);
		}
#endif
		return;
	}


	footstep = qfalse;

#ifndef SMOKINGUNS
	if ( pm->ps->pm_flags & PMF_DUCKED ) {
		bobmove = 0.5;	// ducked characters bob much faster
		if ( pm->ps->pm_flags & PMF_BACKWARDS_RUN ) {
			PM_ContinueLegsAnim( LEGS_BACKCR );
		}
		else {
			PM_ContinueLegsAnim( LEGS_WALKCR );
		}
#else
	if(pml.ladder) {
		PM_ContinueTorsoAnim( BOTH_LADDER );
		PM_ContinueLegsAnim( BOTH_LADDER );
	} else if ( pm->ps->pm_flags & PMF_DUCKED ) {
		bobmove = 0.5;	// ducked characters bob much faster
		if ( pm->ps->pm_flags & PMF_BACKWARDS_RUN ) {
			PM_ContinueLegsAnim( LEGS_CROUCH_BACK );
		}
		else {
			PM_ContinueLegsAnim( LEGS_CROUCH_WALK );
		}
#endif
		// ducked characters never play footsteps
	/*
	} else 	if ( pm->ps->pm_flags & PMF_BACKWARDS_RUN ) {
		if ( !( pm->cmd.buttons & BUTTON_WALKING ) ) {
			bobmove = 0.4;	// faster speeds bob faster
			footstep = qtrue;
		} else {
			bobmove = 0.3;
		}
		PM_ContinueLegsAnim( LEGS_BACK );
	*/
	} else {
		if ( !( pm->cmd.buttons & BUTTON_WALKING ) ) {
			bobmove = 0.4f;	// faster speeds bob faster
			if ( pm->ps->pm_flags & PMF_BACKWARDS_RUN ) {
#ifndef SMOKINGUNS
				PM_ContinueLegsAnim( LEGS_BACK );
#else
				PM_ContinueLegsAnim( LEGS_RUN_BACK );
#endif
			}
			else {
#ifdef SMOKINGUNS
				if(!pm->cmd.forwardmove && pm->cmd.rightmove){
					if(pm->cmd.rightmove > 0)
						PM_ContinueLegsAnim( LEGS_SLIDE_RIGHT );
					else
						PM_ContinueLegsAnim( LEGS_SLIDE_LEFT );
				} else if(pm->cmd.rightmove)
					PM_ContinueLegsAnim( LEGS_RUN_SLIDE );
				else
#endif
					PM_ContinueLegsAnim( LEGS_RUN );
			}
			footstep = qtrue;
		} else {
			bobmove = 0.3f;	// walking bobs slow
			if ( pm->ps->pm_flags & PMF_BACKWARDS_RUN ) {
#ifndef SMOKINGUNS
				PM_ContinueLegsAnim( LEGS_BACKWALK );
#else
				PM_ContinueLegsAnim( LEGS_WALK_BACK );
#endif
			}
			else {
				PM_ContinueLegsAnim( LEGS_WALK );
			}
		}
	}

	// check for footstep / splash sounds
	old = pm->ps->bobCycle;
	pm->ps->bobCycle = (int)( old + bobmove * pml.msec ) & 255;

	// if we just crossed a cycle boundary, play an apropriate footstep event
	if ( ( ( old + 64 ) ^ ( pm->ps->bobCycle + 64 ) ) & 128 ) {
		if ( pm->waterlevel == 0 ) {
			// on ground will only play sounds if running
			if ( footstep && !pm->noFootsteps ) {
				PM_AddEvent( PM_FootstepForSurface() );
			}
		} else if ( pm->waterlevel == 1 ) {
			// splashing
			PM_AddEvent( EV_FOOTSPLASH );
		} else if ( pm->waterlevel == 2 ) {
			// wading / swimming at surface
			PM_AddEvent( EV_SWIM );
		} else if ( pm->waterlevel == 3 ) {
			// no sound when completely underwater

		}
	}
}

/*
==============
PM_WaterEvents

Generate sound events for entering and leaving water
==============
*/
static void PM_WaterEvents( void ) {		// FIXME?
	//
	// if just entered a water volume, play a sound
	//
	if (!pml.previous_waterlevel && pm->waterlevel) {
		PM_AddEvent( EV_WATER_TOUCH );
	}

	//
	// if just completely exited a water volume, play a sound
	//
	if (pml.previous_waterlevel && !pm->waterlevel) {
		PM_AddEvent( EV_WATER_LEAVE );
	}

	//
	// check for head just going under water
	//
	if (pml.previous_waterlevel != 3 && pm->waterlevel == 3) {
		PM_AddEvent( EV_WATER_UNDER );
	}

	//
	// check for head just coming out of water
	//
	if (pml.previous_waterlevel == 3 && pm->waterlevel != 3) {
		PM_AddEvent( EV_WATER_CLEAR );
	}
}


/*
===============
PM_BeginWeaponChange
===============
*/
static void PM_BeginWeaponChange( int weapon ) {
#ifndef SMOKINGUNS
	if ( weapon <= WP_NONE || weapon >= WP_NUM_WEAPONS ) {
		return;
	}

	if ( !( pm->ps->stats[STAT_WEAPONS] & ( 1 << weapon ) ) ) {
		return;
	}
	
	if ( pm->ps->weaponstate == WEAPON_DROPPING ) {
		return;
	}
#else
	int count_pistols = 0;

	if ( (weapon < WP_NONE || weapon >= WP_NUM_WEAPONS) && weapon != WP_AKIMBO ) {
		return;
	}

	if ( weapon != WP_AKIMBO && !( pm->ps->stats[STAT_WEAPONS] & ( 1 << weapon ) )) {
		return;
	}

	if ( pm->ps->weaponstate == WEAPON_DROPPING || (pm->ps->weapon2state == WEAPON_DROPPING
		&& pm->ps->weapon2)) {
		return;
	}

	if (weapon == WP_AKIMBO) {
		if (pm->ps->stats[STAT_FLAGS] & SF_SEC_PISTOL)
			count_pistols = 1;

		count_pistols += pm->ps->stats[STAT_WEAPONS] & ( 1 << WP_REM58 ) ? 1 : 0;
		count_pistols += pm->ps->stats[STAT_WEAPONS] & ( 1 << WP_SCHOFIELD ) ? 1 : 0;
		count_pistols += pm->ps->stats[STAT_WEAPONS] & ( 1 << WP_PEACEMAKER ) ? 1 : 0;

		if (count_pistols < 2)
			return;
	}

	// don't change weapon if dynamite is burning
	if(pm->ps->weapon == WP_DYNAMITE && pm->ps->stats[STAT_WP_MODE] < 0){
		pm->cmd.weapon = pm->ps->weapon = WP_DYNAMITE;

		PM_ChangeWeapon(WP_DYNAMITE);
		return;
	}
	// same with molotov
	// don't change weapon if molotov is burning
	if(pm->ps->weapon == WP_MOLOTOV && pm->ps->stats[STAT_WP_MODE] < 0){
		pm->cmd.weapon = pm->ps->weapon = WP_MOLOTOV;

		PM_ChangeWeapon(WP_MOLOTOV);
		return;
	}

	// abort reloading if possible
	pm->ps->eFlags &= ~EF_RELOAD;
	pm->ps->eFlags &= ~EF_RELOAD2;
	pm->ps->stats[STAT_FLAGS] &= ~SF_RELOAD2;
	pm->ps->stats[STAT_FLAGS] &= ~SF_RELOAD;

	// change to the akimbos
	if(weapon == WP_AKIMBO){

		// if player already has one pistol in his hand
		if(bg_weaponlist[pm->ps->weapon].wp_sort == WPS_PISTOL){

			// switch weapon1 -> weapon2
			pm->ps->weapon2 = pm->ps->weapon;
			pm->ps->weapon2Anim = pm->ps->weaponAnim;
			pm->ps->weapon2state = WEAPON_READY;//pm->ps->weaponstate;
			pm->ps->weapon2Time = 0;//pm->ps->weaponTime;

			pm->ps->weaponTime = 0;

		} else {
			pm->ps->weapon2 = 0;
		}
	}
	// in every case: this flag must be set because of FinishWeaponChange()
	pm->ps->weaponstate = WEAPON_DROPPING;

	// normal weapon change, old weapon change out if to
	// a pistol isn't added another pistol(akimbos)
	if((!pm->ps->stats[STAT_OLDWEAPON] &&
		(!pm->ps->weapon2 || weapon != WP_AKIMBO) && pm->ps->weapon != WP_NONE)
		|| weapon == WP_GATLING){

		pm->ps->weaponTime = PM_AnimLength(WP_ANIM_DROP, qfalse);

		// don't show drop animation if there's no more dynamite/molotov
		if((pm->ps->weapon == WP_DYNAMITE || pm->ps->weapon == WP_MOLOTOV) &&
			!pm->ps->ammo[pm->ps->weapon]){
		} else {
			PM_StartWeaponAnim(WP_ANIM_DROP, qfalse, qfalse, qfalse);
		}

		if(bg_weaponlist[pm->ps->weapon].wp_sort == WPS_PISTOL){
			PM_StartTorsoAnim( TORSO_PISTOL_DROP);
		} else {
			PM_StartTorsoAnim( TORSO_RIFLE_DROP);
		}

	} else if(pm->ps->weapon == WP_GATLING){
		// for Gatling Gun
		pm->ps->stats[STAT_OLDWEAPON] = 0;
	}

	// second weapon change
	if(weapon == WP_AKIMBO || pm->ps->weapon2){

		// change in, don't drop weapon because before there was no weapon2
		if(!pm->ps->stats[STAT_OLDWEAPON]){
			// set akimbo bit and do nothing
			pm->ps->stats[STAT_OLDWEAPON] = 1;

		// drop the two pistols
		} else {
			PM_StartTorsoAnim( TORSO_PISTOL_DROP);

			// weapon1
			pm->ps->weaponstate = WEAPON_DROPPING;

			PM_StartWeaponAnim(WP_ANIM_DROP, qfalse, qfalse, qfalse);
			pm->ps->weaponTime = PM_AnimLength(WP_ANIM_DROP, qfalse);

			// now also drop weapon2
			pm->ps->weapon2state = WEAPON_DROPPING;

			// weapon2
			PM_StartWeaponAnim(WP_ANIM_DROP, qtrue, qfalse, qfalse);
			pm->ps->weapon2Time = PM_AnimLength(WP_ANIM_DROP, qtrue);

			// unset akimbo bit
			pm->ps->stats[STAT_OLDWEAPON] = 0;
		}
	}
#endif
	// Play change sound
	PM_AddEvent( EV_CHANGE_WEAPON );
#ifndef SMOKINGUNS
	pm->ps->weaponstate = WEAPON_DROPPING;
	pm->ps->weaponTime += 200;
	PM_StartTorsoAnim( TORSO_DROP );
#endif
}


/*
===============
PM_FinishWeaponChange
===============
*/
static void PM_FinishWeaponChange( void ) {
	int		weapon;

#ifndef SMOKINGUNS
	weapon = pm->cmd.weapon;
	if ( weapon < WP_NONE || weapon >= WP_NUM_WEAPONS ) {
		weapon = WP_NONE;
	}

	if ( !( pm->ps->stats[STAT_WEAPONS] & ( 1 << weapon ) ) ) {
		weapon = WP_NONE;
	}

	pm->ps->weapon = weapon;
	pm->ps->weaponstate = WEAPON_RAISING;
	pm->ps->weaponTime += 250;
	PM_StartTorsoAnim( TORSO_RAISE );
#else
	int		weapon1, weapon2;
	qboolean	remington_cocking_sound = qtrue;

	weapon = pm->cmd.weapon;
	if ( (weapon < WP_NONE || weapon >= WP_NUM_WEAPONS) && weapon != WP_AKIMBO ) {
		weapon = WP_NONE;
	}

	if(weapon == WP_AKIMBO){
		qboolean sec_pistol = (pm->ps->stats[STAT_FLAGS] & SF_SEC_PISTOL);

		//pm->ps->weapon2 = 0;

		// if weapon1 is added to old weapon1 to get akimbos
		// which weapons are supposed to be changed?
		if(pm->ps->weapon2 && bg_weaponlist[pm->ps->weapon].wp_sort == WPS_PISTOL){

			if(!sec_pistol){
				weapon1 = BG_SearchTypeWeapon(WPS_PISTOL, pm->ps->stats[STAT_WEAPONS], pm->ps->weapon2);
			} else {
				weapon1 = BG_SearchTypeWeapon(WPS_PISTOL, pm->ps->stats[STAT_WEAPONS], 0);
			}
			weapon2 = 0; // don't change weapon2

		} else {
			weapon1 = BG_SearchTypeWeapon(WPS_PISTOL, pm->ps->stats[STAT_WEAPONS], 0);

			if(!sec_pistol){
				weapon2 = BG_SearchTypeWeapon(WPS_PISTOL, pm->ps->stats[STAT_WEAPONS], weapon1);
			} else {
				weapon2 = BG_SearchTypeWeapon(WPS_PISTOL, pm->ps->stats[STAT_WEAPONS], 0);
			}
		}
	} else {
		weapon1 = weapon;
		weapon2 = 0;
	}

	// check if player has the weapon, he is changing to
	if ( !( pm->ps->stats[STAT_WEAPONS] & ( 1 << weapon1 ) ) ) {
		weapon1 = WP_NONE;
	}
	if(weapon2 && !( pm->ps->stats[STAT_WEAPONS] & ( 1 << weapon2 ) )){
		weapon2 = WP_NONE;
	}

	if(!pm->ps->stats[STAT_OLDWEAPON]){
		pm->ps->weapon2 = 0;
		weapon2 = WP_NONE;
	}

	pm->ps->weapon = weapon1;
	pm->ps->weaponstate = WEAPON_RAISING;
	pm->ps->weaponTime = 0;

	if(bg_weaponlist[weapon1].wp_sort == WPS_GUN)
		PM_StartTorsoAnim( TORSO_RIFLE_RAISE );
	else PM_StartTorsoAnim( TORSO_PISTOL_RAISE);

	// change in weapon2 if necessary
	if(weapon2 && !pm->ps->weapon2){
		pm->ps->weapon2 = weapon2;
		pm->ps->weapon2state = WEAPON_RAISING;
		pm->ps->weapon2Time = PM_AnimLength(WP_ANIM_CHANGE, qtrue);
		PM_StartWeaponAnim(WP_ANIM_CHANGE, qtrue, qfalse, qfalse);
	}

	// check if we change to the gatling(already built up)
	if(!pm->ps->stats[STAT_GATLING_MODE] || pm->ps->weapon != WP_GATLING){
		PM_StartWeaponAnim(WP_ANIM_CHANGE, qfalse, qfalse, qfalse);
		pm->ps->weaponTime = PM_AnimLength(WP_ANIM_CHANGE, qfalse);
	} else {
		PM_StartWeaponAnim(WP_ANIM_IDLE, qfalse, qfalse, qfalse);
		PM_StartWeaponAnim(WP_ANIM_IDLE, qtrue, qfalse, qfalse);
		// cancel weapon choose mode
		pm->ps->stats[STAT_FLAGS] &= ~SF_WP_CHOOSE;
	}

	if(!pm->ps->ammo[pm->ps->weapon] && pm->ps->weapon == WP_REMINGTON_GAUGE){
		pm->ps->weaponTime -= 28*bg_weaponlist[WP_REMINGTON_GAUGE].animations[WP_ANIM_CHANGE].frameLerp;
		remington_cocking_sound = qfalse;
	}

	pm->ps->stats[STAT_WP_MODE] = 0;

	pm->ps->stats[IDLE_TIMER] = 0;

	if (remington_cocking_sound) {
		PM_AddEvent2(EV_CHANGETO_SOUND, pm->ps->weapon);
	}
#endif
}


/*
==============
PM_TorsoAnimation

==============
*/
static void PM_TorsoAnimation( void ) {
#ifndef SMOKINGUNS
	if ( pm->ps->weaponstate == WEAPON_READY ) {
		if ( pm->ps->weapon == WP_GAUNTLET ) {
			PM_ContinueTorsoAnim( TORSO_STAND2 );
		} else {
			PM_ContinueTorsoAnim( TORSO_STAND );
		}
		return;
	}
#else
	// start hold animation of torso when leg animations of jumping have ended
	if((pm->ps->legsAnim &~ ANIM_TOGGLEBIT) != BOTH_JUMP &&
		(pm->ps->legsAnim &~ ANIM_TOGGLEBIT) != BOTH_LAND &&
		pm->ps->weaponstate == WEAPON_JUMPING &&
		pm->ps->legsTimer <= 0){
		pm->ps->weaponstate = WEAPON_READY;
		pm->ps->weapon2state = WEAPON_READY;
	}

	if ( pm->ps->weaponstate == WEAPON_READY &&
		pm->ps->weapon2state == WEAPON_READY) {

		//Com_Printf("anim\n");

		if(pm->ps->weapon2){
			PM_ContinueTorsoAnim( TORSO_PISTOLS_STAND);
			return;
		}
		switch(pm->ps->weapon){
		case WP_KNIFE:
		case WP_DYNAMITE:
		case WP_MOLOTOV:
			PM_ContinueTorsoAnim( TORSO_KNIFE_STAND);
			break;
		case WP_PEACEMAKER:
		case WP_REM58:
		case WP_SCHOFIELD:
			PM_ContinueTorsoAnim( TORSO_PISTOL_STAND);
			break;
		case WP_GATLING:
			if(pm->ps->stats[STAT_GATLING_MODE])
				PM_ContinueTorsoAnim( TORSO_GATLING_STAND);
			else
				PM_ContinueTorsoAnim( TORSO_RIFLE_STAND);
			break;
		case WP_NONE:
			PM_ContinueTorsoAnim( TORSO_HOLSTERED);
			break;
		default:
			PM_ContinueTorsoAnim( TORSO_RIFLE_STAND);
			break;
		}
		return;
	}
#endif
}


/*
==============
PM_ReloadStart
Starts the reload
==============
*/
#ifdef SMOKINGUNS
static qboolean PM_ReloadStart( qboolean weapon2, int *weaponTime, int *weaponstate ){
	int weapon = weapon2 ? pm->ps->weapon2 : pm->ps->weapon;
	int reload = weapon2 ? EF_RELOAD2 : EF_RELOAD;
	qboolean reloading = (pm->ps->eFlags & reload) ? qtrue : qfalse ;
	int clip = bg_weaponlist[weapon].clip;
	int ammo = weapon;

	int oldweaponstate = *weaponstate;
	// if the anim has to be started later(akimbos)
	int oldanim = weapon2 ? pm->ps->weapon2Anim : pm->ps->weaponAnim;
	qboolean reloadsound = !pm->ps->weapon2; // start reloadsound later?
	qboolean all_at_once;

	if(!weapon2 && pm->ps->stats[STAT_FLAGS] & SF_SEC_PISTOL &&
		bg_weaponlist[pm->ps->weapon].wp_sort == WPS_PISTOL && pm->ps->weapon2)
		ammo = WP_AKIMBO;

	if(weapon == WP_GATLING && !pm->ps->stats[STAT_GATLING_MODE])
		return qfalse;

	if (pm->ps->weaponstate == WEAPON_DROPPING || pm->ps->weaponstate == WEAPON_RAISING) {
	/* Probably happen when the player wants to start reloading and already
	 * initiates a PM_BeginWeaponChange() or PM_FinishWeaponChange() to another weapon at the same time.
	 * So we cancel the reloading attempt.
	 */
		return qfalse;
	}

	// Tequila: In akimbo mode, we don't want to start second weapon reloading
	// when the first is still being reloaded
	if ((weapon2 && pm->ps->weaponstate == WEAPON_RELOADING) ||
		(!weapon2 && pm->ps->weapon2state == WEAPON_RELOADING)) {
		return qfalse;
	}

	if(pm->ps->ammo[clip] <= 0) return qfalse;

	if(pm->ps->ammo[ammo] >= bg_weaponlist[weapon].clipAmmo) return qfalse;

	if(weapon == WP_NONE) return qfalse;

	if(weapon != WP_WINCHESTER66 && weapon != WP_PEACEMAKER
		&& weapon != WP_LIGHTNING && weapon != WP_WINCH97) {

		all_at_once = qtrue;
		if (reloading) return qfalse;
	} else {
		all_at_once = qfalse;
		if (reloading) {
			// Prevent any reload, if the weapon is already full
			if (pm->ps->ammo[ammo] + 1 >= bg_weaponlist[weapon].clipAmmo) return qfalse;
			// Prevent any reload if no ammo left
			if (pm->ps->ammo[clip] <= 1) return qfalse;
		}
	}


	// if reload is only one sequence
	if(all_at_once) {

		*weaponTime = PM_AnimLength(WP_ANIM_RELOAD, weapon2);

		if((weapon == WP_REMINGTON_GAUGE || weapon == WP_SAWEDOFF) &&
			pm->ps->ammo[ammo] == 1)
			PM_StartWeaponAnim(WP_ANIM_SPECIAL2, weapon2, reloadsound, qfalse);
		else
			PM_StartWeaponAnim(WP_ANIM_RELOAD, weapon2, reloadsound, qfalse);

		pm->ps->eFlags |= reload;

	} else {

		if (reloading) {
		// Successive reloads, here
			PM_StartWeaponAnim(WP_ANIM_SPECIAL2, weapon2, qtrue, qtrue);
			*weaponTime = PM_AnimLength(WP_ANIM_SPECIAL2, weapon2);
			pm->ps->ammo[ammo]++;
			pm->ps->ammo[clip]--;
		} else {
			PM_StartWeaponAnim(WP_ANIM_RELOAD, weapon2, reloadsound, qfalse);
			*weaponTime = PM_AnimLength(WP_ANIM_RELOAD, weapon2);
			pm->ps->eFlags |= reload;
		}

	}


	//start playeranim
	if(bg_weaponlist[weapon].wp_sort == WPS_PISTOL)
		PM_StartTorsoAnim(TORSO_PISTOL_RELOAD);
	else if(weapon == WP_GATLING)
		PM_StartTorsoAnim(TORSO_GATLING_RELOAD);
	else
		PM_StartTorsoAnim(TORSO_RIFLE_RELOAD);

	*weaponstate = WEAPON_RELOADING;


	// if akimbos, drop the other weapon as long as the other is reloading
	if(pm->ps->weapon2){

		// drop weapon1
		if(weapon2){

			// reset animation, first do the drop-anim of the other weapon
			if(oldweaponstate != WEAPON_RELOADING){
				PM_StartWeaponAnim(WP_ANIM_DROP, qfalse, qfalse, qfalse);
				pm->ps->weapon2Anim = oldanim;
				pm->ps->stats[STAT_FLAGS] |= SF_RELOAD2; // start animation later

				pm->ps->weaponTime = pm->ps->weapon2Time + PM_AnimLength(WP_ANIM_DROP, !weapon2);
				pm->ps->weapon2Time = PM_AnimLength(WP_ANIM_DROP, !weapon2);
				pm->ps->eFlags &= ~reload;
			} else {
				// reloadtime of other weapon
				pm->ps->weaponTime = pm->ps->weapon2Time;
			}
		} else {
			if(oldweaponstate != WEAPON_RELOADING){
				PM_StartWeaponAnim(WP_ANIM_DROP, qtrue, qfalse, qfalse);
				pm->ps->weaponAnim = oldanim;
				pm->ps->stats[STAT_FLAGS] |= SF_RELOAD;

				pm->ps->weapon2Time = pm->ps->weaponTime + PM_AnimLength(WP_ANIM_DROP, !weapon2);
				pm->ps->weaponTime = PM_AnimLength(WP_ANIM_DROP, !weapon2);
				pm->ps->eFlags &= ~reload;
			} else {
				pm->ps->weapon2Time = pm->ps->weaponTime;
			}
		}
	}
	return qtrue;

}

/*
==============
PM_ReloadEnd
==============
*/
static void PM_ReloadEnd( qboolean weapon2 ){
	int weapon = weapon2 ? pm->ps->weapon2 : pm->ps->weapon;
	int reload = weapon2 ? EF_RELOAD2 : EF_RELOAD;
	int clip = bg_weaponlist[weapon].clip;
	int ammo = weapon;

	// if there's the same pistol two times, use WP_AKIMBO for ammo saving
	if(!weapon2 && pm->ps->stats[STAT_FLAGS] & SF_SEC_PISTOL &&
		bg_weaponlist[pm->ps->weapon].wp_sort == WPS_PISTOL && pm->ps->weapon2)
		ammo = WP_AKIMBO;

	// these weapons get their ammo now
	if(weapon != WP_WINCHESTER66 && weapon != WP_PEACEMAKER
		&& weapon != WP_LIGHTNING && weapon != WP_WINCH97){

		// the weapon will not get full by this reload -> just add ammo, no checks
		if(pm->ps->ammo[clip] < bg_weaponlist[weapon].clipAmmo && weapon != WP_GATLING) {
			int over;

			over = pm->ps->ammo[ammo] + pm->ps->ammo[clip] - bg_weaponlist[weapon].clipAmmo;
			if(over < 0) {
				pm->ps->ammo[ammo] += pm->ps->ammo[clip];

				pm->ps->ammo[clip] = 0;
			} else {
				pm->ps->ammo[ammo] += (pm->ps->ammo[clip] - over);

				pm->ps->ammo[clip] = over;
			}

		} else if (weapon == WP_GATLING){
			pm->ps->ammo[WP_GATLING] = bg_weaponlist[WP_GATLING].clipAmmo;
			pm->ps->ammo[bg_weaponlist[WP_GATLING].clip] -= 1;

		} else {
			int amount;

			amount = bg_weaponlist[weapon].clipAmmo - pm->ps->ammo[ammo];
			pm->ps->ammo[ammo] = bg_weaponlist[weapon].clipAmmo;
			pm->ps->ammo[clip] -= amount;
		}
	// those weapons get the last ammo now
	} else {
	// For weapons reloading in one sequence, for the last one.
		pm->ps->ammo[ammo] ++;
		pm->ps->ammo[clip] -- ;
	}
	pm->ps->eFlags &= ~reload;

	// if player has akimbos change in the other weapon
	if(pm->ps->weapon2){

		// change in weapon
		PM_StartWeaponAnim(WP_ANIM_CHANGE, !weapon2, qfalse, qfalse);

		if(!weapon2){
			pm->ps->weapon2Time += PM_AnimLength(WP_ANIM_CHANGE, !weapon2);
		} else {
			pm->ps->weaponTime += PM_AnimLength(WP_ANIM_CHANGE, !weapon2);
		}

		// Play changeTO sound
		PM_AddEvent2( EV_CHANGETO_SOUND, pm->ps->weapon );
	}
}

/*
=================
PM_WeaponIdle
=================
*/
static void PM_WeaponIdle( int *weapon, int *weaponTime, int *weaponstate, qboolean weapon2 ) {
	*weaponTime = 0;

	if(*weaponstate != WEAPON_JUMPING)
		*weaponstate = WEAPON_READY;

	if(*weapon == WP_REMINGTON_GAUGE ||
		*weapon == WP_SAWEDOFF ||
		*weapon == WP_SHARPS) //no idle anims
		return;

	// nothing to do
	// the endfire-anim is played
	if ((!pm->ps->ammo[WP_DYNAMITE] && *weapon == WP_DYNAMITE) ||
		(!pm->ps->ammo[WP_MOLOTOV] && *weapon == WP_MOLOTOV)){
		PM_ContinueWeaponAnim(WP_ANIM_RELOAD, weapon2);
		return;
	// burning dynamite
	} else if (*weapon == WP_DYNAMITE && pm->ps->stats[STAT_WP_MODE] < 0) {
		PM_ContinueWeaponAnim(WP_ANIM_SPECIAL, weapon2);
		return;
	}

	// don't do idle animations on MOLOTOV if burning
	if(*weapon == WP_MOLOTOV && pm->ps->stats[STAT_WP_MODE] < 0){
		// stop burning if under water
		if (pm->waterlevel == 3) {
			pm->ps->stats[STAT_WP_MODE] = 0;
			pm->ps->stats[IDLE_TIMER] = 200;
		}
		// if it's time to throw it away signalize it by an animation
		else if(MOLOTOV_BURNTIME+pm->ps->stats[STAT_WP_MODE] <= PM_AnimLength(WP_ANIM_SPECIAL, qfalse) &&
			(pm->ps->weapon2Anim & ~ANIM_TOGGLEBIT) != WP_ANIM_SPECIAL){
			PM_StartWeaponAnim(WP_ANIM_SPECIAL, weapon2, qfalse, qfalse);
			return;
		}
	}

	// alt fire mode of knife
	if(pm->ps->stats[IDLE_TIMER] <= 0 || *weapon == WP_GATLING){
		if(*weapon == WP_KNIFE && pm->ps->stats[STAT_WP_MODE]){
		} else if(*weapon == WP_MOLOTOV && pm->ps->stats[STAT_WP_MODE] < 0) {
			PM_StartWeaponAnim(WP_ANIM_SPECIAL, weapon2, qfalse, qfalse);
			// gatling idle anim
		} else if(*weapon == WP_GATLING && !pm->ps->stats[STAT_GATLING_MODE]){
			PM_StartWeaponAnim(WP_ANIM_SPECIAL, weapon2, qfalse, qfalse);
		} else {
			if(pm->ps->stats[IDLE_TIMER] == -1 && weapon2){
			} else {
				PM_StartWeaponAnim(WP_ANIM_IDLE, weapon2, qfalse, qfalse);
			}
		}

		if(weapon2 || !pm->ps->weapon2)
			pm->ps->stats[IDLE_TIMER] = 2500 + rand()%5000 + PM_AnimLength(WP_ANIM_IDLE, weapon2);
	}
}

/*
====================
PM_PlanarCheck
checks if gatling can be built up here
====================
*/
static qboolean PM_PlanarCheck ( void ){
	vec3_t origin, end;
	vec3_t angles;
	trace_t trace;

	if(!pml.groundPlane)
		return qfalse;

	if (pm->waterlevel > 1) {
		PM_AddEvent(EV_GATLING_TOODEEP);
		return qfalse;
	}

	// see if there is enough place to build up the gatling
	VectorClear(angles);
	angles[YAW] = pm->ps->viewangles[YAW];
	AngleVectors( angles, origin, NULL, NULL);
	VectorScale(origin,30,origin);
	VectorAdd(pm->ps->origin, origin, origin);

	// Tequila comment: Use bigger gatling min/max to avoid bad situation when leaving the gatling
	pm->trace(&trace, origin, gatling_mins2, gatling_maxs2, origin, pm->ps->clientNum,
		MASK_SHOT);

	if(trace.allsolid || trace.startsolid){
		PM_AddEvent(EV_GATLING_NOSPACE);
		return qfalse;
	}

	// now check the ground (is there a ground at all?)
	VectorCopy(origin, end);
	end[2] -= 40;

	pm->trace(&trace, origin, NULL, NULL, end, pm->ps->clientNum, MASK_SHOT);

	// no ground
	if(trace.fraction == 1.0){
		return qfalse;
	}

	if(fabs(pml.groundTrace.endpos[2] + MINS_Z - trace.endpos[2]) > 13){
		PM_AddEvent(EV_GATLING_NOTPLANAR);
		return qfalse;
	}

	if(pml.groundTrace.plane.normal[2] < 0.96f){
		PM_AddEvent(EV_GATLING_NOTPLANAR);
		return qfalse;
	}

	return qtrue;
}

/*
==============
PM_WeaponAction

Generates the weapon actions
==============
*/
static void PM_WeaponAction( int *weapon, int *weaponTime, int *weaponstate, qboolean weapon2){
	int		addTime, delay = 0;
	int		reload = weapon2 ? EF_RELOAD2 : EF_RELOAD;
	int		ammo = *weapon;
	int old_weaponTime = *weaponTime ;

	// if there's the same pistol two times, use WP_AKIMBO for ammo saving
	if(!weapon2 && pm->ps->stats[STAT_FLAGS] & SF_SEC_PISTOL &&
		bg_weaponlist[*weapon].wp_sort == WPS_PISTOL && pm->ps->weapon2)
		ammo = WP_AKIMBO;

	if( pm->ps->eFlags & reload){

		switch(*weapon){
		case WP_WINCHESTER66:
		case WP_LIGHTNING:
			delay = 6*bg_weaponlist[*weapon].animations[WP_ANIM_RELOAD].frameLerp;
			break;
		case WP_WINCH97:
			delay = 7*bg_weaponlist[*weapon].animations[WP_ANIM_RELOAD].frameLerp;
			break;
		case WP_PEACEMAKER:
			delay = 16*bg_weaponlist[*weapon].animations[WP_ANIM_RELOAD].frameLerp;
			break;
		}

		if (delay && (*weaponTime <= delay)
			&& (pm->cmd.buttons & BUTTON_RELOAD)
			&& pm->ps->ammo[ammo] != bg_weaponlist[*weapon].clipAmmo
			&& pm->ps->ammo[bg_weaponlist[*weapon].clip]){
			*weaponTime = 0;
		}

	}

	if ( *weaponTime > 0 ) {
		return;
	}

	// finish changing zoom-mode
	if(*weapon == WP_SHARPS && pm->ps->stats[STAT_WP_MODE] < 0 &&
		pm->ps->stats[STAT_WP_MODE] != -3){
		pm->ps->stats[STAT_WP_MODE] += 1;
		pm->ps->stats[STAT_WP_MODE] *= -1;
	}

	// now end reload sequence by starting reload animation if necessary (akimbos)
	if((weapon2 && (pm->ps->stats[STAT_FLAGS] & SF_RELOAD2)) ||
		(!weapon2 && (pm->ps->stats[STAT_FLAGS] & SF_RELOAD))){
		pm->ps->stats[STAT_FLAGS] &= ~SF_RELOAD2;
		pm->ps->stats[STAT_FLAGS] &= ~SF_RELOAD;
		PM_StartWeaponAnim(WP_ANIM_RELOAD, weapon2, qtrue, qfalse);
		pm->ps->eFlags |= reload;

		if (weapon2)
			pm->ps->weapon2Time = pm->ps->weaponTime;
		else
			pm->ps->weaponTime = pm->ps->weapon2Time;

		return;
	}

	if((pm->cmd.buttons & BUTTON_RELOAD) && bg_weaponlist[*weapon].clip) {

		if(pm->ps->weapon2) {
			if((pm->cmd.buttons & BUTTON_ATTACK) && !weapon2) {
				if(PM_ReloadStart(qfalse, weaponTime, weaponstate)){
					return;
				}
			} else if((pm->cmd.buttons & BUTTON_ALT_ATTACK) && weapon2) {

				if(PM_ReloadStart(qtrue, weaponTime, weaponstate)){
					return;
				}
			}
		} else if(PM_ReloadStart(qfalse, weaponTime, weaponstate)){
			// disable zoom for the time of reloading
			if(*weapon == WP_SHARPS && pm->ps->stats[STAT_WP_MODE] == 1){
				pm->ps->stats[STAT_WP_MODE] = -3;
			}
			return;
		}
	}

	*weaponTime = old_weaponTime ;
	if ( *weaponTime > 0 ) {
		return;
	}


	if (pm->ps->eFlags & reload){
		PM_ReloadEnd(weapon2);

		pm->ps->stats[IDLE_TIMER] = 0;
		pm->ps->eFlags &= ~reload;

		// reset the zoom mode if nessecary
		if(*weapon == WP_SHARPS && pm->ps->stats[STAT_WP_MODE] == -3)
			pm->ps->stats[STAT_WP_MODE] = 1;
		//return;
	}

	// check for getting the scope on the weapon
	if ( pm->cmd.buttons & BUTTON_USE_HOLDABLE && !(pm->ps->oldbuttons & BUTTON_USE_HOLDABLE) &&
		pm->ps->weapon == WP_SHARPS) {
		if(pm->ps->powerups[PW_SCOPE]){
			if(pm->ps->powerups[PW_SCOPE]==1)
				pm->ps->powerups[PW_SCOPE] = -2;
			else
				pm->ps->powerups[PW_SCOPE] = -1;

			PM_StartWeaponAnim(WP_ANIM_SPECIAL, weapon2, qfalse, qfalse);
			PM_AddEvent(EV_SCOPE_PUT);
			*weaponTime += PM_AnimLength(WP_ANIM_SPECIAL, weapon2);
			return;
		}
	}

	// finish scope action
	if(pm->ps->powerups[PW_SCOPE] < 0){
		pm->ps->powerups[PW_SCOPE] *= -1;
		PM_StartWeaponAnim(WP_ANIM_SPECIAL2, weapon2, qfalse, qfalse);
		*weaponTime += PM_AnimLength(WP_ANIM_SPECIAL2, weapon2);
		return;
	}

	// change weapon if time
	if ( *weaponstate == WEAPON_DROPPING) {
		PM_FinishWeaponChange();
		return;
	}

	if ( *weaponstate == WEAPON_RAISING ||
		(pm->ps->weaponstate == WEAPON_READY &&
		pm->ps->weapon2state == WEAPON_READY)) {
		pm->ps->weaponstate = pm->ps->weapon2state = WEAPON_READY;

		if (!pm->ps->torsoTimer) {
			if(!pm->ps->weapon2){
				switch(*weapon){
				case WP_KNIFE:
				case WP_DYNAMITE:
				case WP_MOLOTOV:
					PM_StartTorsoAnim( TORSO_KNIFE_STAND);
					break;
				case WP_PEACEMAKER:
				case WP_REM58:
				case WP_SCHOFIELD:
					PM_StartTorsoAnim( TORSO_PISTOL_STAND);
					break;
				case WP_GATLING:
					if(pm->ps->stats[STAT_GATLING_MODE])
						PM_StartTorsoAnim( TORSO_GATLING_STAND);
					else
						PM_StartTorsoAnim( TORSO_RIFLE_STAND);
					break;
				case WP_NONE:
					PM_StartTorsoAnim( TORSO_HOLSTERED);
					break;
				default:
					PM_StartTorsoAnim( TORSO_RIFLE_STAND);
					break;
				}
			} else {
				PM_StartTorsoAnim( TORSO_PISTOLS_STAND);
			}
		}
		//return;
	}

	// weapons which don't have an >active< alternative fire mode
	if(pm->cmd.buttons & BUTTON_ALT_ATTACK){
		switch(*weapon){
		case WP_KNIFE:
		case WP_WINCHESTER66:
		case WP_LIGHTNING:
		case WP_WINCH97:
		case WP_SHARPS:
		case WP_DYNAMITE:
		case WP_MOLOTOV:
		case WP_GATLING:
			pm->cmd.buttons &= ~BUTTON_ALT_ATTACK;
			break;
		}
	}

	// check for fire
	if ( ((! (pm->cmd.buttons & BUTTON_ATTACK) &&
		! (pm->cmd.buttons & BUTTON_ALT_ATTACK)) ||
		(pm->cmd.buttons & BUTTON_RELOAD)) || pm->ps->weapon2) {

		if(pm->cmd.buttons & BUTTON_RELOAD){
			PM_WeaponIdle(weapon, weaponTime, weaponstate, weapon2);
			return;
		} else if(weapon2 && (pm->cmd.buttons & BUTTON_ALT_ATTACK)){
		} else if (pm->ps->weapon2 && !weapon2 && (pm->cmd.buttons & BUTTON_ATTACK)) {
		} else {
			PM_WeaponIdle(weapon, weaponTime, weaponstate, weapon2);
			return;
		}
	}

	// check if weapon == pistol AND if button is still pressed
	if( bg_weaponlist[*weapon].wp_sort == WPS_PISTOL) {

		if((pm->cmd.buttons & BUTTON_ATTACK) && (pm->ps->oldbuttons & BUTTON_ATTACK) &&
			((pm->ps->weapon2 && !weapon2) || !pm->ps->weapon2)){

			PM_WeaponIdle(weapon, weaponTime, weaponstate, weapon2);
			return;
		}

		if((pm->cmd.buttons & BUTTON_ALT_ATTACK) && (pm->ps->oldbuttons & BUTTON_ALT_ATTACK) &&
			((pm->ps->weapon2 && weapon2) || !pm->ps->weapon2)){

			PM_WeaponIdle(weapon, weaponTime, weaponstate, weapon2);
			return;
		}
	}

	// prevent +attack-button from being pressed when in wp_choose mode
	if((pm->ps->stats[STAT_FLAGS] & SF_WP_CHOOSE) &&
		    ((pm->cmd.buttons & BUTTON_ATTACK) || (pm->cmd.buttons & BUTTON_ALT_ATTACK))) {
		return;
	}

	// build gatling up if STAT_GATLING_MODE not 1
	if(*weapon == WP_GATLING && !pm->ps->stats[STAT_GATLING_MODE]){
		if(pm->ps->oldbuttons & BUTTON_ATTACK)
			return;

		// Tequila: Don't build a gatling if it is not in the inventory
		if(!(pm->ps->stats[STAT_WEAPONS] & (1 << WP_GATLING)))
			return;

		if( PM_PlanarCheck() ) {
			vec3_t	angles, origin;

			pm->ps->stats[STAT_FLAGS] &= ~SF_GAT_CARRY;
			PM_AddEvent(EV_BUILD_TURRET);
			PM_StartWeaponAnim(WP_ANIM_DROP, qfalse, qfalse, qfalse);
			*weaponTime += TRIPOD_TIME + 4*STAGE_TIME;
			pm->ps->stats[STAT_GATLING_MODE] = 1;

			// calculate origin of gatling gun
			VectorCopy( pm->ps->viewangles, angles );
			angles[ROLL] = 0;
			angles[PITCH] = 0;	// always forward

			VectorClear(origin);
			AngleVectors( angles, origin, NULL, NULL);
			VectorScale(origin,30,origin);
			VectorAdd(pm->ps->origin, origin, pm->ps->grapplePoint);

			// snap to integer coordinates for more efficient network bandwidth usage
			SnapVector( pm->ps->grapplePoint);
		}
		return;
	}

	// start the animation even if out of ammo
	if(!pm->ps->weapon2){
		switch(*weapon){
		case WP_KNIFE:
			if(pm->ps->stats[STAT_WP_MODE])
				PM_StartTorsoAnim(TORSO_THROW);
			else
				PM_StartTorsoAnim(TORSO_KNIFE_ATTACK);
			break;
		case WP_DYNAMITE:
		case WP_MOLOTOV:
			PM_StartTorsoAnim(TORSO_THROW);
			break;
		case WP_PEACEMAKER:
		case WP_REM58:
		case WP_SCHOFIELD:
			PM_StartTorsoAnim(TORSO_PISTOL_ATTACK);
			break;
		case WP_GATLING:
			PM_StartTorsoAnim(TORSO_GATLING_STAND);
			break;
		default:
			PM_StartTorsoAnim(TORSO_RIFLE_ATTACK);
			break;
		}
	} else {
		PM_StartTorsoAnim(TORSO_PISTOLS_ATTACK);
	}

	*weaponstate = WEAPON_FIRING;

	// check once more if two same pistols
	if (!weapon2 && (pm->ps->stats[STAT_FLAGS] & SF_SEC_PISTOL) &&
		bg_weaponlist[pm->ps->weapon].wp_sort == WPS_PISTOL && pm->ps->weapon2){
		if(!pm->ps->ammo[WP_AKIMBO]){

			PM_AddEvent( EV_NOAMMO_CLIP );
			*weaponTime += 500;
			return;
		}
	} else
	// check for out of ammo
	if (!pm->ps->ammo[*weapon] ||
		((*weapon == WP_REMINGTON_GAUGE || *weapon == WP_SAWEDOFF) &&
		pm->ps->ammo[*weapon] < 2 && (pm->cmd.buttons & BUTTON_ALT_ATTACK))) {

		if (*weapon != WP_DYNAMITE && *weapon != WP_NONE) {
			PM_AddEvent( EV_NOAMMO_CLIP );
			*weaponTime += 500;
		}
		return;
	}

	// take an ammo away if not infinite
	if ( pm->ps->ammo[ *weapon ] != -1 ) {

		if(!weapon2 && pm->ps->stats[STAT_FLAGS] & SF_SEC_PISTOL &&
			bg_weaponlist[pm->ps->weapon].wp_sort == WPS_PISTOL && pm->ps->weapon2)
			pm->ps->ammo[WP_AKIMBO]--;
		else
			pm->ps->ammo[ *weapon ]--;
	}

	//shotguns do use two shells for double-shot
	if((pm->cmd.buttons & BUTTON_ALT_ATTACK) && (*weapon == WP_REMINGTON_GAUGE ||
		*weapon == WP_SAWEDOFF))
		pm->ps->ammo[ *weapon ]--;

	//knife don't uses any ammo on normal fire mode
	if(*weapon == WP_KNIFE && !pm->ps->stats[STAT_WP_MODE])
		pm->ps->ammo[*weapon]++;


	//fire delay
	if((*weapon == WP_KNIFE && pm->ps->stats[STAT_WP_MODE])||
		*weapon == WP_DYNAMITE || *weapon == WP_MOLOTOV)
		pm->ps->stats[STAT_DELAY_TIME] = bg_weaponlist[*weapon].count;

	// fire weapon
	if((pm->cmd.buttons & BUTTON_ALT_ATTACK || (*weapon == WP_KNIFE &&
		pm->ps->stats[STAT_WP_MODE])) && !pm->ps->weapon2){

		PM_FireWeapon(WP_ANIM_ALT_FIRE, weapon2, qtrue, qfalse);
		addTime = PM_AnimLength(WP_ANIM_ALT_FIRE, weapon2);

	} else {
		int anim = 0;

		if(*weapon == WP_REMINGTON_GAUGE && pm->ps->ammo[*weapon] == 0){
			anim =  WP_ANIM_SPECIAL;
		} else if((*weapon == WP_DYNAMITE && pm->ps->ammo[WP_DYNAMITE]<1) ||
			(*weapon == WP_MOLOTOV && pm->ps->ammo[WP_MOLOTOV]<1)) {
			anim = WP_ANIM_RELOAD; //fireend
		} else if(*weapon != WP_GATLING){
			anim = WP_ANIM_FIRE;
		}

		PM_FireWeapon(anim, weapon2, qfalse, qfalse);

		addTime = PM_AnimLength(WP_ANIM_FIRE, weapon2)+
			bg_weaponlist[*weapon].addTime;
	}

	// this prevents weird animation-behaviours
	if(*weapon == WP_GATLING && pm->ps->stats[STAT_GATLING_MODE])
		PM_StartWeaponAnim(WP_ANIM_IDLE, weapon2, qfalse, qfalse);

	if(!addTime)
		addTime = 800;

	*weaponTime += addTime;

	//let idle anim begin after firing
	pm->ps->stats[IDLE_TIMER] = addTime + 2500 + rand()%5000;
}
#endif

/*
==============
PM_Weapon

Generates weapon events and modifes the weapon counter
==============
*/
static void PM_Weapon( void ) {
#ifndef SMOKINGUNS
	int		addTime;
#endif

	// don't allow attack until all buttons are up
	if ( pm->ps->pm_flags & PMF_RESPAWNED ) {
		return;
	}

	// ignore if spectator
#ifndef SMOKINGUNS
	if ( pm->ps->persistant[PERS_TEAM] == TEAM_SPECTATOR ) {
#else
	if ( pm->ps->persistant[PERS_TEAM] >= TEAM_SPECTATOR ) {
#endif
		return;
	}

	// check for dead player
	if ( pm->ps->stats[STAT_HEALTH] <= 0 ) {
		pm->ps->weapon = WP_NONE;
#ifdef SMOKINGUNS
		pm->ps->weapon2 = WP_NONE;
#endif
		return;
	}

#ifndef SMOKINGUNS
	// check for item using
	if ( pm->cmd.buttons & BUTTON_USE_HOLDABLE ) {
		if ( ! ( pm->ps->pm_flags & PMF_USE_ITEM_HELD ) ) {
			if ( bg_itemlist[pm->ps->stats[STAT_HOLDABLE_ITEM]].giTag == HI_MEDKIT
				&& pm->ps->stats[STAT_HEALTH] >= (pm->ps->stats[STAT_MAX_HEALTH] + 25) ) {
				// don't use medkit if at max health
			} else {
				pm->ps->pm_flags |= PMF_USE_ITEM_HELD;
				PM_AddEvent( EV_USE_ITEM0 + bg_itemlist[pm->ps->stats[STAT_HOLDABLE_ITEM]].giTag );
				pm->ps->stats[STAT_HOLDABLE_ITEM] = 0;
			}
			return;
		}
	} else {
		pm->ps->pm_flags &= ~PMF_USE_ITEM_HELD;
	}
#else
	//new weapon choose system
	if(pm->ps->stats[STAT_FLAGS] & SF_WP_CHOOSE) {
		if((!(pm->cmd.buttons & BUTTON_ATTACK) && (pm->ps->oldbuttons & BUTTON_ATTACK))
			    || (!(pm->cmd.buttons & BUTTON_ALT_ATTACK) && (pm->ps->oldbuttons & BUTTON_ALT_ATTACK))
			    || pm->cmd.buttons & BUTTON_CHOOSE_CANCEL) {
			pm->ps->stats[STAT_FLAGS] &= ~SF_WP_CHOOSE;
		}
	}

	//if you have dynammo you have also dynamite!
	if(pm->ps->ammo[WP_DYNAMITE] && !(pm->ps->stats[STAT_WEAPONS] & (1 << WP_DYNAMITE)))
		pm->ps->stats[STAT_WEAPONS] |= (1 << WP_DYNAMITE);
	if(pm->ps->ammo[WP_MOLOTOV] && !(pm->ps->stats[STAT_WEAPONS] & (1 << WP_MOLOTOV)))
		pm->ps->stats[STAT_WEAPONS] |= (1 << WP_MOLOTOV);

	//other case
	if(!pm->ps->ammo[WP_DYNAMITE] && (pm->ps->stats[STAT_WEAPONS] & (1 << WP_DYNAMITE)))
		pm->ps->stats[STAT_WEAPONS] &= ~(1 << WP_DYNAMITE);
	if(!pm->ps->ammo[WP_MOLOTOV] && (pm->ps->stats[STAT_WEAPONS] & (1 << WP_MOLOTOV)))
		pm->ps->stats[STAT_WEAPONS] &= ~(1 << WP_MOLOTOV);

	//no scope: no zoom
	if(pm->ps->stats[STAT_WP_MODE] && pm->ps->powerups[PW_SCOPE] != 2
		&& pm->ps->weapon == WP_SHARPS)
		pm->ps->stats[STAT_WP_MODE] = 0;
#endif

	// make weapon function
	if ( pm->ps->weaponTime > 0 ) {
		pm->ps->weaponTime -= pml.msec;
	}

#ifdef SMOKINGUNS
	if ( pm->ps->weapon2Time > 0) {
		pm->ps->weapon2Time -= pml.msec;
	}

	//count down idle timer
	if(pm->ps->stats[IDLE_TIMER] > 0){
		pm->ps->stats[IDLE_TIMER] -= pml.msec;
	}

	// decrease delay time
	if ( pm->ps->stats[STAT_DELAY_TIME] > 0){
		pm->ps->stats[STAT_DELAY_TIME] -= pml.msec;

		if(pm->ps->stats[STAT_DELAY_TIME] <= 0){
			pm->ps->stats[STAT_DELAY_TIME] = 0;

			PM_AddEvent(EV_FIRE_WEAPON_DELAY);
		}
		return;
	}

	if(pm->ps->stats[STAT_WP_MODE] < 0 && (pm->ps->weapon == WP_DYNAMITE ||
		pm->ps->weapon == WP_MOLOTOV)){ //no other weapons have negative WP_MODE values
		pm->ps->stats[STAT_WP_MODE] -= pml.msec;

		if(pm->ps->stats[STAT_WP_MODE] <= -(PM_AnimLength(WP_ANIM_ALT_FIRE, qfalse)+PM_AnimLength(WP_ANIM_SPECIAL, qfalse)) &&
			pm->ps->weapon == WP_DYNAMITE){
			PM_AddEvent(EV_FIRE_WEAPON);
			pm->ps->ammo[pm->ps->weapon]--;
			return;
		}
		if(pm->ps->weapon == WP_MOLOTOV){
			if(pm->ps->stats[STAT_WP_MODE] <= -MOLOTOV_BURNTIME){
				PM_AddEvent(EV_FIRE_WEAPON);
				pm->ps->ammo[pm->ps->weapon]--;
				return;
			}
		} else {
			PM_AddEvent(EV_DBURN);
		}
	}

//
//	ADDITIONAL WEAPON FUNCTIONS
//
	//check for passive fire mode change
	if(pm->cmd.buttons & BUTTON_ALT_ATTACK){
		switch(pm->ps->weapon){
		case WP_KNIFE:
			if(pm->ps->weaponTime > 0)
				break;

			if(pm->ps->ammo[pm->ps->weapon] == 1)
				break;

			pm->ps->stats[STAT_WP_MODE] = -(pm->ps->stats[STAT_WP_MODE]-1);
			pm->ps->stats[IDLE_TIMER] = 0;
			PM_AddEvent(EV_ALT_WEAPON_MODE);

			if(pm->ps->stats[STAT_WP_MODE]){
				PM_StartWeaponAnim(WP_ANIM_SPECIAL2, qfalse, qfalse, qfalse);
				pm->ps->weaponTime += PM_AnimLength(WP_ANIM_SPECIAL2, qfalse);
			} else {
				PM_StartWeaponAnim(WP_ANIM_SPECIAL, qfalse, qfalse, qfalse);
				pm->ps->weaponTime += PM_AnimLength(WP_ANIM_SPECIAL, qfalse);
			}
			break;
		case WP_SHARPS:
			if(pm->ps->powerups[PW_SCOPE] != 2 || (pm->ps->oldbuttons & BUTTON_ALT_ATTACK) ||
				(pm->ps->weaponTime > 0 && pm->ps->stats[STAT_WP_MODE] != -3))
				break;

			if (pm->ps->stats[STAT_WP_MODE] == -3)
				pm->ps->stats[STAT_WP_MODE] = 0;
			else
				pm->ps->stats[STAT_WP_MODE] = pm->ps->stats[STAT_WP_MODE]-2;
			PM_AddEvent(EV_ALT_WEAPON_MODE);
			pm->ps->weaponTime += SCOPE_TIME;
			break;
		case WP_DYNAMITE:
		case WP_MOLOTOV:
			if(pm->ps->stats[STAT_WP_MODE] >= 0 && pm->ps->ammo[pm->ps->weapon] &&
				pm->ps->weaponTime <= 0 && pm->waterlevel < 3){

 				pm->ps->stats[STAT_WP_MODE] = -1;
				pm->ps->weaponTime = PM_AnimLength(WP_ANIM_ALT_FIRE, qfalse);

				PM_StartWeaponAnim(WP_ANIM_ALT_FIRE, qfalse, qfalse, qfalse);
				PM_AddEvent(EV_LIGHT_DYNAMITE);

				if(pm->ps->weapon == WP_MOLOTOV)
					pm->ps->stats[IDLE_TIMER] = 0;
				return;
			}
			break;
		}
	}
#endif

	// check for weapon change
	// can't change if weapon is firing, but can change
	// again if lowering or raising
#ifndef SMOKINGUNS
	if ( pm->ps->weaponTime <= 0 || pm->ps->weaponstate != WEAPON_FIRING ) {
		if ( pm->ps->weapon != pm->cmd.weapon ) {
			PM_BeginWeaponChange( pm->cmd.weapon );
		}
	}

	if ( pm->ps->weaponTime > 0 ) {
		return;
	}

	// change weapon if time
	if ( pm->ps->weaponstate == WEAPON_DROPPING ) {
		PM_FinishWeaponChange();
		return;
	}

	if ( pm->ps->weaponstate == WEAPON_RAISING ) {
		pm->ps->weaponstate = WEAPON_READY;
		if ( pm->ps->weapon == WP_GAUNTLET ) {
			PM_StartTorsoAnim( TORSO_STAND2 );
		} else {
			PM_StartTorsoAnim( TORSO_STAND );
		}
		return;
	}

	// check for fire
	if ( ! (pm->cmd.buttons & BUTTON_ATTACK) ) {
		pm->ps->weaponTime = 0;
		pm->ps->weaponstate = WEAPON_READY;
		return;
	}

	// start the animation even if out of ammo
	if ( pm->ps->weapon == WP_GAUNTLET ) {
		// the guantlet only "fires" when it actually hits something
		if ( !pm->gauntletHit ) {
			pm->ps->weaponTime = 0;
			pm->ps->weaponstate = WEAPON_READY;
			return;
		}
		PM_StartTorsoAnim( TORSO_ATTACK2 );
	} else {
		PM_StartTorsoAnim( TORSO_ATTACK );
	}

	pm->ps->weaponstate = WEAPON_FIRING;

	// check for out of ammo
	if ( ! pm->ps->ammo[ pm->ps->weapon ] ) {
		PM_AddEvent( EV_NOAMMO );
		pm->ps->weaponTime += 500;
		return;
	}

	// take an ammo away if not infinite
	if ( pm->ps->ammo[ pm->ps->weapon ] != -1 ) {
		pm->ps->ammo[ pm->ps->weapon ]--;
	}

	// fire weapon
	PM_AddEvent( EV_FIRE_WEAPON );

	switch( pm->ps->weapon ) {
	default:
	case WP_GAUNTLET:
		addTime = 400;
		break;
	case WP_LIGHTNING:
		addTime = 50;
		break;
	case WP_SHOTGUN:
		addTime = 1000;
		break;
	case WP_MACHINEGUN:
		addTime = 100;
		break;
	case WP_GRENADE_LAUNCHER:
		addTime = 800;
		break;
	case WP_ROCKET_LAUNCHER:
		addTime = 800;
		break;
	case WP_PLASMAGUN:
		addTime = 100;
		break;
	case WP_RAILGUN:
		addTime = 1500;
		break;
	case WP_BFG:
		addTime = 200;
		break;
	case WP_GRAPPLING_HOOK:
		addTime = 400;
		break;
#ifdef MISSIONPACK
	case WP_NAILGUN:
		addTime = 1000;
		break;
	case WP_PROX_LAUNCHER:
		addTime = 800;
		break;
	case WP_CHAINGUN:
		addTime = 30;
		break;
#endif
	}

#ifdef MISSIONPACK
	if( bg_itemlist[pm->ps->stats[STAT_PERSISTANT_POWERUP]].giTag == PW_SCOUT ) {
		addTime /= 1.5;
	}
	else
	if( bg_itemlist[pm->ps->stats[STAT_PERSISTANT_POWERUP]].giTag == PW_AMMOREGEN ) {
		addTime /= 1.3;
  }
  else
#endif
	if ( pm->ps->powerups[PW_HASTE] ) {
		addTime /= 1.3;
	}

	pm->ps->weaponTime += addTime;
#else
	// spoon modified it so you can't change weapon if lowering or raising (dunno why)
	// i've written it back, because of the last used weapon func.
	// it can't work very nice, if you can't change weapon again. - iv
	if ((pm->ps->weaponTime <= 0 || (pm->ps->weaponstate != WEAPON_FIRING)) &&
		(pm->ps->weapon2Time <= 0 || (pm->ps->weapon2state != WEAPON_FIRING))) {
		qboolean automatic = qfalse;

		// if chooseweapon == WP_AKIMBO and akimbos are already in
		if(pm->cmd.weapon == WP_AKIMBO && pm->ps->weapon2){

			if(!(pm->ps->stats[STAT_WEAPONS] & (1 << pm->ps->weapon2)))
				automatic = qtrue;

			if(!(pm->ps->stats[STAT_WEAPONS] & (1 << pm->ps->weapon)))
				automatic = qtrue;

			// if both weapons the same, but no two weapons are available
			if(pm->ps->weapon == pm->ps->weapon2 &&
				!(pm->ps->stats[STAT_FLAGS] & SF_SEC_PISTOL))
				automatic = qtrue;

		} else if ( pm->ps->weapon != pm->cmd.weapon ||
			(pm->ps->weapon && pm->ps->weapon2)) { // if not akimbo and weapon2 -> change it

			// NB:
			// In case, pm->cmd.weapon == WP_SEC_PISTOL,
			// pm->ps->weapon != pm->cmd.weapon will always be true.
			// Then, a change weapon will happen even if it can result in the same pistol type
			qboolean left_pistol = (pm->cmd.weapon == WP_SEC_PISTOL);
			int weapon_pistol = 0;
			if (left_pistol) {
			// Detect a change to the pistol on the left holster
			// Here, we simply reset pm->cmd.weapon to the first pistol type
			// (Case when the player has only one pistol type *NORMALLY*)
				weapon_pistol = BG_SearchTypeWeapon(WPS_PISTOL, pm->ps->stats[STAT_WEAPONS], 0);
				if (weapon_pistol)
					pm->cmd.weapon = weapon_pistol;
			}

			PM_BeginWeaponChange( pm->cmd.weapon );

			if (left_pistol && weapon_pistol && ( pm->ps->stats[STAT_FLAGS] & SF_SEC_PISTOL )) {
			// Detect a change to the pistol in the left holster
			// Swap the ammo between WP_AKIMBO and
			// WP_REM58, WP_SCHOFIELD or WP_PEACEMAKER.
				int ammo = pm->ps->ammo[ WP_AKIMBO ];
				pm->ps->ammo[ WP_AKIMBO ] = pm->ps->ammo[ weapon_pistol ];
				pm->ps->ammo[ weapon_pistol ] = ammo;
			}

		} //automatic weapon change
		else if(!(pm->ps->stats[STAT_WEAPONS] & (1 << pm->ps->weapon)) &&
			pm->ps->weapon != WP_DYNAMITE && pm->ps->weapon != WP_NONE &&
			!pm->ps->stats[STAT_OLDWEAPON]){

			automatic = qtrue;
		}

		if(automatic){
			int i, start;

			start = pm->cmd.weapon == WP_AKIMBO ? WP_PEACEMAKER : WP_GATLING;

			for ( i = start ; i > 0 ; i-- ) {
				if ( pm->ps->stats[STAT_WEAPONS] & ( 1 << i ) ) {
					pm->cmd.weapon = i;
					PM_ChangeWeapon(i);
					break;
				}
			}
		}
	}

	//uncheck alternative fire mode if ammo < 2
	if(pm->ps->ammo[pm->ps->weapon] == 1 && pm->ps->weapon == WP_KNIFE &&
		pm->ps->stats[STAT_WP_MODE] && !pm->ps->stats[STAT_DELAY_TIME] &&
		pm->cmd.weapon == WP_KNIFE){

		pm->ps->stats[STAT_WP_MODE] = 0;

		PM_AddEvent(EV_ALT_WEAPON_MODE);
		PM_StartWeaponAnim(WP_ANIM_SPECIAL, qfalse, qfalse, qfalse);
		pm->ps->weaponTime += PM_AnimLength(WP_ANIM_SPECIAL2, qfalse);
	}

//
//	WEAPONTIME CHECK
//

	PM_WeaponAction(&pm->ps->weapon, &pm->ps->weaponTime, &pm->ps->weaponstate, qfalse);

	if(pm->ps->weapon2)
		PM_WeaponAction(&pm->ps->weapon2, &pm->ps->weapon2Time, &pm->ps->weapon2state, qtrue);
#endif
}

/*
================
PM_Animate
================
*/

static void PM_Animate( void ) {
	if ( pm->cmd.buttons & BUTTON_GESTURE ) {
		if ( pm->ps->torsoTimer == 0 ) {
#ifndef SMOKINGUNS
			PM_StartTorsoAnim( TORSO_GESTURE );
#else
			if(bg_weaponlist[pm->ps->weapon].wp_sort == WPS_PISTOL)
				PM_StartTorsoAnim( TORSO_TAUNT_PISTOL );
			else
				PM_StartTorsoAnim( TORSO_TAUNT );
#endif

			pm->ps->torsoTimer = TIMER_GESTURE;
			PM_AddEvent( EV_TAUNT );
		}
#ifndef SMOKINGUNS
	} else if ( pm->cmd.buttons & BUTTON_GETFLAG ) {
		if ( pm->ps->torsoTimer == 0 ) {
			PM_StartTorsoAnim( TORSO_GETFLAG );
			pm->ps->torsoTimer = 600;	//TIMER_GESTURE;
		}
	} else if ( pm->cmd.buttons & BUTTON_GUARDBASE ) {
		if ( pm->ps->torsoTimer == 0 ) {
			PM_StartTorsoAnim( TORSO_GUARDBASE );
			pm->ps->torsoTimer = 600;	//TIMER_GESTURE;
		}
	} else if ( pm->cmd.buttons & BUTTON_PATROL ) {
		if ( pm->ps->torsoTimer == 0 ) {
			PM_StartTorsoAnim( TORSO_PATROL );
			pm->ps->torsoTimer = 600;	//TIMER_GESTURE;
		}
	} else if ( pm->cmd.buttons & BUTTON_FOLLOWME ) {
		if ( pm->ps->torsoTimer == 0 ) {
			PM_StartTorsoAnim( TORSO_FOLLOWME );
			pm->ps->torsoTimer = 600;	//TIMER_GESTURE;
		}
	} else if ( pm->cmd.buttons & BUTTON_AFFIRMATIVE ) {
		if ( pm->ps->torsoTimer == 0 ) {
			PM_StartTorsoAnim( TORSO_AFFIRMATIVE);
			pm->ps->torsoTimer = 600;	//TIMER_GESTURE;
		}
	} else if ( pm->cmd.buttons & BUTTON_NEGATIVE ) {
		if ( pm->ps->torsoTimer == 0 ) {
			PM_StartTorsoAnim( TORSO_NEGATIVE );
			pm->ps->torsoTimer = 600;	//TIMER_GESTURE;
		}
#endif
	}
}


/*
================
PM_DropTimers
================
*/
static void PM_DropTimers( void ) {
	// drop misc timing counter
	if ( pm->ps->pm_time ) {
		if ( pml.msec >= pm->ps->pm_time ) {
			pm->ps->pm_flags &= ~PMF_ALL_TIMES;
			pm->ps->pm_time = 0;
		} else {
			pm->ps->pm_time -= pml.msec;
		}
	}

	// drop animation counter
	if ( pm->ps->legsTimer > 0 ) {
		pm->ps->legsTimer -= pml.msec;
		if ( pm->ps->legsTimer < 0 ) {
			pm->ps->legsTimer = 0;
#ifdef SMOKINGUNS
			if(pm->ps->weaponstate == WEAPON_JUMPING)
				pm->ps->weaponstate = WEAPON_READY;
#endif
		}
	}

	if ( pm->ps->torsoTimer > 0 ) {
		pm->ps->torsoTimer -= pml.msec;
		if ( pm->ps->torsoTimer < 0 ) {
			pm->ps->torsoTimer = 0;
		}
	}
}

/*
================
PM_UpdateViewAngles

This can be used as another entry point when only the viewangles
are being updated instead of a full move
================
*/
void PM_UpdateViewAngles( playerState_t *ps, const usercmd_t *cmd ) {
	short		temp;
	int		i;
#ifdef SMOKINGUNS
	vec3_t oldangles;

	VectorCopy(pm->ps->viewangles, oldangles);

	// if in duel intro mode, don't update viewangles
	if (ps->stats[STAT_FLAGS] & SF_DU_INTRO){

		// set the delta angle
		for (i=0 ; i<3 ; i++) {
			int		cmdAngle;

			cmdAngle = ANGLE2SHORT(pm->ps->viewangles[i]);
			pm->ps->delta_angles[i] = cmdAngle - pm->cmd.angles[i];
		}
		return;
	}
#endif

	if ( ps->pm_type == PM_INTERMISSION || ps->pm_type == PM_SPINTERMISSION) {
		return;		// no view changes at all
	}

	if ( ps->pm_type != PM_SPECTATOR && ps->stats[STAT_HEALTH] <= 0 ) {
		return;		// no view changes at all
	}

	// circularly clamp the angles with deltas
	for (i=0 ; i<3 ; i++) {
		temp = cmd->angles[i] + ps->delta_angles[i];
		if ( i == PITCH ) {
#ifndef SMOKINGUNS
			// don't let the player look up or down more than 90 degrees
			if ( temp > 16000 ) {
				ps->delta_angles[i] = 16000 - cmd->angles[i];
				temp = 16000;
			} else if ( temp < -16000 ) {
				ps->delta_angles[i] = -16000 - cmd->angles[i];
				temp = -16000;
			}
#else
			float maxangle = 16000;

			if(pm->ps->stats[STAT_GATLING_MODE] && pm->ps->weapon == WP_GATLING){
				if(temp > 0)
					maxangle = ANGLE2SHORT(GATLING_DOWN);
				else
					maxangle = ANGLE2SHORT(GATLING_UP);

			}

			// don't let the player look up or down more than 90 degrees
			if ( temp > maxangle) {
				ps->delta_angles[i] = maxangle - cmd->angles[i];
				temp = maxangle;
			} else if ( temp < -maxangle) {
				ps->delta_angles[i] = -maxangle - cmd->angles[i];
				temp = -maxangle;
			}
#endif
		}
		ps->viewangles[i] = SHORT2ANGLE(temp);
	}

#ifdef SMOKINGUNS
	//position player when on gatling
	if(pm->ps->stats[STAT_GATLING_MODE] && pm->ps->weapon == WP_GATLING){
		vec3_t angles, forward, oldorigin;
		//float height = ps->grapplePoint[2]+0.125f;
		float pheight = ps->origin[2];
		trace_t trace;
		vec3_t mins, maxs;
		qboolean goback = qfalse;

		// disable normal movement
		pm->ps->velocity[0] = pm->ps->velocity[1] = 0.0f;

		VectorCopy(ps->origin, oldorigin);

		VectorCopy(ps->viewangles, angles);
		angles[PITCH] = 0;
		angles[ROLL] = 0;

		AngleVectors(angles, forward, NULL, NULL);
		VectorScale(forward, GATLING_RANGE, forward);

		VectorSubtract(ps->grapplePoint, forward, ps->origin);
		ps->origin[2] = pheight;

		// check if we're going through solid
		if(!goback){

			VectorSet(mins, -16, -16, -18);// set this value a bit higher
									// to be able to walk on some higher planes-24);
			VectorSet(maxs, 16, 16, 28);

			pm->trace(&trace, oldorigin, mins,
				maxs, ps->origin, ps->clientNum, CONTENTS_SOLID|CONTENTS_BODY);

			if(trace.fraction != 1.0)
				goback = qtrue;
		}

		// see if we're falling/stepping down
		if(!goback){
			vec3_t down;

			// trace down
			VectorCopy(pm->ps->origin, down);
			down[2] -= 10;

			// set normal mins/maxs
			VectorSet(mins, -10, -10, -24);
			VectorSet(maxs, 10, 10, 28);

			pm->trace(&trace, pm->ps->origin, mins, maxs, down, ps->clientNum,
				CONTENTS_SOLID|CONTENTS_BODY);

			if(trace.fraction == 1.0){
				// go back to the last valid position
				trace.fraction = 0;
				goback = qtrue;
			}
		}

		// there's something solid in between
		if(goback){
			//calculate new viewangles
			float delta = 0, olddelta;
			vec3_t viewangles;

			AnglesNormalize180(viewangles);
			AnglesNormalize180(oldangles);

			// go back some degrees
			olddelta = AngleSubtract(ps->viewangles[YAW], oldangles[YAW]);
			delta = (1-trace.fraction)*olddelta;

			// modify deltaangles
			ps->viewangles[YAW] -= delta;
			ps->delta_angles[YAW] = ANGLE2SHORT(ps->viewangles[YAW]) - cmd->angles[YAW];

			// set the origin for the gatling use
			VectorCopy(pm->ps->viewangles, angles);
			angles[PITCH] = 0;
			angles[ROLL] = 0;

			AngleVectors(angles, forward, NULL, NULL);
			VectorScale(forward, GATLING_RANGE, forward);

			VectorSubtract(pm->ps->grapplePoint, forward, pm->ps->origin);
			pm->ps->origin[2] = pheight;
		}
	}
#endif
}


/*
===================
PM_LadderMove()
by: Calrathan [Arthur Tomlin]

Right now all I know is that this works for VERTICAL ladders.
Ladders with angles on them (urban2 for AQ2) haven't been tested.
===================
*/
#ifdef SMOKINGUNS
static void PM_LadderMove( void ) {
	int i;
	vec3_t wishvel;
	float wishspeed;
	vec3_t wishdir;
	float scale;
	float vel;

	//
	// FIRST CHECK IF THE PLAYER HAS TO BE ABLE TO WALK BACKWARDS
	//
	trace_t trace;
	vec3_t origin;
	qboolean backwards =  qfalse;

	VectorCopy(pm->ps->origin, origin);
	origin[2] -= 20;

	pm->trace (&trace, pm->ps->origin, pm->mins, pm->maxs, origin,
		pm->ps->clientNum, MASK_PLAYERSOLID);

	if(trace.fraction < 1)
		backwards = qtrue;
	// CHECK END

	PM_Friction ();

	scale = PM_CmdScale( &pm->cmd );

	// user intentions [what the user is attempting to do]
	if ( !scale ) {
		wishvel[0] = 0;
		wishvel[1] = 0;
		wishvel[2] = 0;
	}
	else {   // if they're trying to move... lets calculate it
		for (i=0 ; i<3 ; i++){
			float fw = pm->cmd.forwardmove, rt = pm->cmd.rightmove;
			if(fw < 0 && !backwards)
				fw = 0;

			wishvel[i] = scale * pml.forward[i]*fw +
				     scale * pml.right[i]*rt;
		}
		wishvel[2] = scale * (pm->cmd.forwardmove + pm->cmd.upmove);
	}

	VectorCopy (wishvel, wishdir);
	wishspeed = VectorNormalize(wishdir);

	if ( wishspeed > pm->ps->speed * pm_ladderScale ) {
		wishspeed = pm->ps->speed * pm_ladderScale;
	}

	PM_Accelerate (wishdir, wishspeed, pm_ladderAccelerate);

	// This SHOULD help us with sloped ladders, but it remains untested.
	if ( pml.groundPlane && DotProduct( pm->ps->velocity,
		pml.groundTrace.plane.normal ) < 0 ) {
		vel = VectorLength(pm->ps->velocity);
		// slide along the ground plane [the ladder section under our feet]
		PM_ClipVelocity (pm->ps->velocity, pml.groundTrace.plane.normal,
			pm->ps->velocity, OVERCLIP );

		VectorNormalize(pm->ps->velocity);
		VectorScale(pm->ps->velocity, vel, pm->ps->velocity);
	}

	PM_SlideMove( qfalse ); // move without gravity
}


/*
=============
CheckLadder [ ARTHUR TOMLIN ]
=============
*/
void CheckLadder( void )
{
	vec3_t flatforward,spot;
	vec3_t origin;
	trace_t trace;

	// check if we should be backwards walking down the ladder
	qboolean backwards = qfalse;

	VectorCopy(pm->ps->origin, origin);
	origin[2] -= 30;

	pm->trace (&trace, pm->ps->origin, pm->mins, pm->maxs, origin,
		pm->ps->clientNum, MASK_PLAYERSOLID);

	if(trace.fraction == 1)
		backwards = qtrue;
	// check end

	VectorCopy(pm->ps->origin, origin);
	if(backwards)
		origin[2] -= 20;

	// check for ladder
	flatforward[0] = pml.forward[0];
	flatforward[1] = pml.forward[1];
	flatforward[2] = 0;

	VectorNormalize (flatforward);
	VectorMA (origin, 2, flatforward, spot);

	pm->trace (&trace, origin, pm->mins, pm->maxs, spot,
		pm->ps->clientNum, MASK_PLAYERSOLID);

	if ((trace.fraction < 1) && (trace.surfaceFlags & SURF_LADDER)){
		pml.ladder = qtrue;
	} else {
		pml.ladder = qfalse;
	}
}
#endif


/*
================
PmoveSingle

================
*/
void trap_SnapVector( float *v );

void PmoveSingle (pmove_t *pmove) {
#ifdef SMOKINGUNS
	vec3_t origin; //just to save origin for PM_Freeze
#endif
	pm = pmove;

	// this counter lets us debug movement problems with a journal
	// by setting a conditional breakpoint fot the previous frame
	c_pmove++;

	// clear results
	pm->numtouch = 0;
	pm->watertype = 0;
	pm->waterlevel = 0;

	if ( pm->ps->stats[STAT_HEALTH] <= 0 ) {
		pm->tracemask &= ~CONTENTS_BODY;	// corpses can fly through bodies
	}

	// make sure walking button is clear if they are running, to avoid
	// proxy no-footsteps cheats
	if ( abs( pm->cmd.forwardmove ) > 64 || abs( pm->cmd.rightmove ) > 64 ) {
		pm->cmd.buttons &= ~BUTTON_WALKING;
	}

	// set the talk balloon flag
	if ( pm->cmd.buttons & BUTTON_TALK ) {
		pm->ps->eFlags |= EF_TALK;
#ifndef SMOKINGUNS
	} else {
		pm->ps->eFlags &= ~EF_TALK;
#else
		// set the buy balloon flag, instead.
		// EF_BUY is evaluated before EF_TALK
		// in ./code/cgame/cg_players.c:CG_PlayerSprites(), so it's OK.
		//
		// BUTTON_TALK should be set indirectly by a previous
		// trap_Key_SetCatcher() call.
		// This will prevent any explicit "+button8" console command,
		// instead of "wq_buy".
		if ( pm->cmd.buttons & BUTTON_BUYMENU ) {
			pm->ps->eFlags |= EF_BUY;
		} else {
			pm->ps->eFlags &= ~EF_BUY;
		}
	} else {
		pm->ps->eFlags &= ~( EF_TALK | EF_BUY );
#endif
	}

	// set the firing flag for continuous beam weapons
	if ( !(pm->ps->pm_flags & PMF_RESPAWNED) && pm->ps->pm_type != PM_INTERMISSION && pm->ps->pm_type != PM_NOCLIP
#ifndef SMOKINGUNS
		&& ( pm->cmd.buttons & BUTTON_ATTACK ) && pm->ps->ammo[ pm->ps->weapon ] ) {
#else
		&& (( pm->cmd.buttons & BUTTON_ATTACK ) || ( pm->cmd.buttons & BUTTON_ALT_ATTACK ))
		&& pm->ps->ammo[ pm->ps->weapon ] && pm->ps->weaponstate == WEAPON_FIRING) {
#endif
		pm->ps->eFlags |= EF_FIRING;
	} else {
		pm->ps->eFlags &= ~EF_FIRING;
	}

#ifdef SMOKINGUNS
	if(pm->ps->weapon == WP_GATLING && pm->ps->stats[STAT_GATLING_MODE]){
		pm->ps->powerups[PW_GATLING] = 1;
	} else {
		pm->ps->powerups[PW_GATLING] = 0;
	}

	// Smokin' Guns
	if ( pm->cmd.buttons & BUTTON_CHOOSE_CANCEL ) {
		pm->ps->stats[STAT_FLAGS] &= ~SF_WP_CHOOSE;
	}

	if ( pm->cmd.buttons & BUTTON_CHOOSE_MODE && !(pm->ps->oldbuttons & BUTTON_CHOOSE_MODE) ) {
		pm->ps->stats[STAT_FLAGS] |= SF_WP_CHOOSE;
	}
#endif

	// clear the respawned flag if attack and use are cleared
	if ( pm->ps->stats[STAT_HEALTH] > 0 &&
		!( pm->cmd.buttons & (BUTTON_ATTACK | BUTTON_USE_HOLDABLE) ) ) {
		pm->ps->pm_flags &= ~PMF_RESPAWNED;
	}

	// if talk button is down, dissallow all other input
	// this is to prevent any possible intercept proxy from
	// adding fake talk balloons
	if ( pmove->cmd.buttons & BUTTON_TALK ) {
		// keep the talk button set tho for when the cmd.serverTime > 66 msec
		// and the same cmd is used multiple times in Pmove
		pmove->cmd.buttons = BUTTON_TALK;
		pmove->cmd.forwardmove = 0;
		pmove->cmd.rightmove = 0;
		pmove->cmd.upmove = 0;
	}

	// clear all pmove local vars
	memset (&pml, 0, sizeof(pml));

	// determine the time
	pml.msec = pmove->cmd.serverTime - pm->ps->commandTime;
	if ( pml.msec < 1 ) {
		pml.msec = 1;
	} else if ( pml.msec > 200 ) {
		pml.msec = 200;
	}
	pm->ps->commandTime = pmove->cmd.serverTime;

	// save old org in case we get stuck
	VectorCopy (pm->ps->origin, pml.previous_origin);

	// save old velocity for crashlanding
	VectorCopy (pm->ps->velocity, pml.previous_velocity);

	pml.frametime = pml.msec * 0.001;

	// update the viewangles
	PM_UpdateViewAngles( pm->ps, &pm->cmd );

	AngleVectors (pm->ps->viewangles, pml.forward, pml.right, pml.up);

	if ( pm->cmd.upmove < 10 ) {
		// not holding jump
		pm->ps->pm_flags &= ~PMF_JUMP_HELD;
	}

	// decide if backpedaling animations should be used
	if ( pm->cmd.forwardmove < 0 ) {
		pm->ps->pm_flags |= PMF_BACKWARDS_RUN;
	} else if ( pm->cmd.forwardmove > 0 || ( pm->cmd.forwardmove == 0 && pm->cmd.rightmove ) ) {
		pm->ps->pm_flags &= ~PMF_BACKWARDS_RUN;
	}

	if ( pm->ps->pm_type >= PM_DEAD ) {
		pm->cmd.forwardmove = 0;
		pm->cmd.rightmove = 0;
		pm->cmd.upmove = 0;
	}

	if ( pm->ps->pm_type == PM_SPECTATOR ) {
		PM_CheckDuck ();
		PM_FlyMove ();
		PM_DropTimers ();
		return;
	}

#ifdef SMOKINGUNS
	if (pm->ps->pm_type == PM_CHASECAM ) {
		PM_NoclipMove ();
		PM_DropTimers ();
		return;
	}
#endif

	if ( pm->ps->pm_type == PM_NOCLIP ) {
		PM_NoclipMove ();
		PM_DropTimers ();
		return;
	}

	if (pm->ps->pm_type == PM_FREEZE) {
#ifdef SMOKINGUNS
		VectorCopy(pm->ps->origin, origin);
		PM_NoclipMove ();
		PM_DropTimers ();
		VectorCopy(origin, pm->ps->origin);
#endif
		return;		// no movement at all
	}

	if ( pm->ps->pm_type == PM_INTERMISSION || pm->ps->pm_type == PM_SPINTERMISSION) {
		return;		// no movement at all
	}

	// set watertype, and waterlevel
	PM_SetWaterLevel();
	pml.previous_waterlevel = pmove->waterlevel;

	// set mins, maxs, and viewheight
	PM_CheckDuck ();

	// set groundentity
	PM_GroundTrace();

	if ( pm->ps->pm_type == PM_DEAD ) {
		PM_DeadMove ();
	}

	PM_DropTimers();
#ifdef SMOKINGUNS
	CheckLadder();  // ARTHUR TOMLIN check and see if we're on a ladder
#endif

#ifndef SMOKINGUNS
	if ( pm->ps->powerups[PW_INVULNERABILITY] ) {
		PM_InvulnerabilityMove();
	} else
#endif
#ifndef SMOKINGUNS
	if ( pm->ps->powerups[PW_FLIGHT] ) {
		// flight powerup doesn't allow jump and has different friction
		PM_FlyMove();
	} else if (pm->ps->pm_flags & PMF_GRAPPLE_PULL) {
		PM_GrappleMove();
		// We can wiggle a bit
		PM_AirMove();
	} else if (pm->ps->pm_flags & PMF_TIME_WATERJUMP) {
#else
	// TheDoctor: Fix out of water climbing
	if (pml.ladder) {
		PM_LadderMove();
	} else if (pm->ps->pm_flags & PMF_TIME_WATERJUMP) {
#endif
		PM_WaterJumpMove();
	} else if ( pm->waterlevel > 1 ) {
		// swimming
		PM_WaterMove();
	} else if ( pml.walking ) {
		// walking on ground
		PM_WalkMove();
	} else {
		// airborne
		PM_AirMove();
	}

	PM_Animate();

	// set groundentity, watertype, and waterlevel
	PM_GroundTrace();
	PM_SetWaterLevel();

	// weapons
	PM_Weapon();

	// torso animation
	PM_TorsoAnimation();

	// footstep events / legs animations
	PM_Footsteps();

	// entering / leaving water splashes
	PM_WaterEvents();

	// snap some parts of playerstate to save network bandwidth
	trap_SnapVector( pm->ps->velocity );
}


/*
================
Pmove

Can be called by either the server or the client
================
*/
void Pmove (pmove_t *pmove) {
	int			finalTime;

	finalTime = pmove->cmd.serverTime;

	if ( finalTime < pmove->ps->commandTime ) {
		return;	// should not happen
	}

	if ( finalTime > pmove->ps->commandTime + 1000 ) {
		pmove->ps->commandTime = finalTime - 1000;
	}

	pmove->ps->pmove_framecount = (pmove->ps->pmove_framecount+1) & ((1<<PS_PMOVEFRAMECOUNTBITS)-1);

	// chop the move up if it is too long, to prevent framerate
	// dependent behavior
	while ( pmove->ps->commandTime != finalTime ) {
		int		msec;

		msec = finalTime - pmove->ps->commandTime;

		if ( pmove->pmove_fixed ) {
			if ( msec > pmove->pmove_msec ) {
				msec = pmove->pmove_msec;
			}
		}
		else {
			if ( msec > 66 ) {
				msec = 66;
			}
		}
		pmove->cmd.serverTime = pmove->ps->commandTime + msec;
		PmoveSingle( pmove );

		if ( pmove->ps->pm_flags & PMF_JUMP_HELD ) {
			pmove->cmd.upmove = 20;
		}
	}

	//PM_CheckStuck();

}

