#!/usr/bin/env bash
set -euo pipefail

script_dir=$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd)
repo_root=$(cd -- "$script_dir/../.." && pwd)
dist_dir=${SG_COMPANION_DIST:-"$script_dir/dist"}
version=$(sed -n 's/^version = "\([^"]*\)"/\1/p' "$script_dir/Cargo.toml" | head -n1)
toolchain=${SG_RUST_TOOLCHAIN:-stable}
stage=$(mktemp -d)
use_zigbuild=false
trap 'rm -rf "$stage"' EXIT

mkdir -p "$dist_dir" "$stage/src"
dist_dir=$(cd -- "$dist_dir" && pwd)
cp "$script_dir/Cargo.toml" "$script_dir/Cargo.lock" "$script_dir/README.md" "$stage/"
cp "$script_dir/src/"*.rs "$stage/src/"
cp "$repo_root/COPYING.txt" "$stage/LICENSE.txt"
cp "$script_dir/THIRD_PARTY_NOTICES.txt" "$stage/"
(
  cd "$stage"
  zip -q -9 -r "$dist_dir/smokinguns-companion-source-v${version}.zip" .
)

target=${SG_COMPANION_TARGET:-}
if [[ -z "$target" ]]; then
  case "$(uname -s)" in
    Linux) target=x86_64-unknown-linux-gnu ;;
    MINGW*|MSYS*|CYGWIN*) target=x86_64-pc-windows-gnu ;;
    *) echo "Set SG_COMPANION_TARGET to a supported Rust target triple" >&2; exit 2 ;;
  esac
fi

if command -v rustup >/dev/null 2>&1; then
  if ! rustup toolchain list | awk '{print $1}' | grep -Eq "^${toolchain}(-|$)"; then
    rustup toolchain install "$toolchain" --profile minimal --no-self-update
  fi
  if ! rustup target list --installed --toolchain "$toolchain" | grep -Fxq "$target"; then
    rustup target add "$target" --toolchain "$toolchain"
  fi
  rustc_cmd=(rustup run "$toolchain" rustc)
  cargo_cmd=(rustup run "$toolchain" cargo)
else
  rustc_cmd=(rustc)
  cargo_cmd=(cargo)
fi
target_libdir=$("${rustc_cmd[@]}" --print target-libdir --target "$target" 2>/dev/null || true)
if [[ -z "$target_libdir" || ! -d "$target_libdir" ]]; then
  echo "Rust standard library for target '$target' is unavailable to the selected compiler." >&2
  echo "Install rustup or the matching system Rust target package." >&2
  exit 2
fi

case "$target" in
  x86_64-unknown-linux-musl)
    linker=${CARGO_TARGET_X86_64_UNKNOWN_LINUX_MUSL_LINKER:-}
    if [[ -z "$linker" ]]; then
      for candidate in musl-gcc x86_64-linux-musl-gcc musl-clang; do
        if command -v "$candidate" >/dev/null 2>&1; then linker=$candidate; break; fi
      done
    fi
    if [[ -z "$linker" ]]; then
      echo "Target $target requires a musl linker (musl-gcc, x86_64-linux-musl-gcc, or musl-clang)." >&2
      exit 2
    fi
    export CARGO_TARGET_X86_64_UNKNOWN_LINUX_MUSL_LINKER="$linker"
    export RUSTFLAGS="${RUSTFLAGS:+$RUSTFLAGS }-C relocation-model=static -C link-arg=-static"
    ;;
  aarch64-unknown-linux-musl)
    linker=${CARGO_TARGET_AARCH64_UNKNOWN_LINUX_MUSL_LINKER:-}
    if [[ -z "$linker" ]]; then
      candidates=(aarch64-linux-musl-gcc)
      if [[ "$(uname -m)" == aarch64 || "$(uname -m)" == arm64 ]]; then
        candidates+=(musl-gcc)
      fi
      for candidate in "${candidates[@]}"; do
        if command -v "$candidate" >/dev/null 2>&1; then linker=$candidate; break; fi
      done
    fi
    if [[ -z "$linker" ]]; then
      if command -v cargo-zigbuild >/dev/null 2>&1; then
        use_zigbuild=true
      else
        echo "Target $target requires aarch64-linux-musl-gcc or cargo-zigbuild with Zig." >&2
        exit 2
      fi
    else
      export CARGO_TARGET_AARCH64_UNKNOWN_LINUX_MUSL_LINKER="$linker"
    fi
    export RUSTFLAGS="${RUSTFLAGS:+$RUSTFLAGS }-C relocation-model=static -C link-arg=-static"
    ;;
  aarch64-pc-windows-gnullvm)
    linker=${CARGO_TARGET_AARCH64_PC_WINDOWS_GNULLVM_LINKER:-}
    if [[ -z "$linker" ]]; then
      for candidate in aarch64-w64-mingw32-clang; do
        if command -v "$candidate" >/dev/null 2>&1; then linker=$candidate; break; fi
      done
    fi
    if [[ -z "$linker" ]]; then
      echo "Target $target requires llvm-mingw (aarch64-w64-mingw32-clang)." >&2
      exit 2
    fi
    export CARGO_TARGET_AARCH64_PC_WINDOWS_GNULLVM_LINKER="$linker"
    ;;
  x86_64-pc-windows-gnu)
    linker=${CARGO_TARGET_X86_64_PC_WINDOWS_GNU_LINKER:-x86_64-w64-mingw32-gcc}
    if ! command -v "$linker" >/dev/null 2>&1; then
      echo "Target $target requires x86_64-w64-mingw32-gcc (install MinGW-w64)." >&2
      exit 2
    fi
    export CARGO_TARGET_X86_64_PC_WINDOWS_GNU_LINKER="$linker"
    ;;
  x86_64-pc-windows-msvc)
    echo "The MSVC target requires the Microsoft linker and Windows SDK; build on Windows or use x86_64-pc-windows-gnu with MinGW." >&2
    exit 2
    ;;
  aarch64-pc-windows-msvc)
    case "$(uname -s)" in
      MINGW*|MSYS*|CYGWIN*) ;;
      *) echo "The ARM64 MSVC target must be built on Windows with the ARM64 MSVC tools and Windows SDK." >&2; exit 2 ;;
    esac
    ;;
esac

if [[ "$use_zigbuild" == true ]]; then
  "${cargo_cmd[@]}" zigbuild --manifest-path "$script_dir/Cargo.toml" --release --target "$target"
else
  "${cargo_cmd[@]}" build --manifest-path "$script_dir/Cargo.toml" --release --target "$target"
fi
case "$target" in
  *windows*) os=windows; executable=smokinguns-companion.exe ;;
  *linux*) os=linux; executable=smokinguns-companion ;;
  *) echo "Unsupported release target: $target" >&2; exit 2 ;;
esac
case "$target" in
  x86_64-*) arch=x64 ;;
  aarch64-*) arch=arm64 ;;
  *) echo "Unsupported release architecture: $target" >&2; exit 2 ;;
esac
binary="$script_dir/target/$target/release/$executable"
cp "$binary" "$stage/$executable"
(
  cd "$stage"
  zip -q -9 "$dist_dir/smokinguns-companion-${os}-${arch}-v${version}.zip" "$executable" README.md LICENSE.txt THIRD_PARTY_NOTICES.txt
)
echo "Created companion archives in $dist_dir"
