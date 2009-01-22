/*
===========================================================================
Copyright (C) 1999-2005 Id Software, Inc.
Copyright (C) 2000-2003 Iron Claw Interactive
Copyright (C) 2005-2009 Smokin' Guns

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
// cg_rankings.c -- client global rankings system

#include "cg_local.h"

/*
================
CG_RankRunFrame
================
*/

void CG_RankRunFrame(void)
{
	grank_status_t	status;

	if (! cgs.localServer)
		trap_CL_RankPoll();

	status = trap_CL_RankUserStatus();

	if( cgs.client_status != status )
	{
		// GRank status changed

		// inform UI of current status
		trap_Cvar_Set("client_status", va("%i",(int)(status)));

		// show rankings status dialog if error
		trap_CL_RankShowStatus((int)status);

		cgs.client_status = status;
	}
	return;
}
