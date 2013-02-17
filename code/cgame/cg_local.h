/*
===========================================================================
Copyright (C) 1999-2005 Id Software, Inc.
Copyright (C) 2000-2003 Iron Claw Interactive
Copyright (C) 2005-2013 Smokin' Guns

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
#include "../qcommon/q_shared.h"
#include "../renderercommon/tr_types.h"
#include "../game/bg_public.h"
#include "cg_public.h"


// The entire cgame module is unloaded and reloaded on each level change,
// so there is NO persistant data between levels on the client side.
// If you absolutely need something stored, it can either be kept
// by the server in the server stored userinfos, or stashed in a cvar.

#ifdef MISSIONPACK
#define CG_FONT_THRESHOLD 0.1
#endif

#define	POWERUP_BLINKS		5

#define	POWERUP_BLINK_TIME	1000
#define	FADE_TIME			200
#define	PULSE_TIME			200
#define	DAMAGE_DEFLECT_TIME	100
#define	DAMAGE_RETURN_TIME	400
#define DAMAGE_TIME			500
#define	LAND_DEFLECT_TIME	150
#define	LAND_RETURN_TIME	300
#define	STEP_TIME			200
#define	DUCK_TIME			100
#define	PAIN_TWITCH_TIME	200
#define	WEAPON_SELECT_TIME	1400
#define	ITEM_SCALEUP_TIME	1000
#define	ZOOM_TIME			150
#define	ITEM_BLOB_TIME		200
#define	MUZZLE_FLASH_TIME	20
#define	SINK_TIME			1000		// time for fragments to sink into ground before going away
#define	ATTACKER_HEAD_TIME	10000
#define	REWARD_TIME			3000

#define	PULSE_SCALE			1.5			// amount to scale up the icons when activating

#define	MAX_STEP_CHANGE		32

#define	MAX_VERTS_ON_POLY	10
#define	MAX_MARK_POLYS		256

#define STAT_MINUS			10	// num frame for '-' stats digit

#ifndef SMOKINGUNS
#define	ICON_SIZE			48
#define	CHAR_WIDTH			32
#define	CHAR_HEIGHT			48
#else
#define	ICON_SIZE			32
#define	CHAR_WIDTH			20
#define	CHAR_HEIGHT			30
#endif
#define	TEXT_ICON_SPACE		4

#ifndef SMOKINGUNS
#define	TEAMCHAT_WIDTH		80
#define	TEAMCHAT_HEIGHT		8
#else
#define	CHAT_WIDTH			79 // As chat messages are still shift by one position
#define	CHAT_HEIGHT			8

#define CHATCHAR_WIDTH		4
#define CHATCHAT_HEIGHT		8
#endif

// very large characters
#define	GIANT_WIDTH			32
#define	GIANT_HEIGHT		48

#ifndef SMOKINGUNS
#define	NUM_CROSSHAIRS		10
#else
#define	NUM_CROSSHAIRS		4
#endif

#define TEAM_OVERLAY_MAXNAME_WIDTH	12
#define TEAM_OVERLAY_MAXLOCATION_WIDTH	16

#ifndef SMOKINGUNS
#define	DEFAULT_MODEL			"sarge"
#ifdef MISSIONPACK
#define	DEFAULT_TEAM_MODEL		"james"
#define	DEFAULT_TEAM_HEAD		"*james"
#else
#define	DEFAULT_TEAM_MODEL		"sarge"
#define	DEFAULT_TEAM_HEAD		"sarge"
#endif

#define DEFAULT_REDTEAM_NAME		"Stroggs"
#define DEFAULT_BLUETEAM_NAME		"Pagans"
#else

// Added by hika
// For frustum culling, used in CG_CullBoundingBox()
#define	CULL_IN				0	// completely unclipped
#define	CULL_CLIP			1	// clipped by one or more planes
#define	CULL_OUT			2	// completely outside the clipping planes

// Range under which we guarantee entities will be in the visible entity list
// when cg_boostfps is set
#define FORCE_VISIBILITY_RANGE 150
#endif

typedef enum {
	FOOTSTEP_NORMAL,
	FOOTSTEP_BOOT,
	FOOTSTEP_FLESH,
	FOOTSTEP_MECH,
	FOOTSTEP_ENERGY,
	FOOTSTEP_METAL,
#ifdef SMOKINGUNS
	FOOTSTEP_CLOTH,
	FOOTSTEP_GRASS,
#endif
	FOOTSTEP_SPLASH,
#ifdef SMOKINGUNS
	FOOTSTEP_SNOW,
	FOOTSTEP_SAND,
#endif

	FOOTSTEP_TOTAL
} footstep_t;

#ifdef SMOKINGUNS
typedef enum {
	IMPACT_WOOD,
	IMPACT_WATER,
	IMPACT_METAL,
	IMPACT_GLASS,
	IMPACT_STONE,
	IMPACT_SAND,
	IMPACT_GRASS,
	IMPACT_DEFAULT,

	IMPACT_TOTAL
} impact_t;

typedef enum {
	MENU_NONE,
	MENU_BUY,
	MENU_DUEL_FINISHED
} menutype_t;
#endif

typedef enum {
	IMPACTSOUND_DEFAULT,
	IMPACTSOUND_METAL,
	IMPACTSOUND_FLESH
} impactSound_t;



//=================================================

// player entities need to track more information
// than any other type of entity.

// note that not every player entity is a client entity,
// because corpses after respawn are outside the normal
// client numbering range

// when changing animation, set animationTime to frameTime + lerping time
// The current lerp will finish out, then it will lerp to the new animation
typedef struct {
	int			oldFrame;
	int			oldFrameTime;		// time when ->oldFrame was exactly on

	int			frame;
	int			frameTime;			// time when ->frame will be exactly on

	float		backlerp;

	float		yawAngle;
	qboolean	yawing;
	float		pitchAngle;
	qboolean	pitching;

	int			animationNumber;	// may include ANIM_TOGGLEBIT
	animation_t	*animation;
	int			animationTime;		// time when the first frame of the animation will be exact

#ifdef SMOKINGUNS
	int	weaponAnim;
#endif
} lerpFrame_t;


typedef struct {
#ifndef SMOKINGUNS
	lerpFrame_t		legs, torso, flag;
#else
	lerpFrame_t		legs, torso, flag, weapon, weapon2;
#endif
	int				painTime;
	int				painDirection;	// flip from 0 to 1
	int				lightningFiring;

	int				railFireTime;

	// machinegun spinning
	float			barrelAngle;
	int				barrelTime;
	qboolean		barrelSpinning;

	//gatling
#ifdef SMOKINGUNS
	float			middleAngle;
	float			gatlingAngle;

	int				eFlags;
#endif
} playerEntity_t;

//=================================================



// centity_t have a direct corespondence with gentity_t in the game, but
// only the entityState_t is directly communicated to the cgame
typedef struct centity_s {
	entityState_t	currentState;	// from cg.frame
	entityState_t	nextState;		// from cg.nextFrame, if available
	qboolean		interpolate;	// true if next is valid to interpolate to
	qboolean		currentValid;	// true if cg.frame holds this entity

	int				muzzleFlashTime;	// move to playerEntity?
#ifdef SMOKINGUNS
	int				muzzleFlashTime2;	// 2nd weapon
#endif
	int				previousEvent;
	int				teleportFlag;

	int				trailTime;		// so missile trails can handle dropped initial packets
	int				dustTrailTime;
	int				miscTime;

	int				snapShotTime;	// last time this entity was found in a snapshot

	playerEntity_t	pe;

	int				errorTime;		// decay the error from this time
	vec3_t			errorOrigin;
	vec3_t			errorAngles;

	qboolean		extrapolated;	// false if origin / angles is an interpolation
	vec3_t			rawOrigin;
	vec3_t			rawAngles;

	vec3_t			beamEnd;

	// exact interpolated position of entity on this frame
	vec3_t			lerpOrigin;
	vec3_t			lerpAngles;

#ifdef SMOKINGUNS
	// additional smoke effects for pistols
	int				smokeTime;
	int				lastSmokeTime;
	int				smokeTime2;
	int				lastSmokeTime2;

	int				fallTime, fallTimeReal;
	vec3_t			fallLastPos;
	qboolean		landing;
	int				landtime;
	qboolean		fixed;

	vec3_t oldorigin;

	// Added by hika
	qboolean		visible;	// State in the last CG_IsEntityVisible() call
	int				visibleTime;// cg.time value, when CG_IsEntityVisible() was called
	vec_t			distFromCamera;	// Distance from the camera
					                // ** not an accurate value, but OK for us :D **
	vec3_t			centerOrigin; // May be different from other "origin" properties
					              // This is computed from the bounding box entity.
	vec_t			radius; // its bounding sphere radius

	// Comment by hika:
	// About the above currentState field (and nextState ?),
	// entityState_t->time contains playerState_t->powerups[PW_BURNBIT]
	// entityState_t->time2 contains playerState_t->stats[STAT_WEAPONS]
	// entityState_t->frame contains playerState_t->weapon2
	//
	// which are good things to know :D
	// see ../game/bg_misc.c in BG_PlayerStateToEntityStateExtraPolate()
	// and BG_PlayerStateToEntityState()
#endif
} centity_t;


//======================================================================

// local entities are created as a result of events or predicted actions,
// and live independantly from all server transmitted entities

typedef struct markPoly_s {
	struct markPoly_s	*prevMark, *nextMark;
	int			time;
#ifdef SMOKINGUNS
	int			duration;
#endif
	qhandle_t	markShader;
	qboolean	alphaFade;		// fade alpha instead of rgb
	float		color[4];
	poly_t		poly;
	polyVert_t	verts[MAX_VERTS_ON_POLY];
} markPoly_t;


typedef enum {
	LE_MARK,
	LE_EXPLOSION,
	LE_SPRITE_EXPLOSION,
	LE_FRAGMENT,
	LE_MOVE_SCALE_FADE,
	LE_FALL_SCALE_FADE,
	LE_FADE_RGB,
	LE_SCALE_FADE,
#ifndef SMOKINGUNS
	LE_SCOREPLUM,
	LE_KAMIKAZE,
	LE_INVULIMPACT,
	LE_INVULJUICED,
#else
	LE_SMOKE,
#endif
	LE_SHOWREFENTITY
} leType_t;

typedef enum {
	LEF_PUFF_DONT_SCALE  = 0x0001,			// do not scale size over time
	LEF_TUMBLE			 = 0x0002,			// tumble over time, used for ejecting shells
	LEF_SOUND1			 = 0x0004,			// sound 1 for kamikaze
#ifndef SMOKINGUNS
	LEF_SOUND2			 = 0x0008			// sound 2 for kamikaze
#else
	LEF_SOUND2			 = 0x0008,			// sound 2 for kamikaze
	LEF_BREAKS			 = 0x0010,
	LEF_PARTICLE		 = 0x0020,
	LEF_SPARKS			 = 0x0040,
	LEF_COLOR			 = 0x0080,			// color is changeable
	LEF_BLOOD			 = 0x0100,
	LEF_TRAIL			 = 0x0200,
	LEF_WHISKEY			 = 0x0400,
	LEF_FIRE			 = 0x0800,
	LEF_BREAKS_DEF		 = 0x1000,
	LEF_MARK			 = 0x2000, // manually created impact mark (on breakables)
	LEF_REMOVE			 = 0x4000	// remove after round-restart
#endif
} leFlag_t;

typedef enum {
	LEMT_NONE,
	LEMT_BURN,
	LEMT_BLOOD
} leMarkType_t;			// fragment local entities can leave marks on walls

typedef enum {
	LEBS_NONE,
#ifdef SMOKINGUNS
	LEBS_GLASS,
	LEBS_METAL,
	LEBS_WOOD,
	LEBS_DEFAULT,
#endif
	LEBS_BLOOD,
	LEBS_BRASS
} leBounceSoundType_t;	// fragment local entities can make sounds on impacts

typedef struct localEntity_s {
	struct localEntity_s	*prev, *next;
	leType_t		leType;
	int				leFlags;

	int				startTime;
	int				endTime;
	int				fadeInTime;

	float			lifeRate;			// 1.0 / (endTime - startTime)

	trajectory_t	pos;
	trajectory_t	angles;

	float			bounceFactor;		// 0.0 = no bounce, 1.0 = perfect

	float			color[4];

	float			radius;

	float			light;
	vec3_t			lightColor;

	leMarkType_t		leMarkType;		// mark to leave on fragment impact
	leBounceSoundType_t	leBounceSoundType;

#ifdef SMOKINGUNS
	vec3_t			vector;
	int				boost, wind;
#endif

	refEntity_t		refEntity;

	// Added by hika
#ifdef SMOKINGUNS
	qboolean		visible;	// State in the last CG_IsLocalEntityVisible() call
	int				visibleTime;// cg.time value, when CG_IsLocalEntityVisible() was called
	vec_t			distFromCamera;	// Distance from the camera
					                // ** not an accurate value, but OK for us :D **
#endif
} localEntity_t;

//======================================================================


typedef struct {
	int				client;
	int				score;
	int				ping;
	int				time;
	int				scoreFlags;
	int				powerUps;
	int				accuracy;
	int				impressiveCount;
	int				excellentCount;
	int				guantletCount;
	int				defendCount;
	int				assistCount;
	int				captures;
	qboolean		perfect;
	int				team;
#ifdef SMOKINGUNS
	qboolean		realspec;
	qboolean		won;
#endif
} score_t;

// each client has an associated clientInfo_t
// that contains media references necessary to present the
// client model and other color coded effects
// this is regenerated each time a client's configstring changes,
// usually as a result of a userinfo (name, model, etc) change
#define	MAX_CUSTOM_SOUNDS	32

typedef struct {
	qboolean		infoValid;

	char			name[MAX_QPATH];
	team_t			team;

	int				botSkill;		// 0 = not bot, 1-5 = bot

#ifndef SMOKINGUNS
	vec3_t			color1;
	vec3_t			color2;
	
	byte c1RGBA[4];
	byte c2RGBA[4];
#endif

	int				score;			// updated by score servercmds
	int				location;		// location index for team mode
	int				health;			// you only get this info about your teammates
	int				armor;
	int				curWeapon;

	int				handicap;
	int				wins, losses;	// in tourney mode

	int				teamTask;		// task in teamplay (offence/defence)
	qboolean		teamLeader;		// true when this is a team leader

	int				powerups;		// so can display quad/flag status

	int				medkitUsageTime;
	int				invulnerabilityStartTime;
	int				invulnerabilityStopTime;

	int				breathPuffTime;

	// when clientinfo is changed, the loading of models/skins/sounds
	// can be deferred until you are dead, to prevent hitches in
	// gameplay
	char			modelName[MAX_QPATH];
	char			skinName[MAX_QPATH];
	char			headModelName[MAX_QPATH];
	char			headSkinName[MAX_QPATH];
	char			redTeam[MAX_TEAMNAME];
	char			blueTeam[MAX_TEAMNAME];
	qboolean		deferred;

	qboolean		newAnims;		// true if using the new mission pack animations
#ifndef SMOKINGUNS
	qboolean		fixedlegs;		// true if legs yaw is always the same as torso yaw
	qboolean		fixedtorso;		// true if torso never changes yaw
#endif

	vec3_t			headOffset;		// move head in icon views
	footstep_t		footsteps;
	gender_t		gender;			// from model

	qhandle_t		legsModel;
	qhandle_t		legsSkin;

	qhandle_t		torsoModel;
	qhandle_t		torsoSkin;

	qhandle_t		headModel;
	qhandle_t		headSkin;

#ifdef SMOKINGUNS
	qhandle_t		holsterSkin;

	//damage skins
	qhandle_t		legsSkin_damage_1, legsSkin_damage_2;
	qhandle_t		torsoSkin_damage_1, torsoSkin_damage_2;
	qhandle_t		headSkin_damage_1, headSkin_damage_2;
#endif

	qhandle_t		modelIcon;

	animation_t		animations[MAX_TOTALANIMATIONS];

	sfxHandle_t		sounds[MAX_CUSTOM_SOUNDS];
} clientInfo_t;


// each WP_* weapon enum has an associated weaponInfo_t
// that contains media references necessary to present the
// weapon and its effects
typedef struct weaponInfo_s {
	qboolean		registered;
	gitem_t			*item;

	qhandle_t		handsModel;			// the hands don't actually draw, they just position the weapon
#ifdef SMOKINGUNS
	qhandle_t		l_weaponModel;		// "real" hands-model, first person view, left handed
	qhandle_t		r_weaponModel;		// right handed
#endif
	qhandle_t		weaponModel;
	qhandle_t		barrelModel;
	qhandle_t		flashModel;

	vec3_t			weaponMidpoint;		// so it will rotate centered instead of by tag

	float			flashDlight;
	vec3_t			flashDlightColor;
	sfxHandle_t		flashSound[4];		// fast firing weapons randomly choose
#ifdef SMOKINGUNS
	sfxHandle_t		reloadSound;
#endif

	qhandle_t		weaponIcon;
	qhandle_t		ammoIcon;

	qhandle_t		ammoModel;

	qhandle_t		missileModel;
	sfxHandle_t		missileSound;
	void			(*missileTrailFunc)( centity_t *, const struct weaponInfo_s *wi );
	float			missileDlight;
	vec3_t			missileDlightColor;
	int				missileRenderfx;

	void			(*ejectBrassFunc)( centity_t * );

	float			trailRadius;
	float			wiTrailTime;

#ifndef SMOKINGUNS
	sfxHandle_t		readySound;
	sfxHandle_t		firingSound;
#endif
} weaponInfo_t;


// each IT_* item has an associated itemInfo_t
// that constains media references necessary to present the
// item and its effects
typedef struct {
	qboolean		registered;
	qhandle_t		models[MAX_ITEM_MODELS];
	qhandle_t		icon;
} itemInfo_t;


typedef struct {
	int				itemNum;
} powerupInfo_t;


#define MAX_SKULLTRAIL		10

typedef struct {
	vec3_t positions[MAX_SKULLTRAIL];
	int numpositions;
} skulltrail_t;


#define MAX_REWARDSTACK		10
#define MAX_SOUNDBUFFER		20

//======================================================================

// all cg.stepTime, cg.duckTime, cg.landTime, etc are set to cg.time when the action
// occurs, and they will have visible effects for #define STEP_TIME or whatever msec after

#define MAX_PREDICTED_EVENTS	16

#ifdef SMOKINGUNS
//unlagged - optimized prediction
#define NUM_SAVED_STATES (CMD_BACKUP + 2)
//unlagged - optimized prediction
#endif

typedef struct {
	int			clientFrame;		// incremented each frame

	int			clientNum;

	qboolean	demoPlayback;
	qboolean	levelShot;			// taking a level menu screenshot
	int			deferredPlayerLoading;
	qboolean	loading;			// don't defer players at initial startup
	qboolean	intermissionStarted;	// don't play voice rewards, because game will end shortly
#ifdef SMOKINGUNS
	qboolean	readytostart;
#endif

	// there are only one or two snapshot_t that are relevent at a time
	int			latestSnapshotNum;	// the number of snapshots the client system has received
	int			latestSnapshotTime;	// the time from latestSnapshotNum, so we don't need to read the snapshot yet

	snapshot_t	*snap;				// cg.snap->serverTime <= cg.time
	snapshot_t	*nextSnap;			// cg.nextSnap->serverTime > cg.time, or NULL
	snapshot_t	activeSnapshots[2];

	float		frameInterpolation;	// (float)( cg.time - cg.frame->serverTime ) / (cg.nextFrame->serverTime - cg.frame->serverTime)

	qboolean	thisFrameTeleport;
	qboolean	nextFrameTeleport;

	int			frametime;		// cg.time - cg.oldTime

	int			time;			// this is the time value that the client
								// is rendering at.
	int			oldTime;		// time at last frame, used for missile trails and prediction checking

	int			physicsTime;	// either cg.snap->time or cg.nextSnap->time

	int			timelimitWarnings;	// 5 min, 1 min, overtime
	int			fraglimitWarnings;

	qboolean	mapRestart;			// set on a map restart to set back the weapon

	qboolean	renderingThirdPerson;		// during deaths, chasecams, etc

	// prediction state
	qboolean	hyperspace;				// true if prediction has hit a trigger_teleport
	playerState_t	predictedPlayerState;
	centity_t		predictedPlayerEntity;
	qboolean	validPPS;				// clear until the first call to CG_PredictPlayerState
	int			predictedErrorTime;
	vec3_t		predictedError;

	int			eventSequence;
	int			predictableEvents[MAX_PREDICTED_EVENTS];

	float		stepChange;				// for stair up smoothing
	int			stepTime;

	float		duckChange;				// for duck viewheight smoothing
	int			duckTime;

	float		landChange;				// for landing hard
#ifdef SMOKINGUNS
	float		landAngleChange;
#endif
	int			landTime;

#ifdef SMOKINGUNS
	float		shootChange;
	float		shootAngleChange;
	int			shootTime;
#endif

	// input state sent to server
	int			weaponSelect;

	// auto rotating items
	vec3_t		autoAngles;
	vec3_t		autoAxis[3];
	vec3_t		autoAnglesFast;
	vec3_t		autoAxisFast[3];

	// view rendering
	refdef_t	refdef;
	vec3_t		refdefViewAngles;		// will be converted to refdef.viewaxis

	// zoom key
	qboolean	zoomed;
	int			zoomTime;
	float		zoomSensitivity;

	// information screen text during loading
	char		infoScreenText[MAX_STRING_CHARS];

	// scoreboard
	int			scoresRequestTime;
	int			numScores;
	int			selectedScore;
	int			teamScores[2];
	score_t		scores[MAX_CLIENTS];
	qboolean	showScores;
	qboolean	scoreBoardShowing;
	int			scoreFadeTime;
	char		killerName[MAX_NAME_LENGTH];

	// Spectating field in scoreboard
	char			spectatorList[MAX_STRING_CHARS];		// list of names
	int				spectatorLen;												// length of list
	float			spectatorWidth;											// width in device units
	int				spectatorTime;											// next time to offset
	int				spectatorPaintX;										// current paint x
	int				spectatorPaintX2;										// current paint x
	int				spectatorOffset;										// current offset from start
	int				spectatorPaintLen; 									// current offset from start
#ifdef SMOKINGUNS
	vec4_t		spectatorCurrentColor;									// current color from start
#else
	// skull trails
	skulltrail_t	skulltrails[MAX_CLIENTS];
#endif

	// centerprinting
	int			centerPrintTime;
	int			centerPrintCharWidth;
	int			centerPrintY;
	char		centerPrint[1024];
	int			centerPrintLines;

	// low ammo warning state
	int			lowAmmoWarning;		// 1 = low, 2 = empty

	// crosshair client ID
	int			crosshairClientNum;
	int			crosshairClientTime;

	// powerup active flashing
	int			powerupActive;
	int			powerupTime;

	// attacking player
	int			attackerTime;
	int			voiceTime;

	// reward medals
	int			rewardStack;
	int			rewardTime;
	int			rewardCount[MAX_REWARDSTACK];
	qhandle_t	rewardShader[MAX_REWARDSTACK];
	qhandle_t	rewardSound[MAX_REWARDSTACK];

	// sound buffer mainly for announcer sounds
	int			soundBufferIn;
	int			soundBufferOut;
	int			soundTime;
	qhandle_t	soundBuffer[MAX_SOUNDBUFFER];

#ifndef SMOKINGUNS
	// for voice chat buffer
	int			voiceChatTime;
	int			voiceChatBufferIn;
	int			voiceChatBufferOut;
#endif

	// warmup countdown
	int			warmup;
	int			warmupCount;

	//==========================

#ifdef SMOKINGUNS
	int			itemPickupQuant;
#endif
	int			itemPickup;
	int			itemPickupTime;
	int			itemPickupBlendTime;	// the pulse around the crosshair is timed seperately

	int			weaponSelectTime;
	int			weaponAnimation;
	int			weaponAnimationTime;

	// blend blobs
	float		damageTime;
	float		damageX, damageY, damageValue;

	// status bar head
	float		headYaw;
	float		headEndPitch;
	float		headEndYaw;
	int			headEndTime;
	float		headStartPitch;
	float		headStartYaw;
	int			headStartTime;

	// view movement
	float		v_dmg_time;
	float		v_dmg_pitch;
	float		v_dmg_roll;

	// temp working variables for player view
	float		bobfracsin;
	int			bobcycle;
	float		xyspeed;
	int     nextOrbitTime;

	//qboolean cameraMode;		// if rendering from a loaded camera


	// development tool
	refEntity_t		testModelEntity;
	char			testModelName[MAX_QPATH];
	qboolean		testGun;

#ifdef SMOKINGUNS
//unlagged - optimized prediction
	int			lastPredictedCommand;
	int			lastServerTime;
	playerState_t savedPmoveStates[NUM_SAVED_STATES];
	int			stateHead, stateTail;
//unlagged - optimized prediction

	//Spoon: deathmessages
	char attacker[MAX_DEATH_MESSAGES][32];
	char target[MAX_DEATH_MESSAGES][32];
	int messagetime[MAX_DEATH_MESSAGES];
	int mod[MAX_DEATH_MESSAGES];

	//marked item
	int	marked_item_index;
	vec3_t	marked_item_mins;
	vec3_t	marked_item_maxs;
	int		marked_item_time;

	//menu screens for cgame for better information(money, player-class, team)
	int		menu;
	int		menustat;
	int		menuitem;
	qboolean	menumove;

	//int button;
	qboolean oldbutton;

	//weaponselect additions
	int		markedweapon;
	int		lastusedweapon;
	int		mousesensitivity;
	int		duelstartsound;

	// true when the top pistol in the weapons selection menu is marked.
	// Only used when the player has two pistols of the same kind.
	//
	// The first will be the one in the left holster
	//    (with WP_AKIMBO ammo type)
	// The second will be the one in the right holster
	//    (with WP_REM58, WP_SCHOFIELD or WP_PEACEMAKER ammo type)
	qboolean	markedfirstpistol;

	// A copy of weaponSelect in case weaponSelect == WP_SEC_PISTOL,
	// -> see ./cgame/cg_draw.c
	int		_weaponSelect;

	int		roundendtime;
	int		roundstarttime;
	int		introstart;
	int		introend;

	//music values
	qboolean musicfile;
	qboolean custommusic;
	qboolean custommusic_started;

	int		playmusic_endtime;
	int		playmusic_starttime;
	int		oldTrackNum;

	// new view vectors (controlling view for dying scenes)
	vec3_t	anim_viewangles;
	vec3_t	anim_vieworigin;

	//legOffset of 1st person view
	vec3_t legOffset;

	// if weapon is reloading
	qboolean r_weapon;

	// for better animation system through network
	int		firingtime, firingtime2;

	// intermission info for duel
	vec3_t	intermission_origin;
	vec3_t	intermission_angles;
	int currentmappart;

	// variables to control the medal assignment to the current player
	int lastmedals;		// number of medals he owns
	int medaltime;	// time when last medal has been given

	int	round, oldround;

	// predicton of weapon animations
	int weaponAnim, weapon2Anim, animtime, weapon, weapon2;
	int	weaponAnimOld , weapon2AnimOld, weaponold, weapon2old;
	qboolean	gatlingmode, gatlingmodeOld;

	int scopetime;
	int	scopedeltatime;
	int oldscopestat;

	vec3_t deathpos[MAX_CLIENTS];
	int	fallTimes[MAX_CLIENTS];
	int	realFallTimes[MAX_CLIENTS];
	qboolean landings[MAX_CLIENTS];
	int	landTimes[MAX_CLIENTS];
	qboolean landTypes[MAX_CLIENTS];

	vec3_t escapePoint;
#endif
} cg_t;


// all of the model, shader, and sound references that are
// loaded at gamestate time are stored in cgMedia_t
// Other media that can be tied to clients, weapons, or items are
// stored in the clientInfo_t, itemInfo_t, weaponInfo_t, and powerupInfo_t
typedef struct {
#ifdef SMOKINGUNS
	// Spoon icons for ui
	qhandle_t	dead;
	qhandle_t	won[5];

	//hud
	qhandle_t	hud_ammo_bullet;
	qhandle_t	hud_ammo_sharps;
	qhandle_t	hud_ammo_cart;
	qhandle_t	hud_ammo_shell;

	qhandle_t	hud_button;
	qhandle_t	hud_button_selected;

	qhandle_t	hud_wp_box;
	qhandle_t	hud_wp_top;
	qhandle_t	hud_wp_box_quad;

	//menu
	qhandle_t	menu_top;
	qhandle_t	menu_tex;
	qhandle_t	menu_end;
	qhandle_t	menu_arrow;

	//wqfx
	qhandle_t	wqfx_sparks;
	qhandle_t	wqfx_matchfire;
	qhandle_t	wqfx_smokeexp;

	//Scope
	qhandle_t	scopeShader;
	qhandle_t	scopecrossShader;

	//additional models

	//gibs
	qhandle_t	glassgib[NUM_GIBS];
	qhandle_t	clothgib[5];
	qhandle_t	woodgib[3];
	qhandle_t	dirtgib[3];
	qhandle_t	stonegibshader, metalgibshader;
	// molotov gibs
	qhandle_t	molotovgib[4];

	//particles
	qhandle_t	particles[NUM_PREFIXINFO][3];
	qhandle_t	whiskey_drops[6];
	qhandle_t	whiskey_pools[3];
	qhandle_t	blood_particles[6];
	qhandle_t	blood_marks[3];
	qhandle_t	flare;
	qhandle_t	marks[NUM_PREFIXINFO][2];
	qhandle_t	breakmarks[NUM_PREFIXINFO];
//	qhandle_t	sparktrail;
	qhandle_t	wqfx_fire;
	qhandle_t	wqfx_firedrop;
	sfxHandle_t	firesound;

	//new player model addition
	qhandle_t	holster_r;
	qhandle_t	holster_l;

	// special hold-models for missile weapons
	qhandle_t	e_knife;
	qhandle_t	e_dynamite;
	qhandle_t	e_moneybag;

	// scope
	qhandle_t	model_scope;

	// smoke effects
	qhandle_t	pistol_smoke;
	qhandle_t	ent_smoke;

	// additional weapon icons
	qhandle_t	sharps_scope;
	qhandle_t	scope_sharps;

	// Spoon End
#endif

	qhandle_t	charsetShader;
	qhandle_t	charsetProp;
#ifndef SMOKINGUNS
// Tequila comment: Removed that as it is not used, charset not existing
	qhandle_t	charsetPropGlow;
#endif
	qhandle_t	charsetPropB;
	qhandle_t	whiteShader;

#ifndef SMOKINGUNS
	qhandle_t	redCubeModel;
	qhandle_t	blueCubeModel;
	qhandle_t	redCubeIcon;
	qhandle_t	blueCubeIcon;
#endif
	qhandle_t	redFlagModel;
	qhandle_t	blueFlagModel;
	qhandle_t	neutralFlagModel;
	qhandle_t	redFlagShader[3];
	qhandle_t	blueFlagShader[3];
	qhandle_t	flagShader[4];

	qhandle_t	flagPoleModel;
	qhandle_t	flagFlapModel;

	qhandle_t	redFlagFlapSkin;
	qhandle_t	blueFlagFlapSkin;
	qhandle_t	neutralFlagFlapSkin;

	qhandle_t	redFlagBaseModel;
	qhandle_t	blueFlagBaseModel;
	qhandle_t	neutralFlagBaseModel;

#ifndef SMOKINGUNS
	qhandle_t	overloadBaseModel;
	qhandle_t	overloadTargetModel;
	qhandle_t	overloadLightsModel;
	qhandle_t	overloadEnergyModel;

	qhandle_t	harvesterModel;
	qhandle_t	harvesterRedSkin;
	qhandle_t	harvesterBlueSkin;
	qhandle_t	harvesterNeutralModel;
#endif

	qhandle_t	armorModel;
	qhandle_t	armorIcon;

	qhandle_t	teamStatusBar;

	qhandle_t	deferShader;

	// gib explosions
	qhandle_t	gibAbdomen;
	qhandle_t	gibArm;
	qhandle_t	gibChest;
	qhandle_t	gibFist;
	qhandle_t	gibFoot;
	qhandle_t	gibForearm;
	qhandle_t	gibIntestine;
	qhandle_t	gibLeg;
	qhandle_t	gibSkull;
	qhandle_t	gibBrain;

	qhandle_t	smoke2;

	qhandle_t	machinegunBrassModel;
	qhandle_t	shotgunBrassModel;

	qhandle_t	railRingsShader;
	qhandle_t	railCoreShader;

	qhandle_t	lightningShader;

	qhandle_t	friendShader;

	qhandle_t	balloonShader;
#ifdef SMOKINGUNS
	qhandle_t	balloonBuyShader;
#endif
	qhandle_t	connectionShader;

	qhandle_t	selectShader;
	qhandle_t	viewBloodShader;
	qhandle_t	tracerShader;
	qhandle_t	crosshairShader[NUM_CROSSHAIRS];
#ifdef SMOKINGUNS
	qhandle_t	crosshairFriendShader;
#endif

	qhandle_t	lagometerShader;
	qhandle_t	backTileShader;
#ifndef SMOKINGUNS
	qhandle_t	noammoShader;
#endif

	qhandle_t	smokePuffShader;
	qhandle_t	smokePuffRageProShader;
	qhandle_t	shotgunSmokePuffShader;
	qhandle_t	plasmaBallShader;
	qhandle_t	waterBubbleShader;
	qhandle_t	bloodTrailShader;
#ifndef SMOKINGUNS
	qhandle_t	nailPuffShader;
	qhandle_t	blueProxMine;
#endif

	qhandle_t	numberShaders[11];
#ifdef SMOKINGUNS
	qhandle_t	numbermoneyShaders[11];
#endif

	qhandle_t	shadowMarkShader;

	qhandle_t	botSkillShaders[5];

	// wall mark shaders
	qhandle_t	wakeMarkShader;
	qhandle_t	bloodMarkShader;
	qhandle_t	bulletMarkShader;
	qhandle_t	burnMarkShader;
	qhandle_t	holeMarkShader;
	qhandle_t	energyMarkShader;

	// powerup shaders
	qhandle_t	quadShader;
	qhandle_t	redQuadShader;
	qhandle_t	quadWeaponShader;
	qhandle_t	invisShader;
	qhandle_t	regenShader;
	qhandle_t	battleSuitShader;
	qhandle_t	battleWeaponShader;
	qhandle_t	hastePuffShader;
#ifdef MISSIONPACK
	qhandle_t	redKamikazeShader;
	qhandle_t	blueKamikazeShader;
#endif

	// weapon effect models
	qhandle_t	bulletFlashModel;
	qhandle_t	ringFlashModel;
	qhandle_t	dishFlashModel;
	qhandle_t	lightningExplosionModel;

	// weapon effect shaders
	qhandle_t	railExplosionShader;
	qhandle_t	plasmaExplosionShader;
	qhandle_t	bulletExplosionShader;
	qhandle_t	rocketExplosionShader;
#ifndef SMOKINGUNS
	qhandle_t	grenadeExplosionShader;
#else
	qhandle_t	bulletPuffShader;
	qhandle_t	dynamiteExplosionShader;
#endif
	qhandle_t	bfgExplosionShader;
	qhandle_t	bloodExplosionShader;

	// special effects models
	qhandle_t	teleportEffectModel;
	qhandle_t	teleportEffectShader;
#ifndef SMOKINGUNS
	qhandle_t	kamikazeEffectModel;
	qhandle_t	kamikazeShockWave;
	qhandle_t	kamikazeHeadModel;
	qhandle_t	kamikazeHeadTrail;
	qhandle_t	guardPowerupModel;
	qhandle_t	scoutPowerupModel;
	qhandle_t	doublerPowerupModel;
	qhandle_t	ammoRegenPowerupModel;
	qhandle_t	invulnerabilityImpactModel;
	qhandle_t	invulnerabilityJuicedModel;
	qhandle_t	medkitUsageModel;
	qhandle_t	dustPuffShader;
	qhandle_t	heartShader;
	qhandle_t	invulnerabilityPowerupModel;
#else
	qhandle_t	dustPuffShader;
#endif

	// scoreboard headers
	qhandle_t	scoreboardName;
	qhandle_t	scoreboardPing;
	qhandle_t	scoreboardScore;
	qhandle_t	scoreboardTime;

	// medals shown during gameplay
	qhandle_t	medalImpressive;
	qhandle_t	medalExcellent;
	qhandle_t	medalGauntlet;
	qhandle_t	medalDefend;
	qhandle_t	medalAssist;
	qhandle_t	medalCapture;

	// sounds
#ifndef SMOKINGUNS
	sfxHandle_t	quadSound;
#endif
	sfxHandle_t	tracerSound;
	sfxHandle_t	selectSound;
#ifndef SMOKINGUNS
	sfxHandle_t	useNothingSound;
	sfxHandle_t	wearOffSound;
#else
	sfxHandle_t snd_pistol_raise;
	sfxHandle_t	snd_winch66_raise;
	sfxHandle_t	snd_lightn_raise;
	sfxHandle_t	snd_sharps_raise;
	sfxHandle_t	snd_shotgun_raise;
	sfxHandle_t snd_winch97_raise;
#endif
	sfxHandle_t	footsteps[FOOTSTEP_TOTAL][4];
#ifdef SMOKINGUNS
	sfxHandle_t	snd_death[11];

	//impact sounds on specific surfaces
	sfxHandle_t	impact[IMPACT_TOTAL][3];
#endif

	sfxHandle_t	sfx_lghit1;
	sfxHandle_t	sfx_lghit2;
	sfxHandle_t	sfx_lghit3;
#ifndef SMOKINGUNS
	sfxHandle_t	sfx_ric1;
	sfxHandle_t	sfx_ric2;
	sfxHandle_t	sfx_ric3;
	//sfxHandle_t	sfx_railg;
#else
	sfxHandle_t	sfx_ric[6];
#endif
	sfxHandle_t	sfx_rockexp;
#ifndef SMOKINGUNS
	sfxHandle_t	sfx_plasmaexp;
	sfxHandle_t	sfx_proxexp;
	sfxHandle_t	sfx_nghit;
	sfxHandle_t	sfx_nghitflesh;
	sfxHandle_t	sfx_nghitmetal;
	sfxHandle_t	sfx_chghit;
	sfxHandle_t	sfx_chghitflesh;
	sfxHandle_t	sfx_chghitmetal;
	sfxHandle_t kamikazeExplodeSound;
	sfxHandle_t kamikazeImplodeSound;
	sfxHandle_t kamikazeFarSound;
	sfxHandle_t useInvulnerabilitySound;
	sfxHandle_t invulnerabilityImpactSound1;
	sfxHandle_t invulnerabilityImpactSound2;
	sfxHandle_t invulnerabilityImpactSound3;
	sfxHandle_t invulnerabilityJuicedSound;
	sfxHandle_t obeliskHitSound1;
	sfxHandle_t obeliskHitSound2;
	sfxHandle_t obeliskHitSound3;
	sfxHandle_t	obeliskRespawnSound;
	sfxHandle_t	winnerSound;
	sfxHandle_t	loserSound;
	sfxHandle_t	gibSound;
	sfxHandle_t	gibBounce1Sound;
	sfxHandle_t	gibBounce2Sound;
	sfxHandle_t	gibBounce3Sound;
	sfxHandle_t	teleInSound;
	sfxHandle_t	teleOutSound;
#endif
	sfxHandle_t	noAmmoSound;
#ifndef SMOKINGUNS
	sfxHandle_t	respawnSound;
#endif
	sfxHandle_t talkSound;
#ifdef SMOKINGUNS
	sfxHandle_t talkSound1;
	sfxHandle_t talkSound2;
	sfxHandle_t talkSound3;
#endif
	sfxHandle_t landSound;
	sfxHandle_t fallSound;
#ifndef SMOKINGUNS
	sfxHandle_t jumpPadSound;

	sfxHandle_t oneMinuteSound;
	sfxHandle_t fiveMinuteSound;
	sfxHandle_t suddenDeathSound;

	sfxHandle_t threeFragSound;
	sfxHandle_t twoFragSound;
	sfxHandle_t oneFragSound;

	sfxHandle_t hitSound;
	sfxHandle_t hitSoundHighArmor;
	sfxHandle_t hitSoundLowArmor;
	sfxHandle_t hitTeamSound;
	sfxHandle_t impressiveSound;
	sfxHandle_t excellentSound;
	sfxHandle_t deniedSound;
	sfxHandle_t humiliationSound;
	sfxHandle_t assistSound;
	sfxHandle_t defendSound;
	sfxHandle_t firstImpressiveSound;
	sfxHandle_t firstExcellentSound;
	sfxHandle_t firstHumiliationSound;

	sfxHandle_t takenLeadSound;
	sfxHandle_t tiedLeadSound;
	sfxHandle_t lostLeadSound;

	sfxHandle_t voteNow;
	sfxHandle_t votePassed;
	sfxHandle_t voteFailed;
#endif

	sfxHandle_t watrInSound;
	sfxHandle_t watrOutSound;
	sfxHandle_t watrUnSound;

#ifndef SMOKINGUNS
	sfxHandle_t flightSound;
	sfxHandle_t medkitSound;

#ifdef MISSIONPACK
	sfxHandle_t weaponHoverSound;
#endif

	// teamplay sounds
	sfxHandle_t captureAwardSound;
	sfxHandle_t redScoredSound;
	sfxHandle_t blueScoredSound;
	sfxHandle_t redLeadsSound;
	sfxHandle_t blueLeadsSound;
	sfxHandle_t teamsTiedSound;

	sfxHandle_t	captureYourTeamSound;
	sfxHandle_t	captureOpponentSound;
	sfxHandle_t	returnYourTeamSound;
	sfxHandle_t	returnOpponentSound;
	sfxHandle_t	takenYourTeamSound;
	sfxHandle_t	takenOpponentSound;

	sfxHandle_t redFlagReturnedSound;
	sfxHandle_t blueFlagReturnedSound;
#ifdef MISSIONPACK
	sfxHandle_t neutralFlagReturnedSound;
#endif
	sfxHandle_t	enemyTookYourFlagSound;
	sfxHandle_t yourTeamTookEnemyFlagSound;
	sfxHandle_t	youHaveFlagSound;
#ifdef MISSIONPACK
	sfxHandle_t	enemyTookTheFlagSound;
	sfxHandle_t yourTeamTookTheFlagSound;
	sfxHandle_t yourBaseIsUnderAttackSound;
#endif
	sfxHandle_t holyShitSound;

	// tournament sounds
	sfxHandle_t	count3Sound;
	sfxHandle_t	count2Sound;
	sfxHandle_t	count1Sound;
	sfxHandle_t	countFightSound;
	sfxHandle_t	countPrepareSound;

	// new stuff
	qhandle_t patrolShader;
	qhandle_t assaultShader;
	qhandle_t campShader;
	qhandle_t followShader;
	qhandle_t defendShader;
	qhandle_t teamLeaderShader;
	qhandle_t retrieveShader;
	qhandle_t escortShader;
	qhandle_t flagShaders[3];
	sfxHandle_t	countPrepareTeamSound;

	sfxHandle_t ammoregenSound;
	sfxHandle_t doublerSound;
	sfxHandle_t guardSound;
	sfxHandle_t scoutSound;

#else
	qhandle_t cursor;
	qhandle_t selectCursor;
	qhandle_t sizeCursor;
#endif

#ifndef SMOKINGUNS
	sfxHandle_t	regenSound;
	sfxHandle_t	protectSound;
	sfxHandle_t	n_healthSound;
	sfxHandle_t	hgrenb1aSound;
	sfxHandle_t	hgrenb2aSound;
	sfxHandle_t	wstbimplSound;
	sfxHandle_t	wstbimpmSound;
	sfxHandle_t	wstbimpdSound;
	sfxHandle_t	wstbactvSound;
#else
	//Spoon Sounds
	sfxHandle_t knifehit2;
	sfxHandle_t	knifehit;
	sfxHandle_t dynamiteburn;
	sfxHandle_t boilerhit;
	sfxHandle_t dynamitezundan;
	sfxHandle_t sfxglassgib[2];
	sfxHandle_t	underwater;
	sfxHandle_t	taunt[2];
	sfxHandle_t	sfxstonegib;

	sfxHandle_t	peacem_reload2;
	sfxHandle_t	rifle_reload2;
	sfxHandle_t	winch97_reload2;
	//sfxHandle_t reloadRifle;
	//sfxHandle_t reloadRifle2;
	sfxHandle_t reloadShotgun;

//	sfxHandle_t buySound;

	sfxHandle_t	scopePutSound;
	sfxHandle_t	gatling_build;
	sfxHandle_t	gatling_dism;

	sfxHandle_t	sfx_gatling_loop;

	// menu/ui sounds
	sfxHandle_t	bang[3];

	//Round sounds
	sfxHandle_t roundstart;
	sfxHandle_t roundwin;
	sfxHandle_t	roundlost;

	// duel music (pocket watch)
	sfxHandle_t	duelstart[5];

	//2d graphics
	qhandle_t	itembound;

	//gatling gun models
	qhandle_t	g_tripod;
	qhandle_t	g_middle;
	qhandle_t	g_gatling;
	qhandle_t	g_barrel;
	qhandle_t	g_crank;
	qhandle_t	g_hand;
	qhandle_t	g_mag;
	qhandle_t	g_mag_v;

	// duel
	qhandle_t	du_medal;
	sfxHandle_t	du_won_snd;
	sfxHandle_t	du_medal_snd;

	// ai_node
	qhandle_t	ai_node;

	// spectatorfly
	qhandle_t	fly;
	qhandle_t	bpoint;
	sfxHandle_t fly_sound;

	// other money-models
	qhandle_t	coins;
	qhandle_t	bills;
	qhandle_t	coins_pic;
	qhandle_t	bills_pic;

	// indicate escape point
	qhandle_t	indicate_lf;
	qhandle_t	indicate_fw;
	qhandle_t	indicate_rt;
	qhandle_t	indicate_bk;

	// a quad model for bullet holes in breakables, etc.
	qhandle_t	quad;
#endif

} cgMedia_t;


// The client game static (cgs) structure hold everything
// loaded or calculated from the gamestate.  It will NOT
// be cleared when a tournement restart is done, allowing
// all clients to begin playing instantly
typedef struct {
	gameState_t		gameState;			// gamestate from server
	glconfig_t		glconfig;			// rendering configuration
	float			screenXScale;		// derived from glconfig
	float			screenYScale;
	float			screenXBias;

	int				serverCommandSequence;	// reliable command stream counter
	int				processedSnapshotNum;// the number of snapshots cgame has requested

	qboolean		localServer;		// detected on startup by checking sv_running

	// parsed from serverinfo
	gametype_t		gametype;
	int				dmflags;
	int				teamflags;
	int				fraglimit;
#ifndef SMOKINGUNS
	int				capturelimit;
#else
	int				scorelimit;
	int				duellimit;
#endif
	int				timelimit;
	int				maxclients;
	char			mapname[MAX_QPATH];
	char			redTeam[MAX_QPATH];
	char			blueTeam[MAX_QPATH];


#ifdef SMOKINGUNS
	int				newShotgunPattern;
	int				maxMoney;
	int				roundNoMoveTime;

	qboolean		specsareflies;
	qboolean		deathcam;
#endif

	int				voteTime;
	int				voteYes;
	int				voteNo;
	qboolean		voteModified;			// beep whenever changed
	char			voteString[MAX_STRING_TOKENS];

	int				teamVoteTime[2];
	int				teamVoteYes[2];
	int				teamVoteNo[2];
	qboolean		teamVoteModified[2];	// beep whenever changed
	char			teamVoteString[2][MAX_STRING_TOKENS];

	int				levelStartTime;

	int				scores1, scores2;		// from configstrings
	int				redflag, blueflag;		// flag status from configstrings
	int				flagStatus;

	qboolean  newHud;

	//
	// locally derived information from gamestate
	//
	qhandle_t		gameModels[MAX_MODELS];
	sfxHandle_t		gameSounds[MAX_SOUNDS];

	int				numInlineModels;
	qhandle_t		inlineDrawModel[MAX_MODELS];
	vec3_t			inlineModelMidpoints[MAX_MODELS];

	clientInfo_t	clientinfo[MAX_CLIENTS];

#ifndef SMOKINGUNS
	// teamchat width is *3 because of embedded color codes
	char			teamChatMsgs[TEAMCHAT_HEIGHT][TEAMCHAT_WIDTH*3+1];
	int				teamChatMsgTimes[TEAMCHAT_HEIGHT];
	int				teamChatPos;
	int				teamLastChatPos;
#else
	// chat width is *3 because of embedded color codes
	char			ChatMsgs[CHAT_HEIGHT][CHAT_WIDTH*3+1];
	int				ChatMsgTimes[CHAT_HEIGHT];
	int				ChatPos;
	int				LastChatPos;
#endif

	int cursorX;
	int cursorY;
	qboolean eventHandling;
	qboolean mouseCaptured;
	qboolean sizingHud;
	void *capturedItem;
	qhandle_t activeCursor;

	// orders
	int currentOrder;
	qboolean orderPending;
	int orderTime;
	int currentVoiceClient;
	int acceptOrderTime;
	int acceptTask;
	int acceptLeader;
	char acceptVoice[MAX_NAME_LENGTH];

	// media
	cgMedia_t		media;

#ifdef SMOKINGUNS
//unlagged - client options
	// this will be set to the server's g_delagHitscan
	int				delagHitscan;
//unlagged - client options
#endif
} cgs_t;

//==============================================================================

extern	cgs_t			cgs;
extern	cg_t			cg;
extern	centity_t		cg_entities[MAX_GENTITIES];
extern	weaponInfo_t	cg_weapons[MAX_WEAPONS];
extern	itemInfo_t		cg_items[MAX_ITEMS];
extern	markPoly_t		cg_markPolys[MAX_MARK_POLYS];
#ifdef SMOKINGUNS
extern	int				cg_burnTimes[MAX_CLIENTS];
// Added by hika
// Smokin' Guns specific
extern	cplane_t		cg_frustum[4];

// duel
extern	vec3_t		cg_intermission_origin[MAX_MAPPARTS];
extern	vec3_t		cg_intermission_angles[MAX_MAPPARTS];
#endif

extern	vmCvar_t		cg_centertime;
extern	vmCvar_t		cg_runpitch;
extern	vmCvar_t		cg_runroll;
extern	vmCvar_t		cg_bobup;
extern	vmCvar_t		cg_bobpitch;
extern	vmCvar_t		cg_bobroll;
extern	vmCvar_t		cg_swingSpeed;
extern	vmCvar_t		cg_shadows;
extern	vmCvar_t		cg_gibs;
extern	vmCvar_t		cg_drawTimer;
extern	vmCvar_t		cg_drawFPS;
extern	vmCvar_t		cg_drawSnapshot;
extern	vmCvar_t		cg_draw3dIcons;
extern	vmCvar_t		cg_drawIcons;
extern	vmCvar_t		cg_drawAmmoWarning;
extern	vmCvar_t		cg_drawCrosshair;
extern	vmCvar_t		cg_drawCrosshairNames;
extern	vmCvar_t		cg_drawRewards;
extern	vmCvar_t		cg_drawTeamOverlay;
extern	vmCvar_t		cg_teamOverlayUserinfo;
extern	vmCvar_t		cg_crosshairX;
extern	vmCvar_t		cg_crosshairY;
extern	vmCvar_t		cg_crosshairSize;
extern	vmCvar_t		cg_crosshairHealth;
extern	vmCvar_t		cg_drawStatus;
extern	vmCvar_t		cg_draw2D;
extern	vmCvar_t		cg_animSpeed;
extern	vmCvar_t		cg_debugAnim;
extern	vmCvar_t		cg_debugPosition;
extern	vmCvar_t		cg_debugEvents;
extern	vmCvar_t		cg_railTrailTime;
extern	vmCvar_t		cg_errorDecay;
extern	vmCvar_t		cg_nopredict;
extern	vmCvar_t		cg_noPlayerAnims;
extern	vmCvar_t		cg_showmiss;
extern	vmCvar_t		cg_footsteps;
extern	vmCvar_t		cg_addMarks;
extern	vmCvar_t		cg_brassTime;
extern	vmCvar_t		cg_gun_frame;
extern	vmCvar_t		cg_gun_x;
extern	vmCvar_t		cg_gun_y;
extern	vmCvar_t		cg_gun_z;
extern	vmCvar_t		cg_drawGun;
extern	vmCvar_t		cg_viewsize;
extern	vmCvar_t		cg_tracerChance;
extern	vmCvar_t		cg_tracerWidth;
extern	vmCvar_t		cg_tracerLength;
#ifndef SMOKINGUNS
extern	vmCvar_t		cg_autoswitch;
#endif
extern	vmCvar_t		cg_ignore;
extern	vmCvar_t		cg_simpleItems;
extern	vmCvar_t		cg_fov;
#ifndef SMOKINGUNS
extern	vmCvar_t		cg_zoomFov;
#endif
extern	vmCvar_t		cg_thirdPersonRange;
extern	vmCvar_t		cg_thirdPersonAngle;
extern	vmCvar_t		cg_thirdPerson;
extern	vmCvar_t		cg_lagometer;
extern	vmCvar_t		cg_drawAttacker;
extern	vmCvar_t		cg_synchronousClients;
#ifndef SMOKINGUNS
extern	vmCvar_t		cg_teamChatTime;
extern	vmCvar_t		cg_teamChatHeight;
#else
extern	vmCvar_t		cg_chatTime;
extern	vmCvar_t		cg_chatHeight;
extern	vmCvar_t		cg_chatWidth;
extern	vmCvar_t		cg_chatMode;
extern	vmCvar_t		cg_chatPosition;
extern	vmCvar_t		cg_chatBigText;
#endif
extern	vmCvar_t		cg_stats;
extern	vmCvar_t 		cg_forceModel;
extern	vmCvar_t 		cg_buildScript;
extern	vmCvar_t		cg_paused;
#ifdef SMOKINGUNS
extern	vmCvar_t		cg_menu;
#endif
extern	vmCvar_t		cg_blood;
extern	vmCvar_t		cg_predictItems;
extern	vmCvar_t		cg_deferPlayers;
extern	vmCvar_t		cg_drawFriend;
#ifndef SMOKINGUNS
extern	vmCvar_t		cg_teamChatsOnly;
extern	vmCvar_t		cg_noVoiceChats;
extern	vmCvar_t		cg_noVoiceText;
extern  vmCvar_t		cg_scorePlum;
//unlagged - smooth clients #2
// this is done server-side now
extern	vmCvar_t		cg_smoothClients;
//unlagged - smooth clients #2
#endif
extern	vmCvar_t		pmove_fixed;
extern	vmCvar_t		pmove_msec;
//extern	vmCvar_t		cg_pmove_fixed;
extern	vmCvar_t		cg_cameraOrbit;
extern	vmCvar_t		cg_cameraOrbitDelay;
extern	vmCvar_t		cg_timescaleFadeEnd;
extern	vmCvar_t		cg_timescaleFadeSpeed;
extern	vmCvar_t		cg_timescale;
extern	vmCvar_t		cg_cameraMode;
extern  vmCvar_t		cg_smallFont;
extern  vmCvar_t		cg_bigFont;
extern	vmCvar_t		cg_noTaunt;
#ifndef SMOKINGUNS
extern	vmCvar_t		cg_noProjectileTrail;
extern	vmCvar_t		cg_oldRail;
extern	vmCvar_t		cg_oldRocket;
extern	vmCvar_t		cg_oldPlasma;
extern	vmCvar_t		cg_trueLightning;
#else
extern	vmCvar_t		cg_redTeamName;
extern	vmCvar_t		cg_blueTeamName;
extern	vmCvar_t		cg_currentSelectedPlayer;
extern	vmCvar_t		cg_currentSelectedPlayerName;
extern	vmCvar_t		cg_singlePlayer;
extern	vmCvar_t		cg_enableDust;
extern	vmCvar_t		cg_enableBreath;
extern	vmCvar_t		cg_singlePlayerActive;
extern  vmCvar_t		cg_recordSPDemo;
extern  vmCvar_t		cg_recordSPDemoName;
extern	vmCvar_t		cg_obeliskRespawnDelay;
#endif

//unlagged - client options
#ifdef SMOKINGUNS
extern	vmCvar_t		cg_delag;
extern	vmCvar_t		cg_debugDelag;
extern	vmCvar_t		cg_drawBBox;
extern	vmCvar_t		cg_cmdTimeNudge;
extern	vmCvar_t		sv_fps;
extern	vmCvar_t		cg_projectileNudge;
extern	vmCvar_t		cg_optimizePrediction;
extern	vmCvar_t		cl_timeNudge;
extern	vmCvar_t		cg_latentSnaps;
extern	vmCvar_t		cg_latentCmds;
extern	vmCvar_t		cg_plOut;
//unlagged - client options

extern	vmCvar_t		cg_farclip;
extern	vmCvar_t		cg_farclipValue;
extern	vmCvar_t		cg_farclipZoomValue;

extern	vmCvar_t		cg_mapLOD;

extern	vmCvar_t		cg_maxMoney;
extern	vmCvar_t		cg_newShotgunPattern;
extern	vmCvar_t		cg_roundNoMoveTime;

// Smokin'Guns Cvars
//extern	vmCvar_t		cg_sg_lefthanded;

extern	vmCvar_t		cg_buydebug;
extern	vmCvar_t		cg_serverfraglimit;
extern	vmCvar_t		cg_serverduellimit;

//extern	vmCvar_t		cg_roundmusic;
extern	vmCvar_t		cg_impactparticles;
extern	vmCvar_t		cg_gunsmoke;
extern	vmCvar_t		cg_addguns;
extern	vmCvar_t		cg_killmsg;
extern	vmCvar_t		cg_hitmsg;
extern	vmCvar_t		cg_hitfarmsg;
extern	vmCvar_t		cg_ownhitmsg;
extern	vmCvar_t		cg_playownflysound;
extern	vmCvar_t		cg_showescape;
extern	vmCvar_t		cg_debug;
extern	vmCvar_t		cg_glowflares;
extern	vmCvar_t		cg_boostfps;
extern	vmCvar_t		cg_drawdebug;
extern	vmCvar_t		cg_drawspeed;


//extern	vmCvar_t		cg_teamredcount;
//extern	vmCvar_t		cg_teambluecount;

//talk sound
extern	vmCvar_t		cg_talksound;

//get key binding for +attack, +forward, +back, +left, +right, etc
extern	vmCvar_t		cg_button_attack[2];
extern	vmCvar_t		cg_button_altattack[2];
/*extern	vmCvar_t		cg_button_forward[2];
extern	vmCvar_t		cg_button_back[2];
extern	vmCvar_t		cg_button_moveleft[2];
extern	vmCvar_t		cg_button_moveright[2];
extern	vmCvar_t		cg_button_turnleft[2];
extern	vmCvar_t		cg_button_turnright[2];*/

//extern	qboolean	cg_keylist[K_LAST_KEY];


extern	vec3_t			ai_nodes[MAX_AINODES];
extern	vec3_t			ai_angles[MAX_AINODES];
extern	int				ai_nodepointer;

extern	qboolean		cg_cheats;

extern	vmCvar_t		hit_model;

extern	vmCvar_t		cg_warmupmessage;

/*extern	int				frame_lower;
extern	int				oldframe_lower;
extern	float			backlerp_lower;

extern	int				frame_upper;
extern	int				oldframe_upper;
extern	float			backlerp_upper;*/
extern	int				sa_engine_inuse;


//unlagged - cg_unlagged.c
void CG_PredictWeaponEffects( centity_t *cent );
void CG_AddBoundingBox( centity_t *cent );
qboolean CG_Cvar_ClampInt( const char *name, vmCvar_t *vmCvar, int min, int max );
//unlagged - cg_unlagged.c
#endif

//
// cg_main.c
//
const char *CG_ConfigString( int index );
const char *CG_Argv( int arg );

void QDECL CG_Printf( const char *msg, ... ) __attribute__ ((format (printf, 1, 2)));
void QDECL CG_Error( const char *msg, ... ) __attribute__ ((noreturn, format (printf, 1, 2)));

#ifndef SMOKINGUNS
void CG_StartMusic( void );
#else
void CG_SelectMusic( void );
void CG_PlayMusic( void );
#endif

void CG_UpdateCvars( void );

int CG_CrosshairPlayer( void );
int CG_LastAttacker( void );
void CG_LoadMenus(const char *menuFile);
void CG_KeyEvent(int key, qboolean down);
void CG_MouseEvent(int x, int y);
void CG_EventHandling(int type);
#ifndef SMOKINGUNS
void CG_RankRunFrame( void );
#endif
void CG_SetScoreSelection(void *menu);
score_t *CG_GetSelectedScore( void );
void CG_BuildSpectatorString( void );


//
// cg_view.c
//
void CG_TestModel_f (void);
void CG_TestGun_f (void);
void CG_TestModelNextFrame_f (void);
void CG_TestModelPrevFrame_f (void);
void CG_TestModelNextSkin_f (void);
void CG_TestModelPrevSkin_f (void);
void CG_ZoomDown_f( void );
void CG_ZoomUp_f( void );
void CG_AddBufferedSound( sfxHandle_t sfx);

void CG_DrawActiveFrame( int serverTime, stereoFrame_t stereoView, qboolean demoPlayback );


//
// cg_drawtools.c
//
void CG_AdjustFrom640( float *x, float *y, float *w, float *h );
void CG_FillRect( float x, float y, float width, float height, const float *color );
void CG_DrawPic( float x, float y, float width, float height, qhandle_t hShader );
void CG_DrawString( float x, float y, const char *string,
				   float charWidth, float charHeight, const float *modulate );


void CG_DrawStringExt( int x, int y, const char *string, const float *setColor,
		qboolean forceColor, qboolean shadow, int charWidth, int charHeight, int maxChars );
void CG_DrawBigString( int x, int y, const char *s, float alpha );
void CG_DrawBigStringColor( int x, int y, const char *s, vec4_t color );
void CG_DrawSmallString( int x, int y, const char *s, float alpha );
void CG_DrawSmallStringColor( int x, int y, const char *s, vec4_t color );
#ifdef SMOKINGUNS
void UI_DrawProportionalString2( int x, int y, const char* str, vec4_t color, float sizeScale, qhandle_t charset );
#endif

int CG_DrawStrlen( const char *str );

float	*CG_FadeColor( int startMsec, int totalMsec );
float *CG_TeamColor( int team );
void CG_TileClear( void );
void CG_ColorForHealth( vec4_t hcolor );
void CG_GetColorForHealth( int health, int armor, vec4_t hcolor );

void UI_DrawProportionalString( int x, int y, const char* str, int style, vec4_t color );
void CG_DrawRect( float x, float y, float width, float height, float size, const float *color );
void CG_DrawSides(float x, float y, float w, float h, float size);
void CG_DrawTopBottom(float x, float y, float w, float h, float size);


//
// cg_draw.c, cg_newDraw.c
//
extern	int sortedTeamPlayers[TEAM_MAXOVERLAY];
extern	int	numSortedTeamPlayers;
extern	int drawTeamOverlayModificationCount;
extern  char systemChat[256];
extern  char teamChat1[256];
extern  char teamChat2[256];

void CG_AddLagometerFrameInfo( void );
void CG_AddLagometerSnapshotInfo( snapshot_t *snap );
void CG_CenterPrint( const char *str, int y, int charWidth );
void CG_DrawHead( float x, float y, float w, float h, int clientNum, vec3_t headAngles );
void CG_DrawActive( stereoFrame_t stereoView );
void CG_DrawFlagModel( float x, float y, float w, float h, int team, qboolean force2D );
void CG_DrawTeamBackground( int x, int y, int w, int h, float alpha, int team );

#ifndef SMOKINGUNS
void CG_OwnerDraw(float x, float y, float w, float h, float text_x, float text_y, int ownerDraw, int ownerDrawFlags, int align, float special, float scale, vec4_t color, qhandle_t shader, int textStyle);
#else
// Tequila: It needs itemDef_t type definition from ui_shared.h
#include "../ui/ui_shared.h"
void CG_OwnerDraw(itemDef_t *item, float x, float y, float w, float h, float text_x, float text_y, int ownerDraw, int ownerDrawFlags, int align, float special, float scale, vec4_t color, qhandle_t shader, int textStyle);
#endif

void CG_Text_Paint(float x, float y, float scale, vec4_t color, const char *text, float adjust, int limit, int style);
int CG_Text_Width(const char *text, float scale, int limit);
int CG_Text_Height(const char *text, float scale, int limit);
void CG_SelectPrevPlayer( void );
void CG_SelectNextPlayer( void );
float CG_GetValue(int ownerDraw);
qboolean CG_OwnerDrawVisible(int flags);
void CG_RunMenuScript(char **args);
void CG_ShowResponseHead( void );
void CG_SetPrintString(int type, const char *p);
void CG_InitTeamChat( void );
void CG_GetTeamColor(vec4_t *color);
const char *CG_GetGameStatusText( void );
const char *CG_GetKillerText( void );
void CG_Draw3DModel(float x, float y, float w, float h, qhandle_t model, qhandle_t skin, vec3_t origin, vec3_t angles);
void CG_Text_PaintChar(float x, float y, float width, float height, float scale, float s, float t, float s2, float t2, qhandle_t hShader);
#ifdef SMOKINGUNS
void CG_Text_PaintCenter(float x, float y, float scale, vec4_t color, const char *text, float adjust, int limit, int style);
#endif
void CG_CheckOrderPending( void );
const char *CG_GameTypeString( void );
qboolean CG_YourTeamHasFlag( void );
qboolean CG_OtherTeamHasFlag( void );
qhandle_t CG_StatusHandle(int task);



//
// cg_player.c
//
#ifdef SMOKINGUNS
float	CG_MachinegunSpinAngle( centity_t *cent/*, qboolean reloading*/ );
void CG_Turret( centity_t *cent );
void CG_Fly( centity_t *cent);
#endif
void CG_Player( centity_t *cent );
void CG_ResetPlayerEntity( centity_t *cent );
void CG_AddRefEntityWithPowerups( refEntity_t *ent, entityState_t *state, int team );
void CG_NewClientInfo( int clientNum );
sfxHandle_t	CG_CustomSound( int clientNum, const char *soundName );

//
// cg_predict.c
//
#ifndef SMOKINGUNS
void CG_BuildSolidList( void );
#else
void CG_BuildSolidList( qboolean light );
void CG_BuildVisibleSolidList( qboolean light );
int CG_PointContentsVisible( const vec3_t point, int passEntityNum );
#endif
int	CG_PointContents( const vec3_t point, int passEntityNum );
void CG_Trace( trace_t *result, const vec3_t start, const vec3_t mins, const vec3_t maxs, const vec3_t end,
					 int skipNumber, int mask );
#ifdef SMOKINGUNS
int CG_Trace_New( trace_t *result, const vec3_t start, const vec3_t mins, const vec3_t maxs, const vec3_t end,
					 int skipNumber, int mask );
void CG_Trace2( trace_t *result, const vec3_t start, const vec3_t mins, const vec3_t maxs, const vec3_t end,
					 int skipNumber, int mask );
int CG_Trace_Visible( trace_t *result, const vec3_t start, const vec3_t mins, const vec3_t maxs, const vec3_t end,
					 int skipNumber, int mask, vec_t max_distance );
int CG_Trace_Flare( trace_t *result, const vec3_t start, const vec3_t mins, const vec3_t maxs, const vec3_t end,
					 int skipNumber, int mask, vec_t max_distance );
int CG_Trace_Fragment( trace_t *result, const vec3_t start, const vec3_t mins, const vec3_t maxs, const vec3_t end,
					 int skipNumber, int mask );
#endif
void CG_PredictPlayerState( void );
void CG_LoadDeferredPlayers( void );


//
// cg_events.c
//
void CG_CheckEvents( centity_t *cent );
const char	*CG_PlaceString( int rank );
void CG_EntityEvent( centity_t *cent, vec3_t position );
void CG_PainEvent( centity_t *cent, int health );


//
// cg_ents.c
//
void CG_SetEntitySoundPosition( centity_t *cent );
void CG_AddPacketEntities( void );
void CG_Beam( centity_t *cent );
void CG_AdjustPositionForMover(const vec3_t in, int moverNum, int fromTime, int toTime, vec3_t out, vec3_t angles_in, vec3_t angles_out);

void CG_PositionEntityOnTag( refEntity_t *entity, const refEntity_t *parent,
							qhandle_t parentModel, char *tagName );
#ifdef SMOKINGUNS
void CG_PositionViewWeaponOnTag( refEntity_t *entity, const refEntity_t *parent,
							qhandle_t parentModel, char *tagName );
#endif
void CG_PositionRotatedEntityOnTag( refEntity_t *entity, const refEntity_t *parent,
							qhandle_t parentModel, char *tagName );



//
// cg_weapons.c
//
void CG_NextWeapon_f( void );
void CG_PrevWeapon_f( void );
void CG_Weapon_f( void );
#ifdef SMOKINGUNS
void CG_LastUsedWeapon_f(void);
#endif

void CG_RegisterWeapon( int weaponNum );
void CG_RegisterItemVisuals( int itemNum );

#ifndef SMOKINGUNS
void CG_FireWeapon( centity_t *cent );
void CG_MissileHitWall( int weapon, int clientNum, vec3_t origin, vec3_t dir, impactSound_t soundType );
#else
void CG_FireWeapon( centity_t *cent, qboolean altfire, int weapon);
void CG_MissileHitWall( int weapon, int clientNum, vec3_t origin, vec3_t dir, impactSound_t soundType, int surfaceFlags, int shaderNum, qboolean fire, vec3_t dirs[ALC_COUNT], int entityNum);
#endif
void CG_MissileHitPlayer( int weapon, vec3_t origin, vec3_t dir, int entityNum );
void CG_ShotgunFire( entityState_t *es );
#ifndef SMOKINGUNS
void CG_Bullet( vec3_t origin, int sourceEntityNum, vec3_t normal, qboolean flesh, int fleshEntityNum );
#else
void CG_Bullet( vec3_t origin, int sourceEntityNum, vec3_t normal, qboolean flesh, int fleshEntityNum , int weapon, int surfaceFlags, int shaderNum, int entityNum);
void CG_MakeSmokePuff(entityState_t *ent);
void CG_BulletTracer( vec3_t start, vec3_t end, int number, int entityNum );
#endif

void CG_RailTrail( clientInfo_t *ci, vec3_t start, vec3_t end );
void CG_GrappleTrail( centity_t *ent, const weaponInfo_t *wi );
void CG_AddViewWeapon (playerState_t *ps);
void CG_AddPlayerWeapon( refEntity_t *parent, playerState_t *ps, centity_t *cent, int team );
void CG_DrawWeaponSelect( void );

void CG_OutOfAmmoChange( void );	// should this be in pmove?

//
// cg_marks.c
//
void	CG_InitMarkPolys( void );
void	CG_AddMarks( void );
void	CG_ImpactMark( qhandle_t markShader,
				    const vec3_t origin, const vec3_t dir,
					float orientation,
				    float r, float g, float b, float a,
					qboolean alphaFade,
#ifndef SMOKINGUNS
					float radius, qboolean temporary );
#else
					float radius, qboolean temporary, int duration );
#endif

//
// cg_localents.c
//
void	CG_InitLocalEntities( void );
localEntity_t	*CG_AllocLocalEntity( void );
void	CG_AddLocalEntities( void );
#ifdef SMOKINGUNS
void	CG_CreateFire(vec3_t origin, vec3_t normal);
localEntity_t	*CG_GetActiveLocalEntities( void );
#endif

//
// cg_effects.c
//
localEntity_t *CG_SmokePuff( const vec3_t p,
				   const vec3_t vel,
				   float radius,
				   float r, float g, float b, float a,
				   float duration,
				   int startTime,
				   int fadeInTime,
				   int leFlags,
				   qhandle_t hShader );
void CG_BubbleTrail( vec3_t start, vec3_t end, float spacing );
void CG_SpawnEffect( vec3_t org );
#ifndef SMOKINGUNS
void CG_KamikazeEffect( vec3_t org );
void CG_ObeliskExplode( vec3_t org, int entityNum );
void CG_ObeliskPain( vec3_t org );
void CG_InvulnerabilityImpact( vec3_t org, vec3_t angles );
void CG_InvulnerabilityJuiced( vec3_t org );
void CG_LightningBoltBeam( vec3_t start, vec3_t end );
void CG_ScorePlum( int client, vec3_t org, int score );
#endif

void CG_GibPlayer( vec3_t playerOrigin );
void CG_BigExplode( vec3_t playerOrigin );

void CG_Bleed( vec3_t origin, int entityNum );

localEntity_t *CG_MakeExplosion( vec3_t origin, vec3_t dir,
								qhandle_t hModel, qhandle_t shader, int msec,
#ifndef SMOKINGUNS
								qboolean isSprite );
#else
								qboolean isSprite, float radius, qboolean smokeexp );
#endif

//
// cg_snapshot.c
//
void CG_ProcessSnapshots( void );
#ifdef SMOKINGUNS
//unlagged - early transitioning
void CG_TransitionEntity( centity_t *cent );
//unlagged - early transitioning
#endif

//
// cg_info.c
//
void CG_LoadingString( const char *s );
void CG_LoadingItem( int itemNum );
void CG_LoadingClient( int clientNum );
void CG_DrawInformation( void );

#ifdef SMOKINGUNS
void CG_LoadingStage( int amount);
#endif

//
// cg_consolecmds.c
//
qboolean CG_ConsoleCommand( void );
void CG_InitConsoleCommands( void );
#ifdef SMOKINGUNS
void CG_CloseBuyMenu( void );
#endif

//
// cg_servercmds.c
//
void CG_ExecuteNewServerCommands( int latestSequence );
void CG_ParseServerinfo( void );
void CG_SetConfigValues( void );
void CG_ShaderStateChanged(void);
#ifndef SMOKINGUNS
void CG_LoadVoiceChats( void );
void CG_VoiceChatLocal( int mode, qboolean voiceOnly, int clientNum, int color, const char *cmd );
void CG_PlayBufferedVoiceChats( void );
#endif

//
// cg_playerstate.c
//
void CG_Respawn( void );
void CG_TransitionPlayerState( playerState_t *ps, playerState_t *ops );
void CG_CheckChangedPredictableEvents( playerState_t *ps );


//===============================================

//
// system traps
// These functions are how the cgame communicates with the main game system
//

// print message on the local console
void		trap_Print( const char *fmt );

// abort the game
void		trap_Error(const char *fmt) __attribute__((noreturn));

// milliseconds should only be used for performance tuning, never
// for anything game related.  Get time from the CG_DrawActiveFrame parameter
int			trap_Milliseconds( void );

// console variable interaction
void		trap_Cvar_Register( vmCvar_t *vmCvar, const char *varName, const char *defaultValue, int flags );
void		trap_Cvar_Update( vmCvar_t *vmCvar );
void		trap_Cvar_Set( const char *var_name, const char *value );
void		trap_Cvar_VariableStringBuffer( const char *var_name, char *buffer, int bufsize );

// ServerCommand and ConsoleCommand parameter access
int			trap_Argc( void );
void		trap_Argv( int n, char *buffer, int bufferLength );
void		trap_Args( char *buffer, int bufferLength );

// filesystem access
// returns length of file
int			trap_FS_FOpenFile( const char *qpath, fileHandle_t *f, fsMode_t mode );
void		trap_FS_Read( void *buffer, int len, fileHandle_t f );
void		trap_FS_Write( const void *buffer, int len, fileHandle_t f );
void		trap_FS_FCloseFile( fileHandle_t f );
int			trap_FS_Seek( fileHandle_t f, long offset, int origin ); // fsOrigin_t

// add commands to the local console as if they were typed in
// for map changing, etc.  The command is not executed immediately,
// but will be executed in order the next time console commands
// are processed
void		trap_SendConsoleCommand( const char *text );

// register a command name so the console can perform command completion.
// FIXME: replace this with a normal console command "defineCommand"?
void		trap_AddCommand( const char *cmdName );
void		trap_RemoveCommand( const char *cmdName );

// send a string to the server over the network
void		trap_SendClientCommand( const char *s );

// force a screen update, only used during gamestate load
void		trap_UpdateScreen( void );

// model collision
void		trap_CM_LoadMap( const char *mapname );
int			trap_CM_NumInlineModels( void );
clipHandle_t trap_CM_InlineModel( int index );		// 0 = world, 1+ = bmodels
clipHandle_t trap_CM_TempBoxModel( const vec3_t mins, const vec3_t maxs );
int			trap_CM_PointContents( const vec3_t p, clipHandle_t model );
int			trap_CM_TransformedPointContents( const vec3_t p, clipHandle_t model, const vec3_t origin, const vec3_t angles );
void		trap_CM_BoxTrace( trace_t *results, const vec3_t start, const vec3_t end,
					  const vec3_t mins, const vec3_t maxs,
					  clipHandle_t model, int brushmask );
void		trap_CM_CapsuleTrace( trace_t *results, const vec3_t start, const vec3_t end,
					  const vec3_t mins, const vec3_t maxs,
					  clipHandle_t model, int brushmask );
void		trap_CM_TransformedBoxTrace( trace_t *results, const vec3_t start, const vec3_t end,
					  const vec3_t mins, const vec3_t maxs,
					  clipHandle_t model, int brushmask,
					  const vec3_t origin, const vec3_t angles );
void		trap_CM_TransformedCapsuleTrace( trace_t *results, const vec3_t start, const vec3_t end,
					  const vec3_t mins, const vec3_t maxs,
					  clipHandle_t model, int brushmask,
					  const vec3_t origin, const vec3_t angles );

// Returns the projection of a polygon onto the solid brushes in the world
int			trap_CM_MarkFragments( int numPoints, const vec3_t *points,
			const vec3_t projection,
			int maxPoints, vec3_t pointBuffer,
			int maxFragments, markFragment_t *fragmentBuffer );

// normal sounds will have their volume dynamically changed as their entity
// moves and the listener moves
void		trap_S_StartSound( vec3_t origin, int entityNum, int entchannel, sfxHandle_t sfx );
void		trap_S_StopLoopingSound(int entnum);

// a local sound is always played full volume
void		trap_S_StartLocalSound( sfxHandle_t sfx, int channelNum );
void		trap_S_ClearLoopingSounds( qboolean killall );
void		trap_S_AddLoopingSound( int entityNum, const vec3_t origin, const vec3_t velocity, sfxHandle_t sfx );
void		trap_S_AddRealLoopingSound( int entityNum, const vec3_t origin, const vec3_t velocity, sfxHandle_t sfx );
void		trap_S_UpdateEntityPosition( int entityNum, const vec3_t origin );

// respatialize recalculates the volumes of sound as they should be heard by the
// given entityNum and position
void		trap_S_Respatialize( int entityNum, const vec3_t origin, vec3_t axis[3], int inwater );
sfxHandle_t	trap_S_RegisterSound( const char *sample, qboolean compressed );		// returns buzz if not found
void		trap_S_StartBackgroundTrack( const char *intro, const char *loop );	// empty name stops music
void	trap_S_StopBackgroundTrack( void );


void		trap_R_LoadWorldMap( const char *mapname );

// all media should be registered during level startup to prevent
// hitches during gameplay
qhandle_t	trap_R_RegisterModel( const char *name );			// returns rgb axis if not found
qhandle_t	trap_R_RegisterSkin( const char *name );			// returns all white if not found
qhandle_t	trap_R_RegisterShader( const char *name );			// returns all white if not found
qhandle_t	trap_R_RegisterShaderNoMip( const char *name );			// returns all white if not found

// a scene is built up by calls to R_ClearScene and the various R_Add functions.
// Nothing is drawn until R_RenderScene is called.
void		trap_R_ClearScene( void );
void		trap_R_AddRefEntityToScene( const refEntity_t *re );

// polys are intended for simple wall marks, not really for doing
// significant construction
void		trap_R_AddPolyToScene( qhandle_t hShader , int numVerts, const polyVert_t *verts );
void		trap_R_AddPolysToScene( qhandle_t hShader , int numVerts, const polyVert_t *verts, int numPolys );
void		trap_R_AddLightToScene( const vec3_t org, float intensity, float r, float g, float b );
void		trap_R_AddAdditiveLightToScene( const vec3_t org, float intensity, float r, float g, float b );
int			trap_R_LightForPoint( vec3_t point, vec3_t ambientLight, vec3_t directedLight, vec3_t lightDir );
void		trap_R_RenderScene( const refdef_t *fd );
void		trap_R_SetColor( const float *rgba );	// NULL = 1,1,1,1
void		trap_R_DrawStretchPic( float x, float y, float w, float h,
			float s1, float t1, float s2, float t2, qhandle_t hShader );
void		trap_R_ModelBounds( clipHandle_t model, vec3_t mins, vec3_t maxs );
int			trap_R_LerpTag( orientation_t *tag, clipHandle_t mod, int startFrame, int endFrame,
					   float frac, const char *tagName );
void		trap_R_RemapShader( const char *oldShader, const char *newShader, const char *timeOffset );
qboolean	trap_R_inPVS( const vec3_t p1, const vec3_t p2 );

// The glconfig_t will not change during the life of a cgame.
// If it needs to change, the entire cgame will be restarted, because
// all the qhandle_t are then invalid.
void		trap_GetGlconfig( glconfig_t *glconfig );

// the gamestate should be grabbed at startup, and whenever a
// configstring changes
void		trap_GetGameState( gameState_t *gamestate );

// cgame will poll each frame to see if a newer snapshot has arrived
// that it is interested in.  The time is returned seperately so that
// snapshot latency can be calculated.
void		trap_GetCurrentSnapshotNumber( int *snapshotNumber, int *serverTime );

// a snapshot get can fail if the snapshot (or the entties it holds) is so
// old that it has fallen out of the client system queue
qboolean	trap_GetSnapshot( int snapshotNumber, snapshot_t *snapshot );

// retrieve a text command from the server stream
// the current snapshot will hold the number of the most recent command
// qfalse can be returned if the client system handled the command
// argc() / argv() can be used to examine the parameters of the command
qboolean	trap_GetServerCommand( int serverCommandNumber );

// returns the most recent command number that can be passed to GetUserCmd
// this will always be at least one higher than the number in the current
// snapshot, and it may be quite a few higher if it is a fast computer on
// a lagged connection
int			trap_GetCurrentCmdNumber( void );

qboolean	trap_GetUserCmd( int cmdNumber, usercmd_t *ucmd );

// used for the weapon select and zoom
void		trap_SetUserCmdValue( int stateValue, float sensitivityScale );

// aids for VM testing
void		testPrintInt( char *string, int i );
void		testPrintFloat( char *string, float f );

int			trap_MemoryRemaining( void );
void		trap_R_RegisterFont(const char *fontName, int pointSize, fontInfo_t *font);
qboolean	trap_Key_IsDown( int keynum );
int			trap_Key_GetCatcher( void );
void		trap_Key_SetCatcher( int catcher );
int			trap_Key_GetKey( const char *binding );

//cg_sg_utils.c
#ifdef SMOKINGUNS
#define BOUNCE_LIGHT	0
#define BOUNCE_MEDIUM	1
#define BOUNCE_HEAVY	2

localEntity_t *CG_CreateBulletHole( vec3_t origin, vec3_t dir, int surfaceFlags,
								   int entityNum);
localEntity_t *CG_LaunchParticle( vec3_t origin, vec3_t dir, qhandle_t hModel, int snd,
						int bouncefactor, int duration,
						int leFlags);
void CG_LaunchFuncBreakable ( centity_t *cent );
void CG_LaunchImpactParticle( vec3_t origin, vec3_t dir, int surfaceFlags, int shaderNum, int weapon, qboolean flesh);
void CG_CheckSparkGen(localEntity_t	*le);
void CG_LightForPoint(vec3_t origin, vec3_t normal, vec4_t color);
localEntity_t *CG_LaunchSpriteParticle( vec3_t origin, vec3_t dir, /*int sound,*/ int bouncefactor, int duration,
						int leFlags);

void BuildHitModel(hit_data_t *data, vec3_t origin,
				   vec3_t angles, vec3_t torso_angles, vec3_t head_angles,
				   int frame_upper, int oldframe_upper, float backlerp_upper,
				   int frame_lower, int oldframe_lower, float backlerp_lower);
qboolean CG_CheckPlayerVisible(vec3_t view, vec3_t player);
void CG_PlayReloadSound(int weapon, centity_t *cent, qboolean sec);
localEntity_t *CG_AddSmokeParticle(vec3_t origin, int radius, int randomradius, int lifetime,
								   int boost, int wind, vec3_t winddir);
void CG_DeleteRoundEntities( void );
#endif

typedef enum {
  SYSTEM_PRINT,
  CHAT_PRINT,
  TEAMCHAT_PRINT
} q3print_t;


int trap_CIN_PlayCinematic( const char *arg0, int xpos, int ypos, int width, int height, int bits);
e_status trap_CIN_StopCinematic(int handle);
e_status trap_CIN_RunCinematic (int handle);
void trap_CIN_DrawCinematic (int handle);
void trap_CIN_SetExtents (int handle, int x, int y, int w, int h);

int			trap_RealTime(qtime_t *qtime);
void		trap_SnapVector( float *v );

qboolean	trap_loadCamera(const char *name);
void		trap_startCamera(int time);
qboolean	trap_getCameraInfo(int time, vec3_t *origin, vec3_t *angles);

qboolean	trap_GetEntityToken( char *buffer, int bufferSize );

#ifndef SMOKINGUNS
void	CG_ClearParticles (void);
void	CG_AddParticles (void);
void	CG_ParticleSnow (qhandle_t pshader, vec3_t origin, vec3_t origin2, int turb, float range, int snum);
void	CG_ParticleSmoke (qhandle_t pshader, centity_t *cent);
void	CG_AddParticleShrapnel (localEntity_t *le);
void	CG_ParticleSnowFlurry (qhandle_t pshader, centity_t *cent);
void	CG_ParticleBulletDebris (vec3_t	org, vec3_t vel, int duration);
void	CG_ParticleSparks (vec3_t org, vec3_t vel, int duration, float x, float y, float speed);
void	CG_ParticleDust (centity_t *cent, vec3_t origin, vec3_t dir);
void	CG_ParticleMisc (qhandle_t pshader, vec3_t origin, int size, int duration, float alpha);
void	CG_ParticleExplosion (char *animStr, vec3_t origin, vec3_t vel, int duration, int sizeStart, int sizeEnd);
extern qboolean		initparticles;
int CG_NewParticleArea ( int num );

#else
int			trap_R_CullBoundingBox( vec3_t box_vertex[8] );
int			trap_R_CullPointAndRadius( vec3_t pt, float radius );
int			trap_R_GetFrustumPlane( cplane_t frustum[4] );

void	trap_GetEnv( const char *name, char *buffer, int bufsize );

//anims
void CG_RunLerpFrame( clientInfo_t *ci, lerpFrame_t *lf, int newAnimation, float speedScale, int weapon);
void BG_CopyLerpFrame(lerpFrame_t *org, lerpFrame_t *targ);

// Talk sound
void CG_PlayTalkSound(void);

int CG_Cvar_VariableInteger(const char *cvar);
void CG_SetupFrustum( void );
qboolean CG_IsEntityVisible(centity_t *cent, vec_t max_sight);
qboolean CG_IsLocalEntityVisible(localEntity_t *lent, vec_t max_sight);
qboolean CG_EntityVectors(centity_t *cent, vec3_t origin, vec3_t box_vertex[8]);
void CG_AddBoundingBoxEntity(centity_t *cent);

#define MAX_MENU_ITEMS 6
typedef struct {
	char string[64];
	qboolean money;	// does the player have enough money?
	qboolean inventory; // does the player already have this item?
} menu_items_t;

typedef struct {
	float x;
	float y;
} coord_t;



// Far clipping by Joe Kari //
extern qboolean ( * CG_Farclip_Tester[] )( vec3_t , vec3_t , float , float ) ;
extern int CG_Farclip_Tester_Table_Size ;
#endif


