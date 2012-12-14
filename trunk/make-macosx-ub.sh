#!/bin/bash
APPBUNDLE="Smokin' Guns.app"
BINARY=smokinguns.ub
DEDBIN=smokinguns_dedicated.ub
PKGINFO=APPLSG
ICNS=misc/smokinguns.icns
DESTDIR=build/release-darwin-ub
BASEDIR=smokinguns

BIN_OBJ="
	build/release-darwin-ppc/smokinguns.ppc
	build/release-darwin-i386/smokinguns.i386
"
BIN_DEDOBJ="
	build/release-darwin-ppc/smokinguns_dedicated.ppc
	build/release-darwin-i386/smokinguns_dedicated.i386
"

cd `dirname $0`
if [ ! -f Makefile ]; then
	echo "This script must be run from the smokinguns build directory"
	exit 1
fi

SG_VERSION=`grep '^VERSION *=' Makefile.local | sed -e 's/.*= *\(.*\)/\1/'`

# We only care if we're >= 10.4, not if we're specifically Tiger.
# "8" is the Darwin major kernel version.
TIGERHOST=`uname -r |perl -w -p -e 's/\A(\d+)\..*\Z/$1/; $_ = (($_ >= 8) ? "1" : "0");'`

# we want to use the oldest available SDK for max compatiblity
unset PPC_SDK
unset PPC_CFLAGS
unset PPC_LDFLAGS
unset X86_SDK
unset X86_CFLAGS
unset X86_LDFLAGS
if [ -d /Developer/SDKs/MacOSX10.5.sdk ]; then
	PPC_SDK=/Developer/SDKs/MacOSX10.5.sdk
	PPC_CFLAGS="-arch ppc -isysroot /Developer/SDKs/MacOSX10.5.sdk \
			-DMAC_OS_X_VERSION_MIN_REQUIRED=1050"
	PPC_LDFLAGS=" -mmacosx-version-min=10.5"

	X86_SDK=/Developer/SDKs/MacOSX10.5.sdk
	X86_CFLAGS="-arch i386 -isysroot /Developer/SDKs/MacOSX10.5.sdk \
			-DMAC_OS_X_VERSION_MIN_REQUIRED=1050"
	X86_LDFLAGS=" -mmacosx-version-min=10.5"
fi

if [ -d /Developer/SDKs/MacOSX10.4u.sdk ]; then
	PPC_SDK=/Developer/SDKs/MacOSX10.4u.sdk
	PPC_CFLAGS="-arch ppc -isysroot /Developer/SDKs/MacOSX10.4u.sdk \
			-DMAC_OS_X_VERSION_MIN_REQUIRED=1040"
	PPC_LDFLAGS=" -mmacosx-version-min=10.4"

	X86_SDK=/Developer/SDKs/MacOSX10.4u.sdk
	X86_CFLAGS="-arch i386 -isysroot /Developer/SDKs/MacOSX10.4u.sdk \
			-DMAC_OS_X_VERSION_MIN_REQUIRED=1040"
	X86_LDFLAGS=" -mmacosx-version-min=10.4"
fi

if [ -d /Developer/SDKs/MacOSX10.3.9.sdk ] && [ $TIGERHOST ]; then
	PPC_SDK=/Developer/SDKs/MacOSX10.3.9.sdk
	PPC_CFLAGS="-arch ppc -isysroot /Developer/SDKs/MacOSX10.3.9.sdk \
			-DMAC_OS_X_VERSION_MIN_REQUIRED=1030"
	PPC_LDFLAGS=" -mmacosx-version-min=10.3"
fi

if [ -z $PPC_SDK ] || [ -z $X86_SDK ]; then
	echo "\
ERROR: This script is for building a Universal Binary.  You cannot build
       for a different architecture unless you have the proper Mac OS X SDKs
       installed.  If you just want to to compile for your own system run
       'make' instead of this script."
	exit 1
fi

echo "Building PPC Client/Dedicated Server against \"$PPC_SDK\""
echo "Building X86 Client/Dedicated Server against \"$X86_SDK\""
if [ "$PPC_SDK" != "/Developer/SDKs/MacOSX10.3.9.sdk" ] || \
	[ "$X86_SDK" != "/Developer/SDKs/MacOSX10.4u.sdk" ]; then
	echo "\
WARNING: in order to build a binary with maximum compatibility you must
         build on Mac OS X 10.4 using Xcode 2.3 or 2.5 and have the
         MacOSX10.3.9, and MacOSX10.4u SDKs installed from the Xcode 
         install disk Packages folder."
fi
sleep 3

if [ ! -d $DESTDIR ]; then
	mkdir -p $DESTDIR
fi

# For parallel make on multicore boxes...
NCPU=`sysctl -n hw.ncpu`

# ppc client and server
if [ -d build/release-release-ppc ]; then
	rm -r build/release-darwin-ppc
fi
(ARCH=ppc CFLAGS=$PPC_CFLAGS LDFLAGS=$PPC_LDFLAGS make -j$NCPU) || exit 1;

# intel client and server
if [ -d build/release-darwin-i386 ]; then
	rm -r build/release-darwin-i386
fi
(ARCH=i386 CFLAGS=$X86_CFLAGS LDFLAGS=$X86_LDFLAGS make -j$NCPU) || exit 1;

echo "Creating .app bundle $DESTDIR/$APPBUNDLE"
if [ ! -d "$DESTDIR/$APPBUNDLE/Contents/MacOS/$BASEDIR" ]; then
	mkdir -p "$DESTDIR/$APPBUNDLE/Contents/MacOS/$BASEDIR" || exit 1;
fi
if [ ! -d "$DESTDIR/$APPBUNDLE/Contents/Resources" ]; then
	mkdir -p "$DESTDIR/$APPBUNDLE/Contents/Resources"
fi
cp $ICNS "$DESTDIR/$APPBUNDLE/Contents/Resources/smokinguns.icns" || exit 1;
echo $PKGINFO > "$DESTDIR/$APPBUNDLE/Contents/PkgInfo"
echo "
	<?xml version=\"1.0\" encoding=\"UTF-8\"?>
	<!DOCTYPE plist
		PUBLIC \"-//Apple Computer//DTD PLIST 1.0//EN\"
		\"http://www.apple.com/DTDs/PropertyList-1.0.dtd\">
	<plist version=\"1.0\">
	<dict>
		<key>CFBundleDevelopmentRegion</key>
		<string>English</string>
		<key>CFBundleExecutable</key>
		<string>$BINARY</string>
		<key>CFBundleGetInfoString</key>
		<string>Smokin' Guns $SG_VERSION</string>
		<key>CFBundleIconFile</key>
		<string>smokinguns.icns</string>
		<key>CFBundleIdentifier</key>
		<string>net.smokin-guns.www</string>
		<key>CFBundleInfoDictionaryVersion</key>
		<string>6.0</string>
		<key>CFBundleName</key>
		<string>smokinguns</string>
		<key>CFBundlePackageType</key>
		<string>APPL</string>
		<key>CFBundleShortVersionString</key>
		<string>$SG_VERSION</string>
		<key>CFBundleSignature</key>
		<string>$PKGINFO</string>
		<key>CFBundleVersion</key>
		<string>$SG_VERSION</string>
		<key>NSExtensions</key>
		<dict/>
		<key>NSPrincipalClass</key>
		<string>NSApplication</string>
	</dict>
	</plist>
	" > "$DESTDIR/$APPBUNDLE/Contents/Info.plist"

lipo build/release-darwin-ppc/smokinguns.ppc build/release-darwin-i386/smokinguns.i386 -create -output "$DESTDIR/$APPBUNDLE/Contents/MacOS/$BINARY"
lipo build/release-darwin-ppc/smokinguns_dedicated.ppc build/release-darwin-i386/smokinguns_dedicated.i386 -create -output "$DESTDIR/$APPBUNDLE/Contents/MacOS/$DEDBIN"
cp code/libs/macosx/*.dylib "$DESTDIR/$APPBUNDLE/Contents/MacOS/"
