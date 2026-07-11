# Structured Script View Prototype

Issue: #43

## Prototype Question

Can a Narrative Creator edit a `.nrstory` through a line-based Script Editor View while preserving the same structured nodes and edges used by Graph View?

Prototype answer: yes for the current node vocabulary, as a constrained row editor over the existing `.nrstory` graph. The view should not become free text and should not introduce a parallel `.nrscript` format.

## Working Model

Structured Script View is a peer Script Editor View over one story document:

- Graph View edits `nodes` and `edges` spatially.
- Structured Script View edits a derived row model.
- Saving writes the same `.nrstory` nodes and edges.
- Validation runs against the same story document and Project Review Queue items can be shown inline by `nodeId` or `edgeId`.

The first prototype row model is implemented in `NarrRailEditor/src/core/structured-script-view.js` as `createStructuredScriptRows()`. It proves that representative `.nrstory` structures can become stable rows without UI state, browser APIs, or storage decisions.

## Representative Rows

```text
001  Hero: 我们到了。
002  Hero: 先观察一下。
003  [旁白] 走廊尽头的警示灯正在闪烁。
004  [选择] SinglePass
005    - 撬开门 -> N_Condition
006    - 呼叫支援 -> N_Set
007  [条件] N_Condition
008    if 有门禁卡: HasKeycard == true -> N_Event
009    else -> N_Jump
010  [变量] AlarmLevel Add 1
011  [事件] DoorOpened { doorId: "A-17" }
012  [跳转] N_Jump -> N_End
013  [结束]
```

Row kinds:

- `dialogue`: speaker-bound dialogue line from `Dialogue` or `MultiDialogue`.
- `narration`: dialogue-compatible action/narration row when speaker is empty.
- `choice`: choice group row.
- `choice-option`: one row per `choices[]` item, linked through `choice-N` edges.
- `choice-complete`: exhaustive-choice completion row, linked through `choice-complete`.
- `condition`: condition group row.
- `condition-branch`: one row per `condition.branches[]`, linked through `condition-N` edges.
- `condition-fallback`: else row, linked through `condition-fallback`.
- `variable-operation`: `SetVariable` node action row.
- `event`: `EmitEvent` row.
- `jump`: explicit jump row.
- `end`: terminal row.

## Insert, Reorder, Group, Fold

Insertion is type-aware, not free text:

- Pressing Enter after a dialogue row inserts another dialogue row in the same speaker block.
- Slash/add-row menu inserts explicit row types: Dialogue, Narration, Choice, Condition, Variable, Event, Jump, End, Review Note.
- Inserting Choice or Condition creates a group row plus at least one child branch row.
- Inserting a row inside a branch targets that branch's subpath.

Reordering is constrained by graph semantics:

- Moving linear dialogue/narration/event/variable rows in a single path rewires ordinary next edges.
- Moving a `choice-option` changes option order and `choice-N` handles.
- Moving a `condition-branch` changes branch order and `condition-N` handles.
- Moving a row across branch boundaries requires an explicit drop target, because it changes graph topology.

Grouping and folding:

- Choice and Condition rows are foldable groups.
- Multi-line dialogue can be displayed as a folded speaker block, but each line remains addressable.
- Branch bodies can fold under their option or condition row once the UI supports nested path rendering.
- Review Notes should fold with the node or branch they annotate.

## Synchronization With Graph View

The synchronization rule is replace-the-derived-view:

1. `.nrstory` parses into editor `nodes` and `edges`.
2. `createStructuredScriptRows({ nodes, edges, reviewItems })` derives rows.
3. Row edits mutate the same `nodes` and `edges`.
4. Graph View re-renders from those updated `nodes` and `edges`.
5. Export serializes the same `.nrstory`.

Graph View remains the clearest topology editor. Structured Script View should optimize authoring flow for line-by-line writing and branch review, while escalating complex rewires to a constrained action instead of pretending they are plain text edits.

## Validation And Review

Inline validation should reuse existing validation and Project Review Queue outputs:

- Node-targeted review items attach to every row derived from that node.
- Edge-targeted review items attach to the row that owns the relevant branch handle.
- Project-level warnings remain in Project Review Queue and can appear in a top banner.
- Rows with errors block export in the same way Graph View validation blocks export.

Example inline presentation:

```text
005    - 撬开门 -> N_Condition
       ! warning: 这个选择需要补充失败反馈
```

## Schema Support

Current `.nrstory` can support the first Structured Script View prototype:

- `Dialogue` and `MultiDialogue` cover dialogue lines.
- Empty speaker dialogue can represent narration/action in the prototype.
- `Choice` plus `choice-N` edges supports branch option rows.
- `Condition` plus `condition-N` and `condition-fallback` edges supports condition rows.
- `SetVariable`, `EmitEvent`, `Jump`, and `End` map directly to explicit row kinds.

## Schema Pressure

The prototype reveals these follow-up decisions:

- Narration/action deserves either a first-class node type or a documented convention. Empty speaker dialogue works, but the intent is implicit.
- Review Note is not currently a `.nrstory` node type. It should likely remain a Project Review Queue/import-review annotation unless creators need persistent story comments.
- Branch body ordering is graph-derived, not stored as a row outline. This is acceptable for now, but complex nested branches may need a deterministic path linearization rule.
- Edge-targeted review items need a stable mapping from `edgeId` or source handle to row id if inline branch validation becomes first-class.
- Row ids derived from `nodeId` plus row role are good enough for display, but collaborative editing may need persistent row identity later.

## Next Implementation Slice

The next production slice should not build a full editor immediately. It should add a read-only Structured Script View tab backed by `createStructuredScriptRows()`, then add editing one row kind at a time:

1. Read-only rows for current story.
2. Dialogue/narration text edit.
3. Choice option edit and reorder.
4. Condition branch display/edit.
5. Inline validation attachment from Project Review Queue.
