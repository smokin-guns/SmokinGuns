#! /bin/sh

export LANG=C

FILE="$1"
[ -z "$FILE" ] && exit 1

# Output C header
cat >$FILE <<HEADER
// SDK DIFF for modders tools

const char *sdk_diff[] = {
HEADER

if [ -d .git ]; then
	DIFF="git diff HEAD"
else
	DIFF="svn diff -x -w"
fi
SIZE=$( $DIFF 2>/dev/null | wc -c )
BASE64CMD=$( base64 --version 2>/dev/null )
PERLCMD=$( perl --version 2>/dev/null )
CHUNK=10

if [ "$SIZE" -eq 0 ]; then
	FORMAT=0
elif [ -n "$BASE64CMD" ]; then
	FORMAT=1
	$DIFF | gzip -c9nq | base64 -w$((CHUNK*57*4/3)) | while read buffer
	do
		echo "\"$buffer\","
	done >>$FILE
elif [ -n "$PERLCMD" ]; then
	FORMAT=2
	PERLSCRIPT="while (read(STDIN, \$buf, $CHUNK*57)) { print \"\t\\\"\",encode_base64(\$buf,\"\"),\"\\\",\n\"; }"
	$DIFF | gzip -c9nq | perl -MMIME::Base64 -e "$PERLSCRIPT" >>$FILE
else
	FORMAT=3
	$DIFF | tr '"\\aeiouy123456AEIOUY7890@#=\-+rst(}' '^!654321YUOIEA7890@#yuoiearst=\-+}(' | while read line
	do
		echo "\"${line}\","
	done >>$FILE
fi

# Output C footer
cat >>$FILE <<HEADER
NULL
};

const int sdk_diff_format = $FORMAT;
const int sdk_diff_size = sizeof(sdk_diff)/sizeof(sdk_diff[0]);

const char **sdk_shared_textarray = sdk_diff;
const int *sdk_shared_size = &sdk_diff_size;

// Generated on $(date)

HEADER

echo $FORMAT
