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
// cg_consolecmds.c -- text commands typed in at the local console, or
// executed by a key binding

#include "cg_local.h"
#ifdef SMOKINGUNS
#include "../ui/ui_shared.h"
extern menuDef_t *menuScoreboard;
// Smoking' Guns specific
extern menuDef_t *menuBuy;
extern menuDef_t *menuItem;
extern int menuItemCount;
extern coord_t save_menuBuy;
extern coord_t save_menuItem;
#endif



void CG_TargetCommand_f( void ) {
	int		targetNum;
	char	test[4];

	targetNum = CG_CrosshairPlayer();
	if ( targetNum == -1 ) {
		return;
	}

	trap_Argv( 1, test, 4 );
	trap_SendClientCommand( va( "gc %i %i", targetNum, atoi( test ) ) );
}



/*
=================
CG_SizeUp_f

Keybinding command
=================
*/
static void CG_SizeUp_f (void) {
	trap_Cvar_Set("cg_viewsize", va("%i",(int)(cg_viewsize.integer+10)));
}


/*
=================
CG_SizeDown_f

Keybinding command
=================
*/
static void CG_SizeDown_f (void) {
	trap_Cvar_Set("cg_viewsize", va("%i",(int)(cg_viewsize.integer-10)));
}


/*
=============
CG_Viewpos_f

Debugging command to print the current position
=============
*/
static void CG_Viewpos_f (void) {
#ifndef SMOKINGUNS
	CG_Printf ("(%i %i %i) : %i\n", (int)cg.refdef.vieworg[0],
		(int)cg.refdef.vieworg[1], (int)cg.refdef.vieworg[2], 
		(int)cg.refdefViewAngles[YAW]);
#else
	CG_Printf ("(%f %f %f) : (%f %f %f)\n", cg.refdef.vieworg[0],
		cg.refdef.vieworg[1], cg.refdef.vieworg[2],
		cg.refdefViewAngles[0], cg.refdefViewAngles[1],
		cg.refdefViewAngles[2]);
#endif
}


static void CG_ScoresDown_f( void ) {

#ifdef SMOKINGUNS
		CG_BuildSpectatorString();
#endif
	if ( cg.scoresRequestTime + 2000 < cg.time ) {
		// the scores are more than two seconds out of data,
		// so request new ones
		cg.scoresRequestTime = cg.time;
		trap_SendClientCommand( "score" );

		// leave the current scores up if they were already
		// displayed, but if this is the first hit, clear them out
		if ( !cg.showScores ) {
			cg.showScores = qtrue;
			cg.numScores = 0;
		}
	} else {
		// show the cached contents even if they just pressed if it
		// is within two seconds
		cg.showScores = qtrue;
	}
}

static void CG_ScoresUp_f( void ) {
	if ( cg.showScores ) {
		cg.showScores = qfalse;
		cg.scoreFadeTime = cg.time;
	}
}

#ifdef SMOKINGUNS
void CG_ClearFocusses(void);
//set up the menu
void CG_BuyMenu (void) {

	if(cgs.gametype == GT_DUEL && cg.introend < cg.time)
		return;

	if(cg.snap->ps.persistant[PERS_TEAM] >= TEAM_SPECTATOR)
		return;

	if(cg.snap->ps.stats[STAT_HEALTH] <= 0)
		return;

	// if one is too far away from buy-point
	if(!(cg.snap->ps.stats[STAT_FLAGS] & SF_CANBUY) && (cgs.gametype >= GT_RTP))
		return;

	/* as suggested by The Doctor, this test shouldn't be done in cgame, it is already done in game, it removes a bug
	// if it's to late to buy
	if(cg.time - cg.roundstarttime > BUY_TIME && cgs.gametype >= GT_RTP){
		CG_Printf("60 seconds have passed ... you can't buy anything\n");
		return;
	}
	*/
	
	// if we're throwing away the gatling and it's being used, abort
	/*if(cg.snap->ps.weapon == WP_GATLING && cg.snap->ps.stats[STAT_GATLING_MODE])
		return;*/ // weapon change isn't activated anymore when buying and having a gatling

	// show the menu
	if(menuBuy == NULL){
		menuBuy = Menus_FindByName("buymenu");
		save_menuBuy.x = menuBuy->window.rect.x;
		save_menuBuy.y = menuBuy->window.rect.y;
	}

	if(menuItem == NULL){
		menuItem = Menus_FindByName("buymenu_items");
		save_menuItem.x = menuItem->window.rect.x;
		save_menuItem.y = menuItem->window.rect.y;
	}

	cgs.cursorX = cgs.cursorY = 0;

	cg.menu = MENU_BUY;
	cg.menustat = 1;
	cg.menumove = qtrue;
	cg.menuitem = 0;
	cg.oldbutton = qfalse;

	CG_ClearFocusses();

	if(cgs.gametype == GT_DUEL){
		int i;
		// set focus to 1st item
		cg.menuitem = 1;

		for(i=0;i< menuItem->itemCount; i++){
			if(menuItem->items[i]->window.ownerDraw == CG_BUTTON1){
				menuItem->items[i]->window.flags |= WINDOW_HASFOCUS;
				break;
			}
		}
	}

	trap_Cvar_Set("cl_menu", "0");
	trap_Key_SetCatcher(KEYCATCH_CGAME);

	// Notify the server we open the buy menu.
	// The "client" engine code will set BUTTON_BUYMENU to usercmd_t.buttons .
	trap_SendConsoleCommand( "+button8" );
}
#endif

#ifdef SMOKINGUNS
extern menuDef_t *menuScoreboard;
void Menu_Reset( void );			// FIXME: add to right include file

static void CG_LoadHud_f( void) {
  char buff[1024];
	const char *hudSet;
  memset(buff, 0, sizeof(buff));

	String_Init();
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
#endif

	CG_LoadMenus(hudSet);
	menuScoreboard = NULL;
#ifdef SMOKINGUNS
	menuBuy = NULL;
	menuItem = NULL;
	menuItemCount = 0;
#endif
}


#ifdef SMOKINGUNS
static void CG_scrollScoresDown_f( void) {
	if (menuScoreboard && cg.scoreBoardShowing) {
		Menu_ScrollFeeder(menuScoreboard, FEEDER_SCOREBOARD, qtrue);
		Menu_ScrollFeeder(menuScoreboard, FEEDER_REDTEAM_LIST, qtrue);
		Menu_ScrollFeeder(menuScoreboard, FEEDER_BLUETEAM_LIST, qtrue);
	}
}


static void CG_scrollScoresUp_f( void) {
	if (menuScoreboard && cg.scoreBoardShowing) {
		Menu_ScrollFeeder(menuScoreboard, FEEDER_SCOREBOARD, qfalse);
		Menu_ScrollFeeder(menuScoreboard, FEEDER_REDTEAM_LIST, qfalse);
		Menu_ScrollFeeder(menuScoreboard, FEEDER_BLUETEAM_LIST, qfalse);
	}
}


static void CG_spWin_f( void) {
	trap_Cvar_Set("cg_cameraOrbit", "2");
	trap_Cvar_Set("cg_cameraOrbitDelay", "35");
	trap_Cvar_Set("cg_thirdPerson", "1");
	trap_Cvar_Set("cg_thirdPersonAngle", "0");
	trap_Cvar_Set("cg_thirdPersonRange", "100");
#ifndef SMOKINGUNS
	CG_AddBufferedSound(cgs.media.winnerSound);
#endif
	//trap_S_StartLocalSound(cgs.media.winnerSound, CHAN_ANNOUNCER);
	CG_CenterPrint("YOU WIN!", SCREEN_HEIGHT * .30, 0);
}

static void CG_spLose_f( void) {
	trap_Cvar_Set("cg_cameraOrbit", "2");
	trap_Cvar_Set("cg_cameraOrbitDelay", "35");
	trap_Cvar_Set("cg_thirdPerson", "1");
	trap_Cvar_Set("cg_thirdPersonAngle", "0");
	trap_Cvar_Set("cg_thirdPersonRange", "100");
#ifndef SMOKINGUNS
	CG_AddBufferedSound(cgs.media.loserSound);
#endif
	//trap_S_StartLocalSound(cgs.media.loserSound, CHAN_ANNOUNCER);
	CG_CenterPrint("YOU LOSE...", SCREEN_HEIGHT * .30, 0);
}

#endif

static void CG_TellTarget_f( void ) {
	int		clientNum;
	char	command[128];
	char	message[128];

	clientNum = CG_CrosshairPlayer();
	if ( clientNum == -1 ) {
		return;
	}

	trap_Args( message, 128 );
	Com_sprintf( command, 128, "tell %i %s", clientNum, message );
	trap_SendClientCommand( command );
}

static void CG_TellAttacker_f( void ) {
	int		clientNum;
	char	command[128];
	char	message[128];

	clientNum = CG_LastAttacker();
	if ( clientNum == -1 ) {
		return;
	}

	trap_Args( message, 128 );
	Com_sprintf( command, 128, "tell %i %s", clientNum, message );
	trap_SendClientCommand( command );
}

#ifdef SMOKINGUNS
static void CG_VoiceTellTarget_f( void ) {
	int		clientNum;
	char	command[128];
	char	message[128];

	clientNum = CG_CrosshairPlayer();
	if ( clientNum == -1 ) {
		return;
	}

	trap_Args( message, 128 );
	Com_sprintf( command, 128, "vtell %i %s", clientNum, message );
	trap_SendClientCommand( command );
}

static void CG_VoiceTellAttacker_f( void ) {
	int		clientNum;
	char	command[128];
	char	message[128];

	clientNum = CG_LastAttacker();
	if ( clientNum == -1 ) {
		return;
	}

	trap_Args( message, 128 );
	Com_sprintf( command, 128, "vtell %i %s", clientNum, message );
	trap_SendClientCommand( command );
}

static void CG_NextTeamMember_f( void ) {
  CG_SelectNextPlayer();
}

static void CG_PrevTeamMember_f( void ) {
  CG_SelectPrevPlayer();
}

// ASS U ME's enumeration order as far as task specific orders, OFFENSE is zero, CAMP is last
//
static void CG_NextOrder_f( void ) {
	clientInfo_t *ci = cgs.clientinfo + cg.snap->ps.clientNum;
	if (ci) {
		if (!ci->teamLeader && sortedTeamPlayers[cg_currentSelectedPlayer.integer] != cg.snap->ps.clientNum) {
			return;
		}
	}
	if (cgs.currentOrder < TEAMTASK_CAMP) {
		cgs.currentOrder++;

#ifndef SMOKINGUNS
		if (cgs.currentOrder == TEAMTASK_RETRIEVE) {
			if (!CG_OtherTeamHasFlag()) {
				cgs.currentOrder++;
			}
		}

		if (cgs.currentOrder == TEAMTASK_ESCORT) {
			if (!CG_YourTeamHasFlag()) {
				cgs.currentOrder++;
			}
		}
#endif

	} else {
		cgs.currentOrder = TEAMTASK_OFFENSE;
	}
	cgs.orderPending = qtrue;
	cgs.orderTime = cg.time + 3000;
}


static void CG_ConfirmOrder_f (void ) {
	trap_SendConsoleCommand(va("cmd vtell %d %s\n", cgs.acceptLeader, VOICECHAT_YES));
	trap_SendConsoleCommand("+button5; wait; -button5");
	if (cg.time < cgs.acceptOrderTime) {
		trap_SendClientCommand(va("teamtask %d\n", cgs.acceptTask));
		cgs.acceptOrderTime = 0;
	}
}

static void CG_DenyOrder_f (void ) {
	trap_SendConsoleCommand(va("cmd vtell %d %s\n", cgs.acceptLeader, VOICECHAT_NO));
	trap_SendConsoleCommand("+button6; wait; -button6");
	if (cg.time < cgs.acceptOrderTime) {
		cgs.acceptOrderTime = 0;
	}
}

static void CG_TaskOffense_f (void ) {
#ifndef SMOKINGUNS
	if (cgs.gametype == GT_CTF || cgs.gametype == GT_1FCTF) {
#else
	if (cgs.gametype == GT_DUEL) {
#endif
		trap_SendConsoleCommand(va("cmd vsay_team %s\n", VOICECHAT_ONGETFLAG));
	} else {
		trap_SendConsoleCommand(va("cmd vsay_team %s\n", VOICECHAT_ONOFFENSE));
	}
	trap_SendClientCommand(va("teamtask %d\n", TEAMTASK_OFFENSE));
}

static void CG_TaskDefense_f (void ) {
	trap_SendConsoleCommand(va("cmd vsay_team %s\n", VOICECHAT_ONDEFENSE));
	trap_SendClientCommand(va("teamtask %d\n", TEAMTASK_DEFENSE));
}

static void CG_TaskPatrol_f (void ) {
	trap_SendConsoleCommand(va("cmd vsay_team %s\n", VOICECHAT_ONPATROL));
	trap_SendClientCommand(va("teamtask %d\n", TEAMTASK_PATROL));
}

static void CG_TaskCamp_f (void ) {
	trap_SendConsoleCommand(va("cmd vsay_team %s\n", VOICECHAT_ONCAMPING));
	trap_SendClientCommand(va("teamtask %d\n", TEAMTASK_CAMP));
}

static void CG_TaskFollow_f (void ) {
	trap_SendConsoleCommand(va("cmd vsay_team %s\n", VOICECHAT_ONFOLLOW));
	trap_SendClientCommand(va("teamtask %d\n", TEAMTASK_FOLLOW));
}

static void CG_TaskRetrieve_f (void ) {
	trap_SendConsoleCommand(va("cmd vsay_team %s\n", VOICECHAT_ONRETURNFLAG));
	trap_SendClientCommand(va("teamtask %d\n", TEAMTASK_RETRIEVE));
}

static void CG_TaskEscort_f (void ) {
	trap_SendConsoleCommand(va("cmd vsay_team %s\n", VOICECHAT_ONFOLLOWCARRIER));
	trap_SendClientCommand(va("teamtask %d\n", TEAMTASK_ESCORT));
}

static void CG_TaskOwnFlag_f (void ) {
	trap_SendConsoleCommand(va("cmd vsay_team %s\n", VOICECHAT_IHAVEFLAG));
}

static void CG_TauntKillInsult_f (void ) {
	trap_SendConsoleCommand("cmd vsay kill_insult\n");
}

static void CG_TauntPraise_f (void ) {
	trap_SendConsoleCommand("cmd vsay praise\n");
}

static void CG_TauntTaunt_f (void ) {
	trap_SendConsoleCommand("cmd vtaunt\n");
}

static void CG_TauntDeathInsult_f (void ) {
	trap_SendConsoleCommand("cmd vsay death_insult\n");
}

static void CG_TauntGauntlet_f (void ) {
	trap_SendConsoleCommand("cmd vsay kill_guantlet\n");
}

static void CG_TaskSuicide_f (void ) {
	int		clientNum;
	char	command[128];

	clientNum = CG_CrosshairPlayer();
	if ( clientNum == -1 ) {
		return;
	}

	Com_sprintf( command, 128, "tell %i suicide", clientNum );
	trap_SendClientCommand( command );
}



/*
==================
CG_TeamMenu_f
==================
*/
/*
static void CG_TeamMenu_f( void ) {
  if (trap_Key_GetCatcher() & KEYCATCH_CGAME) {
    CG_EventHandling(CGAME_EVENT_NONE);
    trap_Key_SetCatcher(0);
  } else {
    CG_EventHandling(CGAME_EVENT_TEAMMENU);
    //trap_Key_SetCatcher(KEYCATCH_CGAME);
  }
}
*/

/*
==================
CG_EditHud_f
==================
*/
/*
static void CG_EditHud_f( void ) {
  //cls.keyCatchers ^= KEYCATCH_CGAME;
  //VM_Call (cgvm, CG_EVENT_HANDLING, (cls.keyCatchers & KEYCATCH_CGAME) ? CGAME_EVENT_EDITHUD : CGAME_EVENT_NONE);
}
*/

#endif

/*
==================
CG_StartOrbit_f
==================
*/

static void CG_StartOrbit_f( void ) {
#ifndef SMOKINGUNS
	char var[MAX_TOKEN_CHARS];

	trap_Cvar_VariableStringBuffer( "developer", var, sizeof( var ) );
	if ( !atoi(var) ) {
		return;
	}
#endif
	if (cg_cameraOrbit.value != 0) {
		trap_Cvar_Set ("cg_cameraOrbit", "0");
		trap_Cvar_Set("cg_thirdPerson", "0");
	} else {
		trap_Cvar_Set("cg_cameraOrbit", "5");
		trap_Cvar_Set("cg_thirdPerson", "1");
		trap_Cvar_Set("cg_thirdPersonAngle", "0");
		trap_Cvar_Set("cg_thirdPersonRange", "100");
	}
}

/*
static void CG_Camera_f( void ) {
	char name[1024];
	trap_Argv( 1, name, sizeof(name));
	if (trap_loadCamera(name)) {
		cg.cameraMode = qtrue;
		trap_startCamera(cg.time);
	} else {
		CG_Printf ("Unable to load camera %s\n",name);
	}
}
*/

#ifdef SMOKINGUNS
static void CG_AngleDecrease_f(void){
	cg_thirdPersonAngle.value -= 10.0;

	if(cg_thirdPersonAngle.value < 0)
		cg_thirdPersonAngle.value = 360 - cg_thirdPersonAngle.value;

}

static void CG_AngleIncrease_f(void){
	cg_thirdPersonAngle.value += 10.0;

	if(cg_thirdPersonAngle.value > 360)
		cg_thirdPersonAngle.value = cg_thirdPersonAngle.value - 360;

}

static void CG_ShowCoords_f(void){
	CG_Printf("%f %f %f\n", cg.refdef.vieworg[0], cg.refdef.vieworg[1],
		cg.refdef.vieworg[2]);
}
#endif

#ifndef SMOKINGUNS
static void CG_ScopeUp(void){
	cg.scopetime += 5;
	if(cg.scopetime > SCOPE_TIME)
		cg.scopetime = SCOPE_TIME;

	//CG_Printf("%i\n", cg.scopetime);
}


static void CG_ScopeDown(void){
	cg.scopetime -= 5;
	if(cg.scopetime < 0)
		cg.scopetime = 0;

	//CG_Printf("%i\n", cg.scopetime);
}
#endif

/*
 * ==================
 *  AI-Node-Code
 * ==================
 */

#ifdef SMOKINGUNS
static void CG_SaveFileAiNode_f(void){
	int				i;
	fileHandle_t	file;
	char *filename;
	const char	*info;
	const char	*map;
	char	header[] = "AI-NODES";

	if(!cg_cheats)
		return;

	// get mapname
	info = CG_ConfigString( CS_SERVERINFO );
	map = Info_ValueForKey( info, "mapname" );
	filename = va("maps/%s.ai", map);

	if ( trap_FS_FOpenFile( filename, &file, FS_WRITE ) < 0 ) {
		CG_Printf(  "Could not write AI-node file: %s\n", filename  );
		trap_FS_FCloseFile( file );
		return;
	}

	// write the ai-node-date
	// first write ai-node header for verification
	trap_FS_Write(header, sizeof(header), file);

	// loop through the nodes
	for(i=0; i<ai_nodepointer; i++){
		trap_FS_Write(ai_nodes[i], sizeof(vec3_t), file);
	}

	trap_FS_FCloseFile(file);
	CG_Printf("AI-Node_File: %s succesfully written.\n", filename);
}

static void CG_OpenFileAiNode_f(void){
	int				i;
	fileHandle_t	file;
	int				len;
	char			headercheck[sizeof(AINODE_FILE_HEADER)+1];
	char			*filename;
	const char		*info;
	const char		*map;
	char 			buf[MAX_AINODEFILE+1];

	int				pos = 0;

	if(!cg_cheats)
		return;

	// get mapname
	info = CG_ConfigString( CS_SERVERINFO );
	map = Info_ValueForKey( info, "mapname" );
	filename = va("maps/%s.ai", map);

	len = trap_FS_FOpenFile( filename, &file, FS_READ );
	if ( !file ) {
		trap_FS_FCloseFile( file );
		return;
	}
	if ( len > MAX_AINODEFILE ) {
		CG_Printf("ai file too big: %s! %d > " STRING(MAX_AINODEFILE) "\n", filename);
		trap_FS_FCloseFile( file );
		return;
	}
	trap_FS_Read( buf, len, file );
	buf[len] = 0;
	trap_FS_FCloseFile( file );

	// read the header
	Q_strncpyz(headercheck, buf, sizeof(AINODE_FILE_HEADER));
	pos += sizeof(AINODE_FILE_HEADER);

	//check
	if(Q_stricmp(headercheck, AINODE_FILE_HEADER)){
		CG_Printf("Not a valid ai-file: %s\n", filename);
		return;
	}

	// now read the waypoints into the memory
	for(i=0; i<MAX_AINODES; i++){
		if(len <= pos){
			break;
		}
		Com_Memcpy(ai_nodes[i], buf+pos, sizeof(vec3_t));
		pos += sizeof(vec3_t);
		//G_Printf("%i. %f %f %f\n", i+1, ai_nodes[i][0], ai_nodes[i][1], ai_nodes[i][2]);
	}

	ai_nodepointer = i;
	CG_Printf("AiNode-File: %s: %i nodes parsed\n", filename, ai_nodepointer);
}

static void CG_SetAiNode_f(void){

	if(!cg_cheats)
		return;

	if(ai_nodepointer > MAX_AINODES){
		CG_Printf("%i nodes! No more nodes allowed!\n", ai_nodepointer-1);
		return;
	}

	// set the node
	VectorCopy(cg.snap->ps.origin, ai_nodes[ai_nodepointer]);
	ai_nodes[ai_nodepointer][2] -= 10;

	ai_nodepointer++;
}

static void CG_ResetAiNode_f(void){

	if(!cg_cheats)
		return;

	// just reset the pointer
	ai_nodepointer = 0;
}

static void  CG_DeleteAiNode_f(void){
	int node, i;

	if(!cg_cheats)
		return;


	// find the nearest node
	for(i=0, node=0; i < MAX_AINODES; i++){
		if(Distance(cg.snap->ps.origin, ai_nodes[i]) <
			Distance(cg.snap->ps.origin, ai_nodes[node]))
			node = i;
	}

	// now move all nodes 1 step forward
	for(i=node; i < ai_nodepointer-1; i++){
		VectorCopy(ai_nodes[i+1], ai_nodes[i]);
	}
	ai_nodepointer--;
}

static void CG_PrintAiNode_f(void){
	int i;

	if(!cg_cheats)
		return;

	CG_Printf("-------------------\n");
	for(i=0; i < ai_nodepointer; i++){
		CG_Printf("%i. node: %f %f %f\n", i+1, ai_nodes[i][0],
			ai_nodes[i][1], ai_nodes[i][2]);
	}
	if(!ai_nodepointer)
		CG_Printf("no nodes.\n");
	CG_Printf("-------------------\n");
}
#endif

typedef struct {
	char	*cmd;
	void	(*function)(void);
} consoleCommand_t;

static consoleCommand_t	commands[] = {
	{ "testgun", CG_TestGun_f },
	{ "testmodel", CG_TestModel_f },
	{ "nextframe", CG_TestModelNextFrame_f },
	{ "prevframe", CG_TestModelPrevFrame_f },
	{ "nextskin", CG_TestModelNextSkin_f },
	{ "prevskin", CG_TestModelPrevSkin_f },
	{ "viewpos", CG_Viewpos_f },
	{ "+scores", CG_ScoresDown_f },
	{ "-scores", CG_ScoresUp_f },
#ifndef SMOKINGUNS
	{ "+zoom", CG_ZoomDown_f },
	{ "-zoom", CG_ZoomUp_f },
#endif
	{ "sizeup", CG_SizeUp_f },
	{ "sizedown", CG_SizeDown_f },
	{ "weapnext", CG_NextWeapon_f },
	{ "weapprev", CG_PrevWeapon_f },
	{ "weapon", CG_Weapon_f },
#ifdef SMOKINGUNS
	{ "lastusedweapon", CG_LastUsedWeapon_f },
#endif
	{ "tcmd", CG_TargetCommand_f },
	{ "tell_target", CG_TellTarget_f },
	{ "tell_attacker", CG_TellAttacker_f },
#ifdef SMOKINGUNS
	{ "vtell_attacker", CG_VoiceTellAttacker_f },
	{ "loadhud", CG_LoadHud_f },
	{ "nextTeamMember", CG_NextTeamMember_f },
	{ "prevTeamMember", CG_PrevTeamMember_f },
	{ "nextOrder", CG_NextOrder_f },
	{ "confirmOrder", CG_ConfirmOrder_f },
	{ "denyOrder", CG_DenyOrder_f },
	{ "taskOffense", CG_TaskOffense_f },
	{ "taskDefense", CG_TaskDefense_f },
	{ "taskPatrol", CG_TaskPatrol_f },
	{ "taskCamp", CG_TaskCamp_f },
	{ "taskFollow", CG_TaskFollow_f },
	{ "taskRetrieve", CG_TaskRetrieve_f },
	{ "taskEscort", CG_TaskEscort_f },
	{ "taskSuicide", CG_TaskSuicide_f },
	{ "taskOwnFlag", CG_TaskOwnFlag_f },
	{ "tauntKillInsult", CG_TauntKillInsult_f },
	{ "tauntPraise", CG_TauntPraise_f },
	{ "tauntTaunt", CG_TauntTaunt_f },
	{ "tauntDeathInsult", CG_TauntDeathInsult_f },
	{ "tauntGauntlet", CG_TauntGauntlet_f },
	{ "spWin", CG_spWin_f },
	{ "spLose", CG_spLose_f },
	{ "scoresDown", CG_scrollScoresDown_f },
	{ "scoresUp", CG_scrollScoresUp_f },
#endif
#ifdef SMOKINGUNS
	//{ "+scope", CG_ScopeUp },
	//{ "-scope", CG_ScopeDown },
	{ "-angle", CG_AngleDecrease_f },
	{ "+angle", CG_AngleIncrease_f },
	{ "coords", CG_ShowCoords_f },
	{ "node_set", CG_SetAiNode_f },
	{ "node_reset", CG_ResetAiNode_f},
	{ "node_delete", CG_DeleteAiNode_f},
	{ "node_print", CG_PrintAiNode_f},
	{ "node_savefile", CG_SaveFileAiNode_f},
	{ "node_openfile", CG_OpenFileAiNode_f},
	{ "wq_buy", CG_BuyMenu },
#endif
	{ "startOrbit", CG_StartOrbit_f },
	//{ "camera", CG_Camera_f },
	{ "loaddeferred", CG_LoadDeferredPlayers }	
};


/*
=================
CG_ConsoleCommand

The string has been tokenized and can be retrieved with
Cmd_Argc() / Cmd_Argv()
=================
*/
qboolean CG_ConsoleCommand( void ) {
	const char	*cmd;
	int		i;

	cmd = CG_Argv(0);

	for ( i = 0 ; i < ARRAY_LEN( commands ) ; i++ ) {
		if ( !Q_stricmp( cmd, commands[i].cmd ) ) {
			commands[i].function();
			return qtrue;
		}
	}

	return qfalse;
}


/*
=================
CG_InitConsoleCommands

Let the client system know about all of our commands
so it can perform tab completion
=================
*/
void CG_InitConsoleCommands( void ) {
	int		i;

	for ( i = 0 ; i < ARRAY_LEN( commands ) ; i++ ) {
		trap_AddCommand( commands[i].cmd );
	}

	//
	// the game server will interpret these commands, which will be automatically
	// forwarded to the server after they are not recognized locally
	//
	trap_AddCommand ("kill");
	trap_AddCommand ("say");
	trap_AddCommand ("say_team");
	trap_AddCommand ("tell");
#ifdef SMOKINGUNS
	trap_AddCommand ("vsay");
	trap_AddCommand ("vsay_team");
	trap_AddCommand ("vtell");
	trap_AddCommand ("vtaunt");
	trap_AddCommand ("vosay");
	trap_AddCommand ("vosay_team");
	trap_AddCommand ("votell");
#endif
	trap_AddCommand ("give");
	trap_AddCommand ("god");
	trap_AddCommand ("notarget");
	trap_AddCommand ("noclip");
	trap_AddCommand ("where");
	trap_AddCommand ("team");
	trap_AddCommand ("follow");
	trap_AddCommand ("follownext");
	trap_AddCommand ("followprev");
	trap_AddCommand ("levelshot");
	trap_AddCommand ("addbot");
#ifdef SMOKINGUNS
	trap_AddCommand ("kickbots");
#endif
	trap_AddCommand ("setviewpos");
	trap_AddCommand ("callvote");
	trap_AddCommand ("vote");
	trap_AddCommand ("callteamvote");
	trap_AddCommand ("teamvote");
	trap_AddCommand ("stats");
	trap_AddCommand ("teamtask");
	trap_AddCommand ("loaddefered");	// spelled wrong, but not changing for demo

#ifdef SMOKINGUNS
	//new console commands
	trap_AddCommand ("dropweapon");
	trap_AddCommand ("cg_buy");
	trap_AddCommand ("buy");
#endif
}
