/*
 *      tr_framebuffer.c
 *
 *      Copyright 2007 Gord Allott <gordallott@gmail.com>
 *      Copyright (C) 2005-2014 Smokin' Guns
 *
 *      This file is part of Smokin' Guns.
 *
 *      This program is free software; you can redistribute it and/or modify
 *      it under the terms of the GNU General Public License as published by
 *      the Free Software Foundation; either version 2 of the License, or
 *      (at your option) any later version.
 *
 *      This program is distributed in the hope that it will be useful,
 *      but WITHOUT ANY WARRANTY; without even the implied warranty of
 *      MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *      GNU General Public License for more details.
 *
 *      You should have received a copy of the GNU General Public License
 *      along with this program; if not, write to the Free Software
 *      Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 *      MA 02110-1301, USA.
 */
// tr_framebuffer.c: framebuffer object rendering path code
// Okay i am going to try and document what I doing here, appologies to anyone
// that already understands this. basically the idea is that normally everything
// opengl renders will be rendered into client memory, that is the space the
// graphics card reserves for anything thats going to be sent to the monitor.
// Using this method we instead redirect all the rendering to a separate bit of
// memory called a frame buffer.
// we can then bind this framebuffer to a texture and render that texture to the
// client memory again so that the image will be sent to the monitor. this
// redirection allows for some neat effects to be applied.

// Some ideas for what to use this path for:
//		- Bloom	-done
//		- Rotoscope cartoon effects (edge detect + colour mapping)
//		- Fake anti-aliasing. (edge detect and blur positive matches)
//		- Motion blur
//			- generate a speed vector based on how the camera has moved since
//			  the last frame and use that to compute per pixel blur vectors
//		- These would require mods to use some sort of framebuffer qvm api
//			- special effects for certain powerups
//			- Image Blur when the player is hit
//			- Depth of field blur


#include "tr_local.h"
#include "tr_glslprogs.h"

#ifndef GL_DEPTH_STENCIL_EXT
#define GL_DEPTH_STENCIL_EXT GL_DEPTH_STENCIL_NV
#endif

#ifndef GL_UNSIGNED_INT_24_8_EXT
#define GL_UNSIGNED_INT_24_8_EXT GL_UNSIGNED_INT_24_8_NV
#endif

qboolean	packedDepthStencilSupported;
qboolean	depthTextureSupported;
qboolean	separateDepthStencilSupported;

// Tequila: FBO Multisample support is still WIP
#define DISABLE_FBO_MULTISAMPLE_SUPPORT

struct glslobj {
	const char **vertex_glsl;
	int vert_numSources;
	const char **fragment_glsl;
	int frag_numSources;
	GLuint vertex;
	GLuint fragment;
	GLuint program;
} glslobj;

struct r_fbuffer {
	GLuint	fbo;
	GLuint	mfbo;					//fbo buffer for multisampling
	GLuint	numBuffers;
	GLuint	front;					//front buffer
	GLuint	back;					//back buffer
	GLuint	depth;					//depth buffer
	GLuint	stencil;				//stencil buffer
	int		modeFlags;				//the modeflags
} r_fbuffer;

qboolean needBlur = qfalse;			//is set if effects need a blur
qboolean rendered = qfalse;			//is set when FBO has been rendered
qboolean useFrameBuffer = qfalse;	//is set after FBO is safely initialized
qboolean useMultisample = qfalse;	//is set after multisample support is initialized
qboolean useBloomEffect = qfalse;	//is set after bloom effect is initialized
qboolean useRotoscopeEffect = qfalse;//is set after rotoscope effect is initialized
qboolean useRotoZEdgeEffect = qfalse;//is set after rotoscope zedge effect is selected
static int	blur_size = 256;		//is set after blur effect is initialized
static GLint	samples = 0;		//is set after multisample support is initialized

struct r_fbuffer screenBuffer;
struct r_fbuffer gaussblurBuffer;

//------------------------------
// better framebuffer creation
//------------------------------
// for this we do a more opengl way of figuring out what level of framebuffer
// objects are supported. we try each mode from 'best' to 'worst' until we
// get a mode that works.

#define FB_ZBUFFER 		0x01
#define FB_STENCIL 		0x02
#define FB_PACKED 		0x04
#define FB_ZTEXTURE 	0x08
#define FB_BACKBUFFER	0x10
#define FB_SEPARATEZS	0x20
#define FB_SMOOTH		0x40
#define FB_MULTISAMPLE	0x80

// defines not available in SDL<1.3
#ifndef GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE_EXT
#define GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE_EXT 0x8D56
#endif
#ifndef GL_MAX_SAMPLES_EXT
#define GL_MAX_SAMPLES_EXT 0x8D57
#endif
#ifndef GL_READ_FRAMEBUFFER_EXT
#define GL_READ_FRAMEBUFFER_EXT 0x8CA8
#endif
#ifndef GL_DRAW_FRAMEBUFFER_EXT
#define GL_DRAW_FRAMEBUFFER_EXT 0x8CA9
#endif

// FBO error reporting inspired by http://www.opengl.org/wiki/GL_EXT_framebuffer_multisample
qboolean R_CheckFramebufferStatus( const char* tag )
{
	const char* message ;
	GLenum status = qglCheckFramebufferStatusEXT(GL_FRAMEBUFFER_EXT);
	switch(status)
	{
		case GL_FRAMEBUFFER_COMPLETE_EXT:
			return qtrue;
		case GL_FRAMEBUFFER_UNSUPPORTED_EXT:
			message = "Framebuffer object format is unsupported by the video hardware. (GL_FRAMEBUFFER_UNSUPPORTED_EXT)(FBO - 820)";
			break;
		case GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT_EXT:
			message = "Incomplete attachment. (GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT_EXT)(FBO - 820)";
			break;
		case GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT_EXT:
			message = "Incomplete missing attachment. (GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT_EXT)(FBO - 820)";
			break;
		case GL_FRAMEBUFFER_INCOMPLETE_DIMENSIONS_EXT:
			message = "Incomplete dimensions. (GL_FRAMEBUFFER_INCOMPLETE_DIMENSIONS_EXT)(FBO - 820)";
			break;
		case GL_FRAMEBUFFER_INCOMPLETE_FORMATS_EXT:
			message = "Incomplete formats. (GL_FRAMEBUFFER_INCOMPLETE_FORMATS_EXT)(FBO - 820)";
			break;
		case GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER_EXT:
			message = "Incomplete draw buffer. (GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER_EXT)(FBO - 820)";
			break;
		case GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER_EXT:
			message = "Incomplete read buffer. (GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER_EXT)(FBO - 820)";
			break;
		case GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE_EXT:
			message = "Incomplete multisample buffer. (GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE_EXT)(FBO - 820)";
			break;
		default:
			//Programming error; will fail on all hardware
			message = "Some video driver error or programming error occured. Framebuffer object status is invalid. (FBO - 823)";
	}
	ri.Printf(PRINT_WARNING,"WARNING, %s: %s\n", tag, message );
	return qfalse;
}

//two functions to bind and unbind the main framebuffer, generally just to be
//called externaly
void R_FrameBufferBind(void) {
	if (!useFrameBuffer)
		return;

	qglBindFramebufferEXT(GL_FRAMEBUFFER_EXT, screenBuffer.fbo);
}

void R_FrameBufferUnBind(void) {
	if (!useFrameBuffer)
		return;

	qglBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);
}

void R_SetGL2DSize (int width, int height) {
	// set 2D virtual screen size
	qglViewport( 0, 0, width, height );
	qglScissor( 0, 0, width, height );
	qglMatrixMode(GL_PROJECTION);
	qglLoadIdentity ();
	qglOrtho (0, width, height, 0, 0, 1);
	qglMatrixMode(GL_MODELVIEW);
	qglLoadIdentity ();

	GL_State( GLS_DEPTHTEST_DISABLE );
	qglDisable( GL_BLEND );
}

void R_DrawQuadMT( GLuint tex1, GLuint tex2, int width, int height ) {
	qglEnable(GL_TEXTURE_2D);
	if ( glState.currenttextures[1] != tex2 ) {
		GL_SelectTexture( 1 );
		qglBindTexture(GL_TEXTURE_2D, tex2);
		glState.currenttextures[1] = tex2;
	}
	if ( glState.currenttextures[0] != tex1 ) {
		GL_SelectTexture( 0 );
		qglBindTexture(GL_TEXTURE_2D, tex1);
		glState.currenttextures[0] = tex1;
	}

	qglBegin(GL_QUADS);
	  qglMultiTexCoord2fARB(GL_TEXTURE0, 0.0, 1.0); qglMultiTexCoord2fARB(GL_TEXTURE1, 0.0, 1.0); qglVertex2f(0.0  , 0.0   );
	  qglMultiTexCoord2fARB(GL_TEXTURE0, 1.0, 1.0); qglMultiTexCoord2fARB(GL_TEXTURE1, 1.0, 1.0); qglVertex2f(width, 0.0   );
	  qglMultiTexCoord2fARB(GL_TEXTURE0, 1.0, 0.0); qglMultiTexCoord2fARB(GL_TEXTURE1, 1.0, 0.0); qglVertex2f(width, height);
	  qglMultiTexCoord2fARB(GL_TEXTURE0, 0.0, 0.0); qglMultiTexCoord2fARB(GL_TEXTURE1, 0.0, 0.0); qglVertex2f(0.0  , height);
	qglEnd();

	qglActiveTextureARB(GL_TEXTURE0);
}

void R_DrawQuad( GLuint tex, int width, int height) {
	qglEnable(GL_TEXTURE_2D);
	if ( glState.currenttextures[0] != tex ) {
		GL_SelectTexture( 0 );
		qglBindTexture(GL_TEXTURE_2D, tex);
		glState.currenttextures[0] = tex;
	}

	qglBegin(GL_QUADS);
	  qglTexCoord2f(0.0, 1.0); qglVertex2f(0.0  , 0.0   );
	  qglTexCoord2f(1.0, 1.0); qglVertex2f(width, 0.0   );
	  qglTexCoord2f(1.0, 0.0); qglVertex2f(width, height);
	  qglTexCoord2f(0.0, 0.0); qglVertex2f(0.0  , height);
	qglEnd();
}

GLuint *R_CreateTexbuffer(	GLuint *store, int width, int height,
							qboolean smooth, GLenum bindSize, GLenum bindType)
{
	GLenum filter = GL_NEAREST;

	if (smooth) {
		filter = GL_LINEAR;
	}

	qglGenTextures( 1, store );
	qglBindTexture( GL_TEXTURE_2D, *store );
	qglTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, filter );
	qglTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, filter );

	qglTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
	qglTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );
	qglTexImage2D( GL_TEXTURE_2D, 0, bindType, width, height, 0,
					bindType, bindSize, NULL );

	return store;
}

GLuint *R_CreateRenderbuffer(	GLuint *store, int width, int height,
								GLenum bindType, qboolean withMultisample)
{
	qglGenRenderbuffersEXT( 1, store );
	qglBindRenderbufferEXT( GL_RENDERBUFFER_EXT, *store );

	if (withMultisample) {
		// Add GL_EXT_framebuffer_multisample support
		qglRenderbufferStorageMultisampleEXT( GL_RENDERBUFFER_EXT, samples, bindType, width, height );
	} else {
		qglRenderbufferStorageEXT( GL_RENDERBUFFER_EXT, bindType, width, height );
	}

	return store;
}

int R_DeleteFBuffer( struct r_fbuffer *buffer) {
	int flags = buffer->modeFlags;

	if (flags & FB_STENCIL) {
			if (flags & FB_ZTEXTURE) {
				qglDeleteFramebuffersEXT(1, &buffer->depth);
				qglDeleteFramebuffersEXT(1, &buffer->stencil);
				qglDeleteTextures(1, &buffer->depth);
			}
			else {
				qglDeleteRenderbuffersEXT(1, &buffer->depth);
				qglDeleteRenderbuffersEXT(1, &buffer->stencil);
			}
	}
	else if (flags & FB_ZBUFFER) {
			if (flags & FB_ZTEXTURE) {
				qglDeleteFramebuffersEXT(1, &buffer->depth);
				qglDeleteTextures(1, &buffer->depth);
			}
			else {
				qglDeleteRenderbuffersEXT(1, &buffer->depth);
			}
	}
	if (flags & FB_BACKBUFFER) {
		qglDeleteFramebuffersEXT(1, &buffer->back);
		qglDeleteTextures(1, &buffer->back);
	}

	qglDeleteFramebuffersEXT(1, &buffer->front);
	qglDeleteTextures(1, &buffer->front);

	qglDeleteFramebuffersEXT(1, &(buffer->fbo));

	return 0;
}

int R_CreateFBuffer( struct r_fbuffer *buffer, int width, int height, int flags)
{
	qboolean filter = qfalse;

	//do some checks
	if ((flags & FB_PACKED) && (flags & FB_SEPARATEZS)) {
		return -1;
	}
	if ((flags & FB_PACKED) && !(flags & FB_STENCIL)) {
		return -2;
	}
	if ((flags & FB_SEPARATEZS) && !(flags & FB_STENCIL)) {
		return -3;
	}

	//store the flags in the struct
	buffer->modeFlags = flags;

	//set the filter state
	if (flags & FB_SMOOTH) {
		filter = qtrue;
	}

	//gen the frame buffer
	qglGenFramebuffersEXT(1, &(buffer->fbo));
	qglBindFramebufferEXT(GL_FRAMEBUFFER_EXT, buffer->fbo);

	if (flags & FB_STENCIL) {
		if (flags & FB_PACKED) {
			if (flags & FB_ZTEXTURE) {
				R_CreateTexbuffer(	&buffer->depth, width, height, qfalse,
									GL_UNSIGNED_INT_24_8_EXT, GL_DEPTH_STENCIL_EXT);

				qglFramebufferTexture2DEXT(	GL_FRAMEBUFFER_EXT, GL_DEPTH_ATTACHMENT_EXT,
											GL_TEXTURE_2D, buffer->depth, 0);

				qglFramebufferTexture2DEXT(	GL_FRAMEBUFFER_EXT, GL_STENCIL_ATTACHMENT_EXT,
											GL_TEXTURE_2D, buffer->stencil, 0);
			}
			else {
				R_CreateRenderbuffer(&buffer->depth, width, height, GL_DEPTH_STENCIL_EXT, flags & FB_MULTISAMPLE);
				qglFramebufferRenderbufferEXT(	GL_FRAMEBUFFER_EXT,
												GL_DEPTH_ATTACHMENT_EXT,
												GL_RENDERBUFFER_EXT,
												buffer->depth);

				qglFramebufferRenderbufferEXT(	GL_FRAMEBUFFER_EXT,
												GL_STENCIL_ATTACHMENT_EXT,
												GL_RENDERBUFFER_EXT,
												buffer->stencil);
			}
		}
		if (flags & FB_SEPARATEZS) {
			if (flags & FB_ZTEXTURE) {
				R_CreateTexbuffer(	&buffer->depth, width, height, qfalse,
									GL_UNSIGNED_INT, GL_DEPTH_COMPONENT);

				qglFramebufferTexture2DEXT(	GL_FRAMEBUFFER_EXT, GL_DEPTH_ATTACHMENT_EXT,
											GL_TEXTURE_2D, buffer->depth, 0);

				R_CreateRenderbuffer(&buffer->stencil, width, height, GL_STENCIL_INDEX8_EXT, flags & FB_MULTISAMPLE);

				qglFramebufferRenderbufferEXT(	GL_FRAMEBUFFER_EXT, GL_STENCIL_ATTACHMENT_EXT,
												GL_RENDERBUFFER_EXT, buffer->stencil);
			}
			else {
				R_CreateRenderbuffer(&buffer->depth, width, height, GL_DEPTH_STENCIL_EXT, flags & FB_MULTISAMPLE);
				qglFramebufferRenderbufferEXT(	GL_FRAMEBUFFER_EXT,
												GL_DEPTH_ATTACHMENT_EXT,
												GL_RENDERBUFFER_EXT,
												buffer->depth);

				R_CreateRenderbuffer(&buffer->stencil, width, height, GL_STENCIL_INDEX8_EXT, flags & FB_MULTISAMPLE);
				qglFramebufferRenderbufferEXT(	GL_FRAMEBUFFER_EXT,
												GL_STENCIL_ATTACHMENT_EXT,
												GL_RENDERBUFFER_EXT,
												buffer->stencil);
			}
		}
	}
	else if (flags & FB_ZBUFFER) {
		if (flags & FB_ZTEXTURE) {
			R_CreateTexbuffer(	&buffer->depth, width, height, qfalse,
								GL_UNSIGNED_INT, GL_DEPTH_COMPONENT);

			qglFramebufferTexture2DEXT(	GL_FRAMEBUFFER_EXT, GL_DEPTH_ATTACHMENT_EXT,
										GL_TEXTURE_2D, buffer->depth, 0);
		}
		else {
			R_CreateRenderbuffer(&buffer->depth, width, height, GL_DEPTH_STENCIL_EXT, flags & FB_MULTISAMPLE);
			qglFramebufferRenderbufferEXT(	GL_FRAMEBUFFER_EXT,
											GL_DEPTH_ATTACHMENT_EXT,
											GL_RENDERBUFFER_EXT,
											buffer->depth);
		}
	}

	if (flags & FB_BACKBUFFER) {
		R_CreateTexbuffer(	&buffer->back, width, height, filter,
							GL_UNSIGNED_BYTE, GL_RGBA);

		//we link to the second colour attachment
		qglFramebufferTexture2DEXT(	GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT1_EXT,
									GL_TEXTURE_2D, buffer->back, 0);
	}

	//create the main colour buffer
	R_CreateTexbuffer(	&buffer->front, width, height, filter,
						GL_UNSIGNED_BYTE, GL_RGBA);

	qglFramebufferTexture2DEXT(	GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT,
								GL_TEXTURE_2D, buffer->front, 0);

	return R_CheckFramebufferStatus("FBO creation") ? 0 : -1 ;
}

qboolean R_TestFbuffer_SeparateDS( void ) {
	int width = 512;
	int height = 512;
	qboolean status;
	GLuint mbuffer;

	//try and get a perfect separate stencil/zbuffer
	//this does not work on any hardware i know, but might in the future.
	GLuint buffers[3]; //three buffers GL_UNSIGNED_INT_24_8_EXT
	qglGenFramebuffersEXT(3, buffers);
	qglBindFramebufferEXT(GL_FRAMEBUFFER_EXT, buffers[0]);

	//create the depth buffer
	R_CreateRenderbuffer( &buffers[1], width, height, GL_DEPTH_COMPONENT, useMultisample);

	//stencil buffer as a render buffer
	R_CreateRenderbuffer( &buffers[2], width, height, GL_STENCIL_INDEX8_EXT, useMultisample);
	//attach the textures/renderbuffers to the framebuffer
	qglFramebufferRenderbufferEXT(	GL_FRAMEBUFFER_EXT, GL_DEPTH_ATTACHMENT_EXT,
									GL_RENDERBUFFER_EXT, buffers[1]);
	qglFramebufferRenderbufferEXT(	GL_FRAMEBUFFER_EXT, GL_STENCIL_ATTACHMENT_EXT,
									GL_RENDERBUFFER_EXT, buffers[2]);

	//create our framebuffer context

	R_CreateTexbuffer(	&buffers[0], width, height, qfalse,
						GL_UNSIGNED_BYTE, GL_RGBA);

	//handle multisampling case
	if (useMultisample) {
		qglGenFramebuffersEXT(1, &mbuffer);
		qglBindFramebufferEXT(GL_FRAMEBUFFER_EXT, mbuffer);
	}

	//shall we link our texture to the frame buffer? yes!
	qglFramebufferTexture2DEXT(	GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT,
								GL_TEXTURE_2D, buffers[0], 0);

	status = R_CheckFramebufferStatus("separate depth stencil");

	qglDeleteFramebuffersEXT(1, &buffers[0]);
	qglDeleteTextures(1,  &buffers[0]);
	qglDeleteRenderbuffersEXT(1, &buffers[1]);
	qglDeleteRenderbuffersEXT(1, &buffers[2]);
	if (useMultisample)
		qglDeleteFramebuffersEXT(1, &mbuffer);

	return status;
}

qboolean R_TestFbuffer_PackedDS( void ) {
	int width = 512;
	int height = 512;
	qboolean status;
	GLuint buffers[2];
	GLuint mbuffer;

	//try and get a packed separate stencil/zbuffer
	qglGenFramebuffersEXT(2, buffers);
	qglBindFramebufferEXT(GL_FRAMEBUFFER_EXT, buffers[0]);

	//create the buffer
	R_CreateRenderbuffer( &buffers[1], width, height, GL_DEPTH_STENCIL_EXT, useMultisample);
	//attach the textures/renderbuffers to the framebuffer
	qglFramebufferRenderbufferEXT(	GL_FRAMEBUFFER_EXT, GL_DEPTH_ATTACHMENT_EXT,
									GL_RENDERBUFFER_EXT, buffers[1]);
	qglFramebufferRenderbufferEXT(	GL_FRAMEBUFFER_EXT, GL_STENCIL_ATTACHMENT_EXT,
									GL_RENDERBUFFER_EXT, buffers[1]);

	qglBindRenderbufferEXT(GL_RENDERBUFFER_EXT, buffers[1]);
	//create our framebuffer context

	R_CreateTexbuffer(	&buffers[0], width, height, qfalse,
						GL_UNSIGNED_BYTE, GL_RGBA);

	//handle multisampling case
	if (useMultisample) {
		qglGenFramebuffersEXT(1, &mbuffer);
		qglBindFramebufferEXT(GL_FRAMEBUFFER_EXT, mbuffer);
	}

	//shall we link our texture to the frame buffer? yes!
	qglFramebufferTexture2DEXT(	GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT,
								GL_TEXTURE_2D, buffers[0], 0);

	status = R_CheckFramebufferStatus("packed_depth_stencil");

	qglDeleteFramebuffersEXT(1, &buffers[0]);
	qglDeleteTextures(1,  &buffers[0]);
	qglDeleteRenderbuffersEXT(1, &buffers[1]);
	if (useMultisample)
		qglDeleteFramebuffersEXT(1, &mbuffer);

	return status;
}

qboolean R_TestFbuffer_texD( void ) {
	int width = 512;
	int height = 512;
	qboolean status;
	GLuint buffers[2];
	GLuint mbuffer;

	//try and get a packed separate stencil/zbuffer
	qglGenFramebuffersEXT(2, buffers);
	qglBindFramebufferEXT(GL_FRAMEBUFFER_EXT, buffers[0]);

	//create the buffer

	R_CreateTexbuffer(	&buffers[0], width, height, qfalse,
						GL_UNSIGNED_INT, GL_DEPTH_COMPONENT);

	qglFramebufferTexture2DEXT(	GL_FRAMEBUFFER_EXT, GL_DEPTH_ATTACHMENT_EXT,
								GL_TEXTURE_2D, buffers[1], 0);

	//create our framebuffer context

	R_CreateTexbuffer(	&buffers[0], width, height, qfalse,
						GL_UNSIGNED_BYTE, GL_RGBA);

	//handle multisampling case
	if (useMultisample) {
		qglGenFramebuffersEXT(1, &mbuffer);
		qglBindFramebufferEXT(GL_FRAMEBUFFER_EXT, mbuffer);
	}

	//shall we link our texture to the frame buffer? yes!
	qglFramebufferTexture2DEXT(	GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT,
								GL_TEXTURE_2D, buffers[0], 0);

	status = R_CheckFramebufferStatus("depth texture");

	qglDeleteFramebuffersEXT(1, &buffers[0]);
	qglDeleteTextures(1,  &buffers[0]);
	qglDeleteFramebuffersEXT(1, &buffers[1]);
	qglDeleteTextures(1,  &buffers[1]);
	if (useMultisample)
		qglDeleteFramebuffersEXT(1, &mbuffer);

	return status;
}


// for shader debugging
void printShaderInfoLog(GLuint obj)
{
	int infologLength = 0;
	int charsWritten  = 0;
	char *infoLog;

	qglGetShaderiv(obj, GL_INFO_LOG_LENGTH,&infologLength);

	if (infologLength > 1)
	{
		infoLog = (char *)ri.Malloc(infologLength);
		qglGetShaderInfoLog(obj, infologLength, &charsWritten, infoLog);
		ri.Printf( PRINT_ALL, "----- Shader InfoLog -----\n" );;
		ri.Printf( PRINT_ALL, "%s\n", infoLog );
		ri.Free(infoLog);
	}
}

void printProgramInfoLog(GLuint obj)
{
	int infologLength = 0;
	int charsWritten  = 0;
	char *infoLog;

	qglGetProgramiv(obj, GL_INFO_LOG_LENGTH,&infologLength);

	if (infologLength > 1)
	{
		infoLog = (char *)ri.Malloc(infologLength);
		qglGetProgramInfoLog(obj, infologLength, &charsWritten, infoLog);
		ri.Printf( PRINT_ALL, "----- Program InfoLog -----\n" );;
		ri.Printf( PRINT_ALL, "%s\n", infoLog );
		ri.Free(infoLog);
	}
}

void R_Build_glsl(struct glslobj *obj) {
	GLuint vert_shader, frag_shader, program;

	vert_shader = qglCreateShader(GL_VERTEX_SHADER);
	frag_shader = qglCreateShader(GL_FRAGMENT_SHADER);

	qglShaderSource(vert_shader, obj->vert_numSources, obj->vertex_glsl, NULL);
	qglShaderSource(frag_shader, obj->frag_numSources, obj->fragment_glsl, NULL);

	printShaderInfoLog(vert_shader);
	printShaderInfoLog(frag_shader);

	qglCompileShader(vert_shader);
	qglCompileShader(frag_shader);

	program = qglCreateProgram();
	qglAttachShader(program, vert_shader);
	qglAttachShader(program, frag_shader);
	qglLinkProgram(program);

	printProgramInfoLog(program);

	obj->vertex = vert_shader;
	obj->fragment = frag_shader;
	obj->program = program;

}

void R_Delete_glsl(struct glslobj *obj) {
	qglDeleteProgram(obj->program);
	qglDeleteShader(obj->vertex);
	qglDeleteShader(obj->fragment);
}

struct glslobj glslBlur;

void R_FrameBuffer_BlurInit( void ) {
	//inits our blur code;
	if (!needBlur) {
		return;
	}
	// Force blur_size to be at least 2
	blur_size = ( r_ext_framebuffer_blur_size->integer < 2 ) ? 2 : r_ext_framebuffer_blur_size->integer;

	R_CreateFBuffer( &gaussblurBuffer, blur_size, blur_size, FB_BACKBUFFER | FB_SMOOTH);
	R_FrameBufferUnBind();

	//create our glsl shader
	glslBlur.vert_numSources = 1;
	glslBlur.frag_numSources = 2;

	glslBlur.vertex_glsl = ri.Malloc(sizeof(char *) * glslBlur.vert_numSources);
	glslBlur.vertex_glsl[0] = R_GLSLGetProgByName("glslBase_vert");

	glslBlur.fragment_glsl = ri.Malloc(sizeof(char *) * glslBlur.frag_numSources);
	switch (r_ext_framebuffer_blur_samples->integer) {
		case (5):
			glslBlur.fragment_glsl[0] = R_GLSLGetProgByName("glslGauss5");
			break;
		case (7):
			glslBlur.fragment_glsl[0] = R_GLSLGetProgByName("glslGauss7");
			break;
		case (9):
			glslBlur.fragment_glsl[0] = R_GLSLGetProgByName("glslGauss9");
			break;
		default:
			glslBlur.fragment_glsl[0] = R_GLSLGetProgByName("glslGauss9");
			break;
	}
	glslBlur.fragment_glsl[1] = R_GLSLGetProgByName("glslBlurMain");

	R_Build_glsl(&glslBlur);
}

void R_FrameBuffer_BlurDraw( GLuint *srcTex ) {
	int fb_size ;
	GLuint program, loc;

	if (!needBlur) {
		return;
	}
	// Force fb_size to be set like blur_size
	fb_size = blur_size;

	// first we draw the framebuffer into the blur buffer before any fragment
	// programs are used is quicker, the rational behind this is that we want
	// as many texels to fit inside the texture cache as possible for the
	// gaussian sampling
	qglBindFramebufferEXT(GL_FRAMEBUFFER_EXT, gaussblurBuffer.fbo);
	qglDrawBuffer(GL_COLOR_ATTACHMENT0_EXT);
	R_SetGL2DSize(fb_size, fb_size);
	qglUseProgram(0);
	R_DrawQuad(	*srcTex, fb_size, fb_size);

	//now we do the first gaussian pass

	qglDrawBuffer(GL_COLOR_ATTACHMENT1_EXT);
	R_SetGL2DSize(fb_size, fb_size);

	program = glslBlur.program;
	qglUseProgram(0);
	qglUseProgram(program);

	//find and set the samplers
	//set the texture number... silly this really. oh well thats glsl
	loc = qglGetUniformLocation(program, "srcSampler");
	qglUniform1i(loc, 0);
	loc = qglGetUniformLocation(program, "blurSize");
	qglUniform2f(loc, r_ext_framebuffer_blur_amount->value / 100.0, 0.0);

	R_DrawQuad(gaussblurBuffer.front, fb_size, fb_size);

	//we do the second pass of the blur here
	qglDrawBuffer(GL_COLOR_ATTACHMENT0_EXT);
	loc = qglGetUniformLocation(program, "blurSize");
	qglUniform2f(loc, 0.0, r_ext_framebuffer_blur_amount->value / 100.0);

	R_SetGL2DSize(fb_size, fb_size);
	R_DrawQuad(gaussblurBuffer.back, fb_size, fb_size);
	qglUseProgram(0);

	//finally the FRONT buffer in the fbo is given the blurred image
}

void R_FrameBuffer_BlurDelete( void ) {

	R_Delete_glsl(&glslBlur);
	ri.Free(glslBlur.vertex_glsl);
	ri.Free(glslBlur.fragment_glsl);

	R_DeleteFBuffer(&gaussblurBuffer);
}

struct glslobj glslRoto;

void R_FrameBuffer_RotoInit( struct r_fbuffer *src ) {

	// check to see if we need to create a framebuffer for this (only if there
	// is a shader running after this)

	//create our glsl shader
	glslRoto.vert_numSources = 1;
	glslRoto.frag_numSources = 3;

	glslRoto.vertex_glsl = ri.Malloc(sizeof(char *) * glslRoto.vert_numSources);
	glslRoto.vertex_glsl[0] = R_GLSLGetProgByName("glslBase_vert");

	glslRoto.fragment_glsl = ri.Malloc(sizeof(char *) * glslRoto.frag_numSources);
	glslRoto.fragment_glsl[0] = R_GLSLGetProgByName("glslToonColour");

	if (useRotoZEdgeEffect) {
		glslRoto.fragment_glsl[1] = R_GLSLGetProgByName("glslSobelZ");
		glslRoto.fragment_glsl[2] = R_GLSLGetProgByName("glslRotoscopeZ");
	} else {
		glslRoto.fragment_glsl[1] = R_GLSLGetProgByName("glslSobel");
		glslRoto.fragment_glsl[2] = R_GLSLGetProgByName("glslRotoscope");
	}

	R_Build_glsl(&glslRoto);
}

void R_FrameBuffer_RotoDraw( struct r_fbuffer *src, GLuint *srcTex ) {
	GLuint program, loc;

	program = glslRoto.program;
	qglUseProgram(0);
	qglUseProgram(program);

	R_SetGL2DSize(glConfig.vidWidth, glConfig.vidHeight);

	//find and set the samplers
	loc = qglGetUniformLocation(program, "srcSampler");
	qglUniform1i(loc, 0);
	loc = qglGetUniformLocation(program, "depthSampler");
	qglUniform1i(loc, 1);
	loc = qglGetUniformLocation(program, "texelSize");
	qglUniform2f(loc, 1.0 / glConfig.vidWidth, 1.0 / glConfig.vidHeight);

	if (useRotoZEdgeEffect) {
		R_DrawQuadMT(	*srcTex, src->depth,
						glConfig.vidWidth, glConfig.vidHeight);
	} else {
		R_DrawQuad(	*srcTex, glConfig.vidWidth, glConfig.vidHeight);
	}
	qglUseProgram(0);
}

void R_FrameBuffer_RotoDelete( void ) {
	R_Delete_glsl(&glslRoto);
	ri.Free(glslRoto.vertex_glsl);
	ri.Free(glslRoto.fragment_glsl);
}

struct glslobj glslBloom;

void R_FrameBuffer_BloomInit( void ) {
	//we need blur for this
	needBlur = qtrue;
	//create our glsl shader
	glslBloom.vert_numSources = 1;
	glslBloom.frag_numSources = 1;
	glslBloom.vertex_glsl = ri.Malloc(sizeof(char *) * glslBloom.frag_numSources);
	glslBloom.vertex_glsl[0] = R_GLSLGetProgByName("glslBase_vert");
	glslBloom.fragment_glsl = ri.Malloc(sizeof(char *) * glslBloom.frag_numSources);
	glslBloom.fragment_glsl[0] = R_GLSLGetProgByName("glslSigScreen");

	R_Build_glsl(&glslBloom);
}

void R_FrameBuffer_BloomDraw( GLuint *srcTex ) {
	GLuint program, loc;

	program = glslBloom.program;
	qglUseProgram(0);
	qglUseProgram(program);

	R_SetGL2DSize(glConfig.vidWidth, glConfig.vidHeight);

	//find and set the samplers
	loc = qglGetUniformLocation(program, "srcSampler");
	qglUniform1i(loc, 0);
	loc = qglGetUniformLocation(program, "blurSampler");
	qglUniform1i(loc, 1);
	loc = qglGetUniformLocation(program, "brightness");
	qglUniform1f(loc, r_ext_framebuffer_bloom_brightness->value);
	loc = qglGetUniformLocation(program, "sharpness");
	qglUniform1f(loc, r_ext_framebuffer_bloom_sharpness->value);

	R_DrawQuadMT(	*srcTex, gaussblurBuffer.front,
					glConfig.vidWidth, glConfig.vidHeight);
	qglUseProgram(0);
	//quick test to just see the blur
	//R_DrawQuad(*gaussblurBuffer.front, glConfig.vidWidth, glConfig.vidHeight);
}

void R_FrameBuffer_BloomDelete( void ) {
	R_Delete_glsl(&glslBloom);
	ri.Free(glslBloom.vertex_glsl);
	ri.Free(glslBloom.fragment_glsl);
}

static void R_FrameBuffer_Draw( void ) {
	//draws the framebuffer to the screen, pretty simple really.
	R_SetGL2DSize(glConfig.vidWidth, glConfig.vidHeight);
	R_DrawQuad(screenBuffer.front, glConfig.vidWidth, glConfig.vidHeight);
}

void R_FrameBuffer_Init( void ) {
	GLint maxbuffers;
	int screenbuff_flags = 0x00;
	int status;

	// Framebuffer will only be used if safely initialized
	useFrameBuffer = qfalse;
	useBloomEffect = qfalse;
	useRotoscopeEffect = qfalse;
	useRotoZEdgeEffect = qfalse;
	useMultisample = qfalse;

	needBlur = qfalse;
	if (!framebufferSupported || !glslSupported) {
		ri.Printf( PRINT_WARNING, "WARNING: Framebuffer rendering path disabled\n");
		return;
	}

	if (r_ext_framebuffer->integer != 1) {
		return;
	}

	ri.Printf( PRINT_ALL, "----- Enabling FrameBuffer Path -----\n" );

	R_CheckFramebufferStatus("FBO current status");

	// Framebuffer multisampling support
	if (multisampleSupported) {
		qglGetIntegerv(GL_MAX_SAMPLES_EXT, &samples);
		ri.Printf(PRINT_ALL,"multisamples: supported with max %ix\n", samples );

		if (r_ext_framebuffer_multisample->integer) {
			useMultisample = qtrue;
			if (r_ext_framebuffer_multisample->integer<samples)
				samples = r_ext_framebuffer_multisample->integer;
			ri.Printf(PRINT_ALL,"multisamples: enabled with %ix\n", samples );
		} else {
			useMultisample = qfalse;
			ri.Printf(PRINT_ALL,"multisamples: disabled\n");
		}
	}

	//lets see what works and what doesn't
	packedDepthStencilSupported = qfalse;
	depthTextureSupported = qfalse;
	separateDepthStencilSupported = qfalse;

	if (R_TestFbuffer_PackedDS()) {
		packedDepthStencilSupported = qtrue;
	} else {
		ri.Printf( PRINT_WARNING, "WARNING: packed_depth_stencil failed\n");
	}
	if (R_TestFbuffer_SeparateDS()) {
		separateDepthStencilSupported = qtrue;
	} else {
		ri.Printf( PRINT_WARNING, "WARNING: separate depth stencil not supported\n");
	}
	if (R_TestFbuffer_texD()) {
		depthTextureSupported = qtrue;
	} else {
		ri.Printf( PRINT_WARNING, "WARNING: depth texture failed\n");
	}

#ifdef DISABLE_FBO_MULTISAMPLE_SUPPORT
	if (useMultisample) {
		useMultisample = qfalse;
		ri.Printf(PRINT_WARNING,"multisamples: disabled at compile time\n");
	}
#endif

	qglGetIntegerv(GL_MAX_COLOR_ATTACHMENTS_EXT, &maxbuffers);
	if (maxbuffers < 2) {
		framebufferSupported = qfalse;
		ri.Cvar_Set("r_ext_framebuffer","0");
		ri.Printf( PRINT_WARNING, "ERROR: Framebuffer rendering path disabled: Not enough color buffers available\n");
		ri.Cmd_ExecuteText(EXEC_APPEND, "vid_restart"); // Reset the video without FBO
		return;
	}

	//set our main screen flags
	if ((glConfig.stencilBits > 0)) {
		if ( packedDepthStencilSupported ) {
			screenbuff_flags |= FB_PACKED | FB_STENCIL;
		}
		else if ( separateDepthStencilSupported ) {
			screenbuff_flags |= FB_SEPARATEZS | FB_STENCIL;
		}
	}

	if ((depthTextureSupported) && (r_ext_framebuffer_rotoscope_zedge->integer)) {
		screenbuff_flags |= FB_ZTEXTURE;
		useRotoZEdgeEffect = qtrue;
	}
	screenbuff_flags |= FB_BACKBUFFER;

	screenbuff_flags |= FB_ZBUFFER;

	if (useMultisample)
		screenbuff_flags |= FB_MULTISAMPLE;

	//create our main frame buffer
	status = R_CreateFBuffer(	&screenBuffer, glConfig.vidWidth,
								glConfig.vidHeight, screenbuff_flags);

	if (status) {
		// if the main fbuffer failed then we should disable framebuffer
		// rendering
		framebufferSupported = qfalse;
		ri.Cvar_Set("r_ext_framebuffer","0");
		ri.Printf( PRINT_WARNING, "ERROR: Framebuffer rendering path disabled: Framebuffer creation failed\n");
		ri.Cmd_ExecuteText(EXEC_APPEND, "vid_restart"); // Reset the video without FBO
		return;
	}

	// Load GLSL programs
	R_GLSLProgs_Init();

	//init our effects
	if (r_ext_framebuffer_rotoscope->integer == 1) {
		R_FrameBuffer_RotoInit(&screenBuffer);
		useRotoscopeEffect = qtrue;
	}

	if (r_ext_framebuffer_bloom->integer == 1) {
		R_FrameBuffer_BloomInit();
		useBloomEffect = qtrue;
	}
	//we don't need an if here, if any effects before need a blur then this
	//auto detects that its needed
	R_FrameBuffer_BlurInit();

	// Now framebuffer is initialized, we can safely use it
	useFrameBuffer = qtrue;
}

void R_FrameBuffer_EndFrame( void ) {
	qboolean screenDrawDone = qfalse;
	GLuint *srcBuffer ;

	if (!useFrameBuffer)
		return;

	if (rendered) {
		return;
	}

	//don't flip if drawing to front buffer
	if ( Q_stricmp( r_drawBuffer->string, "GL_FRONT" ) == 0 ) {
		return;
	}

	GL_State( GLS_DEPTHTEST_DISABLE );
	GL_State( GLS_SRCBLEND_ONE | GLS_DSTBLEND_ZERO );

	qglColor4f( 1, 1, 1, 1 );

	srcBuffer = &screenBuffer.front;

	if (useRotoscopeEffect) {
		if (useBloomEffect) {
			//we need to draw to the back buffer
			qglDrawBuffer(GL_COLOR_ATTACHMENT1_EXT);
			R_FrameBuffer_RotoDraw(&screenBuffer, srcBuffer);
			srcBuffer = &screenBuffer.back;
			qglDrawBuffer(GL_COLOR_ATTACHMENT0_EXT);
		}
		else {
			R_FrameBufferUnBind();
			R_FrameBuffer_RotoDraw(&screenBuffer, srcBuffer);
			screenDrawDone = qtrue;
		}
	}
	//everything before this point does not need blurred surfaces (or can do
	//with the last frames burred surface)
	//call the blur code, it auto detects weather its needed :)
	R_FrameBuffer_BlurDraw(srcBuffer);

	if (useBloomEffect) {
		R_FrameBufferUnBind();
		R_FrameBuffer_BloomDraw(srcBuffer);
		screenDrawDone = qtrue;
	}

	if (!screenDrawDone) {
		R_FrameBufferUnBind();
		R_FrameBuffer_Draw();
	}

	//Any other draw will be rendered other that view
	rendered = qtrue;
}

void R_FrameBuffer_Shutdown( void ) {
	//cleanup
	if (!useFrameBuffer)
		return;

	qglBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);

	if (useBloomEffect) {
		R_FrameBuffer_BloomDelete();
	}
	if (needBlur) {
		R_FrameBuffer_BlurDelete();
	}
	if (useRotoscopeEffect) {
		R_FrameBuffer_RotoDelete();
	}

	//unload GLSL Programs
	R_GLSLProgs_Delete();

	//delete the main screen buffer
	R_DeleteFBuffer(&screenBuffer);
}

/*
=============
RB_DrawFrameBuffer
Replacement for RB_DrawBuffer when useFrameBuffer is set
=============
*/
const void	*RB_DrawFrameBuffer( const void *data ) {
	const drawBufferCommand_t	*cmd;

	cmd = (const drawBufferCommand_t *)data;

	// We re-bind our framebuffer so everything gets rendered into it.
	R_FrameBufferBind();

	qglDrawBuffer(GL_COLOR_ATTACHMENT0_EXT);

	// Reset the rendered flag
	rendered = qfalse;

	// clear screen for debugging
	if ( r_clear->integer ) {
		qglClearColor( 1, 0, 0.5, 1 );
		qglClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
	}

	return (const void *)(cmd + 1);
}
