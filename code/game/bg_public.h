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
// bg_public.h -- definitions shared by both the server game and client game modules

// because games can change separately from the main system version, we need a
// second version that must match between game and cgame

#ifndef SDK_BASEGAME
#define	GAME_VERSION		BASEGAME "-1"
#else
#define	GAME_VERSION		SDK_BASEGAME "-1"
#endif

#ifndef SMOKINGUNS
#define	DEFAULT_GRAVITY		800
#else
#define ACTIVATE_RANGE 45

#define	DEFAULT_GRAVITY		900
#endif
#define	GIB_HEALTH			-40
#define	ARMOR_PROTECTION	0.66

#define	MAX_ITEMS			256

#define	RANK_TIED_FLAG		0x4000

//dynamite
#ifdef SMOKINGUNS
#define DYN_SPLASH_DAM			150
#define DYN_SPLASH_RAD			200
#define DYN_DAM					200

// scope
#define	SCOPE_TIME				400

//molotov
#define	MOLOTOV_BURNTIME		30000  // burning in hand
#define	WHISKEY_BURNTIME		15000  // whiskey pool
#define WHISKEY_SICKERTIME		60000  // time till whiskey sinks into the ground
#define ALC_COUNT				8 // number of "sprotzs" of alcohol

#define	MAX_DEATH_MESSAGES		4

//boiler plate
#define BOILER_PLATE 60

#define	MAX_BULLET_DISTANCE		800
#endif

#define DEFAULT_SHOTGUN_SPREAD	700
#define DEFAULT_SHOTGUN_COUNT	11

//gatling gun
#ifdef SMOKINGUNS
#define TRIPOD_TIME			750
#define	STAGE_TIME			500

#define GATLING_RANGE	20
#define	GATLING_DOWN	23.0
#define	GATLING_UP		23.0
#define GATLING_SIDE	70.0

// duel stats
#define DU_INTRO_CAM		7000.0f
#define DU_INTRO_DRAW		3000.0f
#define	DU_CROSSHAIR_START	1500.0f
#define DU_CROSSHAIR_FADE	4500.0f

#define MEDAL_TIME 3000

#define DU_WP_STEP			2

#define DEFAULT_MODEL				"wq_male1"
#define DEFAULT_TEAM_MODEL			"wq_male1"
#define DEFAULT_TEAM_HEAD			"wq_male1"
#define DEFAULT_REDTEAM_NAME		"Lawmen"
#define DEFAULT_BLUETEAM_NAME		"Outlaws"

#define	FALLOFF_RANGE	150

// total intro time of 10000

/*
-----------------------
MONEY_SYSTEM-VALUES
by Spoon
-----------------------
*/
//#define MAX_MONEY				200.00	// now, it is stored in the cvar g_maxMoney
#define	MIN_MONEY				"20"
#define DU_MIN_MONEY			20.00
#define SOCIAL_MONEY			28.00

#define MAX_REWARD				"20"
#define	LOSE_MONEY				7.00

#define	ROUND_WIN_MONEY			"16"
#define	ROUND_LOSE_MONEY		"10"

//buy properties
#define	BUY_TIME				60000

//money-system end

// Reward for player who rob the bank (added by Joe Kari)
#define MAX_POINT_ROBBER_REWARD			5
#define MIN_POINT_ROBBER_REWARD			0
#endif


#define	ITEM_RADIUS			15		// item sizes are needed for client side pickup detection

#define	LIGHTNING_RANGE		768

#define	SCORE_NOT_PRESENT	-9999	// for the CS_SCORES[12] when only one player is present

#define	VOTE_TIME			30000	// 30 seconds before vote times out

#define	MINS_Z				-24
#ifdef SMOKINGUNS
// Tequila: maxs bbox will be added on Z axis with this for the hit detection system
#define MAXS_Z_BBOX_UPDATE	10
#define MAXS_Z				28
#endif
#define	DEFAULT_VIEWHEIGHT	26
#define CROUCH_VIEWHEIGHT	12
#define	DEAD_VIEWHEIGHT		-16

//
// config strings are a general means of communicating variable length strings
// from the server to all connected clients.
//

// CS_SERVERINFO and CS_SYSTEMINFO are defined in q_shared.h
#define	CS_MUSIC				2
#define	CS_MESSAGE				3		// from the map worldspawn's message field
#define	CS_MOTD					4		// g_motd string for server message of the day
#define	CS_WARMUP				5		// server time when the match will be restarted
#define	CS_SCORES1				6
#define	CS_SCORES2				7
#define CS_VOTE_TIME			8
#define CS_VOTE_STRING			9
#define	CS_VOTE_YES				10
#define	CS_VOTE_NO				11

#define CS_TEAMVOTE_TIME		12
#define CS_TEAMVOTE_STRING		14
#define	CS_TEAMVOTE_YES			16
#define	CS_TEAMVOTE_NO			18

#define	CS_GAME_VERSION			20
#define	CS_LEVEL_START_TIME		21		// so the timer only shows the current level
#define	CS_INTERMISSION			22		// when 1, fraglimit/timelimit has been hit and intermission will start in a second or two
#define CS_FLAGSTATUS			23		// string indicating flag status in CTF
#define CS_SHADERSTATE			24
#define CS_BOTINFO				25

#define	CS_ITEMS				27		// string of 0's and 1's that tell which items are present

#ifdef SMOKINGUNS
#define	CS_MAPCYCLES			29		// for the map cycle vote menu
#endif

#define	CS_MODELS				32
#define	CS_SOUNDS				(CS_MODELS+MAX_MODELS)
#define	CS_PLAYERS				(CS_SOUNDS+MAX_SOUNDS)
#define CS_LOCATIONS			(CS_PLAYERS+MAX_CLIENTS)
#define CS_PARTICLES			(CS_LOCATIONS+MAX_LOCATIONS)

#define CS_MAX					(CS_PARTICLES+MAX_LOCATIONS)

#if (CS_MAX) > MAX_CONFIGSTRINGS
#error overflow: (CS_MAX) > MAX_CONFIGSTRINGS
#endif

typedef enum {
	GT_FFA,				// free for all
#ifndef SMOKINGUNS
	GT_TOURNAMENT,		// one on one tournament
#else
	GT_DUEL,		// duelling mode
#endif
	GT_SINGLE_PLAYER,	// single player ffa

	//-- team games go after this --

	GT_TEAM,			// team deathmatch
#ifndef SMOKINGUNS
	GT_CTF,				// capture the flag
	GT_MAX_GAME_TYPE,		// other gametypes haven't been implemented.
	GT_1FCTF,
	GT_OBELISK,
	GT_HARVESTER
#else
	GT_RTP,				// round teamplay
	GT_BR,				// bank robbery
	GT_MAX_GAME_TYPE
#endif
} gametype_t;

typedef enum { GENDER_MALE, GENDER_FEMALE, GENDER_NEUTER } gender_t;

/*
-----------------------
Far clipping form definition
(by Joe Kari)
-----------------------
*/
// !! Don't re-order this: it is used in this order for a function pointer array !!
#ifdef SMOKINGUNS
typedef enum {
	FARCLIP_NONE ,				// no clipping
	FARCLIP_ALWAYS ,			// always clip
	FARCLIP_SPHERE ,			// Standard default spherical clipping
	FARCLIP_ELLIPSE_X ,			// X-axis rescaled sphere
	FARCLIP_ELLIPSE_Y ,			// Y-axis rescaled
	FARCLIP_ELLIPSE_Z ,			// Z-axis rescaled
	FARCLIP_CYLINDER_X ,		// X-axis cylinder for its axis length)
	FARCLIP_CYLINDER_Y ,		// Y-axis 
	FARCLIP_CYLINDER_Z ,		// Z-axis 
	FARCLIP_CUBE ,				// Cubic clipping (along axis), bounding-box clipping
	FARCLIP_BOX_X ,				// X-axis rescaled cube
	FARCLIP_BOX_Y ,			
	FARCLIP_BOX_Z ,
	FARCLIP_CONE_X ,			// distance along X-axis is substracted from YZ-axis distance calculation
	FARCLIP_CONE_Y ,
	FARCLIP_CONE_Z ,			// could be usefull for clipping grass brush/models
	FARCLIP_PYRAMID_X ,			// distance along X-axis is substracted from YZ-axis bounding box calculation
	FARCLIP_PYRAMID_Y ,
	FARCLIP_PYRAMID_Z ,
	FARCLIP_CIRCLE_INFINITE_X ,	// X-axis is not used while calculating distance (cylinder with infinite length)
	FARCLIP_CIRCLE_INFINITE_Y ,
	FARCLIP_CIRCLE_INFINITE_Z ,
	FARCLIP_SQUARE_INFINITE_X ,	// X-axis is not used while calculating bounding box (box with infinite length on X-axis)
	FARCLIP_SQUARE_INFINITE_Y ,
	FARCLIP_SQUARE_INFINITE_Z
} farclip_t ;

#define CLOSECLIP_BINARY_MASK	0x80		// bit on => this is a close-clip rather than a far-clip

/*
-----------------------
Map LOD (Level Of Detail)
(by Joe Kari)
-----------------------
*/
#define MAPLOD_BINARY_MASK	0x400		// bit on => LOD definition, bit off => no LOD definition: always displayed
#define	MAPLOD_GTE_BINARY_MASK	0x800 	// bit on => object is displayed if LOD is GreaTer or Equal (GTE) to the value
										// bit off => object is displayed if LOD is less or equal to the value
#endif



/*
===================================================================================

PMOVE MODULE

The pmove code takes a player_state_t and a usercmd_t and generates a new player_state_t
and some other output data.  Used for local prediction on the client game and true
movement on the server game.
===================================================================================
*/

typedef enum {
	PM_NORMAL,		// can accelerate and turn
	PM_NOCLIP,		// noclip movement
	PM_SPECTATOR,	// still run into walls
	PM_DEAD,		// no acceleration or turning, but free falling
	PM_FREEZE,		// stuck in place with no control
	PM_INTERMISSION,	// no movement or status bar
#ifndef SMOKINGUNS
	PM_SPINTERMISSION	// no movement or status bar
#else
	PM_SPINTERMISSION,	// no movement or status bar
	PM_CHASECAM		// chasecam by Spoon
#endif
} pmtype_t;

typedef enum {
	WEAPON_READY,
	WEAPON_RAISING,
	WEAPON_DROPPING,
#ifndef SMOKINGUNS
	WEAPON_FIRING
#else
	WEAPON_FIRING,
	WEAPON_RELOADING,
	WEAPON_JUMPING,
#endif
} weaponstate_t;

// pmove->pm_flags
#define	PMF_DUCKED			1
#define	PMF_JUMP_HELD		2
#define	PMF_BACKWARDS_JUMP	8		// go into backwards land
#define	PMF_BACKWARDS_RUN	16		// coast down to backwards run
#define	PMF_TIME_LAND		32		// pm_time is time before rejump
#define	PMF_TIME_KNOCKBACK	64		// pm_time is an air-accelerate only time
#define	PMF_TIME_WATERJUMP	256		// pm_time is waterjump
#define	PMF_RESPAWNED		512		// clear after attack and jump buttons come up
#define	PMF_USE_ITEM_HELD	1024
#define PMF_GRAPPLE_PULL	2048	// pull towards grapple location
#define PMF_FOLLOW			4096	// spectate following another player
#define PMF_SCOREBOARD		8192	// spectate as a scoreboard
#define PMF_INVULEXPAND		16384	// invulnerability sphere set to full size
#ifdef SMOKINGUNS
#define PMF_SUICIDE			32768
#endif

#define	PMF_ALL_TIMES	(PMF_TIME_WATERJUMP|PMF_TIME_LAND|PMF_TIME_KNOCKBACK)

#ifdef SMOKINGUNS
#define	KNOCKTIME			500		// time player can't move with normal speed after being hit
#endif

#define	MAXTOUCH	32
typedef struct {
	// state (in / out)
	playerState_t	*ps;

	// command (in)
	usercmd_t	cmd;
	int			tracemask;			// collide against these types of surfaces
	int			debugLevel;			// if set, diagnostic output will be printed
	qboolean	noFootsteps;		// if the game is setup for no footsteps by the server
	qboolean	gauntletHit;		// true if a gauntlet attack would actually hit something

	int			framecount;

	// results (out)
	int			numtouch;
	int			touchents[MAXTOUCH];

	vec3_t		mins, maxs;			// bounding box size

	int			watertype;
	int			waterlevel;

	float		xyspeed;

	// for fixed msec Pmove
	int			pmove_fixed;
	int			pmove_msec;

	// callbacks to test the world
	// these will be different functions during game and cgame
	void		(*trace)( trace_t *results, const vec3_t start, const vec3_t mins, const vec3_t maxs, const vec3_t end, int passEntityNum, int contentMask );
	int			(*pointcontents)( const vec3_t point, int passEntityNum );
} pmove_t;

// if a full pmove isn't done on the client, you can just update the angles
void PM_UpdateViewAngles( playerState_t *ps, const usercmd_t *cmd );
void Pmove (pmove_t *pmove);

//===================================================================================


// player_state->stats[] indexes
// NOTE: may not have more than 16
typedef enum {
	STAT_HEALTH,
	STAT_HOLDABLE_ITEM,
#ifndef SMOKINGUNS
	STAT_PERSISTANT_POWERUP,
#endif
	STAT_WEAPONS,					// 16 bit fields
	STAT_ARMOR,
#ifndef SMOKINGUNS
	STAT_DEAD_YAW,				// look this direction when dead (FIXME: get rid of?)
#endif
	STAT_CLIENTS_READY,				// bit mask of clients wishing to exit the intermission (FIXME: configstring?)
#ifndef SMOKINGUNS
	STAT_MAX_HEALTH					// health / armor limit, changable by handicap
#else
	STAT_MAX_HEALTH,				// health / armor limit, changable by handicap

	STAT_WP_MODE,
	STAT_GATLING_MODE,
	CHASECLIENT,
	STAT_DELAY_TIME,
	IDLE_TIMER,
	STAT_MONEY,
	STAT_OLDWEAPON,
	STAT_KNOCKTIME,
	STAT_FLAGS,
	STAT_WINS
#endif
} statIndex_t;

// stat flags
#ifdef SMOKINGUNS
#define SF_CANBUY		0x00001 // is player able to buy something?
#define SF_SEC_PISTOL	0x00002 // has player a second pistol, equal to the first one?
#define SF_GAT_CHANGE	0x00004 // player has taken a built-up gatling -> play WP_ANIM_CHANGE
#define SF_RELOAD		0x00008 // reload animation of weapon1 must still be started (akimbos)
#define SF_RELOAD2		0x00010 // reload animation of weapon2 must still be started (akimbos)
#define SF_DU_INTRO		0x00020	// player is in intro mode, don't update the viewangles
#define	SF_DU_WON		0x00040	// player has won in duel and is now able to spectate
#define	SF_WP_CHOOSE	0x00080	// this player is in weapon choose mode don't react to BUTTON_ATTACK
#define SF_BOT			0x00100	// player is a bot
#define	SF_DUELINTRO	0x00200	// player is in a intro of a duel
#define	SF_GAT_CARRY	0x00400
#endif

// player_state->persistant[] indexes
// these fields are the only part of player_state that isn't
// cleared on respawn
// NOTE: may not have more than 16
typedef enum {
	PERS_SCORE,						// !!! MUST NOT CHANGE, SERVER AND GAME BOTH REFERENCE !!!
	PERS_HITS,						// total points damage inflicted so damage beeps can sound on change
	PERS_RANK,						// player rank or team rank
	PERS_TEAM,						// player team
	PERS_SPAWN_COUNT,				// incremented every respawn
	PERS_PLAYEREVENTS,				// 16 bits that can be flipped for events
	PERS_ATTACKER,					// clientnum of last damage inflicter
	PERS_ATTACKEE_ARMOR,			// health/armor of last person we attacked
	PERS_KILLED,					// count of the number of times you died
	// player awards tracking
#ifndef SMOKINGUNS
	PERS_IMPRESSIVE_COUNT,			// two railgun hits in a row
	PERS_EXCELLENT_COUNT,			// two successive kills in a short amount of time
	PERS_DEFEND_COUNT,				// defend awards
	PERS_ASSIST_COUNT,				// assist awards
	PERS_GAUNTLET_FRAG_COUNT,		// kills with the guantlet
	PERS_CAPTURES					// captures
#else
	PERS_ROBBER				// 1 if the player can rob the bank
#endif
} persEnum_t;


// entityState_t->eFlags
#define	EF_DEAD				0x00000001		// don't draw a foe marker over players with EF_DEAD
#ifndef SMOKINGUNS
#define EF_TICKING			0x00000002		// used to make players play the prox mine ticking sound
#endif
#define	EF_TELEPORT_BIT		0x00000004		// toggled every time the origin abruptly changes
#define	EF_AWARD_EXCELLENT	0x00000008		// draw an excellent sprite
#define EF_SAME_TEAM		0x00000008	// Joe Kari: EV_OBITUARY + EF_SAME_TEAM = TeamKilling
						// Note that it is the same than EF_AWARD_EXCELLENT that is not use in SG
#ifndef SMOKINGUNS
#define EF_PLAYER_EVENT		0x00000010
#endif
#define	EF_BOUNCE			0x00000010		// for missiles
#define	EF_BOUNCE_HALF		0x00000020		// for missiles
#define	EF_AWARD_GAUNTLET	0x00000040		// draw a gauntlet sprite
#define	EF_NODRAW			0x00000080		// may have an event, but no model (unspawned items)
#define	EF_FIRING			0x00000100		// for lightning gun
#define	EF_KAMIKAZE			0x00000200
#define	EF_MOVER_STOP		0x00000400		// will push otherwise
#define EF_AWARD_CAP		0x00000800		// draw the capture sprite
#define	EF_TALK				0x00001000		// draw a talk balloon
#define	EF_CONNECTION		0x00002000		// draw a connection trouble sprite
#define	EF_VOTED			0x00004000		// already cast a vote
#define	EF_AWARD_IMPRESSIVE	0x00008000		// draw an impressive sprite
#define	EF_AWARD_DEFEND		0x00010000		// draw a defend sprite
#define	EF_AWARD_ASSIST		0x00020000		// draw a assist sprite
#define EF_AWARD_DENIED		0x00040000		// denied
#define EF_TEAMVOTED		0x00080000		// already cast a team vote
#ifdef SMOKINGUNS
#define	EF_HIT_MESSAGE		0x00100000		// a hit message of several locations is being sent

//Spoon flags
#define	EF_RELOAD			0x00100000		// if reloading
#define EF_RELOAD2			0x00200000		// if weapon2 is reloading
#define EF_BROKEN			0x00400000		// broken func_breakable just too respawn on roundstart
#define	EF_SOUNDOFF			0x00800000		// to restart sound on round restart

#define	EF_ROUND_START		0x01000000		// round start
#define	EF_DROPPED_ITEM		0x02000000		// marks dropped item
#define	EF_BUY				0x04000000
#define	EF_ACTIVATE			0x08000000

// Tequila flags
#define EF_ROTATING_DOOR	0x10000000	// Tequila: is a rotating door
#endif



// NOTE: may not have more than 16
typedef enum {
	PW_NONE,

#ifndef SMOKINGUNS
	PW_QUAD,
	PW_BATTLESUIT,
	PW_HASTE,
	PW_INVIS,
	PW_REGEN,
	PW_FLIGHT,

	PW_REDFLAG,
	PW_BLUEFLAG,
	PW_NEUTRALFLAG,

	PW_SCOUT,
	PW_GUARD,
	PW_DOUBLER,
	PW_AMMOREGEN,
	PW_INVULNERABILITY,
#else
	// normal powerups
	PW_SCOPE,
	PW_BELT,
	PW_GOLD,

	//motion stats
	//MV_SLOW,

	//damage stats
	DM_HEAD_1,
	DM_HEAD_2,
	DM_TORSO_1,
	DM_TORSO_2,
	DM_LEGS_1,
	DM_LEGS_2,

	PW_GATLING, // has the player a gatling?

	// these PW's are only for powerups of the entitystate
	PW_BURN,	// burning dynamite/molotov
	PW_BURNBIT,
#endif

	PW_NUM_POWERUPS

} powerup_t;

#ifndef SMOKINGUNS
// Tequila comment: not used in Smokin' Guns
typedef enum {
	HI_NONE,

	HI_TELEPORTER,
	HI_MEDKIT,
	HI_KAMIKAZE,
	HI_PORTAL,
	HI_INVULNERABILITY,

	HI_NUM_HOLDABLE
} holdable_t;
#endif


typedef enum {
	WP_NONE,

#ifndef SMOKINGUNS
	WP_GAUNTLET,
	WP_MACHINEGUN,
	WP_SHOTGUN,
	WP_GRENADE_LAUNCHER,
	WP_ROCKET_LAUNCHER,
	WP_LIGHTNING,
	WP_RAILGUN,
	WP_PLASMAGUN,
	WP_BFG,
	WP_GRAPPLING_HOOK,
#ifdef MISSIONPACK
	WP_NAILGUN,
	WP_PROX_LAUNCHER,
	WP_CHAINGUN,
#endif

	WP_NUM_WEAPONS
#else
	//melee
	WP_KNIFE,

	//pistols
	WP_REM58,
	WP_SCHOFIELD,
	WP_PEACEMAKER,

	//rifles
	WP_WINCHESTER66,	// should always be the first weapon after pistols
	WP_LIGHTNING,
	WP_SHARPS,

	//shotguns
	WP_REMINGTON_GAUGE,
	WP_SAWEDOFF,
	WP_WINCH97,

	//automatics
	WP_GATLING,

	//explosives
	WP_DYNAMITE,	// this always should be the last weapon after the special weapons
	WP_MOLOTOV,		// has to be the last weapon in here, because of the +prevweap-cmd

	WP_NUM_WEAPONS,

	WP_AKIMBO,

	WP_BULLETS_CLIP,
	WP_SHELLS_CLIP,
	WP_CART_CLIP,
	WP_GATLING_CLIP,
	WP_SHARPS_CLIP, // 21 !! no more place in weapons-storage !

	// For representing the left side pistol when the player has two of the
	// same kind.  This is NOT a valid index for playerState_t.ammo.
	WP_SEC_PISTOL
#endif
} weapon_t;


// reward sounds (stored in ps->persistant[PERS_PLAYEREVENTS])
#define	PLAYEREVENT_DENIEDREWARD		0x0001
#define	PLAYEREVENT_GAUNTLETREWARD		0x0002
#define PLAYEREVENT_HOLYSHIT			0x0004

// entityState_t->event values
// entity events are for effects that take place relative
// to an existing entities origin.  Very network efficient.

// two bits at the top of the entityState->event field
// will be incremented with each change in the event so
// that an identical event started twice in a row can
// be distinguished.  And off the value with ~EV_EVENT_BITS
// to retrieve the actual event number
#define	EV_EVENT_BIT1		0x00000100
#define	EV_EVENT_BIT2		0x00000200
#define	EV_EVENT_BITS		(EV_EVENT_BIT1|EV_EVENT_BIT2)

#define	EVENT_VALID_MSEC	300

typedef enum {
	EV_NONE,

	EV_FOOTSTEP,
	EV_FOOTSTEP_METAL,
#ifdef SMOKINGUNS
	EV_FOOTSTEP_CLOTH,
	EV_FOOTSTEP_GRASS,
	EV_FOOTSTEP_SNOW, //5
	EV_FOOTSTEP_SAND,
#endif
	EV_FOOTSPLASH,
	EV_FOOTWADE,
	EV_SWIM,

	EV_STEP_4, //10
	EV_STEP_8,
	EV_STEP_12,
	EV_STEP_16,

#ifdef SMOKINGUNS
	EV_FALL_VERY_SHORT,
#endif
	EV_FALL_SHORT, //15
	EV_FALL_MEDIUM,
	EV_FALL_FAR,
#ifdef SMOKINGUNS
	EV_FALL_VERY_FAR,
#endif

	EV_JUMP_PAD,			// boing sound at origin, jump sound on player

	EV_JUMP, // 20
	EV_WATER_TOUCH,	// foot touches
	EV_WATER_LEAVE,	// foot leaves
	EV_WATER_UNDER,	// head touches
	EV_WATER_CLEAR,	// head leaves

	EV_ITEM_PICKUP,			// normal item pickups are predictable //25
#ifndef SMOKINGUNS
	EV_GLOBAL_ITEM_PICKUP,	// powerup / team sounds are broadcast to everyone
#else
	EV_MONEY_PICKUP,
#endif

	EV_NOAMMO,
#ifdef SMOKINGUNS
	EV_CHOOSE_WEAPON,
#endif
	EV_CHANGE_WEAPON,
#ifdef SMOKINGUNS
	EV_CHANGE_WEAPON_NOANIM, // 30
	EV_CHANGETO_SOUND,
	EV_CHANGE_TO_WEAPON,
#endif
	EV_FIRE_WEAPON,
#ifdef SMOKINGUNS
	EV_FIRE_WEAPON_DELAY,
	EV_FIRE_WEAPON2,	//35
	EV_ALT_FIRE_WEAPON,
	EV_ANIM_WEAPON,
	EV_ALT_WEAPON_MODE,
	EV_BUILD_TURRET,
	EV_DISM_TURRET,	//40
	EV_SCOPE_PUT,
	EV_LIGHT_DYNAMITE,
	EV_RELOAD,
	EV_RELOAD2,
	EV_GATLING_NOTPLANAR,	//45
	EV_GATLING_NOSPACE,
	EV_GATLING_TOODEEP,

	EV_MARKED_ITEM,

	EV_UNDERWATER, //added by Spoon

	//other weapon events
	EV_DBURN,	// 50
	EV_KNIFEHIT,
	EV_NOAMMO_CLIP,
#endif

	EV_USE_ITEM0,
	EV_USE_ITEM1,
	EV_USE_ITEM2,  // 55
	EV_USE_ITEM3,
	EV_USE_ITEM4,
	EV_USE_ITEM5,
	EV_USE_ITEM6,
	EV_USE_ITEM7,  // 60
	EV_USE_ITEM8,
	EV_USE_ITEM9,
	EV_USE_ITEM10,
	EV_USE_ITEM11,
	EV_USE_ITEM12,  // 65
	EV_USE_ITEM13,
	EV_USE_ITEM14,
	EV_USE_ITEM15,

	EV_ITEM_RESPAWN,
	EV_ITEM_POP,  // 70
	EV_PLAYER_TELEPORT_IN,
	EV_PLAYER_TELEPORT_OUT,

	EV_GRENADE_BOUNCE,		// eventParm will be the soundindex

	EV_GENERAL_SOUND,
	EV_GLOBAL_SOUND,		// 75, no attenuation
	EV_GLOBAL_TEAM_SOUND,
#ifdef SMOKINGUNS
	EV_ROUND_START,
	EV_ROUND_TIME,
	EV_DUEL_INTRO,
	EV_DUEL_WON,  // 80

	EV_HIT_CHECK,

	//func events
	EV_FUNCBREAKABLE,
#endif

	EV_BULLET_HIT_FLESH,
	EV_BULLET_HIT_WALL,

#ifdef SMOKINGUNS
	EV_BULLET_HIT_BOILER,  // 85
	EV_BOILER_HIT,

	EV_WHISKEY_BURNS,
	EV_MISSILE_ALCOHOL,
	EV_MISSILE_FIRE,
#endif
	EV_MISSILE_HIT,  // 90
	EV_MISSILE_MISS,
	EV_MISSILE_MISS_METAL,
#ifndef SMOKINGUNS
	EV_RAILTRAIL,
#endif
	EV_SHOTGUN,
	EV_BULLET,				// otherEntity is the shooter

	EV_SMOKE,				// 95, make smoke puff

	EV_PAIN,
#ifndef SMOKINGUNS
	EV_DEATH1,
	EV_DEATH2,
	EV_DEATH3,
#else
	EV_DEATH_DEFAULT,
	EV_DEATH_HEAD,
	EV_DEATH_ARM_L,
	EV_DEATH_ARM_R,	// 100
	EV_DEATH_CHEST,
	EV_DEATH_STOMACH,
	EV_DEATH_LEG_L,
	EV_DEATH_LEG_R,
	EV_DEATH_FALL,	// 105
	EV_DEATH_FALL_BACK,
#endif
	EV_OBITUARY,
#ifdef SMOKINGUNS
	EV_HIT_MESSAGE,
#endif

	EV_POWERUP_QUAD,
	EV_POWERUP_BATTLESUIT,	// 110
	EV_POWERUP_REGEN,

	EV_GIB_PLAYER,			// gib a previously living player
#ifndef SMOKINGUNS
	EV_SCOREPLUM,			// score plum

//#ifdef MISSIONPACK
	EV_PROXIMITY_MINE_STICK,
	EV_PROXIMITY_MINE_TRIGGER,
	EV_KAMIKAZE,			// kamikaze explodes
	EV_OBELISKEXPLODE,		// obelisk explodes
	EV_OBELISKPAIN,			// obelisk is in pain
	EV_INVUL_IMPACT,		// invulnerability sphere impact
	EV_JUICED,				// invulnerability juiced effect
	EV_LIGHTNINGBOLT,		// lightning bolt bounced of invulnerability sphere
//#endif

	EV_DEBUG_LINE,
	EV_STOPLOOPINGSOUND,
	EV_TAUNT,
	EV_TAUNT_YES,
	EV_TAUNT_NO,
	EV_TAUNT_FOLLOWME,
	EV_TAUNT_GETFLAG,
	EV_TAUNT_GUARDBASE,
	EV_TAUNT_PATROL
#else
	EV_DEBUG_LINE,
	EV_STOPLOOPINGSOUND,	// 115
	EV_TAUNT,
	EV_DEBUG_BULLET,
	EV_HIT_FAR,
	EV_NOTHING
#endif

} entity_event_t;


typedef enum {
	GTS_RED_CAPTURE,
	GTS_BLUE_CAPTURE,
	GTS_RED_RETURN,
	GTS_BLUE_RETURN,
	GTS_RED_TAKEN,
	GTS_BLUE_TAKEN,
	GTS_REDOBELISK_ATTACKED,
	GTS_BLUEOBELISK_ATTACKED,
	GTS_REDTEAM_SCORED,
	GTS_BLUETEAM_SCORED,
	GTS_REDTEAM_TOOK_LEAD,
	GTS_BLUETEAM_TOOK_LEAD,
	GTS_TEAMS_ARE_TIED,
	GTS_KAMIKAZE
} global_team_sound_t;

/*
// ---BOTH--- //

0	41	0	15	// BOTH_DEATH_DEFAULT
41	46	0	15	// BOTH_DEATH_HEAD
87	40	0	15	// BOTH_DEATH_ARM_L
127	40	0	15	// BOTH_DEATH_ARM_R
167	41	0	15	// BOTH_DEATH_CHEST
208	45	0	20	// BOTH_DEATH_STOMACH
253	56	0	15	// BOTH_DEATH_LEG_L
309	56	0	15	// BOTH_DEATH_LEG_R
365	16	0	15	// BOTH_FALL_BACK
381	14	0	15	// BOTH_FALL
395	42	0	15	// BOTH_DEATH_LAND
437	9	0	15	// BOTH_JUMP
446	10	0	15	// BOTH_LAND
456	16	0	15	// BOTH_LADDER



// ---TORSO--- //

472	1	0	15	// TORSO_HOLSTERED
473	13	0	10	// TORSO_KNIFE_ATTACK
486	7	0	10	// TORSO_THROW

493	7	0	15	// TORSO_PISTOL_ATTACK
500	11	0	10	// TORSO_PISTOL_RELOAD
511	8	0	15	// TORSO_PISTOLS_ATTACK

519	6	0	15	// TORSO_RIFLE_ATTACK
525	8	0	10	// TORSO_RIFLE_RELOAD

533	11	0	10	// TORSO_GATLING_FIRE
544	15	0	10	// TORSO_GATLING_RELOAD

559	8	0	15	// TORSO_PISTOL_DROP
567	7	0	15	// TORSO_PISTOL_RAISE
574	7	0	15	// TORSO_RIFLE_DROP
581	10	0	15	// TORSO_RIFLE_RAISE
591	15	0	15	// TORSO_PISTOL_DROP_RIFLE
606	14	0	15	// TORSO_RIFLE_DROP_PISTOL

620	11	0	10	// TORSO_TAUNT
631	17	0	15	// TORSO_TAUNT_PISTOL



// ---LEGS--- //

472	21	0	15	// LEGS_WALK
493	8	0	15	// LEGS_TURN

501	15	0	15	// LEGS_RUN
516	15	0	15	// LEGS_RUN_BACK
531	11	0	15	// LEGS_SLIDE

542	20	0	15	// LEGS_IDLE

593	20	0	15	// LEGS_CROUCHED_IDLE

613	10	0	10	// LEGS_SWIM
*/

#ifndef SMOKINGUNS
// animations
typedef enum {
	BOTH_DEATH1,
	BOTH_DEAD1,
	BOTH_DEATH2,
	BOTH_DEAD2,
	BOTH_DEATH3,
	BOTH_DEAD3,

	TORSO_GESTURE,

	TORSO_ATTACK,
	TORSO_ATTACK2,

	TORSO_DROP,
	TORSO_RAISE,

	TORSO_STAND,
	TORSO_STAND2,

	LEGS_WALKCR,
	LEGS_WALK,
	LEGS_RUN,
	LEGS_BACK,
	LEGS_SWIM,

	LEGS_JUMP,
	LEGS_LAND,

	LEGS_JUMPB,
	LEGS_LANDB,

	LEGS_IDLE,
	LEGS_IDLECR,

	LEGS_TURN,

	TORSO_GETFLAG,
	TORSO_GUARDBASE,
	TORSO_PATROL,
	TORSO_FOLLOWME,
	TORSO_AFFIRMATIVE,
	TORSO_NEGATIVE,
#else
// player animations
typedef enum {

	// ---BOTH--- //

	BOTH_DEATH_DEFAULT,
	BOTH_DEAD_DEFAULT,

	BOTH_DEATH_HEAD,
	BOTH_DEAD_HEAD,

	BOTH_DEATH_ARM_L,
	BOTH_DEAD_ARM_L,

	BOTH_DEATH_ARM_R,
	BOTH_DEAD_ARM_R,

	BOTH_DEATH_CHEST,
	BOTH_DEAD_CHEST,

	BOTH_DEATH_STOMACH,
	BOTH_DEAD_STOMACH,

	BOTH_DEATH_LEG_L,
	BOTH_DEAD_LEG_L,

	BOTH_DEATH_LEG_R,
	BOTH_DEAD_LEG_R,

	BOTH_FALL_BACK,
	BOTH_DEATH_LAND,
	BOTH_FALL,
	BOTH_DEAD_LAND,

	BOTH_JUMP,
	BOTH_LAND,
	BOTH_LADDER,
	BOTH_LADDER_STAND,



// ---TORSO--- //

	TORSO_HOLSTERED,
	TORSO_KNIFE_STAND,
	TORSO_KNIFE_ATTACK,
	TORSO_THROW,

	TORSO_PISTOL_STAND,
	TORSO_PISTOL_ATTACK,
	TORSO_PISTOL_RELOAD,
	TORSO_PISTOLS_STAND,
	TORSO_PISTOLS_ATTACK,

	TORSO_RIFLE_STAND,
	TORSO_RIFLE_ATTACK,
	TORSO_RIFLE_RELOAD,

	TORSO_GATLING_STAND,
	TORSO_GATLING_FIRE,
	TORSO_GATLING_RELOAD,

	TORSO_PISTOL_DROP,
	TORSO_PISTOL_RAISE,
	TORSO_RIFLE_DROP,
	TORSO_RIFLE_RAISE,

	TORSO_TAUNT,
	TORSO_TAUNT_PISTOL,

// ---LEGS--- //

	LEGS_WALK,
	LEGS_WALK_BACK,
	LEGS_TURN,

	LEGS_RUN,
	LEGS_RUN_BACK,
	LEGS_SLIDE_LEFT,
	LEGS_SLIDE_RIGHT,

	LEGS_IDLE,

	LEGS_CROUCH_WALK,
	LEGS_CROUCH_BACK,
	LEGS_CROUCHED_IDLE,

	LEGS_SWIM,

	MAX_ANIMATIONS,
#endif

#ifndef SMOKINGUNS
	LEGS_BACKCR,
	LEGS_BACKWALK,
	FLAG_RUN,
	FLAG_STAND,
	FLAG_STAND2RUN,
#else
	LEGS_RUN_SLIDE,
#endif

	MAX_TOTALANIMATIONS
} animNumber_t;

// weapon animations
#ifdef SMOKINGUNS
typedef enum {
	WP_ANIM_CHANGE,
	WP_ANIM_DROP,
	WP_ANIM_IDLE,
	WP_ANIM_FIRE,
	WP_ANIM_ALT_FIRE,
	WP_ANIM_RELOAD,
	WP_ANIM_SPECIAL,
	WP_ANIM_SPECIAL2,

	NUM_WP_ANIMATIONS
} wp_anims_t;
#endif

typedef struct animation_s {
	int		firstFrame;
	int		numFrames;
	int		loopFrames;			// 0 to numFrames
	int		frameLerp;			// msec between frames
	int		initialLerp;		// msec to get to first frame
	int		reversed;			// true if animation is reversed
	int		flipflop;			// true if animation should flipflop back to base
} animation_t;


// flip the togglebit every time an animation
// changes so a restart of the same anim can be detected
#define	ANIM_TOGGLEBIT		128


typedef enum {
	TEAM_FREE,
	TEAM_RED,
	TEAM_BLUE,
	TEAM_SPECTATOR,
#ifdef SMOKINGUNS
	TEAM_RED_SPECTATOR,
	TEAM_BLUE_SPECTATOR,
#endif

	TEAM_NUM_TEAMS
} team_t;

// Time between location updates
#define TEAM_LOCATION_UPDATE_TIME		1000

// How many players on the overlay
#define TEAM_MAXOVERLAY		32

//team task
typedef enum {
	TEAMTASK_NONE,
	TEAMTASK_OFFENSE,
	TEAMTASK_DEFENSE,
	TEAMTASK_PATROL,
	TEAMTASK_FOLLOW,
	TEAMTASK_RETRIEVE,
	TEAMTASK_ESCORT,
	TEAMTASK_CAMP
} teamtask_t;

// means of death
typedef enum {
	//means of death of the weapons have to be in order of the weaponnums, important!
	MOD_UNKNOWN,
#ifndef SMOKINGUNS
	MOD_SHOTGUN,
	MOD_GAUNTLET,
	MOD_MACHINEGUN,
	MOD_GRENADE,
	MOD_GRENADE_SPLASH,
	MOD_ROCKET,
	MOD_ROCKET_SPLASH,
	MOD_PLASMA,
	MOD_PLASMA_SPLASH,
	MOD_RAILGUN,
	MOD_LIGHTNING,
	MOD_BFG,
	MOD_BFG_SPLASH,
#else
	//melee
	MOD_KNIFE,

	//pistols
	MOD_REM58,
	MOD_SCHOFIELD,
	MOD_PEACEMAKER,

	//rifles
	MOD_WINCHESTER66,
	MOD_LIGHTNING,
	MOD_SHARPS,

	//shotguns
	MOD_REMINGTON_GAUGE,
	MOD_SAWEDOFF,
	MOD_WINCH97,

	//automatics
	MOD_GATLING,

	//explosives
	MOD_DYNAMITE,
	MOD_MOLOTOV,
#endif

	MOD_WATER,
	MOD_SLIME,
	MOD_LAVA,
	MOD_CRUSH,
	MOD_TELEFRAG,
	MOD_FALLING,
	MOD_SUICIDE,
	MOD_TARGET_LASER,
	MOD_TRIGGER_HURT,
#ifndef SMOKINGUNS
#ifdef MISSIONPACK
	MOD_NAIL,
	MOD_CHAINGUN,
	MOD_PROXIMITY_MINE,
	MOD_KAMIKAZE,
	MOD_JUICED,
#endif
	MOD_GRAPPLE
#else
	MOD_BOILER //just to check if the boilerplate was hit
#endif
} meansOfDeath_t;


//---------------------------------------------------------

// gitem_t->type
typedef enum {
	IT_BAD,
	IT_WEAPON,				// EFX: rotate + upscale + minlight
	IT_AMMO,				// EFX: rotate
	IT_ARMOR,				// EFX: rotate + minlight
	IT_HEALTH,				// EFX: static external sphere + rotating internal
	IT_POWERUP,				// instant on, timer based
							// EFX: rotate + external ring that rotates
	IT_HOLDABLE,			// single use, holdable item
							// EFX: rotate + bob
#ifndef SMOKINGUNS
	IT_PERSISTANT_POWERUP,
#endif
	IT_TEAM
} itemType_t;

#define MAX_ITEM_MODELS 4

#ifdef SMOKINGUNS
typedef struct wpinfo_s {
	animation_t	animations[NUM_WP_ANIMATIONS];
	const	float	spread;
	const	float	damage;
	const	int		range;
	const	int		addTime;
	const	int		count;
	const	int		clipAmmo;	//ammo that fits in the weapon
	const	int		clip;
	const	int		maxAmmo;	// maximum of holdable ammo
	const	char	*v_model;
	const	char	*v_barrel;
	const	char	*snd_fire;
	const	char	*snd_reload;
	const	char	*name;
	const	char	*path;
	const	int		wp_sort;
} wpinfo_t;

typedef enum {
	WPS_NONE,
	WPS_MELEE,
	WPS_PISTOL,
	WPS_GUN,
	WPS_OTHER,
	WPS_NUM_ITEMS
} wp_sort_t;

typedef enum {
	WS_NONE,
	WS_PISTOL,
	WS_RIFLE,
	WS_SHOTGUN,
	WS_MGUN,
	WS_MISC
} wp_buy_type;
#endif

typedef struct gitem_s {
	char		*classname;	// spawning name
	char		*pickup_sound;
	char		*world_model[MAX_ITEM_MODELS];

	char		*icon;
#ifdef SMOKINGUNS
	float		xyrelation;		// for the icon
#endif
	char		*pickup_name;	// for printing on pickup

	int			quantity;		// for ammo how much, or duration of powerup
	itemType_t	giType;			// IT_* flags

	int			giTag;

#ifdef SMOKINGUNS
	int			prize;
	int			weapon_sort;	// used for sorted weapons in changeweapon-menu
#endif

	char		*precaches;		// string of all models and images this item will use
	char		*sounds;		// string of all sounds this item will use
} gitem_t;

// included in both the game dll and the client
extern	gitem_t	bg_itemlist[];
#ifdef SMOKINGUNS
extern	wpinfo_t bg_weaponlist[];
#endif
extern	int		bg_numItems;

gitem_t	*BG_FindItem( const char *pickupName );
#ifdef SMOKINGUNS
gitem_t	*BG_FindItemForClassname( const char *classname );
#endif
gitem_t	*BG_FindItemForWeapon( weapon_t weapon );
#ifndef SMOKINGUNS
gitem_t	*BG_FindItemForPowerup( powerup_t pw );
gitem_t	*BG_FindItemForHoldable( holdable_t pw );
#else
gitem_t	*BG_FindItemForAmmo( weapon_t ammo ) ;
int	BG_FindPlayerWeapon( int firstweapon, int lastweapon, playerState_t	*ps);
#endif
#define	ITEM_INDEX(x) ((x)-bg_itemlist)

qboolean	BG_CanItemBeGrabbed( int gametype, const entityState_t *ent, const playerState_t *ps );


// g_dmflags->integer flags
#define	DF_NO_FALLING			8
#define DF_FIXED_FOV			16
#define	DF_NO_FOOTSTEPS			32

// content masks
#define	MASK_ALL				(-1)
#define	MASK_SOLID				(CONTENTS_SOLID)
#define	MASK_PLAYERSOLID		(CONTENTS_SOLID|CONTENTS_PLAYERCLIP|CONTENTS_BODY)
#define	MASK_DEADSOLID			(CONTENTS_SOLID|CONTENTS_PLAYERCLIP)
#define	MASK_WATER				(CONTENTS_WATER|CONTENTS_LAVA|CONTENTS_SLIME)
#define	MASK_OPAQUE				(CONTENTS_SOLID|CONTENTS_SLIME|CONTENTS_LAVA)
#ifndef SMOKINGUNS
#define	MASK_SHOT				(CONTENTS_SOLID|CONTENTS_BODY|CONTENTS_CORPSE)
#else
#define	MASK_SHOT				(CONTENTS_SOLID|CONTENTS_EXPLOSIVE|CONTENTS_BODY|CONTENTS_CORPSE)
#endif


//
// entityState_t->eType
//
typedef enum {
	ET_GENERAL,
	ET_PLAYER,
	ET_ITEM,
	ET_MISSILE,
	ET_MOVER,
	ET_BEAM,
	ET_PORTAL,
	ET_SPEAKER,
	ET_PUSH_TRIGGER,
	ET_TELEPORT_TRIGGER,
	ET_INVISIBLE,
#ifdef SMOKINGUNS
	ET_FLY,
#endif
	ET_GRAPPLE,				// grapple hooked on wall
	ET_TEAM,
#ifdef SMOKINGUNS
	// Entities list to be synchronized in code/g_svcmds.c, Svcmd_EntityList_f()
	// and cgame/cg_draw.c, CG_DrawStatusEntities()
	ET_BREAKABLE,
	ET_INTERMISSION,	// need for duel (camera move), not anymore
	ET_FLARE,
	ET_SMOKE,
	ET_TURRET, // gatling guns, etc
	ET_ESCAPE, // escape points
#endif

	ET_EVENTS				// any of the EV_* events can be added freestanding
							// by setting eType to ET_EVENTS + eventNum
							// this avoids having to set eFlags and eventNum
} entityType_t;



void	BG_EvaluateTrajectory( const trajectory_t *tr, int atTime, vec3_t result );
void	BG_EvaluateTrajectoryDelta( const trajectory_t *tr, int atTime, vec3_t result );

void	BG_AddPredictableEventToPlayerstate( int newEvent, int eventParm, playerState_t *ps );

#ifndef SMOKINGUNS
void	BG_TouchJumpPad( playerState_t *ps, entityState_t *jumppad );
#endif

void	BG_PlayerStateToEntityState( playerState_t *ps, entityState_t *s, qboolean snap );
void	BG_PlayerStateToEntityStateExtraPolate( playerState_t *ps, entityState_t *s, int time, qboolean snap );

qboolean	BG_PlayerTouchesItem( playerState_t *ps, entityState_t *item, int atTime );

#ifdef SMOKINGUNS
int BG_AnimLength( int anim, int weapon);
#endif

#define ARENAS_PER_TIER		4
#define MAX_ARENAS			1024
#define	MAX_ARENAS_TEXT		8192

#define MAX_BOTS			1024
#define MAX_BOTS_TEXT		8192


// Kamikaze

// 1st shockwave times
#define KAMI_SHOCKWAVE_STARTTIME		0
#define KAMI_SHOCKWAVEFADE_STARTTIME	1500
#define KAMI_SHOCKWAVE_ENDTIME			2000
// explosion/implosion times
#define KAMI_EXPLODE_STARTTIME			250
#define KAMI_IMPLODE_STARTTIME			2000
#define KAMI_IMPLODE_ENDTIME			2250
// 2nd shockwave times
#define KAMI_SHOCKWAVE2_STARTTIME		2000
#define KAMI_SHOCKWAVE2FADE_STARTTIME	2500
#define KAMI_SHOCKWAVE2_ENDTIME			3000
// radius of the models without scaling
#define KAMI_SHOCKWAVEMODEL_RADIUS		88
#define KAMI_BOOMSPHEREMODEL_RADIUS		72
// maximum radius of the models during the effect
#define KAMI_SHOCKWAVE_MAXRADIUS		1320
#define KAMI_BOOMSPHERE_MAXRADIUS		720
#define KAMI_SHOCKWAVE2_MAXRADIUS		704

// Relative direction strike came from
#ifdef SMOKINGUNS
#define LOCATION_LEFT		0x00000100
#define LOCATION_RIGHT		0x00000200
#define LOCATION_FRONT		0x00000400
#define LOCATION_BACK		0x00000800

#define NUM_GIBS	9

#define NUM_PREFIXINFO		12 //very important
#define	MAX_TEXINFOFILE		10000
#define	MAX_BRUSHSIDES		200

// ai nodes for the bots
#define MAX_AINODES			100
#define MAX_AINODEFILE		10000
#define AINODE_FILE_HEADER	"AI-NODES"

//prefixInfo-stats
typedef struct {
	char	*name;
	int		surfaceFlags;
	float	radius;
	float	weight;
	int		numParticles;
	float	fallingDamageFactor;
	float	thickness; // used for shoot-thru-walls code
} prefixInfo_t;

extern	prefixInfo_t	prefixInfo[NUM_PREFIXINFO];

///////////////////////////////////////
// HIT FILE STRUCTURES
///////////////////////////////////////


#define MAX_HITFILE		30000
#define	MAX_HIT_FRAMES	300
#define HIT_DEATHANIM_OFFSET 437

#define PART_NONE	0
#define	PART_HEAD	1
#define PART_UPPER	2
#define PART_LOWER	3

#define HIT_IDENT	21

// FIXME: add spherical hit-detection for head
typedef enum {
	TYPE_CYLINDER,
	TYPE_SPHERE,
	NUM_TYPES
} hit_types;

// hit locations:
typedef enum {
	HIT_HEAD,
	HIT_NECK,
	HIT_SHOULDER_R,
	HIT_UPPER_ARM_R,
	HIT_LOWER_ARM_R,
	HIT_HAND_R,
	HIT_SHOULDER_L,
	HIT_UPPER_ARM_L,
	HIT_LOWER_ARM_L,
	HIT_HAND_L,
	HIT_CHEST,
	HIT_STOMACH,
	HIT_UPPER_LEG_R,
	HIT_LOWER_LEG_R,
	HIT_FOOT_R,
	HIT_UPPER_LEG_L,
	HIT_LOWER_LEG_L,
	HIT_FOOT_L,
	HIT_PELVIS,

	NUM_HIT_LOCATIONS
} hit_locations;

#define NUM_HIT_HEAD (HIT_NECK - HIT_HEAD)
#define NUM_HIT_UPPER (HIT_UPPER_LEG_R - HIT_NECK)
#define NUM_HIT_LOWER (NUM_HIT_LOCATIONS - HIT_UPPER_LEG_R)

// create some info to make parsing the hitfiles a little easier
typedef struct hit_info_s {
	char	*meshname;
	char	*forename;
	char	*backname;
	int		hit_location;
	int		hit_part;
} hit_info_t;

extern	hit_info_t	hit_info[NUM_HIT_LOCATIONS];

// Hit data
typedef struct hit_header {
	short	ident;
	short	numFrames;
	short	numMeshes;
} hit_header_t;

typedef struct hit_tag_s {
	vec3_t		angles;
	vec3_t		origin;
} hit_tag_t;

typedef struct hit_mesh_s {
	short		normal[3];
	short		origin[3];
} hit_mesh_t;

typedef struct mesh_header_s {
	char		name[20];
	short		a1;
	short		a2;
	short		dir1[3];
	short		dir2[3];
	short		length;
} mesh_header_t;

typedef struct hit_part_s {
	mesh_header_t	header;
	hit_mesh_t		pos[MAX_HIT_FRAMES];
} hit_part_t;


typedef struct hit_data_s {
	hit_tag_t		tag_head[MAX_HIT_FRAMES];
	hit_tag_t		tag_torso[MAX_HIT_FRAMES];

	hit_part_t		meshes[NUM_HIT_LOCATIONS];

	animation_t		animations[MAX_TOTALANIMATIONS];
} hit_data_t;

////////////////////////////////////////////
// HIT FILE STRUCTURES END
////////////////////////////////////////////


void BG_ModifyEyeAngles( vec3_t origin, vec3_t viewangles,
						void (*trace)( trace_t *results, const vec3_t start, const vec3_t mins, const vec3_t maxs, const vec3_t end, int passEntityNum, int contentMask ),
						vec3_t legOffset, qboolean print);
qboolean BG_ShootThruWall( float *damage, vec3_t start, vec3_t muzzle, int surfaceFlags, vec3_t end,
						void (*trace)( trace_t *results, const vec3_t start, const vec3_t mins, const vec3_t maxs, const vec3_t end, int passEntityNum, int contentMask ));
int BG_CountTypeWeapons(int type, int weapons);
int BG_SearchTypeWeapon(int type, int weapons, int wp_ignore);

void BG_SetWhiskeyDrop(trajectory_t *tr, vec3_t org, vec3_t normal, vec3_t dir);
void BG_DirsToEntityState(entityState_t *es, vec3_t bottledirs[ALC_COUNT]);
void BG_EntityStateToDirs(entityState_t *es, vec3_t bottledirs[ALC_COUNT]);

qboolean CheckPistols(playerState_t *ps, int *weapon);
int BG_MapPrefix(char *map, int gametype);

extern vec3_t	playerMins;
extern vec3_t	playerMaxs;

extern vec3_t gatling_mins;
extern vec3_t gatling_maxs;
extern vec3_t gatling_mins2;
extern vec3_t gatling_maxs2;

// Shared by game & cgame for surfaceFlags info decompression
typedef struct shaderInfo_s {
	int			surfaceFlags;
#ifdef CGAME
	vec4_t		color; // Only used on client side
#endif
} shaderInfo_t;

extern	shaderInfo_t shaderInfo[MAX_BRUSHSIDES];
extern	int	shaderInfoNum;

#ifdef QAGAME
extern	vmCvar_t	g_maxMoney;
#define BG_MAX_MONEY() (g_maxMoney.integer)
#else
#ifdef CGAME
extern	vmCvar_t	cg_maxMoney;
#define BG_MAX_MONEY() (cg_maxMoney.integer)
#else
// Because bg_* are also shared with UI VM... but we don't care about it, let's give it the default maxMoney value
#define BG_MAX_MONEY() (200)
#endif
#endif


#endif
