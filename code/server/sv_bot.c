/*
===========================================================================
Copyright (C) 1999-2005 Id Software, Inc.
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
// sv_bot.c

#include "server.h"
#include "../botlib/botlib.h"

typedef struct bot_debugpoly_s
{
	int inuse;
	int color;
	int numPoints;
	vec3_t points[128];
} bot_debugpoly_t;

static bot_debugpoly_t *debugpolygons;
int bot_maxdebugpolys;

extern botlib_export_t	*botlib_export;
int	bot_enable;


/*
==================
SV_BotAllocateClient
==================
*/
int SV_BotAllocateClient(void) {
	int			i;
	client_t	*cl;

	// find a client slot
	for ( i = 0, cl = svs.clients; i < sv_maxclients->integer; i++, cl++ ) {
		if ( cl->state == CS_FREE ) {
			break;
		}
	}

	if ( i == sv_maxclients->integer ) {
		return -1;
	}

	cl->gentity = SV_GentityNum( i );
	cl->gentity->s.number = i;
	cl->state = CS_ACTIVE;
	cl->lastPacketTime = svs.time;
	cl->netchan.remoteAddress.type = NA_BOT;
	cl->rate = 16384;

	return i;
}

/*
==================
SV_BotFreeClient
==================
*/
void SV_BotFreeClient( int clientNum ) {
	client_t	*cl;

	if ( clientNum < 0 || clientNum >= sv_maxclients->integer ) {
		Com_Error( ERR_DROP, "SV_BotFreeClient: bad clientNum: %i", clientNum );
	}
	cl = &svs.clients[clientNum];
	cl->state = CS_FREE;
	cl->name[0] = 0;
	if ( cl->gentity ) {
		cl->gentity->r.svFlags &= ~SVF_BOT;
	}
}

/*
==================
BotDrawDebugPolygons
==================
*/
void BotDrawDebugPolygons(void (*drawPoly)(int color, int numPoints, float *points), int value) {
	static cvar_t *bot_debug, *bot_groundonly, *bot_reachability, *bot_highlightarea;
	bot_debugpoly_t *poly;
	int i, parm0;

	if (!debugpolygons)
		return;
	//bot debugging
	if (!bot_debug) bot_debug = Cvar_Get("bot_debug", "0", 0);
	//
	if (bot_enable && bot_debug->integer) {
		//show reachabilities
		if (!bot_reachability) bot_reachability = Cvar_Get("bot_reachability", "0", 0);
		//show ground faces only
		if (!bot_groundonly) bot_groundonly = Cvar_Get("bot_groundonly", "1", 0);
		//get the hightlight area
		if (!bot_highlightarea) bot_highlightarea = Cvar_Get("bot_highlightarea", "0", 0);
		//
		parm0 = 0;
		if (svs.clients[0].lastUsercmd.buttons & BUTTON_ATTACK) parm0 |= 1;
		if (bot_reachability->integer) parm0 |= 2;
		if (bot_groundonly->integer) parm0 |= 4;
		botlib_export->BotLibVarSet("bot_highlightarea", bot_highlightarea->string);
		botlib_export->Test(parm0, NULL, svs.clients[0].gentity->r.currentOrigin,
			svs.clients[0].gentity->r.currentAngles);
	} //end if
	//draw all debug polys
	for (i = 0; i < bot_maxdebugpolys; i++) {
		poly = &debugpolygons[i];
		if (!poly->inuse) continue;
		drawPoly(poly->color, poly->numPoints, (float *) poly->points);
		//Com_Printf("poly %i, numpoints = %d\n", i, poly->numPoints);
	}
}

/*
==================
BotImport_Print
==================
*/
static __attribute__ ((format (printf, 2, 3))) void QDECL BotImport_Print(int type, char *fmt, ...)
{
	char str[2048];
	va_list ap;

	va_start(ap, fmt);
	Q_vsnprintf(str, sizeof(str), fmt, ap);
	va_end(ap);

	switch(type) {
		case PRT_MESSAGE: {
			Com_Printf("%s", str);
			break;
		}
		case PRT_WARNING: {
			Com_Printf(S_COLOR_YELLOW "Warning: %s", str);
			break;
		}
		case PRT_ERROR: {
			Com_Printf(S_COLOR_RED "Error: %s", str);
			break;
		}
		case PRT_FATAL: {
			Com_Printf(S_COLOR_RED "Fatal: %s", str);
			break;
		}
		case PRT_EXIT: {
			Com_Error(ERR_DROP, S_COLOR_RED "Exit: %s", str);
			break;
		}
		default: {
			Com_Printf("unknown print type\n");
			break;
		}
	}
}

/*
==================
BotImport_Trace
==================
*/
static void BotImport_Trace(bsp_trace_t *bsptrace, vec3_t start, vec3_t mins, vec3_t maxs, vec3_t end, int passent, int contentmask) {
	trace_t trace;

	SV_Trace(&trace, start, mins, maxs, end, passent, contentmask, qfalse);
	//copy the trace information
	bsptrace->allsolid = trace.allsolid;
	bsptrace->startsolid = trace.startsolid;
	bsptrace->fraction = trace.fraction;
	VectorCopy(trace.endpos, bsptrace->endpos);
	bsptrace->plane.dist = trace.plane.dist;
	VectorCopy(trace.plane.normal, bsptrace->plane.normal);
	bsptrace->plane.signbits = trace.plane.signbits;
	bsptrace->plane.type = trace.plane.type;
	bsptrace->surface.value = trace.surfaceFlags;
	bsptrace->ent = trace.entityNum;
	bsptrace->exp_dist = 0;
	bsptrace->sidenum = 0;
	bsptrace->contents = 0;
}

/*
==================
BotImport_EntityTrace
==================
*/
static void BotImport_EntityTrace(bsp_trace_t *bsptrace, vec3_t start, vec3_t mins, vec3_t maxs, vec3_t end, int entnum, int contentmask) {
	trace_t trace;

	SV_ClipToEntity(&trace, start, mins, maxs, end, entnum, contentmask, qfalse);
	//copy the trace information
	bsptrace->allsolid = trace.allsolid;
	bsptrace->startsolid = trace.startsolid;
	bsptrace->fraction = trace.fraction;
	VectorCopy(trace.endpos, bsptrace->endpos);
	bsptrace->plane.dist = trace.plane.dist;
	VectorCopy(trace.plane.normal, bsptrace->plane.normal);
	bsptrace->plane.signbits = trace.plane.signbits;
	bsptrace->plane.type = trace.plane.type;
	bsptrace->surface.value = trace.surfaceFlags;
	bsptrace->ent = trace.entityNum;
	bsptrace->exp_dist = 0;
	bsptrace->sidenum = 0;
	bsptrace->contents = 0;
}


/*
==================
BotImport_PointContents
==================
*/
static int BotImport_PointContents(vec3_t point) {
	return SV_PointContents(point, -1);
}

/*
==================
BotImport_inPVS
==================
*/
static int BotImport_inPVS(vec3_t p1, vec3_t p2) {
	return SV_inPVS (p1, p2);
}

/*
==================
BotImport_BSPEntityData
==================
*/
static char *BotImport_BSPEntityData(void) {
	return CM_EntityString();
}

/*
==================
BotImport_BSPModelMinsMaxsOrigin
==================
*/
static void BotImport_BSPModelMinsMaxsOrigin(int modelnum, vec3_t angles, vec3_t outmins, vec3_t outmaxs, vec3_t origin) {
	clipHandle_t h;
	vec3_t mins, maxs;
	float max;
	int	i;

	h = CM_InlineModel(modelnum);
	CM_ModelBounds(h, mins, maxs);
	//if the model is rotated
	if ((angles[0] || angles[1] || angles[2])) {
		// expand for rotation

		max = RadiusFromBounds(mins, maxs);
		for (i = 0; i < 3; i++) {
			mins[i] = -max;
			maxs[i] = max;
		}
	}
	if (outmins) VectorCopy(mins, outmins);
	if (outmaxs) VectorCopy(maxs, outmaxs);
	if (origin) VectorClear(origin);
}

/*
==================
BotImport_GetMemory
==================
*/
static void *BotImport_GetMemory(int size) {
	void *ptr;

	ptr = Z_TagMalloc( size, TAG_BOTLIB );
	return ptr;
}

/*
==================
BotImport_FreeMemory
==================
*/
static void BotImport_FreeMemory(void *ptr) {
	Z_Free(ptr);
}

/*
=================
BotImport_HunkAlloc
=================
*/
static void *BotImport_HunkAlloc( int size ) {
	if( Hunk_CheckMark() ) {
		Com_Error( ERR_DROP, "SV_Bot_HunkAlloc: Alloc with marks already set" );
	}
	return Hunk_Alloc( size, h_high );
}

/*
==================
BotImport_DebugPolygonCreate
==================
*/
int BotImport_DebugPolygonCreate(int color, int numPoints, vec3_t *points) {
	bot_debugpoly_t *poly;
	int i;

	if (!debugpolygons)
		return 0;

	for (i = 1; i < bot_maxdebugpolys; i++) 	{
		if (!debugpolygons[i].inuse)
			break;
	}
	if (i >= bot_maxdebugpolys)
		return 0;
	poly = &debugpolygons[i];
	poly->inuse = qtrue;
	poly->color = color;
	poly->numPoints = numPoints;
	Com_Memcpy(poly->points, points, numPoints * sizeof(vec3_t));
	//
	return i;
}

/*
==================
BotImport_DebugPolygonShow
==================
*/
static void BotImport_DebugPolygonShow(int id, int color, int numPoints, vec3_t *points) {
	bot_debugpoly_t *poly;

	if (!debugpolygons) return;
	poly = &debugpolygons[id];
	poly->inuse = qtrue;
	poly->color = color;
	poly->numPoints = numPoints;
	Com_Memcpy(poly->points, points, numPoints * sizeof(vec3_t));
}

/*
==================
BotImport_DebugPolygonDelete
==================
*/
void BotImport_DebugPolygonDelete(int id)
{
	if (!debugpolygons) return;
	debugpolygons[id].inuse = qfalse;
}

/*
==================
BotImport_DebugLineCreate
==================
*/
static int BotImport_DebugLineCreate(void) {
	vec3_t points[1];
	return BotImport_DebugPolygonCreate(0, 0, points);
}

/*
==================
BotImport_DebugLineDelete
==================
*/
static void BotImport_DebugLineDelete(int line) {
	BotImport_DebugPolygonDelete(line);
}

/*
==================
BotImport_DebugLineShow
==================
*/
static void BotImport_DebugLineShow(int line, vec3_t start, vec3_t end, int color) {
	vec3_t points[4], dir, cross, up = {0, 0, 1};
	float dot;

	VectorCopy(start, points[0]);
	VectorCopy(start, points[1]);
	//points[1][2] -= 2;
	VectorCopy(end, points[2]);
	//points[2][2] -= 2;
	VectorCopy(end, points[3]);


	VectorSubtract(end, start, dir);
	VectorNormalize(dir);
	dot = DotProduct(dir, up);
	if (dot > 0.99 || dot < -0.99) VectorSet(cross, 1, 0, 0);
	else CrossProduct(dir, up, cross);

	VectorNormalize(cross);

	VectorMA(points[0], 2, cross, points[0]);
	VectorMA(points[1], -2, cross, points[1]);
	VectorMA(points[2], -2, cross, points[2]);
	VectorMA(points[3], 2, cross, points[3]);

	BotImport_DebugPolygonShow(line, color, 4, points);
}

/*
==================
SV_BotClientCommand
==================
*/
static void BotClientCommand( int client, char *command ) {
	SV_ExecuteClientCommand( &svs.clients[client], command, qtrue );
}

/*
==================
SV_BotFrame
==================
*/
void SV_BotFrame( int time ) {
	if (!bot_enable) return;
	//NOTE: maybe the game is already shutdown
	if (!gvm) return;
	VM_Call( gvm, BOTAI_START_FRAME, time );
}

/*
===============
SV_BotLibSetup
===============
*/
int SV_BotLibSetup( void ) {
	if (!bot_enable) {
		return 0;
	}

	if ( !botlib_export ) {
		Com_Printf( S_COLOR_RED "Error: SV_BotLibSetup without SV_BotInitBotLib\n" );
		return -1;
	}

	botlib_export->BotLibVarSet( "basegame", com_basegame->string );

	return botlib_export->BotLibSetup();
}

/*
===============
SV_ShutdownBotLib

Called when either the entire server is being killed, or
it is changing to a different game directory.
===============
*/
int SV_BotLibShutdown( void ) {

	if ( !botlib_export ) {
		return -1;
	}

	return botlib_export->BotLibShutdown();
}

/*
==================
SV_BotInitCvars
==================
*/
void SV_BotInitCvars(void) {

	Cvar_Get("bot_enable", "1", 0);						//enable the bot
	Cvar_Get("bot_developer", "0", CVAR_CHEAT);			//bot developer mode
	Cvar_Get("bot_debug", "0", CVAR_CHEAT);				//enable bot debugging
	Cvar_Get("bot_maxdebugpolys", "2", 0);				//maximum number of debug polys
	Cvar_Get("bot_groundonly", "1", 0);					//only show ground faces of areas
	Cvar_Get("bot_reachability", "0", 0);				//show all reachabilities to other areas
	Cvar_Get("bot_visualizejumppads", "0", CVAR_CHEAT);	//show jumppads
	Cvar_Get("bot_forceclustering", "0", 0);			//force cluster calculations
	Cvar_Get("bot_forcereachability", "0", 0);			//force reachability calculations
	Cvar_Get("bot_forcewrite", "0", 0);					//force writing aas file
	Cvar_Get("bot_aasoptimize", "0", 0);				//no aas file optimisation
	Cvar_Get("bot_saveroutingcache", "0", 0);			//save routing cache
	Cvar_Get("bot_thinktime", "100", CVAR_CHEAT);		//msec the bots thinks
	Cvar_Get("bot_reloadcharacters", "0", 0);			//reload the bot characters each time
	Cvar_Get("bot_testichat", "0", 0);					//test ichats
	Cvar_Get("bot_testrchat", "0", 0);					//test rchats
	Cvar_Get("bot_testsolid", "0", CVAR_CHEAT);			//test for solid areas
	Cvar_Get("bot_testclusters", "0", CVAR_CHEAT);		//test the AAS clusters
	Cvar_Get("bot_fastchat", "0", 0);					//fast chatting bots
	Cvar_Get("bot_nochat", "0", 0);						//disable chats
	Cvar_Get("bot_pause", "0", CVAR_CHEAT);				//pause the bots thinking
	Cvar_Get("bot_report", "0", CVAR_CHEAT);			//get a full report in ctf
	Cvar_Get("bot_grapple", "0", 0);					//enable grapple
	Cvar_Get("bot_rocketjump", "1", 0);					//enable rocket jumping
	Cvar_Get("bot_challenge", "0", 0);					//challenging bot
	Cvar_Get("bot_minplayers", "0", 0);					//minimum players in a team or the game
	Cvar_Get("bot_interbreedchar", "", CVAR_CHEAT);		//bot character used for interbreeding
	Cvar_Get("bot_interbreedbots", "10", CVAR_CHEAT);	//number of bots used for interbreeding
	Cvar_Get("bot_interbreedcycle", "20", CVAR_CHEAT);	//bot interbreeding cycle
	Cvar_Get("bot_interbreedwrite", "", CVAR_CHEAT);	//write interbreeded bots to this file
}

/*
==================
SV_BotInitBotLib
==================
*/
void SV_BotInitBotLib(void) {
	botlib_import_t	botlib_import;

	if (debugpolygons) Z_Free(debugpolygons);
	bot_maxdebugpolys = Cvar_VariableIntegerValue("bot_maxdebugpolys");
	debugpolygons = Z_Malloc(sizeof(bot_debugpoly_t) * bot_maxdebugpolys);

	botlib_import.Print = BotImport_Print;
	botlib_import.Trace = BotImport_Trace;
	botlib_import.EntityTrace = BotImport_EntityTrace;
	botlib_import.PointContents = BotImport_PointContents;
	botlib_import.inPVS = BotImport_inPVS;
	botlib_import.BSPEntityData = BotImport_BSPEntityData;
	botlib_import.BSPModelMinsMaxsOrigin = BotImport_BSPModelMinsMaxsOrigin;
	botlib_import.BotClientCommand = BotClientCommand;

	//memory management
	botlib_import.GetMemory = BotImport_GetMemory;
	botlib_import.FreeMemory = BotImport_FreeMemory;
	botlib_import.AvailableMemory = Z_AvailableMemory;
	botlib_import.HunkAlloc = BotImport_HunkAlloc;

	// file system access
	botlib_import.FS_FOpenFile = FS_FOpenFileByMode;
	botlib_import.FS_Read = FS_Read2;
	botlib_import.FS_Write = FS_Write;
	botlib_import.FS_FCloseFile = FS_FCloseFile;
	botlib_import.FS_Seek = FS_Seek;

	//debug lines
	botlib_import.DebugLineCreate = BotImport_DebugLineCreate;
	botlib_import.DebugLineDelete = BotImport_DebugLineDelete;
	botlib_import.DebugLineShow = BotImport_DebugLineShow;

	//debug polygons
	botlib_import.DebugPolygonCreate = BotImport_DebugPolygonCreate;
	botlib_import.DebugPolygonDelete = BotImport_DebugPolygonDelete;

	botlib_export = (botlib_export_t *)GetBotLibAPI( BOTLIB_API_VERSION, &botlib_import );
	assert(botlib_export); 	// somehow we end up with a zero import.
}


//
//  * * * BOT AI CODE IS BELOW THIS POINT * * *
//

/*
==================
SV_BotGetConsoleMessage
==================
*/
int SV_BotGetConsoleMessage( int client, char *buf, int size )
{
	client_t	*cl;
	int			index;

	cl = &svs.clients[client];
	cl->lastPacketTime = svs.time;

	if ( cl->reliableAcknowledge == cl->reliableSequence ) {
		return qfalse;
	}

	cl->reliableAcknowledge++;
	index = cl->reliableAcknowledge & ( MAX_RELIABLE_COMMANDS - 1 );

	if ( !cl->reliableCommands[index][0] ) {
		return qfalse;
	}

	Q_strncpyz( buf, cl->reliableCommands[index], size );
	return qtrue;
}

#if 0
/*
==================
EntityInPVS
==================
*/
int EntityInPVS( int client, int entityNum ) {
	client_t			*cl;
	clientSnapshot_t	*frame;
	int					i;

	cl = &svs.clients[client];
	frame = &cl->frames[cl->netchan.outgoingSequence & PACKET_MASK];
	for ( i = 0; i < frame->num_entities; i++ )	{
		if ( svs.snapshotEntities[(frame->first_entity + i) % svs.numSnapshotEntities].number == entityNum ) {
			return qtrue;
		}
	}
	return qfalse;
}
#endif

/*
==================
SV_BotGetSnapshotEntity
==================
*/
int SV_BotGetSnapshotEntity( int client, int sequence ) {
	client_t			*cl;
	clientSnapshot_t	*frame;

	cl = &svs.clients[client];
	frame = &cl->frames[cl->netchan.outgoingSequence & PACKET_MASK];
	if (sequence < 0 || sequence >= frame->num_entities) {
		return -1;
	}
	return svs.snapshotEntities[(frame->first_entity + sequence) % svs.numSnapshotEntities].number;
}


/*
==================
InFieldOfVision
==================
InFieldOfVision(vec3_t viewer.angles, float fov, vec3_t enemy.angles, int viewer.clientnum)
*/
qboolean InFieldOfVision(vec3_t viewangles, float fov, vec3_t angles, int client)
{
	int i;
	float diff, angle;
//	int event = g_entities[client].s.event & ~EV_EVENT_BITS;
//
//	switch(event){
//	case EV_FOOTSTEP:
//	case EV_FOOTSTEP_METAL:
//	case EV_FOOTSTEP_CLOTH:
//	case EV_FOOTSTEP_GRASS:
//	case EV_FOOTSTEP_SNOW:
//	case EV_FOOTSTEP_SAND:
//	case EV_FOOTSPLASH:
//	case EV_FOOTWADE:
//	case EV_SWIM:
//	case EV_FALL_SHORT:
//	case EV_FALL_MEDIUM:
//	case EV_FALL_FAR:
//	case EV_STEP_4:
//	case EV_STEP_8:
//	case EV_STEP_12:
//	case EV_STEP_16:
//	case EV_JUMP_PAD:
//	case EV_JUMP:
//	case EV_TAUNT:
//	case EV_WATER_TOUCH:
//	case EV_WATER_LEAVE:
//	case EV_WATER_UNDER:
//	case EV_WATER_CLEAR:
//	case EV_ITEM_PICKUP:
////	case EV_GLOBAL_ITEM_PICKUP:
//	case EV_NOAMMO:
//	case EV_NOAMMO_CLIP:
//	case EV_CHANGE_WEAPON:
//	case EV_FIRE_WEAPON:
//	case EV_FIRE_WEAPON_DELAY:
//	case EV_FIRE_WEAPON2:
//	case EV_ALT_FIRE_WEAPON:
//	case EV_BUILD_TURRET:
//		return qtrue;
//		break;
//	}

	for (i = 0; i < 2; i++) {
		angle = AngleMod(viewangles[i]);
		angles[i] = AngleMod(angles[i]);
		diff = angles[i] - angle;

		if (angles[i] > angle) {
			if (diff > 180.0) diff -= 360.0;
		}
		else {
			if (diff < -180.0) diff += 360.0;
		}

		if (diff > 0) {
			if (diff > fov * 1.1f) return qfalse; // fov * 0.5
		}
		else {
			if (diff < -fov * 1.1f) return qfalse;
		}
	}
	return qtrue;
}

const int MEMORY = 210;
#define offsetrandom(MIN,MAX) (((double)(rand() + 0.5) / ((double)RAND_MAX + 1)) * ((MAX)-(MIN)) + (MIN))
/*
 * Tests whether the player entity ent is visible from the point origin.
 */
qboolean SV_IsPlayerVisibleFromPoint( vec3_t origin, clientSnapshot_t *frame, sharedEntity_t *ent, vec3_t diff) {
	int      i,contents_mask,goal_ent,viewer_clnum,ent_clnum,tries;
	trace_t   tr;
	vec3_t   start,end,dir,entangles,angles,temp,forward;
	sharedEntity_t *viewer_ent;
	client_t *viewer_cl;
//	client_t *ent_cl;
	playerState_t *viewer_ps, *ent_ps;
	float pitch;

	viewer_clnum = frame->ps.clientNum; // get the client number of the viewer
	ent_clnum = ent->s.clientNum; // get the client number of the other player

	if (viewer_clnum == ent_clnum) { // in case the viewer is the player entity
		return qtrue; // we don't need to hide us from ourselves
	}

	viewer_ps = &frame->ps;
	ent_ps = SV_GameClientNum(ent_clnum);

	if (viewer_ps->pm_type != PM_NORMAL) { // if the viewer is dead or spectating
		return qtrue; // let every entity be visible
	}

	if (ent_ps->pm_type != PM_NORMAL || (ent->s.weapon == WP_NONE)) { // if the player entity is dead or spectating
		return qtrue;
	}

	viewer_cl = svs.clients+viewer_clnum; // get the client of the viewer
//	ent_cl = svs.clients+ent_clnum; // get the client of the other player

//	if (viewer_clnum > ent_clnum) { // if viewer_clnum > ent_clnum, we have already tested whether ent_clnum is able to see viewer_clnum.
//		if (ent_cl->tracetimer[viewer_clnum] > sv.time) return qtrue; // and we could assume symmetry of SV_IsPlayerVisibleFromPoint
//	}

	if (viewer_cl->tracetimer[ent_clnum] > sv.time+MEMORY+10) { // if the sv.time has been reset
		viewer_cl->tracetimer[ent_clnum] = sv.time; // reset the tracetimer
	} else if (viewer_cl->tracetimer[ent_clnum] > (sv.time+MEMORY-10)) { // if we have recently seen this entity, we are lazy and assume it is still visible
		// Com_Printf(va("client: %i, seen: %i\n", ent_clnum, viewer_cl->tracetimer[ent_clnum]));
		return qtrue;
	}

	goal_ent = SV_NumForGentity(ent); // this might always be the same as ent_clnum
	viewer_ent = SV_GentityNum(viewer_clnum);
	contents_mask = CONTENTS_SOLID;// |CONTENTS_BODY will work for doors, but also for windows  |CONTENTS_PLAYERCLIP|CONTENTS_SOLID|CONTENTS_MOVER|CONTENTS_PLAYERCLIP

//	if (seen->v.movetype == MOVETYPE_PUSH ) { //don't cull doors and plats :(
//		return false;
//	}

//	if (sv_antiwallhack.value == 1)    //1 only check player models, 2 = check all ents
//	if (strcmp(pr_strings + seen->v.classname, "player"))
//	return qfalse;

	// get camera origin (according to \cg_drawdebug 1)
	start[0] = origin[0];
	start[1] = origin[1];
	start[2] = origin[2]+3.0f;
	VectorCopy(viewer_ps->viewangles, angles);
	AnglesNormalize180(angles);
	pitch = angles[PITCH];
	angles[PITCH] = 0;
	angles[ROLL] = 0;
	AngleVectors(angles, forward, NULL, NULL);
	VectorScale(forward, (pitch/3.5f), temp);
	VectorAdd( start, temp, start);

	// if there is sufficient distance between viewer and player entity, check if player entity is within viewer's field of vision
	VectorSubtract(ent->r.currentOrigin, start, dir);
//	VectorAdd(ent->r.currentOrigin,dir,diff);// fill diff
	VectorCopy(viewer_ent->s.pos.trBase,diff);// fill diff
	vectoangles(dir, entangles);
	dir[2]=0; // pretend, players are on the same level (the height should no be taken into account)
	if (VectorLength(dir) > 1024) {// if it is not within close range (x,y-wise, not z-wise)
		if (!InFieldOfVision(viewer_ps->viewangles, 60.f, entangles, ent_clnum)) {// If the player entity is not in the field of vision of the viewer
//			 Com_Printf( va("behind: %i  vorg: %f,%f,%f  vang: %f,%f,%f  eorg: %f,%f,%f  dir: %f,%f,%f  eang: %f,%f,%f  ent: %i\n", viewer_clnum,origin[0],origin[1],origin[2],viewer_ps->viewangles[0],viewer_ps->viewangles[1],viewer_ps->viewangles[2],ent->r.currentOrigin[0],ent->r.currentOrigin[1],ent->r.currentOrigin[2],dir[0],dir[1],dir[2],entangles[0],entangles[1],entangles[2],ent_clnum));
			return qtrue; // if the player entity is behind the viewer, abstain from any computations (and transmit the entity to hear sounds)
//		} else {
//			 Com_Printf( va("front: %i  vorg: %f,%f,%f  vang: %f,%f,%f  eorg: %f,%f,%f  dir: %f,%f,%f  eang: %f,%f,%f  ent: %i\n", viewer_clnum,origin[0],origin[1],origin[2],viewer_ps->viewangles[0],viewer_ps->viewangles[1],viewer_ps->viewangles[2],ent->r.currentOrigin[0],ent->r.currentOrigin[1],ent->r.currentOrigin[2],dir[0],dir[1],dir[2],entangles[0],entangles[1],entangles[2],ent_clnum));
		}
	}

	// aim straight at the head of the entity from our eyes
	end[0] = ent->r.currentOrigin[0];
	end[1] = ent->r.currentOrigin[1];
	end[2] = ent->r.currentOrigin[2]+ent->r.maxs[2];// "+3.0f" doesn't do it. "+ent->r.maxs[2]" is at the top of the BBox
	VectorCopy(ent_ps->viewangles, angles);
	AnglesNormalize180(angles);
	pitch = angles[PITCH];
	angles[PITCH] = 0;
	angles[ROLL] = 0;
	AngleVectors(angles, forward, NULL, NULL);
	VectorScale(forward, (pitch/3.5f), temp);
	VectorAdd( end, temp, end);



	memset (&tr, 0, sizeof(tr));
	tr.fraction = 1;

	// check the head
	SV_Trace(&tr, start, NULL, NULL, end, viewer_clnum, contents_mask, qfalse);
	// Com_Printf(va("client: %i, trace: %f\n", ent->s.clientNum, tr.fraction));
	if (tr.fraction == 1 || tr.entityNum==goal_ent) {// tr.fraction == 1 || if there is a line of sight to the entity
		viewer_cl->tracetimer[ent_clnum] = sv.time + MEMORY;// update the trace timer so the entity will be visible the next 200 time units
		return qtrue;// and signal the entity is visible
	}
	// Com_Printf(va("origin(%f %f %f) min(%f %f %f) max(%f %f %f)\n", start[0], start[1], start[2], mins[0],mins[1],mins[2],maxs[0],maxs[1],maxs[2]));

	// check the last good offset
	VectorAdd(ent->r.currentOrigin,viewer_cl->lasttrace[ent_clnum],end);
	SV_Trace(&tr, start, NULL, NULL, end, viewer_clnum, contents_mask, qfalse);
	// Com_Printf(va("client: %i, trace: %f\n", ent->s.clientNum, tr.fraction));
	if (tr.fraction == 1 || tr.entityNum==goal_ent) {// tr.fraction == 1 || if there is a line of sight to the entity
		viewer_cl->tracetimer[ent_clnum] = sv.time + MEMORY;// update the trace timer so the entity will be visible the next 200 time units
		return qtrue;// and signal the entity is visible
	}

	// random
	tries = (viewer_cl->tracetimer[ent_clnum]+(MEMORY*20) > sv.time)?8:2;// if we have seen an entity recently, we try hard to locate it again
	for (i=0; i<tries; i++) {// Even if the head is not visible, other body parts might be. Let's check a few randomly selected points
		end[0] = ent->r.currentOrigin[0] + offsetrandom(ent->r.mins[0], ent->r.maxs[0]);
		end[1] = ent->r.currentOrigin[1] + offsetrandom(ent->r.mins[1], ent->r.maxs[1]);
		end[2] = ent->r.currentOrigin[2] + offsetrandom(ent->r.mins[2], ent->r.maxs[2]+3.f);

		SV_Trace(&tr, start, NULL, NULL, end, viewer_clnum, contents_mask, qfalse);
		if (tr.fraction == 1 || tr.entityNum==goal_ent) {//  if there is a line of sight to the entity
			// Com_Printf(va("found ent in %i hits\n", i));
			viewer_cl->tracetimer[ent_clnum] = sv.time + MEMORY;// update the trace timer so the entity will be visible the next 200 time units
			VectorSubtract(end, ent->r.currentOrigin, viewer_cl->lasttrace[ent_clnum]);// remember the offset
			return qtrue;// and signal the entity is visible
		}
	}

	viewer_cl->lasttrace[ent_clnum][0] = offsetrandom(ent->r.mins[0], ent->r.maxs[0]);
	viewer_cl->lasttrace[ent_clnum][1] = offsetrandom(ent->r.mins[1], ent->r.maxs[1]);
	viewer_cl->lasttrace[ent_clnum][2] = offsetrandom(ent->r.mins[2], ent->r.maxs[2]+3.f);

	return (viewer_cl->tracetimer[ent_clnum] > sv.time);// returns true if the entity was visible within the last 200 time units
}

