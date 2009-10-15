#!/bin/bash
APPBUNDLE="Smokin' Guns.app"
BINARY=smokinguns.i386
DEDBIN=smokinguns_dedicated.i386
PKGINFO=APPLSG
ICNS=misc/smokinguns.icns
DESTDIR=build/release-darwin-i386
BASEDIR=smokinguns

cd `dirname $0`
if [ ! -f Makefile ]; then
	echo "This script must be run from the smokinguns build directory"
	exit 1
fi

SG_VERSION=`grep '^VERSION=' Makefile | sed -e 's/.*=\(.*\)/\1/'`

if [ ! -d $DESTDIR ]; then
	mkdir -p $DESTDIR
fi

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

cp code/libs/macosx/*.dylib "$DESTDIR/$APPBUNDLE/Contents/MacOS/"
cp $DESTDIR/$BINARY "$DESTDIR/$APPBUNDLE/Contents/MacOS/"
cp $DESTDIR/$DEDBIN "$DESTDIR/$APPBUNDLE/Contents/MacOS/"
