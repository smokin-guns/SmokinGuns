/*
===========================================================================
Copyright (C) 1999-2005 Id Software, Inc.
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
#include "tr_common.h"


qboolean ( * qwglSwapIntervalEXT)( int interval );
void ( * qglMultiTexCoord2fARB )( GLenum texture, float s, float t );
void ( * qglActiveTextureARB )( GLenum texture );
void ( * qglClientActiveTextureARB )( GLenum texture );


void ( * qglLockArraysEXT)( int, int);
void ( * qglUnlockArraysEXT) ( void );

#ifdef FRAMEBUFFER_AND_GLSL_SUPPORT
//added framebuffer extensions
 void ( * glGenFramebuffersEXT )(GLsizei, GLuint *);
 void ( * glBindFramebufferEXT )(GLenum, GLuint);
 void ( * glGenRenderbuffersEXT )(GLsizei, GLuint *);
 void ( * glBindRenderbufferEXT )(GLenum, GLuint);
 void ( * glRenderbufferStorageEXT )(GLenum, GLenum, GLsizei, GLsizei);
 void ( * glRenderbufferStorageMultisampleEXT )(GLenum, GLsizei, GLenum, GLsizei, GLsizei);
 void ( * glFramebufferRenderbufferEXT )(GLenum, GLenum, GLenum, GLuint);
 void ( * glFramebufferTexture2DEXT )(GLenum, GLenum, GLenum, GLuint, GLint);
 GLenum ( * glCheckFramebufferStatusEXT )(GLenum);
 void ( * glDeleteFramebuffersEXT )(GLsizei, const GLuint *);
 void ( * glDeleteRenderbuffersEXT )(GLsizei, const GLuint *);

//added fragment/vertex program extensions
 void ( * glAttachShader) (GLuint, GLuint);
 void ( * glBindAttribLocation) (GLuint, GLuint, const GLchar *);
 void ( * glCompileShader) (GLuint);
GLuint ( * glCreateProgram) (void);
GLuint ( * glCreateShader) (GLenum);
void ( * glDeleteProgram) (GLuint);
void ( * glDeleteShader) (GLuint);
void ( * glShaderSource) (GLuint, GLsizei, const GLchar* *, const GLint *);
void ( * glLinkProgram) (GLuint);
void ( * glUseProgram) (GLuint);
GLint ( * glGetUniformLocation) (GLuint, const GLchar *);
void ( * glUniform1f) (GLint, GLfloat);
void ( * glUniform2f) (GLint, GLfloat, GLfloat);
void ( * glUniform1i) (GLint, GLint);
void ( * glGetProgramiv) (GLuint, GLenum, GLint *);
void ( * glGetProgramInfoLog) (GLuint, GLsizei, GLsizei *, GLchar *);
void ( * glGetShaderiv) (GLuint, GLenum, GLint *);
void ( * glGetShaderInfoLog) (GLuint, GLsizei, GLsizei *, GLchar *);
#endif

void		GLimp_EndFrame( void ) {
}

int 		GLimp_Init( void )
{
}

void		GLimp_Shutdown( void ) {
}

void		GLimp_EnableLogging( qboolean enable ) {
}

void GLimp_LogComment( char *comment ) {
}

qboolean QGL_Init( const char *dllname ) {
	return qtrue;
}

void		QGL_Shutdown( void ) {
}
