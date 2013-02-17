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
// cg_main.c -- initialization and primary entry point for cgame
#include "cg_local.h"

#ifdef SMOKINGUNS
#include "../ui/ui_shared.h"
// display context for new ui stuff
displayContextDef_t cgDC;
#endif

int forceModelModificationCount = -1;

void CG_Init( int serverMessageNum, int serverCommandSequence, int clientNum );
void CG_Shutdown( void );


/*
================
vmMain

This is the only way control passes into the module.
This must be the very first function compiled into the .q3vm file
================
*/
Q_EXPORT intptr_t vmMain( int command, int arg0, int arg1, int arg2, int arg3, int arg4, int arg5, int arg6, int arg7, int arg8, int arg9, int arg10, int arg11  ) {

	switch ( command ) {
	case CG_INIT:
		CG_Init( arg0, arg1, arg2 );
		return 0;
	case CG_SHUTDOWN:
		CG_Shutdown();
		return 0;
	case CG_CONSOLE_COMMAND:
		return CG_ConsoleCommand();
	case CG_DRAW_ACTIVE_FRAME:
		CG_DrawActiveFrame( arg0, arg1, arg2 );
		return 0;
	case CG_CROSSHAIR_PLAYER:
		return CG_CrosshairPlayer();
	case CG_LAST_ATTACKER:
		return CG_LastAttacker();
	case CG_KEY_EVENT:
		CG_KeyEvent(arg0, arg1);
		return 0;
	case CG_MOUSE_EVENT:
#ifdef SMOKINGUNS
		cgDC.cursorx = cgs.cursorX;
		cgDC.cursory = cgs.cursorY;
#endif
		CG_MouseEvent(arg0, arg1);
		return 0;
	case CG_EVENT_HANDLING:
		CG_EventHandling(arg0);
		return 0;
	default:
		CG_Error( "vmMain: unknown command %i", command );
		break;
	}
	return -1;
}


cg_t				cg;
cgs_t				cgs;
centity_t			cg_entities[MAX_GENTITIES];
weaponInfo_t		cg_weapons[MAX_WEAPONS];
itemInfo_t			cg_items[MAX_ITEMS];
#ifdef SMOKINGUNS
int					cg_burnTimes[MAX_CLIENTS];
// Smokin' Guns specific
cplane_t	cg_frustum[4];

// duel intermission points
vec3_t		cg_intermission_origin[MAX_MAPPARTS];
vec3_t		cg_intermission_angles[MAX_MAPPARTS];
#endif

vmCvar_t	cg_railTrailTime;
vmCvar_t	cg_centertime;
vmCvar_t	cg_runpitch;
vmCvar_t	cg_runroll;
vmCvar_t	cg_bobup;
vmCvar_t	cg_bobpitch;
vmCvar_t	cg_bobroll;
vmCvar_t	cg_swingSpeed;
vmCvar_t	cg_shadows;
vmCvar_t	cg_gibs;
vmCvar_t	cg_drawTimer;
vmCvar_t	cg_drawFPS;
vmCvar_t	cg_drawSnapshot;
vmCvar_t	cg_draw3dIcons;
vmCvar_t	cg_drawIcons;
vmCvar_t	cg_drawAmmoWarning;
vmCvar_t	cg_drawCrosshair;
vmCvar_t	cg_drawCrosshairNames;
vmCvar_t	cg_drawRewards;
vmCvar_t	cg_crosshairSize;
vmCvar_t	cg_crosshairX;
vmCvar_t	cg_crosshairY;
vmCvar_t	cg_crosshairHealth;
vmCvar_t	cg_draw2D;
vmCvar_t	cg_drawStatus;
vmCvar_t	cg_animSpeed;
vmCvar_t	cg_debugAnim;
vmCvar_t	cg_debugPosition;
vmCvar_t	cg_debugEvents;
vmCvar_t	cg_errorDecay;
vmCvar_t	cg_nopredict;
vmCvar_t	cg_noPlayerAnims;
vmCvar_t	cg_showmiss;
vmCvar_t	cg_footsteps;
vmCvar_t	cg_addMarks;
vmCvar_t	cg_brassTime;
vmCvar_t	cg_viewsize;
vmCvar_t	cg_drawGun;
vmCvar_t	cg_gun_frame;
vmCvar_t	cg_gun_x;
vmCvar_t	cg_gun_y;
vmCvar_t	cg_gun_z;
vmCvar_t	cg_tracerChance;
vmCvar_t	cg_tracerWidth;
vmCvar_t	cg_tracerLength;
#ifndef SMOKINGUNS
vmCvar_t	cg_autoswitch;
#endif
vmCvar_t	cg_ignore;
vmCvar_t	cg_simpleItems;
vmCvar_t	cg_fov;
#ifndef SMOKINGUNS
vmCvar_t	cg_zoomFov;
#endif
vmCvar_t	cg_thirdPerson;
vmCvar_t	cg_thirdPersonRange;
vmCvar_t	cg_thirdPersonAngle;
vmCvar_t	cg_lagometer;
vmCvar_t	cg_drawAttacker;
vmCvar_t	cg_synchronousClients;
#ifndef SMOKINGUNS
vmCvar_t	cg_teamChatTime;
vmCvar_t	cg_teamChatHeight;
#else
vmCvar_t	cg_chatTime;
vmCvar_t	cg_chatHeight;
vmCvar_t	cg_chatWidth;
vmCvar_t	cg_chatMode;
vmCvar_t	cg_chatPosition;
vmCvar_t	cg_chatBigText;
#endif
vmCvar_t	cg_stats;
vmCvar_t	cg_buildScript;
vmCvar_t	cg_forceModel;
vmCvar_t	cg_paused;
#ifdef SMOKINGUNS
vmCvar_t	cg_menu;
#endif
vmCvar_t	cg_blood;
vmCvar_t	cg_predictItems;
vmCvar_t	cg_deferPlayers;
vmCvar_t	cg_drawTeamOverlay;
vmCvar_t	cg_teamOverlayUserinfo;
vmCvar_t	cg_drawFriend;
#ifndef SMOKINGUNS
vmCvar_t	cg_teamChatsOnly;
#ifdef MISSIONPACK
vmCvar_t	cg_noVoiceChats;
vmCvar_t	cg_noVoiceText;
#endif
vmCvar_t	cg_hudFiles;
vmCvar_t	cg_scorePlum;
vmCvar_t	cg_smoothClients;
#endif
vmCvar_t	pmove_fixed;
//vmCvar_t	cg_pmove_fixed;
vmCvar_t	pmove_msec;
vmCvar_t	cg_pmove_msec;
vmCvar_t	cg_cameraMode;
vmCvar_t	cg_cameraOrbit;
vmCvar_t	cg_cameraOrbitDelay;
vmCvar_t	cg_timescaleFadeEnd;
vmCvar_t	cg_timescaleFadeSpeed;
vmCvar_t	cg_timescale;
vmCvar_t	cg_smallFont;
vmCvar_t	cg_bigFont;
vmCvar_t	cg_noTaunt;
#ifndef SMOKINGUNS
vmCvar_t	cg_noProjectileTrail;
vmCvar_t	cg_oldRail;
vmCvar_t	cg_oldRocket;
vmCvar_t	cg_oldPlasma;
vmCvar_t	cg_trueLightning;
#endif

vmCvar_t	cg_redTeamName;
vmCvar_t	cg_blueTeamName;
vmCvar_t	cg_currentSelectedPlayer;
vmCvar_t	cg_currentSelectedPlayerName;
vmCvar_t	cg_singlePlayer;
vmCvar_t	cg_enableDust;
vmCvar_t	cg_enableBreath;
vmCvar_t	cg_singlePlayerActive;
vmCvar_t	cg_recordSPDemo;
vmCvar_t	cg_recordSPDemoName;
#ifndef SMOKINGUNS
vmCvar_t	cg_obeliskRespawnDelay;
#endif

#ifdef SMOKINGUNS
//unlagged - client options
vmCvar_t	cg_delag;
vmCvar_t	cg_debugDelag;
vmCvar_t	cg_drawBBox;
vmCvar_t	cg_cmdTimeNudge;
vmCvar_t	sv_fps;
vmCvar_t	cg_projectileNudge;
vmCvar_t	cg_optimizePrediction;
vmCvar_t	cl_timeNudge;
vmCvar_t	cg_latentSnaps;
vmCvar_t	cg_latentCmds;
vmCvar_t	cg_plOut;
//unlagged - client options

/*
======================================
SG Cvars // by Spoon, Hika, TheDoctor
======================================
*/

vmCvar_t		cg_buydebug;
vmCvar_t		cg_serverfraglimit;
vmCvar_t		cg_serverduellimit;

vmCvar_t		cg_impactparticles;
vmCvar_t		cg_gunsmoke;
vmCvar_t		cg_addguns;
vmCvar_t		cg_killmsg;
vmCvar_t		cg_hitmsg;
vmCvar_t		cg_hitfarmsg;
vmCvar_t		cg_ownhitmsg;
vmCvar_t		cg_playownflysound;
vmCvar_t		cg_showescape;
vmCvar_t		cg_debug;
vmCvar_t		cg_glowflares;
vmCvar_t		cg_boostfps;
vmCvar_t		cg_drawdebug;
vmCvar_t		cg_drawspeed;

//talk sound
vmCvar_t		cg_talksound;

//important button bindings
vmCvar_t		cg_button_attack[2];
vmCvar_t		cg_button_altattack[2];
/*vmCvar_t		cg_button_forward[2];
vmCvar_t		cg_button_back[2];
vmCvar_t		cg_button_moveleft[2];
vmCvar_t		cg_button_moveright[2];
vmCvar_t		cg_button_turnleft[2];
vmCvar_t		cg_button_turnright[2];*/

// key list, which button is down?
//qboolean	cg_keylist[K_LAST_KEY];

vec3_t			ai_nodes[MAX_AINODES];
vec3_t			ai_angles[MAX_AINODES];
int				ai_nodepointer;

qboolean		cg_cheats;

vmCvar_t		hit_model;

vmCvar_t		cg_warmupmessage;

vmCvar_t		cg_farclip;
vmCvar_t		cg_farclipValue;
vmCvar_t		cg_farclipZoomValue;

vmCvar_t		cg_mapLOD;

// experimental
vmCvar_t		cg_newShotgunPattern;
vmCvar_t		cg_maxMoney;
//vmCvar_t		cg_availablePlaylist;
vmCvar_t		cg_roundNoMoveTime;

/*int				frame_lower;
int				oldframe_lower;
float			backlerp_lower;

int				frame_upper;
int				oldframe_upper;
float			backlerp_upper;*/
int				sa_engine_inuse;
#endif

typedef struct {
	vmCvar_t	*vmCvar;
	char		*cvarName;
	char		*defaultString;
	int		cvarFlags;
} cvarTable_t;

static cvarTable_t		cvarTable[] = {
	{ &cg_ignore, "cg_ignore", "0", 0 },	// used for debugging
#ifndef SMOKINGUNS
	{ &cg_autoswitch, "cg_autoswitch", "1", CVAR_ARCHIVE },
#endif
	{ &cg_drawGun, "cg_drawGun", "1", CVAR_ARCHIVE },
#ifndef SMOKINGUNS
	{ &cg_zoomFov, "cg_zoomfov", "22.5", CVAR_ARCHIVE },
	{ &cg_fov, "cg_fov", "90", CVAR_ARCHIVE },
#else
	{ &cg_fov, "cg_fov", "90", CVAR_CHEAT },
#endif
	{ &cg_viewsize, "cg_viewsize", "100", CVAR_ARCHIVE },
	{ &cg_shadows, "cg_shadows", "1", CVAR_ARCHIVE  },
	{ &cg_gibs, "cg_gibs", "1", CVAR_ARCHIVE  },
	{ &cg_draw2D, "cg_draw2D", "1", CVAR_ARCHIVE  },
	{ &cg_drawStatus, "cg_drawStatus", "1", CVAR_ARCHIVE  },
	{ &cg_drawTimer, "cg_drawTimer", "0", CVAR_ARCHIVE  },
	{ &cg_drawFPS, "cg_drawFPS", "0", CVAR_ARCHIVE  },
	{ &cg_drawSnapshot, "cg_drawSnapshot", "0", CVAR_ARCHIVE  },
	{ &cg_draw3dIcons, "cg_draw3dIcons", "1", CVAR_ARCHIVE  },
	{ &cg_drawIcons, "cg_drawIcons", "1", CVAR_ARCHIVE  },
	{ &cg_drawAmmoWarning, "cg_drawAmmoWarning", "1", CVAR_ARCHIVE  },
	{ &cg_drawAttacker, "cg_drawAttacker", "1", CVAR_ARCHIVE  },
#ifndef SMOKINGUNS
	{ &cg_drawCrosshair, "cg_drawCrosshair", "4", CVAR_ARCHIVE },
#else
	{ &cg_drawCrosshair, "cg_drawCrosshair", "2", CVAR_ARCHIVE },
#endif
	{ &cg_drawCrosshairNames, "cg_drawCrosshairNames", "1", CVAR_ARCHIVE },
	{ &cg_drawRewards, "cg_drawRewards", "1", CVAR_ARCHIVE },
	{ &cg_crosshairSize, "cg_crosshairSize", "24", CVAR_ARCHIVE },
	{ &cg_crosshairHealth, "cg_crosshairHealth", "1", CVAR_ARCHIVE },
	{ &cg_crosshairX, "cg_crosshairX", "0", CVAR_ARCHIVE },
	{ &cg_crosshairY, "cg_crosshairY", "0", CVAR_ARCHIVE },
	{ &cg_brassTime, "cg_brassTime", "2500", CVAR_ARCHIVE },
	{ &cg_simpleItems, "cg_simpleItems", "0", CVAR_ARCHIVE },
	{ &cg_addMarks, "cg_marks", "1", CVAR_ARCHIVE },
	{ &cg_lagometer, "cg_lagometer", "1", CVAR_ARCHIVE },
	{ &cg_railTrailTime, "cg_railTrailTime", "400", CVAR_ARCHIVE  },
	{ &cg_gun_x, "cg_gunX", "0", CVAR_CHEAT },
	{ &cg_gun_y, "cg_gunY", "0", CVAR_CHEAT },
	{ &cg_gun_z, "cg_gunZ", "0", CVAR_CHEAT },
#ifndef SMOKINGUNS
	{ &cg_centertime, "cg_centertime", "3", CVAR_CHEAT },
#else
	{ &cg_centertime, "cg_centertime", "2", CVAR_CHEAT },
#endif
	{ &cg_runpitch, "cg_runpitch", "0.002", CVAR_ARCHIVE},
	{ &cg_runroll, "cg_runroll", "0.005", CVAR_ARCHIVE },
#ifndef SMOKINGUNS
	{ &cg_bobup , "cg_bobup", "0.005", CVAR_CHEAT },
	{ &cg_bobpitch, "cg_bobpitch", "0.002", CVAR_ARCHIVE },
	{ &cg_bobroll, "cg_bobroll", "0.002", CVAR_ARCHIVE },
#else
	{ &cg_bobup , "cg_bobup", "0.000", CVAR_CHEAT },
	{ &cg_bobpitch, "cg_bobpitch", "0.000", CVAR_CHEAT },
	{ &cg_bobroll, "cg_bobroll", "0.000", CVAR_CHEAT },
#endif
	{ &cg_swingSpeed, "cg_swingSpeed", "0.3", CVAR_CHEAT },
	{ &cg_animSpeed, "cg_animspeed", "1", CVAR_CHEAT },
	{ &cg_debugAnim, "cg_debuganim", "0", CVAR_CHEAT },
	{ &cg_debugPosition, "cg_debugposition", "0", CVAR_CHEAT },
	{ &cg_debugEvents, "cg_debugevents", "0", CVAR_CHEAT },
	{ &cg_errorDecay, "cg_errordecay", "100", 0 },
	{ &cg_nopredict, "cg_nopredict", "0", 0 },
	{ &cg_noPlayerAnims, "cg_noplayeranims", "0", CVAR_CHEAT },
	{ &cg_showmiss, "cg_showmiss", "0", 0 },
	{ &cg_footsteps, "cg_footsteps", "1", CVAR_CHEAT },
#ifndef SMOKINGUNS
	{ &cg_tracerChance, "cg_tracerchance", "0.4", CVAR_CHEAT },
	{ &cg_tracerWidth, "cg_tracerwidth", "1", CVAR_CHEAT },
	{ &cg_tracerLength, "cg_tracerlength", "100", CVAR_CHEAT },
#else
	{ &cg_tracerChance, "cg_tracerchance", "0.8", CVAR_CHEAT },
	{ &cg_tracerWidth, "cg_tracerwidth", "7", CVAR_CHEAT },
	{ &cg_tracerLength, "cg_tracerlength", "500", CVAR_CHEAT },
#endif
	{ &cg_thirdPersonRange, "cg_thirdPersonRange", "40", CVAR_CHEAT },
	{ &cg_thirdPersonAngle, "cg_thirdPersonAngle", "0", CVAR_CHEAT },
#ifndef SMOKINGUNS
	{ &cg_thirdPerson, "cg_thirdPerson", "0", 0 },
	{ &cg_teamChatTime, "cg_teamChatTime", "3000", CVAR_ARCHIVE  },
	{ &cg_teamChatHeight, "cg_teamChatHeight", "0", CVAR_ARCHIVE  },
#else
	{ &cg_thirdPerson, "cg_thirdPerson", "0", CVAR_CHEAT },
	{ &cg_chatTime, "cg_chatTime", "3000", CVAR_ARCHIVE  },
	{ &cg_chatHeight, "cg_chatHeight", "0", CVAR_ARCHIVE  },
	{ &cg_chatWidth, "cg_chatWidth", "50", CVAR_ARCHIVE  },
	{ &cg_chatMode, "cg_chatMode", "0", CVAR_ARCHIVE },
	{ &cg_chatPosition, "cg_chatPosition", "0", CVAR_ARCHIVE },
	{ &cg_chatBigText, "cg_chatBigText", "0", CVAR_ARCHIVE },
#endif
	{ &cg_forceModel, "cg_forceModel", "0", CVAR_ARCHIVE  },
	{ &cg_predictItems, "cg_predictItems", "1", CVAR_ARCHIVE },
#ifndef SMOKINGUNS
	{ &cg_deferPlayers, "cg_deferPlayers", "0", CVAR_ARCHIVE },
#else
	{ &cg_deferPlayers, "cg_deferPlayers", "1", CVAR_ARCHIVE },
#endif
	{ &cg_drawTeamOverlay, "cg_drawTeamOverlay", "0", CVAR_ARCHIVE },
	{ &cg_teamOverlayUserinfo, "teamoverlay", "0", CVAR_ROM | CVAR_USERINFO },
	{ &cg_stats, "cg_stats", "0", 0 },
	{ &cg_drawFriend, "cg_drawFriend", "1", CVAR_ARCHIVE },
#ifndef SMOKINGUNS
	{ &cg_teamChatsOnly, "cg_teamChatsOnly", "0", CVAR_ARCHIVE },
#ifdef MISSIONPACK
	{ &cg_noVoiceChats, "cg_noVoiceChats", "0", CVAR_ARCHIVE },
	{ &cg_noVoiceText, "cg_noVoiceText", "0", CVAR_ARCHIVE },
#endif
#endif
	// the following variables are created in other parts of the system,
	// but we also reference them here
	{ &cg_buildScript, "com_buildScript", "0", 0 },	// force loading of all possible data amd error on failures
	{ &cg_paused, "cl_paused", "0", CVAR_ROM },
#ifdef SMOKINGUNS
	{ &cg_menu, "cl_menu", "0", CVAR_ROM },
#endif
	{ &cg_blood, "com_blood", "1", CVAR_ARCHIVE },
	{ &cg_synchronousClients, "g_synchronousClients", "0", CVAR_SYSTEMINFO },
	{ &cg_redTeamName, "g_redteam", DEFAULT_REDTEAM_NAME, CVAR_ARCHIVE | CVAR_SERVERINFO | CVAR_USERINFO },
	{ &cg_blueTeamName, "g_blueteam", DEFAULT_BLUETEAM_NAME, CVAR_ARCHIVE | CVAR_SERVERINFO | CVAR_USERINFO },
	{ &cg_currentSelectedPlayer, "cg_currentSelectedPlayer", "0", CVAR_ARCHIVE},
	{ &cg_currentSelectedPlayerName, "cg_currentSelectedPlayerName", "", CVAR_ARCHIVE},
	{ &cg_singlePlayer, "ui_singlePlayerActive", "0", CVAR_USERINFO},
	{ &cg_enableDust, "g_enableDust", "0", CVAR_SERVERINFO},
	{ &cg_enableBreath, "g_enableBreath", "0", CVAR_SERVERINFO},
	{ &cg_singlePlayerActive, "ui_singlePlayerActive", "0", CVAR_USERINFO},
	{ &cg_recordSPDemo, "ui_recordSPDemo", "0", CVAR_ARCHIVE},
	{ &cg_recordSPDemoName, "ui_recordSPDemoName", "", CVAR_ARCHIVE},
#ifndef SMOKINGUNS
	{ &cg_obeliskRespawnDelay, "g_obeliskRespawnDelay", "10", CVAR_SERVERINFO},
	{ &cg_hudFiles, "cg_hudFiles", "ui/hud.txt", CVAR_ARCHIVE},
#else
	//added to SG <-
	{ &cg_farclip, "cg_farclip", "1", CVAR_ARCHIVE},
	{ &cg_farclipValue, "cg_farclipValue", "1", CVAR_ARCHIVE},
	{ &cg_farclipZoomValue, "cg_farclipZoomValue", "3", CVAR_ARCHIVE},

	{ &cg_mapLOD, "cg_mapLOD", "2", CVAR_ARCHIVE},

	{ &cg_newShotgunPattern, "cg_newShotgunPattern", "0", CVAR_ROM},
	{ &cg_maxMoney, "cg_maxMoney", "0", CVAR_ROM},
//	{ &cg_availablePlaylist, "cg_availablePlaylist", "0", CVAR_ROM},
	{ &cg_roundNoMoveTime, "cg_roundNoMoveTime", "3", CVAR_ROM},
#endif
	{ &cg_cameraOrbit, "cg_cameraOrbit", "0", CVAR_CHEAT},
	{ &cg_cameraOrbitDelay, "cg_cameraOrbitDelay", "50", CVAR_ARCHIVE},
	{ &cg_timescaleFadeEnd, "cg_timescaleFadeEnd", "1", 0},
	{ &cg_timescaleFadeSpeed, "cg_timescaleFadeSpeed", "0", 0},
	{ &cg_timescale, "timescale", "1", 0},
#ifndef SMOKINGUNS
	{ &cg_scorePlum, "cg_scorePlums", "1", CVAR_USERINFO | CVAR_ARCHIVE},
	{ &cg_smoothClients, "cg_smoothClients", "0", CVAR_USERINFO | CVAR_ARCHIVE},
#endif
	{ &cg_cameraMode, "com_cameraMode", "0", CVAR_CHEAT},

	{ &pmove_fixed, "pmove_fixed", "0", CVAR_SYSTEMINFO},
	{ &pmove_msec, "pmove_msec", "8", CVAR_SYSTEMINFO},
	{ &cg_noTaunt, "cg_noTaunt", "0", CVAR_ARCHIVE},
#ifndef SMOKINGUNS
	{ &cg_noProjectileTrail, "cg_noProjectileTrail", "0", CVAR_ARCHIVE},
#endif
	{ &cg_smallFont, "ui_smallFont", "0.25", CVAR_ARCHIVE},
	{ &cg_bigFont, "ui_bigFont", "0.4", CVAR_ARCHIVE},
#ifdef SMOKINGUNS
//unlagged - client options
	{ &cg_delag, "cg_delag", "1", CVAR_ARCHIVE | CVAR_USERINFO },
	{ &cg_debugDelag, "cg_debugDelag", "0", CVAR_USERINFO | CVAR_CHEAT },
	{ &cg_drawBBox, "cg_drawBBox", "0", CVAR_CHEAT },
	{ &cg_cmdTimeNudge, "cg_cmdTimeNudge", "0", CVAR_ARCHIVE | CVAR_USERINFO },
	// this will be automagically copied from the server
	{ &sv_fps, "sv_fps", "20", 0 },
	{ &cg_projectileNudge, "cg_projectileNudge", "0", CVAR_ARCHIVE },
	{ &cg_optimizePrediction, "cg_optimizePrediction", "1", CVAR_ARCHIVE },
	{ &cl_timeNudge, "cl_timeNudge", "0", CVAR_ARCHIVE },
	{ &cg_latentSnaps, "cg_latentSnaps", "0", CVAR_USERINFO | CVAR_CHEAT },
	{ &cg_latentCmds, "cg_latentCmds", "0", CVAR_USERINFO | CVAR_CHEAT },
	{ &cg_plOut, "cg_plOut", "0", CVAR_USERINFO | CVAR_CHEAT },
//unlagged - client options
	//SG Cvars
//	{ &cg_sg_lefthanded, "sg_lefthanded", "0", CVAR_ARCHIVE},

	{ &cg_buydebug, "cg_buydebug", "0", CVAR_ARCHIVE },
	// used by the scoreboard, in score.menu
	{ &cg_serverfraglimit, "cg_serverfraglimit", "0", CVAR_ROM },
	{ &cg_serverduellimit, "cg_serverduellimit", "0", CVAR_ROM },

	//to get amount of players in ui
	//{ &cg_teamredcount, "cg_teamredcount", "0", CVAR_ROM },
	//{ &cg_teambluecount, "cg_teambluecount", "0", CVAR_ROM },

//	{ &cg_roundmusic, "cg_roundmusic", "1", CVAR_ARCHIVE },
	{ &cg_impactparticles, "cg_impactparticles", "2", CVAR_ARCHIVE },
	{ &cg_gunsmoke, "cg_gunsmoke", "1", CVAR_ARCHIVE },
	{ &cg_addguns, "cg_addguns", "1", CVAR_ARCHIVE },
	{ &cg_killmsg, "cg_killmsg", "0", CVAR_ARCHIVE },
	{ &cg_playownflysound, "cg_flysound", "0", CVAR_ARCHIVE },
	{ &cg_showescape, "cg_showescape", "1", CVAR_ARCHIVE },
	{ &cg_debug, "cg_debug", "0", CVAR_ARCHIVE },
	{ &cg_glowflares, "cg_glowflares", "1", CVAR_ARCHIVE },
	{ &cg_boostfps, "cg_boostfps", "1", CVAR_ARCHIVE },
	{ &cg_drawdebug, "cg_drawdebug", "0", CVAR_CHEAT },
	{ &cg_drawspeed, "cg_drawspeed", "0", CVAR_ARCHIVE },
	{ &cg_hitmsg, "cg_hitmsg", "1", CVAR_ARCHIVE },
	{ &cg_hitfarmsg, "cg_hitfarmsg", "1", CVAR_ARCHIVE },
	{ &cg_ownhitmsg, "cg_ownhitmsg", "1", CVAR_ARCHIVE },

	//talk sound
	{ &cg_talksound, "cg_talksound", "1", CVAR_ARCHIVE },

	{ &hit_model, "hit_model", "0", CVAR_ARCHIVE|CVAR_CHEAT },
	{ &cg_warmupmessage, "cg_warmupmessage", "1", CVAR_ARCHIVE|CVAR_CHEAT },

	//important key bindings
	{ &cg_button_attack[0], "cl_button_attack1", "0", CVAR_ROM },
	{ &cg_button_attack[1], "cl_button_attack2", "0", CVAR_ROM },

	{ &cg_button_altattack[0], "cl_button_altattack1", "0", CVAR_ROM },
	{ &cg_button_altattack[1], "cl_button_altattack2", "0", CVAR_ROM },

	/*{ &cg_button_forward[0], "cl_button_forward1", "0", CVAR_ROM },
	{ &cg_button_forward[1], "cl_button_forward2", "0", CVAR_ROM },

	{ &cg_button_back[0], "cl_button_back1", "0", CVAR_ROM },
	{ &cg_button_back[1], "cl_button_back2", "0", CVAR_ROM },

	{ &cg_button_moveleft[0], "cl_button_moveleft1", "0", CVAR_ROM },
	{ &cg_button_moveleft[1], "cl_button_moveleft2", "0", CVAR_ROM },

	{ &cg_button_moveright[0], "cl_button_moveright1", "0", CVAR_ROM },
	{ &cg_button_moveright[1], "cl_button_moveright2", "0", CVAR_ROM },

	{ &cg_button_turnleft[0], "cl_button_turnleft1", "0", CVAR_ROM },
	{ &cg_button_turnleft[1], "cl_button_turnleft2", "0", CVAR_ROM },

	{ &cg_button_turnright[0], "cl_button_turnright1", "0", CVAR_ROM },
	{ &cg_button_turnright[1], "cl_button_turnright2", "0", CVAR_ROM },*/
#endif

#ifndef SMOKINGUNS
	{ &cg_oldRail, "cg_oldRail", "1", CVAR_ARCHIVE},
	{ &cg_oldRocket, "cg_oldRocket", "1", CVAR_ARCHIVE},
	{ &cg_oldPlasma, "cg_oldPlasma", "1", CVAR_ARCHIVE},
	{ &cg_trueLightning, "cg_trueLightning", "0.0", CVAR_ARCHIVE}
#endif
//	{ &cg_pmove_fixed, "cg_pmove_fixed", "0", CVAR_USERINFO | CVAR_ARCHIVE }
};

static int  cvarTableSize = ARRAY_LEN( cvarTable );

/*
=================
CG_RegisterCvars
=================
*/
void CG_RegisterCvars( void ) {
	int			i;
	cvarTable_t	*cv;
	char		var[MAX_TOKEN_CHARS];

	for ( i = 0, cv = cvarTable ; i < cvarTableSize ; i++, cv++ ) {
		trap_Cvar_Register( cv->vmCvar, cv->cvarName,
			cv->defaultString, cv->cvarFlags );
	}

	// see if we are also running the server on this machine
	trap_Cvar_VariableStringBuffer( "sv_running", var, sizeof( var ) );
	cgs.localServer = atoi( var );

	// see if the Smokin' Guns standalone engine is used.
	// If it is set "maliciously" from command line ... what the hell ...
	// The client will get a "Bad cgame system trap" message, when used in a VM
#ifdef SMOKINGUNS
	trap_Cvar_VariableStringBuffer( "sa_engine_inuse", var, sizeof( var ) );
	sa_engine_inuse = atoi( var );
#endif

	forceModelModificationCount = cg_forceModel.modificationCount;

	trap_Cvar_Register(NULL, "model", DEFAULT_MODEL, CVAR_USERINFO | CVAR_ARCHIVE );
	trap_Cvar_Register(NULL, "headmodel", DEFAULT_MODEL, CVAR_USERINFO | CVAR_ARCHIVE );
	trap_Cvar_Register(NULL, "team_model", DEFAULT_TEAM_MODEL, CVAR_USERINFO | CVAR_ARCHIVE );
	trap_Cvar_Register(NULL, "team_headmodel", DEFAULT_TEAM_HEAD, CVAR_USERINFO | CVAR_ARCHIVE );
}

/*
===================
CG_ForceModelChange
===================
*/
static void CG_ForceModelChange( void ) {
	int		i;

	for (i=0 ; i<MAX_CLIENTS ; i++) {
		const char		*clientInfo;

		clientInfo = CG_ConfigString( CS_PLAYERS+i );
		if ( !clientInfo[0] ) {
			continue;
		}
		CG_NewClientInfo( i );
	}
}

/*
=================
CG_UpdateCvars
=================
*/
void CG_UpdateCvars( void ) {
	int			i;
	cvarTable_t	*cv;

	for ( i = 0, cv = cvarTable ; i < cvarTableSize ; i++, cv++ ) {
#ifdef SMOKINGUNS
//unlagged - client options
		// clamp the value between 0 and 999
		// negative values would suck - people could conceivably shoot other
		// players *long* after they had left the area, on purpose
		if ( cv->vmCvar == &cg_cmdTimeNudge ) {
			CG_Cvar_ClampInt( cv->cvarName, cv->vmCvar, 0, 999 );
		}
		// cl_timenudge less than -50 or greater than 50 doesn't actually
		// do anything more than -50 or 50 (actually the numbers are probably
		// closer to -30 and 30, but 50 is nice and round-ish)
		// might as well not feed the myth, eh?
		else if ( cv->vmCvar == &cl_timeNudge ) {
			CG_Cvar_ClampInt( cv->cvarName, cv->vmCvar, -50, 50 );
		}
		// don't let this go too high - no point
		else if ( cv->vmCvar == &cg_latentSnaps ) {
			CG_Cvar_ClampInt( cv->cvarName, cv->vmCvar, 0, 10 );
		}
		// don't let this get too large
		else if ( cv->vmCvar == &cg_latentCmds ) {
			CG_Cvar_ClampInt( cv->cvarName, cv->vmCvar, 0, MAX_LATENT_CMDS - 1 );
		}
		// no more than 100% packet loss
		else if ( cv->vmCvar == &cg_plOut ) {
			CG_Cvar_ClampInt( cv->cvarName, cv->vmCvar, 0, 100 );
		}
//unlagged - client options
#endif
		trap_Cvar_Update( cv->vmCvar );
	}

#ifdef SMOKINGUNS
	if(!cg_cheats)
		cg_fov.integer = 90;
#endif

	// check for modications here

	// If team overlay is on, ask for updates from the server.  If it's off,
	// let the server know so we don't receive it
	if ( drawTeamOverlayModificationCount != cg_drawTeamOverlay.modificationCount ) {
		drawTeamOverlayModificationCount = cg_drawTeamOverlay.modificationCount;

#ifndef SMOKINGUNS
		if ( cg_drawTeamOverlay.integer > 0 ) {
			trap_Cvar_Set( "teamoverlay", "1" );
		} else {
			trap_Cvar_Set( "teamoverlay", "0" );
		}
#else
		// FIXME E3 HACK
		trap_Cvar_Set( "teamoverlay", "1" );
#endif
	}

	// if force model changed
	if ( forceModelModificationCount != cg_forceModel.modificationCount ) {
		forceModelModificationCount = cg_forceModel.modificationCount;
		CG_ForceModelChange();
	}
}

int CG_CrosshairPlayer( void ) {
	if ( cg.time > ( cg.crosshairClientTime + 1000 ) ) {
		return -1;
	}
	return cg.crosshairClientNum;
}

int CG_LastAttacker( void ) {
	if ( !cg.attackerTime ) {
		return -1;
	}
	return cg.snap->ps.persistant[PERS_ATTACKER];
}

void QDECL CG_Printf( const char *msg, ... ) {
	va_list		argptr;
	char		text[1024];

	va_start (argptr, msg);
	Q_vsnprintf (text, sizeof(text), msg, argptr);
	va_end (argptr);

	trap_Print( text );
}

void QDECL CG_Error( const char *msg, ... ) {
	va_list		argptr;
	char		text[1024];

	va_start (argptr, msg);
	Q_vsnprintf (text, sizeof(text), msg, argptr);
	va_end (argptr);

	trap_Error( text );
}

void QDECL Com_Error( int level, const char *error, ... ) {
	va_list		argptr;
	char		text[1024];

	va_start (argptr, error);
	Q_vsnprintf (text, sizeof(text), error, argptr);
	va_end (argptr);

	trap_Error( text );
}

void QDECL Com_Printf( const char *msg, ... ) {
	va_list		argptr;
	char		text[1024];

	va_start (argptr, msg);
	Q_vsnprintf (text, sizeof(text), msg, argptr);
	va_end (argptr);

	trap_Print( text );
}

/*
================
CG_Argv
================
*/
const char *CG_Argv( int arg ) {
	static char	buffer[MAX_STRING_CHARS];

	trap_Argv( arg, buffer, sizeof( buffer ) );

	return buffer;
}


//========================================================================

/*
=================
CG_RegisterItemSounds

The server says this item is used on this level
=================
*/
static void CG_RegisterItemSounds( int itemNum ) {
	gitem_t			*item;
	char			data[MAX_QPATH];
	char			*s, *start;
	int				len;

	item = &bg_itemlist[ itemNum ];

	if( item->pickup_sound ) {
		trap_S_RegisterSound( item->pickup_sound, qfalse );
	}

	// parse the space seperated precache string for other media
	s = item->sounds;
	if (!s || !s[0])
		return;

	while (*s) {
		start = s;
		while (*s && *s != ' ') {
			s++;
		}

		len = s-start;
		if (len >= MAX_QPATH || len < 5) {
			CG_Error( "PrecacheItem: %s has bad precache string",
				item->classname);
			return;
		}
		memcpy (data, start, len);
		data[len] = 0;
		if ( *s ) {
			s++;
		}

		if ( !strcmp(data+len-3, "wav" )) {
			trap_S_RegisterSound( data, qfalse );
		}
	}
}


/*
=================
CG_RegisterSounds

called during a precache command
=================
*/
static void CG_RegisterSounds( void ) {
	int		i;
	char	items[MAX_ITEMS+1];
	char	name[MAX_QPATH];
	const char	*soundName;

#ifndef SMOKINGUNS
	// voice commands
#ifdef MISSIONPACK
	CG_LoadVoiceChats();
#endif

	cgs.media.oneMinuteSound = trap_S_RegisterSound( "sound/feedback/1_minute.wav", qtrue );
	cgs.media.fiveMinuteSound = trap_S_RegisterSound( "sound/feedback/5_minute.wav", qtrue );
	cgs.media.suddenDeathSound = trap_S_RegisterSound( "sound/feedback/sudden_death.wav", qtrue );
	cgs.media.oneFragSound = trap_S_RegisterSound( "sound/feedback/1_frag.wav", qtrue );
	cgs.media.twoFragSound = trap_S_RegisterSound( "sound/feedback/2_frags.wav", qtrue );
	cgs.media.threeFragSound = trap_S_RegisterSound( "sound/feedback/3_frags.wav", qtrue );
	cgs.media.count3Sound = trap_S_RegisterSound( "sound/feedback/three.wav", qtrue );
	cgs.media.count2Sound = trap_S_RegisterSound( "sound/feedback/two.wav", qtrue );
	cgs.media.count1Sound = trap_S_RegisterSound( "sound/feedback/one.wav", qtrue );
	cgs.media.countFightSound = trap_S_RegisterSound( "sound/feedback/fight.wav", qtrue );
	cgs.media.countPrepareSound = trap_S_RegisterSound( "sound/feedback/prepare.wav", qtrue );
#ifdef MISSIONPACK
	cgs.media.countPrepareTeamSound = trap_S_RegisterSound( "sound/feedback/prepare_team.wav", qtrue );
#endif

	if ( cgs.gametype >= GT_TEAM || cg_buildScript.integer ) {

		cgs.media.captureAwardSound = trap_S_RegisterSound( "sound/teamplay/flagcapture_yourteam.wav", qtrue );
		cgs.media.redLeadsSound = trap_S_RegisterSound( "sound/feedback/redleads.wav", qtrue );
		cgs.media.blueLeadsSound = trap_S_RegisterSound( "sound/feedback/blueleads.wav", qtrue );
		cgs.media.teamsTiedSound = trap_S_RegisterSound( "sound/feedback/teamstied.wav", qtrue );
		cgs.media.hitTeamSound = trap_S_RegisterSound( "sound/feedback/hit_teammate.wav", qtrue );

		cgs.media.redScoredSound = trap_S_RegisterSound( "sound/teamplay/voc_red_scores.wav", qtrue );
		cgs.media.blueScoredSound = trap_S_RegisterSound( "sound/teamplay/voc_blue_scores.wav", qtrue );

		cgs.media.captureYourTeamSound = trap_S_RegisterSound( "sound/teamplay/flagcapture_yourteam.wav", qtrue );
		cgs.media.captureOpponentSound = trap_S_RegisterSound( "sound/teamplay/flagcapture_opponent.wav", qtrue );

		cgs.media.returnYourTeamSound = trap_S_RegisterSound( "sound/teamplay/flagreturn_yourteam.wav", qtrue );
		cgs.media.returnOpponentSound = trap_S_RegisterSound( "sound/teamplay/flagreturn_opponent.wav", qtrue );

		cgs.media.takenYourTeamSound = trap_S_RegisterSound( "sound/teamplay/flagtaken_yourteam.wav", qtrue );
		cgs.media.takenOpponentSound = trap_S_RegisterSound( "sound/teamplay/flagtaken_opponent.wav", qtrue );

		if ( cgs.gametype == GT_CTF || cg_buildScript.integer ) {
			cgs.media.redFlagReturnedSound = trap_S_RegisterSound( "sound/teamplay/voc_red_returned.wav", qtrue );
			cgs.media.blueFlagReturnedSound = trap_S_RegisterSound( "sound/teamplay/voc_blue_returned.wav", qtrue );
			cgs.media.enemyTookYourFlagSound = trap_S_RegisterSound( "sound/teamplay/voc_enemy_flag.wav", qtrue );
			cgs.media.yourTeamTookEnemyFlagSound = trap_S_RegisterSound( "sound/teamplay/voc_team_flag.wav", qtrue );
		}

#ifdef MISSIONPACK
		if ( cgs.gametype == GT_1FCTF || cg_buildScript.integer ) {
			// FIXME: get a replacement for this sound ?
			cgs.media.neutralFlagReturnedSound = trap_S_RegisterSound( "sound/teamplay/flagreturn_opponent.wav", qtrue );
			cgs.media.yourTeamTookTheFlagSound = trap_S_RegisterSound( "sound/teamplay/voc_team_1flag.wav", qtrue );
			cgs.media.enemyTookTheFlagSound = trap_S_RegisterSound( "sound/teamplay/voc_enemy_1flag.wav", qtrue );
		}

		if ( cgs.gametype == GT_1FCTF || cgs.gametype == GT_CTF || cg_buildScript.integer ) {
			cgs.media.youHaveFlagSound = trap_S_RegisterSound( "sound/teamplay/voc_you_flag.wav", qtrue );
			cgs.media.holyShitSound = trap_S_RegisterSound("sound/feedback/voc_holyshit.wav", qtrue);
		}

		if ( cgs.gametype == GT_OBELISK || cg_buildScript.integer ) {
			cgs.media.yourBaseIsUnderAttackSound = trap_S_RegisterSound( "sound/teamplay/voc_base_attack.wav", qtrue );
		}
#else
		cgs.media.youHaveFlagSound = trap_S_RegisterSound( "sound/teamplay/voc_you_flag.wav", qtrue );
		cgs.media.holyShitSound = trap_S_RegisterSound("sound/feedback/voc_holyshit.wav", qtrue);
#endif
	}
#else
	CG_LoadingStage(1);
#endif

	cgs.media.tracerSound = trap_S_RegisterSound( "sound/weapons/machinegun/buletby1.wav", qfalse );
	cgs.media.selectSound = trap_S_RegisterSound( "sound/weapons/change.wav", qfalse );
#ifndef SMOKINGUNS
	cgs.media.wearOffSound = trap_S_RegisterSound( "sound/items/wearoff.wav", qfalse );
	cgs.media.useNothingSound = trap_S_RegisterSound( "sound/items/use_nothing.wav", qfalse );
	cgs.media.gibSound = trap_S_RegisterSound( "sound/player/gibsplt1.wav", qfalse );
	cgs.media.gibBounce1Sound = trap_S_RegisterSound( "sound/player/gibimp1.wav", qfalse );
	cgs.media.gibBounce2Sound = trap_S_RegisterSound( "sound/player/gibimp2.wav", qfalse );
	cgs.media.gibBounce3Sound = trap_S_RegisterSound( "sound/player/gibimp3.wav", qfalse );
#else
	cgs.media.snd_pistol_raise = trap_S_RegisterSound( "sound/weapons/changeto_pistol.wav", qfalse);
	cgs.media.snd_winch66_raise = trap_S_RegisterSound( "sound/weapons/changeto_winch66.wav", qfalse);
	cgs.media.snd_lightn_raise = trap_S_RegisterSound( "sound/weapons/changeto_lightning.wav", qfalse);
	cgs.media.snd_sharps_raise = trap_S_RegisterSound( "sound/weapons/changeto_sharps.wav", qfalse);
	cgs.media.snd_shotgun_raise = trap_S_RegisterSound( "sound/weapons/changeto_shotgun.wav", qfalse);
	cgs.media.snd_winch97_raise = trap_S_RegisterSound( "sound/weapons/changeto_winch97.wav", qfalse);
	CG_LoadingStage(1);
#endif

#ifndef SMOKINGUNS
#ifdef MISSIONPACK
	cgs.media.useInvulnerabilitySound = trap_S_RegisterSound( "sound/items/invul_activate.wav", qfalse );
	cgs.media.invulnerabilityImpactSound1 = trap_S_RegisterSound( "sound/items/invul_impact_01.wav", qfalse );
	cgs.media.invulnerabilityImpactSound2 = trap_S_RegisterSound( "sound/items/invul_impact_02.wav", qfalse );
	cgs.media.invulnerabilityImpactSound3 = trap_S_RegisterSound( "sound/items/invul_impact_03.wav", qfalse );
	cgs.media.invulnerabilityJuicedSound = trap_S_RegisterSound( "sound/items/invul_juiced.wav", qfalse );
	cgs.media.obeliskHitSound1 = trap_S_RegisterSound( "sound/items/obelisk_hit_01.wav", qfalse );
	cgs.media.obeliskHitSound2 = trap_S_RegisterSound( "sound/items/obelisk_hit_02.wav", qfalse );
	cgs.media.obeliskHitSound3 = trap_S_RegisterSound( "sound/items/obelisk_hit_03.wav", qfalse );
	cgs.media.obeliskRespawnSound = trap_S_RegisterSound( "sound/items/obelisk_respawn.wav", qfalse );

	cgs.media.ammoregenSound = trap_S_RegisterSound("sound/items/cl_ammoregen.wav", qfalse);
	cgs.media.doublerSound = trap_S_RegisterSound("sound/items/cl_doubler.wav", qfalse);
	cgs.media.guardSound = trap_S_RegisterSound("sound/items/cl_guard.wav", qfalse);
	cgs.media.scoutSound = trap_S_RegisterSound("sound/items/cl_scout.wav", qfalse);
#endif

	cgs.media.teleInSound = trap_S_RegisterSound( "sound/world/telein.wav", qfalse );
	cgs.media.teleOutSound = trap_S_RegisterSound( "sound/world/teleout.wav", qfalse );
	cgs.media.respawnSound = trap_S_RegisterSound( "sound/items/respawn1.wav", qfalse );
#endif

	cgs.media.noAmmoSound = trap_S_RegisterSound( "sound/weapons/noammo.wav", qfalse );

	cgs.media.talkSound = trap_S_RegisterSound( "sound/player/talk.wav", qfalse );
#ifdef SMOKINGUNS
	cgs.media.talkSound1 = trap_S_RegisterSound( "sound/player/talk1.wav", qfalse );
	cgs.media.talkSound2 = trap_S_RegisterSound( "sound/player/talk2.wav", qfalse );
	cgs.media.talkSound3 = trap_S_RegisterSound( "sound/player/talk3.wav", qfalse );
	CG_LoadingStage(1);
#endif
	cgs.media.landSound = trap_S_RegisterSound( "sound/player/land1.wav", qfalse);

#ifndef SMOKINGUNS
	cgs.media.hitSound = trap_S_RegisterSound( "sound/feedback/hit.wav", qfalse );
#ifdef MISSIONPACK
	cgs.media.hitSoundHighArmor = trap_S_RegisterSound( "sound/feedback/hithi.wav", qfalse );
	cgs.media.hitSoundLowArmor = trap_S_RegisterSound( "sound/feedback/hitlo.wav", qfalse );
#endif

	cgs.media.impressiveSound = trap_S_RegisterSound( "sound/feedback/impressive.wav", qtrue );
	cgs.media.excellentSound = trap_S_RegisterSound( "sound/feedback/excellent.wav", qtrue );
	cgs.media.deniedSound = trap_S_RegisterSound( "sound/feedback/denied.wav", qtrue );
	cgs.media.humiliationSound = trap_S_RegisterSound( "sound/feedback/humiliation.wav", qtrue );
	cgs.media.assistSound = trap_S_RegisterSound( "sound/feedback/assist.wav", qtrue );
	cgs.media.defendSound = trap_S_RegisterSound( "sound/feedback/defense.wav", qtrue );
#ifdef MISSIONPACK
	cgs.media.firstImpressiveSound = trap_S_RegisterSound( "sound/feedback/first_impressive.wav", qtrue );
	cgs.media.firstExcellentSound = trap_S_RegisterSound( "sound/feedback/first_excellent.wav", qtrue );
	cgs.media.firstHumiliationSound = trap_S_RegisterSound( "sound/feedback/first_gauntlet.wav", qtrue );
#endif

	cgs.media.takenLeadSound = trap_S_RegisterSound( "sound/feedback/takenlead.wav", qtrue);
	cgs.media.tiedLeadSound = trap_S_RegisterSound( "sound/feedback/tiedlead.wav", qtrue);
	cgs.media.lostLeadSound = trap_S_RegisterSound( "sound/feedback/lostlead.wav", qtrue);

#ifdef MISSIONPACK
	cgs.media.voteNow = trap_S_RegisterSound( "sound/feedback/vote_now.wav", qtrue);
	cgs.media.votePassed = trap_S_RegisterSound( "sound/feedback/vote_passed.wav", qtrue);
	cgs.media.voteFailed = trap_S_RegisterSound( "sound/feedback/vote_failed.wav", qtrue);
#endif
#endif

	cgs.media.watrInSound = trap_S_RegisterSound( "sound/player/watr_in.wav", qfalse);
	cgs.media.watrOutSound = trap_S_RegisterSound( "sound/player/watr_out.wav", qfalse);
	cgs.media.watrUnSound = trap_S_RegisterSound( "sound/player/watr_un.wav", qfalse);

#ifndef SMOKINGUNS
	cgs.media.jumpPadSound = trap_S_RegisterSound ("sound/world/jumppad.wav", qfalse );
#else
	// death sounds
	cgs.media.snd_death[0] = trap_S_RegisterSound ( "sound/player/death_default.wav", qfalse);
	cgs.media.snd_death[1] = trap_S_RegisterSound ( "sound/player/death_head.wav", qfalse);
	cgs.media.snd_death[2] = cgs.media.snd_death[3] =
		trap_S_RegisterSound ( "sound/player/death_arm.wav", qfalse);
	cgs.media.snd_death[4] = trap_S_RegisterSound ( "sound/player/death_chest.wav", qfalse);
	cgs.media.snd_death[5] = trap_S_RegisterSound ( "sound/player/death_stomach.wav", qfalse);
	cgs.media.snd_death[6] = cgs.media.snd_death[7] =
		trap_S_RegisterSound ( "sound/player/death_leg.wav", qfalse);
	cgs.media.snd_death[8] = cgs.media.snd_death[9] =
		trap_S_RegisterSound ( "sound/player/death_falloff.wav", qfalse);
	cgs.media.snd_death[10] = trap_S_RegisterSound ( "sound/player/death_land.wav", qfalse);
#endif

	for (i=0 ; i<4 ; i++) {
#ifndef SMOKINGUNS
		Com_sprintf (name, sizeof(name), "sound/player/footsteps/step%i.wav", i+1);
#else
		Com_sprintf (name, sizeof(name), "sound/footsteps/step%i.wav", i+1);
#endif
		cgs.media.footsteps[FOOTSTEP_NORMAL][i] = trap_S_RegisterSound (name, qfalse);

#ifndef SMOKINGUNS
		Com_sprintf (name, sizeof(name), "sound/player/footsteps/boot%i.wav", i+1);
		cgs.media.footsteps[FOOTSTEP_BOOT][i] = trap_S_RegisterSound (name, qfalse);

		Com_sprintf (name, sizeof(name), "sound/player/footsteps/flesh%i.wav", i+1);
		cgs.media.footsteps[FOOTSTEP_FLESH][i] = trap_S_RegisterSound (name, qfalse);

		Com_sprintf (name, sizeof(name), "sound/player/footsteps/mech%i.wav", i+1);
		cgs.media.footsteps[FOOTSTEP_MECH][i] = trap_S_RegisterSound (name, qfalse);

		Com_sprintf (name, sizeof(name), "sound/player/footsteps/energy%i.wav", i+1);
		cgs.media.footsteps[FOOTSTEP_ENERGY][i] = trap_S_RegisterSound (name, qfalse);

		Com_sprintf (name, sizeof(name), "sound/player/footsteps/splash%i.wav", i+1);
#else
		Com_sprintf (name, sizeof(name), "sound/footsteps/splash%i.wav", i+1);
#endif
		cgs.media.footsteps[FOOTSTEP_SPLASH][i] = trap_S_RegisterSound (name, qfalse);

#ifndef SMOKINGUNS
		Com_sprintf (name, sizeof(name), "sound/player/footsteps/clank%i.wav", i+1);
#else
		Com_sprintf (name, sizeof(name), "sound/footsteps/clank%i.wav", i+1);
#endif
		cgs.media.footsteps[FOOTSTEP_METAL][i] = trap_S_RegisterSound (name, qfalse);

#ifdef SMOKINGUNS
		Com_sprintf (name, sizeof(name), "sound/footsteps/snow%i.wav", i+1);
		cgs.media.footsteps[FOOTSTEP_SNOW][i] = trap_S_RegisterSound (name, qfalse);

		Com_sprintf (name, sizeof(name), "sound/footsteps/sand%i.wav", i+1);
		cgs.media.footsteps[FOOTSTEP_SAND][i] = trap_S_RegisterSound (name, qfalse);

		Com_sprintf (name, sizeof(name), "sound/footsteps/grass%i.wav", i+1);
		cgs.media.footsteps[FOOTSTEP_GRASS][i] = trap_S_RegisterSound (name, qfalse);

		Com_sprintf (name, sizeof(name), "sound/footsteps/cloth%i.wav", i+1);
		cgs.media.footsteps[FOOTSTEP_CLOTH][i] = trap_S_RegisterSound (name, qfalse);
#endif
	}

#ifdef SMOKINGUNS
	CG_LoadingStage(1);
#endif

	// only register the items that the server says we need
	Q_strncpyz(items, CG_ConfigString(CS_ITEMS), sizeof(items));

	for ( i = 1 ; i < bg_numItems ; i++ ) {
//		if ( items[ i ] == '1' || cg_buildScript.integer ) {
			CG_RegisterItemSounds( i );

#ifdef SMOKINGUNS
			if(!(i%5))
				CG_LoadingStage(1);
#endif
//		}
	}

	for ( i = 1 ; i < MAX_SOUNDS ; i++ ) {
		soundName = CG_ConfigString( CS_SOUNDS+i );
		if ( !soundName[0] ) {
			break;
		}
		if ( soundName[0] == '*' ) {
			continue;	// custom sound
		}
#ifdef SMOKINGUNS
		if(!(i%5))
			CG_LoadingStage(1);
#endif
		cgs.gameSounds[i] = trap_S_RegisterSound( soundName, qfalse );
	}

	// FIXME: only needed with item
#ifndef SMOKINGUNS
	cgs.media.flightSound = trap_S_RegisterSound( "sound/items/flight.wav", qfalse );
	cgs.media.medkitSound = trap_S_RegisterSound ("sound/items/use_medkit.wav", qfalse);
	cgs.media.quadSound = trap_S_RegisterSound("sound/items/damage3.wav", qfalse);
	cgs.media.sfx_ric1 = trap_S_RegisterSound ("sound/weapons/machinegun/ric1.wav", qfalse);
	cgs.media.sfx_ric2 = trap_S_RegisterSound ("sound/weapons/machinegun/ric2.wav", qfalse);
	cgs.media.sfx_ric3 = trap_S_RegisterSound ("sound/weapons/machinegun/ric3.wav", qfalse);
	//cgs.media.sfx_railg = trap_S_RegisterSound ("sound/weapons/railgun/railgf1a.wav", qfalse);
	cgs.media.sfx_rockexp = trap_S_RegisterSound ("sound/weapons/rocket/rocklx1a.wav", qfalse);
	cgs.media.sfx_plasmaexp = trap_S_RegisterSound ("sound/weapons/plasma/plasmx1a.wav", qfalse);
#else
	for (i=0 ; i<6 ; i++) {
		Com_sprintf (name, sizeof(name), "sound/weapons/ric%i.wav", i+1);
		cgs.media.sfx_ric[i] = trap_S_RegisterSound (name, qfalse);
	}
	cgs.media.sfx_rockexp = trap_S_RegisterSound ("sound/weapons/exp_1.wav", qfalse);
#endif
#ifndef SMOKINGUNS
#ifdef MISSIONPACK
	cgs.media.sfx_proxexp = trap_S_RegisterSound( "sound/weapons/proxmine/wstbexpl.wav" , qfalse);
	cgs.media.sfx_nghit = trap_S_RegisterSound( "sound/weapons/nailgun/wnalimpd.wav" , qfalse);
	cgs.media.sfx_nghitflesh = trap_S_RegisterSound( "sound/weapons/nailgun/wnalimpl.wav" , qfalse);
	cgs.media.sfx_nghitmetal = trap_S_RegisterSound( "sound/weapons/nailgun/wnalimpm.wav", qfalse );
	cgs.media.sfx_chghit = trap_S_RegisterSound( "sound/weapons/vulcan/wvulimpd.wav", qfalse );
	cgs.media.sfx_chghitflesh = trap_S_RegisterSound( "sound/weapons/vulcan/wvulimpl.wav", qfalse );
	cgs.media.sfx_chghitmetal = trap_S_RegisterSound( "sound/weapons/vulcan/wvulimpm.wav", qfalse );
	cgs.media.weaponHoverSound = trap_S_RegisterSound( "sound/weapons/weapon_hover.wav", qfalse );
	cgs.media.kamikazeExplodeSound = trap_S_RegisterSound( "sound/items/kam_explode.wav", qfalse );
	cgs.media.kamikazeImplodeSound = trap_S_RegisterSound( "sound/items/kam_implode.wav", qfalse );
	cgs.media.kamikazeFarSound = trap_S_RegisterSound( "sound/items/kam_explode_far.wav", qfalse );
	cgs.media.winnerSound = trap_S_RegisterSound( "sound/feedback/voc_youwin.wav", qfalse );
	cgs.media.loserSound = trap_S_RegisterSound( "sound/feedback/voc_youlose.wav", qfalse );

	cgs.media.wstbimplSound = trap_S_RegisterSound("sound/weapons/proxmine/wstbimpl.wav", qfalse);
	cgs.media.wstbimpmSound = trap_S_RegisterSound("sound/weapons/proxmine/wstbimpm.wav", qfalse);
	cgs.media.wstbimpdSound = trap_S_RegisterSound("sound/weapons/proxmine/wstbimpd.wav", qfalse);
	cgs.media.wstbactvSound = trap_S_RegisterSound("sound/weapons/proxmine/wstbactv.wav", qfalse);
#endif

	cgs.media.regenSound = trap_S_RegisterSound("sound/items/regen.wav", qfalse);
	cgs.media.protectSound = trap_S_RegisterSound("sound/items/protect3.wav", qfalse);
	cgs.media.n_healthSound = trap_S_RegisterSound("sound/items/n_health.wav", qfalse );
	cgs.media.hgrenb1aSound = trap_S_RegisterSound("sound/weapons/grenade/hgrenb1a.wav", qfalse);
	cgs.media.hgrenb2aSound = trap_S_RegisterSound("sound/weapons/grenade/hgrenb2a.wav", qfalse);

#ifdef MISSIONPACK
	trap_S_RegisterSound("sound/player/james/death1.wav", qfalse );
	trap_S_RegisterSound("sound/player/james/death2.wav", qfalse );
	trap_S_RegisterSound("sound/player/james/death3.wav", qfalse );
	trap_S_RegisterSound("sound/player/james/jump1.wav", qfalse );
	trap_S_RegisterSound("sound/player/james/pain25_1.wav", qfalse );
	trap_S_RegisterSound("sound/player/james/pain75_1.wav", qfalse );
	trap_S_RegisterSound("sound/player/james/pain100_1.wav", qfalse );
	trap_S_RegisterSound("sound/player/james/falling1.wav", qfalse );
	trap_S_RegisterSound("sound/player/james/gasp.wav", qfalse );
	trap_S_RegisterSound("sound/player/james/drown.wav", qfalse );
	trap_S_RegisterSound("sound/player/james/fall1.wav", qfalse );
	trap_S_RegisterSound("sound/player/james/taunt.wav", qfalse );

	trap_S_RegisterSound("sound/player/janet/death1.wav", qfalse );
	trap_S_RegisterSound("sound/player/janet/death2.wav", qfalse );
	trap_S_RegisterSound("sound/player/janet/death3.wav", qfalse );
	trap_S_RegisterSound("sound/player/janet/jump1.wav", qfalse );
	trap_S_RegisterSound("sound/player/janet/pain25_1.wav", qfalse );
	trap_S_RegisterSound("sound/player/janet/pain75_1.wav", qfalse );
	trap_S_RegisterSound("sound/player/janet/pain100_1.wav", qfalse );
	trap_S_RegisterSound("sound/player/janet/falling1.wav", qfalse );
	trap_S_RegisterSound("sound/player/janet/gasp.wav", qfalse );
	trap_S_RegisterSound("sound/player/janet/drown.wav", qfalse );
	trap_S_RegisterSound("sound/player/janet/fall1.wav", qfalse );
	trap_S_RegisterSound("sound/player/janet/taunt.wav", qfalse );
#endif
#else
	//Spoon Sounds
	//round sounds
	/*cgs.media.roundstart = trap_S_RegisterSound( "sound/music/roundstart.wav", qfalse);
	cgs.media.roundwin = trap_S_RegisterSound( "sound/music/roundwin.wav", qfalse );
	cgs.media.roundlost = trap_S_RegisterSound( "sound/music/roundlost.wav", qfalse );*/

	cgs.media.knifehit = trap_S_RegisterSound( "sound/weapons/knife_hitflesh.wav", qfalse );
	cgs.media.knifehit2 = trap_S_RegisterSound ("sound/weapons/knife_hitwall.wav", qfalse);
	cgs.media.dynamiteburn = trap_S_RegisterSound("sound/weapons/dyn_burn.wav", qfalse);

	CG_LoadingStage(1);

	cgs.media.du_medal_snd = trap_S_RegisterSound( "sound/misc/du_medal.wav", qfalse);
	cgs.media.du_won_snd = trap_S_RegisterSound( "sound/misc/du_won.wav", qfalse);

	cgs.media.boilerhit = trap_S_RegisterSound("sound/damage/boiler.wav", qfalse);
	cgs.media.dynamitezundan = trap_S_RegisterSound("sound/weapons/dyn_burn2.wav", qfalse);
	cgs.media.sfxglassgib[0] = trap_S_RegisterSound("sound/gibs/glass1.wav", qfalse);
	cgs.media.sfxglassgib[1] = trap_S_RegisterSound("sound/gibs/glass2.wav", qfalse);
	cgs.media.sfxstonegib = trap_S_RegisterSound("sound/gibs/stone1.wav", qfalse);

	cgs.media.underwater = trap_S_RegisterSound("sound/player/underwater.wav", qfalse);
	cgs.media.taunt[0] = trap_S_RegisterSound("sound/player/taunt.wav", qfalse);
	cgs.media.taunt[1] = trap_S_RegisterSound("sound/player/spin.wav", qfalse);

	cgs.media.peacem_reload2 = trap_S_RegisterSound("sound/weapons/peacem_reload2.wav", qfalse);
	cgs.media.winch97_reload2 = trap_S_RegisterSound("sound/weapons/winch97_reload2.wav", qfalse);
	cgs.media.rifle_reload2 = trap_S_RegisterSound("sound/weapons/rifle_reload2.wav", qfalse);

	cgs.media.scopePutSound = trap_S_RegisterSound("sound/weapons/scope.wav", qfalse);
	cgs.media.gatling_build = trap_S_RegisterSound("sound/weapons/gatling_build.wav", qfalse);
	cgs.media.gatling_dism = trap_S_RegisterSound("sound/weapons/gatling_dism.wav", qfalse);

//	cgs.media.buySound = trap_S_RegisterSound("sound/misc/buy_sound.wav", qfalse);

	CG_LoadingStage(1);

	//cgs.media.reloadRifle  = trap_S_RegisterSound("sound/weapons/shotgun/reload.wav", qfalse);
	cgs.media.reloadShotgun = trap_S_RegisterSound("sound/weapons/shotgun_reload.wav", qfalse);
	//cgs.media.reloadRifle2 = trap_S_RegisterSound("sound/weapons/shotgun/reload2.wav", qfalse);
	cgs.media.sfx_gatling_loop = trap_S_RegisterSound("sound/weapons/gatling_loop.wav", qfalse);

	for (i=0 ; i<3 ; i++) {
		Com_sprintf (name, sizeof(name), "sound/weapons/ric_wood%i.wav", i+1);
		cgs.media.impact[IMPACT_WOOD][i] = trap_S_RegisterSound (name, qfalse);

		Com_sprintf (name, sizeof(name), "sound/weapons/ric_metal%i.wav", i+1);
		cgs.media.impact[IMPACT_METAL][i] = trap_S_RegisterSound (name, qfalse);

		Com_sprintf (name, sizeof(name), "sound/weapons/ric_water%i.wav", i+1);
		cgs.media.impact[IMPACT_WATER][i] = trap_S_RegisterSound (name, qfalse);

		Com_sprintf (name, sizeof(name), "sound/weapons/ric_glass%i.wav", i+1);
		cgs.media.impact[IMPACT_GLASS][i] = trap_S_RegisterSound (name, qfalse);

		Com_sprintf (name, sizeof(name), "sound/weapons/ric_stone%i.wav", i+1);
		cgs.media.impact[IMPACT_STONE][i] = trap_S_RegisterSound (name, qfalse);

		Com_sprintf (name, sizeof(name), "sound/weapons/ric_sand%i.wav", i+1);
		cgs.media.impact[IMPACT_SAND][i] = trap_S_RegisterSound (name, qfalse);

		Com_sprintf (name, sizeof(name), "sound/weapons/ric_grass%i.wav", i+1);
		cgs.media.impact[IMPACT_GRASS][i] = trap_S_RegisterSound (name, qfalse);

		Com_sprintf (name, sizeof(name), "sound/weapons/ric_default%i.wav", i+1);
		cgs.media.impact[IMPACT_DEFAULT][i] = trap_S_RegisterSound (name, qfalse);

		// bang sounds
		Com_sprintf( name, sizeof(name), "sound/misc/bang%i.wav", i+1);
		cgs.media.bang[i] = trap_S_RegisterSound (name, qfalse);
	}

	for(i=0; i < 5; i++){
		Com_sprintf(name, sizeof(name), "music/duel_start%i.wav", i+1);
		// duel sounds
		cgs.media.duelstart[i] = trap_S_RegisterSound(name, qfalse);
	}
#endif

}


//===================================================================================


/*
=================
CG_RegisterGraphics

This function may execute for a couple of minutes with a slow disk.
=================
*/
static void CG_RegisterGraphics( void ) {
#ifndef SMOKINGUNS
	int			i;
#else
	int			i, j;
#endif
	char		items[MAX_ITEMS+1];
	static char		*sb_nums[11] = {
#ifndef SMOKINGUNS
		"gfx/2d/numbers/zero_32b",
		"gfx/2d/numbers/one_32b",
		"gfx/2d/numbers/two_32b",
		"gfx/2d/numbers/three_32b",
		"gfx/2d/numbers/four_32b",
		"gfx/2d/numbers/five_32b",
		"gfx/2d/numbers/six_32b",
		"gfx/2d/numbers/seven_32b",
		"gfx/2d/numbers/eight_32b",
		"gfx/2d/numbers/nine_32b",
		"gfx/2d/numbers/minus_32b",
#else
		"hud/0",
		"hud/1",
		"hud/2",
		"hud/3",
		"hud/4",
		"hud/5",
		"hud/6",
		"hud/7",
		"hud/8",
		"hud/9",
		"hud/minus",
	};

	static char		*sb_nums_money[11] = {
		"hud/money0",
		"hud/money1",
		"hud/money2",
		"hud/money3",
		"hud/money4",
		"hud/money5",
		"hud/money6",
		"hud/money7",
		"hud/money8",
		"hud/money9",
		"hud/moneyminus",
#endif
	};

	// clear any references to old media
	memset( &cg.refdef, 0, sizeof( cg.refdef ) );
	trap_R_ClearScene();

	CG_LoadingString( cgs.mapname );

	trap_R_LoadWorldMap( cgs.mapname );

#ifdef SMOKINGUNS
	CG_LoadingStage(4);
#endif

	// precache status bar pics
	CG_LoadingString( "game media" );

#ifdef SMOKINGUNS
	CG_LoadingStage(1);
#endif

	for ( i=0 ; i<11 ; i++) {
		cgs.media.numberShaders[i] = trap_R_RegisterShader( sb_nums[i] );
	}

#ifndef SMOKINGUNS
	cgs.media.botSkillShaders[0] = trap_R_RegisterShader( "menu/art/skill1.tga" );
	cgs.media.botSkillShaders[1] = trap_R_RegisterShader( "menu/art/skill2.tga" );
	cgs.media.botSkillShaders[2] = trap_R_RegisterShader( "menu/art/skill3.tga" );
	cgs.media.botSkillShaders[3] = trap_R_RegisterShader( "menu/art/skill4.tga" );
	cgs.media.botSkillShaders[4] = trap_R_RegisterShader( "menu/art/skill5.tga" );
#else
	for ( i=0 ; i<11 ; i++) {
		cgs.media.numbermoneyShaders[i] = trap_R_RegisterShader( sb_nums_money[i] );
	}

	// Spoon

	CG_LoadingStage(1);

	cgs.media.dead = trap_R_RegisterShaderNoMip( "gfx/score/dead.tga" );
	for( i= 0; i<5; i++){
		char name[64];

		Com_sprintf(name, sizeof(name), "gfx/score/medal%i.tga", i+1);
		cgs.media.won[i] = trap_R_RegisterShaderNoMip( name );
	}

	CG_LoadingStage(1);

	CG_LoadingStage(1);

	cgs.media.hud_ammo_bullet	= trap_R_RegisterShaderNoMip( "hud/ammo_bullet.tga" );
	cgs.media.hud_ammo_sharps	= trap_R_RegisterShaderNoMip( "hud/ammo_sharps.tga" );

	CG_LoadingStage(1);

	cgs.media.hud_ammo_cart		= trap_R_RegisterShaderNoMip( "hud/ammo_cart.tga");
	cgs.media.hud_ammo_shell	= trap_R_RegisterShaderNoMip( "hud/ammo_shell.tga");
	cgs.media.hud_button		= trap_R_RegisterShaderNoMip( "ui/wq3_assets/button.tga");
	cgs.media.hud_button_selected	= trap_R_RegisterShaderNoMip( "ui/wq3_assets/button_selected.tga");

	CG_LoadingStage(1);

	cgs.media.wqfx_sparks		= trap_R_RegisterShader( "wqfx/sparks" );
	cgs.media.wqfx_matchfire	= trap_R_RegisterShader( "wqfx/matchfire" );
	cgs.media.wqfx_smokeexp		= trap_R_RegisterShader( "wqfx/smokeexp");
	cgs.media.menu_end			= trap_R_RegisterShaderNoMip( "ui/wq3_assets/menu_end");

	CG_LoadingStage(1);

	cgs.media.menu_tex			= trap_R_RegisterShaderNoMip( "ui/wq3_assets/menu_tex");
	cgs.media.menu_top			= trap_R_RegisterShaderNoMip( "ui/wq3_assets/menu_top");
	cgs.media.menu_arrow		= trap_R_RegisterShaderNoMip( "ui/wq3_assets/menu_arrow");

	CG_LoadingStage(1);

	cgs.media.hud_wp_box		= trap_R_RegisterShaderNoMip( "hud/wp_box");
	cgs.media.hud_wp_top		= trap_R_RegisterShaderNoMip( "hud/wp_top");
	cgs.media.hud_wp_box_quad	= trap_R_RegisterShaderNoMip( "hud/wp_box_quad");

	CG_LoadingStage(1);

	CG_LoadingStage(1);

	//scope
	cgs.media.scopeShader = trap_R_RegisterShaderNoMip("gfx/2d/scope");
	cgs.media.scopecrossShader = trap_R_RegisterShaderNoMip("gfx/2d/scopecrosshair");

	CG_LoadingStage(1);

	// Spoon End

	cgs.media.botSkillShaders[0] = trap_R_RegisterShaderNoMip( "gfx/score/botskill1.tga" );
	cgs.media.botSkillShaders[1] = trap_R_RegisterShaderNoMip( "gfx/score/botskill2.tga" );
	cgs.media.botSkillShaders[2] = trap_R_RegisterShaderNoMip( "gfx/score/botskill3.tga" );
	cgs.media.botSkillShaders[3] = trap_R_RegisterShaderNoMip( "gfx/score/botskill4.tga" );
	cgs.media.botSkillShaders[4] = trap_R_RegisterShaderNoMip( "gfx/score/botskill5.tga" );

	CG_LoadingStage(1);
#endif

	cgs.media.viewBloodShader = trap_R_RegisterShader( "viewBloodBlend" );

	cgs.media.deferShader = trap_R_RegisterShaderNoMip( "gfx/2d/defer.tga" );

#ifndef SMOKINGUNS
	cgs.media.scoreboardName = trap_R_RegisterShaderNoMip( "menu/tab/name.tga" );
	cgs.media.scoreboardPing = trap_R_RegisterShaderNoMip( "menu/tab/ping.tga" );
	cgs.media.scoreboardScore = trap_R_RegisterShaderNoMip( "menu/tab/score.tga" );
	cgs.media.scoreboardTime = trap_R_RegisterShaderNoMip( "menu/tab/time.tga" );
#else
	CG_LoadingStage(1);
#endif

	cgs.media.smokePuffShader = trap_R_RegisterShader( "smokePuff" );
	cgs.media.smokePuffRageProShader = trap_R_RegisterShader( "smokePuffRagePro" );
	cgs.media.shotgunSmokePuffShader = trap_R_RegisterShader( "shotgunSmokePuff" );
#ifndef SMOKINGUNS
#ifdef MISSIONPACK
	cgs.media.nailPuffShader = trap_R_RegisterShader( "nailtrail" );
	cgs.media.blueProxMine = trap_R_RegisterModel( "models/weaphits/proxmineb.md3" );
#endif
	cgs.media.plasmaBallShader = trap_R_RegisterShader( "sprites/plasma1" );
#else
	CG_LoadingStage(1);
#endif
	cgs.media.bloodTrailShader = trap_R_RegisterShader( "bloodTrail" );
#ifdef SMOKINGUNS
	if (cg_lagometer.integer==2)
		cgs.media.lagometerShader = trap_R_RegisterShader("console" );
	else
#endif
		cgs.media.lagometerShader = trap_R_RegisterShader("lagometer" );
	cgs.media.connectionShader = trap_R_RegisterShader( "disconnected" );

	cgs.media.waterBubbleShader = trap_R_RegisterShader( "waterBubble" );

#ifdef SMOKINGUNS
	CG_LoadingStage(1);
#endif

	cgs.media.tracerShader = trap_R_RegisterShader( "gfx/misc/tracer" );
	cgs.media.selectShader = trap_R_RegisterShader( "gfx/2d/select" );

#ifndef SMOKINGUNS
	for ( i = 0 ; i < NUM_CROSSHAIRS ; i++ ) {
		cgs.media.crosshairShader[i] = trap_R_RegisterShader( va("gfx/2d/crosshair%c", 'a'+i) );
	}
#else
	for ( i = 0 ; i < NUM_CROSSHAIRS ; i++ ) {
		cgs.media.crosshairShader[i] = trap_R_RegisterShader( va("hud/crosshair%c", 'a'+i) );
	}
	cgs.media.crosshairFriendShader = trap_R_RegisterShader( "hud/crosshair_friend");

	CG_LoadingStage(1);
#endif

#ifndef SMOKINGUNS
	cgs.media.backTileShader = trap_R_RegisterShader( "gfx/2d/backtile" );
	cgs.media.noammoShader = trap_R_RegisterShader( "icons/noammo" );

	// powerup shaders
	cgs.media.quadShader = trap_R_RegisterShader("powerups/quad" );
	cgs.media.quadWeaponShader = trap_R_RegisterShader("powerups/quadWeapon" );
	cgs.media.battleSuitShader = trap_R_RegisterShader("powerups/battleSuit" );
	cgs.media.battleWeaponShader = trap_R_RegisterShader("powerups/battleWeapon" );
	cgs.media.invisShader = trap_R_RegisterShader("powerups/invisibility" );
	cgs.media.regenShader = trap_R_RegisterShader("powerups/regen" );
	cgs.media.hastePuffShader = trap_R_RegisterShader("hasteSmokePuff" );

#ifdef MISSIONPACK
	if ( cgs.gametype == GT_HARVESTER || cg_buildScript.integer ) {
		cgs.media.redCubeModel = trap_R_RegisterModel( "models/powerups/orb/r_orb.md3" );
		cgs.media.blueCubeModel = trap_R_RegisterModel( "models/powerups/orb/b_orb.md3" );
		cgs.media.redCubeIcon = trap_R_RegisterShader( "icons/skull_red" );
		cgs.media.blueCubeIcon = trap_R_RegisterShader( "icons/skull_blue" );
	}
#endif
#else
	cgs.media.backTileShader = trap_R_RegisterShader( "hud/tile" );

	CG_LoadingStage(1);
#endif

#ifndef SMOKINGUNS
#ifdef MISSIONPACK
	if ( cgs.gametype == GT_CTF || cgs.gametype == GT_1FCTF || cgs.gametype == GT_HARVESTER || cg_buildScript.integer ) {
#else
	if ( cgs.gametype == GT_CTF || cg_buildScript.integer ) {
#endif
		cgs.media.redFlagModel = trap_R_RegisterModel( "models/flags/r_flag.md3" );
		cgs.media.blueFlagModel = trap_R_RegisterModel( "models/flags/b_flag.md3" );
		cgs.media.redFlagShader[0] = trap_R_RegisterShaderNoMip( "icons/iconf_red1" );
		cgs.media.redFlagShader[1] = trap_R_RegisterShaderNoMip( "icons/iconf_red2" );
		cgs.media.redFlagShader[2] = trap_R_RegisterShaderNoMip( "icons/iconf_red3" );
		cgs.media.blueFlagShader[0] = trap_R_RegisterShaderNoMip( "icons/iconf_blu1" );
		cgs.media.blueFlagShader[1] = trap_R_RegisterShaderNoMip( "icons/iconf_blu2" );
		cgs.media.blueFlagShader[2] = trap_R_RegisterShaderNoMip( "icons/iconf_blu3" );
#ifdef MISSIONPACK
		cgs.media.flagPoleModel = trap_R_RegisterModel( "models/flag2/flagpole.md3" );
		cgs.media.flagFlapModel = trap_R_RegisterModel( "models/flag2/flagflap3.md3" );

		cgs.media.redFlagFlapSkin = trap_R_RegisterSkin( "models/flag2/red.skin" );
		cgs.media.blueFlagFlapSkin = trap_R_RegisterSkin( "models/flag2/blue.skin" );
		cgs.media.neutralFlagFlapSkin = trap_R_RegisterSkin( "models/flag2/white.skin" );

		cgs.media.redFlagBaseModel = trap_R_RegisterModel( "models/mapobjects/flagbase/red_base.md3" );
		cgs.media.blueFlagBaseModel = trap_R_RegisterModel( "models/mapobjects/flagbase/blue_base.md3" );
		cgs.media.neutralFlagBaseModel = trap_R_RegisterModel( "models/mapobjects/flagbase/ntrl_base.md3" );
#endif
	}

#ifdef MISSIONPACK
	if ( cgs.gametype == GT_1FCTF || cg_buildScript.integer ) {
		cgs.media.neutralFlagModel = trap_R_RegisterModel( "models/flags/n_flag.md3" );
		cgs.media.flagShader[0] = trap_R_RegisterShaderNoMip( "icons/iconf_neutral1" );
		cgs.media.flagShader[1] = trap_R_RegisterShaderNoMip( "icons/iconf_red2" );
		cgs.media.flagShader[2] = trap_R_RegisterShaderNoMip( "icons/iconf_blu2" );
		cgs.media.flagShader[3] = trap_R_RegisterShaderNoMip( "icons/iconf_neutral3" );
	}

	if ( cgs.gametype == GT_OBELISK || cg_buildScript.integer ) {
		cgs.media.rocketExplosionShader = trap_R_RegisterShader("rocketExplosion");
		cgs.media.overloadBaseModel = trap_R_RegisterModel( "models/powerups/overload_base.md3" );
		cgs.media.overloadTargetModel = trap_R_RegisterModel( "models/powerups/overload_target.md3" );
		cgs.media.overloadLightsModel = trap_R_RegisterModel( "models/powerups/overload_lights.md3" );
		cgs.media.overloadEnergyModel = trap_R_RegisterModel( "models/powerups/overload_energy.md3" );
	}

	if ( cgs.gametype == GT_HARVESTER || cg_buildScript.integer ) {
		cgs.media.harvesterModel = trap_R_RegisterModel( "models/powerups/harvester/harvester.md3" );
		cgs.media.harvesterRedSkin = trap_R_RegisterSkin( "models/powerups/harvester/red.skin" );
		cgs.media.harvesterBlueSkin = trap_R_RegisterSkin( "models/powerups/harvester/blue.skin" );
		cgs.media.harvesterNeutralModel = trap_R_RegisterModel( "models/powerups/obelisk/obelisk.md3" );
	}

	cgs.media.redKamikazeShader = trap_R_RegisterShader( "models/weaphits/kamikred" );
	cgs.media.dustPuffShader = trap_R_RegisterShader("hasteSmokePuff" );
#endif
#else
	cgs.media.dustPuffShader = trap_R_RegisterShader("hasteSmokePuff" );
#endif

#ifndef SMOKINGUNS
	if ( cgs.gametype >= GT_TEAM || cg_buildScript.integer ) {
		cgs.media.friendShader = trap_R_RegisterShader( "sprites/foe" );
		cgs.media.redQuadShader = trap_R_RegisterShader("powerups/blueflag" );
		cgs.media.teamStatusBar = trap_R_RegisterShader( "gfx/2d/colorbar.tga" );
#ifdef MISSIONPACK
		cgs.media.blueKamikazeShader = trap_R_RegisterShader( "models/weaphits/kamikblu" );
#endif
	}

	cgs.media.armorModel = trap_R_RegisterModel( "models/powerups/armor/armor_yel.md3" );
	cgs.media.armorIcon  = trap_R_RegisterShaderNoMip( "icons/iconr_yellow" );
#endif

	cgs.media.machinegunBrassModel = trap_R_RegisterModel( "models/weapons2/shells/m_shell.md3" );
#ifndef SMOKINGUNS
	cgs.media.shotgunBrassModel = trap_R_RegisterModel( "models/weapons2/shells/s_shell.md3" );

	cgs.media.gibAbdomen = trap_R_RegisterModel( "models/gibs/abdomen.md3" );
	cgs.media.gibArm = trap_R_RegisterModel( "models/gibs/arm.md3" );
	cgs.media.gibChest = trap_R_RegisterModel( "models/gibs/chest.md3" );
	cgs.media.gibFist = trap_R_RegisterModel( "models/gibs/fist.md3" );
	cgs.media.gibFoot = trap_R_RegisterModel( "models/gibs/foot.md3" );
	cgs.media.gibForearm = trap_R_RegisterModel( "models/gibs/forearm.md3" );
	cgs.media.gibIntestine = trap_R_RegisterModel( "models/gibs/intestine.md3" );
	cgs.media.gibLeg = trap_R_RegisterModel( "models/gibs/leg.md3" );
	cgs.media.gibSkull = trap_R_RegisterModel( "models/gibs/skull.md3" );
	cgs.media.gibBrain = trap_R_RegisterModel( "models/gibs/brain.md3" );

	cgs.media.smoke2 = trap_R_RegisterModel( "models/weapons2/shells/s_shell.md3" );

	cgs.media.balloonShader = trap_R_RegisterShader( "sprites/balloon3" );

	cgs.media.bloodExplosionShader = trap_R_RegisterShader( "bloodExplosion" );
#else
	cgs.media.balloonShader = trap_R_RegisterShader( "gfx/bubbles/talk.tga" );
	cgs.media.balloonBuyShader = trap_R_RegisterShader( "gfx/bubbles/buy.tga" );
#endif

	cgs.media.bulletFlashModel = trap_R_RegisterModel("models/weaphits/bullet.md3");
#ifndef SMOKINGUNS
	cgs.media.ringFlashModel = trap_R_RegisterModel("models/weaphits/ring02.md3");
	cgs.media.dishFlashModel = trap_R_RegisterModel("models/weaphits/boom01.md3");
#ifdef MISSIONPACK
	cgs.media.teleportEffectModel = trap_R_RegisterModel( "models/powerups/pop.md3" );
#else
	cgs.media.teleportEffectModel = trap_R_RegisterModel( "models/misc/telep.md3" );
	cgs.media.teleportEffectShader = trap_R_RegisterShader( "teleportEffect" );
#endif
#ifdef MISSIONPACK
	cgs.media.kamikazeEffectModel = trap_R_RegisterModel( "models/weaphits/kamboom2.md3" );
	cgs.media.kamikazeShockWave = trap_R_RegisterModel( "models/weaphits/kamwave.md3" );
	cgs.media.kamikazeHeadModel = trap_R_RegisterModel( "models/powerups/kamikazi.md3" );
	cgs.media.kamikazeHeadTrail = trap_R_RegisterModel( "models/powerups/trailtest.md3" );
	cgs.media.guardPowerupModel = trap_R_RegisterModel( "models/powerups/guard_player.md3" );
	cgs.media.scoutPowerupModel = trap_R_RegisterModel( "models/powerups/scout_player.md3" );
	cgs.media.doublerPowerupModel = trap_R_RegisterModel( "models/powerups/doubler_player.md3" );
	cgs.media.ammoRegenPowerupModel = trap_R_RegisterModel( "models/powerups/ammo_player.md3" );
	cgs.media.invulnerabilityImpactModel = trap_R_RegisterModel( "models/powerups/shield/impact.md3" );
	cgs.media.invulnerabilityJuicedModel = trap_R_RegisterModel( "models/powerups/shield/juicer.md3" );
	cgs.media.medkitUsageModel = trap_R_RegisterModel( "models/powerups/regen.md3" );
	cgs.media.heartShader = trap_R_RegisterShaderNoMip( "ui/assets/statusbar/selectedhealth.tga" );
	cgs.media.invulnerabilityPowerupModel = trap_R_RegisterModel( "models/powerups/shield/shield.md3" );
#endif
#else
	CG_LoadingStage(1);
#endif

#ifndef SMOKINGUNS
	cgs.media.medalImpressive = trap_R_RegisterShaderNoMip( "medal_impressive" );
	cgs.media.medalExcellent = trap_R_RegisterShaderNoMip( "medal_excellent" );
	cgs.media.medalGauntlet = trap_R_RegisterShaderNoMip( "medal_gauntlet" );
	cgs.media.medalDefend = trap_R_RegisterShaderNoMip( "medal_defend" );
	cgs.media.medalAssist = trap_R_RegisterShaderNoMip( "medal_assist" );
	cgs.media.medalCapture = trap_R_RegisterShaderNoMip( "medal_capture" );

#else
	// special models for missile-weapons
	cgs.media.e_knife = trap_R_RegisterModel("models/weapons2/knife/e_knife_hold.md3");
	cgs.media.e_dynamite = trap_R_RegisterModel("models/weapons2/dyn/e_dynamite_hold.md3");
	cgs.media.e_moneybag = trap_R_RegisterModel("models/items/e_moneybag.md3");

	// scope
	cgs.media.model_scope = trap_R_RegisterModel("models/weapons2/scope/scope.md3");

	// smoke effects
	cgs.media.pistol_smoke = trap_R_RegisterShaderNoMip("wqfx/pistol_smoke.tga");
	cgs.media.ent_smoke = trap_R_RegisterShaderNoMip("wqfx/smoke.tga");

	// additional weapon icons
	cgs.media.sharps_scope = trap_R_RegisterShaderNoMip("hud/weapons/sharps_scope.tga");
	cgs.media.scope_sharps = trap_R_RegisterShaderNoMip("hud/weapons/scope_sharps.tga");

	CG_LoadingStage(1);

	//additional SG models
	for(i=0;i<NUM_GIBS; i++){
		char name[64];

		Com_sprintf(name, sizeof(name), "models/gibs/glassgib%i.md3", i+1);
		cgs.media.glassgib[i] = trap_R_RegisterModel(name);

		if(i<5){
			Com_sprintf(name, sizeof(name), "models/gibs/papergib%i.md3", i+1);
			cgs.media.clothgib[i] = trap_R_RegisterModel(name);
		}
	}

	CG_LoadingStage(1);

	cgs.media.stonegibshader = trap_R_RegisterShader("stone");
	cgs.media.metalgibshader = trap_R_RegisterShader("metal");

	cgs.media.woodgib[0]	= trap_R_RegisterModel("models/gibs/woodgib1.md3");
	cgs.media.woodgib[1]	= trap_R_RegisterModel("models/gibs/woodgib2.md3");
	cgs.media.woodgib[2]	= trap_R_RegisterModel("models/gibs/woodgib3.md3");

	cgs.media.dirtgib[0]	= trap_R_RegisterModel("models/gibs/dirtgib1.md3");
	cgs.media.dirtgib[1]	= trap_R_RegisterModel("models/gibs/dirtgib2.md3");
	cgs.media.dirtgib[2]	= trap_R_RegisterModel("models/gibs/dirtgib3.md3");

	CG_LoadingStage(1);

	// molotov gibs
	cgs.media.molotovgib[0] = trap_R_RegisterModel("models/weapons2/molotov/gib1.md3");
	cgs.media.molotovgib[1] = trap_R_RegisterModel("models/weapons2/molotov/gib2.md3");
	cgs.media.molotovgib[2] = trap_R_RegisterModel("models/weapons2/molotov/gib3.md3");
	cgs.media.molotovgib[3] = trap_R_RegisterModel("models/weapons2/molotov/gib4.md3");

	//player model additions
	cgs.media.holster_l		= trap_R_RegisterModel("models/wq3_players/holster_l.md3");
	cgs.media.holster_r		= trap_R_RegisterModel("models/wq3_players/holster_r.md3");

	cgs.media.wqfx_fire		= trap_R_RegisterShader("wqfx/fire");
	cgs.media.firesound		= trap_S_RegisterSound("sound/world/fire.wav", qfalse);

	//particles
	for(j=0;j<NUM_PREFIXINFO; j++){
		for(i=0;i<3;i++){
			char name[64];

			Com_sprintf(name, sizeof(name), "particles/particle_%s_%i", prefixInfo[j].name, i+1);
			cgs.media.particles[j][i] = trap_R_RegisterShader(name);
		}
	}
	//marks
	for(j=0;j<NUM_PREFIXINFO; j++){
		char name[64];

		if(prefixInfo[j].surfaceFlags == SURF_WATER)
			continue;

		Com_sprintf(name, sizeof(name), "gfx/wq_marks/mark_%s", prefixInfo[j].name);
		cgs.media.breakmarks[j] = trap_R_RegisterShader(name);

		for(i=0;i<2;i++){
			Com_sprintf(name, sizeof(name), "gfx/wq_marks/mark_%s_%i", prefixInfo[j].name, i+1);
			cgs.media.marks[j][i] = trap_R_RegisterShader(name);
		}
	}

	// firedrop
	cgs.media.wqfx_firedrop = trap_R_RegisterShader("wqfx/fire_drop");

	//blood and whiskey particles
	for(i=0;i<6;i++){
		char name[64];

		// blood
		Com_sprintf(name, sizeof(name), "gfx/damage/blood_%i", i+1);
		cgs.media.blood_particles[i] = trap_R_RegisterShader(name);

		// whiskey
		Com_sprintf(name, sizeof(name), "wqfx/whiskey_%i", i+1);
		cgs.media.whiskey_drops[i] = trap_R_RegisterShader(name);
	}
	//blood and whiskey marks
	for(i=0;i<3;i++){
		char name[64];

		Com_sprintf(name, sizeof(name), "gfx/damage/blood_mrk_%i", i+1);
		cgs.media.blood_marks[i] = trap_R_RegisterShader(name);

		Com_sprintf(name, sizeof(name), "wqfx/whiskey_mrk_%i", i+1);
		cgs.media.whiskey_pools[i] = trap_R_RegisterShader(name);
	}

	cgs.media.flare = trap_R_RegisterShader("wqfx/flare");
	//cgs.media.sparktrail = trap_R_RegisterShader("wqfx/spark");

	CG_LoadingStage(1);
#endif

	memset( cg_items, 0, sizeof( cg_items ) );
	memset( cg_weapons, 0, sizeof( cg_weapons ) );

	// only register the items that the server says we need
	Q_strncpyz(items, CG_ConfigString(CS_ITEMS), sizeof(items));

	for ( i = 1 ; i < bg_numItems ; i++ ) {
		if ( items[ i ] == '1' || cg_buildScript.integer ) {
			CG_LoadingItem( i );
			CG_RegisterItemVisuals( i );
		}
	}

	// wall marks
#ifndef SMOKINGUNS
	cgs.media.bulletMarkShader = trap_R_RegisterShader( "gfx/damage/bullet_mrk" );
#endif
	cgs.media.burnMarkShader = trap_R_RegisterShader( "gfx/damage/burn_med_mrk" );
#ifndef SMOKINGUNS
	cgs.media.holeMarkShader = trap_R_RegisterShader( "gfx/damage/hole_lg_mrk" );
	cgs.media.energyMarkShader = trap_R_RegisterShader( "gfx/damage/plasma_mrk" );
#endif
	cgs.media.shadowMarkShader = trap_R_RegisterShader( "markShadow" );
	cgs.media.wakeMarkShader = trap_R_RegisterShader( "wake" );
#ifndef SMOKINGUNS
	cgs.media.bloodMarkShader = trap_R_RegisterShader( "bloodMark" );
#else
	CG_LoadingStage(1);
#endif

	// register the inline models
	cgs.numInlineModels = trap_CM_NumInlineModels();
	for ( i = 1 ; i < cgs.numInlineModels ; i++ ) {
		char	name[10];
		vec3_t			mins, maxs;
		int				j;

		Com_sprintf( name, sizeof(name), "*%i", i );
		cgs.inlineDrawModel[i] = trap_R_RegisterModel( name );
		trap_R_ModelBounds( cgs.inlineDrawModel[i], mins, maxs );
		for ( j = 0 ; j < 3 ; j++ ) {
			cgs.inlineModelMidpoints[i][j] = mins[j] + 0.5 * ( maxs[j] - mins[j] );
		}
#ifdef SMOKINGUNS
		if(!(i %3))
			CG_LoadingStage(1);
#endif
	}

	// register all the server specified models
	for (i=1 ; i<MAX_MODELS ; i++) {
		const char		*modelName;

		modelName = CG_ConfigString( CS_MODELS+i );
		if ( !modelName[0] ) {
			break;
		}
		cgs.gameModels[i] = trap_R_RegisterModel( modelName );

#ifdef SMOKINGUNS
		if(!(i %3))
			CG_LoadingStage(1);
#endif
	}

#ifndef SMOKINGUNS
#ifdef MISSIONPACK
	// new stuff
	cgs.media.patrolShader = trap_R_RegisterShaderNoMip("ui/assets/statusbar/patrol.tga");
	cgs.media.assaultShader = trap_R_RegisterShaderNoMip("ui/assets/statusbar/assault.tga");
	cgs.media.campShader = trap_R_RegisterShaderNoMip("ui/assets/statusbar/camp.tga");
	cgs.media.followShader = trap_R_RegisterShaderNoMip("ui/assets/statusbar/follow.tga");
	cgs.media.defendShader = trap_R_RegisterShaderNoMip("ui/assets/statusbar/defend.tga");
	cgs.media.teamLeaderShader = trap_R_RegisterShaderNoMip("ui/assets/statusbar/team_leader.tga");
	cgs.media.retrieveShader = trap_R_RegisterShaderNoMip("ui/assets/statusbar/retrieve.tga");
	cgs.media.escortShader = trap_R_RegisterShaderNoMip("ui/assets/statusbar/escort.tga");
	cgs.media.cursor = trap_R_RegisterShaderNoMip( "menu/art/3_cursor2" );
	cgs.media.sizeCursor = trap_R_RegisterShaderNoMip( "ui/assets/sizecursor.tga" );
	cgs.media.selectCursor = trap_R_RegisterShaderNoMip( "ui/assets/selectcursor.tga" );
	cgs.media.flagShaders[0] = trap_R_RegisterShaderNoMip("ui/assets/statusbar/flag_in_base.tga");
	cgs.media.flagShaders[1] = trap_R_RegisterShaderNoMip("ui/assets/statusbar/flag_capture.tga");
	cgs.media.flagShaders[2] = trap_R_RegisterShaderNoMip("ui/assets/statusbar/flag_missing.tga");

	trap_R_RegisterModel( "models/players/james/lower.md3" );
	trap_R_RegisterModel( "models/players/james/upper.md3" );
	trap_R_RegisterModel( "models/players/heads/james/james.md3" );

	trap_R_RegisterModel( "models/players/janet/lower.md3" );
	trap_R_RegisterModel( "models/players/janet/upper.md3" );
	trap_R_RegisterModel( "models/players/heads/janet/janet.md3" );

#endif
	CG_ClearParticles ();
/*
	for (i=1; i<MAX_PARTICLES_AREAS; i++)
	{
		{
			int rval;

			rval = CG_NewParticleArea ( CS_PARTICLES + i);
			if (!rval)
				break;
		}
	}
*/
#else
	//2d graphic of item bound
	//cgs.media.itembound = trap_R_RegisterShaderNoMip("gfx/2d/itembound.tga");

	//gatling gun models
	cgs.media.g_tripod = trap_R_RegisterModel( "models/weapons2/gatling/g_tripod.md3");
	cgs.media.g_middle = trap_R_RegisterModel( "models/weapons2/gatling/g_middle.md3");
	cgs.media.g_gatling = trap_R_RegisterModel( "models/weapons2/gatling/g_gatling.md3");
	cgs.media.g_barrel = trap_R_RegisterModel( "models/weapons2/gatling/g_barrel.md3");
	cgs.media.g_crank = trap_R_RegisterModel( "models/weapons2/gatling/g_crank.md3");
	cgs.media.g_hand = trap_R_RegisterModel( "models/weapons2/gatling/g_hand.md3");
	cgs.media.g_mag = trap_R_RegisterModel("models/weapons2/gatling/g_mag.md3");
	cgs.media.g_mag_v = trap_R_RegisterModel("models/weapons2/gatling/g_mag_v.md3");

	// duel
	// medal
	cgs.media.du_medal = trap_R_RegisterModel("models/misc/medal.md3");
	// ai_node
	cgs.media.ai_node = trap_R_RegisterModel("models/misc/ai_node.md3");
	// spectatorfly
	cgs.media.fly = trap_R_RegisterModel("models/misc/fly.md3");
	cgs.media.bpoint = trap_R_RegisterShader("wqfx/blackpoint");
	cgs.media.fly_sound = trap_S_RegisterSound("sound/misc/fly.wav", qfalse);
	// other money-models
	cgs.media.coins = trap_R_RegisterModel("models/items/coins.md3");
	cgs.media.bills = trap_R_RegisterModel("models/items/bills.md3");
	cgs.media.coins_pic = trap_R_RegisterShader("hud/weapons/coins");
	cgs.media.bills_pic = trap_R_RegisterShader("hud/weapons/bills");
	// indicate escape point
	cgs.media.indicate_lf = trap_R_RegisterShader("gfx/misc/indicator_lf");
	cgs.media.indicate_fw = trap_R_RegisterShader("gfx/misc/indicator_fw");
	cgs.media.indicate_rt = trap_R_RegisterShader("gfx/misc/indicator_rt");
	cgs.media.indicate_bk = trap_R_RegisterShader("gfx/misc/indicator_bk");

	cgs.media.quad = trap_R_RegisterModel("models/misc/quad.md3");
	// not quad damage, but a model consisting of a simple square
	// (needed for bullet holes in breakables
#endif
}



/*
=======================
CG_BuildSpectatorString

=======================
*/
void CG_BuildSpectatorString(void) {
	int i;
	cg.spectatorList[0] = 0;
	for (i = 0; i < MAX_CLIENTS; i++) {
		int j, client = -1;
		// find scores for player
		for(j = 0; j < cg.numScores; j++){
			if(cg.scores[j].client == i){
				client = j;
				break;
			}
		}
		if (cgs.clientinfo[i].infoValid && cgs.clientinfo[i].team == TEAM_SPECTATOR
			&& (cgs.gametype != GT_DUEL || client == -1 || cg.scores[client].realspec)) {
			// TheDoctor: correctly colored
			if (*cg.spectatorList)
				Q_strcat(cg.spectatorList, sizeof(cg.spectatorList), ",   ");
			Q_strcat(cg.spectatorList, sizeof(cg.spectatorList), va(S_COLOR_WHITE "%s" S_COLOR_YELLOW, cgs.clientinfo[i].name));
		}
	}
	i = strlen(cg.spectatorList);
	if (i != cg.spectatorLen) {
		cg.spectatorLen = i;
		cg.spectatorWidth = -1;
		Vector4Copy( colorWhite, cg.spectatorCurrentColor );
	}
}


/*
===================
CG_RegisterClients
===================
*/
static void CG_RegisterClients( void ) {
	int		i;

	CG_LoadingClient(cg.clientNum);
	CG_NewClientInfo(cg.clientNum);

	for (i=0 ; i<MAX_CLIENTS ; i++) {
		const char		*clientInfo;

		if (cg.clientNum == i) {
			continue;
		}

		clientInfo = CG_ConfigString( CS_PLAYERS+i );
		if ( !clientInfo[0]) {
			continue;
		}
		CG_LoadingClient( i );
		CG_NewClientInfo( i );
	}
	CG_BuildSpectatorString();
}

//===========================================================================

/*
=================
CG_ConfigString
=================
*/
const char *CG_ConfigString( int index ) {
	if ( index < 0 || index >= MAX_CONFIGSTRINGS ) {
		CG_Error( "CG_ConfigString: bad index: %i", index );
	}
	return cgs.gameState.stringData + cgs.gameState.stringOffsets[ index ];
}

//==================================================================

#ifdef SMOKINGUNS
static void CG_ClearTrackStats(void){
	int i;

	for ( i=0; i<trackInfoNum; i++){
		trackInfo[i].played = qfalse;
	}
}
static qboolean CG_CheckTrackStats(void){
	int i;
	int count = 0;

	for ( i=0; i<trackInfoNum; i++){
		if(trackInfo[i].played)
			count++;
	}
	if(count == trackInfoNum)
		return qtrue;

	return qfalse;
}
#endif

#ifndef SMOKINGUNS
/*
======================
CG_StartMusic

======================
*/
void CG_StartMusic( void ) {
	char	*s;
	char	parm1[MAX_QPATH], parm2[MAX_QPATH];

	// start the background music
	s = (char *)CG_ConfigString( CS_MUSIC );
	Q_strncpyz( parm1, COM_Parse( &s ), sizeof( parm1 ) );
	Q_strncpyz( parm2, COM_Parse( &s ), sizeof( parm2 ) );

	trap_S_StartBackgroundTrack( parm1, parm2 );
}
#else
/*
======================
CG_PlayMusic

======================
*/

// Fading time is 10 seconds
#define	MUSICFADE_TIME		10000
// Wait at least 100 ms before trying to adjust music fading volume
#define	MUSICFADE_TIMESTEP	100

void CG_PlayMusic( void ) {
	char	parm1[MAX_QPATH], parm2[MAX_QPATH];
	int		trackNum;
	char	buffer[64];
	int		fadetime;

	// if it's duel and startup, don't play anything
	if(cgs.gametype == GT_DUEL && cg.introend >= cg.time){
		trap_Cvar_Set("s_musicfading", "0.0");
		return;
	}

	if ( cg.custommusic ) {  // start custom music
		if ( cg.custommusic_started) {
			return;
		} else {
			// read and parse name of custom wav file
			char *s = (char *)CG_ConfigString( CS_MUSIC );
			Q_strncpyz( parm1, COM_Parse( &s ), sizeof( parm1 ) );
			Q_strncpyz( parm2, COM_Parse( &s ), sizeof( parm2 ) );
			cg.custommusic_started = qtrue;
		}
	} else { // random music system
		static float fading = 0.0f ;
		static int fadingtime = 0 ;

		if ( !cg.musicfile )
			return;

		// Avoid to try to set music fade volume too often
		if (cg.time < fadingtime)
			return;
		else
			fadingtime = cg.time + MUSICFADE_TIMESTEP;

		//fade in
		fadetime = cg.time - cg.playmusic_starttime;
		if(fadetime < MUSICFADE_TIME && fadetime >= 0 && cg.playmusic_starttime){

			fading = (float)fadetime/MUSICFADE_TIME;

			trap_Cvar_Set("s_musicfading", va("%f", fading));
			return;
		}

		//fade out
		fadetime = cg.playmusic_endtime - cg.time;
		if(fadetime < MUSICFADE_TIME && fadetime >= 0 && cg.playmusic_endtime){

			fading = (float)fadetime/MUSICFADE_TIME;

			trap_Cvar_Set("s_musicfading", va("%f", fading));
			return;
		}

		if(cg.playmusic_starttime < cg.playmusic_endtime && fading < 1.0f) {
			trap_Cvar_Set("s_musicfading", "1.0");
			fading = 1.0f;
			// We want to avoid to set s_musicfading at the end of fade-out
			cg.playmusic_starttime = cg.playmusic_endtime + MUSICFADE_TIMESTEP;
		}

		if(cg.playmusic_endtime > cg.time)
			return;

		if (fading > 0.0f) {
			trap_Cvar_Set("s_musicfading", "0.0");
			fading = 0.0f;
		}

		if(CG_CheckTrackStats())
			CG_ClearTrackStats();

		do {
			trackNum = rand()%trackInfoNum;

		} while(trackInfo[trackNum].played || trackNum == cg.oldTrackNum);

		Com_sprintf( parm1, sizeof(parm1), trackInfo[trackNum].path);
		Com_sprintf( parm2, sizeof(parm2), trackInfo[trackNum].path);
		trackInfo[trackNum].played = qtrue;
		cg.playmusic_endtime = cg.time + trackInfo[trackNum].replays*trackInfo[trackNum].length;
		cg.playmusic_starttime = cg.time;
		cg.oldTrackNum = trackNum;
	}

	trap_S_StartBackgroundTrack( parm1, parm2 );
}

void CG_SelectMusic( void ) {
	char *s = (char *)CG_ConfigString( CS_MUSIC );

	if ( s[0] ) {  // enable random music system
		cg.custommusic = qtrue;
	} else {  // enable custom music for this map
		cg.custommusic = qfalse;
		cg.custommusic_started = qfalse;
	}
}
#endif

#ifdef SMOKINGUNS
char *CG_GetMenuBuffer(const char *filename) {
	int	len;
	fileHandle_t	f;
	static char buf[MAX_MENUFILE];

	len = trap_FS_FOpenFile( filename, &f, FS_READ );
	if ( !f ) {
		trap_Print( va( S_COLOR_RED "menu file not found: %s, using default\n", filename ) );
		return NULL;
	}
	if ( len >= MAX_MENUFILE ) {
		trap_Print( va( S_COLOR_RED "menu file too large: %s is %i, max allowed is %i\n", filename, len, MAX_MENUFILE ) );
		trap_FS_FCloseFile( f );
		return NULL;
	}

	trap_FS_Read( buf, len, f );
	buf[len] = 0;
	trap_FS_FCloseFile( f );

	return buf;
}

//
// ==============================
// new hud stuff ( mission pack )
// ==============================
//
qboolean CG_Asset_Parse(int handle) {
	pc_token_t token;
	const char *tempStr;

	if (!trap_PC_ReadToken(handle, &token))
		return qfalse;
	if (Q_stricmp(token.string, "{") != 0) {
		return qfalse;
	}

	while ( 1 ) {
		if (!trap_PC_ReadToken(handle, &token))
			return qfalse;

		if (Q_stricmp(token.string, "}") == 0) {
			return qtrue;
		}

		// font
		if (Q_stricmp(token.string, "font") == 0) {
			int pointSize;
			if (!PC_String_Parse(handle, &tempStr) || !PC_Int_Parse(handle, &pointSize)) {
				return qfalse;
			}
			cgDC.registerFont(tempStr, pointSize, &cgDC.Assets.textFont);
			continue;
		}

		// smallFont
		if (Q_stricmp(token.string, "smallFont") == 0) {
			int pointSize;
			if (!PC_String_Parse(handle, &tempStr) || !PC_Int_Parse(handle, &pointSize)) {
				return qfalse;
			}
			cgDC.registerFont(tempStr, pointSize, &cgDC.Assets.smallFont);
			continue;
		}

		// font
		if (Q_stricmp(token.string, "bigfont") == 0) {
			int pointSize;
			if (!PC_String_Parse(handle, &tempStr) || !PC_Int_Parse(handle, &pointSize)) {
				return qfalse;
			}
			cgDC.registerFont(tempStr, pointSize, &cgDC.Assets.bigFont);
			continue;
		}

		// gradientbar
		if (Q_stricmp(token.string, "gradientbar") == 0) {
			if (!PC_String_Parse(handle, &tempStr)) {
				return qfalse;
			}
			cgDC.Assets.gradientBar = trap_R_RegisterShaderNoMip(tempStr);
			continue;
		}

		// enterMenuSound
		if (Q_stricmp(token.string, "menuEnterSound") == 0) {
			if (!PC_String_Parse(handle, &tempStr)) {
				return qfalse;
			}
			cgDC.Assets.menuEnterSound = trap_S_RegisterSound( tempStr, qfalse );
			continue;
		}

		// exitMenuSound
		if (Q_stricmp(token.string, "menuExitSound") == 0) {
			if (!PC_String_Parse(handle, &tempStr)) {
				return qfalse;
			}
			cgDC.Assets.menuExitSound = trap_S_RegisterSound( tempStr, qfalse );
			continue;
		}

		// itemFocusSound
		if (Q_stricmp(token.string, "itemFocusSound") == 0) {
			if (!PC_String_Parse(handle, &tempStr)) {
				return qfalse;
			}
			cgDC.Assets.itemFocusSound = trap_S_RegisterSound( tempStr, qfalse );
			continue;
		}

		// menuBuzzSound
		if (Q_stricmp(token.string, "menuBuzzSound") == 0) {
			if (!PC_String_Parse(handle, &tempStr)) {
				return qfalse;
			}
			cgDC.Assets.menuBuzzSound = trap_S_RegisterSound( tempStr, qfalse );
			continue;
		}

		if (Q_stricmp(token.string, "cursor") == 0) {
			if (!PC_String_Parse(handle, &cgDC.Assets.cursorStr)) {
				return qfalse;
			}
			cgDC.Assets.cursor = trap_R_RegisterShaderNoMip( cgDC.Assets.cursorStr);
			continue;
		}

		if (Q_stricmp(token.string, "fadeClamp") == 0) {
			if (!PC_Float_Parse(handle, &cgDC.Assets.fadeClamp)) {
				return qfalse;
			}
			continue;
		}

		if (Q_stricmp(token.string, "fadeCycle") == 0) {
			if (!PC_Int_Parse(handle, &cgDC.Assets.fadeCycle)) {
				return qfalse;
			}
			continue;
		}

		if (Q_stricmp(token.string, "fadeAmount") == 0) {
			if (!PC_Float_Parse(handle, &cgDC.Assets.fadeAmount)) {
				return qfalse;
			}
			continue;
		}

		if (Q_stricmp(token.string, "shadowX") == 0) {
			if (!PC_Float_Parse(handle, &cgDC.Assets.shadowX)) {
				return qfalse;
			}
			continue;
		}

		if (Q_stricmp(token.string, "shadowY") == 0) {
			if (!PC_Float_Parse(handle, &cgDC.Assets.shadowY)) {
				return qfalse;
			}
			continue;
		}

		if (Q_stricmp(token.string, "shadowColor") == 0) {
			if (!PC_Color_Parse(handle, &cgDC.Assets.shadowColor)) {
				return qfalse;
			}
			cgDC.Assets.shadowFadeClamp = cgDC.Assets.shadowColor[3];
			continue;
		}
	}
	return qfalse;
}

void CG_ParseMenu(const char *menuFile) {
	pc_token_t token;
	int handle;

	handle = trap_PC_LoadSource(menuFile);
#ifndef SMOKINGUNS
	if (!handle)
		handle = trap_PC_LoadSource("ui/testhud.menu");
#endif
	if (!handle)
		return;

	while ( 1 ) {
		if (!trap_PC_ReadToken( handle, &token )) {
			break;
		}

		//if ( Q_stricmp( token, "{" ) ) {
		//	Com_Printf( "Missing { in menu file\n" );
		//	break;
		//}

		//if ( menuCount == MAX_MENUS ) {
		//	Com_Printf( "Too many menus!\n" );
		//	break;
		//}

		if ( token.string[0] == '}' ) {
			break;
		}

		if (Q_stricmp(token.string, "assetGlobalDef") == 0) {
			if (CG_Asset_Parse(handle)) {
				continue;
			} else {
				break;
			}
		}


		if (Q_stricmp(token.string, "menudef") == 0) {
			// start a new menu
			Menu_New(handle);
		}
	}
	trap_PC_FreeSource(handle);
}

qboolean CG_Load_Menu(char **p) {
	char *token;

	token = COM_ParseExt(p, qtrue);

	if (token[0] != '{') {
		return qfalse;
	}

	while ( 1 ) {

		token = COM_ParseExt(p, qtrue);

		if (Q_stricmp(token, "}") == 0) {
			return qtrue;
		}

		if ( !token || token[0] == 0 ) {
			return qfalse;
		}

		CG_ParseMenu(token);
	}
	return qfalse;
}



void CG_LoadMenus(const char *menuFile) {
	char	*token;
	char *p;
	int	len, start;
	fileHandle_t	f;
	static char buf[MAX_MENUDEFFILE];

	start = trap_Milliseconds();

#ifndef SMOKINGUNS
	len = trap_FS_FOpenFile( menuFile, &f, FS_READ );
	if ( !f ) {
		Com_Printf( S_COLOR_YELLOW "menu file not found: %s, using default\n", menuFile );
		len = trap_FS_FOpenFile( "ui/hud.txt", &f, FS_READ );
		if (!f) {
			CG_Error( S_COLOR_RED "default menu file not found: ui/hud.txt, unable to continue!" );
		}
	}
#else
	len = trap_FS_FOpenFile( "ui/hud.txt", &f, FS_READ );
	if (!f) {
		CG_Error( S_COLOR_RED "menu file not found: ui/hud.txt, unable to continue!" );
	}
#endif

	if ( len >= MAX_MENUDEFFILE ) {
		trap_FS_FCloseFile( f );
		CG_Error( S_COLOR_RED "menu file too large: %s is %i, max allowed is %i", menuFile, len, MAX_MENUDEFFILE );
		return;
	}

	trap_FS_Read( buf, len, f );
	buf[len] = 0;
	trap_FS_FCloseFile( f );

	COM_Compress(buf);

	Menu_Reset();

	p = buf;

	while ( 1 ) {
		token = COM_ParseExt( &p, qtrue );
		if( !token || token[0] == 0 || token[0] == '}') {
			break;
		}

		//if ( Q_stricmp( token, "{" ) ) {
		//	Com_Printf( "Missing { in menu file\n" );
		//	break;
		//}

		//if ( menuCount == MAX_MENUS ) {
		//	Com_Printf( "Too many menus!\n" );
		//	break;
		//}

		if ( Q_stricmp( token, "}" ) == 0 ) {
			break;
		}

		if (Q_stricmp(token, "loadmenu") == 0) {
			if (CG_Load_Menu(&p)) {
				continue;
			} else {
				break;
			}
		}
	}

	Com_Printf("UI menu load time = %d milli seconds\n", trap_Milliseconds() - start);

}



static qboolean CG_OwnerDrawHandleKey(int ownerDraw, int flags, float *special, int key) {
	return qfalse;
}


static int CG_FeederCount(float feederID) {
	int i, count;
	count = 0;
	if (feederID == FEEDER_REDTEAM_LIST) {
		for (i = 0; i < cg.numScores; i++) {
#ifndef SMOKINGUNS
			if (cg.scores[i].team == TEAM_RED) {
#else
			if (cg.scores[i].team == TEAM_RED || cg.scores[i].team == TEAM_RED_SPECTATOR) {
#endif
				count++;
			}
		}
	} else if (feederID == FEEDER_BLUETEAM_LIST) {
		for (i = 0; i < cg.numScores; i++) {
#ifndef SMOKINGUNS
			if (cg.scores[i].team == TEAM_BLUE) {
#else
			if (cg.scores[i].team == TEAM_BLUE || cg.scores[i].team == TEAM_BLUE_SPECTATOR) {
#endif
				count++;
			}
		}
	} else if (feederID == FEEDER_SCOREBOARD) {
#ifndef SMOKINGUNS
		return cg.numScores;
#else
		for(i = 0; i < cg.numScores; i++){
			if(cg.scores[i].team == TEAM_FREE || (cgs.gametype == GT_DUEL &&
				!cg.scores[i].realspec && cg.scores[i].team == TEAM_SPECTATOR)){
				count++;
			}
		}
#endif
	}
	return count;
}


void CG_SetScoreSelection(void *p) {
	menuDef_t *menu = (menuDef_t*)p;
	playerState_t *ps = &cg.snap->ps;
	int i, red, blue;
	red = blue = 0;
	for (i = 0; i < cg.numScores; i++) {
		if (cg.scores[i].team == TEAM_RED) {
			red++;
		} else if (cg.scores[i].team == TEAM_BLUE) {
			blue++;
		}
		if (ps->clientNum == cg.scores[i].client) {
			cg.selectedScore = i;
		}
	}

	if (menu == NULL) {
		// just interested in setting the selected score
		return;
	}

	if ( cgs.gametype >= GT_TEAM ) {
		int feeder = FEEDER_REDTEAM_LIST;
		i = red;
		if (cg.scores[cg.selectedScore].team == TEAM_BLUE) {
			feeder = FEEDER_BLUETEAM_LIST;
			i = blue;
		}
		Menu_SetFeederSelection(menu, feeder, i, NULL);
	} else {
		Menu_SetFeederSelection(menu, FEEDER_SCOREBOARD, cg.selectedScore, NULL);
	}
}

// FIXME: might need to cache this info
static clientInfo_t * CG_InfoFromScoreIndex(int index, int team, int *scoreIndex) {
	int i, count;
	if ( cgs.gametype >= GT_TEAM ) {
		count = 0;
		for (i = 0; i < cg.numScores; i++) {
#ifndef SMOKINGUNS
			if (cg.scores[i].team == team) {
#else
			if (cg.scores[i].team == team || cg.scores[i].team-3 == team) {
				if (count == index) {
					*scoreIndex = i;
					return &cgs.clientinfo[cg.scores[i].client];
				}
				count++;
			}
		}
	} else {
		count = 0;
		for (i = 0; i < cg.numScores; i++) {
			if (cg.scores[i].team == TEAM_FREE ||
				(cgs.gametype == GT_DUEL && !cg.scores[i].realspec
				&& cg.scores[i].team == TEAM_SPECTATOR)) {
#endif

				if (count == index) {
					*scoreIndex = i;
					return &cgs.clientinfo[cg.scores[i].client];
				}
				count++;
			}
		}
	}
	*scoreIndex = index;
	return &cgs.clientinfo[ cg.scores[index].client ];
}

static const char *CG_FeederItemText(float feederID, int index, int column, qhandle_t *handle) {
#ifndef SMOKINGUNS
	gitem_t *item;
#else
	int trueteam;
#endif
	int scoreIndex = 0;
	clientInfo_t *info = NULL;
	int team = -1;
	score_t *sp = NULL;

	*handle = -1;

	if (feederID == FEEDER_REDTEAM_LIST) {
		team = TEAM_RED;
	} else if (feederID == FEEDER_BLUETEAM_LIST) {
		team = TEAM_BLUE;
	}

	info = CG_InfoFromScoreIndex(index, team, &scoreIndex);
	sp = &cg.scores[scoreIndex];

#ifndef SMOKINGUNS
	if (info && info->infoValid) {
		switch (column) {
			case 0:
				if ( info->powerups & ( 1 << PW_NEUTRALFLAG ) ) {
					item = BG_FindItemForPowerup( PW_NEUTRALFLAG );
					*handle = cg_items[ ITEM_INDEX(item) ].icon;
				} else if ( info->powerups & ( 1 << PW_REDFLAG ) ) {
					item = BG_FindItemForPowerup( PW_REDFLAG );
					*handle = cg_items[ ITEM_INDEX(item) ].icon;
				} else if ( info->powerups & ( 1 << PW_BLUEFLAG ) ) {
					item = BG_FindItemForPowerup( PW_BLUEFLAG );
					*handle = cg_items[ ITEM_INDEX(item) ].icon;
				} else {
					if ( info->botSkill > 0 && info->botSkill <= 5 ) {
						*handle = cgs.media.botSkillShaders[ info->botSkill - 1 ];
					} else if ( info->handicap < 100 ) {
					return va("%i", info->handicap );
					}
				}
#else
	trueteam = info->team;

	if (info && info->infoValid) {

		if(column >= 2 && cgs.gametype >= GT_TEAM)
			column += 1;

		switch (column) {
			case 0:
					if ( info->botSkill > 0 && info->botSkill <= 5 ) {
						*handle = cgs.media.botSkillShaders[ info->botSkill - 1 ];
					} else if ( info->handicap < 100 ) {
					return va("%i", info->handicap );
					}
#endif
			break;
			case 1:
#ifndef SMOKINGUNS
				if (team == -1) {
					return "";
				} else {
					*handle = CG_StatusHandle(info->teamTask);
				}
#else
				if (team == -1 && cgs.gametype != GT_DUEL) {
					return "";
				} else {
					if(trueteam == TEAM_RED_SPECTATOR || trueteam == TEAM_BLUE_SPECTATOR)
						*handle = cgs.media.dead;//CG_StatusHandle(info->teamTask);
					if(cgs.gametype == GT_DUEL && !sp->realspec && trueteam >= TEAM_SPECTATOR)
						*handle = cgs.media.dead;
				}
#endif
			break;
			case 2:
#ifndef SMOKINGUNS
				if ( cg.snap->ps.stats[ STAT_CLIENTS_READY ] & ( 1 << sp->client ) ) {
					return "Ready";
				}
				if (team == -1) {
					if (cgs.gametype == GT_TOURNAMENT) {
						return va("%i/%i", info->wins, info->losses);
					} else if (info->infoValid && info->team == TEAM_SPECTATOR ) {
						return "Spectator";
					} else {
						return "";
					}
				} else {
					if (info->teamLeader) {
						return "Leader";
					}
				}
#else
				// give the number of stars
				if(info->wins && cgs.gametype == GT_DUEL){
					if(info->wins <= 5)
						*handle = cgs.media.won[info->wins-1];
					return "";
				}
#endif
			break;
			case 3:
#ifndef SMOKINGUNS
				return info->name;
#else
				if ( cg.snap->ps.stats[ STAT_CLIENTS_READY ] & ( 1 << sp->client ) ) {
					return "Ready";
				}
#endif
			break;
			case 4:
#ifndef SMOKINGUNS
				return va("%i", info->score);
#else
				if(cgs.gametype == GT_DUEL)
					return va("(%i) %s", info->losses, info->name);
				return info->name;
#endif
			break;
			case 5:
#ifndef SMOKINGUNS
				return va("%4i", sp->time);
#else
				return va("%i", info->score);
#endif
			break;
			case 6:
#ifndef SMOKINGUNS
				if ( sp->ping == -1 ) {
					return "connecting";
				}
				return va("%4i", sp->ping);
#else
				return va("%4i", sp->time);
			break;
			case 7:
				if ( sp->ping == -1 ) {
					return "connecting";
				}
				return va(" %4i", sp->ping);
#endif
			break;
		}
	}

	return "";
}

static qhandle_t CG_FeederItemImage(float feederID, int index) {
	return 0;
}

static void CG_FeederSelection(float feederID, int index) {
	if ( cgs.gametype >= GT_TEAM ) {
		int i, count;
		int team = (feederID == FEEDER_REDTEAM_LIST) ? TEAM_RED : TEAM_BLUE;
		count = 0;
		for (i = 0; i < cg.numScores; i++) {
			if (cg.scores[i].team == team) {
				if (index == count) {
					cg.selectedScore = i;
				}
				count++;
			}
		}
	} else {
		cg.selectedScore = index;
	}
}
#endif

#ifdef SMOKINGUNS
static float CG_Cvar_Get(const char *cvar) {
	char buff[128];
	memset(buff, 0, sizeof(buff));
	trap_Cvar_VariableStringBuffer(cvar, buff, sizeof(buff));
	return atof(buff);
}
#endif

#ifdef SMOKINGUNS
void CG_Text_PaintWithCursor(float x, float y, float scale, vec4_t color, const char *text, int cursorPos, char cursor, int limit, int style) {
	CG_Text_Paint(x, y, scale, color, text, 0, limit, style);
}

static int CG_OwnerDrawWidth(int ownerDraw, float scale) {
	switch (ownerDraw) {
	  case CG_GAME_TYPE:
			return CG_Text_Width(CG_GameTypeString(), scale, 0);
	  case CG_GAME_STATUS:
			return CG_Text_Width(CG_GetGameStatusText(), scale, 0);
			break;
	  case CG_KILLER:
			return CG_Text_Width(CG_GetKillerText(), scale, 0);
			break;
	  case CG_RED_NAME:
			return CG_Text_Width(cg_redTeamName.string, scale, 0);
			break;
	  case CG_BLUE_NAME:
			return CG_Text_Width(cg_blueTeamName.string, scale, 0);
			break;


	}
	return 0;
}

static int CG_PlayCinematic(const char *name, float x, float y, float w, float h) {
  return trap_CIN_PlayCinematic(name, x, y, w, h, CIN_loop);
}

static void CG_StopCinematic(int handle) {
  trap_CIN_StopCinematic(handle);
}

static void CG_DrawCinematic(int handle, float x, float y, float w, float h) {
  trap_CIN_SetExtents(handle, x, y, w, h);
  trap_CIN_DrawCinematic(handle);
}

static void CG_RunCinematicFrame(int handle) {
  trap_CIN_RunCinematic(handle);
}

/*
=================
CG_LoadHudMenu();

=================
*/
void CG_LoadHudMenu( void ) {
#ifndef SMOKINGUNS
	char buff[1024];
#endif
	const char *hudSet;

	cgDC.registerShaderNoMip = &trap_R_RegisterShaderNoMip;
	cgDC.setColor = &trap_R_SetColor;
	cgDC.drawHandlePic = &CG_DrawPic;
	cgDC.drawStretchPic = &trap_R_DrawStretchPic;
	cgDC.drawText = &CG_Text_Paint;
	cgDC.textWidth = &CG_Text_Width;
	cgDC.textHeight = &CG_Text_Height;
	cgDC.registerModel = &trap_R_RegisterModel;
	cgDC.modelBounds = &trap_R_ModelBounds;
	cgDC.fillRect = &CG_FillRect;
	cgDC.drawRect = &CG_DrawRect;
	cgDC.drawSides = &CG_DrawSides;
	cgDC.drawTopBottom = &CG_DrawTopBottom;
	cgDC.clearScene = &trap_R_ClearScene;
	cgDC.addRefEntityToScene = &trap_R_AddRefEntityToScene;
	cgDC.renderScene = &trap_R_RenderScene;
	cgDC.registerFont = &trap_R_RegisterFont;
	cgDC.ownerDrawItem = &CG_OwnerDraw;
	cgDC.getValue = &CG_GetValue;
	cgDC.ownerDrawVisible = &CG_OwnerDrawVisible;
	cgDC.runScript = &CG_RunMenuScript;
	cgDC.getTeamColor = &CG_GetTeamColor;
	cgDC.setCVar = trap_Cvar_Set;
	cgDC.getCVarString = trap_Cvar_VariableStringBuffer;
	cgDC.getCVarValue = CG_Cvar_Get;
	cgDC.drawTextWithCursor = &CG_Text_PaintWithCursor;
	//cgDC.setOverstrikeMode = &trap_Key_SetOverstrikeMode;
	//cgDC.getOverstrikeMode = &trap_Key_GetOverstrikeMode;
	cgDC.startLocalSound = &trap_S_StartLocalSound;
	cgDC.ownerDrawHandleKey = &CG_OwnerDrawHandleKey;
	cgDC.feederCount = &CG_FeederCount;
	cgDC.feederItemImage = &CG_FeederItemImage;
	cgDC.feederItemText = &CG_FeederItemText;
	cgDC.feederSelection = &CG_FeederSelection;
	//cgDC.setBinding = &trap_Key_SetBinding;
	//cgDC.getBindingBuf = &trap_Key_GetBindingBuf;
	//cgDC.keynumToStringBuf = &trap_Key_KeynumToStringBuf;
	//cgDC.executeText = &trap_Cmd_ExecuteText;
	cgDC.Error = &Com_Error;
	cgDC.Print = &Com_Printf;
	cgDC.ownerDrawWidth = &CG_OwnerDrawWidth;
	//cgDC.Pause = &CG_Pause;
	cgDC.registerSound = &trap_S_RegisterSound;
	cgDC.startBackgroundTrack = &trap_S_StartBackgroundTrack;
	cgDC.stopBackgroundTrack = &trap_S_StopBackgroundTrack;
	cgDC.playCinematic = &CG_PlayCinematic;
	cgDC.stopCinematic = &CG_StopCinematic;
	cgDC.drawCinematic = &CG_DrawCinematic;
	cgDC.runCinematicFrame = &CG_RunCinematicFrame;

	Init_Display(&cgDC);

	Menu_Reset();

#ifndef SMOKINGUNS
	trap_Cvar_VariableStringBuffer("cg_hudFiles", buff, sizeof(buff));
	hudSet = buff;
	if (hudSet[0] == '\0') {
		hudSet = "ui/hud.txt";
	}
#else
	hudSet = "ui/hud.txt";
#endif

	CG_LoadMenus(hudSet);
}

void CG_AssetCache( void ) {
	//if (Assets.textFont == NULL) {
	//  trap_R_RegisterFont("fonts/arial.ttf", 72, &Assets.textFont);
	//}
	//Assets.background = trap_R_RegisterShaderNoMip( ASSET_BACKGROUND );
	//Com_Printf("Menu Size: %i bytes\n", sizeof(Menus));
	cgDC.Assets.gradientBar = trap_R_RegisterShaderNoMip( ASSET_GRADIENTBAR );
#ifndef SMOKINGUNS
	cgDC.Assets.fxBasePic = trap_R_RegisterShaderNoMip( ART_FX_BASE );
	cgDC.Assets.fxPic[0] = trap_R_RegisterShaderNoMip( ART_FX_RED );
	cgDC.Assets.fxPic[1] = trap_R_RegisterShaderNoMip( ART_FX_YELLOW );
	cgDC.Assets.fxPic[2] = trap_R_RegisterShaderNoMip( ART_FX_GREEN );
	cgDC.Assets.fxPic[3] = trap_R_RegisterShaderNoMip( ART_FX_TEAL );
	cgDC.Assets.fxPic[4] = trap_R_RegisterShaderNoMip( ART_FX_BLUE );
	cgDC.Assets.fxPic[5] = trap_R_RegisterShaderNoMip( ART_FX_CYAN );
	cgDC.Assets.fxPic[6] = trap_R_RegisterShaderNoMip( ART_FX_WHITE );
#endif
	cgDC.Assets.scrollBar = trap_R_RegisterShaderNoMip( ASSET_SCROLLBAR );
	cgDC.Assets.scrollBarArrowDown = trap_R_RegisterShaderNoMip( ASSET_SCROLLBAR_ARROWDOWN );
	cgDC.Assets.scrollBarArrowUp = trap_R_RegisterShaderNoMip( ASSET_SCROLLBAR_ARROWUP );
	cgDC.Assets.scrollBarArrowLeft = trap_R_RegisterShaderNoMip( ASSET_SCROLLBAR_ARROWLEFT );
	cgDC.Assets.scrollBarArrowRight = trap_R_RegisterShaderNoMip( ASSET_SCROLLBAR_ARROWRIGHT );
	cgDC.Assets.scrollBarThumb = trap_R_RegisterShaderNoMip( ASSET_SCROLL_THUMB );
	cgDC.Assets.sliderBar = trap_R_RegisterShaderNoMip( ASSET_SLIDER_BAR );
	cgDC.Assets.sliderThumb = trap_R_RegisterShaderNoMip( ASSET_SLIDER_THUMB );
}
#endif
/*
=================
CG_Init

Called after every level change or subsystem restart
Will perform callbacks to make the loading info screen update.
=================
*/
void CG_Init( int serverMessageNum, int serverCommandSequence, int clientNum ) {
	const char	*s;
#ifdef SMOKINGUNS
	char		name[64], name2[64];

	CG_Printf( "Cgame version: %s, %s, %s\n", Q3_VERSION, PRODUCT_RELEASE, __TIME__ );
#endif

	// clear everything
	memset( &cgs, 0, sizeof( cgs ) );
	memset( &cg, 0, sizeof( cg ) );
	memset( cg_entities, 0, sizeof(cg_entities) );
	memset( cg_weapons, 0, sizeof(cg_weapons) );
	memset( cg_items, 0, sizeof(cg_items) );

	cg.clientNum = clientNum;

	cgs.processedSnapshotNum = serverMessageNum;
	cgs.serverCommandSequence = serverCommandSequence;

	// load a few needed things before we do any screen updates
	cgs.media.charsetShader		= trap_R_RegisterShader( "gfx/2d/bigchars" );
	cgs.media.whiteShader		= trap_R_RegisterShader( "white" );
	cgs.media.charsetProp		= trap_R_RegisterShaderNoMip( "menu/art/font1_prop.tga" );
#ifndef SMOKINGUNS
// Tequila comment: Removed that as it is not existing and is not used
	cgs.media.charsetPropGlow	= trap_R_RegisterShaderNoMip( "menu/art/font1_prop_glo.tga" );
#endif
	cgs.media.charsetPropB		= trap_R_RegisterShaderNoMip( "menu/art/font2_prop.tga" );

	CG_RegisterCvars();

	CG_InitConsoleCommands();

#ifndef SMOKINGUNS
	cg.weaponSelect = WP_MACHINEGUN;
#else
	cg.weaponSelect = WP_REM58;
	cg.lastusedweapon = WP_NONE;

	//menu
	cg.menu = MENU_NONE;
	cg.menustat = 0;
	cg.menuitem = 0;
	trap_Cvar_Set("cl_menu", "0");

	// reset aipointer
	ai_nodepointer = 0;
#endif

	cgs.redflag = cgs.blueflag = -1; // For compatibily, default to unset for
	cgs.flagStatus = -1;
	// old servers

	// get the rendering configuration from the client system
	trap_GetGlconfig( &cgs.glconfig );
	cgs.screenXScale = cgs.glconfig.vidWidth / 640.0;
	cgs.screenYScale = cgs.glconfig.vidHeight / 480.0;

	// get the gamestate from the client system
	trap_GetGameState( &cgs.gameState );

	// check version
	s = CG_ConfigString( CS_GAME_VERSION );
	if ( strcmp( s, GAME_VERSION ) ) {
		CG_Error( "Client/Server game mismatch: %s/%s", GAME_VERSION, s );
	}

	s = CG_ConfigString( CS_LEVEL_START_TIME );
	cgs.levelStartTime = atoi( s );

	CG_ParseServerinfo();

	// load the new map
	CG_LoadingString( "collision map" );

	trap_CM_LoadMap( cgs.mapname );

#ifdef SMOKINGUNS
	//load shader-infos
	COM_StripExtension( cgs.mapname, name2, sizeof(name2) );
	Com_sprintf(name, sizeof(name), "%s.tex", name2);
	CG_ParseTexFile( name );

	//load music.cfg
	if(CG_ParseMusicFile()){
		cg.musicfile = qtrue;
	} else {
		cg.musicfile = qfalse;
	}

	String_Init();
#endif

	cg.loading = qtrue;		// force players to load instead of defer

#ifdef SMOKINGUNS
	//set to zero
	CG_LoadingStage(0);
#endif
	CG_LoadingString( "sounds" );

	CG_RegisterSounds();

	CG_LoadingString( "graphics" );

	CG_RegisterGraphics();

	CG_LoadingString( "clients" );

	CG_RegisterClients();		// if low on memory, some clients will be deferred

#ifdef SMOKINGUNS
	CG_AssetCache();
	CG_LoadHudMenu();      // load new hud stuff
#endif

	cg.loading = qfalse;	// future players will be deferred

	CG_InitLocalEntities();

	CG_InitMarkPolys();

	// remove the last loading update
	cg.infoScreenText[0] = 0;

	// Make sure we have update values (scores)
	CG_SetConfigValues();

#ifndef SMOKINGUNS
	CG_StartMusic();
#else
	CG_SelectMusic();
	trap_Cvar_Set("s_musicfading", "0.0");
#endif

	CG_LoadingString( "" );

#ifdef SMOKINGUNS
	CG_InitTeamChat();
#endif

	CG_ShaderStateChanged();

	trap_S_ClearLoopingSounds( qtrue );
}

/*
=================
CG_Shutdown

Called before every level change or subsystem restart
=================
*/
void CG_Shutdown( void ) {
	// some mods may need to do cleanup work here,
	// like closing files or archiving session data
}


/*
==================
CG_EventHandling
==================
 type 0 - no event handling
      1 - team menu
      2 - hud editor

*/
#ifndef SMOKINGUNS
void CG_EventHandling(int type) {
}



void CG_KeyEvent(int key, qboolean down) {
}

void CG_MouseEvent(int x, int y) {
}
#endif

