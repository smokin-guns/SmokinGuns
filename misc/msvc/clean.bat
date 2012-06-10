@echo off
echo Cleaning out object files and other junk.

echo Cleaning Debug
rd /S /Q ..\..\build\cgame_debug
rd /S /Q ..\..\build\game_debug
rd /S /Q ..\..\build\sg_debug
rd /S /Q ..\..\build\ui_debug

echo Cleaning Release
rd /S /Q ..\..\build\cgame_release
rd /S /Q ..\..\build\game_release
rd /S /Q ..\..\build\sg_release
rd /S /Q ..\..\build\ui_release

echo Cleaning Release SSE
rd /S /Q ..\..\build\cgame_release_sse
rd /S /Q ..\..\build\game_release_sse
rd /S /Q ..\..\build\sg_release_sse
rd /S /Q ..\..\build\ui_release_sse

echo Cleaning Release SSE2
rd /S /Q ..\..\build\cgame_release_sse2
rd /S /Q ..\..\build\game_release_sse2
rd /S /Q ..\..\build\sg_release_sse2
rd /S /Q ..\..\build\ui_release_sse2

echo Cleaning finished.
pause

IF ERRORLEVEL 1 EXIT 1