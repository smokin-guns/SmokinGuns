#!/bin/sh

COMPILE_PLATFORM=`uname|sed -e s/_.*//|tr '[:upper:]' '[:lower:]'`

MAKE=make
export PLATFORM=mingw32

if [ "$COMPILE_PLATFORM" = "darwin" ]; then
	export CC=i386-mingw32msvc-gcc
	export WINDRES=i386-mingw32msvc-windres

elif [ "$COMPILE_PLATFORM" = "freebsd" ]; then
	export CC=mingw32-gcc
	export WINDRES=mingw32-windres
	MAKE=gmake

elif [ "$(</etc/system-release)" == "Fedora release 10 (Cambridge)" ]; then
	export CC=i686-pc-mingw32-gcc
	export WINDRES=i686-pc-mingw32-windres

else
	export CC=i586-mingw32msvc-gcc
	export WINDRES=i586-mingw32msvc-windres
fi

exec $MAKE $*
