# Quickstart: Validate #55

Run from the repository root.

## 1. Contract docs exist

```bash
test -f Docs/spec/CONVERSION_REVIEW_CONTRACT.md
test -f specs/0055-conversion-review-contract/contracts/conversion-review-json.md
```

Expected outcome:

- Both human-facing and Spec Kit contract docs exist.

## 2. Converter policy references the contract

```bash
rg -n "conversion-review.json|conversion-notes.md|CONVERSION_REVIEW_CONTRACT" \
  .codex/skills/narrrail-story-converter Docs/spec/CONVERSION_REVIEW_CONTRACT.md
```

Expected outcome:

- Converter skill and policy point to the standardized notes/review contract.

## 3. Project Review Queue loads JSON review metadata

```bash
cd NarrRailEditor
npm test
```

Expected outcome:

- `story-project.test.mjs` includes an Import Package with `conversion-review.json`.
- The test confirms the JSON item becomes a Project Review Queue item.

## 4. Build still passes

```bash
cd NarrRailEditor
npm run build
```

Expected outcome:

- Production build passes without changes to `.nrstory`, GlobalConfig, or `.nroutline` semantics.
