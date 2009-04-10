@rem make sure we have a safe environment
@set LIBRARY=
@set INCLUDE=
@set PATH=%PATH%;..\..\misc\win32\bin

mkdir ..\..\build
mkdir ..\..\build\asm
mkdir ..\..\build\map
mkdir ..\..\build\vm
cd ..\..\build\asm

@set PATH_GAME=..\..\code\game
@set PATH_CGAME=..\code\cgame
@set PATH_UI=..\code\ui

@set CC=lcc -DQ3_VM -DMISSIONPACK -DSMOKINGUNS -S -Wf-target=bytecode -Wf-g -I%PATH_CGAME% -I%PATH_GAME% -I%PATH_UI% %1
@set CC_CGAME=lcc -DQ3_VM -DVM_CGAME -DMISSIONPACK -S -DSMOKINGUNS -Wf-target=bytecode -Wf-g -I%PATH_CGAME% -I%PATH_GAME% -I%PATH_UI% %1
@set Q3ASM=q3asm

@echo Building game ...

@if not exist g_main.asm %CC% %PATH_GAME%/g_main.c
@if errorlevel 1 goto quit

@if not exist bg_misc.asm %CC% %PATH_GAME%/bg_misc.c
@if errorlevel 1 goto quit

@if not exist bg_lib.asm %CC% %PATH_GAME%/bg_lib.c
@if errorlevel 1 goto quit

@if not exist bg_pmove.asm %CC% %PATH_GAME%/bg_pmove.c
@if errorlevel 1 goto quit

@if not exist bg_slidemove.asm %CC% %PATH_GAME%/bg_slidemove.c
@if errorlevel 1 goto quit

@if not exist q_math.asm %CC% %PATH_GAME%/q_math.c
@if errorlevel 1 goto quit

@if not exist q_shared.asm %CC% %PATH_GAME%/q_shared.c
@if errorlevel 1 goto quit

@if not exist ai_dmnet.asm %CC% %PATH_GAME%/ai_dmnet.c
@if errorlevel 1 goto quit

@if not exist ai_dmq3.asm %CC% %PATH_GAME%/ai_dmq3.c
@if errorlevel 1 goto quit

@if not exist ai_main.asm %CC% %PATH_GAME%/ai_main.c
@if errorlevel 1 goto quit

@if not exist ai_chat.asm %CC% %PATH_GAME%/ai_chat.c
@if errorlevel 1 goto quit

@if not exist ai_cmd.asm %CC% %PATH_GAME%/ai_cmd.c
@if errorlevel 1 goto quit

@if not exist ai_team.asm %CC% %PATH_GAME%/ai_team.c
@if errorlevel 1 goto quit

@if not exist g_active.asm %CC% %PATH_GAME%/g_active.c
@if errorlevel 1 goto quit

@if not exist g_arenas.asm %CC% %PATH_GAME%/g_arenas.c
@if errorlevel 1 goto quit

@if not exist g_bot.asm %CC% %PATH_GAME%/g_bot.c
@if errorlevel 1 goto quit

@if not exist g_client.asm %CC% %PATH_GAME%/g_client.c
@if errorlevel 1 goto quit

@if not exist g_cmds.asm %CC% %PATH_GAME%/g_cmds.c
@if errorlevel 1 goto quit

@if not exist g_combat.asm %CC% %PATH_GAME%/g_combat.c
@if errorlevel 1 goto quit

@if not exist g_items.asm %CC% %PATH_GAME%/g_items.c
@if errorlevel 1 goto quit

@if not exist g_mem.asm %CC% %PATH_GAME%/g_mem.c
@if errorlevel 1 goto quit

@if not exist g_misc.asm %CC% %PATH_GAME%/g_misc.c
@if errorlevel 1 goto quit

@if not exist g_missile.asm %CC% %PATH_GAME%/g_missile.c
@if errorlevel 1 goto quit

@if not exist g_mover.asm %CC% %PATH_GAME%/g_mover.c
@if errorlevel 1 goto quit

@if not exist g_session.asm %CC% %PATH_GAME%/g_session.c
@if errorlevel 1 goto quit

@if not exist g_spawn.asm %CC% %PATH_GAME%/g_spawn.c
@if errorlevel 1 goto quit

@if not exist g_svcmds.asm %CC% %PATH_GAME%/g_svcmds.c
@if errorlevel 1 goto quit

@if not exist g_target.asm %CC% %PATH_GAME%/g_target.c
@if errorlevel 1 goto quit

@if not exist g_team.asm %CC% %PATH_GAME%/g_team.c
@if errorlevel 1 goto quit

@if not exist g_trigger.asm %CC% %PATH_GAME%/g_trigger.c
@if errorlevel 1 goto quit

@if not exist g_utils.asm %CC% %PATH_GAME%/g_utils.c
@if errorlevel 1 goto quit

@if not exist g_weapon.asm %CC% %PATH_GAME%/g_weapon.c
@if errorlevel 1 goto quit

@if not exist ai_vcmd.asm %CC% %PATH_GAME%/ai_vcmd.c
@if errorlevel 1 goto quit

@if not exist g_unlagged.asm %CC% %PATH_GAME%/g_unlagged.c
@if errorlevel 1 goto quit

@if not exist g_sg_utils.asm %CC% %PATH_GAME%/g_sg_utils.c
@if errorlevel 1 goto quit

@if not exist g_hit.asm %CC% %PATH_GAME%/g_hit.c
@if errorlevel 1 goto quit

@if not exist g_unlagged.asm %CC% %PATH_GAME%/g_unlagged.c
@if errorlevel 1 goto quit

@echo Building cgame ...

@if not exist bg_misc.asm %CC_CGAME% %PATH_GAME%/bg_misc.c
@if errorlevel 1 goto quit

@if not exist bg_pmove.asm %CC_CGAME% %PATH_GAME%/bg_pmove.c
@if errorlevel 1 goto quit

@if not exist bg_slidemove.asm %CC_CGAME% %PATH_GAME%/bg_slidemove.c
@if errorlevel 1 goto quit

@if not exist bg_lib.asm %CC_CGAME% %PATH_GAME%/bg_lib.c
@if errorlevel 1 goto quit

@if not exist q_math.asm %CC_CGAME% %PATH_GAME%/q_math.c
@if errorlevel 1 goto quit

@if not exist q_shared.asm %CC_CGAME% %PATH_GAME%/q_shared.c
@if errorlevel 1 goto quit

@if not exist cg_consolecmds.asm %CC_CGAME% %PATH_CGAME%/cg_consolecmds.c
@if errorlevel 1 goto quit

@if not exist cg_draw.asm %CC_CGAME% %PATH_CGAME%/cg_draw.c
@if errorlevel 1 goto quit

@if not exist cg_drawtools.asm %CC_CGAME% %PATH_CGAME%/cg_drawtools.c
@if errorlevel 1 goto quit

@if not exist cg_effects.asm %CC_CGAME% %PATH_CGAME%/cg_effects.c
@if errorlevel 1 goto quit

@if not exist cg_ents.asm %CC_CGAME% %PATH_CGAME%/cg_ents.c
@if errorlevel 1 goto quit

@if not exist cg_event.asm %CC_CGAME% %PATH_CGAME%/cg_event.c
@if errorlevel 1 goto quit

@if not exist cg_info.asm %CC_CGAME% %PATH_CGAME%/cg_info.c
@if errorlevel 1 goto quit

@if not exist cg_localents.asm %CC_CGAME% %PATH_CGAME%/cg_localents.c
@if errorlevel 1 goto quit

@if not exist cg_main.asm %CC_CGAME% %PATH_CGAME%/cg_main.c
@if errorlevel 1 goto quit

@if not exist cg_marks.asm %CC_CGAME% %PATH_CGAME%/cg_marks.c
@if errorlevel 1 goto quit

@if not exist cg_players.asm %CC_CGAME% %PATH_CGAME%/cg_players.c
@if errorlevel 1 goto quit

@if not exist cg_playerstate.asm %CC_CGAME% %PATH_CGAME%/cg_playerstate.c
@if errorlevel 1 goto quit

@if not exist cg_predict.asm %CC_CGAME% %PATH_CGAME%/cg_predict.c
@if errorlevel 1 goto quit

@if not exist cg_scoreboard.asm %CC_CGAME% %PATH_CGAME%/cg_scoreboard.c
@if errorlevel 1 goto quit

@if not exist cg_servercmds.asm %CC_CGAME% %PATH_CGAME%/cg_servercmds.c
@if errorlevel 1 goto quit

@if not exist cg_snapshot.asm %CC_CGAME% %PATH_CGAME%/cg_snapshot.c
@if errorlevel 1 goto quit

@if not exist cg_view.asm %CC_CGAME% %PATH_CGAME%/cg_view.c
@if errorlevel 1 goto quit

@if not exist cg_weapons.asm %CC_CGAME% %PATH_CGAME%/cg_weapons.c
@if errorlevel 1 goto quit

@if not exist ui_shared.asm %CC_CGAME% %PATH_UI%/ui_shared.c
@if errorlevel 1 goto quit

@if not exist cg_newDraw.asm %CC_CGAME% %PATH_CGAME%/cg_newDraw.c
@if errorlevel 1 goto quit

@if not exist cg_sg_utils.asm %CC_CGAME% %PATH_CGAME%/cg_sg_utils.c
@if errorlevel 1 goto quit

@if not exist cg_unlagged.asm %CC_CGAME% %PATH_CGAME%/cg_unlagged.c
@if errorlevel 1 goto quit

@echo Building ui ...

@if not exist ui_main.asm %CC% %PATH_UI%/ui_main.c
@if errorlevel 1 goto quit

@if not exist bg_misc.asm %CC% %PATH_GAME%/bg_misc.c
@if errorlevel 1 goto quit

@if not exist bg_lib.asm %CC% %PATH_GAME%/bg_lib.c
@if errorlevel 1 goto quit

@if not exist q_math.asm %CC% %PATH_GAME%/q_math.c
@if errorlevel 1 goto quit

@if not exist q_shared.asm %CC% %PATH_GAME%/q_shared.c
@if errorlevel 1 goto quit

@if not exist ui_atoms.asm %CC% %PATH_UI%/ui_atoms.c
@if errorlevel 1 goto quit

@if not exist ui_players.asm %CC% %PATH_UI%/ui_players.c
@if errorlevel 1 goto quit

@if not exist ui_util.asm %CC% %PATH_UI%/ui_util.c
@if errorlevel 1 goto quit

@if not exist ui_shared.asm %CC% %PATH_UI%/ui_shared.c
@if errorlevel 1 goto quit

@if not exist ui_gameinfo.asm %CC% %PATH_UI%/ui_gameinfo.c
@if errorlevel 1 goto quit

@set Q3ASM_ARGS=-o qagame
@set Q3ASM_ARGS=%Q3ASM_ARGS% g_main
@set Q3ASM_ARGS=%Q3ASM_ARGS% %PATH_GAME%\g_syscalls
@set Q3ASM_ARGS=%Q3ASM_ARGS% bg_misc
@set Q3ASM_ARGS=%Q3ASM_ARGS% bg_lib
@set Q3ASM_ARGS=%Q3ASM_ARGS% bg_pmove
@set Q3ASM_ARGS=%Q3ASM_ARGS% bg_slidemove
@set Q3ASM_ARGS=%Q3ASM_ARGS% q_math
@set Q3ASM_ARGS=%Q3ASM_ARGS% q_shared
@set Q3ASM_ARGS=%Q3ASM_ARGS% ai_dmnet
@set Q3ASM_ARGS=%Q3ASM_ARGS% ai_dmq3
@set Q3ASM_ARGS=%Q3ASM_ARGS% ai_team
@set Q3ASM_ARGS=%Q3ASM_ARGS% ai_main
@set Q3ASM_ARGS=%Q3ASM_ARGS% ai_chat
@set Q3ASM_ARGS=%Q3ASM_ARGS% ai_cmd
@set Q3ASM_ARGS=%Q3ASM_ARGS% g_active
@set Q3ASM_ARGS=%Q3ASM_ARGS% g_arenas
@set Q3ASM_ARGS=%Q3ASM_ARGS% g_bot
@set Q3ASM_ARGS=%Q3ASM_ARGS% g_client
@set Q3ASM_ARGS=%Q3ASM_ARGS% g_cmds
@set Q3ASM_ARGS=%Q3ASM_ARGS% g_combat
@set Q3ASM_ARGS=%Q3ASM_ARGS% g_items
@set Q3ASM_ARGS=%Q3ASM_ARGS% g_mem
@set Q3ASM_ARGS=%Q3ASM_ARGS% g_misc
@set Q3ASM_ARGS=%Q3ASM_ARGS% g_missile
@set Q3ASM_ARGS=%Q3ASM_ARGS% g_mover
@set Q3ASM_ARGS=%Q3ASM_ARGS% g_session
@set Q3ASM_ARGS=%Q3ASM_ARGS% g_spawn
@set Q3ASM_ARGS=%Q3ASM_ARGS% g_svcmds
@set Q3ASM_ARGS=%Q3ASM_ARGS% g_target
@set Q3ASM_ARGS=%Q3ASM_ARGS% g_team
@set Q3ASM_ARGS=%Q3ASM_ARGS% g_trigger
@set Q3ASM_ARGS=%Q3ASM_ARGS% g_utils
@set Q3ASM_ARGS=%Q3ASM_ARGS% g_weapon
@set Q3ASM_ARGS=%Q3ASM_ARGS% ai_vcmd
@set Q3ASM_ARGS=%Q3ASM_ARGS% g_unlagged
@set Q3ASM_ARGS=%Q3ASM_ARGS% g_sg_utils
@set Q3ASM_ARGS=%Q3ASM_ARGS% g_hit
@if not exist ..\vm\qagame.qvm %Q3ASM% %Q3ASM_ARGS%
@if exist qagame.qvm move qagame.qvm ..\vm
@if exist qagame.map move qagame.map ..\map

@set Q3ASM_ARGS=-o cgame
@set Q3ASM_ARGS=%Q3ASM_ARGS% cg_main
@set Q3ASM_ARGS=%Q3ASM_ARGS% %PATH_CGAME%\cg_syscalls
@set Q3ASM_ARGS=%Q3ASM_ARGS% cg_consolecmds
@set Q3ASM_ARGS=%Q3ASM_ARGS% cg_draw
@set Q3ASM_ARGS=%Q3ASM_ARGS% cg_drawtools
@set Q3ASM_ARGS=%Q3ASM_ARGS% cg_effects
@set Q3ASM_ARGS=%Q3ASM_ARGS% cg_ents
@set Q3ASM_ARGS=%Q3ASM_ARGS% cg_event
@set Q3ASM_ARGS=%Q3ASM_ARGS% cg_info
@set Q3ASM_ARGS=%Q3ASM_ARGS% cg_localents
@set Q3ASM_ARGS=%Q3ASM_ARGS% cg_marks
@set Q3ASM_ARGS=%Q3ASM_ARGS% cg_players
@set Q3ASM_ARGS=%Q3ASM_ARGS% cg_playerstate
@set Q3ASM_ARGS=%Q3ASM_ARGS% cg_predict
@set Q3ASM_ARGS=%Q3ASM_ARGS% cg_scoreboard
@set Q3ASM_ARGS=%Q3ASM_ARGS% cg_servercmds
@set Q3ASM_ARGS=%Q3ASM_ARGS% cg_snapshot
@set Q3ASM_ARGS=%Q3ASM_ARGS% cg_view
@set Q3ASM_ARGS=%Q3ASM_ARGS% cg_weapons
@set Q3ASM_ARGS=%Q3ASM_ARGS% bg_slidemove
@set Q3ASM_ARGS=%Q3ASM_ARGS% bg_pmove
@set Q3ASM_ARGS=%Q3ASM_ARGS% bg_lib
@set Q3ASM_ARGS=%Q3ASM_ARGS% bg_misc
@set Q3ASM_ARGS=%Q3ASM_ARGS% q_math
@set Q3ASM_ARGS=%Q3ASM_ARGS% q_shared
@set Q3ASM_ARGS=%Q3ASM_ARGS% ui_shared
@set Q3ASM_ARGS=%Q3ASM_ARGS% cg_newDraw
@set Q3ASM_ARGS=%Q3ASM_ARGS% cg_sg_utils
@set Q3ASM_ARGS=%Q3ASM_ARGS% cg_unlagged
@if not exist ..\vm\cgame.qvm %Q3ASM% %Q3ASM_ARGS%
@if exist cgame.qvm move cgame.qvm ..\vm
@if exist cgame.map move cgame.map ..\map

@set Q3ASM_ARGS=-o ui
@set Q3ASM_ARGS=%Q3ASM_ARGS% ui_main
@set Q3ASM_ARGS=%Q3ASM_ARGS% %PATH_UI%\ui_syscalls
@set Q3ASM_ARGS=%Q3ASM_ARGS% ui_atoms
@set Q3ASM_ARGS=%Q3ASM_ARGS% ui_players
@set Q3ASM_ARGS=%Q3ASM_ARGS% ui_util
@set Q3ASM_ARGS=%Q3ASM_ARGS% ui_shared
@set Q3ASM_ARGS=%Q3ASM_ARGS% ui_gameinfo
@set Q3ASM_ARGS=%Q3ASM_ARGS% bg_misc
@set Q3ASM_ARGS=%Q3ASM_ARGS% bg_lib
@set Q3ASM_ARGS=%Q3ASM_ARGS% q_math
@set Q3ASM_ARGS=%Q3ASM_ARGS% q_shared
@if not exist ..\vm\ui.qvm %Q3ASM% %Q3ASM_ARGS%
@if exist ui.qvm move ui.qvm ..\vm
@if exist ui.map move ui.map ..\map

:quit
pause