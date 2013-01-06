/*
 *  tr_glslprogs.c
 *
 *  Copyright 2007 Gord Allott <gordallott@gmail.com>
 *  Copyright (C) 2005-2010 Smokin' Guns
 *
 *  This file is part of Smokin' Guns.
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 *  MA 02110-1301, USA.
 */

#include "tr_local.h"
#include "tr_glslprogs.h"
//for all the glsl source its best if we keep them in the mainline code unless
//someone (not me!) wants to implement them into the q3 mainline pak code
//and of course provide a 'standard library' for paks that don't have the glsl
//sources in them. this would allow mod authors to code their own effects but
//meh, they might as well just hack them into the source and send the changes
//upstream.

//when implementing glsl code try and stick to the carmark q3 coding style ie,
//variables go myVariable and stuff. also end lines with \n\ instead of \ as it
//makes debugging glsl code 15,823x easier. (looks nasty either way...)


//this vertex shader is basically complete, we shouldn't really need anything
//else(?). it just maps the vertex position to a screen position.

gslsProg_t glsl_progs[] =
{
	{
		"glslBase_vert", NULL, "\n\
void main() {\n\
  gl_TexCoord[0] = gl_MultiTexCoord0;\n\
  gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;\n\
}\
"
	},
	{
		"glslGauss9", NULL, "\n\
#define NK9_0 0.17857142857142855\n\
#define NK9_1 0.1607142857142857\n\
#define NK9_2 0.14285714285714285\n\
#define NK9_3 0.071428571428571425\n\
#define NK9_4 0.035714285714285712\n\
\
vec4 GaussPass(sampler2D src, vec2 coord, vec2 blrsize) {\n\
  //blrsize is the size (in texture coordinates) of the blur kernel\n\
\n\
  vec4 accum;\n\
  vec2 step, pos;\n\
  step = blrsize / 9.0;\n\
  pos = coord - (step * 4.0);\n\
\n\
  accum  = texture(src, pos) * NK9_4; pos += step;\n\
  accum += texture(src, pos) * NK9_3; pos += step;\n\
  accum += texture(src, pos) * NK9_2; pos += step;\n\
  accum += texture(src, pos) * NK9_1; pos += step;\n\
  accum += texture(src, pos) * NK9_0; pos += step;\n\
  accum += texture(src, pos) * NK9_1; pos += step;\n\
  accum += texture(src, pos) * NK9_2; pos += step;\n\
  accum += texture(src, pos) * NK9_3; pos += step;\n\
  accum += texture(src, pos) * NK9_4; pos += step;\n\
\n\
  return accum;\n\
\n\
}\n\
"
	},
	{
		"glslGauss7", NULL, "\n\
#define NK7_0 0.19230769230769229\n\
#define NK7_1 0.18269230769230768\n\
#define NK7_2 0.15384615384615385\n\
#define NK7_3 0.067307692307692304\n\
\n\
vec4 GaussPass(sampler2D src, vec2 coord, vec2 blrsize) {\n\
  //blrsize is the size (in texture coordinates) of the blur kernel\n\
\n\
  vec4 accum;\n\
  vec2 step, pos;\n\
  step = blrsize / 7.0;\n\
  pos = coord - (step * 3.0);\n\
\n\
  accum  = texture(src, pos) * NK7_3; pos += step;\n\
  accum += texture(src, pos) * NK7_2; pos += step;\n\
  accum += texture(src, pos) * NK7_1; pos += step;\n\
  accum += texture(src, pos) * NK7_0; pos += step;\n\
  accum += texture(src, pos) * NK7_1; pos += step;\n\
  accum += texture(src, pos) * NK7_2; pos += step;\n\
  accum += texture(src, pos) * NK7_3; pos += step;\n\
  \n\
  return accum;\n\
\n\
}\n\
"
	},
	{
		"glslGauss5", NULL, "\n\
#define NK5_0 0.33333333\n\
#define NK5_1 0.26666666\n\
#define NK5_2 0.06666666\n\
\n\
vec4 GaussPass(sampler2D src, vec2 coord, vec2 blrsize) {\n\
  //blrsize is the size (in texture coordinates) of the blur kernel\n\
  \n\
  vec4 accum;\n\
  vec2 step, pos;\n\
  step = blrsize / 5.0;\n\
  pos = coord - (step * 2.0);\n\
  \n\
  accum  = texture(src, pos) * NK5_2; pos += step;\n\
  accum += texture(src, pos) * NK5_1; pos += step;\n\
  accum += texture(src, pos) * NK5_0; pos += step;\n\
  accum += texture(src, pos) * NK5_1; pos += step;\n\
  accum += texture(src, pos) * NK5_2; pos += step;\n\
  \n\
  return accum;\n\
  \n\
}\n\
"
	},
	{
		"glslBlurMain", NULL, "\n\
uniform sampler2D srcSampler;\n\
uniform vec2 blurSize;\n\
void main()\n\
{\n\
  gl_FragColor = GaussPass(srcSampler, gl_TexCoord[0].xy, blurSize);\n\
}\n\
"
	},
	{
		"glslSigScreen", NULL, "\n\
uniform sampler2D srcSampler;\n\
uniform sampler2D blurSampler;\n\
//#define sharpness 0.75 \n\
uniform float	 sharpness;\n\
//#define brightness 0.85\n\
uniform float	 brightness;\n\
#define SIGMOIDAL_BASE        2.0\n\
#define SIGMOIDAL_RANGE      20.0\n\
\n\
void main()\n\
{\n\
    \n\
  vec4 blurcolor 	= texture( blurSampler, gl_TexCoord[0].xy);\n\
  vec4 basecolor 	= texture( srcSampler, gl_TexCoord[0].xy);\n\
  \n\
  //vec4 val = 1.0 / (1.0 + exp (-(SIGMOIDAL_BASE + (sharpness * SIGMOIDAL_RANGE)) * (blurcolor - 0.5)));\n\
  vec4 val;\n\
  val = -(SIGMOIDAL_BASE + (sharpness * SIGMOIDAL_RANGE)) * (blurcolor - 0.5);\n\
  val = 1.0 + pow(vec4(2.718281828459045), val);\n\
  val = 1.0 / val;\n\
  val = val * brightness;\n\
  \n\
  gl_FragColor = 1.0 - ((1.0 - basecolor) * (1.0 - val));\n\
}\n\
"
	},
	{
		"glslSobel", NULL, "\n\
float sobel(sampler2D tex, vec2 basecoord, vec2 texel_size) {\n\
  /* computes a sobel value from the surrounding pixels */\n\
  vec4 hori, vert;\n\
  //vec2 basecoord = coord;\n\
  float stepw, steph;\n\
  stepw = texel_size.x;\n\
  steph = texel_size.y;\n\
  \n\
  vert  = texture(tex, basecoord + vec2(-stepw, -steph)) * -1.0;\n\
  vert += texture(tex, basecoord + vec2(-stepw,  0.0  )) * -2.0;\n\
  vert += texture(tex, basecoord + vec2(-stepw, +steph)) * -1.0;\n\
  \n\
  vert += texture(tex, basecoord + vec2( stepw, -steph)) * 1.0;\n\
  vert += texture(tex, basecoord + vec2( stepw,  0.0  )) * 2.0;\n\
  vert += texture(tex, basecoord + vec2( stepw, +steph)) * 1.0;\n\
  \n\
  hori  = texture(tex, basecoord + vec2(-stepw, -steph)) * -1.0;\n\
  hori += texture(tex, basecoord + vec2( 0.0  , -steph)) * -2.0;\n\
  hori += texture(tex, basecoord + vec2(+stepw, -steph)) * -1.0;\n\
\n\
  hori += texture(tex, basecoord + vec2(-stepw,  steph)) * 1.0;\n\
  hori += texture(tex, basecoord + vec2( 0.0  ,  steph)) * 2.0;\n\
  hori += texture(tex, basecoord + vec2(+stepw,  steph)) * 1.0;\n\
\n\
  /* could use dist() but this is more compatible */\n\
  return sqrt(float((vert * vert) + (hori * hori)));\n\
  \n\
}\n\
"
	},
	{
		"glslSobelZ", NULL, "\n\
float sobel(sampler2D tex, vec2 basecoord, vec2 texel_size) {\n\
  /* computes a sobel value from the surrounding pixels */\n\
  vec4 hori, vert;\n\
  //vec2 basecoord = coord;\n\
  float stepw, steph;\n\
  stepw = texel_size.x;\n\
  steph = texel_size.y;\n\
  \n\
  //when used with a zbuffer we first need to figure out how deep the texel is\n\
  float depth = texture(tex, basecoord).r;\n\
  depth = ((exp(depth * 5.0) /  2.7182818284590451) - 0.36787944117144233);\n\
  depth = depth * 1.5819767068693265;\n\
  \n\
  vert  = texture(tex, basecoord + vec2(-stepw, -steph)) * -1.0;\n\
  vert += texture(tex, basecoord + vec2(-stepw,  0.0  )) * -2.0;\n\
  vert += texture(tex, basecoord + vec2(-stepw, +steph)) * -1.0;\n\
  \n\
  vert += texture(tex, basecoord + vec2( stepw, -steph)) * 1.0;\n\
  vert += texture(tex, basecoord + vec2( stepw,  0.0  )) * 2.0;\n\
  vert += texture(tex, basecoord + vec2( stepw, +steph)) * 1.0;\n\
  \n\
  hori  = texture(tex, basecoord + vec2(-stepw, -steph)) * -1.0;\n\
  hori += texture(tex, basecoord + vec2( 0.0  , -steph)) * -2.0;\n\
  hori += texture(tex, basecoord + vec2(+stepw, -steph)) * -1.0;\n\
\n\
  hori += texture(tex, basecoord + vec2(-stepw,  steph)) * 1.0;\n\
  hori += texture(tex, basecoord + vec2( 0.0  ,  steph)) * 2.0;\n\
  hori += texture(tex, basecoord + vec2(+stepw,  steph)) * 1.0;\n\
\n\
  /* could use dist() but this is more compatible */\n\
  return sqrt(float((vert * vert) + (hori * hori))) * depth;\n\
  \n\
}\n\
"
	},
	{
		"glslToonColour", NULL, "\n\
vec4 ToonColour(vec4 incolour) {\n\
\n\
  vec3 huetemp;\n\
  huetemp.x = 0.0;\n\
  huetemp.y = 0.0;\n\
  huetemp.z = 0.0;\n\
\n\
  huetemp.x = incolour.x + incolour.y + incolour.z;\n\
  huetemp.y = 1.0 / huetemp.x;\n\
  \n\
  /* multiply the pixel colourby 1 / sumrgb */\n\
  incolour = incolour * huetemp.y;\n\
  /* get the  tones */\n\
  \n\
  if (huetemp.x > 0.2) {\n\
    huetemp.z = 0.4;\n\
   \n\
  } else {\n\
    huetemp.z = 0.0;\n\
  }\n\
  \n\
  if (huetemp.x > 0.4) {\n\
    huetemp.y = 1.0;\n\
  } else {\n\
    huetemp.y = 0.0;\n\
  }\n\
  \n\
  if (huetemp.x > 1.0) {\n\
    huetemp.x = 1.5;\n\
  } else {\n\
    huetemp.x = 0.0;\n\
  }\n\
\n\
  \n\
  /* sum the huetones */\n\
  \n\
  huetemp.x = huetemp.x + huetemp.y + huetemp.z;\n\
  \n\
  /* multiply the pixel colour with the resulting intensity */\n\
  \n\
  incolour = incolour * huetemp.x;\n\
\n\
  return vec4(incolour);\n\
}\n\
"
	},
	{
		"glslRotoscope", NULL, "\n\
uniform vec2 texelSize;\n\
uniform sampler2D srcSampler;\n\
void main()\n\
{\n\
\n\
  float fragsobel = sobel(srcSampler, gl_TexCoord[0].xy, texelSize);\n\
  vec4 final_color = ToonColour(texture(srcSampler, gl_TexCoord[0].xy));\n\
\n\
  fragsobel = 1.0 - clamp(fragsobel - 0.2, 0.0, 1.0);\n\
  gl_FragColor = final_color * fragsobel;\n\
\n\
}\n\
"
	},
	{
		"glslRotoscopeZ", NULL, "\n\
uniform vec2 texelSize;\n\
uniform sampler2D srcSampler;\n\
uniform sampler2D depthSampler;\n\
\n\
void main()\n\
{\n\
\n\
  float fragsobel = sobel(depthSampler, gl_TexCoord[0].xy, texelSize);\n\
  vec4 final_color = ToonColour(texture(srcSampler, gl_TexCoord[0].xy));\n\
\n\
  fragsobel = clamp(fragsobel, 0.0, 1.0);\n\
  //gl_FragColor = vec4(texture(depthSampler, gl_TexCoord[0].xy).r) / 2.0;\n\
  //final_color = 1.0;\n\
  gl_FragColor = final_color * (1.0 - fragsobel);\n\
\n\
}\n\
"
	}
};

const int max_progs = ARRAY_LEN( glsl_progs );

void R_GLSLProgs_Init( void ) {
	int i = max_progs;
	int len;
	void *buffer;

	if (!glslSupported) {
		return;
	}

	while (i--) {
		len = -1;

		if (ri.Cvar_VariableIntegerValue("com_developer")) {
			len = ri.FS_ReadFile( va("%s.cfg",glsl_progs[i].name), &buffer );
		}

		if (len<0) {
			len = ri.FS_ReadFile( va("scripts/%s.glsl",glsl_progs[i].name), &buffer );
			if (len>0)
				ri.Printf( PRINT_DEVELOPER, "Loading scripts/%s.glsl GLSL program file (%i bytes)\n", glsl_progs[i].name, len );
		} else {
			ri.Printf( PRINT_DEVELOPER, "Loading %s.cfg GLSL program file (%i bytes)\n", glsl_progs[i].name, len );
		}

		if (len>0) {
			if ( strlen( buffer ) != len ) {
				glsl_progs[i].program = glsl_progs[i].default_program ;
				ri.Printf( PRINT_WARNING, "Can't load %s GLSL program from file\n", glsl_progs[i].name );
			} else {
				byte	*program;
				program = ri.Malloc(len+1);
				Com_Memcpy(program,buffer,len);
				program[len]=0;
				glsl_progs[i].program = (char *)program;
				ri.Printf( PRINT_ALL, "Loaded %s GLSL program (%i bytes)\n", glsl_progs[i].name, len );
				if (ri.Cvar_VariableIntegerValue("com_developer")) {
					ri.Printf( PRINT_DEVELOPER, "==== %s GLSL source ====\n%s\n", glsl_progs[i].name, glsl_progs[i].program );
				}
			}
		} else {
			glsl_progs[i].program = glsl_progs[i].default_program ;
			ri.Printf( PRINT_ALL, "Using default %s GLSL program (%i bytes)\n",
				glsl_progs[i].name, (int)strlen(glsl_progs[i].program) );
		}

		if (buffer) {
			ri.FS_FreeFile(buffer);
			buffer = NULL ;
		}
	}
}

void R_GLSLProgs_Delete( void ) {
	int i = max_progs;

	if (!glslSupported) {
		return;
	}

	while (i--) {
		if (glsl_progs[i].program == glsl_progs[i].default_program)
			continue ;
		ri.Free((void *)glsl_progs[i].program);
	}
}

const char *R_GLSLGetProgByName( const char *name ) {
	int i = max_progs;

	if (!glslSupported) {
		return NULL;
	}

	while (i--) {
		if (!Q_stricmp(name,glsl_progs[i].name)) {
			return glsl_progs[i].program ;
		}
	}

	ri.Printf( PRINT_DEVELOPER, "Bug!!! You're asking for unknown '%s' GLSL program\n", name );
	return NULL ;
}
