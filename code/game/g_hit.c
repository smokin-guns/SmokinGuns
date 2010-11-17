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
//////////////////////////////////////////////
//
// g_hit.c -- hit-detection-code
// by Spoon

#include "g_local.h"
#include "../qcommon/q_shared.h"

/*
=================
new HITBOX
=================
*/

#define VectorShortCopy(a, b) ((b)[0]=(float)(a)[0],(b)[1]=(float)(a)[1],(b)[2]=(float)(a)[2])
#define VectorShortAdd(a,b,c) ((c)[0]=(float)((a)[0]+(b)[0]),(c)[1]=(float)((a)[1]+(b)[1]),(c)[2]=(float)((a)[2]+(b)[2]))
#define	MD3_SCALE		(1.0/64)

static void RotateDirection(vec3_t dir, vec3_t angles, vec3_t result){
	vec3_t dir_angles;

	vectoangles(dir, dir_angles);
	VectorAdd(dir_angles, angles, dir_angles);

	AngleVectors(dir_angles, result, NULL, NULL);
	VectorNormalize(result);
}

/*
================
SetupLines
================
*/
static void SetupLines(vec3_t offset_origin, vec3_t offset_angles,
					   vec3_t start, vec3_t end,
					   hit_part_t *mesh, lerpFrame_t *lerp,
					   vec3_t trace_dir, vec3_t cyl_dir,
					   vec3_t trace_start, vec3_t cyl_start,
					   vec3_t edge1, vec3_t edge2){
	int frame, oldframe;
	vec3_t diff;

	if(lerp==NULL || lerp->frame < HIT_DEATHANIM_OFFSET ||
		lerp->oldFrame < HIT_DEATHANIM_OFFSET) {
		frame = 0;
		oldframe = 0;
	} else {
		frame = lerp->frame-HIT_DEATHANIM_OFFSET;
		oldframe = lerp->oldFrame-HIT_DEATHANIM_OFFSET;
	}

	// SHOOT_LINE
	VectorSubtract(end, start, trace_dir);
	VectorNormalize(trace_dir);
	// pos
	VectorCopy(start, trace_start);

	// CYLINDER LINE
	VectorShortCopy(mesh->pos[frame].normal, cyl_dir);
	VectorNormalize(cyl_dir);
	// pos
	VectorShortCopy(mesh->pos[frame].origin, cyl_start);
	VectorScale(cyl_start, MD3_SCALE, cyl_start);
	if(lerp){
		vec3_t oldpos;

		// lerping the normal
		VectorShortCopy(mesh->pos[oldframe].normal, oldpos);
		VectorNormalize(oldpos);
		VectorSubtract(oldpos, cyl_dir, diff);
		VectorMA(cyl_dir, lerp->backlerp, diff, cyl_dir);

		// lerping the origin
		VectorShortCopy(mesh->pos[oldframe].origin, oldpos);
		VectorScale(oldpos, MD3_SCALE, oldpos);
		VectorSubtract(oldpos, cyl_start, diff);
		VectorMA(cyl_start, lerp->backlerp, diff, cyl_start);
	}

	// in the assign the coordinates to the world coordinates
	// direction
	RotateDirection(cyl_dir, offset_angles, cyl_dir);

	// origin
	RotateVectorAroundVector(vec3_origin, offset_angles, cyl_start);
	VectorAdd(cyl_start, offset_origin, cyl_start);

	// edges
	VectorShortCopy(mesh->header.dir1, edge1);
	VectorShortCopy(mesh->header.dir2, edge2);

	{
		vec3_t oldnormal;
		vec3_t angles;
		vec3_t edge_angles, norm_angles;

		VectorShortCopy(mesh->pos[0].normal, oldnormal);
		vectoangles(oldnormal, norm_angles);

		// edge1
		vectoangles(edge1, edge_angles);
		VectorSubtract(norm_angles, edge_angles, angles);
		AnglesNormalize180(angles);
		RotateDirection(cyl_dir, angles, edge1);
		VectorNormalize(edge1);

		// edge2
		vectoangles(edge2, edge_angles);
		VectorSubtract(norm_angles, edge_angles, angles);
		AnglesNormalize180(angles);
		RotateDirection(cyl_dir, angles, edge2);
		VectorNormalize(edge2);
	}
}

/*
===================
LineHitsPlane
Calculates intersection point of a line and a plane
===================
*/
#define	SMALL	0.001
static qboolean LineHitsPlane(vec3_t plane_org, vec3_t plane_normal,
						  vec3_t line_org, vec3_t line_dir,
						  vec3_t result){
	float	s;
	vec3_t	w;
	float	dot = DotProduct(plane_normal, line_dir);

	// line is parallel, no intersection
	if(dot < SMALL && dot > -SMALL){
		return qfalse;
	}

	VectorSubtract(line_org, plane_org, w);

	// normal * (w + s*u) has to be 0 , if point's on the plane
	s = -DotProduct(plane_normal, w)/DotProduct(plane_normal, line_dir);

	VectorMA(line_org, s, line_dir, result);
	return qtrue;
}

/*
===================
SmallestDistance
Calculates smallest distance between a point and a line
===================
*/
static void SmallestDistance(vec3_t trace_dir, vec3_t cyl_dir, vec3_t trace_start, vec3_t cyl_start,
							 vec3_t trace_p, vec3_t cyl_p){
	float s, t;

	float pu, sqrt_u, qu;
	float uv;
	float pv, sqrt_v, qv;

	float c1, c2;

	float factor;

	// the two lines are: trace: x=trace_start + t*trace_dir;  cyl: x=cyl_start + s*cyl_dir

	pu = DotProduct(trace_start, trace_dir);
	qu = DotProduct(cyl_start, trace_dir);
	sqrt_u = DotProduct(trace_dir, trace_dir);

	uv = DotProduct(trace_dir, cyl_dir);

	pv = DotProduct(trace_start, cyl_dir);
	qv = DotProduct(cyl_start, cyl_dir);
	sqrt_v = DotProduct(cyl_dir, cyl_dir);

	c1 = pu - qu;
	c2 = pv - qv;

	// get the factor
	factor = sqrt_u/uv;
	s = -(factor*c2-c1)/(uv-factor*sqrt_v);
	factor = uv/sqrt_v;
	t = (factor*c2-c1)/(sqrt_u-factor*uv);

	// now calculate the points
	VectorMA(trace_start, t, trace_dir, trace_p);
	VectorMA(cyl_start, s, cyl_dir, cyl_p);
}

/*
======================
CheckIntersection
Checks if cylinder was hit
======================
*/
static qboolean CheckIntersection(vec3_t offset_origin, vec3_t offset_angles, hit_part_t *mesh,
				  lerpFrame_t *lerp, vec3_t start, vec3_t end){
	vec3_t trace_dir, cyl_dir;
	vec3_t trace_start, cyl_start, cyl_end;
	vec3_t trace_p, cyl_p;

	vec3_t diff, p;
	vec3_t edge1, edge2;

	float	a1 = mesh->header.a1*MD3_SCALE, a2 = mesh->header.a2*MD3_SCALE;

	float	dist1, dist2;
	float	dist2_check;

	vec3_t midpoint; // the midpoint of the cylinder
	float length = mesh->header.length*MD3_SCALE;

	// find the directions of the lines
	SetupLines(offset_origin, offset_angles, start, end, mesh,
		lerp, trace_dir, cyl_dir, trace_start, cyl_start, edge1, edge2 );

	VectorMA(cyl_start, length, cyl_dir, cyl_end);

	// find the point on the trace-line with the smallest distance from the cylinder
	SmallestDistance(trace_dir, cyl_dir, trace_start, cyl_start, trace_p, cyl_p);

	// point projected in the middle of the cylinder
	length *= 0.5f;
	VectorSubtract(trace_p, cyl_p, diff);
	VectorMA(cyl_start, length, cyl_dir, midpoint);
	VectorAdd(midpoint, diff, p);

	// check if the point is out of the cylinder
	if(Distance(p, trace_p) > length){
		// check if caps were hit
		if (LineHitsPlane(cyl_start, cyl_dir, trace_start, trace_dir, trace_p)){
			VectorCopy(cyl_start, cyl_p);
		} else if (LineHitsPlane(cyl_end, cyl_dir, trace_start, trace_dir, trace_p)){
			VectorCopy(cyl_end, cyl_p);
		} else
			return qfalse;
	}

	// distance(p, g) = |(p-q)*n| ; q is on g
	dist1 = fabs(DotProduct(edge2, trace_p) - DotProduct(edge2, cyl_p));
	dist2 = fabs(DotProduct(edge1, trace_p) - DotProduct(edge1, cyl_p));

	if(dist1 > a1 || dist2 > a2)
		return qfalse;

	// check the maximum distance
	dist2_check = sqrt( (1- dist1*dist1/a1*a1)) * a2;

	if(dist2 > dist2_check)
		return qfalse;

	return qtrue;
}

/*
========================
TagOffset
//calculates position and angles of the tag
========================
*/
static void TagOffset(vec3_t offset_origin, vec3_t offset_angles,
			   hit_tag_t *ptag, lerpFrame_t *lerp){
	vec3_t	tag;
	vec3_t	angles;
	int		i;
	int frame, oldFrame;

	if(lerp->frame < HIT_DEATHANIM_OFFSET ||
		lerp->oldFrame < HIT_DEATHANIM_OFFSET){
		frame = 0;
		oldFrame = 0;
	} else {

		frame = lerp->frame - HIT_DEATHANIM_OFFSET;
		oldFrame = lerp->oldFrame - HIT_DEATHANIM_OFFSET;
	}

	//calculate pos & angles of
	for(i=0;i<3;i++){
		tag[i] = ptag[oldFrame].origin[i] +
			(ptag[frame].origin[i]-ptag[oldFrame].origin[i])*(1-lerp->backlerp);

		angles[i] = ptag[oldFrame].angles[i] +
			(ptag[frame].angles[i]-ptag[oldFrame].angles[i])*(1-lerp->backlerp);
	}

	RotateVectorAroundVector(vec3_origin, offset_angles, tag);
	VectorAdd(tag, offset_origin, offset_origin);

	VectorAdd(angles, offset_angles, offset_angles);
}

/*
===============
G_BuildHitModel
date: 19.11.2001
updated: 17.8.2002
by: Spoon
manages the hit-detection
===============
*/
static qboolean G_BuildHitModel(hit_data_t *data, vec3_t origin,
				   vec3_t angles, vec3_t torso_angles, vec3_t head_angles,
				   lerpFrame_t *torso, lerpFrame_t *legs,
				   vec3_t start, vec3_t end, int mesh){

	vec3_t	offset_origin, offset_angles;

	VectorCopy(origin, offset_origin);
	VectorCopy(angles, offset_angles);

	//legs
	if(hit_info[mesh].hit_part == PART_LOWER){
		if(CheckIntersection(offset_origin, offset_angles, &data->meshes[mesh],
			legs, start, end))
			return qtrue;
		return qfalse;
	}

	//calculate pos of tag_torso
	TagOffset(offset_origin, offset_angles, data->tag_torso, legs);
	VectorAdd(offset_angles, torso_angles, offset_angles);

	if(hit_info[mesh].hit_part == PART_UPPER){
		if(CheckIntersection(offset_origin, offset_angles, &data->meshes[mesh],
			torso, start, end))
			return qtrue;
		return qfalse;
	}

	//calculate pos of tag_head
	TagOffset(offset_origin, offset_angles, data->tag_head, torso);

	//head
	VectorCopy(head_angles, offset_angles);
	if(hit_info[mesh].hit_part == PART_HEAD){
		if( CheckIntersection(offset_origin, offset_angles, &data->meshes[mesh],
			NULL, start, end))
			return qtrue;
	}

	return qfalse;
}

/*
==========================
GetVectorOfPart
==========================
*/
static void GetVectorOfPart(hit_part_t *mesh, vec3_t vec, lerpFrame_t *lerp,
							vec3_t start, vec3_t end, vec3_t offset_origin, vec3_t offset_angles){
	int frame, oldframe;
	vec3_t diff;
	vec3_t trace_p, vec_p;
	vec3_t vec_dir, trace_dir;
	float length = mesh->header.length*MD3_SCALE;
	vec3_t midpoint;

	if(lerp==NULL || lerp->frame < HIT_DEATHANIM_OFFSET ||
		lerp->oldFrame < HIT_DEATHANIM_OFFSET) {
		frame = 0;
		oldframe = 0;
	} else {
		frame = lerp->frame-HIT_DEATHANIM_OFFSET;
		oldframe = lerp->oldFrame-HIT_DEATHANIM_OFFSET;
	}

	VectorSubtract(end, start, trace_dir);
	VectorNormalize(trace_dir);

	VectorShortCopy(mesh->pos[frame].normal, vec_dir);
	VectorNormalize(vec_dir);
	// pos
	VectorShortCopy(mesh->pos[frame].origin, vec);
	VectorScale(vec, MD3_SCALE, vec);
	if(lerp){
		vec3_t oldpos;

		// lerping the normal
		VectorShortCopy(mesh->pos[oldframe].normal, oldpos);
		VectorNormalize(oldpos);
		VectorSubtract(oldpos, vec_dir, diff);
		VectorMA(vec_dir, lerp->backlerp, diff, vec_dir);

		// lerping the origin
		VectorShortCopy(mesh->pos[oldframe].origin, oldpos);
		VectorScale(oldpos, MD3_SCALE, oldpos);
		VectorSubtract(oldpos, vec, diff);
		VectorMA(vec, lerp->backlerp, diff, vec);
	}

	// in the assign the coordinates to the world coordinates
	// direction
	RotateDirection(vec_dir, offset_angles, vec_dir);

	// origin
	RotateVectorAroundVector(vec3_origin, offset_angles, vec);
	VectorAdd(vec, offset_origin, vec);

	SmallestDistance(trace_dir, vec_dir, start, vec, trace_p, vec_p);

	length *= 0.5f;
	VectorMA(vec, length, vec_dir, midpoint);

	// check if the point is out of the cylinder
	if(Distance(midpoint, vec_p) > length){
		length *= 2;

		if(Distance(midpoint, vec_p) > Distance(midpoint, vec))
			VectorMA(vec, length, vec_dir, vec);

		return;
	}
	VectorCopy(vec_p, vec);
}

/*
=====================
G_NearestPart
=====================
*/
static int G_NearestPart(hit_data_t *data, vec3_t origin,
				   vec3_t angles, vec3_t torso_angles, vec3_t head_angles,
				   lerpFrame_t *torso, lerpFrame_t *legs,
				   vec3_t start, vec3_t end, float *mindist) {
	int i;
	float bestdist = 999;
	int		bestpart = 0;
	vec3_t offset_origin, offset_angles;
	vec3_t vector;
	float dist;

	for(i=0; i<NUM_HIT_LOCATIONS; i++){

		VectorCopy(origin, offset_origin);
		VectorCopy(angles, offset_angles);

		//legs
		if(hit_info[i].hit_part == PART_LOWER){
			GetVectorOfPart(&data->meshes[i], vector, legs, start, end, offset_origin,
				offset_angles);

			dist = Distance(vector, end);

			if( dist < bestdist && dist > *mindist){
				bestdist = dist;
				bestpart = i;
			}
			continue;
		}

		//calculate pos of tag_torso
		TagOffset(offset_origin, offset_angles, data->tag_torso, legs);
		VectorAdd(offset_angles, torso_angles, offset_angles);

		if(hit_info[i].hit_part == PART_UPPER){
			GetVectorOfPart(&data->meshes[i], vector, torso, start, end, offset_origin,
				offset_angles);

			dist = Distance(vector, end);

			if( dist < bestdist && dist > *mindist){
				bestdist = dist;
				bestpart = i;
			}
			continue;
		}

		//calculate pos of tag_head
		TagOffset(offset_origin, offset_angles, data->tag_head, torso);

		//head
		VectorCopy(head_angles, offset_angles);
		if(hit_info[i].hit_part == PART_HEAD){
			GetVectorOfPart(&data->meshes[i], vector, NULL, start, end, offset_origin,
				offset_angles);

			dist = Distance(vector, end);

			if( dist < bestdist && dist > *mindist){
				bestdist = dist;
				bestpart = i;
			}
			continue;
		}
	}

	*mindist = bestdist;
	return bestpart;
}
/*
===============
G_HitModelCheck
by: Spoon
date: 21.8.2001
updated: 17.8.2002

checks if a player was hit by using the .hit-model
===============
*/
int G_HitModelCheck(hit_data_t *data, vec3_t origin,
				   vec3_t angles, vec3_t torso_angles, vec3_t head_angles,
				   lerpFrame_t *torso, lerpFrame_t *legs,
				   vec3_t start, vec3_t end){
	int i;
	float mindist = 0;
	int part;

	// begin with the nearest

	for(i=0; i<NUM_HIT_LOCATIONS; i++){
		part = G_NearestPart(data, origin, angles, torso_angles, head_angles,
			torso, legs, start, end, &mindist);

		if( G_BuildHitModel(data, origin, angles, torso_angles, head_angles,
			torso, legs, start, end, part)){
			return part;
		}
	}

	return -1;
}

/*
==================
PARSEHITFILECODE
==================
*/

/*
======================
G_ParseAnimationFile

Read a configuration file containing animation counts and rates
models/wq3_players/wq_male1/animation.cfg, etc
======================
*/
static qboolean	G_ParseAnimationFile( const char *filename, hit_data_t *data ) {
	char		*text_p, *prev;
	int			len;
	int			i;
	char		*token;
	float		fps;
	int			skip;
	char		text[20000];
	fileHandle_t	f;
	animation_t *animations;

	animations = data->animations;

	// load the file
	len = trap_FS_FOpenFile( filename, &f, FS_READ );
	if ( len <= 0 ) {
		return qfalse;
	}
	if ( len >= sizeof( text ) - 1 ) {
		G_Printf( "File %s too long\n", filename );
		return qfalse;
	}
	trap_FS_Read( text, len, f );
	text[len] = 0;
	trap_FS_FCloseFile( f );

	// parse the text
	text_p = text;
	skip = 0;	// quite the compiler warning

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
			continue;
		} else if ( !Q_stricmp( token, "headoffset" ) ) {
			for ( i = 0 ; i < 3 ; i++ ) {
				token = COM_Parse( &text_p );
				if ( !token ) {
					break;
				}
			}
			continue;
		} else if ( !Q_stricmp( token, "sex" ) ) {
			token = COM_Parse( &text_p );
			if ( !token ) {
				break;
			}
			continue;
		}

		// if it is a number, start parsing animations
		if ( token[0] >= '0' && token[0] <= '9' ) {
			text_p = prev;	// unget the token
			break;
		}
		Com_Printf( "unknown token '%s' is %s\n", token, filename );
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
		// TODOANIM
		/*if ( i == LEGS_WALKCR ) {
			skip = animations[LEGS_WALKCR].firstFrame - animations[TORSO_GESTURE].firstFrame;
		}
		if ( i >= LEGS_WALKCR && i<TORSO_GETFLAG) {
			animations[i].firstFrame -= skip;
		}*/

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

		//G_Printf("%i. anim\nstart: %i\nnum: \n%i framelerp\n\n", i, data->animations[i].firstFrame,
		//	data->animations[i].numFrames, data->animations[i].frameLerp);
	}

	if ( i != MAX_ANIMATIONS ) {
		G_Printf( "Error parsing animation file: %s", filename );
		return qfalse;
	}

	// crouch backward animation
	//memcpy(&animations[LEGS_BACKCR], &animations[LEGS_WALKCR], sizeof(animation_t));
	//animations[LEGS_BACKCR].reversed = qtrue;
	//
	return qtrue;
}

/*
====================
G_OpenFileAiNode
====================
*/
extern	vec3_t	ai_nodes[MAX_AINODES];
extern	int		ai_nodecount;
qboolean G_OpenFileAiNode(const char *filename){
	int				i;
	fileHandle_t	file;
	int				len;
	char			headercheck[sizeof(AINODE_FILE_HEADER)+1];
	char			buf[MAX_AINODEFILE+1];

	int				pos = 0;

	G_Printf("AiNode-File: %s:\n", filename);

	len = trap_FS_FOpenFile( filename, &file, FS_READ );
	if ( !file ) {
		trap_FS_FCloseFile( file );
		return qfalse;
	}
	if ( len > MAX_AINODEFILE ) {
		G_Printf("ai file too big: %s! %d > " STRING(AINODE_FILE_MAX_SIZE) "\n", filename);
		trap_FS_FCloseFile( file );
		return qfalse;
	}
	trap_FS_Read( buf, len, file );
	buf[len] = 0;
	trap_FS_FCloseFile( file );

	// read the header
	Q_strncpyz(headercheck, buf, sizeof(AINODE_FILE_HEADER));
	pos += sizeof(AINODE_FILE_HEADER);

	//check
	if(Q_stricmp(headercheck, AINODE_FILE_HEADER)){
		G_Printf("no valid ai-file: %s\n", filename);
		return qfalse;
	}

	// now read the waypoints into the memory
	for(i=0; i<MAX_AINODES; i++){
		//trace_t trace;
		if(len <= pos){
			break;
		}
		Com_Memcpy(ai_nodes[i], buf+pos, sizeof(vec3_t));
		pos += sizeof(vec3_t);
		//G_Printf("%i. %f %f %f\n", i+1, ai_nodes[i][0], ai_nodes[i][1], ai_nodes[i][2]);

		// check if they're in solid
		/*trap_Trace(&trace, ai_nodes[i], NULL, NULL, ai_nodes[i], -1, CONTENTS_SOLID);
		if(trace.allsolid || trace.startsolid)
			G_Printf("node %i is in solid\n", i);
		else
			G_Printf("node %i is ok\n", i);*/
	}

	ai_nodecount = i;
	G_Printf("%i nodes parsed\n", ai_nodecount);
	return qtrue;
}

/*
===================
G_ParseHitFile
by: Spoon
18.8.2001

Parses the hit-file for better hit detection
===================
*/

// Hit data
/*typedef struct hit_header {
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
} hit_data_t;*/

#define HIT_DEBUG
qboolean G_ParseHitFile(hit_data_t *hit_data, int part){

	int				len, f, i;
	fileHandle_t	file;
	hit_tag_t		*ptag;
	hit_header_t	header;
	char			buf[MAX_HITFILE];

	char			name[64];
//	int				filecount = 0;

	int				pos = 0;

	if(part == PART_UPPER){
		strcpy(name, "models/wq3_players/upper.hit");
	} else if(part == PART_LOWER){
		strcpy(name, "models/wq3_players/lower.hit");
	} else
		strcpy(name, "models/wq3_players/head.hit");

	len = trap_FS_FOpenFile( name, &file, FS_READ );
	if ( !file ) {
#ifdef HIT_DEBUG
		G_Error(
#else
		G_Printf(
#endif
		"hit file not found: %s\n", name  );

		trap_FS_FCloseFile( file );
		return qfalse;
	}
	if ( len >= MAX_HITFILE ) {
#ifdef HIT_DEBUG
		G_Error(
#else
		G_Printf(
#endif
		"hit file too large: %s is %i, max allowed is %i\n", name, len, MAX_HITFILE  );
		trap_FS_FCloseFile( file );
		return qfalse;
	}
	trap_FS_Read( buf, len, file );
	//buf[len] = 0;
	trap_FS_FCloseFile( file );

	//
	// header
	//

	// read in the header seperately, because of size-problems in the qvm
	Com_Memcpy(&header.ident, buf+pos, sizeof(short));
#if defined Q3_VM || !defined LittleShort	// QVM or SO on a big endian processor
	header.ident = LittleShort(header.ident);  // fix endianness
#endif
	pos += sizeof(short);

	Com_Memcpy(&header.numFrames, buf+pos, sizeof(short));
#if defined Q3_VM || !defined LittleShort	// QVM or SO on a big endian processor
	header.numFrames = LittleShort(header.numFrames);
#endif
	pos += sizeof(short);

	Com_Memcpy(&header.numMeshes, buf+pos, sizeof(short));
#if defined Q3_VM || !defined LittleShort	// QVM or SO on a big endian processor
	header.numMeshes = LittleShort(header.numMeshes);
#endif
	pos += sizeof(short);

	if(header.ident != HIT_IDENT){
#ifdef HIT_DEBUG
		G_Error(
#else
		G_Printf(
#endif
		"no valid hitfile: %s\nident: %i\nframes: %i\nmeshes: %i\n%i %i %i %i %i", name,
		(int)header.ident, (int)header.numFrames, (int)header.numMeshes,
		sizeof(header.ident), sizeof(header.numFrames), sizeof(header.numMeshes),
		sizeof(short), sizeof(int));
		return qfalse;
	}

	//
	// tags
	//

	if(part != PART_HEAD){
		for(f = 0; f < header.numFrames; f++){
#if defined Q3_VM || !defined LittleFloat
			int j ;
#endif
			if(part==PART_UPPER)
				ptag = &hit_data->tag_head[f];
			else if(part==PART_LOWER)
				ptag = &hit_data->tag_torso[f];
			else {
#ifdef HIT_DEBUG
				G_Error(
#else
				G_Printf(
#endif
				"no tag found in hitfile: %s\n", name);
				return qfalse;
			}

			Com_Memcpy(ptag, buf+pos, sizeof(hit_tag_t));
#if defined Q3_VM || !defined LittleFloat	// QVM or SO on a big endian processor
			for (j = 0; j < 3; j++) { // fix endianness
				ptag->angles[j] = LittleFloat(((const float *)&ptag->angles)[j]);
				ptag->origin[j] = LittleFloat(((const float *)&ptag->origin)[j]);
			}
#endif
			AnglesNormalize180(ptag->angles);

			pos += sizeof(hit_tag_t);
		}
	}

	//
	// meshes
	//

	for( i = 0; i < header.numMeshes; i++){
#if defined Q3_VM || !defined LittleShort	// QVM or SO on a big endian processor
		int j, k, hit_num = -1;
#else
		int j, hit_num = -1;
#endif
		mesh_header_t m_header;

		// load next file
		/*if(pos >= len && filecount < 2 && part != PART_HEAD){
			filecount++;

			pos = 0;

			if(filecount == 1){
				if(part == PART_UPPER){
					strcpy(name, "models/wq3_players/upper_1.hit");
				} else if (part == PART_LOWER){
					strcpy(name, "models/wq3_players/lower_1.hit");
				} else {
					G_Printf("no valid operation\n");
					return qfalse;
				}
			} else if(filecount==2) {
				if(part == PART_UPPER){
					strcpy(name, "models/wq3_players/upper_2.hit");
				} else if (part == PART_LOWER){
					strcpy(name, "models/wq3_players/lower_2.hit");
				} else {
					G_Printf("no valid operation\n");
					return qfalse;
				}
			} else {
				G_Printf("no more hit-files allowed\n");
				return qfalse;
			}

			Com_Printf("loading new file %s\n", name);

			len = trap_FS_FOpenFile( name, &file, FS_READ );
			if ( !file ) {
				G_Printf(  "hit file not found: %s\n", name  );
				trap_FS_FCloseFile( file );
				return qfalse;
			}
			if ( len >= MAX_HITFILE ) {
				G_Printf(  "hit file too large: %s is %i, max allowed is %i\n", name, len, MAX_HITFILE  );
				trap_FS_FCloseFile( file );
				return qfalse;
			}
			trap_FS_Read( buf, len, file );
			buf[len] = 0;
			trap_FS_FCloseFile( file );
		}*/

		Com_Memcpy(&m_header.name, buf+pos, sizeof(m_header.name));
		pos += sizeof(m_header.name);

		Com_Memcpy(&m_header.a1, buf+pos, sizeof(short));
#if defined Q3_VM || !defined LittleShort	// QVM or SO on a big endian processor
		m_header.a1 = LittleShort(m_header.a1);  // fix endianness
#endif
		pos += sizeof(short);

		Com_Memcpy(&m_header.a2, buf+pos, sizeof(short));
#if defined Q3_VM || !defined LittleShort	// QVM or SO on a big endian processor
		m_header.a2 = LittleShort(m_header.a2);
#endif
		pos += sizeof(short);

		Com_Memcpy(&m_header.dir1, buf+pos, 3*sizeof(short));
#if defined Q3_VM || !defined LittleShort	// QVM or SO on a big endian processor
		for (k = 0; k < 3; k++)
			m_header.dir1[k] = LittleShort(m_header.dir1[k]);
#endif
		pos += 3*sizeof(short);

		Com_Memcpy(&m_header.dir2, buf+pos, 3*sizeof(short));
#if defined Q3_VM || !defined LittleShort	// QVM or SO on a big endian processor
		for (k = 0; k < 3; k++)
			m_header.dir2[k] = LittleShort(m_header.dir2[k]);
#endif
		pos += 3*sizeof(short);

		Com_Memcpy(&m_header.length, buf+pos, sizeof(short));
#if defined Q3_VM || !defined LittleShort	// QVM or SO on a big endian processor
		m_header.length = LittleShort(m_header.length);
#endif
		pos += sizeof(short);

		// look for the right hit_num
		for(j = 0; j < NUM_HIT_LOCATIONS; j++){
			if(hit_info[j].hit_part != part)
				continue;

			if(!strcmp(hit_info[j].meshname, m_header.name)){
				hit_num = j;
				break;
			}
		}

		if(hit_num == -1){
#ifdef HIT_DEBUG
		G_Error(
#else
		G_Printf(
#endif
			"invalid mesh found %s in %s.\n", m_header.name, name);
			return qfalse;
		}

		Com_Memcpy(&hit_data->meshes[hit_num].header, &m_header, sizeof(short)*9+sizeof(m_header.name));

		// now parse the frames
		for(f=0; f<header.numFrames; f++){
			Com_Memcpy(&hit_data->meshes[hit_num].pos[f], buf+pos, sizeof(hit_mesh_t));
#if defined Q3_VM || !defined LittleShort	// QVM or SO on a big endian processor
			for (k = 0; k < 3; k++) { // fix endianness
				hit_data->meshes[hit_num].pos[f].normal[k]
				             = LittleShort(hit_data->meshes[hit_num].pos[f].normal[k]);
				hit_data->meshes[hit_num].pos[f].origin[k]
				             = LittleShort(hit_data->meshes[hit_num].pos[f].origin[k]);
			}
#endif
			pos += sizeof(hit_mesh_t);
		}

	}
	return qtrue;
}

/*
=================
G_LoadHitFiles
by: Spoon
18.8.2001
=================
*/
qboolean G_LoadHitFiles(hit_data_t *hit_data){

	//parse lower body
	if(!G_ParseHitFile(hit_data, PART_LOWER)){
		G_Printf("models/wq3_players/lower.hit could not be loaded\n");
		return qfalse;
	}

	//parse upper body
	if(!G_ParseHitFile(hit_data, PART_UPPER)){
		G_Printf("models/wq3_players/upper.hit could not be loaded\n");
		return qfalse;
	}

	//parse head
	if(!G_ParseHitFile(hit_data, PART_HEAD)){
		G_Printf("models/wq3_players/head.hit could not be loaded\n");
		return qfalse;
	}

	//load animations
	if(!G_ParseAnimationFile("models/wq3_players/wq_male1/animation.cfg", hit_data)){
		G_Printf("models/wq3_players/wq_male1/animation.cfg could not be loaded\n");
		return qfalse;
	}

	return qtrue;
}

///////////////////////////////////////////////////////////////////////////////
//animation code (copied from cgame), needed for server-side-prediction
///////////////////////////////////////////////////////////////////////////////
/*
===============
G_SetLerpFrameAnimation

may include ANIM_TOGGLEBIT
===============
*/
static void G_SetLerpFrameAnimation( lerpFrame_t *lf, int newAnimation) {
	animation_t	*anim;

	lf->animationNumber = newAnimation;
	newAnimation &= ~ANIM_TOGGLEBIT;

	if ( newAnimation < 0 || newAnimation >= MAX_TOTALANIMATIONS ) {
		G_Error( "Server: Bad animation number: %i", newAnimation );
	}

	anim = &hit_data.animations[newAnimation ];
	//G_Printf("%i %i %f\n", anim->firstFrame, anim->numFrames, anim->frameLerp);
	//G_Printf("%i %i %f\n", hit_data[hit_data_num].animations[newAnimation].firstFrame,
	//	hit_data[hit_data_num].animations[newAnimation].numFrames,
	//	hit_data[hit_data_num].animations[newAnimation].frameLerp);

	lf->animation = anim;
	lf->animationTime = lf->frameTime + anim->initialLerp;
}

/*
===============
G_ClearLerpFrame
===============
*/
void G_ClearLerpFrame( lerpFrame_t *lf, int animationNumber) {
	lf->frameTime = lf->oldFrameTime = level.time;
	G_SetLerpFrameAnimation( lf, animationNumber);
	lf->oldFrame = lf->frame = lf->animation->firstFrame;
}

/*
===============
G_RunLerpFrame

Sets frame, oldFrame and backlerp
level.time should be between oldFrameTime and frameTime after exit
===============
*/
void G_RunLerpFrame( lerpFrame_t *lf, int newAnimation, float speedScale) {
	int			f, numFrames;
	animation_t	*anim;
	qboolean	newanim = qfalse;

	// see if the animation sequence is switching
	if ( newAnimation != lf->animationNumber || !lf->animation ) {
		G_SetLerpFrameAnimation( lf, newAnimation);
		newanim = qtrue;
	}

	// if we have passed the current frame, move it to
	// oldFrame and calculate a new frame
	if ( level.time >= lf->frameTime ) {
		lf->oldFrame = lf->frame;
		lf->oldFrameTime = lf->frameTime;

		// get the next frame based on the animation
		anim = lf->animation;
		if ( !anim->frameLerp ) {
			//G_Printf("hehe stopped\n");
			return;		// shouldn't happen
		}

		if ( level.time < lf->animationTime ) {
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
				lf->frameTime = level.time;
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
		if ( level.time > lf->frameTime ) {
			lf->frameTime = level.time;
		}
	}

	if ( lf->frameTime > level.time + 200 ) {
		lf->frameTime = level.time;
	}

	if ( lf->oldFrameTime > level.time ) {
		lf->oldFrameTime = level.time;
	}
	// calculate current lerp value
	if ( lf->frameTime == lf->oldFrameTime ) {
		lf->backlerp = 0;
	} else {
		lf->backlerp = 1.0 - (float)( level.time - lf->oldFrameTime ) / ( lf->frameTime - lf->oldFrameTime );
	}
}

/*
==============================
PLAYER_ANGLES, prediction code for the game module(mainly copied from cgame)
==============================
*/
/*
==================
G_SwingAngles
==================
*/
static void G_SwingAngles( float destination, float swingTolerance, float clampTolerance,
					float speed, float *angle, qboolean *swinging ) {
	float	swing;
	float	move;
	float	scale;
	int		frametime = level.time - level.previousTime;

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
		move = frametime * scale * speed;
		if ( move >= swing ) {
			move = swing;
			*swinging = qfalse;
		}
		*angle = AngleMod( *angle + move );
	} else if ( swing < 0 ) {
		move = frametime * scale * -speed;
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
===============
G_PlayerAngles

Handles seperate torso motion

  legs pivot based on direction of movement

  head always looks exactly at cent->lerpAngles

  if motion < 20 degrees, show in head only
  if < 45 degrees, also show in torso

  copied from cgame
===============
*/
void G_PlayerAngles( gentity_t *ent, vec3_t legs[3], vec3_t torso[3] ) {
	vec3_t		legsAngles, torsoAngles, headAngles;
	float		dest;
	static	int	movementOffsets[8] = { 0, 22, 45, -22, 0, 22, -45, -22 };
	vec3_t		velocity;
	float		speed;
	int			dir;
	gclient_t	*client = ent->client;

	VectorCopy( client->ps.viewangles, headAngles );
	headAngles[YAW] = AngleMod( headAngles[YAW] );
	VectorClear( legsAngles );
	VectorClear( torsoAngles );

	// --------- yaw -------------

	// allow yaw to drift a bit
	if ( ( client->ps.legsAnim & ~ANIM_TOGGLEBIT ) != LEGS_IDLE
		|| ( // Tequila comment: Fixed a wrong test here generating gcc warning
		( client->ps.torsoAnim & ~ANIM_TOGGLEBIT ) != TORSO_KNIFE_STAND
		&& ( client->ps.torsoAnim & ~ANIM_TOGGLEBIT ) != TORSO_PISTOL_STAND
		&& ( client->ps.torsoAnim & ~ANIM_TOGGLEBIT ) != TORSO_PISTOLS_STAND
		&& ( client->ps.torsoAnim & ~ANIM_TOGGLEBIT ) != TORSO_RIFLE_STAND
		&& ( client->ps.torsoAnim & ~ANIM_TOGGLEBIT ) != TORSO_GATLING_STAND
		&& ( client->ps.torsoAnim & ~ANIM_TOGGLEBIT ) != TORSO_HOLSTERED)) {
		// if not standing still, always point all in the same direction
		client->torso.yawing = qtrue;	// always center
		client->torso.pitching = qtrue;	// always center
		client->legs.yawing = qtrue;	// always center
	}

	// adjust legs for movement dir
	if ( ent->s.eFlags & EF_DEAD ) {
		// don't let dead bodies twitch
		dir = 0;
	} else {
		dir = ent->s.angles2[YAW];
		if ( dir < 0 || dir > 7 ) {
			dir = 0;
			//G_Error( "Bad player movement angle" );
		}
	}
	legsAngles[YAW] = headAngles[YAW] + movementOffsets[ dir ];
	torsoAngles[YAW] = headAngles[YAW] + 0.25 * movementOffsets[ dir ];

	// torso
	G_SwingAngles( torsoAngles[YAW], 25, 90, 0.3f, &client->torso.yawAngle, &client->torso.yawing );
	G_SwingAngles( legsAngles[YAW], 40, 90, 0.3f, &client->legs.yawAngle, &client->legs.yawing );

	torsoAngles[YAW] = client->torso.yawAngle;
	legsAngles[YAW] = client->legs.yawAngle;

	// --------- pitch -------------

	// only show a fraction of the pitch angle in the torso
	if ( headAngles[PITCH] > 180 ) {
		dest = (-360 + headAngles[PITCH]) * 0.75f;
	} else {
		dest = headAngles[PITCH] * 0.75f;
	}
	G_SwingAngles( dest, 15, 30, 0.1f, &client->torso.pitchAngle, &client->torso.pitching );
	torsoAngles[PITCH] = client->torso.pitchAngle;

	// --------- roll -------------


	// lean towards the direction of travel
	VectorCopy( ent->s.pos.trDelta, velocity );
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

	// pain twitch
	//G_AddPainTwitch( cent, torsoAngles );

	// pull the angles back out of the hierarchial chain
	// pull the angles back out of the hierarchial chain
	AnglesSubtract( torsoAngles, legsAngles, torsoAngles );
	AnglesToAxis( legsAngles, legs );
	AnglesToAxis( torsoAngles, torso );
}
