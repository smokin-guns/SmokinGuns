#!/bin/sh

COMPILE_PLATFORM=`uname|sed -e s/_.*//|tr '[:upper:]' '[:lower:]'`

MAKE=make

if [ "$COMPILE_PLATFORM" = "darwin" ]; then
	export CC=i386-mingw32msvc-gcc
	export WINDRES=i386-mingw32msvc-windres

elif [ "$COMPILE_PLATFORM" = "freebsd" ]; then
	export CC=mingw32-gcc
	export WINDRES=mingw32-windres
	MAKE=gmake

elif [ -e "/etc/redhat-release" ]; then
	export CC=i686-pc-mingw32-gcc
	export WINDRES=i686-pc-mingw32-windres
fi

if [ -z "$CC" ]
then
  export CC=i586-mingw32msvc-gcc
fi

if [ -z "$WINDRES" ]
then
  export WINDRES=i586-mingw32msvc-windres
fi

export PLATFORM=mingw32
export ARCH=x86

exec $MAKE $*
