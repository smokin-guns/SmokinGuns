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

#ifndef Q_CHECK_H
#define Q_CHECK_H

typedef unsigned long int chk_ref;

static ID_INLINE void Com_CheckCvar(cvar_t *cvar) {
	Com_Printf(PRODUCT_NAME " online checks on\n");
	cvar = Cvar_Get("cl_check2", Com_MD5Text(cvar->string,strlen(cvar->string), PRODUCT_NAME,sizeof(PRODUCT_NAME)), CVAR_ROM);
	if(com_developer->integer)
		Com_Printf("check2: %s\n", cvar->string );
	cvar = Cvar_Get("cl_check3", va("%i",MSG_HashKey(cvar->string,strlen(cvar->string))), CVAR_ROM|CVAR_USERINFO);
	if(com_developer->integer)
		Com_Printf("check3: %s\n", cvar->string );
}

static ID_INLINE void Com_CheckReRef(refexport_t *re) {
	if ( re != NULL ) {
		static char *check;
		static cvar_t *chk_cvar[7];
		Com_Printf(PRODUCT_NAME " checks on\n");
		chk_cvar[0] = Cvar_Get("sa_engine_reref", va("%08lX",(chk_ref)re), CVAR_ROM );
		chk_cvar[1] = Cvar_Get("sa_engine_reref1", va("%08lX",(chk_ref)re->ClearScene), CVAR_ROM );
		chk_cvar[2] = Cvar_Get("sa_engine_reref2", va("%08lX",(chk_ref)re->RenderScene), CVAR_ROM );
		chk_cvar[3] = Cvar_Get("sa_engine_reref3", va("%08lX",(chk_ref)re->RegisterShaderNoMip), CVAR_ROM );
		chk_cvar[4] = Cvar_Get("sa_engine_reref4", va("%08lX",(chk_ref)re->RegisterShader), CVAR_ROM );
		chk_cvar[5] = Cvar_Get("sa_engine_reref5", va("%08lX",(chk_ref)re->RemapShader), CVAR_ROM );
		check = va("%s%s%s%s%s%s", chk_cvar[0]->string, chk_cvar[1]->string,
			chk_cvar[2]->string, chk_cvar[3]->string,
			chk_cvar[4]->string, chk_cvar[5]->string);
		if(com_developer->integer)
			Com_Printf("check : %s\n", check );
		chk_cvar[6] = Cvar_Get("cl_check1", Com_MD5Text(check,strlen(check), PRODUCT_NAME,sizeof(PRODUCT_NAME)), CVAR_ROM|CVAR_USERINFO);
		if(com_developer->integer)
			Com_Printf("check1: %s\n", chk_cvar[6]->string );
	} else {
		Com_Printf(PRODUCT_NAME " checks off\n");
		Cvar_Set ("cl_check1", "off");
	}
}

#endif // Q_CHECK_H

