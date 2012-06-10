#!/bin/sh

CMD_PREFIX="i686-pc-mingw32 i586-mingw32msvc i686-w64-mingw32 i386-mingw32msvc mingw32";


if [ "X$CC" = "X" ]; then
	for check in $CMD_PREFIX; 
	do
		full_check="${check}-gcc"
		if [ ! $(which "$full_check" 2>/dev/null) = "" ]; then
			export CC="$full_check"
			break
		fi
	done
fi

if [ "X$WINDRES" = "X" ]; then
	for check in $CMD_PREFIX
	do
		full_check="${check}-windres"
		if [ ! $(which "$full_check" 2>/dev/null) = "" ]; then
			export WINDRES="$full_check"
			break
		fi
	done
fi

if [ "X$WINDRES" = "X" -o "X$CC" = "X" ]; then
	echo "Error: Must define or find WINDRES and CC"
	exit 1
fi

export PLATFORM=mingw32
export ARCH=x86

COMPILE_PLATFORM=`uname|sed -e s/_.*//|tr '[:upper:]' '[:lower:]'`
if [ "$COMPILE_PLATFORM" = "freebsd" ]; then
	exec gmake $*
else
	exec make $*
fi
