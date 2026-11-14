#!/usr/bin/env bash
set -euo pipefail

repo_root=$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")/.." && pwd)
web_dir="$repo_root/web"
build_dir=${BUILD_DIR:-"$repo_root/build"}
site_dir=${SITE_DIR:-"$build_dir/site"}
emscripten_build=${EMSCRIPTEN_BUILD_DIR:-"$build_dir/release-emscripten-wasm32"}
game_data_dir=${WEB_GAME_DATA_DIR:-"$build_dir/web-game-data"}
version=$(sed -n 's/^version = "\([^"]*\)"/\1/p' "$web_dir/companion/Cargo.toml" | head -n1)

"$web_dir/prepare-game-data.sh"
EM_CACHE=${EM_CACHE:-/tmp/emscripten-cache} make -C "$repo_root" PLATFORM=emscripten

if [[ ! -d "$emscripten_build" ]]; then
  echo "Emscripten build output missing: $emscripten_build" >&2
  echo "Set EMSCRIPTEN_BUILD_DIR if your Makefile uses another output path." >&2
  exit 1
fi

mkdir -p "$site_dir/downloads"
cp "$emscripten_build/smokinguns_opengl2.wasm32.js" \
  "$emscripten_build/smokinguns_opengl2.wasm32.wasm" "$site_dir/"
cp "$web_dir/index.html" "$web_dir/transport.js" "$repo_root/COPYING.txt" \
  "$build_dir/Smokin_Guns_LICENSE.txt" "$site_dir/"
chunk_args=(--chunk-mib "${WEB_GAME_CHUNK_MIB:-16}")
if [[ -n "${WEB_GAME_CDN_BASE:-}" ]]; then
  chunk_args+=(--cdn-base "$WEB_GAME_CDN_BASE")
fi
python3 "$web_dir/split-game-data.py" "$game_data_dir" "$site_dir/game-data" \
  "${chunk_args[@]}"
# Remove generated build products copied by older versions of this script.
rm -rf "$site_dir/client" "$site_dir/ded" "$site_dir/missionpack" \
  "$site_dir/renderergl1" "$site_dir/renderergl2" "$site_dir/tools" "$site_dir/smokinguns"
rm -f "$site_dir"/smokinguns*.data

SG_COMPANION_DIST="$site_dir/downloads" \
SG_COMPANION_TARGET=x86_64-unknown-linux-musl \
  bash "$web_dir/companion/package.sh"
SG_COMPANION_DIST="$site_dir/downloads" \
SG_COMPANION_TARGET=x86_64-pc-windows-gnu \
  bash "$web_dir/companion/package.sh"
SG_COMPANION_DIST="$site_dir/downloads" \
SG_COMPANION_TARGET=aarch64-unknown-linux-musl \
  bash "$web_dir/companion/package.sh"
SG_COMPANION_DIST="$site_dir/downloads" \
SG_COMPANION_TARGET=aarch64-pc-windows-gnullvm \
  bash "$web_dir/companion/package.sh"

echo "Static site assembled at $site_dir"
echo "Host this directory over HTTPS. Companion pairing uses loopback access from the browser."
