# Plinky 12 Community Panel Builds

This repository stores blessed generated artifacts for
[`plinkysynth/community-panels`](https://github.com/plinkysynth/community-panels).

Do not submit source panels here. Source submissions belong in
`community-panels`; this repo contains generated `index.json`, copied source and
artwork used by the website, per-panel `build.json`, and Git LFS-tracked
`.bin.lzma` firmware artifacts. Raw `.bin` and `.uf2` files are intentionally
not stored here; the website derives UF2 downloads from the LZMA artifact when
needed.

Each collection update builds every panel against the newest published,
non-yanked firmware source revision (normally the latest alpha). Author firmware
pins are ignored. The previous collection is retained if any panel fails to build.
Curated examples use that same firmware snapshot and build policy.

Each `build.json` records the full base firmware Git SHA, release code/channel,
firmware build date, panel source SHA, artifact build time and checksums. These
fields are also carried into `index.json` for display on the website. Reusing a
cached binary or republishing documentation does not reset its build timestamp.
The current tree contains one firmware artifact per panel; previous versions
remain available in Git history. Optional PDF manuals and author bios are copied
alongside the artifacts.
