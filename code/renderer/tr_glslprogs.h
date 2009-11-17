/*
 *      tr_glslprogs.h
 *
 *      Copyright 2007 Gord Allott <gordallott@gmail.com>
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


extern const char *glslBase_vert;
extern const char *glslGauss9;
extern const char *glslGauss7;
extern const char *glslGauss5;
extern const char *glslBlurMain;
extern const char *glslSigScreen;
extern const char *glslToonColour;
extern const char *glslSobel;
extern const char *glslRotoscope;
extern const char *glslRotoscopeZ;
extern const char *glslSobelZ;
#endif //TR_GLSLPROGS_H
