# Quickstart: Validate #54

Run from the repository root.

## 1. Confirm package root exists

```bash
test -d split-packages/NarrRail-Unreal-Plugin
test -f split-packages/NarrRail-Unreal-Plugin/README.md
test -f split-packages/NarrRail-Unreal-Plugin/SPLIT_MANIFEST.md
```

Expected outcome:

- The prepared package root and top-level docs exist.

## 2. Confirm copied code and host project exist

```bash
test -f split-packages/NarrRail-Unreal-Plugin/NarrRail/NarrRail.uplugin
test -f split-packages/NarrRail-Unreal-Plugin/NarrRailUEHost/NarrRailUEHost.uproject
test -d split-packages/NarrRail-Unreal-Plugin/NarrRail/Source
test -d split-packages/NarrRail-Unreal-Plugin/NarrRailUEHost/Source
```

Expected outcome:

- Plugin and host project are present in the split package.

## 3. Confirm consumer positioning and format links

```bash
rg -n "Story Consumer|NRSTORY_FORMAT|main repository|neutral format" split-packages/NarrRail-Unreal-Plugin/README.md split-packages/NarrRail-Unreal-Plugin/SPLIT_MANIFEST.md
```

Expected outcome:

- The package positions itself as a Story Consumer and points back to the main repository's neutral format contract.

## 4. Confirm exclusions

```bash
find split-packages/NarrRail-Unreal-Plugin -name .DS_Store -o -path "*/Intermediate/*" -o -path "*/Saved/*" -o -path "*/Binaries/*" -o -path "*/DerivedDataCache/*"
```

Expected outcome:

- No excluded local/generated files are listed.
