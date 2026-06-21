# NarrRail `.nrstory` Format

Generate YAML compatible with schema version 1:

```yaml
meta:
  schemaVersion: 1
  storyId: demo_story
  entryNodeId: N_Start

variables: []
nodes: []
edges: []
```

## Supported Node Types

- `Dialogue`
- `MultiDialogue`
- `Choice`
- `Jump`
- `SetVariable`
- `EmitEvent`
- `Condition`
- `End`

## Dialogue

```yaml
- nodeId: N_Start_001
  nodeType: Dialogue
  dialogue:
    speakerId: Alice
    textKey: 你好。
    speechRate: 1
    voiceAsset: ""
```

## MultiDialogue

Use when multiple consecutive lines belong to the same speaker.

```yaml
- nodeId: N_Start_002
  nodeType: MultiDialogue
  multiDialogue:
    speakerId: Alice
    lines:
      - textKey: 第一行。
      - textKey: 第二行。
```

## Choice

```yaml
- nodeId: N_Start_Choice
  nodeType: Choice
  choices:
    - textKey: 询问发生了什么
      targetNodeId: N_Ask_001
    - textKey: 保持沉默
      targetNodeId: N_Silent_001
  choiceMode: SinglePass
  choiceCompletionTargetNodeId: ""
  choiceTimer:
    enabled: false
    durationSeconds: 8
    timeoutChoiceTextKey: 超时
```

Matching edges:

```yaml
- sourceNodeId: N_Start_Choice
  sourceHandle: choice-0
  targetNodeId: N_Ask_001
  priority: 0
```

## Required Rules

- `meta.schemaVersion` must be `1`.
- `meta.storyId` is required.
- `meta.entryNodeId` must reference an existing node.
- Node IDs must be unique.
- Every edge source and target must reference an existing node.
- Do not generate `edges[].condition`.
- Use `Condition` nodes for conditional branches.
- End branches with an `End` node unless the story intentionally jumps elsewhere.

