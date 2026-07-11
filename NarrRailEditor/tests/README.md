# NarrRailEditor Regression Tests

Run from `NarrRailEditor/`:

```bash
npm test
```

The current suite is intentionally centered on future shared-core seams rather than Vue component internals.

## Covered Seams

- `.nrstory` import, validation, and serialization
- Legacy Choice shape import compatibility
- Deprecated `edges[].condition` rejection
- GlobalConfig serialization, parsing, normalization, and invalid schema handling
- `.nroutline` / legacy `.nrrail` outline parsing, serialization, and validation
- Story preview runner behavior for dialogue, variables, conditions, choices, timers, events, and endings
- Rail/project preview runner behavior across outline Story nodes

## Pending Seams

These need concrete production interfaces before their contract tests can be useful:

- Project Review Queue aggregation
- Story Project Import Package validation
- GitHub project storage adapter with fake transport

Those seams are tracked by the Story Project model and project storage adapter follow-up issues.
