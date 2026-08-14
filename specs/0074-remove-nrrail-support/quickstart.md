# Quickstart Validation: Remove `.nrrail` Support

Run from the repository root.

## 1. Verify project-core classification

```bash
cd NarrRailEditor
npm test
```

Expected: `story-project.test.mjs` proves `.nroutline` is classified as an
outline and `.nrrail` is classified as `unknown`.

## 2. Verify the production bundle

```bash
cd NarrRailEditor
npm run build
```

Expected: Vite completes without errors.

## 3. Inspect active implementation paths

```bash
rg -n -i --glob '!node_modules/**' --glob '!dist/**' --glob '!package-lock.json' '\\.nrrail|nrrail' NarrRailEditor/src NarrRailEditor/api
```

Expected: no matches.

## 4. Inspect active documentation claims

Read `README.md`, `README.en.md`, `Docs/spec/NRSTORY_FORMAT.md`, and
`Docs/02_runtime/SCRIPT_FORMAT.md`.

Expected: each identifies `.nroutline` as the only supported Story Outline
suffix; none claims `.nrrail` can be read or edited.
