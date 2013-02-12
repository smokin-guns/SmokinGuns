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
// g_sg_utils.c -- misc utility functions for the game module
// by Spoon

#include "g_local.h"

/*
====================
G_SetItemBox
sets the exact box for each model
====================
*/
void G_SetItemBox (gentity_t *ent){
	float temp, temp2;

	if(ent->item->giType == IT_WEAPON){

		//bbox: from milkshape: x=z(milk), y=x(milk), z = y(milk)
		switch(ent->item->giTag){
		case WP_LIGHTNING:
			VectorSet(ent->r.mins, -13.14f, -0.19f, -7.73f);
			VectorSet(ent->r.maxs, 24.72f, 0.73f, -0.39f);
			ent->r.currentAngles[2] = 90;
			break;
		case WP_WINCH97:
			VectorSet(ent->r.mins, -15.36f, -2.00f, -4.97f);
			VectorSet(ent->r.maxs, 30.48f, -0.73f, 1.89f);
			ent->r.currentAngles[2] = 90;
			break;
		case WP_KNIFE:
			VectorSet(ent->r.mins, -9.25f, -1.31f, -0.31f);
			VectorSet(ent->r.maxs, 5.52f, 1.70f, 0.28f);
			//ent->r.currentAngles[0] = 90;
			break;
		case WP_PEACEMAKER:
			VectorSet(ent->r.mins, -2.22f, -2.05f, -3.27f);
			VectorSet(ent->r.maxs, 7.06f, -0.67f, 1.17f);
			ent->r.currentAngles[2] = 90;
			break;
		case WP_REMINGTON_GAUGE:
			VectorSet(ent->r.mins, -13.77f, -0.73f, -7.58f);
			VectorSet(ent->r.maxs, 24.31f, 0.98f, -1.34f);
			ent->r.currentAngles[2] = 90;
			break;
		case WP_REM58:
			VectorSet(ent->r.mins, -1.97f, -1.97f, -2.81f);
			VectorSet(ent->r.maxs, 8.20f, -0.77f, 1.02f);
			ent->r.currentAngles[2] = 90;
			break;
		case WP_SAWEDOFF:
			VectorSet(ent->r.mins, -5.13f, -0.61f, -5.25f);
			VectorSet(ent->r.maxs, 8.55f, 1.00f, -1.50f);
			ent->r.currentAngles[2] = 90;
			break;
		case WP_SCHOFIELD:
			VectorSet(ent->r.mins, -2.52f, -2.09f, -3.19f);
			VectorSet(ent->r.maxs, 8.53f, -0.75f, 1.11f);
			ent->r.currentAngles[2] = 90;
			break;
		case WP_SHARPS:
			VectorSet(ent->r.mins, -11.88f, -0.27f, -6.72f);
			VectorSet(ent->r.maxs, 22.17f, 0.73f, -0.45f);
			ent->r.currentAngles[2] = 90;
			break;
		case WP_WINCHESTER66:
			VectorSet(ent->r.mins, -14.80f, -0.36f, -7.59f);
			VectorSet(ent->r.maxs, 23.55f, 0.75f, -0.30f);
			ent->r.currentAngles[2] = 90;
			break;
		case WP_GATLING:
			VectorSet(ent->r.mins, -2.70f, -1.89f, -8.41f);
			VectorSet(ent->r.maxs, 18.73f, 3.72f, 1.69f);
			ent->r.currentAngles[2] = 90;
			break;
		case WP_MOLOTOV:
			VectorSet(ent->r.mins, -3.02f, -2.36f, -6.33f);
			VectorSet(ent->r.maxs, 1.13f, 1.89f, 7.03f);
			ent->r.currentAngles[2] = 90;
			break;
		default:
			VectorSet(ent->r.mins, 10, 10, 10);
			VectorSet(ent->r.maxs, 10, 10, 10);
			ent->r.currentAngles[2] = 90;
			break;
		}
	} else if (ent->item->giType == IT_AMMO){

		switch(ent->item->giTag){
		case WP_DYNAMITE:
			VectorSet(ent->r.mins, -12.88f, -8.91f, - 15.61f);
			VectorSet(ent->r.maxs, 13.00f,8.72f, -4.79f);
			break;
		case WP_BULLETS_CLIP:
		case WP_CART_CLIP:
			VectorSet(ent->r.mins, -4.83f, -6.32f, -13.97f);
			VectorSet(ent->r.maxs, 5.04f, 6.04f, -8.54f);
			break;
		case WP_SHELLS_CLIP:
			VectorSet(ent->r.mins, -7.38f, -6.34f, -14.89f);
			VectorSet(ent->r.maxs, 7.62f, 5.24f, -3.08f);
			break;
		default:
			VectorSet(ent->r.mins, 10, 10, 10);
			VectorSet(ent->r.maxs, 10, 10, 10);
			ent->r.currentAngles[2] = 90;
			break;
		}
	} else if (ent->item->giType == IT_POWERUP){
		switch(ent->item->giTag){
		case PW_BELT:
			VectorSet(ent->r.mins, -17.04f, -18.77f, -15.18f);
			VectorSet(ent->r.maxs, 15.20f, 7.60f, -10.96f);
			break;
		case PW_SCOPE:
			VectorSet(ent->r.mins, -13.75f, -1.91f, -0.34f);
			VectorSet(ent->r.maxs, 49.72f, 1.09f, 2.88f);
			break;
		case PW_GOLD:
			VectorSet(ent->r.mins, -20, -20, -4);
			VectorSet(ent->r.maxs, 20, 20, 21.63f);
			break;
		default:
			VectorSet(ent->r.mins, 10, 10, 10);
			VectorSet(ent->r.maxs, 10, 10, 10);
			break;
		}

	} else if (!Q_stricmp(ent->item->classname, "item_boiler_plate")){

		VectorSet(ent->r.mins, -23.66f, -16.03f, -14.53f);
		VectorSet(ent->r.maxs, 13.42f, 16.06f,-13.20f);

	} else {
		VectorSet(ent->r.mins, 10, 10, 10);
		VectorSet(ent->r.maxs, 10, 10, 10);
	}

	if(ent->r.currentAngles[2] == 90){

		temp = ent->r.mins[2];
		ent->r.mins[2] = ent->r.mins[1];

		temp2 = ent->r.maxs[1];
		ent->r.maxs[1] = -temp;

		temp = ent->r.maxs[2];
		ent->r.maxs[2] = temp2;

		ent->r.mins[1] = -temp;
	} else if(ent->r.currentAngles[0] == 90){

		temp = ent->r.mins[2];
		ent->r.mins[2] = -ent->r.maxs[0];

		temp2 = ent->r.mins[0];
		ent->r.mins[0] = temp;

		temp = ent->r.maxs[2];
		ent->r.maxs[2] = -temp2;

		ent->r.maxs[0] = temp;
	}
}
/*
=============
G_ThrowWeapon

Spoon
=============
*/

void G_ThrowWeapon( int weapon, gentity_t *ent )
{
	playerState_t	*ps;
	gitem_t			*item;
	gentity_t		*drop;

	ps = &ent->client->ps;

	if(!(ps->stats[STAT_WEAPONS] & (1 << weapon)))
		return;

	if(weapon == WP_NONE)
		return;

	item = BG_FindItemForWeapon( weapon );

	drop = G_dropWeapon( ent, item, 0, FL_DROPPED_ITEM | FL_THROWN_ITEM );

	if(drop){
		trap_SendServerCommand( ent-g_entities, va("print \"%s dropped.\n\"", item->pickup_name));
	}

	// delete weapon from inventory
	if(bg_weaponlist[weapon].wp_sort == WPS_PISTOL &&
		(ps->stats[STAT_FLAGS] & SF_SEC_PISTOL)){

		if (ps->ammo[weapon] < ps->ammo[WP_AKIMBO]) {
			int ammo = ps->ammo[weapon];
			ps->ammo[weapon] = ps->ammo[WP_AKIMBO];
			ps->ammo[WP_AKIMBO] = ammo;
		}

		if(drop)
			drop->count= ps->ammo[ WP_AKIMBO ]+1;

		ps->ammo[ WP_AKIMBO] = 0;
		ps->stats[STAT_FLAGS] &= ~SF_SEC_PISTOL;
	} else {

		if(drop)
			drop->count= ps->ammo[ weapon ]+1;

		ps->ammo[ weapon ] = 0;
		ps->stats[STAT_WEAPONS] &= ~( 1 << weapon );
	}
}


/*
========================
LetGoOfGatling

Make a player let go of the deployed gatling he's using.
========================
*/
static void LetGoOfGatling(gclient_t *client, gentity_t *gatling) {
	// add the ammo into the gatling
	gatling->count = client->ps.ammo[WP_GATLING];
	client->ps.weaponTime = 0;
	client->ps.eFlags &= ~EF_RELOAD;
	client->ps.stats[STAT_GATLING_MODE] = 0;

	// only do that if player doesn't carry another gatling
	if(!(client->ps.stats[STAT_FLAGS] & SF_GAT_CARRY)) {
		client->ps.stats[STAT_WEAPONS] &= ~(1 << WP_GATLING);
		client->ps.ammo[WP_GATLING] = 0;
	}
	else {
		client->ps.ammo[WP_GATLING] = client->carriedGatlingAmmo;
	}

	if(!client->ps.stats[STAT_OLDWEAPON] ||
			  (client->ps.stats[STAT_OLDWEAPON] == WP_GATLING &&
			  !(client->ps.stats[STAT_FLAGS] & SF_GAT_CARRY))) {
		int i;

		for ( i = WP_GATLING ; i > 0 ; i-- ) {
			if ( client->ps.stats[STAT_WEAPONS] & ( 1 << i ) ) {
				client->ps.stats[STAT_OLDWEAPON] = i;
				break;
			}
		}

		//G_Printf("away %i\n", client->ps.stats[STAT_OLDWEAPON]);
	}

	client->pers.cmd.weapon = client->ps.stats[STAT_OLDWEAPON];
	G_AddEvent(&g_entities[gatling->s.eventParm], EV_CHANGE_TO_WEAPON, client->ps.stats[STAT_OLDWEAPON]);

	gatling->s.eventParm = -1;

	// Tequila comment: Gatling is now an object in the world
	gatling->r.contents = MASK_SHOT;
}


/*
========================
Gatling_Think
by Spoon
========================
*/
static void Gatling_Think( gentity_t *self) {
	gclient_t *client;
	vec3_t end;
	trace_t trace;
	
	// Tequila: Fixed gcc warning about maybe uninitialized use of client
	client = self->client;

	if(self->s.eventParm != -1){
		client = g_entities[self->s.eventParm].client;
		// store oldbuttons
		self->mapparttime = self->mappart;
		self->mappart = client->buttons;
	} else {
		self->mappart = 0;
	}

	// if deployed on a breakable surface that breaks, the gatling falls down
	if (!self->s.time && self->s.pos.trType == TR_STATIONARY) {
		VectorCopy(self->r.currentOrigin, end);
		end[2] -= 40;
		trap_Trace(&trace, self->r.currentOrigin, NULL, NULL, end, self->s.eventParm, MASK_SHOT);

		if (trace.fraction == 1.0) {  // the ground is gone
			gitem_t *item;
			vec3_t angles, velocity, dir;
			gentity_t *ent;

			if (self->s.eventParm != -1) {
				ent = &g_entities[self->s.eventParm];
				LetGoOfGatling(client, self);
			} else {
				ent = self;
			}

			VectorCopy(ent->s.apos.trBase, angles);
			angles[PITCH] = 0;  // always forward

			AngleVectors(angles, velocity, NULL, NULL);
			VectorScale(velocity, 50, velocity);
			velocity[2] += 10;
			VectorNormalize(velocity);

			if (ent != self) {
				// drop away from client, so he won't just pick it up in the fall
				VectorClear(dir);
				AngleVectors(angles, dir, NULL, NULL);
				VectorScale(dir, 50, dir);
				VectorAdd(ent->s.pos.trBase, dir, dir);
				// snap to integer coordinates for more efficient network bandwidth usage
				SnapVector(dir);
			} else {
				VectorCopy(self->s.pos.trBase, dir);
			}

			item = BG_FindItemForWeapon(WP_GATLING);
			LaunchItem(item, dir, velocity, FL_DROPPED_ITEM);
			G_FreeEntity(self);
			return;
		}
	}

	// gatling is being built up
	if(client && self->s.time2 > 0){

		if(client->ps.stats[STAT_HEALTH] <= 0){
			gitem_t *item = BG_FindItemForWeapon(WP_GATLING);

			G_dropWeapon( self, item, 0, FL_DROPPED_ITEM | FL_THROWN_ITEM );
			G_FreeEntity(self);
			return;
		}

		if(self->s.time2 < level.time){
			self->s.time2 = 0;

			VectorCopy(gatling_mins, self->r.mins);
			VectorCopy(gatling_maxs, self->r.maxs);
			// Set mins/maxs for clientside CG_ScanForCrosshairEntity
			VectorCopy(self->r.mins, self->s.origin);
			VectorCopy(self->r.maxs, self->s.origin2);
			trap_LinkEntity(self);
		}
		self->nextthink = level.time;
		return;
	}

	// gatling is being dismantled
	if(client && self->s.time2 < 0){

		if(client->ps.stats[STAT_HEALTH] <= 0){
			//abort
			self->s.time2 = 0;
			self->s.eventParm = -1;
			self->nextthink = level.time;
			client->ps.stats[STAT_FLAGS] &= ~SF_GAT_CHANGE;
			return;
		}

		// start WP_ANIM_CHANGE
		if(self->s.time2 - TRIPOD_TIME - 4*STAGE_TIME +
			G_AnimLength(WP_ANIM_CHANGE, WP_GATLING) > -level.time &&
			!(client->ps.stats[STAT_FLAGS] & SF_GAT_CHANGE)) {

			client->ps.weaponAnim = WP_ANIM_CHANGE;
			client->ps.weaponTime += G_AnimLength(WP_ANIM_CHANGE, WP_GATLING);
			client->ps.stats[STAT_FLAGS] |= SF_GAT_CHANGE;
		}

		if(self->s.time2 - TRIPOD_TIME - 4*STAGE_TIME > -level.time){
			client->ps.stats[STAT_GATLING_MODE] = 0;
			client->ps.stats[STAT_OLDWEAPON] = 0;
			client->ps.stats[STAT_FLAGS] &= ~SF_GAT_CHANGE;
			self->s.eventParm = -1;
			G_FreeEntity(self);
			return;
		} else {
			self->nextthink = level.time;
		}
		return;
	}

	// player controls the gatling
	if(self->s.eventParm != -1){

		//change the contents
		self->r.contents = CONTENTS_CORPSE;

		//pick gatling up
		if(client->buttons & BUTTON_ALT_ATTACK && !(self->mapparttime & BUTTON_ALT_ATTACK)){
			// don't dismantle if player already has a special weapon
			if(BG_FindPlayerWeapon(WP_WINCHESTER66, WP_GATLING, &client->ps) ||
				(client->ps.stats[STAT_FLAGS] & SF_GAT_CARRY)){
				trap_SendServerCommand( client->ps.clientNum, va("print \"You already have a secondary weapon.\n\""));
			} else {
				int userNum = self->s.eventParm; // save eventparm, because its
												// getting lost when sending the event

				client->ps.weaponTime = TRIPOD_TIME + 4*STAGE_TIME;
				G_AddEvent( self, EV_DISM_TURRET, 0 );
				self->s.time2 = -level.time;
				self->s.eventParm = userNum;
			}
		}

		//get away from gatling is dead or moving, but not when crouching
		else if(client->ps.stats[STAT_HEALTH] <= 0 ||
			fabs(client->pers.cmd.forwardmove) > 40 ||
			fabs(client->pers.cmd.rightmove) > 40 ||
			client->pers.cmd.upmove){
			LetGoOfGatling(client, self);
		}
	} else {
		trace_t trace;

		trap_Trace(&trace, self->r.currentOrigin, gatling_mins, gatling_maxs,
			self->r.currentOrigin, self->s.number, CONTENTS_BODY);

		if((trace.allsolid || trace.startsolid) && trace.entityNum < MAX_CLIENTS){
			// Tequila comment: Don't stuck a client playing with another gatling
			// too next from this one
			self->r.contents = CONTENTS_CORPSE;
		} else {
			self->r.contents = MASK_SHOT;
		}
	}
	self->nextthink = level.time;
	trap_LinkEntity(self);
}

/*
========================
LaunchGatling
by Spoon
========================
*/
gentity_t *LaunchGatling( gentity_t *ent ) {
	gentity_t	*gatling;
	//vec3_t	velocity;
	vec3_t	angles, origin;

	// set aiming origin
	VectorCopy( ent->client->ps.viewangles, angles );
	angles[ROLL] = 0;
	angles[PITCH] = 0;	// always forward

	VectorClear(origin);
	AngleVectors( angles, origin, NULL, NULL);
	VectorScale(origin,30,origin);
	VectorAdd(ent->s.pos.trBase, origin, origin);
	//origin[2] -= 10;

	// snap to integer coordinates for more efficient network bandwidth usage
	SnapVector( origin);

//spawning of the gatling

	gatling = G_Spawn();

	gatling->s.eType = ET_TURRET;
	gatling->classname = "gatling";

	gatling->touch = Touch_Item;

	G_SetOrigin( gatling, origin );
	gatling->s.pos.trType = TR_GRAVITY;
	gatling->s.pos.trTime = level.time;
//	VectorCopy( velocity, gatling->s.pos.trDelta );

	gatling->s.eFlags |= EF_BOUNCE_HALF;

	VectorCopy(angles, gatling->r.currentAngles);
	VectorCopy(gatling->r.currentAngles, gatling->s.apos.trBase);

	gatling->clipmask = MASK_SHOT;
	gatling->r.contents = 0; // will be set to MASK_SHOT if no player controls it
	//gatling->r.ownerNum = ent->s.number;
	//gatling->parent = ent;
	gatling->s.pos.trTime = level.time - 50;	// move a bit on the very first frame
	//VectorScale( velocity, 200, gatling->s.pos.trDelta ); // 700
	SnapVector( gatling->s.pos.trDelta );		// save net bandwidth
	gatling->physicsBounce= 0.05f;
	gatling->physicsObject = qtrue;

	//marks object as unused
	gatling->s.eventParm = -1;
	gatling->r.svFlags |= SVF_BROADCAST;

	//object thinks
	gatling->think = Gatling_Think;
	gatling->nextthink = level.time + 100;

	//add ammo into it
	gatling->count = ent->client->ps.ammo[WP_GATLING];
	// save ammo count in case player also has a gatling in his inventory
	ent->client->carriedGatlingAmmo = ent->client->ps.ammo[WP_GATLING];
	ent->client->ps.ammo[WP_GATLING] = 0;

	//build up anims
	gatling->s.time2 = level.time + TRIPOD_TIME + 4*STAGE_TIME;

	trap_LinkEntity (gatling);

	return gatling;
}

/*
=============
G_GatlingBuildUp

Spoon
=============
*/
#define CHECK_PLANAR 0.95f
void G_GatlingBuildUp( gentity_t *ent ) {
	gclient_t	*client;
	gentity_t	*gatling;

	client = ent->client;

	// check if gatling can be built up on this terrain (must be almost planar)
	/*if(!BG_PlanarCheck(ent->r.currentOrigin, client->ps.clientNum,
		ent->r.mins, ent->r.maxs, trap_Trace) ){
		return;
	}*/ // planar check is done in pmove now

	// build it up
	//trap_SendServerCommand( ent-g_entities, va("print \"Gatling Gun building up...\n\""));

	gatling = LaunchGatling( ent );

	// let gatling gun know, by whom it is used
	gatling->s.eventParm = client->ps.clientNum;
	// the guy who built up the gatling: important for rtp-respawn system(with weapons)
	gatling->s.torsoAnim = client->ps.clientNum;

	// player uses gatling gun
	client->ps.stats[STAT_GATLING_MODE] = 1;
	VectorCopy(gatling->r.currentOrigin, client->ps.grapplePoint);
	client->ps.ammo[WP_GATLING]= gatling->count;
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

qboolean G_CanEntityBeActivated(gentity_t *player, gentity_t *target, qboolean exact){
	trace_t tr;
	vec3_t muzzle, forward, end;

	if(!exact && Distance(player->client->ps.origin, target->r.currentOrigin) > ACTIVATE_RANGE)
		return qfalse;

	VectorCopy( player->client->ps.origin, muzzle );
	muzzle[2] += player->client->ps.viewheight;

	// modify the muzzle point
	G_ModifyEyeAngles(muzzle, player->client->ps.viewangles, qfalse);

	//calculate muzzle and end point
	AngleVectors(player->client->ps.viewangles, forward, NULL, NULL);
	VectorMA (muzzle, 8192, forward, end);

	//find the nearest obstacle in the line
	trap_Trace(&tr, muzzle, NULL, NULL, end, player->client->ps.clientNum, MASK_SHOT);

	if(tr.entityNum != target->s.number){
		return qfalse;
	}

	if(exact && Distance(player->client->ps.origin, tr.endpos) > ACTIVATE_RANGE)
		return qfalse;

	return qtrue;
}

void G_ModifyEyeAngles(vec3_t origin, vec3_t viewangles, qboolean print){

	// first change the CONTENTS_BODY to CONTENTS_SOLID
	G_MoverContents(qtrue);

	BG_ModifyEyeAngles(origin, viewangles, trap_Trace, NULL, print);

	G_MoverContents(qfalse);
}
