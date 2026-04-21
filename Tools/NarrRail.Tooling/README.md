# NarrRail.Tooling

C# CLI tool for NarrRail script validation, import/export, and processing.

## Requirements

- .NET 8.0 SDK or later

## Build

```bash
cd Tools/NarrRail.Tooling/src/NarrRail.Tooling
dotnet build
```

## Run

```bash
dotnet run -- validate <path>
```

Or build and install globally:

```bash
dotnet publish -c Release
# Add to PATH or copy to a bin directory
```

## Commands

### validate

Validate NarrRail script files.

**Single file:**
```bash
narrrail validate story.narrrail.yaml
```

**Directory (non-recursive):**
```bash
narrrail validate scripts/
```

**Directory (recursive):**
```bash
narrrail validate scripts/ --recursive
```

**Aliases:**
- `-r`, `--recursive`: Recursively validate all scripts in subdirectories

## Validation Rules

### Errors (must fix):
- Missing required fields (meta, storyId, entryNodeId)
- Duplicate node IDs
- Duplicate variable names
- Invalid node types
- Invalid variable types
- Invalid operators
- Broken references (edges, choices, jumps)
- Entry node does not exist

### Warnings (should fix):
- Orphaned nodes (not reachable from entry)
- Potential cycles
- Empty text keys in dialogue nodes

## Exit Codes

- `0`: Validation passed (no errors)
- `1`: Validation failed (has errors) or fatal error

## Examples

### Valid Script

```yaml
meta:
  schemaVersion: 1
  storyId: demo
  entryNodeId: N_Start

variables:
  - name: Affinity
    type: Int
    scope: Session
    defaultValue: "0"

nodes:
  - nodeId: N_Start
    nodeType: Dialogue
    dialogue:
      speakerId: Hero
      textKey: line_start
      speechRate: 1.0
      voiceAsset: ""
    choices: []
    jumpTargetNodeId: ""
    enterActions: []
    exitActions: []
  
  - nodeId: N_End
    nodeType: End
    dialogue: {}
    choices: []
    jumpTargetNodeId: ""
    enterActions: []
    exitActions: []

edges:
  - sourceNodeId: N_Start
    targetNodeId: N_End
    priority: 0
    condition:
      logic: All
      terms: []
```

### Output

```
Validating: demo.narrrail.yaml

✓ Validation passed
```

## Development

### Project Structure

```
src/NarrRail.Tooling/
  Models/           # Data models (StoryScript, StoryNode, etc.)
  Parsing/          # YAML parser
  Validation/       # Validation logic
  Program.cs        # CLI entry point
```

### Dependencies

- **YamlDotNet**: YAML parsing
- **System.CommandLine**: CLI framework

## Future Features

- `import`: Import YAML to UE asset format
- `export`: Export UE asset to YAML
- `roundtrip`: Test import/export consistency
- `l10n extract`: Extract localization keys
