# Browser client

The native engine and game sources remain in `../code`. The browser target
builds the game with Emscripten, SDL2, and WebGL2. Run
`./web/prepare-game-data.sh` before building to fetch the official game data;
it verifies the AUR-published SHA-512 checksum and keeps the data in ignored
build output. Optional `.pk3` packages are loaded from the browser and persisted
with IndexedDB.

Build and run the browser client with the Rust local multiplayer companion using the
steps in [`companion/README.md`](companion/README.md). The companion serves the
Emscripten release files, performs UDP master-server discovery and pings, and
relays the selected server's traffic over a loopback WebSocket. It leaves the
game engine's server browser and network protocol unchanged. Its current
discovery path is IPv4-only and omits servers that do not respond to a ping.

The WebSocket transport preserves each Quake packet as one UDP datagram and
pins each session to a server chosen from the companion's discovered list. The
browser gets a short-lived session token after redeeming a one-use code printed
by the local companion. The companion binds to loopback; do not expose it as a
public relay.

To assemble a self-hosted static site, run `bash web/build-static-site.sh`.
The script builds Linux and Windows x64/ARM64 companion archives. For ARM64
cross-builds from x64 Linux, install Zig, `cargo-zigbuild`, LLVM-MinGW, and the
Rust targets documented in `web/companion/README.md`; put LLVM-MinGW's `bin`
directory on `PATH`.
This prepares the official data, builds the Emscripten client, and writes the
page, runtime files, licenses, and companion source/Linux/Windows archives into
`build/site/`. The companion downloads are linked from the launcher. The
companion opens its local manager window and runs on each player's computer.

Game files are split into content-addressed chunks under `build/site/game-data/chunks/`;
the default chunk size is 16 MiB. Use `WEB_GAME_CHUNK_MIB=8` when assembling
the site to make smaller pieces. The launcher downloads them one at a time,
checks each SHA-256 digest, assembles the original game files in IndexedDB, and
reuses the saved copy on later visits. Serve the chunks as binary files with
long lived immutable caching and the manifest with revalidation; an example
Nginx snippet is in [`nginx-game-data.conf`](nginx-game-data.conf). Since chunk
names include their content hash, a CDN can cache them independently and safely.
To serve chunks from a separate CDN origin, set `WEB_GAME_CDN_BASE` to the URL
of its `game-data` directory, including the trailing slash, when assembling the
site (for example, `https://cdn.example/smokinguns/game-data/`). Enable CORS for
the hosted site on that CDN. The launcher and manifest remain on the main site.

The game-data license requires unchanged official files, distribution with the
project, and no charge for downloads. The release folder includes the game
license and engine `COPYING.txt`.
