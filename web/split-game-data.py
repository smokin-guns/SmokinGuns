#!/usr/bin/env python3
"""Write immutable, content-addressed chunks and a manifest for browser game data."""

import argparse
import hashlib
import json
import shutil
from pathlib import Path


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("source", type=Path, help="prepared web-game-data directory")
    parser.add_argument("destination", type=Path, help="site game-data directory")
    parser.add_argument("--chunk-mib", type=int, default=16)
    parser.add_argument("--cdn-base", default="", help="base URL for game-data files, ending in /")
    args = parser.parse_args()
    if args.chunk_mib < 1:
        parser.error("--chunk-mib must be at least 1")
    if not args.source.is_dir():
        parser.error(f"source directory does not exist: {args.source}")

    if args.destination.exists():
        shutil.rmtree(args.destination)
    chunks_dir = args.destination / "chunks"
    chunks_dir.mkdir(parents=True)
    chunk_bytes = args.chunk_mib * 1024 * 1024
    entries = []

    for source_file in sorted(p for p in args.source.rglob("*") if p.is_file()):
        relative = source_file.relative_to(args.source).as_posix()
        size = source_file.stat().st_size
        chunks = []
        with source_file.open("rb") as stream:
            while True:
                payload = stream.read(chunk_bytes)
                if not payload:
                    break
                digest = hashlib.sha256(payload).hexdigest()
                filename = f"{digest}.bin"
                target = chunks_dir / filename
                if not target.exists():
                    target.write_bytes(payload)
                url = f"chunks/{filename}"
                if args.cdn_base:
                    url = f"{args.cdn_base.rstrip('/')}/{url}"
                chunks.append({"url": url, "size": len(payload), "sha256": digest})
        signature = json.dumps(
            {"size": size, "chunks": [{"size": c["size"], "sha256": c["sha256"]} for c in chunks]},
            sort_keys=True,
            separators=(",", ":"),
        ).encode()
        entries.append({
            "path": relative,
            "size": size,
            "version": hashlib.sha256(signature).hexdigest(),
            "chunks": chunks,
        })

    identity = json.dumps(
        [{"path": entry["path"], "size": entry["size"], "version": entry["version"]}
         for entry in entries],
        sort_keys=True,
        separators=(",", ":"),
    ).encode()
    manifest = {
        "version": hashlib.sha256(identity).hexdigest(),
        "chunkSize": chunk_bytes,
        "files": entries,
    }
    (args.destination / "manifest.json").write_text(
        json.dumps(manifest, indent=2) + "\n", encoding="utf-8"
    )
    count = sum(len(entry["chunks"]) for entry in entries)
    total = sum(entry["size"] for entry in entries)
    print(f"Created {count} chunks for {len(entries)} files ({total} bytes) in {args.destination}")


if __name__ == "__main__":
    main()
