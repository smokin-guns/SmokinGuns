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
// cg_players.c -- handle the media and animation for player entities
#include "cg_local.h"

char	*cg_customSoundNames[MAX_CUSTOM_SOUNDS] = {
#ifndef SMOKINGUNS
	"*death1.wav",
	"*death2.wav",
	"*death3.wav",
#endif
	"*jump1.wav",
	"*pain25_1.wav",
	"*pain50_1.wav",
	"*pain75_1.wav",
	"*pain100_1.wav",
	"*falling1.wav",
	"*gasp.wav",
	"*drown.wav",
	"*fall1.wav",
#ifndef SMOKINGUNS
	"*taunt.wav"
#endif
};



/*
================
CG_CustomSound

================
*/
sfxHandle_t	CG_CustomSound( int clientNum, const char *soundName ) {
	clientInfo_t *ci;
	int			i;

	if ( soundName[0] != '*' ) {
		return trap_S_RegisterSound( soundName, qfalse );
	}

	if ( clientNum < 0 || clientNum >= MAX_CLIENTS ) {
		clientNum = 0;
	}
	ci = &cgs.clientinfo[ clientNum ];

	for ( i = 0 ; i < MAX_CUSTOM_SOUNDS && cg_customSoundNames[i] ; i++ ) {
		if ( !strcmp( soundName, cg_customSoundNames[i] ) ) {
			return ci->sounds[i];
		}
	}

	CG_Error( "Unknown custom sound: %s", soundName );
	return 0;
}



/*
=============================================================================

CLIENT INFO

=============================================================================
*/

//map infos added by Spoon
#ifdef SMOKINGUNS
shaderInfo_t shaderInfo[MAX_BRUSHSIDES];
int	texInfoNum;
int	shaderInfoNum;

qboolean Keyword( char *key, char *token, const char* filename){
	if(Q_stricmp(key, token)){
		trap_Print( va( S_COLOR_RED "no \"%s\" in %s\n", key, filename ) );
		return qfalse;
	}
	return qtrue;
}

int trackInfoNum;
trackInfo_t	trackInfo[MAX_TRACKS];

/*
======================
CG_ParseMusicFile

Read length and path of present music track
======================
*/
qboolean CG_ParseMusicFile(void){
	int	len, i;//, j;
	fileHandle_t f;
	char buf[MAX_TEXINFOFILE];
	char **p, *ptr;
	char *token;

	len = trap_FS_FOpenFile( "music/music.cfg", &f, FS_READ );

	if ( !f ) {
		trap_Print( va( S_COLOR_RED "could not open music/music.cfg\n") );
		trap_FS_FCloseFile( f );
		return qfalse;
	}
	if ( len >= MAX_TEXINFOFILE ) {
		trap_Print( va( S_COLOR_RED "music file too large: music/music.cfg is %i, max allowed is %i\n", len, MAX_TEXINFOFILE ) );
		trap_FS_FCloseFile( f );
		return qfalse;
	}

	trap_FS_Read( buf, len, f );
	buf[len] = 0;
	trap_FS_FCloseFile( f );

	ptr = buf;
	p = &ptr;

	trackInfoNum = 0;

	//READ HEADER
	token = COM_ParseExt(p, qtrue);
	if (!token || token[0] == 0)
		return qfalse;

	if(!Keyword("MUSICFILE", token, "music/music.cfg"))
		return qfalse;

	//READ HEADER
	token = COM_ParseExt(p, qtrue);
	trackInfoNum = atoi(token);

	if(trackInfoNum > MAX_TRACKS){
		trap_Print( va( S_COLOR_RED "music-files: %i tracks found; max allowed %i tracks\n", trackInfoNum, MAX_TRACKS ) );
		return qfalse;
	}
	//parse tracks
	for(i=0;i<trackInfoNum;i++){

		token = COM_ParseExt(p, qtrue);
		Com_sprintf(trackInfo[i].path,sizeof(trackInfo[i].path), token);

		token = COM_ParseExt(p, qtrue);
		trackInfo[i].length = atoi(token);

		token = COM_ParseExt(p, qtrue);
		trackInfo[i].replays = atoi(token);

		//trap_Print( va( S_COLOR_RED "%i path: %s length: %i\n", i, trackInfo[i].path, trackInfo[i].length  ) );
	}
	return qtrue;
}

/*
======================
CG_ParseTexFile

Read texture information of a map using map.tex
======================
*/
qboolean CG_ParseTexFile(const char *filename){
	int	len, i;//, j;
	fileHandle_t f;
	char buf[MAX_TEXINFOFILE];
	char **p, *ptr;
	char *token;

	len = trap_FS_FOpenFile( filename, &f, FS_READ );
	if ( !f ) {
		trap_Print( va( S_COLOR_RED "tex file not found: %s\n", filename ) );
		trap_FS_FCloseFile( f );
		return qfalse;
	}
	if ( len >= MAX_TEXINFOFILE ) {
		trap_Print( va( S_COLOR_RED "tex file too large: %s is %i, max allowed is %i\n", filename, len, MAX_TEXINFOFILE ) );
		trap_FS_FCloseFile( f );
		return qfalse;
	}

	trap_FS_Read( buf, len, f );
	buf[len] = 0;
	trap_FS_FCloseFile( f );

	trap_Print( va( S_COLOR_WHITE "Client: Loading tex-file... %s\n", filename ) );

	ptr = buf;
	p = &ptr;

	texInfoNum = 0;

	//READ HEADER
	token = COM_ParseExt(p, qtrue);
	if (!token || token[0] == 0)
		return qtrue;

	if(!Keyword("TEXFILE", token, filename))
		return qfalse;

	/*token = COM_ParseExt(p, qtrue);
	texInfoNum = atoi(token);
	trap_Print( va( S_COLOR_RED "%i surfaces\n", texInfoNum ) );
	if(texInfoNum > MAX_BRUSHSIDES){
		trap_Print( va( S_COLOR_RED "Tex-file: %i brushsides max allowed %i brushsides\n", shaderInfoNum, MAX_BRUSHSIDES ) );
		return qfalse;
	}

	//parse the surfaces
	for(i=0;i<texInfoNum;i++) {
		token = COM_ParseExt(p, qtrue);
		if(!Keyword("{", token, filename))
			return qfalse;

		token = COM_ParseExt(p,qtrue);
		texInfo[i].shaderNum = atoi(token);
		trap_Print( va( S_COLOR_RED "%i\n", texInfo[i].shaderNum ) );

		texInfo[i].numpoints = 0;

		token = COM_ParseExt(p,qtrue);
		while(Q_stricmp("}", token)){
			for(j=0;j<3;j++){
				texInfo[i].points[texInfo[i].numpoints][j] = atof(token);
				token = COM_ParseExt(p,qtrue);
				trap_Print( va( S_COLOR_RED "%f ", texInfo[i].points[texInfo[i].numpoints][j] ) );
			}
			texInfo[i].numpoints++;
			trap_Print( "\n" );
		}
		trap_Print( va( S_COLOR_RED "%i\n", texInfo[i].numpoints ) );
	}
	trap_Print(va( S_COLOR_RED "%i slopes\n", i));*/

	//get shadernames
	//READ HEADER
	token = COM_ParseExt(p, qtrue);
	shaderInfoNum = atoi(token);
	if(shaderInfoNum > MAX_BRUSHSIDES){
		trap_Print( va( S_COLOR_RED "Tex-file: %ishaders max allowed %i shaders\n", shaderInfoNum, MAX_BRUSHSIDES ) );
		return qfalse;
	}
	//parse surfaces
	for(i=0;i<shaderInfoNum;i++){

		token = COM_ParseExt(p, qtrue);
		shaderInfo[i].surfaceFlags = atoi(token);

		token = COM_ParseExt(p, qtrue);
		shaderInfo[i].color[0] = atof(token);
		token = COM_ParseExt(p, qtrue);
		shaderInfo[i].color[1] = atof(token);
		token = COM_ParseExt(p, qtrue);
		shaderInfo[i].color[2] = atof(token);
		shaderInfo[i].color[3] = 1.0f;

		//trap_Print( va( S_COLOR_RED "%i\n", shaderInfo[i].surfaceFlags ) );
	}
	//trap_Print( va( S_COLOR_RED "%i shaders\n", shaderInfoNum ) );
	return qtrue;
}
#endif

/*
======================
CG_ParseAnimationFile

Read a configuration file containing animation counts and rates
models/wq3_players/wq_male1/animation.cfg, etc
======================
*/
static qboolean	CG_ParseAnimationFile( const char *filename, clientInfo_t *ci ) {
	char		*text_p, *prev;
	int			len;
	int			i;
	char		*token;
	float		fps;
	int			skip;
	char		text[20000];
	fileHandle_t	f;
	animation_t *animations;

	animations = ci->animations;

	// load the file
	len = trap_FS_FOpenFile( filename, &f, FS_READ );
	if ( len <= 0 ) {
		return qfalse;
	}
	if ( len >= sizeof( text ) - 1 ) {
		CG_Printf( "File %s too long\n", filename );
		trap_FS_FCloseFile( f );
		return qfalse;
	}
	trap_FS_Read( text, len, f );
	text[len] = 0;
	trap_FS_FCloseFile( f );

	// parse the text
	text_p = text;
	skip = 0;	// quite the compiler warning

	ci->footsteps = FOOTSTEP_NORMAL;
	VectorClear( ci->headOffset );
	ci->gender = GENDER_MALE;
#ifndef SMOKINGUNS
	ci->fixedlegs = qfalse;
	ci->fixedtorso = qfalse;
#endif

	// read optional parameters
	while ( 1 ) {
		prev = text_p;	// so we can unget
		token = COM_Parse( &text_p );
		if ( !token ) {
			break;
		}
		if ( !Q_stricmp( token, "footsteps" ) ) {
			token = COM_Parse( &text_p );
			if ( !token ) {
				break;
			}
#ifndef SMOKINGUNS
			if ( !Q_stricmp( token, "default" ) || !Q_stricmp( token, "normal" ) ) {
				ci->footsteps = FOOTSTEP_NORMAL;
			} else if ( !Q_stricmp( token, "boot" ) ) {
				ci->footsteps = FOOTSTEP_BOOT;
			} else if ( !Q_stricmp( token, "flesh" ) ) {
				ci->footsteps = FOOTSTEP_FLESH;
			} else if ( !Q_stricmp( token, "mech" ) ) {
				ci->footsteps = FOOTSTEP_MECH;
			} else if ( !Q_stricmp( token, "energy" ) ) {
				ci->footsteps = FOOTSTEP_ENERGY;
			} else {
				CG_Printf( "Bad footsteps parm in %s: %s\n", filename, token );
			}
#else
			ci->footsteps = FOOTSTEP_NORMAL;
#endif
			continue;
		} else if ( !Q_stricmp( token, "headoffset" ) ) {
			for ( i = 0 ; i < 3 ; i++ ) {
				token = COM_Parse( &text_p );
				if ( !token ) {
					break;
				}
				ci->headOffset[i] = atof( token );
			}
			continue;
		} else if ( !Q_stricmp( token, "sex" ) ) {
			token = COM_Parse( &text_p );
			if ( !token ) {
				break;
			}
			if ( token[0] == 'f' || token[0] == 'F' ) {
				ci->gender = GENDER_FEMALE;
			} else if ( token[0] == 'n' || token[0] == 'N' ) {
				ci->gender = GENDER_NEUTER;
			} else {
				ci->gender = GENDER_MALE;
			}
			continue;
#ifndef SMOKINGUNS
		} else if ( !Q_stricmp( token, "fixedlegs" ) ) {
			ci->fixedlegs = qtrue;
			continue;
		} else if ( !Q_stricmp( token, "fixedtorso" ) ) {
			ci->fixedtorso = qtrue;
			continue;
#endif
		}

		// if it is a number, start parsing animations
		if ( token[0] >= '0' && token[0] <= '9' ) {
			text_p = prev;	// unget the token
			break;
		}
		Com_Printf( "unknown token '%s' in %s\n", token, filename );
	}

	// read information for each frame
	for ( i = 0 ; i < MAX_ANIMATIONS ; i++ ) {

		token = COM_Parse( &text_p );
		if ( !*token ) {
#ifdef NEW_ANIMS
			if( i >= TORSO_GETFLAG && i <= TORSO_NEGATIVE ) {
				animations[i].firstFrame = animations[TORSO_GESTURE].firstFrame;
				animations[i].frameLerp = animations[TORSO_GESTURE].frameLerp;
				animations[i].initialLerp = animations[TORSO_GESTURE].initialLerp;
				animations[i].loopFrames = animations[TORSO_GESTURE].loopFrames;
				animations[i].numFrames = animations[TORSO_GESTURE].numFrames;
				animations[i].reversed = qfalse;
				animations[i].flipflop = qfalse;
				continue;
			}
#endif
			break;
		}
		animations[i].firstFrame = atoi( token );
		// leg only frames are adjusted to not count the upper body only frames
#ifndef SMOKINGUNS
		if ( i == LEGS_WALKCR ) {
			skip = animations[LEGS_WALKCR].firstFrame - animations[TORSO_GESTURE].firstFrame;
		}
		if ( i >= LEGS_WALKCR && i<TORSO_GETFLAG) {
			animations[i].firstFrame -= skip;
		}
#endif

		token = COM_Parse( &text_p );
		if ( !*token ) {
			break;
		}
		animations[i].numFrames = atoi( token );

		animations[i].reversed = qfalse;
		animations[i].flipflop = qfalse;
		// if numFrames is negative the animation is reversed
		if (animations[i].numFrames < 0) {
			animations[i].numFrames = -animations[i].numFrames;
			animations[i].reversed = qtrue;
		}

		token = COM_Parse( &text_p );
		if ( !*token ) {
			break;
		}
		animations[i].loopFrames = atoi( token );

		token = COM_Parse( &text_p );
		if ( !*token ) {
			break;
		}
		fps = atof( token );
		if ( fps == 0 ) {
			fps = 1;
		}
		animations[i].frameLerp = 1000 / fps;
		animations[i].initialLerp = 1000 / fps;
	}

	if ( i != MAX_ANIMATIONS ) {
		CG_Printf( "Error parsing animation file: %s\n", filename );
		return qfalse;
	}

	// crouch backward animation
#ifndef SMOKINGUNS
	memcpy(&animations[LEGS_BACKCR], &animations[LEGS_WALKCR], sizeof(animation_t));
	animations[LEGS_BACKCR].reversed = qtrue;
	// walk backward animation
	memcpy(&animations[LEGS_BACKWALK], &animations[LEGS_WALK], sizeof(animation_t));
	animations[LEGS_BACKWALK].reversed = qtrue;
	// flag moving fast
	animations[FLAG_RUN].firstFrame = 0;
	animations[FLAG_RUN].numFrames = 16;
	animations[FLAG_RUN].loopFrames = 16;
	animations[FLAG_RUN].frameLerp = 1000 / 15;
	animations[FLAG_RUN].initialLerp = 1000 / 15;
	animations[FLAG_RUN].reversed = qfalse;
	// flag not moving or moving slowly
	animations[FLAG_STAND].firstFrame = 16;
	animations[FLAG_STAND].numFrames = 5;
	animations[FLAG_STAND].loopFrames = 0;
	animations[FLAG_STAND].frameLerp = 1000 / 20;
	animations[FLAG_STAND].initialLerp = 1000 / 20;
	animations[FLAG_STAND].reversed = qfalse;
	// flag speeding up
	animations[FLAG_STAND2RUN].firstFrame = 16;
	animations[FLAG_STAND2RUN].numFrames = 5;
	animations[FLAG_STAND2RUN].loopFrames = 1;
	animations[FLAG_STAND2RUN].frameLerp = 1000 / 15;
	animations[FLAG_STAND2RUN].initialLerp = 1000 / 15;
	animations[FLAG_STAND2RUN].reversed = qtrue;
#else
	memcpy(&animations[LEGS_RUN_SLIDE], &animations[LEGS_RUN], sizeof(animation_t));
	animations[LEGS_RUN_SLIDE].frameLerp *= 1;
#endif
	//
	// new anims changes
	//
//	animations[TORSO_GETFLAG].flipflop = qtrue;
//	animations[TORSO_GUARDBASE].flipflop = qtrue;
//	animations[TORSO_PATROL].flipflop = qtrue;
//	animations[TORSO_AFFIRMATIVE].flipflop = qtrue;
//	animations[TORSO_NEGATIVE].flipflop = qtrue;
	//
	return qtrue;
}

/*
==========================
CG_FileExists
==========================
*/
#ifndef SMOKINGUNS
static qboolean	CG_FileExists(const char *filename) {
	int len;

	len = trap_FS_FOpenFile( filename, NULL, FS_READ );
	if (len>0) {
		return qtrue;
	}
	return qfalse;
}

/*
==========================
CG_FindClientModelFile
==========================
*/
static qboolean	CG_FindClientModelFile( char *filename, int length, clientInfo_t *ci, const char *teamName, const char *modelName, const char *skinName, const char *base, const char *ext ) {
	char *team, *charactersFolder;
	int i;

	if ( cgs.gametype >= GT_TEAM ) {
		switch ( ci->team ) {
			case TEAM_BLUE: {
				team = "blue";
				break;
			}
			default: {
				team = "red";
				break;
			}
		}
	}
	else {
		team = "default";
	}
	charactersFolder = "";
	while(1) {
		for ( i = 0; i < 2; i++ ) {
			if ( i == 0 && teamName && *teamName ) {
				//								"models/players/characters/james/stroggs/lower_lily_red.skin"
				Com_sprintf( filename, length, "models/players/%s%s/%s%s_%s_%s.%s", charactersFolder, modelName, teamName, base, skinName, team, ext );
			}
			else {
				//								"models/players/characters/james/lower_lily_red.skin"
				Com_sprintf( filename, length, "models/players/%s%s/%s_%s_%s.%s", charactersFolder, modelName, base, skinName, team, ext );
			}
			if ( CG_FileExists( filename ) ) {
				return qtrue;
			}
			if ( cgs.gametype >= GT_TEAM ) {
				if ( i == 0 && teamName && *teamName ) {
					//								"models/players/characters/james/stroggs/lower_red.skin"
					Com_sprintf( filename, length, "models/players/%s%s/%s%s_%s.%s", charactersFolder, modelName, teamName, base, team, ext );
				}
				else {
					//								"models/players/characters/james/lower_red.skin"
					Com_sprintf( filename, length, "models/players/%s%s/%s_%s.%s", charactersFolder, modelName, base, team, ext );
				}
			}
			else {
				if ( i == 0 && teamName && *teamName ) {
					//								"models/players/characters/james/stroggs/lower_lily.skin"
					Com_sprintf( filename, length, "models/players/%s%s/%s%s_%s.%s", charactersFolder, modelName, teamName, base, skinName, ext );
				}
				else {
					//								"models/players/characters/james/lower_lily.skin"
					Com_sprintf( filename, length, "models/players/%s%s/%s_%s.%s", charactersFolder, modelName, base, skinName, ext );
				}
			}
			if ( CG_FileExists( filename ) ) {
				return qtrue;
			}
			if ( !teamName || !*teamName ) {
				break;
			}
		}
		// if tried the heads folder first
		if ( charactersFolder[0] ) {
			break;
		}
		charactersFolder = "characters/";
	}

	return qfalse;
}

/*
==========================
CG_FindClientHeadFile
==========================
*/
static qboolean	CG_FindClientHeadFile( char *filename, int length, clientInfo_t *ci, const char *teamName, const char *headModelName, const char *headSkinName, const char *base, const char *ext ) {
	char *team, *headsFolder;
	int i;

	if ( cgs.gametype >= GT_TEAM ) {
		switch ( ci->team ) {
			case TEAM_BLUE: {
				team = "blue";
				break;
			}
			default: {
				team = "red";
				break;
			}
		}
	}
	else {
		team = "default";
	}

	if ( headModelName[0] == '*' ) {
		headsFolder = "heads/";
		headModelName++;
	}
	else {
		headsFolder = "";
	}
	while(1) {
		for ( i = 0; i < 2; i++ ) {
			if ( i == 0 && teamName && *teamName ) {
				Com_sprintf( filename, length, "models/players/%s%s/%s/%s%s_%s.%s", headsFolder, headModelName, headSkinName, teamName, base, team, ext );
			}
			else {
				Com_sprintf( filename, length, "models/players/%s%s/%s/%s_%s.%s", headsFolder, headModelName, headSkinName, base, team, ext );
			}
			if ( CG_FileExists( filename ) ) {
				return qtrue;
			}
			if ( cgs.gametype >= GT_TEAM ) {
				if ( i == 0 &&  teamName && *teamName ) {
					Com_sprintf( filename, length, "models/players/%s%s/%s%s_%s.%s", headsFolder, headModelName, teamName, base, team, ext );
				}
				else {
					Com_sprintf( filename, length, "models/players/%s%s/%s_%s.%s", headsFolder, headModelName, base, team, ext );
				}
			}
			else {
				if ( i == 0 && teamName && *teamName ) {
					Com_sprintf( filename, length, "models/players/%s%s/%s%s_%s.%s", headsFolder, headModelName, teamName, base, headSkinName, ext );
				}
				else {
					Com_sprintf( filename, length, "models/players/%s%s/%s_%s.%s", headsFolder, headModelName, base, headSkinName, ext );
				}
			}
			if ( CG_FileExists( filename ) ) {
				return qtrue;
			}
			if ( !teamName || !*teamName ) {
				break;
			}
		}
		// if tried the heads folder first
		if ( headsFolder[0] ) {
			break;
		}
		headsFolder = "heads/";
	}

	return qfalse;
}
#endif

/*
==========================
CG_RegisterClientSkin
==========================
*/
#ifndef SMOKINGUNS
static qboolean	CG_RegisterClientSkin( clientInfo_t *ci, const char *teamName, const char *modelName, const char *skinName, const char *headModelName, const char *headSkinName ) {
#else
static qboolean	CG_RegisterClientSkin( clientInfo_t *ci, const char *modelName, const char *skinName ) {
#endif
	char filename[MAX_QPATH];

#ifdef SMOKINGUNS
	Com_sprintf( filename, sizeof( filename ), "models/wq3_players/%s/holster_%s.skin", modelName, skinName );
	ci->holsterSkin = trap_R_RegisterSkin( filename );
#endif
	/*
	Com_sprintf( filename, sizeof( filename ), "models/players/%s/%slower_%s.skin", modelName, teamName, skinName );
	ci->legsSkin = trap_R_RegisterSkin( filename );
	if (!ci->legsSkin) {
		Com_sprintf( filename, sizeof( filename ), "models/players/characters/%s/%slower_%s.skin", modelName, teamName, skinName );
		ci->legsSkin = trap_R_RegisterSkin( filename );
		if (!ci->legsSkin) {
			Com_Printf( "Leg skin load failure: %s\n", filename );
		}
	}


	Com_sprintf( filename, sizeof( filename ), "models/players/%s/%supper_%s.skin", modelName, teamName, skinName );
	ci->torsoSkin = trap_R_RegisterSkin( filename );
	if (!ci->torsoSkin) {
		Com_sprintf( filename, sizeof( filename ), "models/players/characters/%s/%supper_%s.skin", modelName, teamName, skinName );
		ci->torsoSkin = trap_R_RegisterSkin( filename );
		if (!ci->torsoSkin) {
			Com_Printf( "Torso skin load failure: %s\n", filename );
		}
	}
	*/
#ifndef SMOKINGUNS
	if ( CG_FindClientModelFile( filename, sizeof(filename), ci, teamName, modelName, skinName, "lower", "skin" ) ) {
		ci->legsSkin = trap_R_RegisterSkin( filename );
	}
	if (!ci->legsSkin) {
		Com_Printf( "Leg skin load failure: %s\n", filename );
	}

	if ( CG_FindClientModelFile( filename, sizeof(filename), ci, teamName, modelName, skinName, "upper", "skin" ) ) {
		ci->torsoSkin = trap_R_RegisterSkin( filename );
	}
	if (!ci->torsoSkin) {
		Com_Printf( "Torso skin load failure: %s\n", filename );
	}

	if ( CG_FindClientHeadFile( filename, sizeof(filename), ci, teamName, headModelName, headSkinName, "head", "skin" ) ) {
		ci->headSkin = trap_R_RegisterSkin( filename );
	}
	if (!ci->headSkin) {
		Com_Printf( "Head skin load failure: %s\n", filename );
	}

	// if any skins failed to load
	if ( !ci->legsSkin || !ci->torsoSkin || !ci->headSkin ) {
		return qfalse;
	}
#else
	Com_sprintf( filename, sizeof( filename ), "models/wq3_players/%s/lower_%s.skin", modelName, skinName );
	ci->legsSkin = trap_R_RegisterSkin( filename );
	if (!ci->legsSkin && cg_cheats) {
		Com_Printf( "Leg skin load failure: %s\n", filename );
	}


	Com_sprintf( filename, sizeof( filename ), "models/wq3_players/%s/upper_%s.skin", modelName, skinName );
	ci->torsoSkin = trap_R_RegisterSkin( filename );
	if (!ci->torsoSkin && cg_cheats) {
		Com_Printf( "Torso skin load failure: %s\n", filename );
	}

	//
	// DAMAGE SKINS
	//

	//head
	Com_sprintf( filename, sizeof( filename ), "models/wq3_players/%s/head_%s_damage_1.skin", modelName, skinName );
	ci->headSkin_damage_1 = trap_R_RegisterSkin( filename );
	if (!ci->headSkin_damage_1 && cg_cheats) {
		Com_Printf( "Head skin load failure: %s\n", filename );
	}
	Com_sprintf( filename, sizeof( filename ), "models/wq3_players/%s/head_%s_damage_2.skin", modelName, skinName );
	ci->headSkin_damage_2 = trap_R_RegisterSkin( filename );
	if (!ci->headSkin_damage_2 && cg_cheats) {
		Com_Printf( "Head skin load failure: %s\n", filename );
	}

	//upper
	Com_sprintf( filename, sizeof( filename ), "models/wq3_players/%s/upper_%s_damage_1.skin", modelName, skinName );
	ci->torsoSkin_damage_1 = trap_R_RegisterSkin( filename );
	if (!ci->torsoSkin_damage_1 && cg_cheats) {
		Com_Printf( "Torso skin load failure: %s\n", filename );
	}
	Com_sprintf( filename, sizeof( filename ), "models/wq3_players/%s/upper_%s_damage_2.skin", modelName, skinName );
	ci->torsoSkin_damage_2 = trap_R_RegisterSkin( filename );
	if (!ci->torsoSkin_damage_2 && cg_cheats) {
		Com_Printf( "Torso skin load failure: %s\n", filename );
	}

	//lower
	Com_sprintf( filename, sizeof( filename ), "models/wq3_players/%s/lower_%s_damage_1.skin", modelName, skinName );
	ci->legsSkin_damage_1 = trap_R_RegisterSkin( filename );
	if (!ci->legsSkin_damage_1 && cg_cheats) {
		Com_Printf( "Torso skin load failure: %s\n", filename );
	}
	Com_sprintf( filename, sizeof( filename ), "models/wq3_players/%s/lower_%s_damage_2.skin", modelName, skinName );
	ci->legsSkin_damage_2 = trap_R_RegisterSkin( filename );
	if (!ci->legsSkin_damage_2 && cg_cheats) {
		Com_Printf( "Torso skin load failure: %s\n", filename );
	}


	/*if( headModelName[0] == '*' ) {
		Com_sprintf( filename, sizeof( filename ), "models/wq3_players/heads/%s/head_%s.skin", &headModelName[1], headSkinName );
	} else {*/
		Com_sprintf( filename, sizeof( filename ), "models/wq3_players/%s/head_%s.skin", modelName, skinName/*headModelName,headSkinName*/ );
	//}
	ci->headSkin = trap_R_RegisterSkin( filename );
	// if the head skin could not be found and we didn't load from the heads folder try to load from there
	/*if ( !ci->headSkin && headModelName[0] != '*' ) {
		Com_sprintf( filename, sizeof( filename ), "models/wq3_players/heads/%s/head_%s.skin", headModelName, headSkinName );
		ci->headSkin = trap_R_RegisterSkin( filename );
	}*/
	if (!ci->headSkin && cg_cheats) {
		Com_Printf( "Head skin load failure: %s\n", filename );
	}
	if ( !ci->legsSkin || !ci->torsoSkin || !ci->headSkin || !ci->legsSkin_damage_1 ||
		!ci->legsSkin_damage_1 || !ci->headSkin_damage_1 || !ci->headSkin_damage_2 ||
		!ci->torsoSkin_damage_1 || !ci->torsoSkin_damage_2) {
		return qfalse;
	}
#endif
	return qtrue;
}

/*
==========================
CG_RegisterClientModelname
==========================
*/
#ifndef SMOKINGUNS
static qboolean CG_RegisterClientModelname( clientInfo_t *ci, const char *modelName, const char *skinName, const char *headModelName, const char *headSkinName, const char *teamName ) {
#else
static qboolean CG_RegisterClientModelname( clientInfo_t *ci, const char *modelName, const char *skinName ) {
#endif
	char	filename[MAX_QPATH];
#ifndef SMOKINGUNS
	const char		*headName;
	char newTeamName[MAX_QPATH];

	if ( headModelName[0] == '\0' ) {
		headName = modelName;
	}
	else {
		headName = headModelName;
	}
	Com_sprintf( filename, sizeof( filename ), "models/players/%s/lower.md3", modelName );
#else
	Com_sprintf( filename, sizeof( filename ), "models/wq3_players/%s/lower.md3", modelName );
#endif
	ci->legsModel = trap_R_RegisterModel( filename );
#ifndef SMOKINGUNS
	if ( !ci->legsModel ) {
		Com_sprintf( filename, sizeof( filename ), "models/players/characters/%s/lower.md3", modelName );
		ci->legsModel = trap_R_RegisterModel( filename );
		if ( !ci->legsModel ) {
#else
	if ( !ci->legsModel && cg_cheats) {
#endif
			Com_Printf( "Failed to load model file %s\n", filename );
			return qfalse;
		}
#ifndef SMOKINGUNS
	}

	Com_sprintf( filename, sizeof( filename ), "models/players/%s/upper.md3", modelName );
#else
	Com_sprintf( filename, sizeof( filename ), "models/wq3_players/%s/upper.md3", modelName );
#endif
	ci->torsoModel = trap_R_RegisterModel( filename );
#ifndef SMOKINGUNS
	if ( !ci->torsoModel ) {
		Com_sprintf( filename, sizeof( filename ), "models/players/characters/%s/upper.md3", modelName );
		ci->torsoModel = trap_R_RegisterModel( filename );
		if ( !ci->torsoModel ) {
#else
	if ( !ci->torsoModel && cg_cheats) {
#endif
			Com_Printf( "Failed to load model file %s\n", filename );
			return qfalse;
		}
#ifndef SMOKINGUNS
	}

	if( headName[0] == '*' ) {
		Com_sprintf( filename, sizeof( filename ), "models/players/heads/%s/%s.md3", &headModelName[1], &headModelName[1] );
	}
	else {
		Com_sprintf( filename, sizeof( filename ), "models/players/%s/head.md3", headName );
	}
#else
	Com_sprintf( filename, sizeof( filename ), "models/wq3_players/%s/head.md3", modelName);
#endif
	ci->headModel = trap_R_RegisterModel( filename );
#ifndef SMOKINGUNS
	// if the head model could not be found and we didn't load from the heads folder try to load from there
	if ( !ci->headModel && headName[0] != '*' ) {
		Com_sprintf( filename, sizeof( filename ), "models/players/heads/%s/%s.md3", headModelName, headModelName );
		ci->headModel = trap_R_RegisterModel( filename );
	}
	if ( !ci->headModel ) {
#else
	if ( !ci->headModel && cg_cheats) {
#endif
		Com_Printf( "Failed to load model file %s\n", filename );
		return qfalse;
	}

	// if any skins failed to load, return failure
#ifndef SMOKINGUNS
	if ( !CG_RegisterClientSkin( ci, teamName, modelName, skinName, headName, headSkinName ) ) {
		if ( teamName && *teamName) {
			Com_Printf( "Failed to load skin file: %s : %s : %s, %s : %s\n", teamName, modelName, skinName, headName, headSkinName );
			if( ci->team == TEAM_BLUE ) {
				Com_sprintf(newTeamName, sizeof(newTeamName), "%s/", DEFAULT_BLUETEAM_NAME);
			}
			else {
				Com_sprintf(newTeamName, sizeof(newTeamName), "%s/", DEFAULT_REDTEAM_NAME);
			}
			if ( !CG_RegisterClientSkin( ci, newTeamName, modelName, skinName, headName, headSkinName ) ) {
				Com_Printf( "Failed to load skin file: %s : %s : %s, %s : %s\n", newTeamName, modelName, skinName, headName, headSkinName );
#else
	if ( !CG_RegisterClientSkin( ci, modelName, skinName ) ) {
		if(cg_cheats)
			Com_Printf( "Failed to load skin file: %s : %s\n", modelName, skinName );
#endif
				return qfalse;
			}
#ifndef SMOKINGUNS
		} else {
			Com_Printf( "Failed to load skin file: %s : %s, %s : %s\n", modelName, skinName, headName, headSkinName );
			return qfalse;
		}
	}
#endif

	// load the animations
#ifndef SMOKINGUNS
	Com_sprintf( filename, sizeof( filename ), "models/players/%s/animation.cfg", modelName );
	if ( !CG_ParseAnimationFile( filename, ci ) ) {
		Com_sprintf( filename, sizeof( filename ), "models/players/characters/%s/animation.cfg", modelName );
		if ( !CG_ParseAnimationFile( filename, ci ) ) {
			Com_Printf( "Failed to load animation file %s\n", filename );
			return qfalse;
		}
	}
#else
	Com_sprintf( filename, sizeof( filename ), "models/wq3_players/%s/animation.cfg", modelName );
	if ( !CG_ParseAnimationFile( filename, ci ) ) {
		if(cg_cheats)
			Com_Printf( "Failed to load animation file %s\n", filename );
		return qfalse;
	}
#endif

#ifndef SMOKINGUNS
	if ( CG_FindClientHeadFile( filename, sizeof(filename), ci, teamName, headName, headSkinName, "icon", "skin" ) ) {
		ci->modelIcon = trap_R_RegisterShaderNoMip( filename );
	}
	else if ( CG_FindClientHeadFile( filename, sizeof(filename), ci, teamName, headName, headSkinName, "icon", "tga" ) ) {
		ci->modelIcon = trap_R_RegisterShaderNoMip( filename );
	}

	if ( !ci->modelIcon ) {
#else
	Com_sprintf( filename, sizeof( filename ), "models/wq3_players/%s/icon_%s.tga", modelName, skinName );
	ci->modelIcon = trap_R_RegisterShaderNoMip( filename );

	if ( !ci->modelIcon && cg_cheats) {
		Com_Printf( "Failed to load icon file: %s\n", filename );
#endif
		return qfalse;
	}

	return qtrue;
}

/*
====================
CG_ColorFromString
====================
*/
#ifndef SMOKINGUNS
static void CG_ColorFromString( const char *v, vec3_t color ) {
	int val;

	VectorClear( color );

	val = atoi( v );

	if ( val < 1 || val > 7 ) {
		VectorSet( color, 1, 1, 1 );
		return;
	}

	if ( val & 1 ) {
		color[2] = 1.0f;
	}
	if ( val & 2 ) {
		color[1] = 1.0f;
	}
	if ( val & 4 ) {
		color[0] = 1.0f;
	}
}
#endif

/*
===================
CG_LoadClientInfo

Load it now, taking the disk hits.
This will usually be deferred to a safe time
===================
*/
static void CG_LoadClientInfo( int clientNum, clientInfo_t *ci ) {
	const char	*dir, *fallback;
	int			i, modelloaded;
	const char	*s;
	char		teamname[MAX_QPATH];

	teamname[0] = 0;
#ifdef SMOKINGUNS
	if( cgs.gametype >= GT_TEAM) {
		if( ci->team == TEAM_BLUE ) {
			Q_strncpyz(teamname, cg_blueTeamName.string, sizeof(teamname) );
		} else {
			Q_strncpyz(teamname, cg_redTeamName.string, sizeof(teamname) );
		}
	}
	if( teamname[0] ) {
		strcat( teamname, "/" );
	}
#endif
	modelloaded = qtrue;
#ifndef SMOKINGUNS
	if ( !CG_RegisterClientModelname( ci, ci->modelName, ci->skinName, ci->headModelName, ci->headSkinName, teamname ) ) {
#else
	if ( !CG_RegisterClientModelname( ci, ci->modelName, ci->skinName ) ) {
#endif
		if ( cg_buildScript.integer ) {
			CG_Error( "CG_RegisterClientModelname( %s, %s, %s, %s %s ) failed", ci->modelName, ci->skinName, ci->headModelName, ci->headSkinName, teamname );
		}

		// fall back to default team name
		if( cgs.gametype >= GT_TEAM) {
#ifndef SMOKINGUNS
			// keep skin name
			if( ci->team == TEAM_BLUE ) {
				Q_strncpyz(teamname, DEFAULT_BLUETEAM_NAME, sizeof(teamname) );
			} else {
				Q_strncpyz(teamname, DEFAULT_REDTEAM_NAME, sizeof(teamname) );
			}
			if ( !CG_RegisterClientModelname( ci, DEFAULT_TEAM_MODEL, ci->skinName, DEFAULT_TEAM_HEAD, ci->skinName, teamname ) ) {
				CG_Error( "DEFAULT_TEAM_MODEL / skin (%s/%s) failed to register", DEFAULT_TEAM_MODEL, ci->skinName );
			}
		} else {
			if ( !CG_RegisterClientModelname( ci, DEFAULT_MODEL, "default", DEFAULT_MODEL, "default", teamname ) ) {
#else
			char		model[64];
			char		skin[64];
			strcpy(model, DEFAULT_MODEL);

			if(ci->team == TEAM_BLUE || ci->team == TEAM_BLUE_SPECTATOR){
				strcpy(skin, "blue");
			} else if(ci->team == TEAM_RED || ci->team == TEAM_RED_SPECTATOR ||
				ci->team  == TEAM_SPECTATOR){
				strcpy(skin, "red");
			}


			// keep skin name
			if ( !CG_RegisterClientModelname( ci, model, ci->skinName ) ) {
				if ( !CG_RegisterClientModelname( ci, DEFAULT_MODEL, "red" ) )
					CG_Error( "DEFAULT_MODEL (%s) failed to register", DEFAULT_MODEL );
			}
		} else {
			if ( !CG_RegisterClientModelname( ci, DEFAULT_MODEL, "red" ) ) {
#endif
				CG_Error( "DEFAULT_MODEL (%s) failed to register", DEFAULT_MODEL );
			}
		}
		modelloaded = qfalse;
	}

	ci->newAnims = qfalse;
	if ( ci->torsoModel ) {
		orientation_t tag;
		// if the torso model has the "tag_flag"
		if ( trap_R_LerpTag( &tag, ci->torsoModel, 0, 0, 1, "tag_flag" ) ) {
			ci->newAnims = qtrue;
		}
	}

	// sounds
	dir = ci->modelName;
#ifndef SMOKINGUNS
	fallback = (cgs.gametype >= GT_TEAM) ? DEFAULT_TEAM_MODEL : DEFAULT_MODEL;
#else
	fallback = DEFAULT_MODEL;
#endif

	for ( i = 0 ; i < MAX_CUSTOM_SOUNDS ; i++ ) {
#ifdef SMOKINGUNS
		char	gender[10];

		if(ci->gender == GENDER_FEMALE)
			strcpy(gender, "female");
		else
			strcpy(gender, "male");
#endif
		s = cg_customSoundNames[i];
		if ( !s ) {
			break;
		}
		ci->sounds[i] = 0;
#ifndef SMOKINGUNS
		// if the model didn't load use the sounds of the default model
		if (modelloaded) {
			ci->sounds[i] = trap_S_RegisterSound( va("sound/player/%s/%s", dir, s + 1), qfalse );
		}
		if ( !ci->sounds[i] ) {
			ci->sounds[i] = trap_S_RegisterSound( va("sound/player/%s/%s", fallback, s + 1), qfalse );
#else
		if ( !ci->sounds[i] ) {
			ci->sounds[i] = trap_S_RegisterSound( va("sound/player/%s/%s", gender, s + 1), qfalse );
#endif
		}
	}

	ci->deferred = qfalse;

	// reset any existing players and bodies, because they might be in bad
	// frames for this new model
	for ( i = 0 ; i < MAX_GENTITIES ; i++ ) {
		if ( cg_entities[i].currentState.clientNum == clientNum
			&& cg_entities[i].currentState.eType == ET_PLAYER ) {
			CG_ResetPlayerEntity( &cg_entities[i] );
		}
	}
}

/*
======================
CG_CopyClientInfoModel
======================
*/
static void CG_CopyClientInfoModel( clientInfo_t *from, clientInfo_t *to ) {
	VectorCopy( from->headOffset, to->headOffset );
	to->footsteps = from->footsteps;
	to->gender = from->gender;

	to->legsModel = from->legsModel;
	to->legsSkin = from->legsSkin;
	to->torsoModel = from->torsoModel;
	to->torsoSkin = from->torsoSkin;
	to->headModel = from->headModel;
	to->headSkin = from->headSkin;
	to->modelIcon = from->modelIcon;
#ifdef SMOKINGUNS
	to->holsterSkin = from->holsterSkin;
	to->torsoSkin_damage_1 = from->torsoSkin_damage_1;
	to->torsoSkin_damage_2 = from->torsoSkin_damage_2;
	to->headSkin_damage_1 = from->headSkin_damage_1;
	to->headSkin_damage_2 = from->headSkin_damage_2;
	to->legsSkin_damage_1 = from->legsSkin_damage_1;
	to->legsSkin_damage_2 = from->legsSkin_damage_2;
#endif

	to->newAnims = from->newAnims;

	memcpy( to->animations, from->animations, sizeof( to->animations ) );
	memcpy( to->sounds, from->sounds, sizeof( to->sounds ) );
}

/*
======================
CG_ScanForExistingClientInfo
======================
*/
static qboolean CG_ScanForExistingClientInfo( clientInfo_t *ci ) {
	int		i;
	clientInfo_t	*match;

	for ( i = 0 ; i < cgs.maxclients ; i++ ) {
		match = &cgs.clientinfo[ i ];
		if ( !match->infoValid ) {
			continue;
		}
		if ( match->deferred ) {
			continue;
		}
		if ( !Q_stricmp( ci->modelName, match->modelName )
			&& !Q_stricmp( ci->skinName, match->skinName )
			&& !Q_stricmp( ci->headModelName, match->headModelName )
			&& !Q_stricmp( ci->headSkinName, match->headSkinName )
			&& !Q_stricmp( ci->blueTeam, match->blueTeam )
#ifndef SMOKINGUNS
			&& !Q_stricmp( ci->redTeam, match->redTeam )
			&& (cgs.gametype < GT_TEAM || ci->team == match->team) ) {
#else
			&& !Q_stricmp( ci->redTeam, match->redTeam ) ) {
#endif
			// this clientinfo is identical, so use its handles

			ci->deferred = qfalse;

			CG_CopyClientInfoModel( match, ci );

			return qtrue;
		}
	}

	// nothing matches, so defer the load
	return qfalse;
}

/*
======================
CG_SetDeferredClientInfo

We aren't going to load it now, so grab some other
client's info to use until we have some spare time.
======================
*/
static void CG_SetDeferredClientInfo( int clientNum, clientInfo_t *ci ) {
	int		i;
	clientInfo_t	*match;

	// if someone else is already the same models and skins we
	// can just load the client info
	for ( i = 0 ; i < cgs.maxclients ; i++ ) {
		match = &cgs.clientinfo[ i ];
		if ( !match->infoValid || match->deferred ) {
			continue;
		}
		if ( Q_stricmp( ci->skinName, match->skinName ) ||
			 Q_stricmp( ci->modelName, match->modelName ) ||
#ifndef SMOKINGUNS
//			 Q_stricmp( ci->headModelName, match->headModelName ) ||
//			 Q_stricmp( ci->headSkinName, match->headSkinName ) ||
			 (cgs.gametype >= GT_TEAM && ci->team != match->team) ) {
#else
			 Q_stricmp( ci->headModelName, match->headModelName ) ||
			 Q_stricmp( ci->headSkinName, match->headSkinName ) ) {
#endif
			continue;
		}
		// just load the real info cause it uses the same models and skins
		CG_LoadClientInfo( clientNum, ci );
		return;
	}

	// if we are in teamplay, only grab a model if the skin is correct
	if ( cgs.gametype >= GT_TEAM ) {
		for ( i = 0 ; i < cgs.maxclients ; i++ ) {
			match = &cgs.clientinfo[ i ];
			if ( !match->infoValid || match->deferred ) {
				continue;
			}
#ifndef SMOKINGUNS
			if ( Q_stricmp( ci->skinName, match->skinName ) ||
				(cgs.gametype >= GT_TEAM && ci->team != match->team) ) {
#else
			if ( Q_stricmp( ci->skinName, match->skinName ) ) {
#endif
				continue;
			}
			ci->deferred = qtrue;
			CG_CopyClientInfoModel( match, ci );
			return;
		}
		// load the full model, because we don't ever want to show
		// an improper team skin.  This will cause a hitch for the first
		// player, when the second enters.  Combat shouldn't be going on
		// yet, so it shouldn't matter
		CG_LoadClientInfo( clientNum, ci );
		return;
	}

	// find the first valid clientinfo and grab its stuff
	for ( i = 0 ; i < cgs.maxclients ; i++ ) {
		match = &cgs.clientinfo[ i ];
		if ( !match->infoValid ) {
			continue;
		}

		ci->deferred = qtrue;
		CG_CopyClientInfoModel( match, ci );
		return;
	}

	// we should never get here...
	CG_Printf( "CG_SetDeferredClientInfo: no valid clients!\n" );

	CG_LoadClientInfo( clientNum, ci );
}


/*
======================
CG_NewClientInfo
======================
*/
void CG_NewClientInfo( int clientNum ) {
	clientInfo_t *ci;
	clientInfo_t newInfo;
	const char	*configstring;
	const char	*v;
	char		*slash;

	ci = &cgs.clientinfo[clientNum];

	configstring = CG_ConfigString( clientNum + CS_PLAYERS );
	if ( !configstring[0] ) {
		memset( ci, 0, sizeof( *ci ) );
		return;		// player just left
	}

	// build into a temp buffer so the defer checks can use
	// the old value
	memset( &newInfo, 0, sizeof( newInfo ) );

	// isolate the player's name
	v = Info_ValueForKey(configstring, "n");
	Q_strncpyz( newInfo.name, v, sizeof( newInfo.name ) );

	// colors
#ifndef SMOKINGUNS
	v = Info_ValueForKey( configstring, "c1" );
	CG_ColorFromString( v, newInfo.color1 );

	newInfo.c1RGBA[0] = 255 * newInfo.color1[0];
	newInfo.c1RGBA[1] = 255 * newInfo.color1[1];
	newInfo.c1RGBA[2] = 255 * newInfo.color1[2];
	newInfo.c1RGBA[3] = 255;

	v = Info_ValueForKey( configstring, "c2" );
	CG_ColorFromString( v, newInfo.color2 );

	newInfo.c2RGBA[0] = 255 * newInfo.color2[0];
	newInfo.c2RGBA[1] = 255 * newInfo.color2[1];
	newInfo.c2RGBA[2] = 255 * newInfo.color2[2];
	newInfo.c2RGBA[3] = 255;
#endif

	// bot skill
	v = Info_ValueForKey( configstring, "skill" );
	newInfo.botSkill = atoi( v );

	// handicap
	v = Info_ValueForKey( configstring, "hc" );
	newInfo.handicap = atoi( v );

	// wins
	v = Info_ValueForKey( configstring, "w" );
	newInfo.wins = atoi( v );

	// losses
	v = Info_ValueForKey( configstring, "l" );
	newInfo.losses = atoi( v );

	// team
	v = Info_ValueForKey( configstring, "t" );
	newInfo.team = atoi( v );

	// team task
	v = Info_ValueForKey( configstring, "tt" );
	newInfo.teamTask = atoi(v);

	// team leader
	v = Info_ValueForKey( configstring, "tl" );
	newInfo.teamLeader = atoi(v);

	v = Info_ValueForKey( configstring, "g_redteam" );
	Q_strncpyz(newInfo.redTeam, v, MAX_TEAMNAME);

	v = Info_ValueForKey( configstring, "g_blueteam" );
	Q_strncpyz(newInfo.blueTeam, v, MAX_TEAMNAME);

	// model
	v = Info_ValueForKey( configstring, "model" );
	if ( cg_forceModel.integer ) {
		// forcemodel makes everyone use a single model
		// to prevent load hitches
		char modelStr[MAX_QPATH];
		char *skin;

		if( cgs.gametype >= GT_TEAM ) {
#ifndef SMOKINGUNS
			Q_strncpyz( newInfo.modelName, DEFAULT_TEAM_MODEL, sizeof( newInfo.modelName ) );
			Q_strncpyz( newInfo.skinName, "default", sizeof( newInfo.skinName ) );
#else
			Q_strncpyz( newInfo.modelName, DEFAULT_MODEL, sizeof( newInfo.modelName ) );
			Q_strncpyz( newInfo.skinName, "red", sizeof( newInfo.skinName ) );
#endif
		} else {
			trap_Cvar_VariableStringBuffer( "model", modelStr, sizeof( modelStr ) );
			if ( ( skin = strchr( modelStr, '/' ) ) == NULL) {
#ifndef SMOKINGUNS
				skin = "default";
#else
				skin = "red";
#endif
			} else {
				*skin++ = 0;
			}

			Q_strncpyz( newInfo.skinName, skin, sizeof( newInfo.skinName ) );
			Q_strncpyz( newInfo.modelName, modelStr, sizeof( newInfo.modelName ) );
		}

		if ( cgs.gametype >= GT_TEAM ) {
			// keep skin name
			slash = strchr( v, '/' );
			if ( slash ) {
				Q_strncpyz( newInfo.skinName, slash + 1, sizeof( newInfo.skinName ) );
			}
		}
	} else {
		Q_strncpyz( newInfo.modelName, v, sizeof( newInfo.modelName ) );

		slash = strchr( newInfo.modelName, '/' );
		if ( !slash ) {
			// modelName didn not include a skin name
#ifndef SMOKINGUNS
			Q_strncpyz( newInfo.skinName, "default", sizeof( newInfo.skinName ) );
#else
			Q_strncpyz( newInfo.skinName, "red", sizeof( newInfo.skinName ) );
#endif
		} else {
			Q_strncpyz( newInfo.skinName, slash + 1, sizeof( newInfo.skinName ) );
			// truncate modelName
			*slash = 0;
		}
	}

	// head model
	v = Info_ValueForKey( configstring, "hmodel" );
	if ( cg_forceModel.integer ) {
		// forcemodel makes everyone use a single model
		// to prevent load hitches
		char modelStr[MAX_QPATH];
		char *skin;

		if( cgs.gametype >= GT_TEAM ) {
#ifndef SMOKINGUNS
			Q_strncpyz( newInfo.headModelName, DEFAULT_TEAM_HEAD, sizeof( newInfo.headModelName ) );
			Q_strncpyz( newInfo.headSkinName, "default", sizeof( newInfo.headSkinName ) );
#else
			Q_strncpyz( newInfo.headModelName, DEFAULT_MODEL, sizeof( newInfo.headModelName ) );
			Q_strncpyz( newInfo.headSkinName, "red", sizeof( newInfo.headSkinName ) );
#endif
		} else {
			trap_Cvar_VariableStringBuffer( "headmodel", modelStr, sizeof( modelStr ) );
			if ( ( skin = strchr( modelStr, '/' ) ) == NULL) {
#ifndef SMOKINGUNS
				skin = "default";
#else
				skin = "red";
#endif
			} else {
				*skin++ = 0;
			}

			Q_strncpyz( newInfo.headSkinName, skin, sizeof( newInfo.headSkinName ) );
			Q_strncpyz( newInfo.headModelName, modelStr, sizeof( newInfo.headModelName ) );
		}

		if ( cgs.gametype >= GT_TEAM ) {
			// keep skin name
			slash = strchr( v, '/' );
			if ( slash ) {
				Q_strncpyz( newInfo.headSkinName, slash + 1, sizeof( newInfo.headSkinName ) );
			}
		}
	} else {
		Q_strncpyz( newInfo.headModelName, v, sizeof( newInfo.headModelName ) );

		slash = strchr( newInfo.headModelName, '/' );
		if ( !slash ) {
			// modelName didn not include a skin name
#ifndef SMOKINGUNS
			Q_strncpyz( newInfo.headSkinName, "default", sizeof( newInfo.headSkinName ) );
#else
			Q_strncpyz( newInfo.headSkinName, "red", sizeof( newInfo.headSkinName ) );
#endif
		} else {
			Q_strncpyz( newInfo.headSkinName, slash + 1, sizeof( newInfo.headSkinName ) );
			// truncate modelName
			*slash = 0;
		}
	}

	// scan for an existing clientinfo that matches this modelname
	// so we can avoid loading checks if possible
	if ( !CG_ScanForExistingClientInfo( &newInfo ) ) {
		qboolean	forceDefer;

		forceDefer = trap_MemoryRemaining() < 4000000;

		// if we are defering loads, just have it pick the first valid
		if ( forceDefer || (cg_deferPlayers.integer && !cg_buildScript.integer && !cg.loading ) ) {
			// keep whatever they had if it won't violate team skins
			CG_SetDeferredClientInfo( clientNum, &newInfo );
			// if we are low on memory, leave them with this model
			if ( forceDefer ) {
				CG_Printf( "Memory is low. Using deferred model.\n" );
				newInfo.deferred = qfalse;
			}
		} else {
			CG_LoadClientInfo( clientNum, &newInfo );
		}
	}

	// replace whatever was there with the new one
	newInfo.infoValid = qtrue;
	*ci = newInfo;
}



/*
======================
CG_LoadDeferredPlayers

Called each frame when a player is dead
and the scoreboard is up
so deferred players can be loaded
======================
*/
void CG_LoadDeferredPlayers( void ) {
	int		i;
	clientInfo_t	*ci;

	// scan for a deferred player to load
	for ( i = 0, ci = cgs.clientinfo ; i < cgs.maxclients ; i++, ci++ ) {
		if ( ci->infoValid && ci->deferred ) {
			// if we are low on memory, leave it deferred
			if ( trap_MemoryRemaining() < 4000000 ) {
				CG_Printf( "Memory is low. Using deferred model.\n" );
				ci->deferred = qfalse;
				continue;
			}
			CG_LoadClientInfo( i, ci );
//			break;
		}
	}
}

/*
=============================================================================

PLAYER ANIMATION

=============================================================================
*/


/*
===============
CG_SetLerpFrameAnimation

may include ANIM_TOGGLEBIT
===============
*/
#ifndef SMOKINGUNS
static void CG_SetLerpFrameAnimation( clientInfo_t *ci, lerpFrame_t *lf, int newAnimation ) {
#else
static void CG_SetLerpFrameAnimation( clientInfo_t *ci, lerpFrame_t *lf, int newAnimation, int weapon) {
#endif
	animation_t	*anim;

	lf->animationNumber = newAnimation;
	newAnimation &= ~ANIM_TOGGLEBIT;

	if ( newAnimation < 0 || newAnimation >= MAX_TOTALANIMATIONS ) {
		CG_Error( "Bad animation number: %i", newAnimation );
	}

#ifndef SMOKINGUNS
	anim = &ci->animations[ newAnimation ];
#else
	if(weapon) {
		anim = &bg_weaponlist[weapon].animations[ newAnimation ];
	} else {
		anim = &ci->animations[ newAnimation ];
	}
#endif

	lf->animation = anim;
	lf->animationTime = lf->frameTime + anim->initialLerp;

#ifdef SMOKINGUNS
	if(weapon && !anim->initialLerp &&
		(cg.weapon != cg.weaponold || cg.weapon == WP_GATLING)){
		if ( anim->reversed ) {
			lf->frame = anim->firstFrame + anim->numFrames - 1;
		}
		else {
			lf->frame = anim->firstFrame;
		}
		lf->oldFrame = lf->frame;
	}
#endif

	if ( cg_debugAnim.integer ) {
		CG_Printf( "Anim: %i\n", newAnimation );
	}
}

/*
===============
CG_RunLerpFrame

Sets cg.snap, cg.oldFrame, and cg.backlerp
cg.time should be between oldFrameTime and frameTime after exit
===============
*/
#ifndef SMOKINGUNS
static void CG_RunLerpFrame( clientInfo_t *ci, lerpFrame_t *lf, int newAnimation, float speedScale ) {
#else
void CG_RunLerpFrame( clientInfo_t *ci, lerpFrame_t *lf, int newAnimation, float speedScale, int weapon) {
#endif
	int			f, numFrames;
	animation_t	*anim;

	// debugging tool to get no animations
	if ( cg_animSpeed.integer == 0 ) {
		lf->oldFrame = lf->frame = lf->backlerp = 0;
		return;
	}

	// see if the animation sequence is switching
	if ( newAnimation != lf->animationNumber || !lf->animation ) {
#ifndef SMOKINGUNS
		CG_SetLerpFrameAnimation( ci, lf, newAnimation );
#else
		CG_SetLerpFrameAnimation( ci, lf, newAnimation, weapon);
#endif
	}

	// if we have passed the current frame, move it to
	// oldFrame and calculate a new frame
	if ( cg.time >= lf->frameTime ) {
		lf->oldFrame = lf->frame;
		lf->oldFrameTime = lf->frameTime;

		// get the next frame based on the animation
		anim = lf->animation;
		if ( !anim->frameLerp ) {
			return;		// shouldn't happen
		}
		if ( cg.time < lf->animationTime ) {
			lf->frameTime = lf->animationTime;		// initial lerp
		} else {
			lf->frameTime = lf->oldFrameTime + anim->frameLerp;
		}
		f = ( lf->frameTime - lf->animationTime ) / anim->frameLerp;
		f *= speedScale;		// adjust for haste, etc

		numFrames = anim->numFrames;
		if (anim->flipflop) {
			numFrames *= 2;
		}
		if ( f >= numFrames ) {
			f -= numFrames;
			if ( anim->loopFrames ) {
				f %= anim->loopFrames;
				f += anim->numFrames - anim->loopFrames;
			} else {
				f = numFrames - 1;
				// the animation is stuck at the end, so it
				// can immediately transition to another sequence
				lf->frameTime = cg.time;
			}
		}
		if ( anim->reversed ) {
			lf->frame = anim->firstFrame + anim->numFrames - 1 - f;
		}
		else if (anim->flipflop && f>=anim->numFrames) {
			lf->frame = anim->firstFrame + anim->numFrames - 1 - (f%anim->numFrames);
		}
		else {
			lf->frame = anim->firstFrame + f;
		}
		if ( cg.time > lf->frameTime ) {
			lf->frameTime = cg.time;
			if ( cg_debugAnim.integer ) {
				CG_Printf( "Clamp lf->frameTime\n");
			}
		}
	}

	if ( lf->frameTime > cg.time + 200 ) {
		lf->frameTime = cg.time;
	}

	if ( lf->oldFrameTime > cg.time ) {
		lf->oldFrameTime = cg.time;
	}
	// calculate current lerp value
	if ( lf->frameTime == lf->oldFrameTime ) {
		lf->backlerp = 0;
	} else {
		lf->backlerp = 1.0 - (float)( cg.time - lf->oldFrameTime ) / ( lf->frameTime - lf->oldFrameTime );
	}
}


/*
===============
CG_ClearLerpFrame
===============
*/
#ifndef SMOKINGUNS
static void CG_ClearLerpFrame( clientInfo_t *ci, lerpFrame_t *lf, int animationNumber ) {
#else
void CG_ClearLerpFrame( clientInfo_t *ci, lerpFrame_t *lf, int animationNumber ) {
#endif
	lf->frameTime = lf->oldFrameTime = cg.time;
#ifndef SMOKINGUNS
	CG_SetLerpFrameAnimation( ci, lf, animationNumber );
#else
	CG_SetLerpFrameAnimation( ci, lf, animationNumber, 0);
#endif
	lf->oldFrame = lf->frame = lf->animation->firstFrame;
}


/*
===============
CG_PlayerAnimation
===============
*/
static void CG_PlayerAnimation( centity_t *cent, int *legsOld, int *legs, float *legsBackLerp,
						int *torsoOld, int *torso, float *torsoBackLerp ) {
	clientInfo_t	*ci;
	int				clientNum;
#ifndef SMOKINGUNS
	float			speedScale;
#else
	const float		speedScale = 1.0f;
#endif

	clientNum = cent->currentState.clientNum;

	if ( cg_noPlayerAnims.integer ) {
		*legsOld = *legs = *torsoOld = *torso = 0;
		return;
	}

#ifndef SMOKINGUNS
	if ( cent->currentState.powerups & ( 1 << PW_HASTE ) ) {
		speedScale = 1.5;
	} else {
		speedScale = 1;
	}
#endif

	ci = &cgs.clientinfo[ clientNum ];

	// do the shuffle turn frames locally
	if ( cent->pe.legs.yawing && ( cent->currentState.legsAnim & ~ANIM_TOGGLEBIT ) == LEGS_IDLE ) {
#ifndef SMOKINGUNS
		CG_RunLerpFrame( ci, &cent->pe.legs, LEGS_TURN, speedScale );
#else
		CG_RunLerpFrame( ci, &cent->pe.legs, LEGS_TURN, speedScale, 0);
#endif
	} else {
#ifndef SMOKINGUNS
		CG_RunLerpFrame( ci, &cent->pe.legs, cent->currentState.legsAnim, speedScale );
#else
		CG_RunLerpFrame( ci, &cent->pe.legs, cent->currentState.legsAnim, speedScale, 0);
#endif
	}

	*legsOld = cent->pe.legs.oldFrame;
	*legs = cent->pe.legs.frame;
	*legsBackLerp = cent->pe.legs.backlerp;

#ifndef SMOKINGUNS
	CG_RunLerpFrame( ci, &cent->pe.torso, cent->currentState.torsoAnim, speedScale );
#else
	CG_RunLerpFrame( ci, &cent->pe.torso, cent->currentState.torsoAnim, speedScale, 0);
#endif

	*torsoOld = cent->pe.torso.oldFrame;
	*torso = cent->pe.torso.frame;
	*torsoBackLerp = cent->pe.torso.backlerp;

#ifdef SMOKINGUNS
	// look if player is using a gatling, if yes the idle-anim will be modified
	if(cent->currentState.weapon == WP_GATLING &&
		(cent->currentState.torsoAnim & ~ANIM_TOGGLEBIT) == TORSO_GATLING_STAND){
		int i;

		for(i = 0; i < MAX_GENTITIES; i++){
			centity_t *t = &cg_entities[i];

			if(t->currentState.eType == ET_TURRET &&
				t->currentState.eventParm == cent->currentState.clientNum){
				float angle, factor, backangle;
				int	frame, oldframe;
				float backlerp;
				float f_frame, stage;
				animation_t *anim = &cgs.clientinfo[cent->currentState.clientNum].animations[TORSO_GATLING_FIRE];
				stage = 360/(anim->numFrames-1);

				// we found a gatling
				angle = CG_MachinegunSpinAngle(cent);
				angle = AngleNormalize360(angle);

				// find frames
				factor = angle/360.0f;

				frame = (int)(factor*(anim->numFrames-1));
				f_frame = factor*(anim->numFrames-1);

				if(frame < f_frame){
					frame++;
				}
				oldframe = frame-1;

				// find backlerp
				backangle = angle-oldframe*stage;
				backlerp = 1-backangle/stage;

				*torsoOld = oldframe + anim->firstFrame;
				*torso = frame + anim->firstFrame;
				*torsoBackLerp = backlerp;
				return;
			}
		}
	}

	//CG_Printf("cg %i, %i, %f, %i\n", cent->pe.torso.frame, cent->pe.torso.oldFrame, cent->pe.torso.backlerp, cent->currentState.torsoAnim);
#endif
}

/*
=============================================================================

PLAYER ANGLES

=============================================================================
*/

/*
==================
CG_SwingAngles
==================
*/
static void CG_SwingAngles( float destination, float swingTolerance, float clampTolerance,
					float speed, float *angle, qboolean *swinging ) {
	float	swing;
	float	move;
	float	scale;

	if ( !*swinging ) {
		// see if a swing should be started
		swing = AngleSubtract( *angle, destination );
		if ( swing > swingTolerance || swing < -swingTolerance ) {
			*swinging = qtrue;
		}
	}

	if ( !*swinging ) {
		return;
	}

	// modify the speed depending on the delta
	// so it doesn't seem so linear
	swing = AngleSubtract( destination, *angle );
	scale = fabs( swing );
	if ( scale < swingTolerance * 0.5 ) {
		scale = 0.5;
	} else if ( scale < swingTolerance ) {
		scale = 1.0;
	} else {
		scale = 2.0;
	}

	// swing towards the destination angle
	if ( swing >= 0 ) {
		move = cg.frametime * scale * speed;
		if ( move >= swing ) {
			move = swing;
			*swinging = qfalse;
		}
		*angle = AngleMod( *angle + move );
	} else if ( swing < 0 ) {
		move = cg.frametime * scale * -speed;
		if ( move <= swing ) {
			move = swing;
			*swinging = qfalse;
		}
		*angle = AngleMod( *angle + move );
	}

	// clamp to no more than tolerance
	swing = AngleSubtract( destination, *angle );
	if ( swing > clampTolerance ) {
		*angle = AngleMod( destination - (clampTolerance - 1) );
	} else if ( swing < -clampTolerance ) {
		*angle = AngleMod( destination + (clampTolerance - 1) );
	}
}

/*
=================
CG_AddPainTwitch
=================
*/
static void CG_AddPainTwitch( centity_t *cent, vec3_t torsoAngles ) {
	int		t;
	float	f;

	t = cg.time - cent->pe.painTime;
	if ( t >= PAIN_TWITCH_TIME ) {
		return;
	}

	f = 1.0 - (float)t / PAIN_TWITCH_TIME;

	if ( cent->pe.painDirection ) {
		torsoAngles[ROLL] += 20 * f;
	} else {
		torsoAngles[ROLL] -= 20 * f;
	}
}


/*
===============
CG_PlayerAngles

Handles seperate torso motion

  legs pivot based on direction of movement

  head always looks exactly at cent->lerpAngles

  if motion < 20 degrees, show in head only
  if < 45 degrees, also show in torso
===============
*/
static void CG_PlayerAngles( centity_t *cent, vec3_t legs[3], vec3_t torso[3], vec3_t head[3] ) {
	vec3_t		legsAngles, torsoAngles, headAngles;
	float		dest;
	static	int	movementOffsets[8] = { 0, 22, 45, -22, 0, 22, -45, -22 };
	vec3_t		velocity;
	float		speed;
#ifndef SMOKINGUNS
	int			dir, clientNum;
	clientInfo_t	*ci;
#else
	int			dir;
#endif

	VectorCopy( cent->lerpAngles, headAngles );
	headAngles[YAW] = AngleMod( headAngles[YAW] );
	VectorClear( legsAngles );
	VectorClear( torsoAngles );

	// --------- yaw -------------

	// allow yaw to drift a bit
	if ( ( cent->currentState.legsAnim & ~ANIM_TOGGLEBIT ) != LEGS_IDLE 
#ifndef SMOKINGUNS
		|| ((cent->currentState.torsoAnim & ~ANIM_TOGGLEBIT) != TORSO_STAND 
		&& (cent->currentState.torsoAnim & ~ANIM_TOGGLEBIT) != TORSO_STAND2)) {
#else
		|| ( // Tequila comment: Fixed a wrong test here generating gcc warning
		( cent->currentState.torsoAnim & ~ANIM_TOGGLEBIT ) != TORSO_KNIFE_STAND
		&& ( cent->currentState.torsoAnim & ~ANIM_TOGGLEBIT ) != TORSO_PISTOL_STAND
		&& ( cent->currentState.torsoAnim & ~ANIM_TOGGLEBIT ) != TORSO_PISTOLS_STAND
		&& ( cent->currentState.torsoAnim & ~ANIM_TOGGLEBIT ) != TORSO_RIFLE_STAND
		&& ( cent->currentState.torsoAnim & ~ANIM_TOGGLEBIT ) != TORSO_GATLING_STAND
		&& ( cent->currentState.torsoAnim & ~ANIM_TOGGLEBIT ) != TORSO_HOLSTERED)) {
#endif
		// if not standing still, always point all in the same direction
		cent->pe.torso.yawing = qtrue;	// always center
		cent->pe.torso.pitching = qtrue;	// always center
		cent->pe.legs.yawing = qtrue;	// always center
	}

	// adjust legs for movement dir
	if ( cent->currentState.eFlags & EF_DEAD ) {
		// don't let dead bodies twitch
		dir = 0;
	} else {
		dir = cent->currentState.angles2[YAW];
		if ( dir < 0 || dir > 7 ) {
#ifndef SMOKINGUNS
			CG_Error( "Bad player movement angle" );
#else
			dir = 0;
#endif
		}
	}
	legsAngles[YAW] = headAngles[YAW] + movementOffsets[ dir ];
	torsoAngles[YAW] = headAngles[YAW] + 0.25 * movementOffsets[ dir ];

	// torso
	CG_SwingAngles( torsoAngles[YAW], 25, 90, cg_swingSpeed.value, &cent->pe.torso.yawAngle, &cent->pe.torso.yawing );
	CG_SwingAngles( legsAngles[YAW], 40, 90, cg_swingSpeed.value, &cent->pe.legs.yawAngle, &cent->pe.legs.yawing );

	torsoAngles[YAW] = cent->pe.torso.yawAngle;
	legsAngles[YAW] = cent->pe.legs.yawAngle;


	// --------- pitch -------------

	// only show a fraction of the pitch angle in the torso
	if ( headAngles[PITCH] > 180 ) {
		dest = (-360 + headAngles[PITCH]) * 0.75f;
	} else {
		dest = headAngles[PITCH] * 0.75f;
	}
	CG_SwingAngles( dest, 15, 30, 0.1f, &cent->pe.torso.pitchAngle, &cent->pe.torso.pitching );
	torsoAngles[PITCH] = cent->pe.torso.pitchAngle;

	//
#ifndef SMOKINGUNS
	clientNum = cent->currentState.clientNum;
	if ( clientNum >= 0 && clientNum < MAX_CLIENTS ) {
		ci = &cgs.clientinfo[ clientNum ];
		if ( ci->fixedtorso ) {
			torsoAngles[PITCH] = 0.0f;
		}
	}
#endif

	// --------- roll -------------


	// lean towards the direction of travel
	VectorCopy( cent->currentState.pos.trDelta, velocity );
	speed = VectorNormalize( velocity );
	if ( speed ) {
		vec3_t	axis[3];
		float	side;

		speed *= 0.05f;

		AnglesToAxis( legsAngles, axis );
		side = speed * DotProduct( velocity, axis[1] );
		legsAngles[ROLL] -= side;

		side = speed * DotProduct( velocity, axis[0] );
		legsAngles[PITCH] += side;
	}

	//
#ifndef SMOKINGUNS
	clientNum = cent->currentState.clientNum;
	if ( clientNum >= 0 && clientNum < MAX_CLIENTS ) {
		ci = &cgs.clientinfo[ clientNum ];
		if ( ci->fixedlegs ) {
			legsAngles[YAW] = torsoAngles[YAW];
			legsAngles[PITCH] = 0.0f;
			legsAngles[ROLL] = 0.0f;
		}
	}
#endif

	// pain twitch
	CG_AddPainTwitch( cent, torsoAngles );

	// pull the angles back out of the hierarchial chain
	AnglesSubtract( headAngles, torsoAngles, headAngles );
	AnglesSubtract( torsoAngles, legsAngles, torsoAngles );

#ifdef SMOKINGUNS
	if ( cent->currentState.eFlags & EF_DEAD ){
		VectorClear(torsoAngles);
		VectorClear(headAngles);
	}

	if((cent->currentState.torsoAnim &~ANIM_TOGGLEBIT) == BOTH_LADDER ||
		(cent->currentState.torsoAnim &~ANIM_TOGGLEBIT) == BOTH_LADDER_STAND)
		torsoAngles[PITCH] = 0;
#endif

	AnglesToAxis( legsAngles, legs );
	AnglesToAxis( torsoAngles, torso );
	AnglesToAxis( headAngles, head );
}


//==========================================================================

/*
===============
CG_HasteTrail
===============
*/
#ifndef SMOKINGUNS
static void CG_HasteTrail( centity_t *cent ) {
	localEntity_t	*smoke;
	vec3_t			origin;
	int				anim;

	if ( cent->trailTime > cg.time ) {
		return;
	}
	anim = cent->pe.legs.animationNumber & ~ANIM_TOGGLEBIT;
	if ( anim != LEGS_RUN && anim != LEGS_BACK ) {
		return;
	}

	cent->trailTime += 100;
	if ( cent->trailTime < cg.time ) {
		cent->trailTime = cg.time;
	}

	VectorCopy( cent->lerpOrigin, origin );
	origin[2] -= 16;

	smoke = CG_SmokePuff( origin, vec3_origin,
				  8,
				  1, 1, 1, 1,
				  500,
				  cg.time,
				  0,
				  0,
				  cgs.media.hastePuffShader );

	// use the optimized local entity add
	smoke->leType = LE_SCALE_FADE;
}
#endif

#ifdef SMOKINGUNS
/*
===============
CG_BreathPuffs
===============
*/
static void CG_BreathPuffs( centity_t *cent, refEntity_t *head) {
	clientInfo_t *ci;
	vec3_t up, origin;
	int contents;

	ci = &cgs.clientinfo[ cent->currentState.number ];

	if (!cg_enableBreath.integer) {
		return;
	}
	if ( cent->currentState.number == cg.snap->ps.clientNum && !cg.renderingThirdPerson) {
		return;
	}
	if ( cent->currentState.eFlags & EF_DEAD ) {
		return;
	}
	contents = CG_PointContents( head->origin, 0 );
	if ( contents & ( CONTENTS_WATER | CONTENTS_SLIME | CONTENTS_LAVA ) ) {
		return;
	}
	if ( ci->breathPuffTime > cg.time ) {
		return;
	}

	VectorSet( up, 0, 0, 8 );
	VectorMA(head->origin, 8, head->axis[0], origin);
	VectorMA(origin, -4, head->axis[2], origin);
	CG_SmokePuff( origin, up, 16, 1, 1, 1, 0.66f, 1500, cg.time, cg.time + 400, LEF_PUFF_DONT_SCALE, cgs.media.shotgunSmokePuffShader );
	ci->breathPuffTime = cg.time + 2000;
}

/*
===============
CG_DustTrail
===============
*/
static void CG_DustTrail( centity_t *cent ) {
	int				anim;
	vec3_t end, vel;
	trace_t tr;

	if (!cg_enableDust.integer)
		return;

	if ( cent->dustTrailTime > cg.time ) {
		return;
	}

	anim = cent->pe.legs.animationNumber & ~ANIM_TOGGLEBIT;
#ifndef SMOKINGUNS
	if ( anim != LEGS_LANDB && anim != LEGS_LAND ) {
#else
	if ( anim != BOTH_LAND ) {
#endif
		return;
	}

	cent->dustTrailTime += 40;
	if ( cent->dustTrailTime < cg.time ) {
		cent->dustTrailTime = cg.time;
	}

	VectorCopy(cent->currentState.pos.trBase, end);
	end[2] -= 64;
	CG_Trace( &tr, cent->currentState.pos.trBase, NULL, NULL, end, cent->currentState.number, MASK_PLAYERSOLID );

	if ( !(tr.surfaceFlags & SURF_DUST) )
		return;

	VectorCopy( cent->currentState.pos.trBase, end );
	end[2] -= 16;

	VectorSet(vel, 0, 0, -30);
	CG_SmokePuff( end, vel,
				  24,
				  .8f, .8f, 0.7f, 0.33f,
				  500,
				  cg.time,
				  0,
				  0,
				  cgs.media.dustPuffShader );
}

#endif

/*
===============
CG_TrailItem
===============
*/
#ifndef SMOKINGUNS
static void CG_TrailItem( centity_t *cent, qhandle_t hModel ) {
	refEntity_t		ent;
	vec3_t			angles;
	vec3_t			axis[3];

	VectorCopy( cent->lerpAngles, angles );
	angles[PITCH] = 0;
	angles[ROLL] = 0;
	AnglesToAxis( angles, axis );

	memset( &ent, 0, sizeof( ent ) );
	VectorMA( cent->lerpOrigin, -16, axis[0], ent.origin );
	ent.origin[2] += 16;
	angles[YAW] += 90;
	AnglesToAxis( angles, ent.axis );

	ent.hModel = hModel;
	trap_R_AddRefEntityToScene( &ent );
}
#endif


/*
===============
CG_PlayerFlag
===============
*/
#ifndef SMOKINGUNS
static void CG_PlayerFlag( centity_t *cent, qhandle_t hSkin, refEntity_t *torso ) {
	clientInfo_t	*ci;
	refEntity_t	pole;
	refEntity_t	flag;
	vec3_t		angles, dir;
	int			legsAnim, flagAnim, updateangles;
	float		angle, d;

	// show the flag pole model
	memset( &pole, 0, sizeof(pole) );
	pole.hModel = cgs.media.flagPoleModel;
	VectorCopy( torso->lightingOrigin, pole.lightingOrigin );
	pole.shadowPlane = torso->shadowPlane;
	pole.renderfx = torso->renderfx;
	CG_PositionEntityOnTag( &pole, torso, torso->hModel, "tag_flag" );
	trap_R_AddRefEntityToScene( &pole );

	// show the flag model
	memset( &flag, 0, sizeof(flag) );
	flag.hModel = cgs.media.flagFlapModel;
	flag.customSkin = hSkin;
	VectorCopy( torso->lightingOrigin, flag.lightingOrigin );
	flag.shadowPlane = torso->shadowPlane;
	flag.renderfx = torso->renderfx;

	VectorClear(angles);

	updateangles = qfalse;
	legsAnim = cent->currentState.legsAnim & ~ANIM_TOGGLEBIT;
	if( legsAnim == LEGS_IDLE || legsAnim == LEGS_IDLECR ) {
		flagAnim = FLAG_STAND;
	} else if ( legsAnim == LEGS_WALK || legsAnim == LEGS_WALKCR ) {
		flagAnim = FLAG_STAND;
		updateangles = qtrue;
	} else {
		flagAnim = FLAG_RUN;
		updateangles = qtrue;
	}

	if ( updateangles ) {

		VectorCopy( cent->currentState.pos.trDelta, dir );
		// add gravity
		dir[2] += 100;
		VectorNormalize( dir );
		d = DotProduct(pole.axis[2], dir);
		// if there is enough movement orthogonal to the flag pole
		if (fabs(d) < 0.9) {
			//
			d = DotProduct(pole.axis[0], dir);
			if (d > 1.0f) {
				d = 1.0f;
			}
			else if (d < -1.0f) {
				d = -1.0f;
			}
			angle = acos(d);

			d = DotProduct(pole.axis[1], dir);
			if (d < 0) {
				angles[YAW] = 360 - angle * 180 / M_PI;
			}
			else {
				angles[YAW] = angle * 180 / M_PI;
			}
			if (angles[YAW] < 0)
				angles[YAW] += 360;
			if (angles[YAW] > 360)
				angles[YAW] -= 360;

			//vectoangles( cent->currentState.pos.trDelta, tmpangles );
			//angles[YAW] = tmpangles[YAW] + 45 - cent->pe.torso.yawAngle;
			// change the yaw angle
			CG_SwingAngles( angles[YAW], 25, 90, 0.15f, &cent->pe.flag.yawAngle, &cent->pe.flag.yawing );
		}

		/*
		d = DotProduct(pole.axis[2], dir);
		angle = Q_acos(d);

		d = DotProduct(pole.axis[1], dir);
		if (d < 0) {
			angle = 360 - angle * 180 / M_PI;
		}
		else {
			angle = angle * 180 / M_PI;
		}
		if (angle > 340 && angle < 20) {
			flagAnim = FLAG_RUNUP;
		}
		if (angle > 160 && angle < 200) {
			flagAnim = FLAG_RUNDOWN;
		}
		*/
	}

	// set the yaw angle
	angles[YAW] = cent->pe.flag.yawAngle;
	// lerp the flag animation frames
	ci = &cgs.clientinfo[ cent->currentState.clientNum ];
	CG_RunLerpFrame( ci, &cent->pe.flag, flagAnim, 1 );
	flag.oldframe = cent->pe.flag.oldFrame;
	flag.frame = cent->pe.flag.frame;
	flag.backlerp = cent->pe.flag.backlerp;

	AnglesToAxis( angles, flag.axis );
	CG_PositionRotatedEntityOnTag( &flag, &pole, pole.hModel, "tag_flag" );

	trap_R_AddRefEntityToScene( &flag );
}
#endif


#ifndef SMOKINGUNS
/*
===============
CG_PlayerTokens
===============
*/
static void CG_PlayerTokens( centity_t *cent, int renderfx ) {
	int			tokens, i, j;
	float		angle;
	refEntity_t	ent;
	vec3_t		dir, origin;
	skulltrail_t *trail;
	trail = &cg.skulltrails[cent->currentState.number];
	tokens = cent->currentState.generic1;
	if ( !tokens ) {
		trail->numpositions = 0;
		return;
	}

	if ( tokens > MAX_SKULLTRAIL ) {
		tokens = MAX_SKULLTRAIL;
	}

	// add skulls if there are more than last time
	for (i = 0; i < tokens - trail->numpositions; i++) {
		for (j = trail->numpositions; j > 0; j--) {
			VectorCopy(trail->positions[j-1], trail->positions[j]);
		}
		VectorCopy(cent->lerpOrigin, trail->positions[0]);
	}
	trail->numpositions = tokens;

	// move all the skulls along the trail
	VectorCopy(cent->lerpOrigin, origin);
	for (i = 0; i < trail->numpositions; i++) {
		VectorSubtract(trail->positions[i], origin, dir);
		if (VectorNormalize(dir) > 30) {
			VectorMA(origin, 30, dir, trail->positions[i]);
		}
		VectorCopy(trail->positions[i], origin);
	}

	memset( &ent, 0, sizeof( ent ) );
	if( cgs.clientinfo[ cent->currentState.clientNum ].team == TEAM_BLUE ) {
		ent.hModel = cgs.media.redCubeModel;
	} else {
		ent.hModel = cgs.media.blueCubeModel;
	}
	ent.renderfx = renderfx;

	VectorCopy(cent->lerpOrigin, origin);
	for (i = 0; i < trail->numpositions; i++) {
		VectorSubtract(origin, trail->positions[i], ent.axis[0]);
		ent.axis[0][2] = 0;
		VectorNormalize(ent.axis[0]);
		VectorSet(ent.axis[2], 0, 0, 1);
		CrossProduct(ent.axis[0], ent.axis[2], ent.axis[1]);

		VectorCopy(trail->positions[i], ent.origin);
		angle = (((cg.time + 500 * MAX_SKULLTRAIL - 500 * i) / 16) & 255) * (M_PI * 2) / 255;
		ent.origin[2] += sin(angle) * 10;
		trap_R_AddRefEntityToScene( &ent );
		VectorCopy(trail->positions[i], origin);
	}
}
#endif


/*
===============
CG_PlayerPowerups
===============
*/
#ifndef SMOKINGUNS
static void CG_PlayerPowerups( centity_t *cent, refEntity_t *torso ) {
	int		powerups;
	clientInfo_t	*ci;

	powerups = cent->currentState.powerups;
	if ( !powerups ) {
		return;
	}

	// quad gives a dlight
	if ( powerups & ( 1 << PW_QUAD ) ) {
		trap_R_AddLightToScene( cent->lerpOrigin, 200 + (rand()&31), 0.2f, 0.2f, 1 );
	}

	// flight plays a looped sound
	if ( powerups & ( 1 << PW_FLIGHT ) ) {
		trap_S_AddLoopingSound( cent->currentState.number, cent->lerpOrigin, vec3_origin, cgs.media.flightSound );
	}

	ci = &cgs.clientinfo[ cent->currentState.clientNum ];
	// redflag
	if ( powerups & ( 1 << PW_REDFLAG ) ) {
		if (ci->newAnims) {
			CG_PlayerFlag( cent, cgs.media.redFlagFlapSkin, torso );
		}
		else {
			CG_TrailItem( cent, cgs.media.redFlagModel );
		}
		trap_R_AddLightToScene( cent->lerpOrigin, 200 + (rand()&31), 1.0, 0.2f, 0.2f );
	}

	// blueflag
	if ( powerups & ( 1 << PW_BLUEFLAG ) ) {
		if (ci->newAnims){
			CG_PlayerFlag( cent, cgs.media.blueFlagFlapSkin, torso );
		}
		else {
			CG_TrailItem( cent, cgs.media.blueFlagModel );
		}
		trap_R_AddLightToScene( cent->lerpOrigin, 200 + (rand()&31), 0.2f, 0.2f, 1.0 );
	}

	// neutralflag
	if ( powerups & ( 1 << PW_NEUTRALFLAG ) ) {
		if (ci->newAnims) {
			CG_PlayerFlag( cent, cgs.media.neutralFlagFlapSkin, torso );
		}
		else {
			CG_TrailItem( cent, cgs.media.neutralFlagModel );
		}
		trap_R_AddLightToScene( cent->lerpOrigin, 200 + (rand()&31), 1.0, 1.0, 1.0 );
	}

	// haste leaves smoke trails
	if ( powerups & ( 1 << PW_HASTE ) ) {
		CG_HasteTrail( cent );
	}
}
#endif


/*
===============
CG_PlayerFloatSprite

Float a sprite over the player's head
===============
*/
static void CG_PlayerFloatSprite( centity_t *cent, qhandle_t shader ) {
	int				rf;
	refEntity_t		ent;

#ifndef SMOKINGUNS
	if ( cent->currentState.number == cg.snap->ps.clientNum && !cg.renderingThirdPerson ) {
#else
	if ( (cent->currentState.number == cg.snap->ps.clientNum && !cg.renderingThirdPerson) ||
		cg.snap->ps.persistant[PERS_TEAM] >= TEAM_SPECTATOR) {
#endif
		rf = RF_THIRD_PERSON;		// only show in mirrors
	} else {
		rf = 0;
	}

	memset( &ent, 0, sizeof( ent ) );
	VectorCopy( cent->lerpOrigin, ent.origin );
	ent.origin[2] += 48;
	ent.reType = RT_SPRITE;
	ent.customShader = shader;
	ent.radius = 10;
	ent.renderfx = rf;
	ent.shaderRGBA[0] = 255;
	ent.shaderRGBA[1] = 255;
	ent.shaderRGBA[2] = 255;
	ent.shaderRGBA[3] = 255;
	trap_R_AddRefEntityToScene( &ent );
}



/*
===============
CG_PlayerSprites

Float sprites over the player's head
===============
*/
static void CG_PlayerSprites( centity_t *cent ) {
	int		team;

	if ( cent->currentState.eFlags & EF_CONNECTION ) {
		CG_PlayerFloatSprite( cent, cgs.media.connectionShader );
		return;
	}

#ifdef SMOKINGUNS
	if( cent->currentState.eFlags & EF_BUY ) {
		CG_PlayerFloatSprite( cent, cgs.media.balloonBuyShader );
		return;
	}
#endif

	if ( cent->currentState.eFlags & EF_TALK ) {
		CG_PlayerFloatSprite( cent, cgs.media.balloonShader );
		return;
	}

	if ( cent->currentState.eFlags & EF_AWARD_IMPRESSIVE ) {
		CG_PlayerFloatSprite( cent, cgs.media.medalImpressive );
		return;
	}

	if ( cent->currentState.eFlags & EF_AWARD_EXCELLENT ) {
		CG_PlayerFloatSprite( cent, cgs.media.medalExcellent );
		return;
	}

	if ( cent->currentState.eFlags & EF_AWARD_GAUNTLET ) {
		CG_PlayerFloatSprite( cent, cgs.media.medalGauntlet );
		return;
	}

	if ( cent->currentState.eFlags & EF_AWARD_DEFEND ) {
		CG_PlayerFloatSprite( cent, cgs.media.medalDefend );
		return;
	}

	if ( cent->currentState.eFlags & EF_AWARD_ASSIST ) {
		CG_PlayerFloatSprite( cent, cgs.media.medalAssist );
		return;
	}

	if ( cent->currentState.eFlags & EF_AWARD_CAP ) {
		CG_PlayerFloatSprite( cent, cgs.media.medalCapture );
		return;
	}

	team = cgs.clientinfo[ cent->currentState.clientNum ].team;
	if ( !(cent->currentState.eFlags & EF_DEAD) &&
		cg.snap->ps.persistant[PERS_TEAM] == team &&
		cgs.gametype >= GT_TEAM) {
#ifndef SMOKINGUNS
		if (cg_drawFriend.integer) {
			CG_PlayerFloatSprite( cent, cgs.media.friendShader );
		}
#endif
		return;
	}
}

/*
===============
CG_PlayerShadow

Returns the Z component of the surface being shadowed

  should it return a full plane instead of a Z?
===============
*/
#define	SHADOW_DISTANCE		128
#ifndef SMOKINGUNS
static qboolean CG_PlayerShadow( centity_t *cent, float *shadowPlane ) {
#else
static qboolean CG_PlayerShadow( centity_t *cent, float *shadowPlane, vec3_t pos ) {
#endif
	vec3_t		end, mins = {-15, -15, 0}, maxs = {15, 15, 2};
	trace_t		trace;
	float		alpha;

	*shadowPlane = 0;

	if ( cg_shadows.integer == 0 ) {
		return qfalse;
	}

	// no shadows when invisible
#ifndef SMOKINGUNS
	if ( cent->currentState.powerups & ( 1 << PW_INVIS ) ) {
		return qfalse;
	}
#endif

	// send a trace down from the player to the ground
#ifndef SMOKINGUNS
	VectorCopy( cent->lerpOrigin, end );
#else
	VectorCopy( pos, end );
#endif
	end[2] -= SHADOW_DISTANCE;

#ifndef SMOKINGUNS
	trap_CM_BoxTrace( &trace, cent->lerpOrigin, end, mins, maxs, 0, MASK_PLAYERSOLID );
#else
	trap_CM_BoxTrace_New( &trace, pos, end, mins, maxs, 0, MASK_PLAYERSOLID );
#endif

	// no shadow if too high
	if ( trace.fraction == 1.0 || trace.startsolid || trace.allsolid ) {
		return qfalse;
	}

	*shadowPlane = trace.endpos[2] + 1;

	if ( cg_shadows.integer != 1 ) {	// no mark for stencil or projection shadows
		return qtrue;
	}

	// fade the shadow out with height
	alpha = 1.0 - trace.fraction;

	// hack / FPE - bogus planes?
	//assert( DotProduct( trace.plane.normal, trace.plane.normal ) != 0.0f )

	// add the mark as a temporary, so it goes directly to the renderer
	// without taking a spot in the cg_marks array
	CG_ImpactMark( cgs.media.shadowMarkShader, trace.endpos, trace.plane.normal,
#ifndef SMOKINGUNS
		cent->pe.legs.yawAngle, alpha,alpha,alpha,1, qfalse, 24, qtrue );
#else
		cent->pe.legs.yawAngle, alpha,alpha,alpha,1, qfalse, 24, qtrue, -1 );
#endif

	return qtrue;
}


/*
===============
CG_PlayerSplash

Draw a mark at the water surface
===============
*/
static void CG_PlayerSplash( centity_t *cent ) {
	vec3_t		start, end;
	trace_t		trace;
	int			contents;
	polyVert_t	verts[4];

	if ( !cg_shadows.integer ) {
		return;
	}

	VectorCopy( cent->lerpOrigin, end );
	end[2] -= 24;

	// if the feet aren't in liquid, don't make a mark
	// this won't handle moving water brushes, but they wouldn't draw right anyway...
	contents = CG_PointContents( end, 0 );
	if ( !( contents & ( CONTENTS_WATER | CONTENTS_SLIME | CONTENTS_LAVA ) ) ) {
		return;
	}

	VectorCopy( cent->lerpOrigin, start );
	start[2] += 32;

	// if the head isn't out of liquid, don't make a mark
	contents = CG_PointContents( start, 0 );
	if ( contents & ( CONTENTS_SOLID | CONTENTS_WATER | CONTENTS_SLIME | CONTENTS_LAVA ) ) {
		return;
	}

	// trace down to find the surface
#ifndef SMOKINGUNS
	trap_CM_BoxTrace( &trace, start, end, NULL, NULL, 0, ( CONTENTS_WATER | CONTENTS_SLIME | CONTENTS_LAVA ) );
#else
	trap_CM_BoxTrace_New( &trace, start, end, NULL, NULL, 0, ( CONTENTS_WATER | CONTENTS_SLIME | CONTENTS_LAVA ) );
#endif

	if ( trace.fraction == 1.0 ) {
		return;
	}

	// create a mark polygon
	VectorCopy( trace.endpos, verts[0].xyz );
	verts[0].xyz[0] -= 32;
	verts[0].xyz[1] -= 32;
	verts[0].st[0] = 0;
	verts[0].st[1] = 0;
	verts[0].modulate[0] = 255;
	verts[0].modulate[1] = 255;
	verts[0].modulate[2] = 255;
	verts[0].modulate[3] = 255;

	VectorCopy( trace.endpos, verts[1].xyz );
	verts[1].xyz[0] -= 32;
	verts[1].xyz[1] += 32;
	verts[1].st[0] = 0;
	verts[1].st[1] = 1;
	verts[1].modulate[0] = 255;
	verts[1].modulate[1] = 255;
	verts[1].modulate[2] = 255;
	verts[1].modulate[3] = 255;

	VectorCopy( trace.endpos, verts[2].xyz );
	verts[2].xyz[0] += 32;
	verts[2].xyz[1] += 32;
	verts[2].st[0] = 1;
	verts[2].st[1] = 1;
	verts[2].modulate[0] = 255;
	verts[2].modulate[1] = 255;
	verts[2].modulate[2] = 255;
	verts[2].modulate[3] = 255;

	VectorCopy( trace.endpos, verts[3].xyz );
	verts[3].xyz[0] += 32;
	verts[3].xyz[1] -= 32;
	verts[3].st[0] = 1;
	verts[3].st[1] = 0;
	verts[3].modulate[0] = 255;
	verts[3].modulate[1] = 255;
	verts[3].modulate[2] = 255;
	verts[3].modulate[3] = 255;

	trap_R_AddPolyToScene( cgs.media.wakeMarkShader, 4, verts );
}



/*
===============
CG_AddRefEntityWithPowerups

Adds a piece with modifications or duplications for powerups
Also called by CG_Missile for quad rockets, but nobody can tell...
===============
*/
void CG_AddRefEntityWithPowerups( refEntity_t *ent, entityState_t *state, int team ) {

#ifndef SMOKINGUNS
	if ( state->powerups & ( 1 << PW_INVIS ) ) {
		ent->customShader = cgs.media.invisShader;
		trap_R_AddRefEntityToScene( ent );
	} else {
		/*
		if ( state->eFlags & EF_KAMIKAZE ) {
			if (team == TEAM_BLUE)
				ent->customShader = cgs.media.blueKamikazeShader;
			else
				ent->customShader = cgs.media.redKamikazeShader;
			trap_R_AddRefEntityToScene( ent );
		}
		else {*/
			trap_R_AddRefEntityToScene( ent );
		//}

		if ( state->powerups & ( 1 << PW_QUAD ) )
		{
			if (team == TEAM_RED)
				ent->customShader = cgs.media.redQuadShader;
			else
				ent->customShader = cgs.media.quadShader;
			trap_R_AddRefEntityToScene( ent );
		}
		if ( state->powerups & ( 1 << PW_REGEN ) ) {
			if ( ( ( cg.time / 100 ) % 10 ) == 1 ) {
				ent->customShader = cgs.media.regenShader;
				trap_R_AddRefEntityToScene( ent );
			}
		}
		if ( state->powerups & ( 1 << PW_BATTLESUIT ) ) {
			ent->customShader = cgs.media.battleSuitShader;
			trap_R_AddRefEntityToScene( ent );
		}
	}
#else
	trap_R_AddRefEntityToScene( ent );
#endif
}

/*
=================
CG_LightVerts
=================
*/
#ifndef SMOKINGUNS
int CG_LightVerts( vec3_t normal, int numVerts, polyVert_t *verts )
{
	int				i, j;
	float			incoming;
	vec3_t			ambientLight;
	vec3_t			lightDir;
	vec3_t			directedLight;

	trap_R_LightForPoint( verts[0].xyz, ambientLight, directedLight, lightDir );

	for (i = 0; i < numVerts; i++) {
		incoming = DotProduct (normal, lightDir);
		if ( incoming <= 0 ) {
			verts[i].modulate[0] = ambientLight[0];
			verts[i].modulate[1] = ambientLight[1];
			verts[i].modulate[2] = ambientLight[2];
			verts[i].modulate[3] = 255;
			continue;
		}
		j = ( ambientLight[0] + incoming * directedLight[0] );
		if ( j > 255 ) {
			j = 255;
		}
		verts[i].modulate[0] = j;

		j = ( ambientLight[1] + incoming * directedLight[1] );
		if ( j > 255 ) {
			j = 255;
		}
		verts[i].modulate[1] = j;

		j = ( ambientLight[2] + incoming * directedLight[2] );
		if ( j > 255 ) {
			j = 255;
		}
		verts[i].modulate[2] = j;

		verts[i].modulate[3] = 255;
	}
	return qtrue;
}
#endif

/*
===============
CG_Turret
by Spoon
===============
*/
#ifdef SMOKINGUNS
void CG_Turret( centity_t *cent ){
	refEntity_t		tripod;
	refEntity_t		middle;
	refEntity_t		gatling;
	refEntity_t		barrel;
	refEntity_t		crank;
	refEntity_t		mag;
	const int		renderfx = 0;
	vec3_t			viewangles;
	vec3_t			angles;
	int				time = cg.time;
	centity_t		*player = NULL;
	int				eFlags = 0;
	qboolean		thisplayer= qfalse;
	qboolean		reload = qfalse;

	VectorCopy (vec3_origin, viewangles);
	VectorCopy (vec3_origin, angles);

	// Tequila comment: eventParm is set to -1 when gatling is free
	if (cent->currentState.eventParm >= 0 && cent->currentState.eventParm < cgs.maxclients) {
		player = &cg_entities[cent->currentState.eventParm];
		eFlags = player->currentState.eFlags;
	}

	memset( &tripod, 0, sizeof(tripod) );
	memset( &middle, 0, sizeof(middle) );
	memset( &gatling, 0, sizeof(gatling) );
	memset( &barrel, 0, sizeof(barrel) );
	memset( &crank, 0, sizeof(crank) );
	memset( &mag, 0, sizeof(mag) );

	//gatling will be picked up
	if(cent->currentState.time2 < 0){
		time *= -1;

		if(player && time <= cent->currentState.time2 - BG_AnimLength( WP_ANIM_CHANGE, WP_GATLING)
			 - TRIPOD_TIME - 3* STAGE_TIME +500){
			player->pe.weapon.weaponAnim = WP_ANIM_CHANGE;
			if(cg.snap->ps.clientNum == cent->currentState.eventParm)
				cg.weaponAnim = WP_ANIM_CHANGE;
		}
	}

	if(cent->currentState.eventParm == cg.snap->ps.clientNum){
		VectorCopy(cg.refdefViewAngles, viewangles);
		thisplayer = qtrue;

		if(cg_thirdPerson.integer && cg_thirdPersonAngle.integer &&
			!cg_nopredict.integer && !cg_synchronousClients.integer){
			VectorCopy(cg.predictedPlayerState.viewangles, viewangles);
		} else if(cg_thirdPerson.integer){
			VectorCopy(player->lerpAngles, viewangles);
		}

		if(!cg_nopredict.integer && !cg_synchronousClients.integer)
			eFlags = cg.predictedPlayerState.eFlags;
	} else if (player){
		VectorCopy(player->lerpAngles, viewangles);
	}
/*
====================================
 setting up the tripod...
====================================
*/

	tripod.hModel = cgs.media.g_tripod;
	VectorCopy( cent->lerpOrigin, tripod.origin );
	VectorCopy( cent->lerpOrigin, tripod.lightingOrigin );
	tripod.renderfx = renderfx;
	VectorCopy (tripod.origin, tripod.oldorigin);	// don't positionally lerp at all
	AnglesToAxis( cent->lerpAngles, tripod.axis );
	tripod.reType = RT_MODEL;

#define	TRIPOD_FRAMES		21

	if(cent->currentState.time2 &&
		time +  4*STAGE_TIME <= cent->currentState.time2 ){

		if( time + 4*STAGE_TIME + TRIPOD_TIME < cent->currentState.time2 ){
			// Show closed tripod is not in TRIPOD_TIME interval
			tripod.frame = TRIPOD_FRAMES ;
			tripod.oldframe = TRIPOD_FRAMES ;
			tripod.backlerp = 0.0f;
		} else {
			// Tequila comment: Calculate delta from TRIPOD_TIME start
			const int delta = time - cent->currentState.time2 + 4*STAGE_TIME + TRIPOD_TIME;
			// Convert it to step frame, but first frame is the last in the model
			const float pos = (float)delta * TRIPOD_FRAMES / TRIPOD_TIME ;
			tripod.frame = TRIPOD_FRAMES - (int)(pos-0.5f);
			if (tripod.frame<0) tripod.frame=0;
			tripod.backlerp = (tripod.oldframe > tripod.frame + 1 ) ? 1.0f : 0.0f ;
			tripod.oldframe = tripod.frame + (tripod.frame < TRIPOD_FRAMES )? 1:0 ;
			//Com_Printf("tripod: time %d ; built time: %d ; frame %d ; delta %d ; pos %.2f\n", time, cent->currentState.time2, tripod.frame, delta, pos);
		}
	}

	trap_R_AddRefEntityToScene( &tripod);

	// if the model failed, allow the default nullmodel to be displayed
	if (!tripod.hModel) {
		return;
	}


/*
====================================
 add the middle part
====================================
*/
	if( cent->currentState.time2 && !cg_thirdPerson.integer &&
		(cent->currentState.time2 - 4*STAGE_TIME - TRIPOD_TIME/2) > time)
		return;

	middle.hModel = cgs.media.g_middle;

	if (!middle.hModel) {
		return;
	}

	VectorCopy( cent->lerpOrigin, middle.lightingOrigin );
	middle.renderfx = renderfx;
	VectorCopy (middle.origin, middle.oldorigin);
	middle.reType = RT_MODEL;

	//set orientation of middle part
	VectorClear(angles);
	if( player && cgs.clientinfo[cent->currentState.eventParm].infoValid ){

		angles[YAW] = viewangles[YAW] - cent->lerpAngles[YAW];

		cent->pe.middleAngle = angles[YAW];
	} else {
		angles[YAW] = cent->pe.middleAngle;
	}

	AnglesToAxis( angles, middle.axis );

	CG_PositionRotatedEntityOnTag( &middle, &tripod, tripod.hModel, "tag_tripod");
	trap_R_AddRefEntityToScene( &middle);

/*
====================================
 add the main part
====================================
*/

	gatling.hModel = cgs.media.g_gatling;

	if (!gatling.hModel) {
		return;
	}

	VectorCopy( cent->lerpOrigin, gatling.lightingOrigin );
	gatling.renderfx = renderfx;
	VectorCopy (gatling.origin, gatling.oldorigin);
	gatling.reType = RT_MODEL;

	//set orientation of gatling part
	VectorClear(angles);
	//if player uses gatling
	if( player && cgs.clientinfo[cent->currentState.eventParm].infoValid ){
		angles[PITCH] = viewangles[PITCH];
		angles[PITCH] -= 5; //get gatling little higer
		cent->pe.gatlingAngle = angles[PITCH];
	} else {
		angles[PITCH] = cent->pe.gatlingAngle;
	}

	AnglesToAxis( angles, gatling.axis );

	CG_PositionRotatedEntityOnTag( &gatling, &middle, middle.hModel, "tag_gatling");
	trap_R_AddRefEntityToScene( &gatling);

/*
====================================
 add the barrel
====================================
*/

	barrel.hModel = cgs.media.g_barrel;

	if (!barrel.hModel) {
		return;
	}

	VectorCopy( cent->lerpOrigin, barrel.lightingOrigin );
	barrel.renderfx = renderfx;
	barrel.reType = RT_MODEL;

	VectorClear(angles);
	if( player && cgs.clientinfo[cent->currentState.eventParm].infoValid ){
		angles[ROLL] = CG_MachinegunSpinAngle(player);
		cent->pe.barrelAngle = angles[ROLL];
	} else {
		angles[ROLL] = cent->pe.barrelAngle;
	}

	AnglesToAxis( angles, barrel.axis );

	CG_PositionRotatedEntityOnTag( &barrel, &gatling, gatling.hModel, "tag_barrel");
	trap_R_AddRefEntityToScene( &barrel);

/*
====================================
 add the crank
====================================
*/

	crank.hModel = cgs.media.g_crank;

	if (!crank.hModel) {
		return;
	}

	VectorCopy( cent->lerpOrigin, crank.lightingOrigin );
	crank.renderfx = renderfx;
	crank.reType = RT_MODEL;

	VectorClear(angles);
	if( player && cgs.clientinfo[cent->currentState.eventParm].infoValid ){
		angles[PITCH] = CG_MachinegunSpinAngle(player);
		cent->pe.barrelAngle = angles[PITCH];
	} else {
		angles[PITCH] = cent->pe.barrelAngle;
	}
	AnglesToAxis( angles, crank.axis );

	CG_PositionRotatedEntityOnTag( &crank, &gatling, gatling.hModel, "tag_crank");
	trap_R_AddRefEntityToScene( &crank);

/*
====================================
 add the mag
====================================
*/

	if( cent->currentState.time2 &&
		(cent->currentState.time2 - 3*STAGE_TIME) > time)
		return;

#define	MAG_FRAMES		41

	if(cent->currentState.eventParm == cg.snap->ps.clientNum
		&& cg.snap->ps.weapon == WP_GATLING &&
		cg.snap->ps.stats[STAT_GATLING_MODE] &&
		!cg_thirdPerson.integer){
		clientInfo_t	*ci = &cgs.clientinfo[cg.snap->ps.clientNum];

		mag.hModel = cgs.media.g_mag_v;

		// when building up/down
		if(cent->currentState.time2 &&
			time < cent->currentState.time2 - 2*STAGE_TIME) {
			mag.frame = MAG_FRAMES-1;
			mag.oldframe = MAG_FRAMES;
			mag.backlerp = 0.0f;
		} else if(cent->currentState.time2 &&
				time >= cent->currentState.time2 - 2*STAGE_TIME &&
				time < cent->currentState.time2) {
			const int delta = time - cent->currentState.time2 + 2*STAGE_TIME;
			const float pos = (float)delta * MAG_FRAMES / (2*STAGE_TIME);

			mag.frame = MAG_FRAMES - (int)(pos+0.5f);
			if(mag.frame<0) mag.frame=0;

			mag.oldframe = mag.frame + 1;
			mag.backlerp = 0.0f;
			//Com_Printf("g_mag_v: time %d ; built time: %d ; frame %d ; delta %d ; pos %.2f\n", time, cent->currentState.time2, mag.frame, delta, pos);
		}
		else if ((player->currentState.torsoAnim &~ ANIM_TOGGLEBIT) == TORSO_GATLING_RELOAD) {
			//Reloading animation
			CG_RunLerpFrame( ci, &cent->pe.weapon, WP_ANIM_RELOAD, 1.0, WP_GATLING);

			mag.frame = cent->pe.weapon.frame;
			mag.oldframe = cent->pe.weapon.oldFrame;
			mag.backlerp = cent->pe.weapon.backlerp;
		}
		else {
			// Just the mag as idle animation
			mag.hModel = cgs.media.g_mag;
			CG_RunLerpFrame( ci, &cent->pe.weapon, WP_ANIM_IDLE, 1.0, WP_GATLING);
		}

	} else {
		if( player && (player->currentState.torsoAnim &~ ANIM_TOGGLEBIT) == TORSO_GATLING_RELOAD)
			reload = qtrue;
		mag.hModel = cgs.media.g_mag;
	}

	if (!mag.hModel) {
		return;
	}

	VectorCopy( cent->lerpOrigin, mag.lightingOrigin );
	mag.renderfx = renderfx;
	VectorCopy (mag.origin, mag.oldorigin);
	mag.reType = RT_MODEL;

	//set orientation of mag
	VectorClear(angles);

	AnglesToAxis( angles, mag.axis );

	CG_PositionRotatedEntityOnTag( &mag, &gatling, gatling.hModel, "tag_mag");

	if(!reload)
		trap_R_AddRefEntityToScene( &mag);

/*
====================================
 if current player is using gatling
====================================
*/
	if(cent->currentState.eventParm == cg.snap->ps.clientNum
		&& !cg_thirdPerson.integer && cg.snap->ps.weapon == WP_GATLING &&
		cg.snap->ps.stats[STAT_GATLING_MODE]){
		refEntity_t		hand;
		float angle;

		memset( &hand, 0, sizeof(hand) );

		hand.hModel = cgs.media.g_hand;

		if (!hand.hModel) {
			return;
		}

		VectorCopy( cent->lerpOrigin, hand.lightingOrigin );
		hand.renderfx = renderfx;
		hand.reType = RT_MODEL;

		//animation of hand
		angle = AngleNormalize360(angles[PITCH]);
		angle /= 6;
		hand.frame = (int)angle;

		if((float)hand.frame < angle){
			hand.frame++;
		}

		hand.oldframe = hand.frame - 1;
		if (hand.oldframe<0) hand.oldframe += 60 ;
		hand.backlerp = (float)hand.frame - angle;

		angles[PITCH] = -1*cent->pe.barrelAngle;
		angles[PITCH] -= AngleNormalize180(player->lerpAngles[PITCH]);
		AnglesToAxis( angles, hand.axis );

		CG_PositionRotatedEntityOnTag( &hand, &crank, crank.hModel, "tag_hand");
		trap_R_AddRefEntityToScene( &hand);
	}

/*
============================
 add the muzzle-flash
============================
*/
	if( !player	|| !cgs.clientinfo[cent->currentState.eventParm].infoValid ){
		return;
	}

	if (eFlags & EF_FIRING){
		refEntity_t		flash;
		weaponInfo_t	*weapon = &cg_weapons[WP_GATLING];

		memset( &flash, 0, sizeof( flash ) );
		VectorCopy( barrel.lightingOrigin, flash.lightingOrigin );
		flash.shadowPlane = gatling.shadowPlane;
		flash.renderfx = gatling.renderfx;

		flash.hModel = weapon->flashModel;
		if (!flash.hModel) {
			return;
		}
		angles[YAW] = 0;
		angles[PITCH] = 0;
		angles[ROLL] = crandom() * 10;
		AnglesToAxis( angles, flash.axis );

		CG_PositionRotatedEntityOnTag( &flash, &gatling, gatling.hModel, "tag_flash");
		trap_R_AddRefEntityToScene( &flash );

		if ( weapon->flashDlightColor[0] || weapon->flashDlightColor[1] || weapon->flashDlightColor[2] ) {
			trap_R_AddLightToScene( flash.origin, 50 + (rand()&31), weapon->flashDlightColor[0],
				weapon->flashDlightColor[1], weapon->flashDlightColor[2] );
		}
	}
#define SMOKE_TIME	1000
#define DELTA_TIME	75
#define LIFE_TIME	4000

/*
===========================
 add smoke
===========================
*/

	if(	cg.time >= cg.predictedPlayerEntity.lastSmokeTime &&
		cg.predictedPlayerEntity.lastSmokeTime && cg_gunsmoke.integer && thisplayer){
		refEntity_t		re;

		if(cg.predictedPlayerEntity.lastSmokeTime - cg.predictedPlayerEntity.smokeTime > SMOKE_TIME){
			cg.predictedPlayerEntity.lastSmokeTime = 0;
			cg.predictedPlayerEntity.smokeTime = 0;
		} else
			cg.predictedPlayerEntity.lastSmokeTime += DELTA_TIME;

		// get position of tag_flash
		CG_PositionRotatedEntityOnTag( &re, &gatling, gatling.hModel, "tag_flash");

		CG_AddSmokeParticle(re.origin, 3, 4, LIFE_TIME, 5, 0, vec3_origin);
	}
}

/*
===============
CG_AddHolster
by: Spoon
adds holster with pistols to the player
===============
*/
static int CG_AddHolster(refEntity_t *holster, refEntity_t *legs, centity_t *cent,
						  clientInfo_t *ci, int h_pistol, int renderfx, float shadowPlane,
						  qboolean left){
	int i;


	if(ci->holsterSkin)
		holster->customSkin = ci->holsterSkin;

	if(left)
		holster->hModel = cgs.media.holster_l;
	else
		holster->hModel = cgs.media.holster_r;

	VectorCopy( cent->lerpOrigin, holster->lightingOrigin );

	AnglesToAxis(vec3_origin, holster->axis);

	if(left)
		CG_PositionRotatedEntityOnTag( holster, legs, ci->legsModel, "tag_holsterl");
	else
		CG_PositionRotatedEntityOnTag( holster, legs, ci->legsModel, "tag_holsterr");

	holster->shadowPlane = shadowPlane;
	holster->renderfx = renderfx;

	CG_AddRefEntityWithPowerups( holster, &cent->currentState, ci->team );

	if(cent->currentState.eFlags & EF_DEAD)
		return h_pistol;

	//add holstered weapons
	for(i=0;i<WP_NUM_WEAPONS;i++){

		if(bg_weaponlist[i].wp_sort != WPS_PISTOL)
			continue;

		if (cent->currentState.time2 & (1 << WP_AKIMBO)) {
		// The player possessed two same pistol.
		// see BG_PlayerStateToEntityState()
		// and BG_PlayerStateToEntityStateExtraPolate() about the "time2" trick

			// The player holds the two pistols.
			if (cent->currentState.weapon == i && cent->currentState.frame == i)
				continue;

			// The player holds only one pistol and has already this kind of pistol
			// in an holster.
			if (cent->currentState.weapon == i && h_pistol == i)
				continue;
		}
		else {
			// Means that the player already has this kind of pistol
			// in an holster
			if(h_pistol == i)
				continue;

			// Means that the player holds this kind of pistol,
			// on the right hand (or on the left, if on akimbo)
			if(cent->currentState.weapon == i)
				continue;

			// currentState.frame contains the pistol on the right hand, on akimbo mode.
			// On akimbo mode, the one on the left hand is currentState.weapon
			if(cent->currentState.frame == i)
				continue;
		}
/*
		hika comment: it seems that currentState.weapon is never set to WP_AKIMBO
		if(cent->currentState.weapon == WP_AKIMBO)
			continue;
*/
		if(cent->currentState.time2 & (1 << i)){
			refEntity_t	pistol;

			h_pistol = i;

			memset( &pistol, 0, sizeof(pistol) );

			pistol.hModel = cg_weapons[i].weaponModel;

			if(!pistol.hModel){
				continue;
			}

			VectorCopy( cent->lerpOrigin, pistol.lightingOrigin );

			AnglesToAxis(vec3_origin, pistol.axis);

			CG_PositionRotatedEntityOnTag( &pistol, holster, holster->hModel, "tag_weapon");

			pistol.shadowPlane = shadowPlane;
			pistol.renderfx = renderfx;

			CG_AddRefEntityWithPowerups( &pistol, &cent->currentState, ci->team );
			break;
		}
	}
	return h_pistol;
}

/*
===============
CG_AddBackWeapon
by: Spoon
adds weapon on the back
===============
*/
static void AddBackWeapon(refEntity_t *torso, centity_t *cent,
						  clientInfo_t *ci, int renderfx, float shadowPlane){
	int i;

	if(cent->currentState.eFlags & EF_DEAD)
		return;

	//add holstered weapons
	for(i=0;i<WP_NUM_WEAPONS;i++){

		if(bg_weaponlist[i].wp_sort != WPS_GUN)
			continue;

		if(cent->currentState.weapon == i)
			continue;

		if(cent->currentState.time2 & (1 << i)){
			refEntity_t	weapon;

			memset( &weapon, 0, sizeof(weapon) );

			weapon.hModel = cg_weapons[i].weaponModel;

			if(!weapon.hModel){
				continue;
			}

			VectorCopy( cent->lerpOrigin, weapon.lightingOrigin );

			AnglesToAxis(vec3_origin, weapon.axis);

			CG_PositionRotatedEntityOnTag( &weapon, torso, torso->hModel, "tag_back");

			weapon.shadowPlane = shadowPlane;
			weapon.renderfx = renderfx;

			CG_AddRefEntityWithPowerups( &weapon, &cent->currentState, ci->team );
			break;
		}
	}
}

/*
===============
CG_Fly
===============
*/
void CG_Fly( centity_t *cent) {
	// if we have a spectator, draw a fly
	refEntity_t fly;
	vec3_t right, up, angles;
	float time1;
	float time2;
	int	deltatime1;
	int deltatime2;

	if(!cgs.specsareflies)
		return;

	memset( &fly, 0, sizeof(fly) );

	if(Distance(cent->lerpOrigin, cg.refdef.vieworg) > 200){
		fly.customShader = cgs.media.bpoint;
		fly.radius = 1;
		fly.reType = RT_SPRITE;
	} else {
		fly.hModel = cgs.media.fly;
		fly.reType = RT_MODEL;
	}

	VectorCopy( cent->lerpOrigin, fly.origin );
	fly.origin[2] += 5;

	deltatime1 = 4000 + cent->currentState.clientNum*100;
	deltatime2 = 6000 + cent->currentState.clientNum*75;
	time1 = fabs((float)((cg.time - cent->currentState.clientNum*33)%deltatime1-(deltatime1/2))/(deltatime1/20))-5;
	time2 = fabs((float)((cg.time - cent->currentState.clientNum*55)%deltatime2-(deltatime2/2))/(deltatime2/20))-5;

	// let the fly swing a bit
	VectorCopy(cent->lerpAngles, angles);
	angles[ROLL] += fabs((float)((cg.time-cent->currentState.clientNum*75)%5000-2500)/83)-15;
	AngleVectors(angles, NULL, right, up);
	VectorMA(fly.origin, time1, right, fly.origin);
	VectorMA(fly.origin, time2, up, fly.origin);

	VectorCopy( fly.origin, fly.lightingOrigin );
	fly.shadowPlane = 0;
	fly.renderfx = 0;
	VectorCopy (fly.origin, fly.oldorigin);	// don't positionally lerp at all
	AnglesToAxis(angles, fly.axis);

	// play the sound
	if((cg_playownflysound.integer || cent->currentState.clientNum != cg.snap->ps.clientNum) &&
		Distance(cent->lerpOrigin, cg.refdef.vieworg) < 500){
		trap_S_AddLoopingSound(cent->currentState.clientNum, cent->lerpOrigin,
			vec3_origin, cgs.media.fly_sound);
	}

	if(cent->currentState.clientNum != cg.snap->ps.clientNum || cg.renderingThirdPerson)
		trap_R_AddRefEntityToScene( &fly );
}
#endif

/*
===============
CG_Player
===============
*/
void CG_Player( centity_t *cent ) {
	clientInfo_t	*ci;
	refEntity_t		legs;
	refEntity_t		torso;
	refEntity_t		head;
#ifdef SMOKINGUNS
	refEntity_t		money;
#endif
	int				clientNum;
	int				renderfx;
	qboolean		shadow;
	float			shadowPlane;
#ifndef SMOKINGUNS
	refEntity_t		skull;
	refEntity_t		powerup;
	int				t;
	float			c;
	float			angle;
	vec3_t			dir, angles;
#else
	int				h_pistol = -1; //biggest wp-num of holstered pistols
	qboolean		addguns = cg_addguns.integer;
#endif

	// the client number is stored in clientNum.  It can't be derived
	// from the entity number, because a single client may have
	// multiple corpses on the level using the same clientinfo
	clientNum = cent->currentState.clientNum;
	if ( clientNum < 0 || clientNum >= MAX_CLIENTS ) {
		CG_Error( "Bad clientNum on player entity");
	}
	ci = &cgs.clientinfo[ clientNum ];

	// it is possible to see corpses from disconnected players that may
	// not have valid clientinfo
	if ( !ci->infoValid ) {
		return;
	}

	// get the player model information
	renderfx = 0;
	if ( cent->currentState.number == cg.snap->ps.clientNum) {
#ifndef SMOKINGUNS
		if (!cg.renderingThirdPerson) {
#else
		if (!cg.renderingThirdPerson ||  cg.snap->ps.persistant[PERS_TEAM] >= TEAM_SPECTATOR ||
			(cg.snap->ps.stats[STAT_HEALTH] < 0 && !cg.renderingThirdPerson)) {
#endif
			renderfx = RF_THIRD_PERSON;			// only draw in mirrors
		} else {
			if (cg_cameraMode.integer) {
				return;
			}
		}
	}


	memset( &legs, 0, sizeof(legs) );
	memset( &torso, 0, sizeof(torso) );
	memset( &head, 0, sizeof(head) );

	// get the rotation information
	CG_PlayerAngles( cent, legs.axis, torso.axis, head.axis );

	// get the animation state (after rotation, to allow feet shuffle)
	CG_PlayerAnimation( cent, &legs.oldframe, &legs.frame, &legs.backlerp,
		 &torso.oldframe, &torso.frame, &torso.backlerp );


#if defined SMOKINGUNS && defined HIT_DATA
	if(hit_model.integer && cent->currentState.clientNum == cg.snap->ps.clientNum){
		vec3_t			legsAngles, torsoAngles;

		vectoangles(legs.axis[0], legsAngles);
		vectoangles(torso.axis[0], torsoAngles);

		CG_Printf("cg %f %f %f  %f %f %f\n", legsAngles[0],
			legsAngles[1], legsAngles[2],
			torsoAngles[0], torsoAngles[1],
			torsoAngles[2]);

		BuildHitModel(&hit_data, cent->lerpOrigin,
			legsAngles, torsoAngles, cent->lerpAngles,
			torso.frame, torso.oldframe, torso.backlerp,
			legs.frame, legs.oldframe, legs.backlerp);
	}
#endif

	// add the talk baloon or disconnect icon
	CG_PlayerSprites( cent );

#ifndef SMOKINGUNS
	// add the shadow
	shadow = CG_PlayerShadow( cent, &shadowPlane );
#endif

	// add a water splash if partially in and out of water
	CG_PlayerSplash( cent );

#ifndef SMOKINGUNS
	if ( cg_shadows.integer == 3 && shadow ) {
		renderfx |= RF_SHADOW_PLANE;
	}
	renderfx |= RF_LIGHTING_ORIGIN;			// use the same origin for all
	if( cgs.gametype == GT_HARVESTER ) {
		CG_PlayerTokens( cent, renderfx );
	}
#endif
	//
	// add the legs
	//
	legs.hModel = ci->legsModel;
#ifndef SMOKINGUNS
	legs.customSkin = ci->legsSkin;
#else
	// decide if damage skin is to be used
	if(cent->currentState.powerups & (1 << DM_LEGS_2))
		legs.customSkin = ci->legsSkin_damage_2;
	else if(cent->currentState.powerups & (1 << DM_LEGS_1))
		legs.customSkin = ci->legsSkin_damage_1;
	else
		legs.customSkin = ci->legsSkin;
#endif

	VectorCopy( cent->lerpOrigin, legs.origin );

#ifndef SMOKINGUNS
	VectorCopy( cent->lerpOrigin, legs.lightingOrigin );
#else
	//modify this origin if we're in a falloff-death-animation
	if((cent->currentState.legsAnim &~ ANIM_TOGGLEBIT) == BOTH_FALL ||
		(cent->currentState.legsAnim &~ ANIM_TOGGLEBIT) == BOTH_FALL_BACK ||
		(cent->currentState.legsAnim &~ ANIM_TOGGLEBIT) == BOTH_DEATH_LAND ||
		(cent->currentState.legsAnim &~ ANIM_TOGGLEBIT) == BOTH_DEAD_LAND) {
		// first accelerate into the direction
		int anim = (cent->currentState.legsAnim &~ ANIM_TOGGLEBIT);
		trajectory_t tr;
		trace_t trace;
		vec3_t pos, mins, maxs;
		//qboolean fall = qfalse;
		const float startheight = legs.origin[2];
		int type = anim == BOTH_FALL ? 1 : -1;

		VectorSet(mins, -20, -20, -24);
		VectorSet(maxs, 20, 20, 15);

		if(anim == BOTH_DEAD_LAND){
			if(cg.landTypes[cent->currentState.clientNum])
				type = 1;
			else
				type = -1;
		}

#define ACCEL_START		200.0
#define	ACCEL_LENGTH	1500.0
		// initialize falldown-calculation
		if(cent->fallTime == 0){

			if(anim == BOTH_DEAD_LAND){

				VectorCopy(cg.deathpos[cent->currentState.clientNum], cent->fallLastPos);

				// get fallTimes
				cent->fallTimeReal = cg.realFallTimes[cent->currentState.clientNum];
				cent->fallTime = cg.fallTimes[cent->currentState.clientNum];
				cent->landing = cg.landings[cent->currentState.clientNum];
				cent->landtime = cg.landTimes[cent->currentState.clientNum];

				//CG_Printf("2 %i %i\n", cent->fallTimeReal, cent->fallTime);

			} else {
				cent->fallTime = cg.time;
				cent->fallTimeReal = cg.time;
				VectorCopy(legs.origin, cent->fallLastPos);

				//CG_Printf("1 %i\n", cent->fallTimeReal);
			}
		}

		if(cent->landing){
			// play land animation
			int frame;
			float f_frame, factor;
			int deltaTime;
			animation_t *anim_d = &ci->animations[BOTH_DEATH_LAND];
			qboolean end = qfalse;

			deltaTime = cg.time - cent->landtime;
			factor = (float)deltaTime/( (float)anim_d->frameLerp*(float)(anim_d->numFrames-1) );
			frame = factor*(anim_d->numFrames-1);
			f_frame = factor*(anim_d->numFrames-1);

			if(frame < f_frame)
				frame++;
			if(frame > anim_d->numFrames-1){
				frame = anim_d->numFrames-1;
				end = qtrue;
				cent->fixed = qtrue;
			}
			frame += anim_d->firstFrame;

			legs.frame = frame;
			legs.oldframe = frame-1;
			legs.backlerp = ((float)legs.oldframe-(float)anim_d->firstFrame)*(float)anim_d->frameLerp;
			legs.backlerp = (float)deltaTime-legs.backlerp;
			legs.backlerp = 1-(float)legs.backlerp/(float)anim_d->frameLerp;

			if(end)
				legs.backlerp = 0;

			torso.oldframe = legs.oldframe;
			torso.frame = legs.frame;
			torso.backlerp = legs.backlerp;
		}

		// movement
		// only move if the player hasn't landed already
		if(!cent->fixed){
			float phase;
			float factor;

			int deltaTime = cg.time - cent->fallTimeReal - ACCEL_START;

			if(deltaTime > 0){
				phase = (float)deltaTime/ACCEL_LENGTH;
				if(phase < 1.0){
					phase *= M_PI/2;
					factor = cos(phase-M_PI)+1;
				} else {
					factor = phase;
				}

				// when are we allowed to play the land-anim?
				//if(deltaTime > ACCEL_LENGTH+500)
				//	fall = qtrue;

				factor *= type;

				VectorMA(legs.origin, factor*FALLOFF_RANGE, legs.axis[0], legs.origin);
				//CG_Printf("%f %i\n", factor, cent->fallTimeReal);
			}

			// gravity
			VectorCopy(legs.origin, tr.trBase);
			tr.trTime = cent->fallTime;
			tr.trType = TR_GRAVITY_LOW2;
			BG_EvaluateTrajectory(&tr, cg.time, pos);

			// check if we're in solid
			CG_Trace(&trace, cent->fallLastPos, mins, maxs, pos, 0, MASK_SOLID);
			// if true, don't fall this frame
			if(trace.fraction != 1.0 || trace.allsolid || trace.startsolid){
				float diff, deltaTime;

				VectorNormalize(trace.plane.normal);

				// but first check if we're flying towards a wall
				if(trace.plane.normal[2] < 0.75f &&
					(trace.plane.normal[0] > 0 || trace.plane.normal[1] > 0|| trace.plane.normal[2] > 0)){
					// if true accelerate a bit away from the wall
					float save = pos[2];

					VectorMA(pos, 20, trace.plane.normal, pos);
					pos[2] = save;
				} else {
					// calculate the last frametime and adjust the starttime
					if(trace.fraction != 1.0){
						cent->fallLastPos[2] = trace.endpos[2];
					}
					diff = cent->fallLastPos[2] - pos[2];

					deltaTime = (2*diff)/DEFAULT_GRAVITY;
					deltaTime *= 1000;

					cent->fallTime += (int)deltaTime;
					pos[2] = cent->fallLastPos[2];

					// don't move anymore
					if(cent->landing)
						cent->fixed = qtrue;
				}
			}
		} else {
			VectorCopy(cent->fallLastPos, pos);
		}

		VectorCopy(pos, legs.origin);
		VectorCopy(pos, cent->fallLastPos);

		// check the distance to the ground to play the land anim
		if(/*fall &&*/ !cent->landing && startheight - legs.origin[2] > 100){
			vec3_t end;

			VectorCopy(legs.origin, end);
			end[2] -= 1000;

			CG_Trace(&trace, legs.origin, mins, maxs, end, -1, MASK_SOLID);

			// ground is very far away
			if(trace.fraction == 1.0){
			} else {
				if(Distance(legs.origin, trace.endpos) < 130 &&
					trace.plane.normal[2] > 0.75f &&
					(trace.plane.normal[0] > 0 || trace.plane.normal[1] > 0|| trace.plane.normal[2] > 0)){
					cent->landing = qtrue;
					cent->landtime = cg.time;
					// play land sound
					trap_S_StartSound( trace.endpos, ENTITYNUM_WORLD, CHAN_AUTO,
						cgs.media.snd_death[10] );
				}
			}
		}

		// save information for the corpse-models
		if(anim != BOTH_DEAD_LAND){
			VectorCopy(legs.origin, cg.deathpos[cent->currentState.clientNum]);
			cg.fallTimes[cent->currentState.clientNum] = cent->fallTime;
			cg.realFallTimes[cent->currentState.clientNum] = cent->fallTimeReal;
			cg.landings[cent->currentState.clientNum] = cent->landing;
			cg.landTimes[cent->currentState.clientNum] = cent->landtime;
			cg.landTypes[cent->currentState.clientNum] = type+1;
		}

	} else {
		cent->landing = cent->fixed = qfalse;
		cent->fallTimeReal = cent->fallTime = cent->landtime = 0;
	}

	if(cent->currentState.clientNum == cg.snap->ps.clientNum && !cg.renderingThirdPerson &&
		!(cent->currentState.eFlags & EF_DEAD)){

		VectorCopy(cg.predictedPlayerState.origin, legs.origin);
		legs.origin[2] = cg.refdef.vieworg[2] - cg.predictedPlayerState.viewheight;
		VectorScale(legs.axis[0], 1.5, legs.axis[0]);
		VectorScale(legs.axis[1], 1.5, legs.axis[1]);
		VectorSubtract(legs.origin, cg.legOffset, legs.origin);
	}

	// A temporary solution to disable shadow projection bug
	// in the Quake 3 engine, as it is a form of cheating.
	if (cg_shadows.integer == 2 && !cg_cheats
	&& CG_Cvar_VariableInteger("r_stencilbits") >= 4) {
		CG_Printf("cg_shadows: 2 -> 1\n");
		cg_shadows.integer = 1;
		trap_Cvar_Set("cg_shadows", "1");
	}

	// add the shadow
	shadow = CG_PlayerShadow( cent, &shadowPlane, legs.origin );

	if ( cg_shadows.integer == 3 && shadow && cg_cheats) {
		renderfx |= RF_SHADOW_PLANE;
	}
	renderfx |= RF_LIGHTING_ORIGIN;

	VectorCopy (cent->oldorigin, legs.oldorigin);	// don't positionally lerp at all
	VectorCopy( legs.origin, legs.lightingOrigin );
	VectorCopy (legs.origin, cent->oldorigin);
#endif
	legs.shadowPlane = shadowPlane;
	legs.renderfx = renderfx;
#ifndef SMOKINGUNS
	VectorCopy (legs.origin, legs.oldorigin);	// don't positionally lerp at all
#else
	if(cg.snap->ps.persistant[PERS_TEAM] < TEAM_SPECTATOR)
		legs.renderfx &= ~RF_THIRD_PERSON;
#endif

	CG_AddRefEntityWithPowerups( &legs, &cent->currentState, ci->team );

	// if the model failed, allow the default nullmodel to be displayed
	if (!legs.hModel) {
		return;
	}

	// add the holsters
#ifdef SMOKINGUNS
	if(cgs.media.holster_l && addguns){
		refEntity_t		holster_l;

		memset( &holster_l, 0, sizeof(holster_l) );

		h_pistol = CG_AddHolster( &holster_l, &legs, cent, ci, h_pistol, renderfx, shadowPlane, qtrue);
	}
	if(cgs.media.holster_r && addguns){
		refEntity_t		holster_r;

		memset( &holster_r, 0, sizeof(holster_r) );

		CG_AddHolster( &holster_r, &legs, cent, ci, h_pistol, renderfx, shadowPlane, qfalse);
	}
#endif

	//
	// add the torso
	//
	torso.hModel = ci->torsoModel;
	if (!torso.hModel) {
		return;
	}

#ifndef SMOKINGUNS
	torso.customSkin = ci->torsoSkin;
#else
	// decide if damage skin is to be used
	if(cent->currentState.powerups & (1 << DM_TORSO_2))
		torso.customSkin = ci->torsoSkin_damage_2;
	else if(cent->currentState.powerups & (1 << DM_TORSO_1))
		torso.customSkin = ci->torsoSkin_damage_1;
	else
		torso.customSkin = ci->torsoSkin;
#endif

	VectorCopy( cent->lerpOrigin, torso.lightingOrigin );

	CG_PositionRotatedEntityOnTag( &torso, &legs, ci->legsModel, "tag_torso");

	torso.shadowPlane = shadowPlane;
	torso.renderfx = renderfx;

	CG_AddRefEntityWithPowerups( &torso, &cent->currentState, ci->team );

#ifndef SMOKINGUNS
	if ( cent->currentState.eFlags & EF_KAMIKAZE ) {

		memset( &skull, 0, sizeof(skull) );

		VectorCopy( cent->lerpOrigin, skull.lightingOrigin );
		skull.shadowPlane = shadowPlane;
		skull.renderfx = renderfx;

		if ( cent->currentState.eFlags & EF_DEAD ) {
			// one skull bobbing above the dead body
			angle = ((cg.time / 7) & 255) * (M_PI * 2) / 255;
			if (angle > M_PI * 2)
				angle -= (float)M_PI * 2;
			dir[0] = sin(angle) * 20;
			dir[1] = cos(angle) * 20;
			angle = ((cg.time / 4) & 255) * (M_PI * 2) / 255;
			dir[2] = 15 + sin(angle) * 8;
			VectorAdd(torso.origin, dir, skull.origin);

			dir[2] = 0;
			VectorCopy(dir, skull.axis[1]);
			VectorNormalize(skull.axis[1]);
			VectorSet(skull.axis[2], 0, 0, 1);
			CrossProduct(skull.axis[1], skull.axis[2], skull.axis[0]);

			skull.hModel = cgs.media.kamikazeHeadModel;
			trap_R_AddRefEntityToScene( &skull );
			skull.hModel = cgs.media.kamikazeHeadTrail;
			trap_R_AddRefEntityToScene( &skull );
		}
		else {
			// three skulls spinning around the player
			angle = ((cg.time / 4) & 255) * (M_PI * 2) / 255;
			dir[0] = cos(angle) * 20;
			dir[1] = sin(angle) * 20;
			dir[2] = cos(angle) * 20;
			VectorAdd(torso.origin, dir, skull.origin);

			angles[0] = sin(angle) * 30;
			angles[1] = (angle * 180 / M_PI) + 90;
			if (angles[1] > 360)
				angles[1] -= 360;
			angles[2] = 0;
			AnglesToAxis( angles, skull.axis );

			/*
			dir[2] = 0;
			VectorInverse(dir);
			VectorCopy(dir, skull.axis[1]);
			VectorNormalize(skull.axis[1]);
			VectorSet(skull.axis[2], 0, 0, 1);
			CrossProduct(skull.axis[1], skull.axis[2], skull.axis[0]);
			*/

			skull.hModel = cgs.media.kamikazeHeadModel;
			trap_R_AddRefEntityToScene( &skull );
			// flip the trail because this skull is spinning in the other direction
			VectorInverse(skull.axis[1]);
			skull.hModel = cgs.media.kamikazeHeadTrail;
			trap_R_AddRefEntityToScene( &skull );

			angle = ((cg.time / 4) & 255) * (M_PI * 2) / 255 + M_PI;
			if (angle > M_PI * 2)
				angle -= (float)M_PI * 2;
			dir[0] = sin(angle) * 20;
			dir[1] = cos(angle) * 20;
			dir[2] = cos(angle) * 20;
			VectorAdd(torso.origin, dir, skull.origin);

			angles[0] = cos(angle - 0.5 * M_PI) * 30;
			angles[1] = 360 - (angle * 180 / M_PI);
			if (angles[1] > 360)
				angles[1] -= 360;
			angles[2] = 0;
			AnglesToAxis( angles, skull.axis );

			/*
			dir[2] = 0;
			VectorCopy(dir, skull.axis[1]);
			VectorNormalize(skull.axis[1]);
			VectorSet(skull.axis[2], 0, 0, 1);
			CrossProduct(skull.axis[1], skull.axis[2], skull.axis[0]);
			*/

			skull.hModel = cgs.media.kamikazeHeadModel;
			trap_R_AddRefEntityToScene( &skull );
			skull.hModel = cgs.media.kamikazeHeadTrail;
			trap_R_AddRefEntityToScene( &skull );

			angle = ((cg.time / 3) & 255) * (M_PI * 2) / 255 + 0.5 * M_PI;
			if (angle > M_PI * 2)
				angle -= (float)M_PI * 2;
			dir[0] = sin(angle) * 20;
			dir[1] = cos(angle) * 20;
			dir[2] = 0;
			VectorAdd(torso.origin, dir, skull.origin);

			VectorCopy(dir, skull.axis[1]);
			VectorNormalize(skull.axis[1]);
			VectorSet(skull.axis[2], 0, 0, 1);
			CrossProduct(skull.axis[1], skull.axis[2], skull.axis[0]);

			skull.hModel = cgs.media.kamikazeHeadModel;
			trap_R_AddRefEntityToScene( &skull );
			skull.hModel = cgs.media.kamikazeHeadTrail;
			trap_R_AddRefEntityToScene( &skull );
		}
	}

	if ( cent->currentState.powerups & ( 1 << PW_GUARD ) ) {
		memcpy(&powerup, &torso, sizeof(torso));
		powerup.hModel = cgs.media.guardPowerupModel;
		powerup.frame = 0;
		powerup.oldframe = 0;
		powerup.customSkin = 0;
		trap_R_AddRefEntityToScene( &powerup );
	}
	if ( cent->currentState.powerups & ( 1 << PW_SCOUT ) ) {
		memcpy(&powerup, &torso, sizeof(torso));
		powerup.hModel = cgs.media.scoutPowerupModel;
		powerup.frame = 0;
		powerup.oldframe = 0;
		powerup.customSkin = 0;
		trap_R_AddRefEntityToScene( &powerup );
	}
	if ( cent->currentState.powerups & ( 1 << PW_DOUBLER ) ) {
		memcpy(&powerup, &torso, sizeof(torso));
		powerup.hModel = cgs.media.doublerPowerupModel;
		powerup.frame = 0;
		powerup.oldframe = 0;
		powerup.customSkin = 0;
		trap_R_AddRefEntityToScene( &powerup );
	}
	if ( cent->currentState.powerups & ( 1 << PW_AMMOREGEN ) ) {
		memcpy(&powerup, &torso, sizeof(torso));
		powerup.hModel = cgs.media.ammoRegenPowerupModel;
		powerup.frame = 0;
		powerup.oldframe = 0;
		powerup.customSkin = 0;
		trap_R_AddRefEntityToScene( &powerup );
	}
	if ( cent->currentState.powerups & ( 1 << PW_INVULNERABILITY ) ) {
		if ( !ci->invulnerabilityStartTime ) {
			ci->invulnerabilityStartTime = cg.time;
		}
		ci->invulnerabilityStopTime = cg.time;
	}
	else {
		ci->invulnerabilityStartTime = 0;
	}
	if ( (cent->currentState.powerups & ( 1 << PW_INVULNERABILITY ) ) ||
		cg.time - ci->invulnerabilityStopTime < 250 ) {

		memcpy(&powerup, &torso, sizeof(torso));
		powerup.hModel = cgs.media.invulnerabilityPowerupModel;
		powerup.customSkin = 0;
		// always draw
		powerup.renderfx &= ~RF_THIRD_PERSON;
		VectorCopy(cent->lerpOrigin, powerup.origin);

		if ( cg.time - ci->invulnerabilityStartTime < 250 ) {
			c = (float) (cg.time - ci->invulnerabilityStartTime) / 250;
		}
		else if (cg.time - ci->invulnerabilityStopTime < 250 ) {
			c = (float) (250 - (cg.time - ci->invulnerabilityStopTime)) / 250;
		}
		else {
			c = 1;
		}
		VectorSet( powerup.axis[0], c, 0, 0 );
		VectorSet( powerup.axis[1], 0, c, 0 );
		VectorSet( powerup.axis[2], 0, 0, c );
		trap_R_AddRefEntityToScene( &powerup );
	}

	t = cg.time - ci->medkitUsageTime;
	if ( ci->medkitUsageTime && t < 500 ) {
		memcpy(&powerup, &torso, sizeof(torso));
		powerup.hModel = cgs.media.medkitUsageModel;
		powerup.customSkin = 0;
		// always draw
		powerup.renderfx &= ~RF_THIRD_PERSON;
		VectorClear(angles);
		AnglesToAxis(angles, powerup.axis);
		VectorCopy(cent->lerpOrigin, powerup.origin);
		powerup.origin[2] += -24 + (float) t * 80 / 500;
		if ( t > 400 ) {
			c = (float) (t - 1000) * 0xff / 100;
			powerup.shaderRGBA[0] = 0xff - c;
			powerup.shaderRGBA[1] = 0xff - c;
			powerup.shaderRGBA[2] = 0xff - c;
			powerup.shaderRGBA[3] = 0xff - c;
		}
		else {
			powerup.shaderRGBA[0] = 0xff;
			powerup.shaderRGBA[1] = 0xff;
			powerup.shaderRGBA[2] = 0xff;
			powerup.shaderRGBA[3] = 0xff;
		}
		trap_R_AddRefEntityToScene( &powerup );
	}
#else
	//add weapon on back if nessecary
	if(addguns)
		AddBackWeapon(&torso, cent, ci, renderfx, shadowPlane);

#define FIRE_DURATION 1000
#define	FIRE_SHRINK	  200
	// add fire if nessecary
	if(cent->currentState.powerups & (1 << PW_BURNBIT) &&
		cent->currentState.time + FIRE_DURATION > cg.time){
		refEntity_t		fire;
		float factor = (float)(cent->currentState.time+FIRE_DURATION-cg.time);
		factor /= FIRE_SHRINK;

		if(cg.time < cent->currentState.time + FIRE_DURATION - FIRE_SHRINK)
			factor = 1.0f;

		memset( &fire, 0, sizeof( fire ) );
		fire.reType = RT_SPRITE;
		fire.customShader = cgs.media.wqfx_fire;
		fire.radius = 8*factor;
		fire.renderfx = renderfx;
		fire.shaderRGBA[3] = factor * 255;

		// on the legs...
		AnglesToAxis( vec3_origin, fire.axis );
		CG_PositionRotatedEntityOnTag( &fire, &legs, legs.hModel, "tag_holsterr");
		trap_R_AddRefEntityToScene( &fire );

		// on the legs 2
		fire.radius = 10*factor;
		CG_PositionRotatedEntityOnTag( &fire, &legs, legs.hModel, "tag_holsterl");
		fire.origin[2] -= 15;
		trap_R_AddRefEntityToScene( &fire );

		// on the torso
		fire.radius = 6*factor;
		CG_PositionRotatedEntityOnTag( &fire, &torso, torso.hModel, "tag_back");
		trap_R_AddRefEntityToScene( &fire );
	}
#endif

	//
	// add the head
	//
	head.hModel = ci->headModel;
	if (!head.hModel) {
		return;
	}

#ifndef SMOKINGUNS
	head.customSkin = ci->headSkin;
#else
	// decide if damage skin is to be used
	if(cent->currentState.powerups & (1 << DM_HEAD_2))
		head.customSkin = ci->headSkin_damage_2;
	else if(cent->currentState.powerups & (1 << DM_HEAD_1))
		head.customSkin = ci->headSkin_damage_1;
	else
		head.customSkin = ci->headSkin;
#endif

	VectorCopy( cent->lerpOrigin, head.lightingOrigin );

	CG_PositionRotatedEntityOnTag( &head, &torso, ci->torsoModel, "tag_head");

	//head angles always are viewangles!!, when swimming changed by Spoon
#ifdef SMOKINGUNS
	if(!(cent->currentState.eFlags & EF_DEAD) &&
		(cent->currentState.legsAnim & ~ANIM_TOGGLEBIT)==LEGS_SWIM){
		AnglesToAxis(cent->lerpAngles, head.axis);
	}
#endif

	head.shadowPlane = shadowPlane;
	head.renderfx = renderfx;

	CG_AddRefEntityWithPowerups( &head, &cent->currentState, ci->team );

#ifdef SMOKINGUNS
	if(!(cent->currentState.eFlags & EF_DEAD) &&
		cg.snap->ps.clientNum == cent->currentState.number)
		VectorCopy(cg.refdef.vieworg, cg.anim_vieworigin);

	// copy information of head-pos-status to the (optional) view-vectors
	if(cg.snap->ps.clientNum == cent->currentState.number){
		vec3_t	view_angles;
		trace_t	trace;
		vec3_t	mins, maxs;

		VectorSet(mins, -24, -24, -24);
		VectorSet(maxs, 24, 24, 24);

		//origin checks
		CG_Trace(  &trace, head.origin,
			mins, maxs, cg.anim_vieworigin, -1, MASK_SOLID);

		if(trace.fraction == 1.0){
			VectorCopy(head.origin, cg.anim_vieworigin);
			//VectorCopy(cg.refdef.vieworg, cg.anim_vieworigin);
			//head.origin[2] = cg.anim_vieworigin[2];
		} else {
			vec3_t origin;

			VectorCopy(cg.anim_vieworigin, origin);
			origin[2] = head.origin[2];

			CG_Trace ( &trace, origin,
				mins, maxs, cg.anim_vieworigin, -1, MASK_SOLID);

			if(trace.fraction == 1.0){
				VectorCopy(origin, cg.anim_vieworigin);
			}
		}
		//CG_Printf("z: %f %f %f\n", head.origin[2], legs.origin[2], cent->lerpOrigin[2]);

		//angles
		vectoangles(head.axis[0], view_angles);
		VectorCopy(view_angles, cg.anim_viewangles);

		//CG_Printf("%f, %f, %f\n", cg.anim_vieworigin[0], cg.anim_vieworigin[1], cg.anim_vieworigin[2]);
	}

	//add the moneybag
	if(cent->currentState.powerups & (1 << PW_GOLD)){
		vec3_t angles;

		VectorClear(angles);

		memset( &money, 0, sizeof(money) );

		money.hModel = cgs.media.e_moneybag;

		if (!money.hModel) {
			return;
		}

		VectorCopy( cent->lerpOrigin, money.lightingOrigin );
		AnglesToAxis(angles, money.axis );

		VectorScale(money.axis[0], 0.75f, money.axis[0]);
		VectorScale(money.axis[1], 0.75f, money.axis[1]);
		VectorScale(money.axis[2], 0.75f, money.axis[2]);

		CG_PositionRotatedEntityOnTag( &money, &torso, ci->torsoModel, "tag_back");

		money.shadowPlane = shadowPlane;
		money.renderfx = renderfx;

		CG_AddRefEntityWithPowerups( &money, &cent->currentState, ci->team );
	}

	// if gatling is being reloaded add the magazine
	if(cent->currentState.weapon == WP_GATLING &&
		(cent->currentState.torsoAnim & ~ANIM_TOGGLEBIT) == TORSO_GATLING_RELOAD){
		int i;

		for(i = 0; i < MAX_GENTITIES; i++){
			centity_t *t = &cg_entities[i];

			if(t->currentState.eType == ET_TURRET &&
				t->currentState.eventParm == cent->currentState.clientNum){
				refEntity_t	mag;

				memset(&mag, 0, sizeof(mag));

				mag.hModel = cgs.media.g_mag;

				if(!mag.hModel)
					return;

				VectorCopy( cent->lerpOrigin, mag.lightingOrigin );
				mag.renderfx = renderfx;
				mag.shadowPlane = shadowPlane;
				mag.reType = RT_MODEL;

				AnglesToAxis( vec3_origin, mag.axis );

				CG_PositionRotatedEntityOnTag( &mag, &torso, torso.hModel, "tag_weapon2");
				VectorMA(mag.origin, -3, mag.axis[0], mag.origin);
				VectorMA(mag.origin, -3, mag.axis[1], mag.origin);
				VectorMA(mag.origin, -5, mag.axis[2], mag.origin);
				VectorCopy (mag.origin, mag.oldorigin);
				CG_AddRefEntityWithPowerups( &mag, &cent->currentState, ci->team);
				break;
			}
		}
	}

	CG_BreathPuffs(cent, &head);

	CG_DustTrail(cent);
#endif

	//
	// add the gun / barrel / flash
	//
	CG_AddPlayerWeapon( &torso, NULL, cent, ci->team );

#ifndef SMOKINGUNS
	// add powerups floating behind the player
	CG_PlayerPowerups( cent, &torso );
#else
//unlagged - client options
	// add the bounding box (if cg_drawBBox is 1)
	CG_AddBoundingBox( cent );
//unlagged - client options
#endif
}


//=====================================================================

/*
===============
CG_ResetPlayerEntity

A player just came into view or teleported, so reset all animation info
===============
*/
void CG_ResetPlayerEntity( centity_t *cent ) {
	cent->errorTime = -99999;		// guarantee no error decay added
	cent->extrapolated = qfalse;

	CG_ClearLerpFrame( &cgs.clientinfo[ cent->currentState.clientNum ], &cent->pe.legs, cent->currentState.legsAnim );
	CG_ClearLerpFrame( &cgs.clientinfo[ cent->currentState.clientNum ], &cent->pe.torso, cent->currentState.torsoAnim );

#ifdef SMOKINGUNS
	if(cg.snap->ps.clientNum == cent->currentState.clientNum){
		CG_ClearLerpFrame( &cgs.clientinfo[ cent->currentState.clientNum ], &cent->pe.weapon, cent->pe.weapon.weaponAnim);
		CG_ClearLerpFrame( &cgs.clientinfo[ cent->currentState.clientNum ], &cent->pe.weapon2, cent->pe.weapon2.weaponAnim);
	}
#endif

	BG_EvaluateTrajectory( &cent->currentState.pos, cg.time, cent->lerpOrigin );
	BG_EvaluateTrajectory( &cent->currentState.apos, cg.time, cent->lerpAngles );

	VectorCopy( cent->lerpOrigin, cent->rawOrigin );
	VectorCopy( cent->lerpAngles, cent->rawAngles );

	memset( &cent->pe.legs, 0, sizeof( cent->pe.legs ) );
	cent->pe.legs.yawAngle = cent->rawAngles[YAW];
	cent->pe.legs.yawing = qfalse;
	cent->pe.legs.pitchAngle = 0;
	cent->pe.legs.pitching = qfalse;

	memset( &cent->pe.torso, 0, sizeof( cent->pe.torso ) );
	cent->pe.torso.yawAngle = cent->rawAngles[YAW];
	cent->pe.torso.yawing = qfalse;
	cent->pe.torso.pitchAngle = cent->rawAngles[PITCH];
	cent->pe.torso.pitching = qfalse;

	if ( cg_debugPosition.integer ) {
		CG_Printf("%i ResetPlayerEntity yaw=%f\n", cent->currentState.number, cent->pe.torso.yawAngle );
	}

#ifdef SMOKINGUNS
	if(cent->currentState.clientNum == cg.snap->ps.clientNum){
		cg.markedweapon = 0;
	}
#endif
}

