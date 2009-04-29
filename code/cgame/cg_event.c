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
//
// cg_event.c -- handle entity events at snapshot or playerstate transitions

#include "cg_local.h"

// for the voice chats
#ifdef SMOKINGUNS
#include "../../ui/menudef.h"
#endif
//==========================================================================

/*
===================
CG_PlaceString

Also called by scoreboard drawing
===================
*/
const char	*CG_PlaceString( int rank ) {
	static char	str[64];
	char	*s, *t;

	if ( rank & RANK_TIED_FLAG ) {
		rank &= ~RANK_TIED_FLAG;
		t = "Tied for ";
	} else {
		t = "";
	}

#ifndef SMOKINGUNS
	if ( rank == 1 ) {
		s = S_COLOR_BLUE "1st" S_COLOR_WHITE;		// draw in blue
	} else if ( rank == 2 ) {
		s = S_COLOR_RED "2nd" S_COLOR_WHITE;		// draw in red
	} else if ( rank == 3 ) {
		s = S_COLOR_YELLOW "3rd" S_COLOR_WHITE;		// draw in yellow
	} else if ( rank == 11 ) {
		s = "11th";
	} else if ( rank == 12 ) {
		s = "12th";
	} else if ( rank == 13 ) {
		s = "13th";
	} else if ( rank % 10 == 1 ) {
		s = va("%ist", rank);
	} else if ( rank % 10 == 2 ) {
		s = va("%ind", rank);
	} else if ( rank % 10 == 3 ) {
		s = va("%ird", rank);
	} else {
		s = va("%ith", rank);
	}
#else
	if ( rank == 1 ) {
		s = S_COLOR_BLUE "1st" S_COLOR_BLACK;		// draw in blue
	} else if ( rank == 2 ) {
		s = S_COLOR_RED "2nd" S_COLOR_BLACK;		// draw in red
	} else if ( rank == 3 ) {
		s = S_COLOR_YELLOW "3rd" S_COLOR_BLACK;		// draw in yellow
	} else if ( rank == 11 ) {
		s = S_COLOR_WHITE "11th" S_COLOR_BLACK;
	} else if ( rank == 12 ) {
		s = S_COLOR_WHITE "12th" S_COLOR_BLACK;
	} else if ( rank == 13 ) {
		s = S_COLOR_WHITE "13th" S_COLOR_BLACK;
	} else if ( rank % 10 == 1 ) {
		s = va("%s%ist%s", S_COLOR_WHITE, rank, S_COLOR_BLACK);
	} else if ( rank % 10 == 2 ) {
		s = va("%s%ind%s", S_COLOR_WHITE, rank, S_COLOR_BLACK);
	} else if ( rank % 10 == 3 ) {
		s = va("%s%ird%s", S_COLOR_WHITE, rank, S_COLOR_BLACK);
	} else {
		s = va("%s%ith%s", S_COLOR_WHITE, rank, S_COLOR_BLACK);
	}
#endif

	Com_sprintf( str, sizeof( str ), "%s%s", t, s );
	return str;
}

#ifdef SMOKINGUNS
static void CG_Hit_Message( entityState_t *ent ){
	const char		*info;
	char			name[32];
	clientInfo_t	*ci;
	int				target = ent->otherEntityNum, shooter = ent->otherEntityNum2;
	int				nameNum;
	int				count = 0;
	int				i;

	if(!cg_hitmsg.integer && shooter == cg.snap->ps.clientNum)
		return;

	if(!cg_ownhitmsg.integer && target == cg.snap->ps.clientNum)
		return;

	// is this client affected by the shoot?
	if(shooter != cg.snap->ps.clientNum &&
		target != cg.snap->ps.clientNum)
		return;

	// name must be the other player
	if(target == cg.snap->ps.clientNum)
		nameNum = shooter;
	else
		nameNum = target;

	if ( nameNum < 0 || nameNum >= MAX_CLIENTS ) {
		return;
	}
	ci = &cgs.clientinfo[nameNum];

	info = CG_ConfigString( CS_PLAYERS + nameNum );
	if ( !info ) {
		return;
	}
	Q_strncpyz( name, Info_ValueForKey( info, "n" ), sizeof(name) - 2);
	strcat( name, S_COLOR_WHITE );

	// if we have to print several locations
	for(i=0; i<3; i++){
		int j;

		if(ent->angles2[i] != -1) {
			qboolean del = qfalse;

			// check if this location is already stored
			for(j = 0; j < i; j++){
				if(ent->angles2[i] == ent->angles2[j]){

					// front
					if(ent->weapon){
						if( !Q_stricmp(hit_info[(int)ent->angles2[i]].forename,
							hit_info[(int)ent->angles2[j]].forename)){
							del = qtrue;
						}
					} else if( !Q_stricmp(hit_info[(int)ent->angles2[i]].backname,
						hit_info[(int)ent->angles2[j]].backname)){
						del = qtrue;
					}
				}
			}

			// delete this location if nessecary
			if(del){
				for(j=i; j<2; j++){
					ent->angles2[j] = ent->angles2[j+1];
				}
				ent->angles2[2] = -1;
				continue;
			}

			count++;
		}
	}

	if(count){
		char part[3][10];

		for(i=0;i<count;i++){

			if((int)ent->angles2[i] < 0 || (int)ent->angles2[i] > NUM_HIT_LOCATIONS)
				return;

			if(ent->weapon)
				strcpy(part[i], hit_info[(int)ent->angles2[i]].forename);
			else
				strcpy(part[i], hit_info[(int)ent->angles2[i]].backname);
		}

		if(target == cg.snap->ps.clientNum){
			if(count == 3)
				CG_Printf("^1Your %s, your %s and your %s are hit.\n", part[0], part[1], part[2]);
			else if(count == 2)
				CG_Printf("^1Your %s and your %s are hit.\n", part[0], part[1]);
			else
				CG_Printf("^1Your %s is hit.\n", part[0]);
		} else {
			if(count == 3)
				CG_Printf("^2You hit ^7%s^2's %s, %s and %s.\n", name, part[0], part[1], part[2]);
			else if(count == 2)
				CG_Printf("^2You hit ^7%s^2's %s and %s.\n", name, part[0], part[1]);
			else
				CG_Printf("^2You hit ^7%s^2's %s.\n", name, part[0]);
		}
	} else {

		if(ent->frame < 0 || ent->frame > NUM_HIT_LOCATIONS)
			return;

		if(target == cg.snap->ps.clientNum){
			if(ent->weapon)
				CG_Printf("^1Your %s is hit.\n", hit_info[ent->frame].forename);
			else
				CG_Printf("^1Your %s is hit.\n", hit_info[ent->frame].backname);
		} else {
			if(ent->weapon)
				CG_Printf("^2You hit ^7%s^2's %s.\n", name, hit_info[ent->frame].forename);
			else
				CG_Printf("^2You hit ^7%s^2's %s.\n", name, hit_info[ent->frame].backname);
		}
	}
}
#endif

/*
=============
CG_Obituary
=============
*/
static void CG_Obituary( entityState_t *ent ) {
	int			mod;
	int			target, attacker;
	char		*message;
	char		*message2;
	const char	*targetInfo;
	const char	*attackerInfo;
	char		targetName[32];
	char		attackerName[32];
	gender_t	gender;
	clientInfo_t	*ci;

	target = ent->otherEntityNum;
	attacker = ent->otherEntityNum2;
	mod = ent->eventParm;

	if ( target < 0 || target >= MAX_CLIENTS ) {
		CG_Error( "CG_Obituary: target out of range" );
	}
	ci = &cgs.clientinfo[target];

	if ( attacker < 0 || attacker >= MAX_CLIENTS ) {
		attacker = ENTITYNUM_WORLD;
		attackerInfo = NULL;
	} else {
		attackerInfo = CG_ConfigString( CS_PLAYERS + attacker );
	}

	targetInfo = CG_ConfigString( CS_PLAYERS + target );
	if ( !targetInfo ) {
		return;
	}
	Q_strncpyz( targetName, Info_ValueForKey( targetInfo, "n" ), sizeof(targetName) - 2);
	strcat( targetName, S_COLOR_WHITE );

	message2 = "";

	// check for single client messages

	switch( mod ) {
	case MOD_SUICIDE:
#ifndef SMOKINGUNS
		message = "suicides";
#else
		message = "couldn't take it anymore";
#endif
		break;
	case MOD_FALLING:
#ifndef SMOKINGUNS
		message = "cratered";
#else
		gender = ci->gender;
		if ( gender == GENDER_FEMALE )
			message = "fell to her death";
		else if ( gender == GENDER_NEUTER )
			message = "fell to its death";
		else
			message = "fell to his death";
		break;
	case MOD_WATER:
		message = "sleeps with the fish";
#endif
		break;
	case MOD_CRUSH:
#ifndef SMOKINGUNS
		message = "was squished";
		break;
	case MOD_WATER:
		message = "sank like a rock";
		break;
#endif
	case MOD_SLIME:
#ifndef SMOKINGUNS
		message = "melted";
		break;
#endif
	case MOD_LAVA:
#ifndef SMOKINGUNS
		message = "does a back flip into the lava";
		break;
#endif
	case MOD_TARGET_LASER:
#ifndef SMOKINGUNS
		message = "saw the light";
		break;
#endif
	case MOD_TRIGGER_HURT:
#ifndef SMOKINGUNS
		message = "was in the wrong place";
#else
		message = "had an accident";
#endif
		break;
	default:
		message = NULL;
		break;
	}

	if (attacker == target) {
		gender = ci->gender;
		switch (mod) {
#ifndef SMOKINGUNS
#ifdef MISSIONPACK
		case MOD_KAMIKAZE:
			message = "goes out with a bang";
			break;
#endif
		case MOD_GRENADE_SPLASH:
			if ( gender == GENDER_FEMALE )
				message = "tripped on her own grenade";
			else if ( gender == GENDER_NEUTER )
				message = "tripped on its own grenade";
			else
				message = "tripped on his own grenade";
			break;
		case MOD_ROCKET_SPLASH:
			if ( gender == GENDER_FEMALE )
				message = "blew herself up";
			else if ( gender == GENDER_NEUTER )
				message = "blew itself up";
			else
				message = "blew himself up";
			break;
		case MOD_PLASMA_SPLASH:
			if ( gender == GENDER_FEMALE )
				message = "melted herself";
			else if ( gender == GENDER_NEUTER )
				message = "melted itself";
			else
				message = "melted himself";
			break;
		case MOD_BFG_SPLASH:
			message = "should have used a smaller gun";
			break;
#ifdef MISSIONPACK
		case MOD_PROXIMITY_MINE:
			if( gender == GENDER_FEMALE ) {
				message = "found her prox mine";
			} else if ( gender == GENDER_NEUTER ) {
				message = "found it's prox mine";
			} else {
				message = "found his prox mine";
			}
			break;
#endif
#else
		case MOD_DYNAMITE:
			if ( gender == GENDER_FEMALE )
				message = "blew herself to smithereens";
			else if ( gender == GENDER_NEUTER )
				message = "blew itself to smithereens";
			else
				message = "blew himself to smithereens";
			break;
		case MOD_MOLOTOV:
			if ( gender == GENDER_FEMALE )
				message = "played with matches";
			else if ( gender == GENDER_NEUTER )
				message = "played with matches";
			else
				message = "played with matches";
			break;
#endif
		default:
			if ( gender == GENDER_FEMALE )
				message = "killed herself";
			else if ( gender == GENDER_NEUTER )
				message = "killed itself";
			else
				message = "killed himself";
			break;
		}
	}

	if (message) {
		CG_Printf( "%s %s.\n", targetName, message);
		return;
	}

#ifndef SMOKINGUNS
	// check for kill messages from the current clientNum
	if ( attacker == cg.snap->ps.clientNum ) {
		char	*s;

		if ( cgs.gametype < GT_TEAM ) {
			s = va("You fragged %s\n%s place with %i", targetName, 
				CG_PlaceString( cg.snap->ps.persistant[PERS_RANK] + 1 ),
				cg.snap->ps.persistant[PERS_SCORE] );
		} else {
			s = va("You fragged %s", targetName );
		}
#ifdef MISSIONPACK
		if (!(cg_singlePlayerActive.integer && cg_cameraOrbit.integer)) {
			CG_CenterPrint( s, SCREEN_HEIGHT * 0.30, BIGCHAR_WIDTH );
		} 
#else
		CG_CenterPrint( s, SCREEN_HEIGHT * 0.30, BIGCHAR_WIDTH );
#endif

		// print the text message as well
	}
#endif

	// check for double client messages
	if ( !attackerInfo ) {
		attacker = ENTITYNUM_WORLD;
		strcpy( attackerName, "noname" );
	} else {
		Q_strncpyz( attackerName, Info_ValueForKey( attackerInfo, "n" ), sizeof(attackerName) - 2);
		strcat( attackerName, S_COLOR_WHITE );
		// check for kill messages about the current clientNum
		if ( target == cg.snap->ps.clientNum ) {
			Q_strncpyz( cg.killerName, attackerName, sizeof( cg.killerName ) );
		}
	}

#ifndef SMOKINGUNS
	if ( attacker != ENTITYNUM_WORLD ) {
		switch (mod) {
		case MOD_GRAPPLE:
			message = "was caught by";
			break;
		case MOD_GAUNTLET:
			message = "was pummeled by";
			break;
		case MOD_MACHINEGUN:
			message = "was machinegunned by";
			break;
		case MOD_SHOTGUN:
			message = "was gunned down by";
			break;
		case MOD_GRENADE:
			message = "ate";
			message2 = "'s grenade";
			break;
		case MOD_GRENADE_SPLASH:
			message = "was shredded by";
			message2 = "'s shrapnel";
			break;
		case MOD_ROCKET:
			message = "ate";
			message2 = "'s rocket";
			break;
		case MOD_ROCKET_SPLASH:
			message = "almost dodged";
			message2 = "'s rocket";
			break;
		case MOD_PLASMA:
			message = "was melted by";
			message2 = "'s plasmagun";
			break;
		case MOD_PLASMA_SPLASH:
			message = "was melted by";
			message2 = "'s plasmagun";
			break;
		case MOD_RAILGUN:
			message = "was railed by";
			break;
		case MOD_LIGHTNING:
			message = "was electrocuted by";
			break;
		case MOD_BFG:
		case MOD_BFG_SPLASH:
			message = "was blasted by";
			message2 = "'s BFG";
			break;
#ifdef MISSIONPACK
		case MOD_NAIL:
			message = "was nailed by";
			break;
		case MOD_CHAINGUN:
			message = "got lead poisoning from";
			message2 = "'s Chaingun";
			break;
		case MOD_PROXIMITY_MINE:
			message = "was too close to";
			message2 = "'s Prox Mine";
			break;
		case MOD_KAMIKAZE:
			message = "falls to";
			message2 = "'s Kamikaze blast";
			break;
		case MOD_JUICED:
			message = "was juiced by";
			break;
#endif
		case MOD_TELEFRAG:
			message = "tried to invade";
			message2 = "'s personal space";
			break;
		default:
			message = "was killed by";
			break;
		}

		if (message) {
			CG_Printf( "%s %s %s%s\n", 
				targetName, message, attackerName, message2);
			return;
		}
	}

	// we don't know what it was
	CG_Printf( "%s died.\n", targetName );
#else
	if(mod ==MOD_TELEFRAG){
		message = "tried to invade";
		message2 = "'s personal space";
		CG_Printf("%s %s %s%s.\n", attackerName, message, targetName, message2);
		return;
	}

	if (attacker != ENTITYNUM_WORLD){
		int i, weapon = 0;
		//drag the old fields
		for(i = MAX_DEATH_MESSAGES-1; i; i--){
			if(cg.messagetime[i-1]){
				strcpy(cg.attacker[i], cg.attacker[i-1]);
				strcpy(cg.target[i], cg.target[i-1]);
				cg.messagetime[i] = cg.messagetime[i-1];
				cg.mod[i] = cg.mod[i-1];
			}
		}
		//the new field
		strcpy(cg.attacker[0], attackerName);
		strcpy(cg.target[0], targetName);
		cg.messagetime[0] = cg.time;
		cg.mod[0] = mod;

		if(mod > 0 && mod < WP_NUM_WEAPONS)
			weapon = mod;

		if(weapon){
			CG_Printf("%s killed %s with %s.\n", attackerName, targetName, bg_weaponlist[weapon].name );
			return;
		}
	}

	// we don't know what has happened
	CG_Printf( "%s bit the dust\n", targetName );
#endif
}

//==========================================================================

/*
===============
CG_UseItem
===============
*/
static void CG_UseItem( centity_t *cent ) {
#ifndef SMOKINGUNS
	clientInfo_t *ci;
	int			itemNum, clientNum;
	gitem_t		*item;
	entityState_t *es;

	es = &cent->currentState;

	itemNum = (es->event & ~EV_EVENT_BITS) - EV_USE_ITEM0;
	if ( itemNum < 0 || itemNum > HI_NUM_HOLDABLE ) {
		itemNum = 0;
	}

	// print a message if the local player
	if ( es->number == cg.snap->ps.clientNum ) {
		if ( !itemNum ) {
			CG_CenterPrint( "No item to use", SCREEN_HEIGHT * 0.30, BIGCHAR_WIDTH );
		} else {
			item = BG_FindItemForHoldable( itemNum );
			CG_CenterPrint( va("Use %s", item->pickup_name), SCREEN_HEIGHT * 0.30, BIGCHAR_WIDTH );
		}
	}

	switch ( itemNum ) {
	default:
	case HI_NONE:
		trap_S_StartSound (NULL, es->number, CHAN_BODY, cgs.media.useNothingSound );
		break;

	case HI_TELEPORTER:
		break;

	case HI_MEDKIT:
		clientNum = cent->currentState.clientNum;
		if ( clientNum >= 0 && clientNum < MAX_CLIENTS ) {
			ci = &cgs.clientinfo[ clientNum ];
			ci->medkitUsageTime = cg.time;
		}
		trap_S_StartSound (NULL, es->number, CHAN_BODY, cgs.media.medkitSound );
		break;

	case HI_KAMIKAZE:
		break;

	case HI_PORTAL:
		break;
	case HI_INVULNERABILITY:
		trap_S_StartSound (NULL, es->number, CHAN_BODY, cgs.media.useInvulnerabilitySound );
		break;
	}

#endif
}

/*
================
CG_ItemPickup

A new item was picked up this frame
================
*/
static void CG_ItemPickup( int itemNum ) {
	cg.itemPickup = itemNum;
	cg.itemPickupTime = cg.time;
	cg.itemPickupBlendTime = cg.time;
	// see if it should be the grabbed weapon
#ifndef SMOKINGUNS
	if ( bg_itemlist[itemNum].giType == IT_WEAPON ) {
		// select it immediately
		if ( cg_autoswitch.integer && bg_itemlist[itemNum].giTag != WP_MACHINEGUN ) {
			cg.weaponSelectTime = cg.time;
			cg.weaponSelect = bg_itemlist[itemNum].giTag;
		}
	}
#endif

}


/*
================
CG_PainEvent

Also called by playerstate transition
================
*/
void CG_PainEvent( centity_t *cent, int health ) {
	char	*snd;

	// don't do more than two pain sounds a second
	if ( cg.time - cent->pe.painTime < 500 ) {
		return;
	}

	if ( health < 25 ) {
		snd = "*pain25_1.wav";
	} else if ( health < 50 ) {
		snd = "*pain50_1.wav";
	} else if ( health < 75 ) {
		snd = "*pain75_1.wav";
	} else {
		snd = "*pain100_1.wav";
	}
	trap_S_StartSound( NULL, cent->currentState.number, CHAN_VOICE,
		CG_CustomSound( cent->currentState.number, snd ) );

	// save pain time for programitic twitch animation
	cent->pe.painTime = cg.time;
	cent->pe.painDirection ^= 1;
}



/*
==============
CG_EntityEvent

An entity has an event value
also called by CG_CheckPlayerstateEvents
==============
*/
#define	DEBUGNAME(x) if(cg_debugEvents.integer){CG_Printf(x"\n");}
void CG_EntityEvent( centity_t *cent, vec3_t position ) {
	entityState_t	*es;
	int				event;
	vec3_t			dir;
	const char		*s;
	int				clientNum;
	clientInfo_t	*ci;
#ifdef SMOKINGUNS
	vec3_t			dirs[ALC_COUNT];
	int weapon = 0;
	int anim = 0;
#endif

	es = &cent->currentState;
	event = es->event & ~EV_EVENT_BITS;

	if ( cg_debugEvents.integer ) {
		CG_Printf( "ent:%3i  event:%3i ", es->number, event );
	}

	if ( !event ) {
		DEBUGNAME("ZEROEVENT");
		return;
	}

	clientNum = es->clientNum;
	if ( clientNum < 0 || clientNum >= MAX_CLIENTS ) {
		clientNum = 0;
	}
	ci = &cgs.clientinfo[ clientNum ];

	switch ( event ) {
	//
	// movement generated events
	//
	case EV_FOOTSTEP:
		DEBUGNAME("EV_FOOTSTEP");
		if (cg_footsteps.integer) {
			trap_S_StartSound (NULL, es->number, CHAN_BODY,
				cgs.media.footsteps[ ci->footsteps ][rand()&3] );
		}
		break;
#ifdef SMOKINGUNS
	case EV_FOOTSTEP_SNOW:
		DEBUGNAME("EV_FOOTSTEP_SNOW");
		if (cg_footsteps.integer) {
			trap_S_StartSound (NULL, es->number, CHAN_BODY,
				cgs.media.footsteps[ FOOTSTEP_SNOW ][rand()&3] );
		}
		break;
	case EV_FOOTSTEP_SAND:
		DEBUGNAME("EV_FOOTSTEP_SAND");
		if (cg_footsteps.integer) {
			trap_S_StartSound (NULL, es->number, CHAN_BODY,
				cgs.media.footsteps[ FOOTSTEP_SAND ][rand()&3] );
		}
		break;
	case EV_FOOTSTEP_CLOTH:
		DEBUGNAME("EV_FOOTSTEP_CLOTH");
		if (cg_footsteps.integer) {
			trap_S_StartSound (NULL, es->number, CHAN_BODY,
				cgs.media.footsteps[ FOOTSTEP_CLOTH ][rand()&3] );
		}
		break;
	case EV_FOOTSTEP_GRASS:
		DEBUGNAME("EV_FOOTSTEP_GRASS");
		if (cg_footsteps.integer) {
			trap_S_StartSound (NULL, es->number, CHAN_BODY,
				cgs.media.footsteps[ FOOTSTEP_GRASS ][rand()&3] );
		}
		break;
#endif
	case EV_FOOTSTEP_METAL:
		DEBUGNAME("EV_FOOTSTEP_METAL");
		if (cg_footsteps.integer) {
			trap_S_StartSound (NULL, es->number, CHAN_BODY,
				cgs.media.footsteps[ FOOTSTEP_METAL ][rand()&3] );
		}
		break;
	case EV_FOOTSPLASH:
		DEBUGNAME("EV_FOOTSPLASH");
		if (cg_footsteps.integer) {
			trap_S_StartSound (NULL, es->number, CHAN_BODY,
				cgs.media.footsteps[ FOOTSTEP_SPLASH ][rand()&3] );
		}
		break;
	case EV_FOOTWADE:
		DEBUGNAME("EV_FOOTWADE");
		if (cg_footsteps.integer) {
			trap_S_StartSound (NULL, es->number, CHAN_BODY,
				cgs.media.footsteps[ FOOTSTEP_SPLASH ][rand()&3] );
		}
		break;
	case EV_SWIM:
		DEBUGNAME("EV_SWIM");
		if (cg_footsteps.integer) {
			trap_S_StartSound (NULL, es->number, CHAN_BODY,
				cgs.media.footsteps[ FOOTSTEP_SPLASH ][rand()&3] );
		}
		break;

#ifdef SMOKINGUNS
	case EV_FALL_VERY_SHORT:
		DEBUGNAME("EV_FALL_VERY_SHORT");
		if (cg_footsteps.integer) {
			trap_S_StartSound (NULL, es->number, CHAN_BODY,
				cgs.media.footsteps[ ci->footsteps ][rand()&3] );
		}
		if ( clientNum == cg.predictedPlayerState.clientNum ) {
			// smooth landing z changes
			cg.landChange = -4;
			cg.landTime = cg.time;
			cg.landAngleChange = 0;
		}
		break;
#endif

	case EV_FALL_SHORT:
		DEBUGNAME("EV_FALL_SHORT");
		trap_S_StartSound (NULL, es->number, CHAN_AUTO, cgs.media.landSound );
		if ( clientNum == cg.predictedPlayerState.clientNum ) {
			// smooth landing z changes
#ifndef SMOKINGUNS
			cg.landChange = -8;
			cg.landTime = cg.time;
#else
			cg.landChange = -4;
			cg.landTime = cg.time;
			cg.landAngleChange = 0;
#endif
		}
		break;
	case EV_FALL_MEDIUM:
		DEBUGNAME("EV_FALL_MEDIUM");
		// use normal pain sound
		trap_S_StartSound( NULL, es->number, CHAN_VOICE, CG_CustomSound( es->number, "*pain100_1.wav" ) );
		if ( clientNum == cg.predictedPlayerState.clientNum ) {
			// smooth landing z changes
			cg.landChange = -16;
			cg.landTime = cg.time;
#ifndef SMOKINGUNS
		}
#else
			switch(rand()%2){
			case 0:
				cg.landAngleChange = 10;
				break;
			case 1:
			default:
				cg.landAngleChange = -10;
				break;
			}
		}
		//launch blood particle
		VectorSet(dir, 0, 0, -1);
		CG_LaunchImpactParticle(es->pos.trBase, dir, -1, -1, -1, qtrue);
#endif
		break;
	case EV_FALL_FAR:
		DEBUGNAME("EV_FALL_FAR");
#ifdef SMOKINGUNS
		// use normal pain sound
		trap_S_StartSound( NULL, es->number, CHAN_VOICE, CG_CustomSound( es->number, "*pain100_1.wav" ) );
		if ( clientNum == cg.predictedPlayerState.clientNum ) {
			// smooth landing z changes
			cg.landChange = -20;
			cg.landTime = cg.time;

			switch(rand()%2){
			case 0:
				cg.landAngleChange = 20;
				break;
			case 1:
			default:
				cg.landAngleChange = -20;
				break;
			}
		}
		//launch blood particle
		VectorSet(dir, 0, 0, -1);
		CG_LaunchImpactParticle(es->pos.trBase, dir, -1, -1, -1, qtrue);
		break;
	case EV_FALL_VERY_FAR:
		DEBUGNAME("EV_FALL_VERY_FAR");
#endif
		trap_S_StartSound (NULL, es->number, CHAN_AUTO, CG_CustomSound( es->number, "*fall1.wav" ) );
		cent->pe.painTime = cg.time;	// don't play a pain sound right after this
		if ( clientNum == cg.predictedPlayerState.clientNum ) {
			// smooth landing z changes
			cg.landChange = -24;
			cg.landTime = cg.time;
#ifndef SMOKINGUNS
		}
#else
			switch(rand()%2){
			case 0:
				cg.landAngleChange = 30;
				break;
			case 1:
			default:
				cg.landAngleChange = -30;
				break;
			}
		}
		//launch blood particle
		VectorSet(dir, 0, 0, -1);
		CG_LaunchImpactParticle(es->pos.trBase, dir, -1, -1, -1, qtrue);
#endif
		break;

	case EV_STEP_4:
	case EV_STEP_8:
	case EV_STEP_12:
	case EV_STEP_16:		// smooth out step up transitions
		DEBUGNAME("EV_STEP");
	{
		float	oldStep;
		int		delta;
		int		step;

		if ( clientNum != cg.predictedPlayerState.clientNum ) {
			break;
		}
		// if we are interpolating, we don't need to smooth steps
#ifndef SMOKINGUNS
		if ( cg.demoPlayback || (cg.snap->ps.pm_flags & PMF_FOLLOW) ||
#else
		if ( cg.demoPlayback ||
#endif
			cg_nopredict.integer || cg_synchronousClients.integer ) {
			break;
		}
		// check for stepping up before a previous step is completed
		delta = cg.time - cg.stepTime;
		if (delta < STEP_TIME) {
			oldStep = cg.stepChange * (STEP_TIME - delta) / STEP_TIME;
		} else {
			oldStep = 0;
		}

		// add this amount
		step = 4 * (event - EV_STEP_4 + 1 );
		cg.stepChange = oldStep + step;
		if ( cg.stepChange > MAX_STEP_CHANGE ) {
			cg.stepChange = MAX_STEP_CHANGE;
		}
		cg.stepTime = cg.time;
		break;
	}

	case EV_JUMP_PAD:
		DEBUGNAME("EV_JUMP_PAD");
//		CG_Printf( "EV_JUMP_PAD w/effect #%i\n", es->eventParm );
#ifndef SMOKINGUNS
		{
			localEntity_t	*smoke;
			vec3_t			up = {0, 0, 1};


			smoke = CG_SmokePuff( cent->lerpOrigin, up,
						  32,
						  1, 1, 1, 0.33f,
						  1000,
						  cg.time, 0,
						  LEF_PUFF_DONT_SCALE,
						  cgs.media.smokePuffShader );
		}

		// boing sound at origin, jump sound on player
		trap_S_StartSound ( cent->lerpOrigin, -1, CHAN_VOICE, cgs.media.jumpPadSound );
		trap_S_StartSound (NULL, es->number, CHAN_VOICE, CG_CustomSound( es->number, "*jump1.wav" ) );
#endif
		break;

	case EV_JUMP:
		DEBUGNAME("EV_JUMP");
		trap_S_StartSound (NULL, es->number, CHAN_VOICE, CG_CustomSound( es->number, "*jump1.wav" ) );
		break;
	case EV_TAUNT:
		DEBUGNAME("EV_TAUNT");
#ifndef SMOKINGUNS
		trap_S_StartSound (NULL, es->number, CHAN_VOICE, CG_CustomSound( es->number, "*taunt.wav" ) );
#else
		if(bg_weaponlist[cent->currentState.weapon].wp_sort == WPS_PISTOL)
			trap_S_StartSound (NULL, es->number, CHAN_VOICE, cgs.media.taunt[1]);
		else
			trap_S_StartSound (NULL, es->number, CHAN_VOICE, cgs.media.taunt[0]);
#endif
		break;
#ifndef SMOKINGUNS
	case EV_TAUNT_YES:
		DEBUGNAME("EV_TAUNT_YES");
		CG_VoiceChatLocal(SAY_TEAM, qfalse, es->number, COLOR_CYAN, VOICECHAT_YES);
		break;
	case EV_TAUNT_NO:
		DEBUGNAME("EV_TAUNT_NO");
		CG_VoiceChatLocal(SAY_TEAM, qfalse, es->number, COLOR_CYAN, VOICECHAT_NO);
		break;
	case EV_TAUNT_FOLLOWME:
		DEBUGNAME("EV_TAUNT_FOLLOWME");
		CG_VoiceChatLocal(SAY_TEAM, qfalse, es->number, COLOR_CYAN, VOICECHAT_FOLLOWME);
		break;
	case EV_TAUNT_GETFLAG:
		DEBUGNAME("EV_TAUNT_GETFLAG");
		CG_VoiceChatLocal(SAY_TEAM, qfalse, es->number, COLOR_CYAN, VOICECHAT_ONGETFLAG);
		break;
	case EV_TAUNT_GUARDBASE:
		DEBUGNAME("EV_TAUNT_GUARDBASE");
		CG_VoiceChatLocal(SAY_TEAM, qfalse, es->number, COLOR_CYAN, VOICECHAT_ONDEFENSE);
		break;
	case EV_TAUNT_PATROL:
		DEBUGNAME("EV_TAUNT_PATROL");
		CG_VoiceChatLocal(SAY_TEAM, qfalse, es->number, COLOR_CYAN, VOICECHAT_ONPATROL);
		break;
#endif
	case EV_WATER_TOUCH:
		DEBUGNAME("EV_WATER_TOUCH");
		trap_S_StartSound (NULL, es->number, CHAN_AUTO, cgs.media.watrInSound );
		break;
	case EV_WATER_LEAVE:
		DEBUGNAME("EV_WATER_LEAVE");
		trap_S_StartSound (NULL, es->number, CHAN_AUTO, cgs.media.watrOutSound );
		break;
	case EV_WATER_UNDER:
		DEBUGNAME("EV_WATER_UNDER");
		trap_S_StartSound (NULL, es->number, CHAN_AUTO, cgs.media.watrUnSound );
		break;
	case EV_WATER_CLEAR:
		DEBUGNAME("EV_WATER_CLEAR");
		trap_S_StartSound (NULL, es->number, CHAN_AUTO, CG_CustomSound( es->number, "*gasp.wav" ) );
		break;

#ifdef SMOKINGUNS
	case EV_MARKED_ITEM:
		DEBUGNAME("EV_MARKED_ITEM");
		cg.marked_item_index = es->eventParm;
		VectorCopy(es->origin, cg.marked_item_mins);
		VectorCopy(es->origin2, cg.marked_item_maxs);
		cg.marked_item_time = cg.time;
		break;

	case EV_MONEY_PICKUP:
		DEBUGNAME("EV_MONEY_PICKUP");
		{
			gitem_t	*item;

			int		quantity;
			int		index = 1;
			quantity = es->eventParm;		// player predicted

			item = bg_itemlist + 1;
			for( ;item->classname; item++){
				if(!Q_stricmp(item->classname, "pickup_money"))
					break;
				index++;
			}

			trap_S_StartSound (NULL, es->number, CHAN_AUTO,	trap_S_RegisterSound( item->pickup_sound, qfalse ) );

			// show icon and name on status bar
			if ( es->number == cg.snap->ps.clientNum ) {
				cg.itemPickupQuant = quantity;
				cg.itemPickup = index;
				cg.itemPickupTime = cg.time;
				cg.itemPickupBlendTime = cg.time;
			}
		}
		break;
#endif

	case EV_ITEM_PICKUP:
		DEBUGNAME("EV_ITEM_PICKUP");
		{
			gitem_t	*item;
			int		index;

			index = es->eventParm;		// player predicted

			if ( index < 1 || index >= bg_numItems ) {
				break;
			}
			item = &bg_itemlist[ index ];

			// powerups and team items will have a separate global sound, this one
			// will be played at prediction time
#ifndef SMOKINGUNS
			if ( item->giType == IT_POWERUP || item->giType == IT_TEAM) {
				trap_S_StartSound (NULL, es->number, CHAN_AUTO,	cgs.media.n_healthSound );
			} else if (item->giType == IT_PERSISTANT_POWERUP) {
#ifdef MISSIONPACK
				switch (item->giTag ) {
					case PW_SCOUT:
						trap_S_StartSound (NULL, es->number, CHAN_AUTO,	cgs.media.scoutSound );
					break;
					case PW_GUARD:
						trap_S_StartSound (NULL, es->number, CHAN_AUTO,	cgs.media.guardSound );
					break;
					case PW_DOUBLER:
						trap_S_StartSound (NULL, es->number, CHAN_AUTO,	cgs.media.doublerSound );
					break;
					case PW_AMMOREGEN:
						trap_S_StartSound (NULL, es->number, CHAN_AUTO,	cgs.media.ammoregenSound );
					break;
				}
#endif
			} else {
				trap_S_StartSound (NULL, es->number, CHAN_AUTO,	trap_S_RegisterSound( item->pickup_sound, qfalse ) );
			}
#else
			trap_S_StartSound (NULL, es->number, CHAN_AUTO,	trap_S_RegisterSound( item->pickup_sound, qfalse ) );
#endif

			// show icon and name on status bar
			if ( es->number == cg.snap->ps.clientNum ) {
				CG_ItemPickup( index );
			}
		}
		break;

#ifndef SMOKINGUNS
	case EV_GLOBAL_ITEM_PICKUP:
		DEBUGNAME("EV_GLOBAL_ITEM_PICKUP");
		{
			gitem_t	*item;
			int		index;

			index = es->eventParm;		// player predicted

			if ( index < 1 || index >= bg_numItems ) {
				break;
			}
			item = &bg_itemlist[ index ];
			// powerup pickups are global
			if( item->pickup_sound ) {
				trap_S_StartSound (NULL, cg.snap->ps.clientNum, CHAN_AUTO, trap_S_RegisterSound( item->pickup_sound, qfalse ) );
			}

			// show icon and name on status bar
			if ( es->number == cg.snap->ps.clientNum ) {
				CG_ItemPickup( index );
			}
		}
		break;
#endif

	//
	// weapon events
	//
	case EV_NOAMMO:
		DEBUGNAME("EV_NOAMMO");
//		trap_S_StartSound (NULL, es->number, CHAN_AUTO, cgs.media.noAmmoSound );
		if ( es->number == cg.snap->ps.clientNum ) {
			CG_OutOfAmmoChange();
		}
		break;
	case EV_CHANGE_WEAPON:
		DEBUGNAME("EV_CHANGE_WEAPON");
#ifdef SMOKINGUNS
		if(cg.introend < cg.time)
#endif
			trap_S_StartSound (NULL, es->number, CHAN_AUTO, cgs.media.selectSound );
		break;

#ifdef SMOKINGUNS
	case EV_CHANGETO_SOUND:
		DEBUGNAME("EV_CHANGETO_SOUND");
		if(cg.introend < cg.time){
			if(bg_weaponlist[es->eventParm].wp_sort == WPS_PISTOL)
				trap_S_StartSound (NULL, es->number, CHAN_AUTO, cgs.media.snd_pistol_raise );
			else {
				switch (es->eventParm) {
				case WP_WINCHESTER66:
					trap_S_StartSound (NULL, es->number, CHAN_AUTO, cgs.media.snd_winch66_raise );
					break;
				case WP_LIGHTNING:
					trap_S_StartSound (NULL, es->number, CHAN_AUTO, cgs.media.snd_lightn_raise );
					break;
				case WP_SHARPS:
					trap_S_StartSound (NULL, es->number, CHAN_AUTO, cgs.media.snd_sharps_raise );
					break;
				case WP_REMINGTON_GAUGE:
					trap_S_StartSound (NULL, es->number, CHAN_AUTO, cgs.media.snd_shotgun_raise );
					break;
				case WP_WINCH97:
					trap_S_StartSound (NULL, es->number, CHAN_AUTO, cgs.media.snd_winch97_raise );
					break;
				}
			}
		}
		break;

	case EV_CHANGE_TO_WEAPON:
		DEBUGNAME("EV_CHANGE_TO_WEAPON");
		if(cent->currentState.clientNum == cg.snap->ps.clientNum)
			cg.weaponSelect = es->eventParm;
		break;

	case EV_CHOOSE_WEAPON:
		/*DEBUGNAME("EV_CHOOSE_WEAPON");
		if(cent->currentState.clientNum == cg.snap->ps.clientNum){
			cg.weaponSelect = cg.markedweapon;
			cg.weaponSelectTime = cg.time;
			cg.markedweapon = 0;
		}*/
		//trap_SendConsoleCommand("wp_choose_off\n");
		break;

	case EV_RELOAD:
		DEBUGNAME("EV_RELOAD");

		//if(cg.snap->ps.clientNum == cent->currentState.clientNum)
		//	break;

		CG_PlayReloadSound(es->eventParm, cent, qfalse);
		break;

	case EV_RELOAD2:
		DEBUGNAME("EV_RELOAD2");

		CG_PlayReloadSound(es->eventParm, cent, qtrue);
		break;

	case EV_LIGHT_DYNAMITE:
		DEBUGNAME("EV_LIGHT_DYNAMIT");

		trap_S_StartSound (NULL, cent->currentState.number, CHAN_WEAPON, cgs.media.dynamitezundan );
		break;

	case EV_ALT_FIRE_WEAPON:
		DEBUGNAME("EV_ALT_FIRE_WEAPON");

		anim = cent->currentState.eventParm;

		if(cent->currentState.clientNum == cg.snap->ps.clientNum
			&& cent->currentState.weapon != WP_KNIFE){
			cg.shootAngleChange = 2;
			cg.shootTime = cg.time;
			cg.shootChange = 0;

			//cent->pe.weapon.weaponAnim = ( ( cent->pe.weapon.weaponAnim & ANIM_TOGGLEBIT ) ^ ANIM_TOGGLEBIT )
			//		| anim;
		}

		//cg.firingtime = cg.time + BG_AnimLength(anim, cg.snap->ps.weapon);
		CG_FireWeapon( cent, qtrue, cent->currentState.weapon );
		break;

	case EV_FIRE_WEAPON2:
#endif
	case EV_FIRE_WEAPON:
		DEBUGNAME("EV_FIRE_WEAPON");
#ifndef SMOKINGUNS
		CG_FireWeapon( cent );
#else
		if(cg.markedweapon)
			return;

		anim = cent->currentState.eventParm;

		if(event == EV_FIRE_WEAPON2)
			weapon = cent->currentState.frame;
		else
			weapon = cent->currentState.weapon;


		if(cent->currentState.clientNum == cg.snap->ps.clientNum){

			if(bg_weaponlist[cent->currentState.weapon].wp_sort == WPS_PISTOL){
				cg.shootAngleChange = 5;
				cg.shootTime = cg.time;
				cg.shootChange = 0;
			} else if(cent->currentState.weapon == WP_SHARPS){
				cg.shootAngleChange = 6;
				cg.shootTime = cg.time;
				cg.shootChange = 0;
			} else if(cent->currentState.weapon != WP_KNIFE &&
				cent->currentState.weapon < WP_GATLING){
				cg.shootAngleChange = 4;
				cg.shootTime = cg.time;
				cg.shootChange = 0;
			}
		}

		CG_FireWeapon( cent, (event == EV_FIRE_WEAPON2), weapon );
		break;

	// 2nd Pistol
	case EV_FIRE_WEAPON_DELAY:
		DEBUGNAME("EV_FIRE_WEAPON_DELAY");
		break;

	case EV_ALT_WEAPON_MODE:
		break;

	case EV_GATLING_NOTPLANAR:
		if(cent->currentState.clientNum == cg.snap->ps.clientNum)
			CG_Printf("The ground is not flat enough.\n");
		break;

	case EV_GATLING_NOSPACE:
		if(cent->currentState.clientNum == cg.snap->ps.clientNum)
			CG_Printf("Not enough room for the gatling here.\n");
		break;

	case EV_GATLING_TOODEEP:
		if(cent->currentState.clientNum == cg.snap->ps.clientNum)
			CG_Printf("The water is too deep.\n");
		break;

	case EV_BUILD_TURRET:
		trap_S_StartSound (NULL, es->number, CHAN_WEAPON, cgs.media.gatling_build );
		break;

	case EV_DISM_TURRET:
		trap_S_StartSound (NULL, es->number, CHAN_WEAPON, cgs.media.gatling_dism );
		break;

	case EV_SCOPE_PUT:
		trap_S_StartSound (NULL, es->number, CHAN_WEAPON, cgs.media.scopePutSound );
		break;

	case EV_UNDERWATER:
		DEBUGNAME("EV_UNDERWATER");
		if(cent->currentState.clientNum == cg.snap->ps.clientNum)
			trap_S_AddLoopingSound(es->number, cent->lerpOrigin, vec3_origin, cgs.media.underwater );
		break;

	//Spoon Sounds, ok perhaps not mine ;) hoho that's so funny! asshole!
	case EV_DBURN:
		DEBUGNAME("EV_DBURN");
		trap_S_AddLoopingSound(es->number, cent->lerpOrigin, vec3_origin, cgs.media.dynamiteburn );
		break;

	case EV_NOAMMO_CLIP:
		DEBUGNAME("EV_WEAPONBLOCK");
		trap_S_StartSound (NULL, es->number, CHAN_WEAPON, cgs.media.noAmmoSound );
#endif
		break;

	case EV_USE_ITEM0:
		DEBUGNAME("EV_USE_ITEM0");
		CG_UseItem( cent );
		break;
	case EV_USE_ITEM1:
		DEBUGNAME("EV_USE_ITEM1");
		CG_UseItem( cent );
		break;
	case EV_USE_ITEM2:
		DEBUGNAME("EV_USE_ITEM2");
		CG_UseItem( cent );
		break;
	case EV_USE_ITEM3:
		DEBUGNAME("EV_USE_ITEM3");
		CG_UseItem( cent );
		break;
	case EV_USE_ITEM4:
		DEBUGNAME("EV_USE_ITEM4");
		CG_UseItem( cent );
		break;
	case EV_USE_ITEM5:
		DEBUGNAME("EV_USE_ITEM5");
		CG_UseItem( cent );
		break;
	case EV_USE_ITEM6:
		DEBUGNAME("EV_USE_ITEM6");
		CG_UseItem( cent );
		break;
	case EV_USE_ITEM7:
		DEBUGNAME("EV_USE_ITEM7");
		CG_UseItem( cent );
		break;
	case EV_USE_ITEM8:
		DEBUGNAME("EV_USE_ITEM8");
		CG_UseItem( cent );
		break;
	case EV_USE_ITEM9:
		DEBUGNAME("EV_USE_ITEM9");
		CG_UseItem( cent );
		break;
	case EV_USE_ITEM10:
		DEBUGNAME("EV_USE_ITEM10");
		CG_UseItem( cent );
		break;
	case EV_USE_ITEM11:
		DEBUGNAME("EV_USE_ITEM11");
		CG_UseItem( cent );
		break;
	case EV_USE_ITEM12:
		DEBUGNAME("EV_USE_ITEM12");
		CG_UseItem( cent );
		break;
	case EV_USE_ITEM13:
		DEBUGNAME("EV_USE_ITEM13");
		CG_UseItem( cent );
		break;
	case EV_USE_ITEM14:
		DEBUGNAME("EV_USE_ITEM14");
		CG_UseItem( cent );
		break;

	//=================================================================

	//
	// other events
	//
	case EV_PLAYER_TELEPORT_IN:
		DEBUGNAME("EV_PLAYER_TELEPORT_IN");
#ifndef SMOKINGUNS
		trap_S_StartSound (NULL, es->number, CHAN_AUTO, cgs.media.teleInSound );
		CG_SpawnEffect( position);
#endif
		break;

	case EV_PLAYER_TELEPORT_OUT:
		DEBUGNAME("EV_PLAYER_TELEPORT_OUT");
#ifndef SMOKINGUNS
		trap_S_StartSound (NULL, es->number, CHAN_AUTO, cgs.media.teleOutSound );
		CG_SpawnEffect(  position);
#endif
		break;

	case EV_ITEM_POP:
		DEBUGNAME("EV_ITEM_POP");
#ifndef SMOKINGUNS
		trap_S_StartSound (NULL, es->number, CHAN_AUTO, cgs.media.respawnSound );
#endif
		break;
	case EV_ITEM_RESPAWN:
		DEBUGNAME("EV_ITEM_RESPAWN");
		cent->miscTime = cg.time;	// scale up from this
#ifndef SMOKINGUNS
		trap_S_StartSound (NULL, es->number, CHAN_AUTO, cgs.media.respawnSound );
#endif
		break;

	case EV_GRENADE_BOUNCE:
		DEBUGNAME("EV_GRENADE_BOUNCE");
#ifndef SMOKINGUNS
		if ( rand() & 1 ) {
			trap_S_StartSound (NULL, es->number, CHAN_AUTO, cgs.media.hgrenb1aSound );
		} else {
			trap_S_StartSound (NULL, es->number, CHAN_AUTO, cgs.media.hgrenb2aSound );
		}
#endif
		break;

#ifndef SMOKINGUNS
	case EV_PROXIMITY_MINE_STICK:
		DEBUGNAME("EV_PROXIMITY_MINE_STICK");
		if( es->eventParm & SURF_FLESH ) {
			trap_S_StartSound (NULL, es->number, CHAN_AUTO, cgs.media.wstbimplSound );
		} else 	if( es->eventParm & SURF_METALSTEPS ) {
			trap_S_StartSound (NULL, es->number, CHAN_AUTO, cgs.media.wstbimpmSound );
		} else {
			trap_S_StartSound (NULL, es->number, CHAN_AUTO, cgs.media.wstbimpdSound );
		}
		break;

	case EV_PROXIMITY_MINE_TRIGGER:
		DEBUGNAME("EV_PROXIMITY_MINE_TRIGGER");
		trap_S_StartSound (NULL, es->number, CHAN_AUTO, cgs.media.wstbactvSound );
		break;
	case EV_KAMIKAZE:
		DEBUGNAME("EV_KAMIKAZE");
		CG_KamikazeEffect( cent->lerpOrigin );
		break;
	case EV_OBELISKEXPLODE:
		DEBUGNAME("EV_OBELISKEXPLODE");
		CG_ObeliskExplode( cent->lerpOrigin, es->eventParm );
		break;
	case EV_OBELISKPAIN:
		DEBUGNAME("EV_OBELISKPAIN");
		CG_ObeliskPain( cent->lerpOrigin );
		break;
	case EV_INVUL_IMPACT:
		DEBUGNAME("EV_INVUL_IMPACT");
		CG_InvulnerabilityImpact( cent->lerpOrigin, cent->currentState.angles );
		break;
	case EV_JUICED:
		DEBUGNAME("EV_JUICED");
		CG_InvulnerabilityJuiced( cent->lerpOrigin );
		break;
	case EV_LIGHTNINGBOLT:
		DEBUGNAME("EV_LIGHTNINGBOLT");
		CG_LightningBoltBeam(es->origin2, es->pos.trBase);
		break;
#endif
	case EV_SCOREPLUM:
		DEBUGNAME("EV_SCOREPLUM");
		CG_ScorePlum( cent->currentState.otherEntityNum, cent->lerpOrigin, cent->currentState.time );
		break;

	//
	// missile impacts
	//
#ifdef SMOKINGUNS
	case EV_WHISKEY_BURNS:
		ByteToDir(es->eventParm, dir);
		CG_CreateFire(position, dir);
		break;

	case EV_MISSILE_FIRE:
		ByteToDir(es->eventParm, dir);
		BG_EntityStateToDirs(es, dirs);
		CG_MissileHitWall( es->weapon, 0, position, dir, IMPACTSOUND_DEFAULT, -1, -1, qtrue, dirs, -1);
		break;

	// not used anymore
	case EV_MISSILE_ALCOHOL:
		//DEBUGNAME("EV_MISSILE_ALCOHOL");
		//ByteToDir( es->eventParm, dir);
		//CG_MissileHitWall( es->weapon, 0, position, dir, IMPACTSOUND_DEFAULT, -1, -1, qtrue, NULL);
		break;
#endif

	case EV_MISSILE_HIT:
		DEBUGNAME("EV_MISSILE_HIT");
		ByteToDir( es->eventParm, dir );
		CG_MissileHitPlayer( es->weapon, position, dir, es->otherEntityNum );
		break;

	case EV_MISSILE_MISS:
		DEBUGNAME("EV_MISSILE_MISS");
		ByteToDir( es->eventParm, dir );
#ifndef SMOKINGUNS
		CG_MissileHitWall( es->weapon, 0, position, dir, IMPACTSOUND_DEFAULT );
#else
		BG_EntityStateToDirs(es, dirs);
		CG_MissileHitWall( es->weapon, 0, position, dir, IMPACTSOUND_DEFAULT, -1, -1, qfalse, dirs, -1 );
#endif
		break;

	case EV_MISSILE_MISS_METAL:
		DEBUGNAME("EV_MISSILE_MISS_METAL");
		ByteToDir( es->eventParm, dir );
#ifndef SMOKINGUNS
		CG_MissileHitWall( es->weapon, 0, position, dir, IMPACTSOUND_METAL );
#else
		CG_MissileHitWall( es->weapon, 0, position, dir, IMPACTSOUND_METAL, -1, -1, qfalse, NULL, -1 );
#endif
		break;

#ifndef SMOKINGUNS
	case EV_RAILTRAIL:
		DEBUGNAME("EV_RAILTRAIL");
		cent->currentState.weapon = WP_RAILGUN;
		// if the end was on a nomark surface, don't make an explosion
		CG_RailTrail( ci, es->origin2, es->pos.trBase );
		if ( es->eventParm != 255 ) {
			ByteToDir( es->eventParm, dir );
			CG_MissileHitWall( es->weapon, es->clientNum, position, dir, IMPACTSOUND_DEFAULT );
		}
		break;
#else

	//func events
	case EV_FUNCBREAKABLE:
		CG_LaunchFuncBreakable(cent);
		break;

	case EV_BULLET:
		break;

	case EV_BULLET_HIT_BOILER:
		DEBUGNAME("EV_BULLET_HIT_BOILER");
		ByteToDir( es->eventParm, dir );
		trap_S_StartSound ( NULL, es->number, CHAN_AUTO, cgs.media.boilerhit);
		trap_S_StartSound ( NULL, es->number, CHAN_AUTO, cgs.media.boilerhit);
		trap_S_StartSound ( NULL, es->number, CHAN_AUTO, cgs.media.boilerhit);
		CG_Bullet( es->pos.trBase, es->otherEntityNum, dir, qfalse, ENTITYNUM_WORLD, es->time, SURF_METAL, -1, -1 );
		//CG_Printf("Your Boiler Plate was hit (debugging)\n");
		if(es->eventParm == cg.snap->ps.clientNum){
			//"shake" the view
			cg.landAngleChange = (rand()%40)-20;
			cg.landTime = cg.time;
			cg.landChange = 0;
		}
		break;

	case EV_KNIFEHIT:
		DEBUGNAME("EV_KNIFEHIT");
		trap_S_StartSound(es->pos.trBase, es->number, CHAN_WEAPON, cgs.media.knifehit );

		// wall was hit
		if( es->torsoAnim != -1){
			CG_LaunchImpactParticle( es->pos.trBase, dir, es->time2,
				es->torsoAnim, WP_KNIFE, qfalse);
			break;
		}

		if(es->weapon < MAX_CLIENTS && es->weapon >= 0){
			CG_LaunchImpactParticle(es->pos.trBase, dir, -1, -1, -1, qtrue);
		}

		if(es->eventParm == cg.snap->ps.clientNum){
			//"shake" the view
			cg.landAngleChange = (rand()%30)-15;
			cg.landTime = cg.time;
			cg.landChange = 0;
		}
		break;

	case EV_BOILER_HIT:
		DEBUGNAME("EV_BOILER_HIT"); //only sound
		trap_S_StartSound ( NULL, es->number, CHAN_AUTO, cgs.media.boilerhit);
		if(es->eventParm == cg.snap->ps.clientNum){
			//"shake" the view
			cg.landAngleChange = (rand()%40)-20;
			cg.landTime = cg.time;
			cg.landChange = 0;
		}
		break;
#endif

	case EV_BULLET_HIT_WALL:
		DEBUGNAME("EV_BULLET_HIT_WALL");
		ByteToDir( es->eventParm, dir );
#ifndef SMOKINGUNS
		CG_Bullet( es->pos.trBase, es->otherEntityNum, dir, qfalse, ENTITYNUM_WORLD );
#else
		CG_Bullet( es->pos.trBase, es->otherEntityNum, dir, qfalse, ENTITYNUM_WORLD, es->time, es->time2, es->torsoAnim, es->otherEntityNum2 );
#endif
		break;

	case EV_BULLET_HIT_FLESH:
		DEBUGNAME("EV_BULLET_HIT_FLESH");
#ifndef SMOKINGUNS
		CG_Bullet( es->pos.trBase, es->otherEntityNum, dir, qtrue, es->eventParm );
#else
		ByteToDir( es->eventParm, dir );
		CG_Bullet( es->pos.trBase, es->otherEntityNum, dir, qtrue, es->eventParm, es->time, -1, -1, -1);
		if(es->eventParm == cg.snap->ps.clientNum){
			//"shake" the view
			cg.landAngleChange = (rand()%30)-15;
			cg.landTime = cg.time;
			cg.landChange = 0;
		}
#endif
	break;

	case EV_SHOTGUN:
		DEBUGNAME("EV_SHOTGUN");
		CG_ShotgunFire( es );
		break;

#ifdef SMOKINGUNS
	case EV_SMOKE:
		DEBUGNAME("EV_SMOKE");
		if(es->frame != cg.snap->ps.clientNum || cg.renderingThirdPerson ||
			!cg_gunsmoke.integer)
			CG_MakeSmokePuff(es);
		break;
#endif

	case EV_GENERAL_SOUND:
		DEBUGNAME("EV_GENERAL_SOUND");
		if ( cgs.gameSounds[ es->eventParm ] ) {
			trap_S_StartSound (NULL, es->number, CHAN_VOICE, cgs.gameSounds[ es->eventParm ] );
		} else {
			s = CG_ConfigString( CS_SOUNDS + es->eventParm );
			trap_S_StartSound (NULL, es->number, CHAN_VOICE, CG_CustomSound( es->number, s ) );
		}
		break;

#ifdef SMOKINGUNS
	case EV_ROUND_START:
		DEBUGNAME("EV_ROUND_START");
		CG_DeleteRoundEntities();
		break;

	case EV_ROUND_TIME:
		DEBUGNAME("EV_ROUND_TIME");

		//if(es->time2)
			cg.roundendtime = es->time2;
		//if(es->time)
			cg.roundstarttime = es->time;
		break;

	case EV_HIT_CHECK:
		/*if(ai_nodepointer >= MAX_AINODES)
			break;

		VectorCopy(es->pos.trBase, ai_nodes[ai_nodepointer]);
		VectorCopy(es->angles, ai_angles[ai_nodepointer]);
		ai_nodepointer++;*/
		break;

	case EV_DUEL_WON:

		trap_S_StartSound(NULL, cg.snap->ps.clientNum, CHAN_ANNOUNCER,
			cgs.media.du_won_snd);
		break;


	case EV_DUEL_INTRO:
		DEBUGNAME("EV_DUEL_INTRO");

		// look if this info is for the current cgame
		/*if((int)es->angles[0] == cg.snap->ps.clientNum ||
			(int)es->angles[1] == cg.snap->ps.clientNum ||
			(int)es->origin[0] == cg.snap->ps.clientNum ||
			(int)es->origin[1] == cg.snap->ps.clientNum ){

			cg.currentmappart = es->eventParm;
		}*/

		cg.roundstarttime = es->time;
		cg.round = es->time2;
		break;
#endif

	case EV_GLOBAL_SOUND:	// play from the player's head so it never diminishes
		DEBUGNAME("EV_GLOBAL_SOUND");
		if ( cgs.gameSounds[ es->eventParm ] ) {
			trap_S_StartSound (NULL, cg.snap->ps.clientNum, CHAN_AUTO, cgs.gameSounds[ es->eventParm ] );
		} else {
			s = CG_ConfigString( CS_SOUNDS + es->eventParm );
			trap_S_StartSound (NULL, cg.snap->ps.clientNum, CHAN_AUTO, CG_CustomSound( es->number, s ) );
		}
		break;

	case EV_GLOBAL_TEAM_SOUND:	// play from the player's head so it never diminishes
		{
#ifndef SMOKINGUNS
			DEBUGNAME("EV_GLOBAL_TEAM_SOUND");
			switch( es->eventParm ) {
				case GTS_RED_CAPTURE: // CTF: red team captured the blue flag, 1FCTF: red team captured the neutral flag
					if ( cgs.clientinfo[cg.clientNum].team == TEAM_RED )
						CG_AddBufferedSound( cgs.media.captureYourTeamSound );
					else
						CG_AddBufferedSound( cgs.media.captureOpponentSound );
					break;
				case GTS_BLUE_CAPTURE: // CTF: blue team captured the red flag, 1FCTF: blue team captured the neutral flag
					if ( cgs.clientinfo[cg.clientNum].team == TEAM_BLUE )
						CG_AddBufferedSound( cgs.media.captureYourTeamSound );
					else
						CG_AddBufferedSound( cgs.media.captureOpponentSound );
					break;
				case GTS_RED_RETURN: // CTF: blue flag returned, 1FCTF: never used
					if ( cgs.clientinfo[cg.clientNum].team == TEAM_RED )
						CG_AddBufferedSound( cgs.media.returnYourTeamSound );
					else
						CG_AddBufferedSound( cgs.media.returnOpponentSound );
					//
					CG_AddBufferedSound( cgs.media.blueFlagReturnedSound );
					break;
				case GTS_BLUE_RETURN: // CTF red flag returned, 1FCTF: neutral flag returned
					if ( cgs.clientinfo[cg.clientNum].team == TEAM_BLUE )
						CG_AddBufferedSound( cgs.media.returnYourTeamSound );
					else
						CG_AddBufferedSound( cgs.media.returnOpponentSound );
					//
					CG_AddBufferedSound( cgs.media.redFlagReturnedSound );
					break;

				case GTS_RED_TAKEN: // CTF: red team took blue flag, 1FCTF: blue team took the neutral flag
					// if this player picked up the flag then a sound is played in CG_CheckLocalSounds
					if (cg.snap->ps.powerups[PW_BLUEFLAG] || cg.snap->ps.powerups[PW_NEUTRALFLAG]) {
					}
					else {
					if (cgs.clientinfo[cg.clientNum].team == TEAM_BLUE) {
#ifdef MISSIONPACK
							if (cgs.gametype == GT_1FCTF)
								CG_AddBufferedSound( cgs.media.yourTeamTookTheFlagSound );
							else
#endif
						 	CG_AddBufferedSound( cgs.media.enemyTookYourFlagSound );
						}
						else if (cgs.clientinfo[cg.clientNum].team == TEAM_RED) {
#ifdef MISSIONPACK
							if (cgs.gametype == GT_1FCTF)
								CG_AddBufferedSound( cgs.media.enemyTookTheFlagSound );
							else
#endif
 							CG_AddBufferedSound( cgs.media.yourTeamTookEnemyFlagSound );
						}
					}
					break;
				case GTS_BLUE_TAKEN: // CTF: blue team took the red flag, 1FCTF red team took the neutral flag
					// if this player picked up the flag then a sound is played in CG_CheckLocalSounds
					if (cg.snap->ps.powerups[PW_REDFLAG] || cg.snap->ps.powerups[PW_NEUTRALFLAG]) {
					}
					else {
						if (cgs.clientinfo[cg.clientNum].team == TEAM_RED) {
#ifdef MISSIONPACK
							if (cgs.gametype == GT_1FCTF)
								CG_AddBufferedSound( cgs.media.yourTeamTookTheFlagSound );
							else
#endif
							CG_AddBufferedSound( cgs.media.enemyTookYourFlagSound );
						}
						else if (cgs.clientinfo[cg.clientNum].team == TEAM_BLUE) {
#ifdef MISSIONPACK
							if (cgs.gametype == GT_1FCTF)
								CG_AddBufferedSound( cgs.media.enemyTookTheFlagSound );
							else
#endif
							CG_AddBufferedSound( cgs.media.yourTeamTookEnemyFlagSound );
						}
					}
					break;
				case GTS_REDOBELISK_ATTACKED: // Overload: red obelisk is being attacked
					if (cgs.clientinfo[cg.clientNum].team == TEAM_RED) {
						CG_AddBufferedSound( cgs.media.yourBaseIsUnderAttackSound );
					}
					break;
				case GTS_BLUEOBELISK_ATTACKED: // Overload: blue obelisk is being attacked
					if (cgs.clientinfo[cg.clientNum].team == TEAM_BLUE) {
						CG_AddBufferedSound( cgs.media.yourBaseIsUnderAttackSound );
					}
					break;

				case GTS_REDTEAM_SCORED:
					CG_AddBufferedSound(cgs.media.redScoredSound);
					break;
				case GTS_BLUETEAM_SCORED:
					CG_AddBufferedSound(cgs.media.blueScoredSound);
					break;
				case GTS_REDTEAM_TOOK_LEAD:
					CG_AddBufferedSound(cgs.media.redLeadsSound);
					break;
				case GTS_BLUETEAM_TOOK_LEAD:
					CG_AddBufferedSound(cgs.media.blueLeadsSound);
					break;
				case GTS_TEAMS_ARE_TIED:
					CG_AddBufferedSound( cgs.media.teamsTiedSound );
					break;
#ifdef MISSIONPACK
				case GTS_KAMIKAZE:
					trap_S_StartLocalSound(cgs.media.kamikazeFarSound, CHAN_ANNOUNCER);
					break;
#endif
				default:
					break;
			}
#endif
			break;
		}

	case EV_PAIN:
		// local player sounds are triggered in CG_CheckLocalSounds,
		// so ignore events on the player
		DEBUGNAME("EV_PAIN");
		if ( cent->currentState.number != cg.snap->ps.clientNum ) {
			CG_PainEvent( cent, es->eventParm );
		}
		break;

#ifndef SMOKINGUNS
	case EV_DEATH1:
	case EV_DEATH2:
	case EV_DEATH3:
		DEBUGNAME("EV_DEATHx");
		trap_S_StartSound( NULL, es->number, CHAN_VOICE, 
				CG_CustomSound( es->number, va("*death%i.wav", event - EV_DEATH1 + 1) ) );
#else
	case EV_DEATH_DEFAULT:
	case EV_DEATH_HEAD:
	case EV_DEATH_ARM_L:
	case EV_DEATH_ARM_R:
	case EV_DEATH_CHEST:
	case EV_DEATH_STOMACH:
	case EV_DEATH_LEG_L:
	case EV_DEATH_LEG_R:
	case EV_DEATH_FALL:
	case EV_DEATH_FALL_BACK:

		DEBUGNAME("EV_DEATHx");
		trap_S_StartSound( NULL, es->number, CHAN_VOICE,
			cgs.media.snd_death[event-EV_DEATH_DEFAULT] );
		break;

	case EV_HIT_MESSAGE:
		DEBUGNAME("EV_HIT_MESSAGE");
		CG_Hit_Message( es);
#endif
		break;

	case EV_OBITUARY:
		DEBUGNAME("EV_OBITUARY");
		CG_Obituary( es );
		break;

	//
	// powerup events
	//
	case EV_POWERUP_QUAD:
		DEBUGNAME("EV_POWERUP_QUAD");
#ifndef SMOKINGUNS
		if ( es->number == cg.snap->ps.clientNum ) {
			cg.powerupActive = PW_QUAD;
			cg.powerupTime = cg.time;
		}
		trap_S_StartSound (NULL, es->number, CHAN_ITEM, cgs.media.quadSound );
#endif
		break;
	case EV_POWERUP_BATTLESUIT:
		DEBUGNAME("EV_POWERUP_BATTLESUIT");
#ifndef SMOKINGUNS
		if ( es->number == cg.snap->ps.clientNum ) {
			cg.powerupActive = PW_BATTLESUIT;
			cg.powerupTime = cg.time;
		}
		trap_S_StartSound (NULL, es->number, CHAN_ITEM, cgs.media.protectSound );
#endif
		break;
	case EV_POWERUP_REGEN:
		DEBUGNAME("EV_POWERUP_REGEN");
#ifndef SMOKINGUNS
		if ( es->number == cg.snap->ps.clientNum ) {
			cg.powerupActive = PW_REGEN;
			cg.powerupTime = cg.time;
		}
		trap_S_StartSound (NULL, es->number, CHAN_ITEM, cgs.media.regenSound );
#endif
		break;

	case EV_GIB_PLAYER:
		DEBUGNAME("EV_GIB_PLAYER");
		// don't play gib sound when using the kamikaze because it interferes
		// with the kamikaze sound, downside is that the gib sound will also
		// not be played when someone is gibbed while just carrying the kamikaze
#ifndef SMOKINGUNS
		if ( !(es->eFlags & EF_KAMIKAZE) ) {
			trap_S_StartSound( NULL, es->number, CHAN_BODY, cgs.media.gibSound );
		}
#endif
		CG_GibPlayer( cent->lerpOrigin );
		break;

	case EV_STOPLOOPINGSOUND:
		DEBUGNAME("EV_STOPLOOPINGSOUND");
		trap_S_StopLoopingSound( es->number );
		es->loopSound = 0;
		break;

	case EV_DEBUG_LINE:
		DEBUGNAME("EV_DEBUG_LINE");
		CG_Beam( cent );
		break;

#ifdef SMOKINGUNS
	case EV_NOTHING:
		DEBUGNAME("EV_NOTHING");
		CG_Printf("who called that event 77?\n");
		break;
#endif

	default:
		DEBUGNAME("UNKNOWN");
		CG_Error( "Unknown event: %i", event );
		break;
	}

}


/*
==============
CG_CheckEvents

==============
*/
void CG_CheckEvents( centity_t *cent ) {
	// check for event-only entities
	if ( cent->currentState.eType > ET_EVENTS ) {
		if ( cent->previousEvent ) {
			return;	// already fired
		}
#ifndef SMOKINGUNS
		// if this is a player event set the entity number of the client entity number
		if ( cent->currentState.eFlags & EF_PLAYER_EVENT ) {
			cent->currentState.number = cent->currentState.otherEntityNum;
		}
#endif

		cent->previousEvent = 1;

		cent->currentState.event = cent->currentState.eType - ET_EVENTS;
	} else {
		// check for events riding with another entity
		if ( cent->currentState.event == cent->previousEvent ) {
			return;
		}
		cent->previousEvent = cent->currentState.event;
		if ( ( cent->currentState.event & ~EV_EVENT_BITS ) == 0 ) {
			return;
		}
	}

	// calculate the position at exactly the frame time
	BG_EvaluateTrajectory( &cent->currentState.pos, cg.snap->serverTime, cent->lerpOrigin );
	CG_SetEntitySoundPosition( cent );

	CG_EntityEvent( cent, cent->lerpOrigin );
}

