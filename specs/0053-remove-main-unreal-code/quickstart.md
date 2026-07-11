# Quickstart: Validate #53

Run from the repository root.

## 1. Confirm active Unreal source is gone

```bash
test ! -e NarrRail
test ! -e NarrRailUEHost
test ! -e split-packages/NarrRail-Unreal-Plugin
```

Expected outcome:

- Active UE source, host, and split package copy are absent from the main repository.

## 2. Confirm main docs point to the external consumer repo

```bash
rg -n "NarrRail-Unreal-Plugin|github.com/Courtshipfy/NarrRail-Unreal-Plugin" README.md README.en.md Docs/story-consumers Docs/spec Docs/02_runtime/SCRIPT_FORMAT.md
```

Expected outcome:

- Main docs contain links to the external Unreal consumer repository.

## 3. Confirm old local UE docs are gone

```bash
test ! -e Docs/03_ui_blueprint
test ! -e Docs/04_narrrail_ue_host
test ! -e Docs/01_architecture/TECH_ARCHITECTURE.md
test ! -e Docs/02_runtime/DEBUGGER_GUIDE.md
```

Expected outcome:

- UE-owned implementation docs are no longer carried as main-repo docs.

## 4. General validation

```bash
git diff --check
```

Expected outcome:

- No whitespace errors.
