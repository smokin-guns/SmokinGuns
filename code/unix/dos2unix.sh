#!/bin/sh

if [ "x$1" != "x" ] && [ -f "$1" ]; then
	if [ ! -f "$1.dos" ]; then
		cp -p "$1" "$1.dos"
		tr -d '\015' < "$1.dos" > "$1"
	fi
fi

