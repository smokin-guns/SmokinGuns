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
// g_weapon.c
// perform the server side effects of a weapon firing

#include "g_local.h"

#ifndef SMOKINGUNS
static	float	s_quadFactor;
static	vec3_t	forward, right, up;
static	vec3_t	muzzle;
#define NUM_NAILSHOTS 15
#else
static	vec3_t	forward, right, up;
static	vec3_t	muzzle;
static	vec3_t	endpos; // used for shoot thru walls code
static	qboolean shootthru;

/*
=====================
Weapon_Trace_Bullet
by: Tequila
Dedicated trace to simplify added trace debug
It is derivated from trap_Trace_New2
======================
*/
static int traceNumber = 0 ;
static int skipNumber = 0 ;
static void Weapon_Trace_ResetDebug(int passEntityNum) {
	skipNumber = passEntityNum ;
	traceNumber = 0;
}

static int Weapon_Trace( trace_t *results, const vec3_t start, const vec3_t end, int passEntityNum ) {
	int shaderNum;
	gentity_t *tent;
	vec3_t origin;
	VectorCopy(start,origin);

	// Here is the real trace
	trap_Trace(results, start, NULL, NULL, end, passEntityNum, MASK_SHOT);

	// don't debug weapon trace if not debugging weapon
	if (g_debugWeapon.integer) {
		// Create Temporary entity to show the trace on the client side from origin
		tent = G_TempEntity( origin, EV_DEBUG_BULLET );
		// Set trace end
		VectorCopy( results->endpos, tent->s.origin2 );
		// Set trace step number
		tent->s.eventParm = traceNumber++;
	}

	if((results->contents & CONTENTS_SOLID) || (results->contents & CONTENTS_PLAYERCLIP)){
		// Spoon stuff to decompress surfaceFlags
		shaderNum = results->surfaceFlags;
		results->surfaceFlags = shaderInfo[shaderNum].surfaceFlags;
	} else
		shaderNum= -1;

	return shaderNum;
}

//#define CHECK_ENTITY_BUG
#ifdef CHECK_ENTITY_BUG
static void CheckEntityBug(const char* functag,gentity_t *ent) {
	int i;
	gentity_t *t;

	// Don't check for entity bug if not debugging weapon
	if (!g_debugWeapon.integer) {
		return;
	}

	// Tequila comment:
	// A workaround was put in weapons API in previous release (< SG 1.1 rev 301)
	// as some entities was not linked in the world for some reason
	// We may want to fix the related bug: why is an entity not linked if it's not normal ?
	for(i=0; i<MAX_CLIENTS; i++){

		if(level.clients[i].pers.connected != CON_CONNECTED)
			continue;

		if(level.clients[i].sess.sessionTeam >= TEAM_SPECTATOR)
			continue;

		t = &g_entities[i];

		if (t->r.linked) continue;

		G_Printf( S_COLOR_MAGENTA "%s, Entity BUG: " S_COLOR_YELLOW, functag);
		if (t->client)
			Com_Error(ERR_FATAL, "Client #%i not linked (netname='%s')\n", i, t->client->pers.netname);
		else
			Com_Error(ERR_FATAL, "Entity #%i not linked (classname='%s')\n", i, t->classname);
	}
}
#endif
#endif

/*
================
G_BounceProjectile
================
*/
void G_BounceProjectile( vec3_t start, vec3_t impact, vec3_t dir, vec3_t endout ) {
	vec3_t v, newv;
	float dot;

	VectorSubtract( impact, start, v );
	dot = DotProduct( v, dir );
	VectorMA( v, -2*dot, dir, newv );

	VectorNormalize(newv);
	VectorMA(impact, 8192, newv, endout);
}


/*
======================================================================

GAUNTLET(KNIFE)

======================================================================
*/

void Weapon_Gauntlet( gentity_t *ent ) {

}

#ifdef SMOKINGUNS
void weapon_knife_fire (gentity_t *ent) {
	gentity_t	*m;

	VectorNormalize( forward );

	m = fire_knife (ent, muzzle, forward, 1200);

	VectorAdd( m->s.pos.trDelta, ent->client->ps.velocity, m->s.pos.trDelta );	// "real" physics
}
#endif

/*
===============
CheckGauntletAttack
===============
*/
#ifndef SMOKINGUNS
qboolean CheckGauntletAttack( gentity_t *ent ) {
	trace_t		tr;
	vec3_t		end;
	gentity_t	*tent;
	gentity_t	*traceEnt;
	int			damage;

	// set aiming directions
	AngleVectors (ent->client->ps.viewangles, forward, right, up);

	CalcMuzzlePoint ( ent, forward, right, up, muzzle );

	VectorMA (muzzle, 32, forward, end);

	trap_Trace (&tr, muzzle, NULL, NULL, end, ent->s.number, MASK_SHOT);
	if ( tr.surfaceFlags & SURF_NOIMPACT ) {
		return qfalse;
	}

	if ( ent->client->noclip ) {
		return qfalse;
	}

	traceEnt = &g_entities[ tr.entityNum ];

	// send blood impact
	if ( traceEnt->takedamage && traceEnt->client ) {
		tent = G_TempEntity( tr.endpos, EV_MISSILE_HIT );
		tent->s.otherEntityNum = traceEnt->s.number;
		tent->s.eventParm = DirToByte( tr.plane.normal );
		tent->s.weapon = ent->s.weapon;
	}

	if ( !traceEnt->takedamage) {
		return qfalse;
	}

	if (ent->client->ps.powerups[PW_QUAD] ) {
		G_AddEvent( ent, EV_POWERUP_QUAD, 0 );
		s_quadFactor = g_quadfactor.value;
	} else {
		s_quadFactor = 1;
	}
#ifdef MISSIONPACK
	if( ent->client->persistantPowerup && ent->client->persistantPowerup->item && ent->client->persistantPowerup->item->giTag == PW_DOUBLER ) {
		s_quadFactor *= 2;
	}
#endif

	damage = 50 * s_quadFactor;
	G_Damage( traceEnt, ent, ent, forward, tr.endpos,
		damage, 0, MOD_GAUNTLET );

	return qtrue;
}
#else
/*
===============
CheckKnifeAttack
===============
*/
qboolean CheckKnifeAttack( gentity_t *ent ) {
	trace_t		tr;
	vec3_t		end;
	gentity_t	*traceEnt;
	int			damage = bg_weaponlist[WP_KNIFE].damage;
	vec3_t		mins,maxs;
	int shaderNum;
	usercmd_t *ucmd;

	if(ent->client->ps.stats[STAT_FLAGS] & SF_WP_CHOOSE){
		return qfalse;
	}

	ucmd = &ent->client->pers.cmd;
	// If reload button is pressed at the same time as the attack button,
	// disable knife action.
	// The same applies to any "talk action"
	// (say, say_team, opening console and buy choosing action)
	if (ucmd->buttons & (BUTTON_RELOAD | BUTTON_TALK))
		return qfalse;

#define BOX 20
	VectorSet(mins, -BOX, -BOX, -BOX);
	VectorSet(maxs, BOX, BOX, BOX);

	// set aiming directions
	AngleVectors (ent->client->ps.viewangles, forward, right, up);

	CalcMuzzlePoint ( ent, forward, right, up, muzzle );

	VectorMA (muzzle, 10, forward, end);

#ifdef CHECK_ENTITY_BUG
	CheckEntityBug("CheckKnifeAttack",ent);
#endif

	trap_Trace_New (&tr, muzzle, mins, maxs, end, ent->s.number, MASK_SHOT);

	traceEnt = &g_entities[ tr.entityNum ];

	// slit the wall
	if ( (tr.surfaceFlags & SURF_NOIMPACT || tr.startsolid || tr.allsolid) &&
		!traceEnt->client) {

		// do another trace
		CalcMuzzlePoint ( ent, forward, right, up, muzzle );
		VectorMA (muzzle, 32, forward, end);
		shaderNum = Weapon_Trace( &tr, muzzle, end, ent->s.number );

		traceEnt = &g_entities[ tr.entityNum ];

		if(tr.contents & CONTENTS_SOLID){
			gentity_t *tent;
			vec3_t dir;

			VectorAdd(tr.plane.normal, traceEnt->s.pos.trDelta, dir);
			VectorNormalize(dir);

			tent = G_TempEntity( tr.endpos, EV_KNIFEHIT );
			// solid was hit
			tent->s.eventParm = DirToByte( dir );
			tent->s.time2 = tr.surfaceFlags;
			tent->s.torsoAnim = shaderNum;
			tent->r.svFlags |= SVF_BROADCAST;
		}

		if(traceEnt->takedamage){
			goto wall;
		}
		return qfalse;
	}

	// check if really hit
	if ( traceEnt->takedamage && traceEnt->client ) {

		ent->client->lasthurt_part = PART_UPPER;

	}

wall:
	if ( !traceEnt->takedamage) {
		return qfalse;
	}

	if(damage){
		gentity_t *tent;
		vec3_t dir;

		VectorAdd(tr.plane.normal, traceEnt->s.pos.trDelta, dir);
		VectorNormalize(dir);

		tent = G_TempEntity( tr.endpos, EV_KNIFEHIT );
		// no wall was hit
		tent->s.torsoAnim = -1;
		tent->s.weapon = traceEnt->s.number;
		tent->s.eventParm = DirToByte( dir );
		tent->r.svFlags |= SVF_BROADCAST;

		if(tr.contents & CONTENTS_SOLID && traceEnt->s.eType == ET_BREAKABLE &&
			!(traceEnt->flags & FL_BREAKABLE_INIT)){
			shaderNum = Weapon_Trace( &tr, muzzle, end, ent->s.number );

			if(tr.entityNum == traceEnt->s.number){
				G_BreakablePrepare(traceEnt, shaderNum);
			}
		}
	}

	G_Damage( traceEnt, ent, ent, forward, tr.endpos,
		damage, 0, MOD_KNIFE );

	return qtrue;
}
#endif


/*
======================================================================

MACHINEGUN

======================================================================
*/

#ifndef SMOKINGUNS
/*
======================
SnapVectorTowards

Round a vector to integers for more efficient network
transmission, but make sure that it rounds towards a given point
rather than blindly truncating.  This prevents it from truncating
into a wall.
======================
*/
void SnapVectorTowards( vec3_t v, vec3_t to ) {
	int		i;

	for ( i = 0 ; i < 3 ; i++ ) {
		if ( to[i] <= v[i] ) {
			v[i] = floor(v[i]);
		} else {
			v[i] = ceil(v[i]);
		}
	}
}

#ifdef MISSIONPACK
#define CHAINGUN_SPREAD		600
#endif
#define MACHINEGUN_SPREAD	200
#define	MACHINEGUN_DAMAGE	7
#define	MACHINEGUN_TEAM_DAMAGE	5		// wimpier MG in teamplay

void Bullet_Fire (gentity_t *ent, float spread, int damage ) {
	trace_t		tr;
	vec3_t		end;
#ifdef MISSIONPACK
	vec3_t		impactpoint, bouncedir;
#endif
	float		r;
	float		u;
	gentity_t	*tent;
	gentity_t	*traceEnt;
	int			i, passent;

	damage *= s_quadFactor;

	r = random() * M_PI * 2.0f;
	u = sin(r) * crandom() * spread * 16;
	r = cos(r) * crandom() * spread * 16;
	VectorMA (muzzle, 8192*16, forward, end);
	VectorMA (end, r, right, end);
	VectorMA (end, u, up, end);

	passent = ent->s.number;
	for (i = 0; i < 10; i++) {

		trap_Trace (&tr, muzzle, NULL, NULL, end, passent, MASK_SHOT);
		if ( tr.surfaceFlags & SURF_NOIMPACT ) {
			return;
		}

		traceEnt = &g_entities[ tr.entityNum ];

		// snap the endpos to integers, but nudged towards the line
		SnapVectorTowards( tr.endpos, muzzle );

		// send bullet impact
		if ( traceEnt->takedamage && traceEnt->client ) {
			tent = G_TempEntity( tr.endpos, EV_BULLET_HIT_FLESH );
			tent->s.eventParm = traceEnt->s.number;
			if( LogAccuracyHit( traceEnt, ent ) ) {
				ent->client->accuracy_hits++;
			}
		} else {
			tent = G_TempEntity( tr.endpos, EV_BULLET_HIT_WALL );
			tent->s.eventParm = DirToByte( tr.plane.normal );
		}
		tent->s.otherEntityNum = ent->s.number;

		if ( traceEnt->takedamage) {
#ifdef MISSIONPACK
			if ( traceEnt->client && traceEnt->client->invulnerabilityTime > level.time ) {
				if (G_InvulnerabilityEffect( traceEnt, forward, tr.endpos, impactpoint, bouncedir )) {
					G_BounceProjectile( muzzle, impactpoint, bouncedir, end );
					VectorCopy( impactpoint, muzzle );
					// the player can hit him/herself with the bounced rail
					passent = ENTITYNUM_NONE;
				}
				else {
					VectorCopy( tr.endpos, muzzle );
					passent = traceEnt->s.number;
				}
				continue;
			}
			else {
#endif
				G_Damage( traceEnt, ent, ent, forward, tr.endpos,
					damage, 0, MOD_MACHINEGUN);
#ifdef MISSIONPACK
			}
#endif
		}
		break;
	}
}

#else
/*
=================
//modify the damage by calculating the distance
=================
*/

static float LegacyDamageReduction( float distance, float range ) {
	// Smokin'Guns 1.0 way of bullet damage calculation
	return (1/sqrt(range/distance))*2 ;
}

// Tequila: Bullet damage calculation alternative for Smokin'Guns v1.1b3
#define DAMAGE_MAXRANGE_FACTOR 10
static float AdvancedLinearDamageReduction( const float damage, const float distance, const float range ) {
	float slope;
	// Set current damage reduction to maximum damage reduction
	float max_damage_red = damage - g_bulletDamageALDRminifactor.value*damage;
	// Set damage reduction other the next range first part
	float damage_red = g_bulletDamageALDRmidpointfactor.value * max_damage_red ;

	// Check if distance is between range and g_bulletDamageALDRmidrangefactor*range
	if ( distance < g_bulletDamageALDRmidrangefactor.value * range ) {
		// Set slope for this first part as reduction by distance
		slope = damage_red / ((g_bulletDamageALDRmidrangefactor.value - 1)*range);
		// Then just apply the slope factor on the remaining distance
		return slope * (distance - range);
	}

	// Check if distance is between g_bulletDamageALDRmidrangefactor*range and
	// DAMAGE_MAXRANGE_FACTOR*range
	if ( distance < DAMAGE_MAXRANGE_FACTOR * range ) {
		// Set slope as first part damage reduction complement to maximum damage reduction
		// reduced to the second part range
		slope = (max_damage_red - damage_red)/((DAMAGE_MAXRANGE_FACTOR - g_bulletDamageALDRmidrangefactor.value) * range);
		// Current damage reduction is the reduction damage at midrange point
		// So, just add the linear reduction for that part
		damage_red += slope * (distance - g_bulletDamageALDRmidrangefactor.value*range);
		return damage_red;
	}
	// Otherwise, just return maximum damage reduction
	return max_damage_red;
}

static float Modify_BulletDamage(float damage, int weapon, vec3_t start, vec3_t end){
	float distance = Distance(start,end);
	float range = (float)bg_weaponlist[weapon].range ;
	float damage_mod = 0.0f;

	if (distance<=range) {
		return damage;
	}

	if (g_bulletDamageMode.integer == 1)
		damage_mod = AdvancedLinearDamageReduction( damage, distance, range );
	else
		damage_mod = LegacyDamageReduction( distance, range );

	if ( g_debugDamage.integer >= 2 ) {
		G_Printf(S_COLOR_CYAN "%i: distance:%.1f, range: %i, org. damage: %.1f, mod. damage: -%.1f, ",
			level.time, distance, (int)range, damage, damage_mod );
		if (g_bulletDamageMode.integer == 1)
			G_Printf("old mod. damage: -%.1f\n",
				LegacyDamageReduction( distance, range ));
		else
			G_Printf("new (1) mod. damage: -%.1f\n",
				AdvancedLinearDamageReduction( damage, distance, range ));
	}

	// Finally modify the damage
	damage -= damage_mod ;

	return damage > 0.0f ? damage : 0.0f ;
}

void CheckBulletDamage(gentity_t *ent, gentity_t *traceEnt, float damage) {
	if (!g_bulletDamageAlert.value)
		return;

	if ( !ent || !traceEnt || !traceEnt->client || !ent->base_damage)
		return;

	if (damage < 0) {
		if (ent->farshot) {
			if (ent->noalerttime<level.time) {
				//Alert client to come closer
				gentity_t *tent = G_TempEntity( vec3_origin, EV_HIT_FAR );
				tent->s.otherEntityNum = traceEnt->s.number;
				tent->r.svFlags |= SVF_SINGLECLIENT;
				tent->r.singleClient = ent->s.number;
				tent->s.angles2[0] = ent->damage_ratio;

				// Avoid to alert client too often, one second before next possible alert should be good
				ent->noalerttime = level.time + 1000;
			}
			ent->base_damage = 0.0f;
			ent->farshot = qfalse;
		}
		return;
	}

	// Check if damage was reduced, than store the reduction ratio
	if (ent->base_damage > damage) {
		float damage_red = ent->base_damage - damage;
		ent->damage_ratio = damage_red / ent->base_damage * 100;
		if (ent->damage_ratio < g_bulletDamageAlert.value)
			return;
		ent->farshot = qtrue;
	}
}

/*
=====================
G_BreakablePrepare
by: Spoon
3.8.2001

Checks if a ET_BREAKABLE was hit->save surfacetype
======================
*/
void G_BreakablePrepare(gentity_t *ent, int shaderNum){

	if(ent->s.eType != ET_BREAKABLE)
		return;

	//save surfaceFlags
	if(ent->count != -1 && shaderNum == -1)
		return;

	ent->count = shaderNum;
}

void Bullet_Fire (gentity_t *ent, float spread, float damage, const int weapon ) {
	trace_t		tr;
	vec3_t		end, tr_dir;
	float		r;
	float		u;
	gentity_t	*tent = NULL, *tent2;
	gentity_t	*traceEnt;
	int			passent;
	int			shaderNum, shaderNum2; //shaderNum2 used for shoot-thru
	int			location;
	int			shootcount = 0;

#ifdef CHECK_ENTITY_BUG
	CheckEntityBug("Bullet_Fire",ent);
#endif

	// Reset trace counter
	Weapon_Trace_ResetDebug(ent->s.number);

//unlagged - backward reconciliation #2
		// backward-reconcile the other clients
		G_DoTimeShiftFor( ent );
//unlagged - backward reconciliation #2

	//smoke puff
	if(weapon > WP_KNIFE &&
		weapon < WP_WINCHESTER66){
		tent2 = G_TempEntity( muzzle, EV_SMOKE );
		VectorScale( forward, 4096, tent2->s.origin2 );
		SnapVector( tent2->s.origin2 );
		// save client
		tent2->s.frame = ent->s.number;
	}

	r = random() * M_PI * 2.0f;
	u = sin(r) * crandom() * spread * 16;
	r = cos(r) * crandom() * spread * 16;

	// Calculate end only one time
	VectorMA (muzzle, 8192*16, forward, end);
	VectorMA (end, r, right, end);
	VectorMA (end, u, up, end);

	// Keep the direction
	VectorSubtract(end, muzzle, tr_dir);
	VectorNormalize(tr_dir);

	passent = ENTITYNUM_NONE;//you should be able to shoot your own missiles...

pistolfire:
	shootthru = qfalse;

	shaderNum = Weapon_Trace(&tr, muzzle, end, passent);

	//check if water was hit
	// check done in cgame now G_WaterWasHit(muzzle, end, passent);

	if ( tr.surfaceFlags & SURF_NOIMPACT ) {
		//NT - make sure we un-time-shift the clients
		goto untimeshift;
	}

	// Tequila: Really don't shoot ourself
	if ( tr.entityNum == ent->s.number ) {
		if (++shootcount>10){
			if (g_debugWeapon.integer>1) {
				G_Printf( S_COLOR_RED "Bullet_Fire: " S_COLOR_YELLOW
					"Trace DEBUG: Shooting only through #%i (netname='%s') !!!\n",
					ent->s.number, ent->client->pers.netname);
			}
			goto untimeshift;
		}
		// Advance a little
		VectorAdd (tr.endpos, tr_dir, muzzle);
		goto pistolfire;
	}

	traceEnt = &g_entities[ tr.entityNum ];

	if (traceEnt->takedamage){
		ent->base_damage = damage ; // Keep current base damage
		damage = Modify_BulletDamage(damage, weapon, ent->r.currentOrigin, tr.endpos);
		CheckBulletDamage(ent,traceEnt,damage);

		if (!damage) {
			CheckBulletDamage(ent,traceEnt,-1); // Look to alert attacker to come closer
			goto untimeshift;
		}

		//check if type is breakable
		G_BreakablePrepare(traceEnt, shaderNum);

		if(traceEnt->client){
			vec3_t dir;
			gclient_t	*client = traceEnt->client;
			int count = 0;

			location = G_HitModelCheck(&hit_data, traceEnt->s.pos.trBase,
				client->legs_angles, client->torso_angles, client->ps.viewangles,
				&client->torso, &client->legs, muzzle, tr.endpos);


			// check as long a model and a wall was not hit
			while(location == -1){
				count++;

				// Tequila: This case may be not reach anymore without enlarged hitbox
				if(count >= 100){
					G_Printf("Error: Too many traces\n");
					G_Printf("Error: %.1f pistol pellet damage expected on entity %d (%s)\n",
						damage, tr.entityNum, client->pers.netname );
					G_Printf("Error: start entity %d (%s) at [%.1f;%.1f;%.1f]\n",
						ent->s.number, ent->client->pers.netname,
						ent->r.currentOrigin[0], ent->r.currentOrigin[1], ent->r.currentOrigin[2]);
					G_Printf("Error: end entity %d (%s) at [%.1f;%.1f;%.1f]\n",
						tr.entityNum, client->pers.netname, tr.endpos[0], tr.endpos[1], tr.endpos[2] );
					if (count > 100)
						goto untimeshift;
				}

				//do another trace
				shaderNum = Weapon_Trace(&tr, tr.endpos, end, tr.entityNum);

				ent->base_damage = damage ; // Keep current base damage
				damage = Modify_BulletDamage(damage, weapon, ent->r.currentOrigin, tr.endpos);
				CheckBulletDamage(ent,traceEnt,damage);

				if ( damage < 0.5f ) {
					CheckBulletDamage(ent,traceEnt,-1); // Look to alert attacker to come closer
					// Tequila comment: Stop anyway when no damage can be applied
					goto untimeshift;
				}

				if ( tr.surfaceFlags & SURF_NOIMPACT ) {
					//NT - make sure we un-time-shift the clients
					goto untimeshift;
				}

				traceEnt = &g_entities[ tr.entityNum ];

				//if its an damageable entity, but no player
				if(!traceEnt->client){
					if(traceEnt->takedamage){
						G_BreakablePrepare(traceEnt, shaderNum);

						G_Damage( traceEnt, ent, ent, forward, tr.endpos,
							damage, 0, weapon);
					}
					goto wall;
				}

				client = traceEnt->client;

				location = G_HitModelCheck(&hit_data, traceEnt->s.pos.trBase,
					client->legs_angles, client->torso_angles, client->ps.viewangles,
					&client->torso, &client->legs, muzzle, tr.endpos);
			}

			// snap the endpos to integers, but nudged towards the line
			SnapVectorTowards( tr.endpos, muzzle );

			client->lasthurt_location = location;
			client->lasthurt_part = hit_info[location].hit_part;

			VectorAdd(tr.plane.normal, traceEnt->s.pos.trDelta, dir);
			VectorNormalize(dir);

			tent = G_TempEntity( tr.endpos, EV_BULLET_HIT_FLESH );
			tent->s.otherEntityNum = traceEnt->s.number;
			tent->s.eventParm = DirToByte( dir );

			G_Damage( traceEnt, ent, ent, forward, tr.endpos,
				damage, 0, weapon);
			CheckBulletDamage(ent,traceEnt,-1); // Look to alert attacker to come closer
		} else {
			G_Damage( traceEnt, ent, ent, forward, tr.endpos,
				damage, 0, weapon);
		}
	}

//mark
wall:
	// snap the endpos to integers, but nudged towards the line
	if(g_entities[tr.entityNum].s.eType != ET_BREAKABLE)
		SnapVectorTowards( tr.endpos, muzzle );

	// send bullet impact
	if(traceEnt->client && traceEnt->client->lasthurt_mod == MOD_BOILER){
		tent = G_TempEntity( tr.endpos, EV_BULLET_HIT_BOILER );
		tent->s.eventParm = DirToByte( tr.plane.normal );
	} else if ( traceEnt->takedamage && traceEnt->client ) {
	} else {

		tent = G_TempEntity( tr.endpos, EV_BULLET_HIT_WALL );
		tent->s.eventParm = DirToByte( tr.plane.normal );
		tent->s.time2 = tr.surfaceFlags;
		tent->s.otherEntityNum2 = tr.entityNum;

		// if we hit a breakable
		if(g_entities[tr.entityNum].s.eType == ET_BREAKABLE)
			tent->s.time2 |= SURF_BREAKABLE;

		tent->s.torsoAnim = shaderNum;

		// look if the weapon is able to shoot through the wall
		shootthru = BG_ShootThruWall(&damage, tr.endpos, muzzle, tr.surfaceFlags, endpos,
			trap_Trace);
	}
	// Avoid compilation warning
	if (tent) {
		tent->r.svFlags |= SVF_BROADCAST;
		tent->s.time = weapon;
		tent->s.otherEntityNum = ent->s.number;
	}

	if(shootthru && (!traceEnt->takedamage || traceEnt->health > 0)){
		// do another mark on the other side of the wall, but only if it still exists
		gentity_t *t;

		// get information of wall
		shaderNum2 = trap_Trace_New2( &tr, endpos, NULL, NULL, muzzle, passent, (MASK_SOLID|CONTENTS_BODY));

		t = G_TempEntity( tr.endpos, EV_BULLET_HIT_WALL );
		t->s.eventParm = DirToByte( tr.plane.normal );
		t->s.time2 = tr.surfaceFlags;
		t->s.torsoAnim = shaderNum2;
		t->s.otherEntityNum2 = tr.entityNum;

		// if we hit a breakable
		if(g_entities[tr.entityNum].s.eType == ET_BREAKABLE){
			t->s.time2 |= SURF_BREAKABLE;
		}

		t->r.svFlags |= SVF_BROADCAST;
		t->s.time = weapon;
		t->s.otherEntityNum = ent->s.number;
	}

	if(shootthru){
		if(++shootcount < 10){
			VectorCopy(endpos, muzzle);
			passent = tr.entityNum;
			goto pistolfire;
		}
		if (g_debugWeapon.integer>1) {
			G_Printf( S_COLOR_RED "Bullet_Fire: " S_COLOR_YELLOW
				"Trace DEBUG: Max shoot count reached\n");
		}
	}

untimeshift:
//unlagged - backward reconciliation #2
		// put them back
		G_UndoTimeShiftFor( ent );
//unlagged - backward reconciliation #2
}
#endif



/*
======================================================================

BFG

======================================================================
*/
#ifndef SMOKINGUNS
void BFG_Fire ( gentity_t *ent ) {
	gentity_t	*m;

	m = fire_bfg (ent, muzzle, forward);
	m->damage *= s_quadFactor;
	m->splashDamage *= s_quadFactor;

//	VectorAdd( m->s.pos.trDelta, ent->client->ps.velocity, m->s.pos.trDelta );	// "real" physics
}
#endif


/*
======================================================================

SHOTGUN

======================================================================
*/

// DEFAULT_SHOTGUN_SPREAD and DEFAULT_SHOTGUN_COUNT	are in bg_public.h, because
// client predicts same spreads
#define	DEFAULT_SHOTGUN_DAMAGE	10

#ifndef SMOKINGUNS
qboolean ShotgunPellet( vec3_t start, vec3_t end, gentity_t *ent ) {
	trace_t		tr;
	int			damage, i, passent;
	gentity_t	*traceEnt;
#ifdef MISSIONPACK
	vec3_t		impactpoint, bouncedir;
#endif
	vec3_t		tr_start, tr_end;

	passent = ent->s.number;
	VectorCopy( start, tr_start );
	VectorCopy( end, tr_end );
	for (i = 0; i < 10; i++) {
		trap_Trace (&tr, tr_start, NULL, NULL, tr_end, passent, MASK_SHOT);
		traceEnt = &g_entities[ tr.entityNum ];

		// send bullet impact
		if (  tr.surfaceFlags & SURF_NOIMPACT ) {
			return qfalse;
		}

		if ( traceEnt->takedamage) {
			damage = DEFAULT_SHOTGUN_DAMAGE * s_quadFactor;
#ifdef MISSIONPACK
			if ( traceEnt->client && traceEnt->client->invulnerabilityTime > level.time ) {
				if (G_InvulnerabilityEffect( traceEnt, forward, tr.endpos, impactpoint, bouncedir )) {
					G_BounceProjectile( tr_start, impactpoint, bouncedir, tr_end );
					VectorCopy( impactpoint, tr_start );
					// the player can hit him/herself with the bounced rail
					passent = ENTITYNUM_NONE;
				}
				else {
					VectorCopy( tr.endpos, tr_start );
					passent = traceEnt->s.number;
				}
				continue;
			}
			else {
				G_Damage( traceEnt, ent, ent, forward, tr.endpos,
					damage, 0, MOD_SHOTGUN);
				if( LogAccuracyHit( traceEnt, ent ) ) {
					return qtrue;
				}
			}
#else
			G_Damage( traceEnt, ent, ent, forward, tr.endpos,	damage, 0, MOD_SHOTGUN);
				if( LogAccuracyHit( traceEnt, ent ) ) {
					return qtrue;
				}
#endif
		}
		return qfalse;
	}
	return qfalse;
}
#else
qboolean ShotgunPellet( vec3_t start, vec3_t end, gentity_t *ent) {
	trace_t		tr;
	float		damage = bg_weaponlist[ent->client->ps.weapon].damage;
	int			passent;
	gentity_t	*traceEnt;
	gentity_t	*tent;
	vec3_t		tr_start, tr_end, tr_dir;
	int			shaderNum;
	int			shootcount = 0;

	passent = ENTITYNUM_NONE; // you should be able to shoot your own missile
	VectorCopy( start, tr_start );
	VectorCopy( end, tr_end );

	// Keep normalized direction to advance when shooting ourself at trace beginning
	VectorSubtract(tr_end, tr_start, tr_dir);
	VectorNormalize(tr_dir);

	// Reset trace counter
	Weapon_Trace_ResetDebug(ent->s.number);

shotgunfire:
	shootthru = qfalse;

	shaderNum = Weapon_Trace(&tr, tr_start, tr_end, passent);

	// Tequila: Really don't shoot ourself
	if ( tr.entityNum == ent->s.number ) {
		if (++shootcount>10){
			if (g_debugWeapon.integer>1) {
				G_Printf( S_COLOR_MAGENTA "ShotgunPellet: " S_COLOR_YELLOW
					"Trace DEBUG: Shooting only through #%i (netname='%s')\n",
					ent->s.number, ent->client->pers.netname);
			}
			return qfalse;
		}
		// Advance a little
		VectorAdd(tr.endpos, tr_dir, tr_start);
		goto shotgunfire;
	}

	traceEnt = &g_entities[ tr.entityNum ];

	// send bullet impact
	if (  tr.surfaceFlags & SURF_NOIMPACT ) {
		return qfalse;
	}

	if ( traceEnt->takedamage) {
		int location;
		gclient_t *client = traceEnt->client;
		float	olddamage = damage;

		ent->base_damage = damage; // Keep current base damage
		damage = Modify_BulletDamage(damage, ent->client->ps.weapon, ent->r.currentOrigin,
			tr.endpos);
		CheckBulletDamage(ent,traceEnt,damage);

		if (!damage) {
			CheckBulletDamage(ent,traceEnt,-1); // Look to alert attacker to come closer
			return qfalse;
		}

		damage = (damage+olddamage)/2;

		//if it hit a player
		if(traceEnt->client){
			int i;
			int count = 0;

			location = G_HitModelCheck(&hit_data, traceEnt->s.pos.trBase,
				client->legs_angles, client->torso_angles, client->ps.viewangles,
				&client->torso, &client->legs, muzzle, tr.endpos);

			// check as long a model was not hit or a wall was not hit
			while(location == -1){
				count++;

				// Tequila: This case may be not reach anymore without enlarged hitbox
				if(count >= 100){
					G_Printf("Error: Too many traces\n");
					G_Printf("Error: %.1f shotgun pellet damage expected on entity %d (%s)\n",
						damage, tr.entityNum, client->pers.netname );
					G_Printf("Error: start entity %d (%s) at [%.1f;%.1f;%.1f]\n",
						ent->s.number, ent->client->pers.netname,
						ent->r.currentOrigin[0], ent->r.currentOrigin[1], ent->r.currentOrigin[2]);
					G_Printf("Error: end entity %d (%s) at [%.1f;%.1f;%.1f]\n",
						tr.entityNum, client->pers.netname, tr.endpos[0], tr.endpos[1], tr.endpos[2] );
					if (count > 100)
						return qfalse;
				}

//				G_LogPrintf("shooting through a player\n");

				shaderNum = Weapon_Trace(&tr, tr.endpos, tr_end, tr.entityNum);

				ent->base_damage = damage;
				damage = Modify_BulletDamage(damage, ent->client->ps.weapon, ent->r.currentOrigin, tr.endpos);
				CheckBulletDamage(ent,traceEnt,damage);

				if ( damage < 0.5f ) {
					CheckBulletDamage(ent,traceEnt,-1); // Look to alert attacker to come closer
					// Tequila comment: Stop when no damage can be applied
					return qfalse;
				}

				if ( tr.surfaceFlags & SURF_NOIMPACT ) {
					return qfalse;
				}

				traceEnt = &g_entities[ tr.entityNum ];

				//if its an damageable entity, but no player
				if(!traceEnt->client){
					if(traceEnt->takedamage){
//						G_LogPrintf("hit something damageable\n");

						//check if type is breakable
						G_BreakablePrepare(traceEnt, shaderNum);

						G_Damage( traceEnt, ent, ent, forward, tr.endpos,
							damage, 0, ent->client->ps.weapon);
					}
					goto wall;
				}

				client = traceEnt->client;

				location = G_HitModelCheck(&hit_data, traceEnt->s.pos.trBase,
					client->legs_angles, client->torso_angles, client->ps.viewangles,
					&client->torso, &client->legs, muzzle, tr.endpos);
			}

			client->lasthurt_location = location;
			client->lasthurt_part = hit_info[location].hit_part;

			if(traceEnt->client && traceEnt->client->lasthurt_mod == MOD_BOILER){
				tent = G_TempEntity( tr.endpos, EV_BOILER_HIT );
				tent->s.eventParm = traceEnt->s.number;
				tent->r.svFlags |= SVF_BROADCAST;
			}
			ent->client->lasthurt_client = tr.entityNum;

			// don't send a hit-message for each pellet, so store 3 locations
			for(i=0;i<3;i++){
				//  this location is already recognized
				if(ent->s.angles2[i] != -1 && (
					!Q_stricmp(hit_info[location].backname,
					hit_info[(int)ent->s.angles2[i]].backname) ||
					!Q_stricmp(hit_info[location].forename,
					hit_info[(int)ent->s.angles2[i]].forename)))
					break;

				if(ent->s.angles2[i] != -1)
					continue;

				ent->s.angles2[i] = location;
				break;
			}
			ent->s.eFlags |= EF_HIT_MESSAGE;

			G_Damage( traceEnt, ent, ent, forward, tr.endpos, damage, 0, ent->client->ps.weapon);
			CheckBulletDamage(ent,traceEnt,-1); // Look to alert attacker to come closer
			return qtrue;
		}

		//check if type is breakable
		G_BreakablePrepare(traceEnt, shaderNum);

		G_Damage( traceEnt, ent, ent, forward, tr.endpos, damage, 0, ent->client->ps.weapon);

wall:
		if(traceEnt->client && traceEnt->client->lasthurt_mod == MOD_BOILER){
			tent = G_TempEntity( tr.endpos, EV_BOILER_HIT );
			tent->s.eventParm = traceEnt->s.number;
			tent->r.svFlags |= SVF_BROADCAST;
		}
	}

	if ( traceEnt->takedamage && traceEnt->client ) {
	} else { //wall was hit

#define DAM_FACTOR 0.5f
		// modify damage for a short time, cause shotguns have particles with low damage
		damage *= DAM_FACTOR;

		// look if the weapon is able to shoot through the wall
		shootthru = BG_ShootThruWall(&damage, tr.endpos, tr_start, tr.surfaceFlags, endpos,
			trap_Trace);

		damage /= DAM_FACTOR;

		if(shootthru){
//			G_LogPrintf("shooting thru wall\n");

			if(++shootcount < 10) {
				VectorCopy(endpos, tr_start);
				passent = tr.entityNum;
				goto shotgunfire;
			}
			if (g_debugWeapon.integer>1) {
				// Tequila: Show this case just in case it happens too often
				G_Printf( S_COLOR_RED "ShotgunPellet: " S_COLOR_YELLOW
					"Trace DEBUG: Max shoot count reached\n");
			}
		}
	}
	return qfalse;
}
#endif

// this should match CG_ShotgunPattern
#ifndef SMOKINGUNS
void ShotgunPattern( vec3_t origin, vec3_t origin2, int seed, gentity_t *ent ) {
	int			i;
	float		r, u;
	vec3_t		end;
	vec3_t		forward, right, up;
	qboolean	hitClient = qfalse;

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
		if( ShotgunPellet( origin, end, ent ) && !hitClient ) {
			hitClient = qtrue;
			ent->client->accuracy_hits++;
		}
	}
}
#else
int ShotgunPattern( vec3_t origin, vec3_t origin2, int seed, gentity_t *ent, qboolean altfire ) {
	int			i;
	float		r, u;
	float		spread_dist , spread_angle , angle_shift , current_angle_shift ;
	float		max_spread_circle , current_spread_circle , extra_circle = 0.0f;
	int			current_spread_cell , pellet_per_circle , extra_center_pellet , current_pellet_per_circle ;
	vec3_t		end;
	vec3_t		forward, right, up;
	int			count = bg_weaponlist[ent->client->ps.weapon].count;
	int			playerhitcount = 0;
	gentity_t	*tent;

#ifdef CHECK_ENTITY_BUG
	CheckEntityBug("ShotgunPattern",ent);
#endif

//unlagged - backward reconciliation #2
	// backward-reconcile the other clients
	G_DoTimeShiftFor( ent );
//unlagged - backward reconciliation #2

	if(altfire)
		count *= 2;

	// derive the right and up vectors from the forward vector, because
	// the client won't have any other information
	VectorNormalize2( origin2, forward );
	PerpendicularVector( right, forward );
	CrossProduct( forward, right, up );

	// clear hit-locations
	for( i=0; i<3; i++){
		ent->s.angles2[i] = -1;
	}

	if ( g_newShotgunPattern.integer ) {

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

			spread_dist = ( current_spread_circle + Q_random( &seed ) ) / max_spread_circle * bg_weaponlist[ent->client->ps.weapon].spread * 16 ;
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


			if( ShotgunPellet( origin, end, ent)){
				if ( ( i + extra_center_pellet + 1 ) < 16 )
				{
					playerhitcount |= ( 1 << ( i + extra_center_pellet + 1 ) ) ;
				}
			}

		}

		// End (Joe Kari) //

	} else {
		// generate the "random" spread pattern
		for ( i = 0 ; i < count ; i++ ) {

			r = Q_crandom( &seed ) * bg_weaponlist[ent->client->ps.weapon].spread * 16;
			u = Q_crandom( &seed ) * bg_weaponlist[ent->client->ps.weapon].spread * 16;
			VectorMA( origin, 8192 * 16, forward, end);
			VectorMA (end, r, right, end);
			VectorMA (end, u, up, end);

			if( ShotgunPellet( origin, end, ent)){
				if((i+1) < 16)
					playerhitcount |= (1 << (i+1));
			}

		}
	}

	if(ent->s.angles2[0] != -1 &&
		(ent->s.eFlags & EF_HIT_MESSAGE)){
		// send the hit message
		tent = G_TempEntity( vec3_origin, EV_HIT_MESSAGE );

		if(ent->client->lasthurt_direction == LOCATION_BACK)
			tent->s.weapon = 0;
		else
			tent->s.weapon = 1;

		tent->s.frame = -1;
		tent->s.otherEntityNum = ent->client->lasthurt_victim;
		tent->s.otherEntityNum2 = ent->s.number;
		tent->r.svFlags |= SVF_BROADCAST;
		tent->s.angles2[0] = ent->s.angles2[0];
		tent->s.angles2[1] = ent->s.angles2[1];
		tent->s.angles2[2] = ent->s.angles2[2];
	}

//unlagged - backward reconciliation #2
	// put them back
	G_UndoTimeShiftFor( ent );
//unlagged - backward reconciliation #2

	ent->s.eFlags &= ~EF_HIT_MESSAGE;

	return playerhitcount;
}
#endif


#ifndef SMOKINGUNS
void weapon_supershotgun_fire (gentity_t *ent) {
	gentity_t		*tent;
#else
void weapon_supershotgun_fire (gentity_t *ent, qboolean altfire) {
	gentity_t		*tent;
	int				temp = ent->client->lasthurt_client;
	int				playerhitcount;
#endif

	// send shotgun blast
	tent = G_TempEntity( muzzle, EV_SHOTGUN );
	VectorScale( forward, 4096, tent->s.origin2 );
	SnapVector( tent->s.origin2 );
	tent->s.eventParm = rand() & 255;		// seed for spread pattern
	tent->s.otherEntityNum = ent->s.number;

#ifndef SMOKINGUNS
	ShotgunPattern( tent->s.pos.trBase, tent->s.origin2, tent->s.eventParm, ent );
#else
	tent->s.weapon = ent->client->ps.weapon;
	tent->s.angles[0] = altfire;

//	G_LogPrintf("\nShotgun Fired Start\n");

	playerhitcount = ShotgunPattern( tent->s.pos.trBase, tent->s.origin2, tent->s.eventParm, ent, altfire );

	tent->s.clientNum = ent->client->lasthurt_client;
	tent->s.angles[1] = playerhitcount;

	ent->client->lasthurt_client = temp;

//	G_LogPrintf("Shotgun Fired End\n\n");
#endif
}


/*
======================================================================

GRENADE LAUNCHER

======================================================================
*/

#ifndef SMOKINGUNS
void weapon_grenadelauncher_fire (gentity_t *ent) {
	gentity_t	*m;

	// extra vertical velocity
	forward[2] += 0.2f;
	VectorNormalize( forward );

	m = fire_grenade (ent, muzzle, forward);
	m->damage *= s_quadFactor;
	m->splashDamage *= s_quadFactor;

//	VectorAdd( m->s.pos.trDelta, ent->client->ps.velocity, m->s.pos.trDelta );	// "real" physics
}
#else
void weapon_dynamite_fire (gentity_t *ent) {
	trace_t		tr;
	vec3_t		mins, maxs;

	// extra vertical velocity
	forward[2] += 0.1f;
	VectorNormalize( forward );

	//test if it is starting in solid
	VectorSet(mins, -8, -8 , -8);
	VectorSet(maxs, 8, 8, 8);

	do {
		trap_Trace( &tr, muzzle, mins, maxs, muzzle, -1, MASK_SOLID );

		if(tr.startsolid){
			// A solid is detected, move forward the starting vector position
			// by a unit of 0.3 until the weapon do not collide a solid
			// This happened especially when we throw the object, the back on a wall !
			VectorMA(muzzle, 0.3f, forward, muzzle);
		}

	} while(tr.startsolid);

	fire_dynamite (ent, muzzle, forward, 700);
}

/*
====================
Molotov Cocktail
====================
*/

void weapon_molotov_fire (gentity_t *ent) {
	trace_t		tr;
	vec3_t		mins, maxs;

	// extra vertical velocity
	forward[2] += 0.1f;
	VectorNormalize( forward );

	//test if it is starting in solid
	VectorSet(mins, -8, -8 , -8);
	VectorSet(maxs, 8, 8, 8);

	do {
		trap_Trace( &tr, muzzle, mins, maxs, muzzle, -1, MASK_SOLID );

		if(tr.startsolid){
			// A solid is detected, move forward the starting vector position
			// by a unit of 0.3 until the weapon do not collide a solid
			// This happened especially when we throw the object, the back on a wall !
			VectorMA(muzzle, 0.3f, forward, muzzle);
		}

	} while(tr.startsolid);

	fire_molotov (ent, muzzle, forward, 700);
}
#endif

/*
======================================================================

ROCKET

======================================================================
*/

#ifndef SMOKINGUNS
void Weapon_RocketLauncher_Fire (gentity_t *ent) {
	gentity_t	*m;

	m = fire_rocket (ent, muzzle, forward);
	m->damage *= s_quadFactor;
	m->splashDamage *= s_quadFactor;

//	VectorAdd( m->s.pos.trDelta, ent->client->ps.velocity, m->s.pos.trDelta );	// "real" physics
}
#endif


/*
======================================================================

PLASMA GUN

======================================================================
*/

#ifndef SMOKINGUNS
void Weapon_Plasmagun_Fire (gentity_t *ent) {
	gentity_t	*m;

	m = fire_plasma (ent, muzzle, forward);
	m->damage *= s_quadFactor;
	m->splashDamage *= s_quadFactor;

//	VectorAdd( m->s.pos.trDelta, ent->client->ps.velocity, m->s.pos.trDelta );	// "real" physics
}
#endif

/*
======================================================================

RAILGUN

======================================================================
*/


#ifndef SMOKINGUNS
/*
=================
weapon_railgun_fire
=================
*/
#define	MAX_RAIL_HITS	4
void weapon_railgun_fire (gentity_t *ent) {
	vec3_t		end;
#ifdef MISSIONPACK
	vec3_t impactpoint, bouncedir;
#endif
	trace_t		trace;
	gentity_t	*tent;
	gentity_t	*traceEnt;
	int			damage;
	int			i;
	int			hits;
	int			unlinked;
	int			passent;
	gentity_t	*unlinkedEntities[MAX_RAIL_HITS];

	damage = 100 * s_quadFactor;

	VectorMA (muzzle, 8192, forward, end);

	// trace only against the solids, so the railgun will go through people
	unlinked = 0;
	hits = 0;
	passent = ent->s.number;
	do {
		trap_Trace (&trace, muzzle, NULL, NULL, end, passent, MASK_SHOT );
		if ( trace.entityNum >= ENTITYNUM_MAX_NORMAL ) {
			break;
		}
		traceEnt = &g_entities[ trace.entityNum ];
		if ( traceEnt->takedamage ) {
#ifdef MISSIONPACK
			if ( traceEnt->client && traceEnt->client->invulnerabilityTime > level.time ) {
				if ( G_InvulnerabilityEffect( traceEnt, forward, trace.endpos, impactpoint, bouncedir ) ) {
					G_BounceProjectile( muzzle, impactpoint, bouncedir, end );
					// snap the endpos to integers to save net bandwidth, but nudged towards the line
					SnapVectorTowards( trace.endpos, muzzle );
					// send railgun beam effect
					tent = G_TempEntity( trace.endpos, EV_RAILTRAIL );
					// set player number for custom colors on the railtrail
					tent->s.clientNum = ent->s.clientNum;
					VectorCopy( muzzle, tent->s.origin2 );
					// move origin a bit to come closer to the drawn gun muzzle
					VectorMA( tent->s.origin2, 4, right, tent->s.origin2 );
					VectorMA( tent->s.origin2, -1, up, tent->s.origin2 );
					tent->s.eventParm = 255;	// don't make the explosion at the end
					//
					VectorCopy( impactpoint, muzzle );
					// the player can hit him/herself with the bounced rail
					passent = ENTITYNUM_NONE;
				}
			}
			else {
				if( LogAccuracyHit( traceEnt, ent ) ) {
					hits++;
				}
				G_Damage (traceEnt, ent, ent, forward, trace.endpos, damage, 0, MOD_RAILGUN);
			}
#else
				if( LogAccuracyHit( traceEnt, ent ) ) {
					hits++;
				}
				G_Damage (traceEnt, ent, ent, forward, trace.endpos, damage, 0, MOD_RAILGUN);
#endif
		}
		if ( trace.contents & CONTENTS_SOLID ) {
			break;		// we hit something solid enough to stop the beam
		}
		// unlink this entity, so the next trace will go past it
		trap_UnlinkEntity( traceEnt );
		unlinkedEntities[unlinked] = traceEnt;
		unlinked++;
	} while ( unlinked < MAX_RAIL_HITS );

	// link back in any entities we unlinked
	for ( i = 0 ; i < unlinked ; i++ ) {
		trap_LinkEntity( unlinkedEntities[i] );
	}

	// the final trace endpos will be the terminal point of the rail trail

	// snap the endpos to integers to save net bandwidth, but nudged towards the line
	SnapVectorTowards( trace.endpos, muzzle );

	// send railgun beam effect
	tent = G_TempEntity( trace.endpos, EV_RAILTRAIL );

	// set player number for custom colors on the railtrail
	tent->s.clientNum = ent->s.clientNum;

	VectorCopy( muzzle, tent->s.origin2 );
	// move origin a bit to come closer to the drawn gun muzzle
	VectorMA( tent->s.origin2, 4, right, tent->s.origin2 );
	VectorMA( tent->s.origin2, -1, up, tent->s.origin2 );

	// no explosion at end if SURF_NOIMPACT, but still make the trail
	if ( trace.surfaceFlags & SURF_NOIMPACT ) {
		tent->s.eventParm = 255;	// don't make the explosion at the end
	} else {
		tent->s.eventParm = DirToByte( trace.plane.normal );
	}
	tent->s.clientNum = ent->s.clientNum;

	// give the shooter a reward sound if they have made two railgun hits in a row
	if ( hits == 0 ) {
		// complete miss
		ent->client->accurateCount = 0;
	} else {
		// check for "impressive" reward sound
		ent->client->accurateCount += hits;
		if ( ent->client->accurateCount >= 2 ) {
			ent->client->accurateCount -= 2;
			ent->client->ps.persistant[PERS_IMPRESSIVE_COUNT]++;
			// add the sprite over the player's head
			ent->client->ps.eFlags &= ~(EF_AWARD_IMPRESSIVE | EF_AWARD_EXCELLENT | EF_AWARD_GAUNTLET | EF_AWARD_ASSIST | EF_AWARD_DEFEND | EF_AWARD_CAP );
			ent->client->ps.eFlags |= EF_AWARD_IMPRESSIVE;
			ent->client->rewardTime = level.time + REWARD_SPRITE_TIME;
		}
		ent->client->accuracy_hits++;
	}

}
#endif


/*
======================================================================

GRAPPLING HOOK

======================================================================
*/

#ifndef SMOKINGUNS
void Weapon_GrapplingHook_Fire (gentity_t *ent)
{
	if (!ent->client->fireHeld && !ent->client->hook)
		fire_grapple (ent, muzzle, forward);

	ent->client->fireHeld = qtrue;
}

void Weapon_HookFree (gentity_t *ent)
{
	ent->parent->client->hook = NULL;
	ent->parent->client->ps.pm_flags &= ~PMF_GRAPPLE_PULL;
	G_FreeEntity( ent );
}

void Weapon_HookThink (gentity_t *ent)
{
	if (ent->enemy) {
		vec3_t v, oldorigin;

		VectorCopy(ent->r.currentOrigin, oldorigin);
		v[0] = ent->enemy->r.currentOrigin[0] + (ent->enemy->r.mins[0] + ent->enemy->r.maxs[0]) * 0.5;
		v[1] = ent->enemy->r.currentOrigin[1] + (ent->enemy->r.mins[1] + ent->enemy->r.maxs[1]) * 0.5;
		v[2] = ent->enemy->r.currentOrigin[2] + (ent->enemy->r.mins[2] + ent->enemy->r.maxs[2]) * 0.5;
		SnapVectorTowards( v, oldorigin );	// save net bandwidth

		G_SetOrigin( ent, v );
	}

	VectorCopy( ent->r.currentOrigin, ent->parent->client->ps.grapplePoint);
}
#endif

/*
======================================================================

LIGHTNING GUN

======================================================================
*/

#ifndef SMOKINGUNS
void Weapon_LightningFire( gentity_t *ent ) {
	trace_t		tr;
	vec3_t		end;
#ifdef MISSIONPACK
	vec3_t impactpoint, bouncedir;
#endif
	gentity_t	*traceEnt, *tent;
	int			damage, i, passent;

	damage = 8 * s_quadFactor;

	passent = ent->s.number;
	for (i = 0; i < 10; i++) {
		VectorMA( muzzle, LIGHTNING_RANGE, forward, end );

		trap_Trace( &tr, muzzle, NULL, NULL, end, passent, MASK_SHOT );

#ifdef MISSIONPACK
		// if not the first trace (the lightning bounced of an invulnerability sphere)
		if (i) {
			// add bounced off lightning bolt temp entity
			// the first lightning bolt is a cgame only visual
			//
			tent = G_TempEntity( muzzle, EV_LIGHTNINGBOLT );
			VectorCopy( tr.endpos, end );
			SnapVector( end );
			VectorCopy( end, tent->s.origin2 );
		}
#endif
		if ( tr.entityNum == ENTITYNUM_NONE ) {
			return;
		}

		traceEnt = &g_entities[ tr.entityNum ];

		if ( traceEnt->takedamage) {
#ifdef MISSIONPACK
			if ( traceEnt->client && traceEnt->client->invulnerabilityTime > level.time ) {
				if (G_InvulnerabilityEffect( traceEnt, forward, tr.endpos, impactpoint, bouncedir )) {
					G_BounceProjectile( muzzle, impactpoint, bouncedir, end );
					VectorCopy( impactpoint, muzzle );
					VectorSubtract( end, impactpoint, forward );
					VectorNormalize(forward);
					// the player can hit him/herself with the bounced lightning
					passent = ENTITYNUM_NONE;
				}
				else {
					VectorCopy( tr.endpos, muzzle );
					passent = traceEnt->s.number;
				}
				continue;
			}
			else {
				G_Damage( traceEnt, ent, ent, forward, tr.endpos,
					damage, 0, MOD_LIGHTNING);
			}
#else
				G_Damage( traceEnt, ent, ent, forward, tr.endpos,
					damage, 0, MOD_LIGHTNING);
#endif
		}

		if ( traceEnt->takedamage && traceEnt->client ) {
			tent = G_TempEntity( tr.endpos, EV_MISSILE_HIT );
			tent->s.otherEntityNum = traceEnt->s.number;
			tent->s.eventParm = DirToByte( tr.plane.normal );
			tent->s.weapon = ent->s.weapon;
			if( LogAccuracyHit( traceEnt, ent ) ) {
				ent->client->accuracy_hits++;
			}
		} else if ( !( tr.surfaceFlags & SURF_NOIMPACT ) ) {
			tent = G_TempEntity( tr.endpos, EV_MISSILE_MISS );
			tent->s.eventParm = DirToByte( tr.plane.normal );
		}

		break;
	}
}
#endif

#ifndef SMOKINGUNS
/*
======================================================================

NAILGUN

======================================================================
*/

void Weapon_Nailgun_Fire (gentity_t *ent) {
	gentity_t	*m;
	int			count;

	for( count = 0; count < NUM_NAILSHOTS; count++ ) {
		m = fire_nail (ent, muzzle, forward, right, up );
		m->damage *= s_quadFactor;
		m->splashDamage *= s_quadFactor;
	}

//	VectorAdd( m->s.pos.trDelta, ent->client->ps.velocity, m->s.pos.trDelta );	// "real" physics
}


/*
======================================================================

PROXIMITY MINE LAUNCHER

======================================================================
*/

void weapon_proxlauncher_fire (gentity_t *ent) {
	gentity_t	*m;

	// extra vertical velocity
	forward[2] += 0.2f;
	VectorNormalize( forward );

	m = fire_prox (ent, muzzle, forward);
	m->damage *= s_quadFactor;
	m->splashDamage *= s_quadFactor;

//	VectorAdd( m->s.pos.trDelta, ent->client->ps.velocity, m->s.pos.trDelta );	// "real" physics
}

#endif

//======================================================================


/*
===============
LogAccuracyHit
===============
*/
qboolean LogAccuracyHit( gentity_t *target, gentity_t *attacker ) {
	if( !target->takedamage ) {
		return qfalse;
	}

	if ( target == attacker ) {
		return qfalse;
	}

	if( !target->client ) {
		return qfalse;
	}

	if( !attacker->client ) {
		return qfalse;
	}

	if( target->client->ps.stats[STAT_HEALTH] <= 0 ) {
		return qfalse;
	}

	if ( OnSameTeam( target, attacker ) ) {
		return qfalse;
	}

	return qtrue;
}


/*
===============
CalcMuzzlePoint

set muzzle location relative to pivoting eye
===============
*/
void CalcMuzzlePoint ( gentity_t *ent, vec3_t forward, vec3_t right, vec3_t up, vec3_t muzzlePoint ) {
#ifndef SMOKINGUNS
	VectorCopy( ent->s.pos.trBase, muzzlePoint );
#else
	VectorCopy( ent->client->ps.origin, muzzlePoint );
#endif
	muzzlePoint[2] += ent->client->ps.viewheight;
#ifndef SMOKINGUNS
	VectorMA( muzzlePoint, 14, forward, muzzlePoint );
#else

	// new eye system, makes it possible to show the legs
	G_ModifyEyeAngles(muzzlePoint, ent->client->ps.viewangles, qfalse);
#endif

	// snap to integer coordinates for more efficient network bandwidth usage
	SnapVector( muzzlePoint );
}

/*
===============
CalcMuzzlePointOrigin

set muzzle location relative to pivoting eye
===============
*/
void CalcMuzzlePointOrigin ( gentity_t *ent, vec3_t origin, vec3_t forward, vec3_t right, vec3_t up, vec3_t muzzlePoint ) {
#ifndef SMOKINGUNS
	VectorCopy( ent->s.pos.trBase, muzzlePoint );
#else
	VectorCopy( ent->client->ps.origin, muzzlePoint );
#endif
	muzzlePoint[2] += ent->client->ps.viewheight;
#ifndef SMOKINGUNS
	VectorMA( muzzlePoint, 14, forward, muzzlePoint );
#else

	// new eye system, makes it possible to show the legs
	G_ModifyEyeAngles(muzzlePoint, ent->client->ps.viewangles, qfalse);

#endif
	// snap to integer coordinates for more efficient network bandwidth usage
	SnapVector( muzzlePoint );
}



/*
===============
FireWeapon
===============
*/
#ifndef SMOKINGUNS
void FireWeapon( gentity_t *ent ) {
	if (ent->client->ps.powerups[PW_QUAD] ) {
		s_quadFactor = g_quadfactor.value;
	} else {
		s_quadFactor = 1;
	}
#ifdef MISSIONPACK
	if( ent->client->persistantPowerup && ent->client->persistantPowerup->item && ent->client->persistantPowerup->item->giTag == PW_DOUBLER ) {
		s_quadFactor *= 2;
	}
#endif

	// track shots taken for accuracy tracking.  Grapple is not a weapon and gauntet is just not tracked
	if( ent->s.weapon != WP_GRAPPLING_HOOK && ent->s.weapon != WP_GAUNTLET ) {
#ifdef MISSIONPACK
		if( ent->s.weapon == WP_NAILGUN ) {
			ent->client->accuracy_shots += NUM_NAILSHOTS;
		} else {
			ent->client->accuracy_shots++;
		}
#else
		ent->client->accuracy_shots++;
#endif
	}
#else
void FireWeapon( gentity_t *ent, qboolean altfire, int weapon ) {
	float spread = bg_weaponlist[weapon].spread;
	float damage = bg_weaponlist[weapon].damage;

	if(ent->client->movestate & MS_WALK){
		spread *= 1.4f;
	}

	if(ent->client->movestate & MS_CROUCHED) {
		spread *= 0.65f;
	} else if(ent->client->movestate & MS_JUMP) {
		spread *= 5.0f;
	}

	// adjust spread/damage for gametypes
	if(g_gametype.integer == GT_DUEL){
		if(weapon != WP_PEACEMAKER)
			spread *= 0.8f;
		if(weapon == WP_PEACEMAKER)
			damage *= 0.9f;
	} else if(weapon != WP_NONE){
		gitem_t *item = BG_FindItemForWeapon(weapon);

		spread *= 0.8f;

		if(item->weapon_sort != WS_SHOTGUN){
			spread *= 0.55f;
		}
	}

	// if playing duel, change accuracy at the beginning of the round
	if(g_gametype.integer && du_introend - DU_INTRO_DRAW - DU_INTRO_CAM <= level.time
		&& du_introend + DU_CROSSHAIR_FADE >= level.time){
		float factor = 1.0f;

		if(du_introend + DU_CROSSHAIR_START <= level.time){
			factor = (float)(level.time - du_introend - DU_CROSSHAIR_START)/DU_CROSSHAIR_FADE;
			factor = 1-factor;
		}

		spread += factor*2000;
	}
#endif

	// set aiming directions
	AngleVectors (ent->client->ps.viewangles, forward, right, up);

	CalcMuzzlePointOrigin ( ent, ent->client->oldOrigin, forward, right, up, muzzle );

	// fire the specific weapon
#ifndef SMOKINGUNS
	switch( ent->s.weapon ) {
	case WP_GAUNTLET:
		Weapon_Gauntlet( ent );
		break;
	case WP_LIGHTNING:
		Weapon_LightningFire( ent );
		break;
	case WP_SHOTGUN:
		weapon_supershotgun_fire( ent );
		break;
	case WP_MACHINEGUN:
		if ( g_gametype.integer != GT_TEAM ) {
			Bullet_Fire( ent, MACHINEGUN_SPREAD, MACHINEGUN_DAMAGE );
		} else {
			Bullet_Fire( ent, MACHINEGUN_SPREAD, MACHINEGUN_TEAM_DAMAGE );
		}
		break;
	case WP_GRENADE_LAUNCHER:
		weapon_grenadelauncher_fire( ent );
		break;
	case WP_ROCKET_LAUNCHER:
		Weapon_RocketLauncher_Fire( ent );
		break;
	case WP_PLASMAGUN:
		Weapon_Plasmagun_Fire( ent );
		break;
	case WP_RAILGUN:
		weapon_railgun_fire( ent );
		break;
	case WP_BFG:
		BFG_Fire( ent );
		break;
	case WP_GRAPPLING_HOOK:
		Weapon_GrapplingHook_Fire( ent );
		break;
#ifdef MISSIONPACK
	case WP_NAILGUN:
		Weapon_Nailgun_Fire( ent );
		break;
	case WP_PROX_LAUNCHER:
		weapon_proxlauncher_fire( ent );
		break;
	case WP_CHAINGUN:
		Bullet_Fire( ent, CHAINGUN_SPREAD, MACHINEGUN_DAMAGE );
		break;
#endif
#else
	switch( weapon ) {
	case WP_KNIFE:
		if(ent->client->ps.stats[STAT_WP_MODE])
			weapon_knife_fire( ent );
		break;
	case WP_SAWEDOFF:
	case WP_REMINGTON_GAUGE:
	case WP_WINCH97:
		weapon_supershotgun_fire( ent, altfire);
		break;
	case WP_WINCHESTER66:
	case WP_LIGHTNING:
	case WP_GATLING:
		Bullet_Fire( ent, spread, damage, weapon );
		break;
	case WP_SHARPS:
		if(ent->client->ps.powerups[PW_SCOPE] == 2 && ent->client->ps.stats[STAT_WP_MODE] != 1)
			spread *= 2;  // when scope is attached, aiming without looking through it is hard
		else if(ent->client->ps.powerups[PW_SCOPE] == 2 && ent->client->ps.stats[STAT_WP_MODE] == 1)
			spread /= 2;
		Bullet_Fire( ent, spread, damage, weapon );
		break;
	case WP_PEACEMAKER:
	case WP_REM58:
	case WP_SCHOFIELD:
		if(altfire)
			Bullet_Fire( ent, spread*3, damage, weapon );
		else
			Bullet_Fire( ent, spread, damage, weapon );
		break;
	case WP_DYNAMITE:
		weapon_dynamite_fire( ent );
		ent->client->ps.stats[STAT_WP_MODE] = 0;
		break;
	case WP_MOLOTOV:
		weapon_molotov_fire( ent);
		ent->client->ps.stats[STAT_WP_MODE] = 0;
		break;
#endif
	default:
// FIXME		G_Error( "Bad ent->s.weapon" );
		break;
	}
}

#ifndef SMOKINGUNS

/*
===============
KamikazeRadiusDamage
===============
*/
static void KamikazeRadiusDamage( vec3_t origin, gentity_t *attacker, float damage, float radius ) {
	float		dist;
	gentity_t	*ent;
	int			entityList[MAX_GENTITIES];
	int			numListedEntities;
	vec3_t		mins, maxs;
	vec3_t		v;
	vec3_t		dir;
	int			i, e;

	if ( radius < 1 ) {
		radius = 1;
	}

	for ( i = 0 ; i < 3 ; i++ ) {
		mins[i] = origin[i] - radius;
		maxs[i] = origin[i] + radius;
	}

	numListedEntities = trap_EntitiesInBox( mins, maxs, entityList, MAX_GENTITIES );

	for ( e = 0 ; e < numListedEntities ; e++ ) {
		ent = &g_entities[entityList[ e ]];

		if (!ent->takedamage) {
			continue;
		}

		// dont hit things we have already hit
		if( ent->kamikazeTime > level.time ) {
			continue;
		}

		// find the distance from the edge of the bounding box
		for ( i = 0 ; i < 3 ; i++ ) {
			if ( origin[i] < ent->r.absmin[i] ) {
				v[i] = ent->r.absmin[i] - origin[i];
			} else if ( origin[i] > ent->r.absmax[i] ) {
				v[i] = origin[i] - ent->r.absmax[i];
			} else {
				v[i] = 0;
			}
		}

		dist = VectorLength( v );
		if ( dist >= radius ) {
			continue;
		}

//		if( CanDamage (ent, origin) ) {
			VectorSubtract (ent->r.currentOrigin, origin, dir);
			// push the center of mass higher than the origin so players
			// get knocked into the air more
			dir[2] += 24;
			G_Damage( ent, NULL, attacker, dir, origin, damage, DAMAGE_RADIUS|DAMAGE_NO_TEAM_PROTECTION, MOD_KAMIKAZE );
			ent->kamikazeTime = level.time + 3000;
//		}
	}
}

/*
===============
KamikazeShockWave
===============
*/
static void KamikazeShockWave( vec3_t origin, gentity_t *attacker, float damage, float push, float radius ) {
	float		dist;
	gentity_t	*ent;
	int			entityList[MAX_GENTITIES];
	int			numListedEntities;
	vec3_t		mins, maxs;
	vec3_t		v;
	vec3_t		dir;
	int			i, e;

	if ( radius < 1 )
		radius = 1;

	for ( i = 0 ; i < 3 ; i++ ) {
		mins[i] = origin[i] - radius;
		maxs[i] = origin[i] + radius;
	}

	numListedEntities = trap_EntitiesInBox( mins, maxs, entityList, MAX_GENTITIES );

	for ( e = 0 ; e < numListedEntities ; e++ ) {
		ent = &g_entities[entityList[ e ]];

		// dont hit things we have already hit
		if( ent->kamikazeShockTime > level.time ) {
			continue;
		}

		// find the distance from the edge of the bounding box
		for ( i = 0 ; i < 3 ; i++ ) {
			if ( origin[i] < ent->r.absmin[i] ) {
				v[i] = ent->r.absmin[i] - origin[i];
			} else if ( origin[i] > ent->r.absmax[i] ) {
				v[i] = origin[i] - ent->r.absmax[i];
			} else {
				v[i] = 0;
			}
		}

		dist = VectorLength( v );
		if ( dist >= radius ) {
			continue;
		}

//		if( CanDamage (ent, origin) ) {
			VectorSubtract (ent->r.currentOrigin, origin, dir);
			dir[2] += 24;
			G_Damage( ent, NULL, attacker, dir, origin, damage, DAMAGE_RADIUS|DAMAGE_NO_TEAM_PROTECTION, MOD_KAMIKAZE );
			//
			dir[2] = 0;
			VectorNormalize(dir);
			if ( ent->client ) {
				ent->client->ps.velocity[0] = dir[0] * push;
				ent->client->ps.velocity[1] = dir[1] * push;
				ent->client->ps.velocity[2] = 100;
			}
			ent->kamikazeShockTime = level.time + 3000;
//		}
	}
}

/*
===============
KamikazeDamage
===============
*/
static void KamikazeDamage( gentity_t *self ) {
	int i;
	float t;
	gentity_t *ent;
	vec3_t newangles;

	self->count += 100;

	if (self->count >= KAMI_SHOCKWAVE_STARTTIME) {
		// shockwave push back
		t = self->count - KAMI_SHOCKWAVE_STARTTIME;
		KamikazeShockWave(self->s.pos.trBase, self->activator, 25, 400,	(int) (float) t * KAMI_SHOCKWAVE_MAXRADIUS / (KAMI_SHOCKWAVE_ENDTIME - KAMI_SHOCKWAVE_STARTTIME) );
	}
	//
	if (self->count >= KAMI_EXPLODE_STARTTIME) {
		// do our damage
		t = self->count - KAMI_EXPLODE_STARTTIME;
		KamikazeRadiusDamage( self->s.pos.trBase, self->activator, 400,	(int) (float) t * KAMI_BOOMSPHERE_MAXRADIUS / (KAMI_IMPLODE_STARTTIME - KAMI_EXPLODE_STARTTIME) );
	}

	// either cycle or kill self
	if( self->count >= KAMI_SHOCKWAVE_ENDTIME ) {
		G_FreeEntity( self );
		return;
	}
	self->nextthink = level.time + 100;

	// add earth quake effect
	newangles[0] = crandom() * 2;
	newangles[1] = crandom() * 2;
	newangles[2] = 0;
	for (i = 0; i < MAX_CLIENTS; i++)
	{
		ent = &g_entities[i];
		if (!ent->inuse)
			continue;
		if (!ent->client)
			continue;

		if (ent->client->ps.groundEntityNum != ENTITYNUM_NONE) {
			ent->client->ps.velocity[0] += crandom() * 120;
			ent->client->ps.velocity[1] += crandom() * 120;
			ent->client->ps.velocity[2] = 30 + random() * 25;
		}

		ent->client->ps.delta_angles[0] += ANGLE2SHORT(newangles[0] - self->movedir[0]);
		ent->client->ps.delta_angles[1] += ANGLE2SHORT(newangles[1] - self->movedir[1]);
		ent->client->ps.delta_angles[2] += ANGLE2SHORT(newangles[2] - self->movedir[2]);
	}
	VectorCopy(newangles, self->movedir);
}

/*
===============
G_StartKamikaze
===============
*/
void G_StartKamikaze( gentity_t *ent ) {
	gentity_t	*explosion;
	gentity_t	*te;
	vec3_t		snapped;

	// start up the explosion logic
	explosion = G_Spawn();

	explosion->s.eType = ET_EVENTS + EV_KAMIKAZE;
	explosion->eventTime = level.time;

	if ( ent->client ) {
		VectorCopy( ent->s.pos.trBase, snapped );
	}
	else {
		VectorCopy( ent->activator->s.pos.trBase, snapped );
	}
	SnapVector( snapped );		// save network bandwidth
	G_SetOrigin( explosion, snapped );

	explosion->classname = "kamikaze";
	explosion->s.pos.trType = TR_STATIONARY;

	explosion->kamikazeTime = level.time;

	explosion->think = KamikazeDamage;
	explosion->nextthink = level.time + 100;
	explosion->count = 0;
	VectorClear(explosion->movedir);

	trap_LinkEntity( explosion );

	if (ent->client) {
		//
		explosion->activator = ent;
		//
		ent->s.eFlags &= ~EF_KAMIKAZE;
		// nuke the guy that used it
		G_Damage( ent, ent, ent, NULL, NULL, 100000, DAMAGE_NO_PROTECTION, MOD_KAMIKAZE );
	}
	else {
		if ( !strcmp(ent->activator->classname, "bodyque") ) {
			explosion->activator = &g_entities[ent->activator->r.ownerNum];
		}
		else {
			explosion->activator = ent->activator;
		}
	}

	// play global sound at all clients
	te = G_TempEntity(snapped, EV_GLOBAL_TEAM_SOUND );
	te->r.svFlags |= SVF_BROADCAST;
	te->s.eventParm = GTS_KAMIKAZE;
}
#endif
