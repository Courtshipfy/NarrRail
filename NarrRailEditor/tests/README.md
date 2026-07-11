# NarrRailEditor Regression Tests

Run from `NarrRailEditor/`:

```bash
npm test
```

The current suite is intentionally centered on future shared-core seams rather than Vue component internals.

## Covered Seams

- `.nrstory` import, validation, and serialization
- Shared-core story-format entrypoint
- Legacy Choice shape import compatibility
- Deprecated `edges[].condition` rejection
- GlobalConfig serialization, parsing, normalization, and invalid schema handling
- `.nroutline` / legacy `.nrrail` outline parsing, serialization, and validation
- Shared-core outline-format and GlobalConfig entrypoints
- Story preview runner behavior for dialogue, variables, conditions, choices, timers, events, and endings
- Rail/project preview runner behavior across outline Story nodes
- Story Project snapshots and Project Review Queue aggregation
- Import Package human-review item aggregation

## Pending Seams

These need concrete production interfaces before their contract tests can be useful:

- GitHub project storage adapter with fake transport
- Full Story Project Import Package validation

Those seams are tracked by the Story Project model and project storage adapter follow-up issues.
