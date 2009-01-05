/*
===========================================================================
Copyright (C) 1999-2005 Id Software, Inc.
Copyright (C) 2005-2008 Smokin' Guns

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
#include <signal.h>
#ifdef DEDICATED
#include <errno.h>
#endif

#include "../game/q_shared.h"
#include "../qcommon/qcommon.h"
#ifndef DEDICATED
#include "../renderer/tr_local.h"
#endif

static qboolean signalcaught = qfalse;;

void Sys_Exit(int); // bk010104 - abstraction

static void signal_handler(int sig) // bk010104 - replace this... (NOTE TTimo huh?)
{
  if (signalcaught)
  {
    printf("DOUBLE SIGNAL FAULT: Received signal %d, exiting...\n", sig);
    Sys_Exit(1); // bk010104 - abstraction
  }

  signalcaught = qtrue;
  
// Original code for handling SIGHUP and SIGINT by hika AT bsdmon DOT com
#ifdef DEDICATED
  // Only for dedicated server
  switch(sig)
  {
    case SIGHUP:
      printf("Received signal %d, reloading...\n", sig);
      
      // Add possible startup commands, given on command line,
      // in the command text buffer 
      if ( Com_AddStartupCommands() )
      {
        // Then, execute them
        Cbuf_Execute();
      }
      // Signal has been caught
      signalcaught = qfalse;
      return;
      break;
  }
#endif
  
  printf("Received signal %d, exiting...\n", sig);
#ifndef DEDICATED
  GLimp_Shutdown(); // bk010104 - shouldn't this be CL_Shutdown
#endif
  Sys_Exit(0); // bk010104 - abstraction NOTE TTimo send a 0 to avoid DOUBLE SIGNAL FAULT
}

void InitSig(void)
{
  signal(SIGHUP, signal_handler);
#ifdef DEDICATED
  signal(SIGINT, signal_handler); // Typically for catching CTRL+C signal interrupt
#endif
  signal(SIGQUIT, signal_handler);
  signal(SIGILL, signal_handler);
  signal(SIGTRAP, signal_handler);
  signal(SIGIOT, signal_handler);
  signal(SIGBUS, signal_handler);
  signal(SIGFPE, signal_handler);
  signal(SIGSEGV, signal_handler);
  signal(SIGTERM, signal_handler);
}
