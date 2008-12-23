#!/bin/sh

if [ "x$1" != "x" ] && [ -f "$1" ]; then
	mv -f "$1" "${1%.dos}"
fi

