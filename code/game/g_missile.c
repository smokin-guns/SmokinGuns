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

#define	MISSILE_PRESTEP_TIME	50

#ifdef SMOKINGUNS
static void G_KnifeThink( gentity_t *self ) {
	trace_t trace;
	int mask;

	if(self->clipmask)
		mask = self->clipmask;
	else
		mask = MASK_PLAYERSOLID & ~CONTENTS_BODY;

	trap_Trace(&trace, self->r.currentOrigin, self->r.mins,self->r.maxs, self->s.origin2,
		self->r.ownerNum, mask);

	if(trace.fraction == 1){
		self->s.groundEntityNum = ENTITYNUM_NONE;
	}

	if(level.time > self->wait)
		G_FreeEntity(self);

	self->nextthink = level.time + 100;
}
#endif

/*
================
G_BounceMissile

================
*/
static void G_BounceMissile( gentity_t *ent, trace_t *trace ) {
	vec3_t	velocity;
	float	dot;
	int		hitTime;

	// reflect the velocity on the trace plane
	hitTime = level.previousTime + ( level.time - level.previousTime ) * trace->fraction;
	BG_EvaluateTrajectoryDelta( &ent->s.pos, hitTime, velocity );
	dot = DotProduct( velocity, trace->plane.normal );
	VectorMA( velocity, -2*dot, trace->plane.normal, ent->s.pos.trDelta );

	if ( ent->s.eFlags & EF_BOUNCE_HALF ) {
#ifndef SMOKINGUNS
		VectorScale( ent->s.pos.trDelta, 0.65, ent->s.pos.trDelta );
#else
		VectorScale( ent->s.pos.trDelta, 0.17, ent->s.pos.trDelta );
#endif
		// check for stop
		if ( trace->plane.normal[2] > 0.2 && VectorLength( ent->s.pos.trDelta ) < 40 ) {
			G_SetOrigin( ent, trace->endpos );
			ent->s.time = level.time / 4;
#ifdef SMOKINGUNS
			if(!Q_stricmp(ent->classname, "grenadeno")){
				ent->classname = "grenadeend";
				VectorCopy(trace->endpos, ent->s.origin2);
			}
#endif
			return;
		}
	}

	VectorAdd( ent->r.currentOrigin, trace->plane.normal, ent->r.currentOrigin);
	VectorCopy( ent->r.currentOrigin, ent->s.pos.trBase );
	ent->s.pos.trTime = level.time;
}


/*
================
G_ExplodeMissile

Explode a missile without an impact
================
*/
static void G_ExplodeMissile( gentity_t *ent ) {
	vec3_t		dir;
	vec3_t		origin;

	BG_EvaluateTrajectory( &ent->s.pos, level.time, origin );
	SnapVector( origin );
	G_SetOrigin( ent, origin );

	// we don't have a valid direction, so just point straight up
	dir[0] = dir[1] = 0;
	dir[2] = 1;

	ent->s.eType = ET_GENERAL;
	G_AddEvent( ent, EV_MISSILE_MISS, DirToByte( dir ) );

	ent->freeAfterEvent = qtrue;
	ent->takedamage = qfalse;

	// splash damage
	if ( ent->splashDamage )
		G_RadiusDamage( ent->r.currentOrigin, ent->parent, ent->splashDamage,
			ent->splashRadius, ent, ent->splashMethodOfDeath );

	trap_LinkEntity( ent );
}
/*
================
G_InstantExplode

Makes a dynamite explode in the given position.
Needs an attacker, to know who caused the explosion.
================
*/
#ifdef SMOKINGUNS
void G_InstantExplode(vec3_t orig, gentity_t *attacker) {
	gentity_t *dynamite;

	//spawn a new entity
	dynamite = G_Spawn();
	dynamite->classname = "grenade";
	//change its current position
	VectorCopy(orig, dynamite->s.pos.trBase);
	VectorCopy(orig, dynamite->r.currentOrigin);
	//set its flags
	dynamite->s.eType = ET_MISSILE;
	dynamite->r.svFlags = SVF_USE_CURRENT_ORIGIN;
	dynamite->r.svFlags |= SVF_BROADCAST;
	dynamite->s.weapon = WP_DYNAMITE;
	dynamite->s.eFlags = EF_BOUNCE_HALF;
	//change the owner to the one who shot the dynamite
	dynamite->r.ownerNum = attacker->s.number;
	dynamite->parent = attacker;
	//dynamite damage
	dynamite->damage = 900 + rand()%200;
	dynamite->splashDamage = 600 + rand()%200;
	dynamite->splashRadius = 200 + rand()%50;
	dynamite->methodOfDeath = MOD_DYNAMITE;
	dynamite->splashMethodOfDeath = MOD_DYNAMITE;
	dynamite->clipmask = MASK_SHOT;
	dynamite->takedamage = qfalse;

	G_ExplodeMissile(dynamite);
}
#endif


#ifndef SMOKINGUNS
/*
================
ProximityMine_Explode
================
*/
static void ProximityMine_Explode( gentity_t *mine ) {
	G_ExplodeMissile( mine );
	// if the prox mine has a trigger free it
	if (mine->activator) {
		G_FreeEntity(mine->activator);
		mine->activator = NULL;
	}
}

/*
================
ProximityMine_Die
================
*/
static void ProximityMine_Die( gentity_t *ent, gentity_t *inflictor, gentity_t *attacker, int damage, int mod ) {
	ent->think = ProximityMine_Explode;
	ent->nextthink = level.time + 1;
}

/*
================
ProximityMine_Trigger
================
*/
void ProximityMine_Trigger( gentity_t *trigger, gentity_t *other, trace_t *trace ) {
	vec3_t		v;
	gentity_t	*mine;

	if( !other->client ) {
		return;
	}

	// trigger is a cube, do a distance test now to act as if it's a sphere
	VectorSubtract( trigger->s.pos.trBase, other->s.pos.trBase, v );
	if( VectorLength( v ) > trigger->parent->splashRadius ) {
		return;
	}


	if ( g_gametype.integer >= GT_TEAM ) {
		// don't trigger same team mines
		if (trigger->parent->s.generic1 == other->client->sess.sessionTeam) {
			return;
		}
	}

	// ok, now check for ability to damage so we don't get triggered thru walls, closed doors, etc...
	if( !CanDamage( other, trigger->s.pos.trBase ) ) {
		return;
	}

	// trigger the mine!
	mine = trigger->parent;
	mine->s.loopSound = 0;
	G_AddEvent( mine, EV_PROXIMITY_MINE_TRIGGER, 0 );
	mine->nextthink = level.time + 500;

	G_FreeEntity( trigger );
}

/*
================
ProximityMine_Activate
================
*/
static void ProximityMine_Activate( gentity_t *ent ) {
	gentity_t	*trigger;
	float		r;

	ent->think = ProximityMine_Explode;
	ent->nextthink = level.time + g_proxMineTimeout.integer;

	ent->takedamage = qtrue;
	ent->health = 1;
	ent->die = ProximityMine_Die;

	ent->s.loopSound = G_SoundIndex( "sound/weapons/proxmine/wstbtick.wav" );

	// build the proximity trigger
	trigger = G_Spawn ();

	trigger->classname = "proxmine_trigger";

	r = ent->splashRadius;
	VectorSet( trigger->r.mins, -r, -r, -r );
	VectorSet( trigger->r.maxs, r, r, r );

	G_SetOrigin( trigger, ent->s.pos.trBase );

	trigger->parent = ent;
	trigger->r.contents = CONTENTS_TRIGGER;
	trigger->touch = ProximityMine_Trigger;

	trap_LinkEntity (trigger);

	// set pointer to trigger so the entity can be freed when the mine explodes
	ent->activator = trigger;
}

/*
================
ProximityMine_ExplodeOnPlayer
================
*/
static void ProximityMine_ExplodeOnPlayer( gentity_t *mine ) {
	gentity_t	*player;

	player = mine->enemy;
	player->client->ps.eFlags &= ~EF_TICKING;

	if ( player->client->invulnerabilityTime > level.time ) {
		G_Damage( player, mine->parent, mine->parent, vec3_origin, mine->s.origin, 1000, DAMAGE_NO_KNOCKBACK, MOD_JUICED );
		player->client->invulnerabilityTime = 0;
		G_TempEntity( player->client->ps.origin, EV_JUICED );
	}
	else {
		G_SetOrigin( mine, player->s.pos.trBase );
		// make sure the explosion gets to the client
		mine->r.svFlags &= ~SVF_NOCLIENT;
		mine->splashMethodOfDeath = MOD_PROXIMITY_MINE;
		G_ExplodeMissile( mine );
	}
}

/*
================
ProximityMine_Player
================
*/
static void ProximityMine_Player( gentity_t *mine, gentity_t *player ) {
	if( mine->s.eFlags & EF_NODRAW ) {
		return;
	}

	G_AddEvent( mine, EV_PROXIMITY_MINE_STICK, 0 );

	if( player->s.eFlags & EF_TICKING ) {
		player->activator->splashDamage += mine->splashDamage;
		player->activator->splashRadius *= 1.50;
		mine->think = G_FreeEntity;
		mine->nextthink = level.time;
		return;
	}

	player->client->ps.eFlags |= EF_TICKING;
	player->activator = mine;

	mine->s.eFlags |= EF_NODRAW;
	mine->r.svFlags |= SVF_NOCLIENT;
	mine->s.pos.trType = TR_LINEAR;
	VectorClear( mine->s.pos.trDelta );

	mine->enemy = player;
	mine->think = ProximityMine_ExplodeOnPlayer;
	if ( player->client->invulnerabilityTime > level.time ) {
		mine->nextthink = level.time + 2 * 1000;
	}
	else {
		mine->nextthink = level.time + 10 * 1000;
	}
}
#endif

//////////////////////////////////////////
/*
==================================================================
//			MOLOTOV FUNCTIONS
==================================================================
*/
/////////////////////////////////////////////
#ifdef SMOKINGUNS
static void WhiskeyBurn( gentity_t *self, gentity_t *other, trace_t *trace ) {
	int		dflags;

	// it's not burning!
	if(self->s.apos.trDelta[0] == 0.0f)
		return;

	if ( !other->takedamage ) {
		return;
	}

	// player already has been burned by another flame
	if(other->timestamp > level.time)
		return;

	other->timestamp = level.time + FRAMETIME*2;

	// play sound
	G_Sound( other, CHAN_AUTO, self->noise_index );

	dflags = 0;

	G_Damage (other, NULL, self->parent, NULL, NULL, 10+rand()%6, dflags, MOD_MOLOTOV);

	if(other->client)
		other->client->ps.powerups[PW_BURNBIT] = level.time;
}

static void WhiskeyThink(gentity_t *self){

	if(self->wait <= level.time){
		G_FreeEntity(self);
		return;
	}

#define RANGE 30
	// look for other whiskey pools that could be burned
	if(self->s.apos.trDelta[0]){
		int num, i;
		int touch[MAX_GENTITIES];
		vec3_t mins, maxs;

		VectorSet(mins, -RANGE, -RANGE, -RANGE);
		VectorSet(maxs, RANGE, RANGE, RANGE);

		VectorAdd(self->r.currentOrigin, mins, mins);
		VectorAdd(self->r.currentOrigin, maxs, maxs);

		num = trap_EntitiesInBox( mins, maxs, touch, MAX_GENTITIES );

		for(i = 0; i < num; i++){
			gentity_t *hit = &g_entities[touch[i]];

			// make the dynamite explode and molotov burning
			if (hit->r.contents & CONTENTS_EXPLOSIVE && hit->takedamage)
				G_Damage (hit, self, self->parent, NULL, NULL, 100.0f, 0, MOD_MOLOTOV);
			else if (hit->r.contents & CONTENTS_FLAMMABLE && hit->takedamage)
				G_Damage (hit, self, self->parent, NULL, NULL, 10.0f, 0, MOD_MOLOTOV);
		}
	}

	self->nextthink = level.time + 100;
}

// pool doesnt die it just burns
static void WhiskeyDie ( gentity_t *self, gentity_t *inflictor,
	gentity_t *attacker, int damage, int mod ) {

	G_AddEvent( self, EV_WHISKEY_BURNS, DirToByte(self->s.angles2));
	self->s.apos.trDelta[0] = 1;

	self->wait = level.time + WHISKEY_BURNTIME;
	self->takedamage = qfalse;
}

static gentity_t *SetWhiskeyPool (gentity_t *self, vec3_t origin, vec3_t normal, qboolean fire) {
	gentity_t	*pool;

	pool = G_Spawn();
	pool->classname = "whiskey_pool";

	pool->r.svFlags = SVF_USE_CURRENT_ORIGIN;
	pool->r.ownerNum = self->s.number;
	pool->parent = self;
	pool->methodOfDeath = MOD_MOLOTOV;
	pool->splashMethodOfDeath = MOD_MOLOTOV;

	pool->s.pos.trType = TR_STATIONARY;
	pool->s.pos.trTime = level.time;		// move a bit on the very first frame
	VectorCopy( origin, pool->s.pos.trBase );
	VectorCopy (origin, pool->r.currentOrigin);

	pool->touch = WhiskeyBurn;
	pool->think = WhiskeyThink;
	pool->nextthink = level.time + 100;
	pool->r.contents |= CONTENTS_FLAMMABLE|CONTENTS_TRIGGER;

	pool->die = WhiskeyDie;
	pool->health = 1;

	if(fire) {
		pool->wait = level.time + WHISKEY_BURNTIME;
		pool->s.apos.trDelta[0] = 1;
		pool->takedamage = qfalse;
	} else {
		pool->wait = level.time + WHISKEY_SICKERTIME;
		pool->s.apos.trDelta[0] = 0;
		pool->takedamage = qtrue;
	}

	pool->noise_index = G_SoundIndex( "sound/world/electro.wav" );

#define RADIUS	30
	// set the mins maxs
	VectorSet(pool->r.mins, -RADIUS, -RADIUS, -RADIUS);
	VectorSet(pool->r.maxs, RADIUS, RADIUS, RADIUS);

	// normal(important for later marks on the ground
	VectorCopy(normal, pool->s.angles2);

	trap_LinkEntity(pool);

	return pool;
}

static void SprotzThink(gentity_t *self){
	vec3_t newOrigin, oldorg;
	trace_t trace;
	int contents;
	VectorCopy(self->s.pos.trBase, oldorg);

	// calculate new position
	BG_EvaluateTrajectory( &self->s.pos, level.time, newOrigin );

	// trace a line from previous position to new position
	trap_Trace( &trace, oldorg, NULL, NULL, newOrigin, -1, CONTENTS_SOLID );
	contents = trap_PointContents(self->r.currentOrigin, self->r.ownerNum);

	if(contents & CONTENTS_WATER){
		G_FreeEntity( self);
		return;
	}

	// still in free fall
	if ( trace.fraction == 1.0 ){
		VectorCopy(newOrigin, self->r.currentOrigin);
	} else {
		SetWhiskeyPool(self->parent, trace.endpos, trace.plane.normal,
			self->s.apos.trDelta[0]);
		G_FreeEntity(self);
		return;
	}

	self->nextthink = level.time+50;
}

static void BottleBreak( gentity_t *ent, vec3_t origin, vec3_t normal, vec3_t dirs[ALC_COUNT]){
	int i;

	for(i = 0; i < ALC_COUNT; i++){
		vec3_t dir;
		gentity_t *sprotz;

		VectorCopy(dirs[i], dir);

		// alcohol drops are client predicted
		// these are the server drops
		sprotz = G_Spawn();
		sprotz->think = SprotzThink;
		sprotz->nextthink = level.time+50;
		sprotz->s.pos.trTime = level.time;
		sprotz->parent = ent->parent;
		sprotz->classname = "sprotz";
		sprotz->r.ownerNum = ent->r.ownerNum;
		sprotz->s.apos.trDelta[0] = ent->s.apos.trDelta[0];
		sprotz->s.weapon = WP_MOLOTOV;

		BG_SetWhiskeyDrop(&sprotz->s.pos, origin, normal, dir);
		VectorCopy(sprotz->s.pos.trBase, sprotz->r.currentOrigin);
	}
}
#endif


/*
================
G_MissileImpact
================
*/
#ifndef SMOKINGUNS
void G_MissileImpact( gentity_t *ent, trace_t *trace ) {
#else
static void G_MissileImpact( gentity_t *ent, trace_t *trace, int shaderNum ) {
#endif
	gentity_t		*other;
	qboolean		hitClient = qfalse;
#ifndef SMOKINGUNS
#ifdef MISSIONPACK
	vec3_t			forward, impactpoint, bouncedir;
	int				eFlags;
#endif
#else
	qboolean		hitKnife  = qfalse;
	vec3_t			bottledirs[ALC_COUNT];
#endif
	other = &g_entities[trace->entityNum];

#ifndef SMOKINGUNS
	// check for bounce
	if ( !other->takedamage &&
		( ent->s.eFlags & ( EF_BOUNCE | EF_BOUNCE_HALF ) ) ) {
		G_BounceMissile( ent, trace );
		G_AddEvent( ent, EV_GRENADE_BOUNCE, 0 );
		return;
	}

#ifdef MISSIONPACK
	if ( other->takedamage ) {
		if ( ent->s.weapon != WP_PROX_LAUNCHER ) {
			if ( other->client && other->client->invulnerabilityTime > level.time ) {
				//
				VectorCopy( ent->s.pos.trDelta, forward );
				VectorNormalize( forward );
				if (G_InvulnerabilityEffect( other, forward, ent->s.pos.trBase, impactpoint, bouncedir )) {
					VectorCopy( bouncedir, trace->plane.normal );
					eFlags = ent->s.eFlags & EF_BOUNCE_HALF;
					ent->s.eFlags &= ~EF_BOUNCE_HALF;
					G_BounceMissile( ent, trace );
					ent->s.eFlags |= eFlags;
				}
				ent->target_ent = other;
				return;
			}
		}
	}
#endif
	// impact damage
	if (other->takedamage) {
#else
	if(other->takedamage)
		hitKnife = qtrue;

	// check for bounce
	if ( ( ent->s.eFlags & ( EF_BOUNCE | EF_BOUNCE_HALF ) ) ) {
		G_BounceMissile( ent, trace );
		return;
	}

	if (other->takedamage && ent->s.weapon != WP_DYNAMITE) {
#endif
		// FIXME: wrong damage direction?
		if ( ent->damage ) {
			vec3_t	velocity;

			if( LogAccuracyHit( other, &g_entities[ent->r.ownerNum] ) ) {
				g_entities[ent->r.ownerNum].client->accuracy_hits++;
				hitClient = qtrue;
			}
			BG_EvaluateTrajectoryDelta( &ent->s.pos, level.time, velocity );
			if ( VectorLength( velocity ) == 0 ) {
				velocity[2] = 1;	// stepped on a grenade
			}
#ifndef SMOKINGUNS
			G_Damage (other, ent, &g_entities[ent->r.ownerNum], velocity,
				ent->s.origin, ent->damage,
				0, ent->methodOfDeath);
#else

			// you can't make dynamite exploding by using a knife
			if(!(ent->s.weapon == WP_KNIFE && other->s.weapon == WP_DYNAMITE &&
				other->s.eType == ET_ITEM)){

				// prepare breakable, if not already initialized
				if(!(other->flags & FL_BREAKABLE_INIT))
					G_BreakablePrepare(other, shaderNum);

				G_Damage (other, ent, &g_entities[ent->r.ownerNum], velocity,
					ent->s.origin, ent->damage,
					0, ent->methodOfDeath);
			}
#endif
		}
	}

#ifndef SMOKINGUNS
	if( ent->s.weapon == WP_PROX_LAUNCHER ) {
		if( ent->s.pos.trType != TR_GRAVITY ) {
			return;
		}

		// if it's a player, stick it on to them (flag them and remove this entity)
		if( other->s.eType == ET_PLAYER && other->health > 0 ) {
			ProximityMine_Player( ent, other );
			return;
		}

		SnapVectorTowards( trace->endpos, ent->s.pos.trBase );
		G_SetOrigin( ent, trace->endpos );
		ent->s.pos.trType = TR_STATIONARY;
		VectorClear( ent->s.pos.trDelta );

		G_AddEvent( ent, EV_PROXIMITY_MINE_STICK, trace->surfaceFlags );

		ent->think = ProximityMine_Activate;
		ent->nextthink = level.time + 2000;

		vectoangles( trace->plane.normal, ent->s.angles );
		ent->s.angles[0] += 90;

		// link the prox mine to the other entity
		ent->enemy = other;
		ent->die = ProximityMine_Die;
		VectorCopy(trace->plane.normal, ent->movedir);
		VectorSet(ent->r.mins, -4, -4, -4);
		VectorSet(ent->r.maxs, 4, 4, 4);
		trap_LinkEntity(ent);

		return;
	}

	if (!strcmp(ent->classname, "hook")) {
		gentity_t *nent;
		vec3_t v;

		nent = G_Spawn();
		if ( other->takedamage && other->client ) {

			G_AddEvent( nent, EV_MISSILE_HIT, DirToByte( trace->plane.normal ) );
			nent->s.otherEntityNum = other->s.number;

			ent->enemy = other;

			v[0] = other->r.currentOrigin[0] + (other->r.mins[0] + other->r.maxs[0]) * 0.5;
			v[1] = other->r.currentOrigin[1] + (other->r.mins[1] + other->r.maxs[1]) * 0.5;
			v[2] = other->r.currentOrigin[2] + (other->r.mins[2] + other->r.maxs[2]) * 0.5;

			SnapVectorTowards( v, ent->s.pos.trBase );	// save net bandwidth
		} else {
			VectorCopy(trace->endpos, v);
			G_AddEvent( nent, EV_MISSILE_MISS, DirToByte( trace->plane.normal ) );
			ent->enemy = NULL;
		}

		SnapVectorTowards( v, ent->s.pos.trBase );	// save net bandwidth

		nent->freeAfterEvent = qtrue;
		// change over to a normal entity right at the point of impact
		nent->s.eType = ET_GENERAL;
		ent->s.eType = ET_GRAPPLE;

		G_SetOrigin( ent, v );
		G_SetOrigin( nent, v );

		ent->think = Weapon_HookThink;
		ent->nextthink = level.time + FRAMETIME;

		ent->parent->client->ps.pm_flags |= PMF_GRAPPLE_PULL;
		VectorCopy( ent->r.currentOrigin, ent->parent->client->ps.grapplePoint);

		trap_LinkEntity( ent );
		trap_LinkEntity( nent );

		return;
	}
#endif

	// is it cheaper in bandwidth to just remove this ent and create a new
	// one, rather than changing the missile into the explosion?

#ifndef SMOKINGUNS
	if ( other->takedamage && other->client ) {
#else
	// alcoohol impact
	if( !Q_stricmp(ent->classname, "alcohol")){
		// no event
		//G_AddEvent( ent, EV_MISSILE_ALCOHOL, DirToByte( trace->plane.normal));
	} else if( !Q_stricmp(ent->classname, "molotov")){
		// we have to launch the whiskey drops
		int i;

		// set the directions
		for(i = 0; i < ALC_COUNT; i++){
			int temp;

			VectorSet(bottledirs[i], (rand()%10)-5, (rand()%10)-5, (rand()%10)-3);

			// direction has to be exactly the same (client and server)
			temp = DirToByte(bottledirs[i]);
			ByteToDir(temp, bottledirs[i]);
		}

		// dirs
		BG_DirsToEntityState(&ent->s, bottledirs);

		// burning
		if(ent->s.apos.trDelta[0])
			G_AddEvent( ent, EV_MISSILE_FIRE, DirToByte( trace->plane.normal));
		// not burning
		else
			G_AddEvent( ent, EV_MISSILE_MISS, DirToByte( trace->plane.normal));
	} else if ( other->takedamage && other->client ) {
#endif
		G_AddEvent( ent, EV_MISSILE_HIT, DirToByte( trace->plane.normal ) );
		ent->s.otherEntityNum = other->s.number;
#ifndef SMOKINGUNS
	} else if( trace->surfaceFlags & SURF_METALSTEPS ) {
#else
	} else if( trace->surfaceFlags & SURF_METAL ) {
#endif
		G_AddEvent( ent, EV_MISSILE_MISS_METAL, DirToByte( trace->plane.normal ) );
	} else {
		G_AddEvent( ent, EV_MISSILE_MISS, DirToByte( trace->plane.normal ) );
	}

#ifndef SMOKINGUNS
	ent->freeAfterEvent = qtrue;

	// change over to a normal entity right at the point of impact
	ent->s.eType = ET_GENERAL;
#else
	if(Q_stricmp(ent->classname, "knife")){
		ent->freeAfterEvent = qtrue;
		// change over to a normal entity right at the point of impact
		ent->s.eType = ET_GENERAL;
	} else {
		vec3_t dir;
		gitem_t			*item;

		item = BG_FindItemForWeapon(WP_KNIFE);

		ent->s.modelindex = item-bg_itemlist;
		ent->s.modelindex2 = 1;

		ent->item = item;

		ent->s.eType = ET_ITEM;
		ent->s.pos.trType = TR_GRAVITY;
		ent->physicsBounce = 0.01f;
		ent->r.contents = CONTENTS_TRIGGER;

		ent->touch = Touch_Item;
		ent->nextthink = level.time + 100;
		ent->think = G_KnifeThink;
		ent->wait = level.time + 30000;
		ent->flags |= FL_THROWN_ITEM;

		vectoangles(ent->s.pos.trDelta, dir);

		VectorCopy(dir, ent->s.apos.trBase);
		VectorCopy(dir, ent->r.currentAngles);
	}
	//modified by Spoon END
#endif

	SnapVectorTowards( trace->endpos, ent->s.pos.trBase );	// save net bandwidth

	G_SetOrigin( ent, trace->endpos );

	// splash damage (doesn't apply to person directly hit)
	if ( ent->splashDamage ) {
		if( G_RadiusDamage( trace->endpos, ent->parent, ent->splashDamage, ent->splashRadius,
			other, ent->splashMethodOfDeath ) ) {
			if( !hitClient ) {
				g_entities[ent->r.ownerNum].client->accuracy_hits++;
			}
		}
	}

	// spawn alcohol missiles
#ifdef SMOKINGUNS
	if(!Q_stricmp(ent->classname, "molotov")){
		BottleBreak( ent, trace->endpos, trace->plane.normal, bottledirs);
	}
#endif

	trap_LinkEntity( ent );
}

/*
================
G_RunMissile
================
*/
void G_RunMissile( gentity_t *ent ) {
	vec3_t		origin;
	trace_t		tr;
	int			passent;
#ifdef SMOKINGUNS
	int			shaderNum;
	gentity_t	*traceEnt;
#endif

	// get current position
	BG_EvaluateTrajectory( &ent->s.pos, level.time, origin );

	// if this missile bounced off an invulnerability sphere
	if ( ent->target_ent ) {
		passent = ent->target_ent->s.number;
	}
#ifndef SMOKINGUNS
	// prox mines that left the owner bbox will attach to anything, even the owner
	else if (ent->s.weapon == WP_PROX_LAUNCHER && ent->count) {
		passent = ENTITYNUM_NONE;
	}
#endif
	else {
		// ignore interactions with the missile owner
		passent = ent->r.ownerNum;
	}
	// trace a line from the previous position to the current position
#ifndef SMOKINGUNS
	trap_Trace( &tr, ent->r.currentOrigin, ent->r.mins, ent->r.maxs, origin, passent, ent->clipmask );
#else
	shaderNum = trap_Trace_New2( &tr, ent->r.currentOrigin, ent->r.mins, ent->r.maxs, origin, passent, ent->clipmask );
	traceEnt = &g_entities[tr.entityNum];
#endif

	if ( tr.startsolid || tr.allsolid ) {
		// make sure the tr.entityNum is set to the entity we're stuck in
#ifndef SMOKINGUNS
		trap_Trace( &tr, ent->r.currentOrigin, ent->r.mins, ent->r.maxs, ent->r.currentOrigin, passent, ent->clipmask );
#else
		trap_Trace_New( &tr, ent->r.currentOrigin, ent->r.mins, ent->r.maxs, ent->r.currentOrigin, passent, ent->clipmask );
#endif
		tr.fraction = 0;
	}
	else {
		VectorCopy( tr.endpos, ent->r.currentOrigin );
	}

	trap_LinkEntity( ent );

	if ( tr.fraction != 1 ) {

#ifdef SMOKINGUNS
		VectorCopy(origin, ent->s.origin2);
#endif

		// never explode or bounce on sky
		if ( tr.surfaceFlags & SURF_NOIMPACT ) {
			// If grapple, reset owner
			if (ent->parent && ent->parent->client && ent->parent->client->hook == ent) {
				ent->parent->client->hook = NULL;
			}

			// if its a dynamite or molotov let it move 10 seconds before deleting it
#ifdef SMOKINGUNS
			if(ent->s.weapon == WP_DYNAMITE || ent->s.weapon == WP_MOLOTOV
				|| ent->s.weapon == WP_KNIFE){

				if(ent->mappart >= level.time && ent->mappart){
					goto think;
				} else if(ent->mappart){
					ent->mappart = 0;
				} else {
					ent->mappart = level.time + 5000;
					goto think;
				}
			}
#endif
			G_FreeEntity( ent );
			return;
		}
#ifndef SMOKINGUNS
		G_MissileImpact( ent, &tr );
		if ( ent->s.eType != ET_MISSILE ) {
#else
		G_MissileImpact( ent, &tr, shaderNum );
		if ( ent->s.eType != ET_MISSILE && ent->s.eType != ET_ITEM) {
#endif
			return;		// exploded
		}
	}
#ifndef SMOKINGUNS
	// if the prox mine wasn't yet outside the player body
	if (ent->s.weapon == WP_PROX_LAUNCHER && !ent->count) {
		// check if the prox mine is outside the owner bbox
		trap_Trace( &tr, ent->r.currentOrigin, ent->r.mins, ent->r.maxs, ent->r.currentOrigin, ENTITYNUM_NONE, ent->clipmask );
		if (!tr.startsolid || tr.entityNum != ent->r.ownerNum) {
			ent->count = 1;
		}
	}
#endif

think:
	// check think function after bouncing
	G_RunThink( ent );
}


//=============================================================================

#ifndef SMOKINGUNS
/*
=================
fire_plasma

=================
*/
gentity_t *fire_plasma (gentity_t *self, vec3_t start, vec3_t dir) {
	gentity_t	*bolt;

	VectorNormalize (dir);

	bolt = G_Spawn();
	bolt->classname = "plasma";
	bolt->nextthink = level.time + 10000;
	bolt->think = G_ExplodeMissile;
	bolt->s.eType = ET_MISSILE;
	bolt->r.svFlags = SVF_USE_CURRENT_ORIGIN;
	bolt->s.weapon = WP_PLASMAGUN;
	bolt->r.ownerNum = self->s.number;
	bolt->parent = self;
	bolt->damage = 20;
	bolt->splashDamage = 15;
	bolt->splashRadius = 20;
	bolt->methodOfDeath = MOD_PLASMA;
	bolt->splashMethodOfDeath = MOD_PLASMA_SPLASH;
	bolt->clipmask = MASK_SHOT;
	bolt->target_ent = NULL;

	bolt->s.pos.trType = TR_LINEAR;
	bolt->s.pos.trTime = level.time - MISSILE_PRESTEP_TIME;		// move a bit on the very first frame
	VectorCopy( start, bolt->s.pos.trBase );
	VectorScale( dir, 2000, bolt->s.pos.trDelta );
	SnapVector( bolt->s.pos.trDelta );			// save net bandwidth

	VectorCopy (start, bolt->r.currentOrigin);

	return bolt;
}

//=============================================================================


/*
=================
fire_grenade
=================
*/
gentity_t *fire_grenade (gentity_t *self, vec3_t start, vec3_t dir) {
	gentity_t	*bolt;

	VectorNormalize (dir);

	bolt = G_Spawn();
	bolt->classname = "grenade";
	bolt->nextthink = level.time + 2500;
	bolt->think = G_ExplodeMissile;
	bolt->s.eType = ET_MISSILE;
	bolt->r.svFlags = SVF_USE_CURRENT_ORIGIN;
	bolt->s.weapon = WP_GRENADE_LAUNCHER;
	bolt->s.eFlags = EF_BOUNCE_HALF;
	bolt->r.ownerNum = self->s.number;
	bolt->parent = self;
	bolt->damage = 100;
	bolt->splashDamage = 100;
	bolt->splashRadius = 150;
	bolt->methodOfDeath = MOD_GRENADE;
	bolt->splashMethodOfDeath = MOD_GRENADE_SPLASH;
	bolt->clipmask = MASK_SHOT;
	bolt->target_ent = NULL;

	bolt->s.pos.trType = TR_GRAVITY;
	bolt->s.pos.trTime = level.time - MISSILE_PRESTEP_TIME;		// move a bit on the very first frame
	VectorCopy( start, bolt->s.pos.trBase );
	VectorScale( dir, 700, bolt->s.pos.trDelta );
	SnapVector( bolt->s.pos.trDelta );			// save net bandwidth

	VectorCopy (start, bolt->r.currentOrigin);

	return bolt;
}

//=============================================================================


/*
=================
fire_bfg
=================
*/
gentity_t *fire_bfg (gentity_t *self, vec3_t start, vec3_t dir) {
	gentity_t	*bolt;

	VectorNormalize (dir);

	bolt = G_Spawn();
	bolt->classname = "bfg";
	bolt->nextthink = level.time + 10000;
	bolt->think = G_ExplodeMissile;
	bolt->s.eType = ET_MISSILE;
	bolt->r.svFlags = SVF_USE_CURRENT_ORIGIN;
	bolt->s.weapon = WP_BFG;
	bolt->r.ownerNum = self->s.number;
	bolt->parent = self;
	bolt->damage = 100;
	bolt->splashDamage = 100;
	bolt->splashRadius = 120;
	bolt->methodOfDeath = MOD_BFG;
	bolt->splashMethodOfDeath = MOD_BFG_SPLASH;
	bolt->clipmask = MASK_SHOT;
	bolt->target_ent = NULL;

	bolt->s.pos.trType = TR_LINEAR;
	bolt->s.pos.trTime = level.time - MISSILE_PRESTEP_TIME;		// move a bit on the very first frame
	VectorCopy( start, bolt->s.pos.trBase );
	VectorScale( dir, 2000, bolt->s.pos.trDelta );
	SnapVector( bolt->s.pos.trDelta );			// save net bandwidth
	VectorCopy (start, bolt->r.currentOrigin);

	return bolt;
}

//=============================================================================


/*
=================
fire_rocket
=================
*/
gentity_t *fire_rocket (gentity_t *self, vec3_t start, vec3_t dir) {
	gentity_t	*bolt;

	VectorNormalize (dir);

	bolt = G_Spawn();
	bolt->classname = "rocket";
	bolt->nextthink = level.time + 15000;
	bolt->think = G_ExplodeMissile;
	bolt->s.eType = ET_MISSILE;
	bolt->r.svFlags = SVF_USE_CURRENT_ORIGIN;
	bolt->s.weapon = WP_ROCKET_LAUNCHER;
	bolt->r.ownerNum = self->s.number;
	bolt->parent = self;
	bolt->damage = 100;
	bolt->splashDamage = 100;
	bolt->splashRadius = 120;
	bolt->methodOfDeath = MOD_ROCKET;
	bolt->splashMethodOfDeath = MOD_ROCKET_SPLASH;
	bolt->clipmask = MASK_SHOT;
	bolt->target_ent = NULL;

	bolt->s.pos.trType = TR_LINEAR;
	bolt->s.pos.trTime = level.time - MISSILE_PRESTEP_TIME;		// move a bit on the very first frame
	VectorCopy( start, bolt->s.pos.trBase );
	VectorScale( dir, 900, bolt->s.pos.trDelta );
	SnapVector( bolt->s.pos.trDelta );			// save net bandwidth
	VectorCopy (start, bolt->r.currentOrigin);

	return bolt;
}

/*
=================
fire_grapple
=================
*/
gentity_t *fire_grapple (gentity_t *self, vec3_t start, vec3_t dir) {
	gentity_t	*hook;

	VectorNormalize (dir);

	hook = G_Spawn();
	hook->classname = "hook";
	hook->nextthink = level.time + 10000;
	hook->think = Weapon_HookFree;
	hook->s.eType = ET_MISSILE;
	hook->r.svFlags = SVF_USE_CURRENT_ORIGIN;
	hook->s.weapon = WP_GRAPPLING_HOOK;
	hook->r.ownerNum = self->s.number;
	hook->methodOfDeath = MOD_GRAPPLE;
	hook->clipmask = MASK_SHOT;
	hook->parent = self;
	hook->target_ent = NULL;

	hook->s.pos.trType = TR_LINEAR;
	hook->s.pos.trTime = level.time - MISSILE_PRESTEP_TIME;		// move a bit on the very first frame
	hook->s.otherEntityNum = self->s.number; // use to match beam in client
	VectorCopy( start, hook->s.pos.trBase );
	VectorScale( dir, 800, hook->s.pos.trDelta );
	SnapVector( hook->s.pos.trDelta );			// save net bandwidth
	VectorCopy (start, hook->r.currentOrigin);

	self->client->hook = hook;

	return hook;
}


#ifdef MISSIONPACK
/*
=================
fire_nail
=================
*/
#define NAILGUN_SPREAD	500

gentity_t *fire_nail( gentity_t *self, vec3_t start, vec3_t forward, vec3_t right, vec3_t up ) {
	gentity_t	*bolt;
	vec3_t		dir;
	vec3_t		end;
	float		r, u, scale;

	bolt = G_Spawn();
	bolt->classname = "nail";
	bolt->nextthink = level.time + 10000;
	bolt->think = G_ExplodeMissile;
	bolt->s.eType = ET_MISSILE;
	bolt->r.svFlags = SVF_USE_CURRENT_ORIGIN;
	bolt->s.weapon = WP_NAILGUN;
	bolt->r.ownerNum = self->s.number;
	bolt->parent = self;
	bolt->damage = 20;
	bolt->methodOfDeath = MOD_NAIL;
	bolt->clipmask = MASK_SHOT;
	bolt->target_ent = NULL;

	bolt->s.pos.trType = TR_LINEAR;
	bolt->s.pos.trTime = level.time;
	VectorCopy( start, bolt->s.pos.trBase );

	r = random() * M_PI * 2.0f;
	u = sin(r) * crandom() * NAILGUN_SPREAD * 16;
	r = cos(r) * crandom() * NAILGUN_SPREAD * 16;
	VectorMA( start, 8192 * 16, forward, end);
	VectorMA (end, r, right, end);
	VectorMA (end, u, up, end);
	VectorSubtract( end, start, dir );
	VectorNormalize( dir );

	scale = 555 + random() * 1800;
	VectorScale( dir, scale, bolt->s.pos.trDelta );
	SnapVector( bolt->s.pos.trDelta );

	VectorCopy( start, bolt->r.currentOrigin );

	return bolt;
}


/*
=================
fire_prox
=================
*/
gentity_t *fire_prox( gentity_t *self, vec3_t start, vec3_t dir ) {
	gentity_t	*bolt;

	VectorNormalize (dir);

	bolt = G_Spawn();
	bolt->classname = "prox mine";
	bolt->nextthink = level.time + 3000;
	bolt->think = G_ExplodeMissile;
	bolt->s.eType = ET_MISSILE;
	bolt->r.svFlags = SVF_USE_CURRENT_ORIGIN;
	bolt->s.weapon = WP_PROX_LAUNCHER;
	bolt->s.eFlags = 0;
	bolt->r.ownerNum = self->s.number;
	bolt->parent = self;
	bolt->damage = 0;
	bolt->splashDamage = 100;
	bolt->splashRadius = 150;
	bolt->methodOfDeath = MOD_PROXIMITY_MINE;
	bolt->splashMethodOfDeath = MOD_PROXIMITY_MINE;
	bolt->clipmask = MASK_SHOT;
	bolt->target_ent = NULL;
	// count is used to check if the prox mine left the player bbox
	// if count == 1 then the prox mine left the player bbox and can attack to it
	bolt->count = 0;

	//FIXME: we prolly wanna abuse another field
	bolt->s.generic1 = self->client->sess.sessionTeam;

	bolt->s.pos.trType = TR_GRAVITY;
	bolt->s.pos.trTime = level.time - MISSILE_PRESTEP_TIME;		// move a bit on the very first frame
	VectorCopy( start, bolt->s.pos.trBase );
	VectorScale( dir, 700, bolt->s.pos.trDelta );
	SnapVector( bolt->s.pos.trDelta );			// save net bandwidth

	VectorCopy (start, bolt->r.currentOrigin);

	return bolt;
}
#endif

#else
static void G_Temp(gentity_t *self){
	self->nextthink = level.time + 100;
}

/*
=================
fire_knife
=================
*/
gentity_t *fire_knife (gentity_t *self, vec3_t start, vec3_t dir, int speed) {
	gentity_t	*bolt;

	VectorNormalize (dir);

	bolt = G_Spawn();
	bolt->classname = "knife";
	bolt->nextthink = level.time + 100;
	bolt->think = G_Temp;
	bolt->s.eType = ET_MISSILE;
	bolt->r.svFlags = SVF_USE_CURRENT_ORIGIN;
	bolt->s.weapon = WP_KNIFE;
	bolt->r.ownerNum = self->s.number;
//unlagged - projectile nudge
	// we'll need this for nudging projectiles later
	bolt->s.otherEntityNum = self->s.number;
//unlagged - projectile nudge
	bolt->parent = self;
	bolt->damage = 65 + rand()%30;
	bolt->splashDamage = 0;
	bolt->splashRadius = 0;
	bolt->methodOfDeath = MOD_KNIFE;
	bolt->splashMethodOfDeath = MOD_KNIFE;
	bolt->clipmask = MASK_SHOT;

	if(self->client)
		VectorCopy(self->client->ps.viewangles, bolt->s.angles2);

	bolt->s.pos.trType = TR_GRAVITY;//_LOW;
	bolt->s.pos.trTime = level.time + MISSILE_PRESTEP_TIME; //- MISSILE_PRESTEP_TIME;		// move a bit on the very first frame
	VectorCopy( start, bolt->s.pos.trBase );
	VectorScale( dir, speed, bolt->s.pos.trDelta );
	SnapVector( bolt->s.pos.trDelta );			// save net bandwidth
	VectorCopy (start, bolt->r.currentOrigin);

	return bolt;
}


/*
==========================================================================================
DYNAMITE
==========================================================================================
*/

/*
=================
G_Suck
=================
*/

static void G_Suck( gentity_t *self ) {
	int wait = self->wait;

	// if in water disable burning
	if (!Q_stricmp(self->classname, "grenadeend")){
		gitem_t *item;

		item = BG_FindItemForWeapon( WP_DYNAMITE );

		self->s.apos.trDelta[0] = 0;

		self->s.modelindex = item- bg_itemlist;
		self->s.modelindex2 = 1;

		self->item = item;

		self->s.eType = ET_ITEM;
		self->r.contents = CONTENTS_EXPLOSIVE;
		self->wait = -1;
		self->flags |= FL_THROWN_ITEM;

		self->touch = Touch_Item;
		self->classname = "grenadesit";
		self->think	= G_KnifeThink;
		self->nextthink = level.time + 100;
		self->wait = level.time + 60000;

		self->r.currentAngles[1] = rand() % 360;
		self->r.currentAngles[PITCH] = 90;
		VectorCopy(self->r.currentAngles, self->s.apos.trBase);
		return;
	}

	if(self->s.pos.trType == TR_STATIONARY){
		// just to be sure KnifeThink doesn't delete the dynamite
		self->wait = level.time + 999999;
		// runs solid checks: is dynamite hovering in the air?
		G_KnifeThink(self);
	}

	self->nextthink = level.time + 100;
	self->think = G_Suck;
	self->wait = wait;

	if (level.time > self->wait && !Q_stricmp(self->classname, "grenade"))
		G_ExplodeMissile( self );
}

/*
================
G_DynamiteDie

Spoon - Destroy the dynamite
================
*/
static void G_DynamiteDie( gentity_t *self, gentity_t *inflictor,
	gentity_t *attacker, int damage, int mod ) {

	self->takedamage = qfalse;
	self->think = G_ExplodeMissile;
	self->nextthink = level.time + 50;
	self->s.modelindex = 0;
	self->touch = 0;
	self->s.eType = ET_MISSILE;

	// change the owner to the one who shot the dynamite
	self->r.ownerNum = attacker->s.number;
	self->parent = attacker;
}

/*
=================
fire_dynamite
=================
*/

gentity_t *fire_dynamite (gentity_t *self, vec3_t start, vec3_t dir, int speed) {
	gentity_t	*bolt;

	VectorNormalize (dir);

	bolt = G_Spawn();
	bolt->classname = "grenade";

	bolt->nextthink = level.time + 100; // call G_Suck in 1/10 second
	bolt->think = G_Suck;

	// Spoon, than Tequila: any dynamite may want to die before final explosion
	bolt->health = 5;
	bolt->takedamage = qtrue;
	bolt->die = G_DynamiteDie;
	bolt->r.contents = CONTENTS_EXPLOSIVE;
	VectorSet(bolt->r.mins, -8, -8, -1);
	VectorCopy(bolt->r.mins, bolt->r.absmin);
	VectorSet(bolt->r.maxs, 8, 8, 8);
	VectorCopy(bolt->r.maxs, bolt->r.absmax);

	if (self->client ) {

		// dynamite(variable) lifetime
		if(self->client->ps.stats[STAT_WP_MODE]){
			bolt->nextthink = level.time + G_AnimLength(WP_ANIM_ALT_FIRE, WP_DYNAMITE)
				+ G_AnimLength(WP_ANIM_SPECIAL, WP_DYNAMITE) +
				self->client->ps.stats[STAT_WP_MODE];
			bolt->think = G_ExplodeMissile;
		}

		if (self->client->ps.stats[STAT_WP_MODE] >= 0){
			bolt->classname = "grenadeno";
			bolt->s.apos.trDelta[0] = 0;
		} else { //needed to check the missilesound on or off
			bolt->s.apos.trDelta[0] = bolt->nextthink;//it's burning down the house
		}
	} else {
		bolt->nextthink = level.time + 2500;
		bolt->think = G_ExplodeMissile;
	}

	bolt->s.eType = ET_MISSILE;
	bolt->r.svFlags = SVF_USE_CURRENT_ORIGIN;
	bolt->r.svFlags |= SVF_BROADCAST;
	bolt->s.weapon = WP_DYNAMITE;
	bolt->s.eFlags = EF_BOUNCE_HALF;
	bolt->r.ownerNum = self->s.number;
//unlagged - projectile nudge
	// we'll need this for nudging projectiles later
	bolt->s.otherEntityNum = self->s.number;
//unlagged - projectile nudge
	bolt->parent = self;
	bolt->damage = 900 + rand()%200;
	bolt->splashDamage = 600 + rand()%200;
	bolt->splashRadius = 200 + rand()%50;
	bolt->methodOfDeath = MOD_DYNAMITE;
	bolt->splashMethodOfDeath = MOD_DYNAMITE;
	bolt->clipmask = MASK_SHOT;

	bolt->s.pos.trType = TR_GRAVITY;
	bolt->s.pos.trTime = level.time + MISSILE_PRESTEP_TIME;		// move a bit on the very first frame
	VectorCopy( start, bolt->s.pos.trBase );
	VectorScale( dir, speed, bolt->s.pos.trDelta );
	SnapVector( bolt->s.pos.trDelta );			// save net bandwidth

	VectorCopy (start, bolt->r.currentOrigin);

	return bolt;
}

gentity_t *fire_molotov (gentity_t *self, vec3_t start, vec3_t dir, int speed) {
	gentity_t	*bolt;

	VectorNormalize (dir);

	bolt = G_Spawn();
	bolt->classname = "molotov";

	bolt->s.eType = ET_MISSILE;
	bolt->r.svFlags = SVF_USE_CURRENT_ORIGIN;
	bolt->r.svFlags |= SVF_BROADCAST;
	bolt->s.weapon = WP_MOLOTOV;
	bolt->r.ownerNum = self->s.number;
//unlagged - projectile nudge
	// we'll need this for nudging projectiles later
	bolt->s.otherEntityNum = self->s.number;
//unlagged - projectile nudge
	bolt->parent = self;
	bolt->damage = 7;
	bolt->splashDamage = 0;
	bolt->splashRadius = 0;
	bolt->methodOfDeath = MOD_MOLOTOV;
	bolt->splashMethodOfDeath = MOD_MOLOTOV;
	bolt->clipmask = MASK_SHOT;

	bolt->s.pos.trType = TR_GRAVITY;
	bolt->s.pos.trTime = level.time + MISSILE_PRESTEP_TIME;		// move a bit on the very first frame
	VectorCopy( start, bolt->s.pos.trBase );
	VectorScale( dir, speed, bolt->s.pos.trDelta );
	SnapVector( bolt->s.pos.trDelta );			// save net bandwidth

	VectorCopy (start, bolt->r.currentOrigin);

	if(self->client){
		if(self->client->ps.stats[STAT_WP_MODE] < 0)
			bolt->s.apos.trDelta[0] = self->client->ps.stats[STAT_WP_MODE];
		else
			bolt->s.apos.trDelta[0] = 0;
	}

	return bolt;
}
#endif

