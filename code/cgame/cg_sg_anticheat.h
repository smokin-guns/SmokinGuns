/*
===========================================================================
Copyright (C) 2009-2010 Smokin' Guns

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

/*
 * Tequila comment:
 * This file is just a facility to introduce few hidden functions we don't
 * need/want to release as opensource to help fighting cheater if needed.
 */


#if 0
#define SG_CvarMagic(x) Com_Printf("No " PRODUCT_NAME " magic on %s defined\n",(x)->name)
#define SG_CheckRef(x,y) Com_Printf(PRODUCT_NAME " checks off\n")
#else
static ID_INLINE void SG_CvarMagic(cvar_t *cvar) {
	Cvar_Get ("sa_engine_in_use", "1", CVAR_USERINFO|CVAR_ROM );
	Com_Printf(PRODUCT_NAME " magic on %s applied\n", cvar->name);
}

#define CHECK_PREFIX __VERSION__
static ID_INLINE void SG_CheckRef(refexport_t *re) {
	if ( re != NULL ) {
		char *check = va("%08lX%08lX%08lX%08lX%08lX", (intptr_t)re->ClearScene,
			(intptr_t)re->RenderScene,	(intptr_t)re->RegisterShaderNoMip,
			(intptr_t)re->RegisterShader, (intptr_t)re->RemapShader );
		Com_Printf("check1: %s\n", check );
		Cvar_Get ("sa_engine_check1", Com_MD5Text(check,strlen(check),PRODUCT_NAME,sizeof(PRODUCT_NAME)), CVAR_USERINFO|CVAR_ROM );
		Com_Printf(PRODUCT_NAME " checks on\n");
	}
}
#endif
