# Contract: conversion-review.json

`conversion-review.json` is an optional machine-readable companion to
`conversion-notes.md`.

## Minimal Shape

```json
{
  "schemaVersion": 1,
  "packageId": "import-2026-07-11",
  "items": [
    {
      "id": "review-001",
      "severity": "review",
      "issueType": "ambiguous-source",
      "message": "The speaker for this line needs confirmation.",
      "sourceLocation": {
        "path": "drafts/chapter-01.docx",
        "lineStart": 42,
        "lineEnd": 43,
        "excerpt": "I never said the station was safe."
      },
      "generatedTarget": {
        "kind": "nrstory",
        "path": "Stories/chapter_01.nrstory",
        "nodeId": "N_Chapter01_042",
        "field": "dialogue.speakerId"
      },
      "suggestedAction": "Confirm the speaker before accepting the import."
    }
  ]
}
```

## Severity

- `error`: blocks import acceptance until resolved.
- `warning`: should be reviewed before acceptance.
- `review`: informational or confirmation-needed item.

Unknown severities normalize to `review`.

## Issue Types

Recommended values:

- `ambiguous-source`
- `omitted-content`
- `needs-confirmation`
- `format-risk`
- `generated-target`
- `source-only`
- `profile-mismatch`

Readers should preserve or display unknown issue types as categories.

## Queue Mapping

- `issueType` maps to Project Review Queue `category`.
- `message` maps to Project Review Queue `message`.
- `generatedTarget.path` maps to `assetPath`.
- `generatedTarget.nodeId`, `edgeId`, `field`, `lineIndex`, `choiceIndex`, and `conditionBranchIndex` map into queue `target`.
- `sourceLocation` and `suggestedAction` remain available on the queue item for future UI.
