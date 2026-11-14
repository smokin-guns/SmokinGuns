# Smokin' Guns companion

The Rust companion is a small local helper for the browser client. It discovers
Smokin' Guns servers over UDP, shows a one-use pairing key, and relays game
packets between the browser's WebSocket and the selected server. It binds to
`127.0.0.1` only; it does not expose an open UDP relay to the network.

## Run a release build

Download and extract the archive matching your operating system and CPU, then
start the executable. Windows builds open a compact native window; Linux opens
one when an X11 display (including XWayland) is available. The companion listens at
`http://127.0.0.1:8765`. Copy the one-use key shown there into the multiplayer
section of the Smokin' Guns web client, then choose **Pair companion** and
**Get new list**. Select a server and start the game. Closing the native window
stops the companion. No browser manager tab is used. If the native window cannot
open, the companion stays running in terminal mode on every build. Use `s` to
show pairing status, `n` to issue a fresh key and revoke existing sessions, and
`q` to quit. On Windows, start it from a terminal to use these commands directly;
when launched without a terminal, the regular native window is used.

Use **New code** in the companion window to revoke the current browser
session and issue a fresh one-use code. The launcher also keeps its **Reset
pairing** control. Either code can be used immediately.

Release archives are published by the `companion-release` GitHub Actions
workflow for `companion-v*` tags. Linux x64 and ARM64 use musl and are statically
linked. Windows x64 uses GNU/MinGW, and Windows ARM64 is built natively with
MSVC on an ARM64 runner. The source archive contains the Rust project and GPL
license.

## Build and run from source

Install Rust/Cargo, then from the repository root:

```sh
cargo run --release --manifest-path web/companion/Cargo.toml
```

To use the locally built browser game, build it first:

```sh
EM_CACHE=/tmp/emscripten-cache make PLATFORM=emscripten
```

Then start the companion from the repository root; it serves the assembled
`build/site` when available, otherwise `build/release-emscripten-wasm32`, at
`http://localhost:8765`. Without a local build it runs proxy-only; open the
hosted browser client and pair using the code in the companion window.

To check discovery against a local dedicated server, start the server first
and run in another terminal:

```sh
SG_SERVERS=127.0.0.1:27960 cargo run --release --manifest-path web/companion/Cargo.toml
```

## Build release archives

Run `bash web/companion/package.sh` from the repository root. It creates the
source archive and a binary archive for the selected target in
`web/companion/dist/`. Cross-compiling both release binaries from Linux
requires the Rust targets and linkers:

```sh
rustup target add x86_64-unknown-linux-musl x86_64-pc-windows-gnu aarch64-unknown-linux-musl
# Install musl-gcc, an AArch64 musl cross-linker, and MinGW-w64.
SG_COMPANION_TARGET=x86_64-unknown-linux-musl bash web/companion/package.sh
SG_COMPANION_TARGET=x86_64-pc-windows-gnu bash web/companion/package.sh
SG_COMPANION_TARGET=aarch64-unknown-linux-musl bash web/companion/package.sh
```

For cross-compiling the ARM64 archives from x64 Linux, install Zig and add the
`cargo-zigbuild` helper and LLVM-MinGW, then add the Rust targets:

```sh
cargo install --locked cargo-zigbuild
rustup target add aarch64-unknown-linux-musl aarch64-pc-windows-gnullvm
SG_COMPANION_TARGET=aarch64-unknown-linux-musl bash web/companion/package.sh
PATH=/path/to/llvm-mingw/bin:$PATH SG_COMPANION_TARGET=aarch64-pc-windows-gnullvm bash web/companion/package.sh
```

The Windows ARM64 cross-build uses the GNU/LLVM ABI (gnullvm). The release
workflow also builds a native MSVC Windows ARM64 package on an ARM64 Windows
runner. After installing the tools and targets, `web/build-static-site.sh`
builds all four x64 and ARM64 companion archives into the site's downloads.

## Settings and limits

The default master is `master.smokin-guns.org:27950`. Discovery checks both
protocol 71 and legacy protocol 68, then keeps responding Smokin' Guns servers
that match either. Set `SG_MASTER` or `SG_MASTER_PORT` to override the master;
set `SG_PROTOCOL=68` or `SG_PROTOCOL=71` to check only one protocol. Set
`SG_SERVERS` to a comma-separated list of `host:port` endpoints to check
directly; for example, `SG_SERVERS=127.0.0.1:27960` targets a local server.
Set `SG_COMPANION_PORT` to change the local port, `SG_WEB_ROOT` to serve a
different local web build, or `SG_ALLOWED_ORIGINS` to restrict web origins.
By default, the one-use pairing key and short-lived session token protect the
local APIs.

Pairing keys are one use. Session tokens expire after 12 hours and are kept in
the current browser tab's session storage. Restarting the companion prints a
new key. Discovery uses IPv4 and checks the full master response with up to 128
server queries in parallel. Offline, IPv6-only, incompatible, or nonresponsive
servers are omitted.
