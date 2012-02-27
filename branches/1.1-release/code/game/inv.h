/*
===========================================================================
Copyright (C) 1999-2005 Id Software, Inc.
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

#ifndef SMOKINGUNS
#error "This file can only be used with " PRODUCT_NAME " sources"
#endif

#define INVENTORY_NONE				0
//armor
#define INVENTORY_ARMOR				1
//weapons
#define INVENTORY_KNIFE				2
#define INVENTORY_REM58				3
#define INVENTORY_SCHOFIELD			4
#define INVENTORY_PEACEMAKER		5
#define INVENTORY_WINCHESTER66		6
#define INVENTORY_LIGHTNING			7
#define INVENTORY_WINCH97			8
#define INVENTORY_REMGAUGE			9
#define INVENTORY_SAWEDOFF			10
#define INVENTORY_GATLING			11
#define INVENTORY_SHARPS			12
#define INVENTORY_DYNAMITE			13
#define	INVENTORY_MOLOTOV			14

//ammo
#define INVENTORY_SHELLS			15
#define INVENTORY_BULLETS			16
#define INVENTORY_CART				17
#define INVENTORY_GATLING_AMMO		18
#define INVENTORY_SHARPS_AMMO		19

//powerups
#define INVENTORY_BELT				20
#define INVENTORY_HEALTH			21
#define INVENTORY_MONEY				22
#define	INVENTORY_SCOPE				23

//enemy stuff
#define ENEMY_HORIZONTAL_DIST		200
#define ENEMY_HEIGHT				201
#define NUM_VISIBLE_ENEMIES			202
#define NUM_VISIBLE_TEAMMATES		203


//item numbers (make sure they are in sync with bg_itemlist in bg_misc.c)
#define MODELINDEX_BOILER			1
#define MODELINDEX_BELT				2
#define MODELINDEX_WINCHESTER66		3
#define MODELINDEX_LIGHTNING		4
#define MODELINDEX_SHARPS			5
#define MODELINDEX_GATLING			6
#define MODELINDEX_REM58			7
#define MODELINDEX_SCHOFIELD		8
#define MODELINDEX_PEACEMAKER		9
#define MODELINDEX_DYNAMITE			10
#define MODELINDEX_KNIFE			11
#define MODELINDEX_REMGAUGE			12
#define MODELINDEX_SAWEDOFF			14
#define MODELINDEX_SHELLS			15
#define MODELINDEX_BULLETS			16
#define MODELINDEX_DYNAMITE_AMMO	17
#define MODELINDEX_KNIVES			18
#define MODELINDEX_CART				19
#define MODELINDEX_GATLING_AMMO		20
#define MODELINDEX_SHARPS_AMMO		21
#define MODELINDEX_MONEY			22
#define MODELINDEX_SCOPE			23

//
#define WEAPONINDEX_KNIFE				1
#define WEAPONINDEX_REM58				2
#define WEAPONINDEX_SCHOFIELD			3
#define WEAPONINDEX_PEACEMAKER			4
#define WEAPONINDEX_WINCHESTER66		5
#define WEAPONINDEX_LIGHTNING			6
#define WEAPONINDEX_SHARPS				7
#define WEAPONINDEX_REMGAUGE			8
#define WEAPONINDEX_SAWEDOFF			9
#define WEAPONINDEX_GATLING				10
#define WEAPONINDEX_DYNAMITE			11
