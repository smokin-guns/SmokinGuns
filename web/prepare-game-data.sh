#!/bin/sh
set -eu

repo_root=$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd)
build_dir=${BUILD_DIR:-"$repo_root/build"}
archive=${SMOKINGUNS_DATA_ARCHIVE:-"$build_dir/Smokin_Guns_1.1.zip"}
data_dir=${WEB_GAME_DATA_DIR:-"$build_dir/web-game-data"}
expected_sha512=51954ce00cccd9eb95a10491f0fcdea2d024058da11cbda7ee56c6369e8bc101d89ed95549cf4255393c3e1bec4585be12937e7ed7b20ff4cc0ce96685a5ce72
archive_url=https://www.smokin-guns.org/download/Smokin_Guns_1.1.zip

mkdir -p "$build_dir"
if [ ! -f "$archive" ]; then
  curl --fail --location --retry 2 --output "$archive" "$archive_url"
fi

actual_sha512=$(sha512sum "$archive" | awk '{print $1}')
if [ "$actual_sha512" != "$expected_sha512" ]; then
  echo "Smokin' Guns 1.1 archive checksum mismatch" >&2
  exit 1
fi

unzip -p "$archive" "Smokin' Guns 1.1/LICENSE.txt" > "$build_dir/Smokin_Guns_LICENSE.txt"

stage=$(mktemp -d "$build_dir/web-game-data.XXXXXX")
trap 'rm -rf "$stage"' EXIT HUP INT TERM
unzip -q "$archive" "Smokin' Guns 1.1/baseq3/*" "Smokin' Guns 1.1/smokinguns/*" -d "$stage"

mkdir -p "$data_dir/baseq3" "$data_dir/smokinguns"
cp -a "$stage/Smokin' Guns 1.1/baseq3/." "$data_dir/baseq3/"
cp -a "$stage/Smokin' Guns 1.1/smokinguns/." "$data_dir/smokinguns/"
touch "$data_dir.prepared"

if [ ! -f "$data_dir/baseq3/pak0.pk3" ] || [ ! -f "$data_dir/smokinguns/sg_pak0.pk3" ]; then
  echo "The official archive did not contain the expected game packages" >&2
  exit 1
fi

printf "Prepared official Smokin' Guns 1.1 browser data in %s\n" "$data_dir"
