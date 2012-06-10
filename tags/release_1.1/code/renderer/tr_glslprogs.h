/*
 *      tr_glslprogs.h
 *
 *      Copyright 2007 Gord Allott <gordallott@gmail.com>
 *      Copyright (C) 2005-2010 Smokin' Guns
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

#ifndef TR_GLSLPROGS_H
#define TR_GLSLPROGS_H

//basicly the idea here is to just store fragments of glsl source files so that
//we can reuse the code in different programs. the problem with glsl is that
//there is no defined way of 'including' code from other places. so we will
//just have to live with this for now

typedef struct {
	const char *name;
	const char *program;
	const char *default_program;
} gslsProg_t ;

void R_GLSLProgs_Init( void );
void R_GLSLProgs_Delete( void );
const char *R_GLSLGetProgByName( const char *name );

#endif //TR_GLSLPROGS_H
