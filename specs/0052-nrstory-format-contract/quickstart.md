# Quickstart: Validate #52

Run from the repository root.

## 1. Confirm Spec Kit integration is Codex-only

```bash
specify integration status
```

Expected outcome:

- Spec Kit reports only the Codex integration as installed.

## 2. Confirm neutral format contract exists

```bash
test -f Docs/spec/NRSTORY_FORMAT.md
rg -n "Story Consumer compatibility|GlobalConfig|schemaVersion|nodeType|import|export" Docs/spec/NRSTORY_FORMAT.md
```

Expected outcome:

- The neutral contract exists.
- It includes schema version ownership, GlobalConfig, node types, validation, import/export, and Story Consumer compatibility language.

## 3. Confirm legacy runtime entry points to the neutral contract

```bash
rg -n "Docs/spec/NRSTORY_FORMAT.md|Story Consumer compatibility" Docs/02_runtime/SCRIPT_FORMAT.md README.md
```

Expected outcome:

- Existing format entry points direct contributors toward the neutral contract.
- UE-specific behavior is described as consumer-specific rather than neutral format semantics.
