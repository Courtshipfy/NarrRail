# NarrRail Shared Core

This folder is the initial shell-neutral shared core for NarrRail authoring logic.

It intentionally exposes story-format, outline-format, validation, GlobalConfig,
Story Project review, and preview semantics without Vue refs, DOM downloads,
`fetch`, GitHub URLs, localStorage, or desktop-shell APIs.

The current implementation wraps existing `src/utils/` modules so the seam can
stabilize before the code is physically moved into a package.
