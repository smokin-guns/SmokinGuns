ECHO OFF
CLS

REM Hardcoded variables
SET PRODUCT_VERSION=1.1

ECHO Today is %date%

SET DATESTAMP=%date:~6,4%%date:~3,2%%date:~0,2%

REM If Win2K we got default DDD DD.MM.YYYY date format
REM Lets split it into two parts
FOR /F "tokens=1,2" %%i in ("%date%") do (
    SET FIRSTPART=%%i
    SET SECONDPART=%%j
)
REM If second part is not empty - we got win2K
REM And sometimes DDD can be two chars instead of three
REM That is why we work with second part
IF "%SECONDPART%" NEQ "" SET DATESTAMP=%SECONDPART:~6,4%%SECONDPART:~3,2%%SECONDPART:~0,2%

REM Make sure we have a safe environment
SET LIBRARY=
SET INCLUDE=
SET LCC_DVARS=-DSMOKINGUNS -DPRODUCT_VERSION=%PRODUCT_VERSION% -DSG_RELEASE=%DATESTAMP% -DSTANDALONE
SET CURDIR=%~dp0
SET PREVPATH=%PATH%
SET PATH=%PATH%;%CURDIR%\bin\

SET PATH_GAME=code\game
SET PATH_CGAME=code\cgame
SET PATH_UI=code\ui
SET PATH_QCOMMON=code\qcommon

SET PATH_GAME_BUILT=build\asm\game
SET PATH_CGAME_BUILT=build\asm\cgame
SET PATH_UI_BUILT=build\asm\ui
SET PATH_QCOMMON_BUILT=build\asm\qcommon

SET CC_QCOMMON="q3lcc %LCC_DVARS%"
SET CC_GAME="q3lcc -DQAGAME %LCC_DVARS%"
SET CC_CGAME="q3lcc -DCGAME %LCC_DVARS%"
SET CC_UI="q3lcc -DUI %LCC_DVARS%"
SET Q3ASM=q3asm

REM Change dir to root of the project
cd ..\..\

IF NOT EXIST build mkdir build
IF NOT EXIST build\vm mkdir build\vm
IF NOT EXIST build\asm mkdir build\asm
IF NOT EXIST %PATH_GAME_BUILT% mkdir %PATH_GAME_BUILT%
IF NOT EXIST %PATH_CGAME_BUILT% mkdir %PATH_CGAME_BUILT%
IF NOT EXIST %PATH_UI_BUILT% mkdir %PATH_UI_BUILT%
IF NOT EXIST %PATH_QCOMMON_BUILT% mkdir %PATH_QCOMMON_BUILT%

ECHO Building qcommon ...
    CALL :compile_it %CC_QCOMMON% %PATH_QCOMMON_BUILT%\q_math.asm %PATH_QCOMMON%/q_math.c
    CALL :compile_it %CC_QCOMMON% %PATH_QCOMMON_BUILT%\q_shared.asm %PATH_QCOMMON%/q_shared.c
ECHO Building game ...
    CALL :compile_it %CC_GAME% %PATH_GAME_BUILT%\g_main.asm %PATH_GAME%/g_main.c
    CALL :compile_it %CC_GAME% %PATH_GAME_BUILT%\bg_misc.asm %PATH_GAME%/bg_misc.c
    CALL :compile_it %CC_GAME% %PATH_GAME_BUILT%\bg_lib.asm %PATH_GAME%/bg_lib.c
    CALL :compile_it %CC_GAME% %PATH_GAME_BUILT%\bg_pmove.asm %PATH_GAME%/bg_pmove.c
    CALL :compile_it %CC_GAME% %PATH_GAME_BUILT%\bg_slidemove.asm %PATH_GAME%/bg_slidemove.c
    CALL :compile_it %CC_GAME% %PATH_GAME_BUILT%\ai_dmnet.asm %PATH_GAME%/ai_dmnet.c
    CALL :compile_it %CC_GAME% %PATH_GAME_BUILT%\ai_dmq3.asm %PATH_GAME%/ai_dmq3.c
    CALL :compile_it %CC_GAME% %PATH_GAME_BUILT%\ai_main.asm %PATH_GAME%/ai_main.c
    CALL :compile_it %CC_GAME% %PATH_GAME_BUILT%\ai_chat.asm %PATH_GAME%/ai_chat.c
    CALL :compile_it %CC_GAME% %PATH_GAME_BUILT%\ai_cmd.asm %PATH_GAME%/ai_cmd.c
    CALL :compile_it %CC_GAME% %PATH_GAME_BUILT%\ai_team.asm %PATH_GAME%/ai_team.c
    CALL :compile_it %CC_GAME% %PATH_GAME_BUILT%\g_active.asm %PATH_GAME%/g_active.c
    CALL :compile_it %CC_GAME% %PATH_GAME_BUILT%\g_arenas.asm %PATH_GAME%/g_arenas.c
    CALL :compile_it %CC_GAME% %PATH_GAME_BUILT%\g_bot.asm %PATH_GAME%/g_bot.c
    CALL :compile_it %CC_GAME% %PATH_GAME_BUILT%\g_client.asm %PATH_GAME%/g_client.c
    CALL :compile_it %CC_GAME% %PATH_GAME_BUILT%\g_cmds.asm %PATH_GAME%/g_cmds.c
    CALL :compile_it %CC_GAME% %PATH_GAME_BUILT%\g_combat.asm %PATH_GAME%/g_combat.c
    CALL :compile_it %CC_GAME% %PATH_GAME_BUILT%\g_items.asm %PATH_GAME%/g_items.c
    CALL :compile_it %CC_GAME% %PATH_GAME_BUILT%\g_mem.asm %PATH_GAME%/g_mem.c
    CALL :compile_it %CC_GAME% %PATH_GAME_BUILT%\g_misc.asm %PATH_GAME%/g_misc.c
    CALL :compile_it %CC_GAME% %PATH_GAME_BUILT%\g_missile.asm %PATH_GAME%/g_missile.c
    CALL :compile_it %CC_GAME% %PATH_GAME_BUILT%\g_mover.asm %PATH_GAME%/g_mover.c
    CALL :compile_it %CC_GAME% %PATH_GAME_BUILT%\g_session.asm %PATH_GAME%/g_session.c
    CALL :compile_it %CC_GAME% %PATH_GAME_BUILT%\g_spawn.asm %PATH_GAME%/g_spawn.c
    CALL :compile_it %CC_GAME% %PATH_GAME_BUILT%\g_svcmds.asm %PATH_GAME%/g_svcmds.c
    CALL :compile_it %CC_GAME% %PATH_GAME_BUILT%\g_target.asm %PATH_GAME%/g_target.c
    CALL :compile_it %CC_GAME% %PATH_GAME_BUILT%\g_team.asm %PATH_GAME%/g_team.c
    CALL :compile_it %CC_GAME% %PATH_GAME_BUILT%\g_trigger.asm %PATH_GAME%/g_trigger.c
    CALL :compile_it %CC_GAME% %PATH_GAME_BUILT%\g_utils.asm %PATH_GAME%/g_utils.c
    CALL :compile_it %CC_GAME% %PATH_GAME_BUILT%\g_weapon.asm %PATH_GAME%/g_weapon.c
    CALL :compile_it %CC_GAME% %PATH_GAME_BUILT%\ai_vcmd.asm %PATH_GAME%/ai_vcmd.c
    CALL :compile_it %CC_GAME% %PATH_GAME_BUILT%\g_unlagged.asm %PATH_GAME%/g_unlagged.c
    CALL :compile_it %CC_GAME% %PATH_GAME_BUILT%\g_sg_utils.asm %PATH_GAME%/g_sg_utils.c
    CALL :compile_it %CC_GAME% %PATH_GAME_BUILT%\g_hit.asm %PATH_GAME%/g_hit.c
ECHO Linking game ...
    SET Q3ASM_ARGS=
    SET Q3ASM_ARGS=%Q3ASM_ARGS% %PATH_GAME_BUILT%\g_main
    SET Q3ASM_ARGS=%Q3ASM_ARGS% %PATH_GAME%\g_syscalls
    SET Q3ASM_ARGS=%Q3ASM_ARGS% %PATH_GAME_BUILT%\bg_misc
    SET Q3ASM_ARGS=%Q3ASM_ARGS% %PATH_GAME_BUILT%\bg_lib
    SET Q3ASM_ARGS=%Q3ASM_ARGS% %PATH_GAME_BUILT%\bg_pmove
    SET Q3ASM_ARGS=%Q3ASM_ARGS% %PATH_GAME_BUILT%\bg_slidemove
    SET Q3ASM_ARGS=%Q3ASM_ARGS% %PATH_QCOMMON_BUILT%\q_math
    SET Q3ASM_ARGS=%Q3ASM_ARGS% %PATH_QCOMMON_BUILT%\q_shared
    SET Q3ASM_ARGS=%Q3ASM_ARGS% %PATH_GAME_BUILT%\ai_dmnet
    SET Q3ASM_ARGS=%Q3ASM_ARGS% %PATH_GAME_BUILT%\ai_dmq3
    SET Q3ASM_ARGS=%Q3ASM_ARGS% %PATH_GAME_BUILT%\ai_team
    SET Q3ASM_ARGS=%Q3ASM_ARGS% %PATH_GAME_BUILT%\ai_main
    SET Q3ASM_ARGS=%Q3ASM_ARGS% %PATH_GAME_BUILT%\ai_chat
    SET Q3ASM_ARGS=%Q3ASM_ARGS% %PATH_GAME_BUILT%\ai_cmd
    SET Q3ASM_ARGS=%Q3ASM_ARGS% %PATH_GAME_BUILT%\g_active
    SET Q3ASM_ARGS=%Q3ASM_ARGS% %PATH_GAME_BUILT%\g_arenas
    SET Q3ASM_ARGS=%Q3ASM_ARGS% %PATH_GAME_BUILT%\g_bot
    SET Q3ASM_ARGS=%Q3ASM_ARGS% %PATH_GAME_BUILT%\g_client
    SET Q3ASM_ARGS=%Q3ASM_ARGS% %PATH_GAME_BUILT%\g_cmds
    SET Q3ASM_ARGS=%Q3ASM_ARGS% %PATH_GAME_BUILT%\g_combat
    SET Q3ASM_ARGS=%Q3ASM_ARGS% %PATH_GAME_BUILT%\g_items
    SET Q3ASM_ARGS=%Q3ASM_ARGS% %PATH_GAME_BUILT%\g_mem
    SET Q3ASM_ARGS=%Q3ASM_ARGS% %PATH_GAME_BUILT%\g_misc
    SET Q3ASM_ARGS=%Q3ASM_ARGS% %PATH_GAME_BUILT%\g_missile
    SET Q3ASM_ARGS=%Q3ASM_ARGS% %PATH_GAME_BUILT%\g_mover
    SET Q3ASM_ARGS=%Q3ASM_ARGS% %PATH_GAME_BUILT%\g_session
    SET Q3ASM_ARGS=%Q3ASM_ARGS% %PATH_GAME_BUILT%\g_spawn
    SET Q3ASM_ARGS=%Q3ASM_ARGS% %PATH_GAME_BUILT%\g_svcmds
    SET Q3ASM_ARGS=%Q3ASM_ARGS% %PATH_GAME_BUILT%\g_target
    SET Q3ASM_ARGS=%Q3ASM_ARGS% %PATH_GAME_BUILT%\g_team
    SET Q3ASM_ARGS=%Q3ASM_ARGS% %PATH_GAME_BUILT%\g_trigger
    SET Q3ASM_ARGS=%Q3ASM_ARGS% %PATH_GAME_BUILT%\g_utils
    SET Q3ASM_ARGS=%Q3ASM_ARGS% %PATH_GAME_BUILT%\g_weapon
    SET Q3ASM_ARGS=%Q3ASM_ARGS% %PATH_GAME_BUILT%\ai_vcmd
    SET Q3ASM_ARGS=%Q3ASM_ARGS% %PATH_GAME_BUILT%\g_unlagged
    SET Q3ASM_ARGS=%Q3ASM_ARGS% %PATH_GAME_BUILT%\g_sg_utils
    SET Q3ASM_ARGS=%Q3ASM_ARGS% %PATH_GAME_BUILT%\g_hit
    CALL :compile_it %Q3ASM% build\vm\qagame.qvm "%Q3ASM_ARGS%"
ECHO Building cgame ...
    CALL :compile_it %CC_CGAME% %PATH_CGAME_BUILT%\cg_consolecmds.asm %PATH_CGAME%/cg_consolecmds.c
    CALL :compile_it %CC_CGAME% %PATH_CGAME_BUILT%\cg_draw.asm %PATH_CGAME%/cg_draw.c
    CALL :compile_it %CC_CGAME% %PATH_CGAME_BUILT%\cg_drawtools.asm %PATH_CGAME%/cg_drawtools.c
    CALL :compile_it %CC_CGAME% %PATH_CGAME_BUILT%\cg_effects.asm %PATH_CGAME%/cg_effects.c
    CALL :compile_it %CC_CGAME% %PATH_CGAME_BUILT%\cg_ents.asm %PATH_CGAME%/cg_ents.c
    CALL :compile_it %CC_CGAME% %PATH_CGAME_BUILT%\cg_event.asm %PATH_CGAME%/cg_event.c
    CALL :compile_it %CC_CGAME% %PATH_CGAME_BUILT%\cg_info.asm %PATH_CGAME%/cg_info.c
    CALL :compile_it %CC_CGAME% %PATH_CGAME_BUILT%\cg_localents.asm %PATH_CGAME%/cg_localents.c
    CALL :compile_it %CC_CGAME% %PATH_CGAME_BUILT%\cg_main.asm %PATH_CGAME%/cg_main.c
    CALL :compile_it %CC_CGAME% %PATH_CGAME_BUILT%\cg_marks.asm %PATH_CGAME%/cg_marks.c
    CALL :compile_it %CC_CGAME% %PATH_CGAME_BUILT%\cg_players.asm %PATH_CGAME%/cg_players.c
    CALL :compile_it %CC_CGAME% %PATH_CGAME_BUILT%\cg_playerstate.asm %PATH_CGAME%/cg_playerstate.c
    CALL :compile_it %CC_CGAME% %PATH_CGAME_BUILT%\cg_predict.asm %PATH_CGAME%/cg_predict.c
    CALL :compile_it %CC_CGAME% %PATH_CGAME_BUILT%\cg_scoreboard.asm %PATH_CGAME%/cg_scoreboard.c
    CALL :compile_it %CC_CGAME% %PATH_CGAME_BUILT%\cg_servercmds.asm %PATH_CGAME%/cg_servercmds.c
    CALL :compile_it %CC_CGAME% %PATH_CGAME_BUILT%\cg_snapshot.asm %PATH_CGAME%/cg_snapshot.c
    CALL :compile_it %CC_CGAME% %PATH_CGAME_BUILT%\cg_view.asm %PATH_CGAME%/cg_view.c
    CALL :compile_it %CC_CGAME% %PATH_CGAME_BUILT%\cg_weapons.asm %PATH_CGAME%/cg_weapons.c
    CALL :compile_it %CC_CGAME% %PATH_CGAME_BUILT%\cg_newdraw.asm %PATH_CGAME%/cg_newdraw.c
    CALL :compile_it %CC_CGAME% %PATH_CGAME_BUILT%\cg_sg_utils.asm %PATH_CGAME%/cg_sg_utils.c
    CALL :compile_it %CC_CGAME% %PATH_CGAME_BUILT%\cg_unlagged.asm %PATH_CGAME%/cg_unlagged.c

    CALL :compile_it %CC_CGAME% %PATH_CGAME_BUILT%\bg_misc.asm %PATH_GAME%/bg_misc.c
    CALL :compile_it %CC_CGAME% %PATH_CGAME_BUILT%\bg_pmove.asm %PATH_GAME%/bg_pmove.c
    CALL :compile_it %CC_CGAME% %PATH_CGAME_BUILT%\bg_slidemove.asm %PATH_GAME%/bg_slidemove.c
    CALL :compile_it %CC_CGAME% %PATH_CGAME_BUILT%\bg_lib.asm %PATH_GAME%/bg_lib.c
    CALL :compile_it %CC_CGAME% %PATH_CGAME_BUILT%\ui_shared.asm %PATH_UI%/ui_shared.c
ECHO linking cgame ...
    SET Q3ASM_ARGS=
    SET Q3ASM_ARGS=%Q3ASM_ARGS% %PATH_CGAME_BUILT%\cg_main
    SET Q3ASM_ARGS=%Q3ASM_ARGS% %PATH_CGAME%\cg_syscalls
    SET Q3ASM_ARGS=%Q3ASM_ARGS% %PATH_CGAME_BUILT%\cg_consolecmds
    SET Q3ASM_ARGS=%Q3ASM_ARGS% %PATH_CGAME_BUILT%\cg_draw
    SET Q3ASM_ARGS=%Q3ASM_ARGS% %PATH_CGAME_BUILT%\cg_drawtools
    SET Q3ASM_ARGS=%Q3ASM_ARGS% %PATH_CGAME_BUILT%\cg_effects
    SET Q3ASM_ARGS=%Q3ASM_ARGS% %PATH_CGAME_BUILT%\cg_ents
    SET Q3ASM_ARGS=%Q3ASM_ARGS% %PATH_CGAME_BUILT%\cg_event
    SET Q3ASM_ARGS=%Q3ASM_ARGS% %PATH_CGAME_BUILT%\cg_info
    SET Q3ASM_ARGS=%Q3ASM_ARGS% %PATH_CGAME_BUILT%\cg_localents
    SET Q3ASM_ARGS=%Q3ASM_ARGS% %PATH_CGAME_BUILT%\cg_marks
    SET Q3ASM_ARGS=%Q3ASM_ARGS% %PATH_CGAME_BUILT%\cg_players
    SET Q3ASM_ARGS=%Q3ASM_ARGS% %PATH_CGAME_BUILT%\cg_playerstate
    SET Q3ASM_ARGS=%Q3ASM_ARGS% %PATH_CGAME_BUILT%\cg_predict
    SET Q3ASM_ARGS=%Q3ASM_ARGS% %PATH_CGAME_BUILT%\cg_scoreboard
    SET Q3ASM_ARGS=%Q3ASM_ARGS% %PATH_CGAME_BUILT%\cg_servercmds
    SET Q3ASM_ARGS=%Q3ASM_ARGS% %PATH_CGAME_BUILT%\cg_snapshot
    SET Q3ASM_ARGS=%Q3ASM_ARGS% %PATH_CGAME_BUILT%\cg_view
    SET Q3ASM_ARGS=%Q3ASM_ARGS% %PATH_CGAME_BUILT%\cg_weapons
    SET Q3ASM_ARGS=%Q3ASM_ARGS% %PATH_CGAME_BUILT%\bg_slidemove
    SET Q3ASM_ARGS=%Q3ASM_ARGS% %PATH_CGAME_BUILT%\bg_pmove
    SET Q3ASM_ARGS=%Q3ASM_ARGS% %PATH_CGAME_BUILT%\bg_lib
    SET Q3ASM_ARGS=%Q3ASM_ARGS% %PATH_CGAME_BUILT%\bg_misc
    SET Q3ASM_ARGS=%Q3ASM_ARGS% %PATH_QCOMMON_BUILT%\q_math
    SET Q3ASM_ARGS=%Q3ASM_ARGS% %PATH_QCOMMON_BUILT%\q_shared
    SET Q3ASM_ARGS=%Q3ASM_ARGS% %PATH_CGAME_BUILT%\ui_shared
    SET Q3ASM_ARGS=%Q3ASM_ARGS% %PATH_CGAME_BUILT%\cg_newdraw
    SET Q3ASM_ARGS=%Q3ASM_ARGS% %PATH_CGAME_BUILT%\cg_sg_utils
    SET Q3ASM_ARGS=%Q3ASM_ARGS% %PATH_CGAME_BUILT%\cg_unlagged
    CALL :compile_it %Q3ASM% build\vm\cgame.qvm "%Q3ASM_ARGS%"
ECHO Building ui ...
    CALL :compile_it %CC_UI% %PATH_UI_BUILT%\ui_atoms.asm %PATH_UI%/ui_atoms.c
    CALL :compile_it %CC_UI% %PATH_UI_BUILT%\ui_players.asm %PATH_UI%/ui_players.c
    CALL :compile_it %CC_UI% %PATH_UI_BUILT%\ui_shared.asm %PATH_UI%/ui_shared.c
    CALL :compile_it %CC_UI% %PATH_UI_BUILT%\ui_gameinfo.asm %PATH_UI%/ui_gameinfo.c
    CALL :compile_it %CC_UI% %PATH_UI_BUILT%\ui_main.asm %PATH_UI%/ui_main.c
    CALL :compile_it %CC_UI% %PATH_UI_BUILT%\bg_misc.asm %PATH_GAME%/bg_misc.c
    CALL :compile_it %CC_UI% %PATH_UI_BUILT%\bg_lib.asm %PATH_GAME%/bg_lib.c
ECHO linking ui ...
    SET Q3ASM_ARGS=
    SET Q3ASM_ARGS=%Q3ASM_ARGS% %PATH_UI_BUILT%\ui_main
    SET Q3ASM_ARGS=%Q3ASM_ARGS% %PATH_UI%\ui_syscalls
    SET Q3ASM_ARGS=%Q3ASM_ARGS% %PATH_UI_BUILT%\ui_atoms
    SET Q3ASM_ARGS=%Q3ASM_ARGS% %PATH_UI_BUILT%\ui_players
    SET Q3ASM_ARGS=%Q3ASM_ARGS% %PATH_UI_BUILT%\ui_shared
    SET Q3ASM_ARGS=%Q3ASM_ARGS% %PATH_UI_BUILT%\ui_gameinfo
    SET Q3ASM_ARGS=%Q3ASM_ARGS% %PATH_UI_BUILT%\bg_misc
    SET Q3ASM_ARGS=%Q3ASM_ARGS% %PATH_UI_BUILT%\bg_lib
    SET Q3ASM_ARGS=%Q3ASM_ARGS% %PATH_QCOMMON_BUILT%\q_math
    SET Q3ASM_ARGS=%Q3ASM_ARGS% %PATH_QCOMMON_BUILT%\q_shared
    CALL :compile_it %Q3ASM% build\vm\ui.qvm "%Q3ASM_ARGS%"
GOTO quit

REM FUNCTONS SECTION
:compile_it
SET CC=%~1
SET OUTPUT=%~2
SET INPUT=%~3

IF EXIST %OUTPUT% EXIT /b

REM Say what we do
ECHO %CC% -o %OUTPUT% %INPUT%

REM Do it
%CC% -o %OUTPUT% %INPUT%
IF NOT ERRORLEVEL 1 EXIT /b

REM ELSE (If exitcode >= 1)
ECHO Failed
GOTO quit
EXIT /b

:quit

REM Go to start dir, set path variable back, pause, exit
cd %CURDIR%
SET PATH=%PREVPATH%
ECHO Done
PAUSE

REM Exit required because of goto quit used in function
EXIT