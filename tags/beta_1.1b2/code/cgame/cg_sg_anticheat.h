/*
===========================================================================
Copyright (C) 2009 Smokin' Guns

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
#define SG_CvarMagic(x) Com_Printf("No Smokin' Guns magic on %s defined\n",(x)->name)
#define SG_CheckRef(x,y) Com_Printf("Smokin' Guns checks off\n")
#else
static ID_INLINE void SG_CvarMagic(cvar_t *cvar) {
	Cvar_Get ("sa_engine_in_use", "1", CVAR_USERINFO|CVAR_ROM );
	Com_Printf("Smokin' Guns magic on %s applied\n", cvar->name);
}

static ID_INLINE void SG_CheckRef(refexport_t *re) {
	if ( re != NULL )
		Com_Printf("Smokin' Guns checks on\n");
}
#endif
