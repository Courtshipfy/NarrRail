# NRSTORY_AI_WRITING_SPEC.md

> 用途：给“写故事的 AI”直接生成可导入的 `.nrstory` 文件。  
> 基线：NarrRail 脚本格式规范 v1（`schemaVersion: 1`）。

---

## 1. 输出要求（必须遵守）

1. 仅输出 **YAML** 内容，不要输出解释文字、Markdown 代码块、注释。
2. 根字段必须且仅包含：`meta`、`variables`、`nodes`、`edges`（可额外字段仅在需求明确允许时添加）。
3. `meta.schemaVersion` 固定为 `1`。
4. `meta.entryNodeId` 必须存在于 `nodes[].nodeId`。
5. `nodeId` 全局唯一；`variables[].name` 全局唯一。
6. 所有引用必须有效：
   - `edges.sourceNodeId/targetNodeId` 必须引用存在的 `nodeId`
   - `choices[].targetNodeId` 必须引用存在的 `nodeId`
   - `jumpTargetNodeId`（若使用）必须引用存在的 `nodeId`
7. 条件与动作中的变量引用（`name/type/scope`）必须与 `variables` 定义一致。
8. 字符串用 UTF-8。

---

## 2. 根结构模板

```yaml
meta:
  schemaVersion: 1
  storyId: demo_story
  entryNodeId: N_Start

variables: []

nodes: []

edges: []
```

---

## 3. 字段规范（AI 生成时的硬规则）

## 3.1 meta

- `schemaVersion`: int，固定 `1`
- `storyId`: string，建议蛇形或小写下划线，如 `campus_meetup`
- `entryNodeId`: string，必须是有效节点 ID

## 3.2 variables

每项结构：

```yaml
- name: Affinity
  type: Int
  scope: Session
  defaultValue: "0"
```

- `type` 仅允许：`Bool` / `Int` / `Float` / `String`
- `scope` 允许：`Session` / `Global`（默认建议 `Session`）
- `defaultValue` 始终写成字符串

## 3.3 nodes

每个节点至少包含：
- `nodeId`
- `nodeType`
- `enterActions`
- `exitActions`

建议保持字段完整（即使为空），以提高稳定性。

支持 `nodeType`：
- `Dialogue`
- `MultiDialogue`
- `Choice`
- `Jump`
- `SetVariable`
- `EmitEvent`
- `Condition`
- `End`

### A) Dialogue

```yaml
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
```

### B) MultiDialogue

```yaml
- nodeId: N_Block
  nodeType: MultiDialogue
  multiDialogue:
    speakerId: Narrator
    lines:
      - textKey: line_001
      - textKey: line_002
  dialogue: {}
  choices: []
  jumpTargetNodeId: ""
  enterActions: []
  exitActions: []
```

规则：
- `lines` 至少 1 行。
- 运行时每次 `Next` 推进一行，最后一行后再 `Next` 才离开节点。

### C) Choice

```yaml
- nodeId: N_Choice
  nodeType: Choice
  choices:
    - textKey: option_yes
      targetNodeId: N_Yes
    - textKey: option_no
      targetNodeId: N_No
  choiceMode: SinglePass
  dialogue: {}
  jumpTargetNodeId: ""
  enterActions: []
  exitActions: []
```

`choiceMode`：
- `SinglePass`（默认）
- `ExhaustiveUntilComplete`（若使用，需填写 `choiceCompletionTargetNodeId`）

规则：
- `choices[]` 不允许写 `availability`。条件控制必须在 `Choice` 节点前使用 `Condition` 节点完成。

### D) Jump

```yaml
- nodeId: N_Jump
  nodeType: Jump
  jumpTargetNodeId: N_Target
  dialogue: {}
  choices: []
  enterActions: []
  exitActions: []
```

### E) EmitEvent

旧兼容写法：

```yaml
- nodeId: N_LegacyEvent
  nodeType: EmitEvent
  eventId: some_event_id
  enterActions: []
  exitActions: []
```

推荐结构化写法：

```yaml
- nodeId: N_PlayBgm
  nodeType: EmitEvent
  eventType: audio.play_bgm
  params:
    bgmId: train_theme
    fadeSeconds: 1.5
  enterActions: []
  exitActions: []
```

规则：
- `eventId` 是兼容字段，可作为旧事件 ID、唯一标识或调试标识。
- `eventType + params` 是推荐的新结构化事件写法。
- `eventId` 和 `eventType` 至少有一个非空。
- `params` 如果存在，必须是 object/dictionary；未填写时按 `{}` 处理。

### F) Condition

```yaml
- nodeId: N_CheckAffinity
  nodeType: Condition
  condition:
    logic: All
    terms:
      - variable:
          name: Affinity
          type: Int
          scope: Session
        operator: ">="
        compareValue: "10"
  dialogue: {}
  choices: []
  jumpTargetNodeId: ""
  enterActions: []
  exitActions: []
```

规则：
- 条件判断必须放在 `Condition` 节点的 `condition` 字段。
- `Condition` 节点必须有一条 `sourceHandle: condition-true` 出边和一条 `sourceHandle: condition-false` 出边。

### G) End

```yaml
- nodeId: N_End
  nodeType: End
  dialogue: {}
  choices: []
  jumpTargetNodeId: ""
  enterActions: []
  exitActions: []
```

> `SetVariable` / `EmitEvent` 若作为独立节点使用，仍建议保留统一壳字段，并通过 `enterActions` 实现动作，以保持执行一致性。

## 3.4 edges

每项结构：

```yaml
- sourceNodeId: N_A
  targetNodeId: N_B
  priority: 0
  sourceHandle: ""
```

规则：
- `priority` 越小越先评估。
- 边不允许写 `condition`；旧的 `edges[].condition` 必须改为 `Condition` 节点。
- 普通边的 `sourceHandle` 为空。
- `Condition` 节点出边只允许 `condition-true` / `condition-false`。

---

## 4. 条件与动作

## 4.1 条件 condition

```yaml
logic: All
terms:
  - variable:
      name: Affinity
      type: Int
      scope: Session
    operator: ">="
    compareValue: "10"
```

运算符仅允许：
- `==` `!=` `>` `>=` `<` `<=`

## 4.2 动作 action

```yaml
- actionType: Add
  variable:
    name: Affinity
    type: Int
    scope: Session
  value: "2"
```

结构化事件动作示例：

```yaml
- actionType: EmitEvent
  eventType: inventory.add_item
  params:
    itemId: key_red
    count: 1
```

- `actionType` 仅允许：`Set` / `Add` / `Subtract` / `EmitEvent`
- `Set/Add/Subtract` 必须有：`variable` + `value`
- `EmitEvent` 中 `eventId` 和 `eventType` 至少有一个非空
- `eventId` 是兼容字段；推荐新写法使用 `eventType + params`
- `params` 如果存在，必须是 object/dictionary

---

## 5. AI 生成策略（推荐）

1. 先列变量，再列节点，最后连边。
2. 每条分支最终应可到达 `End`（避免悬空流程）。
3. 条件分支要有兜底边（例如同源节点下优先级更低的恒真边）。
4. 节点命名建议：`N_场景_语义`，如 `N_Cafe_Choice`。
5. `textKey` 命名建议：`scene_xxx_line_yyy` 或直接短句（取决于你的本地化流程）。

---

## 6. 常见错误清单（生成后自检）

- 重复 `nodeId`
- `entryNodeId` 不存在
- 边引用不存在节点
- 选项目标不存在
- 变量名为空或重复
- 条件/动作变量类型与变量表不一致
- Choice 节点无可用选项且无后续流程

---

## 7. 最小可用故事模板（可直接改）

```yaml
meta:
  schemaVersion: 1
  storyId: first_story
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

  - nodeId: N_Choice
    nodeType: Choice
    dialogue: {}
    choices:
      - textKey: option_yes
        targetNodeId: N_Yes
      - textKey: option_no
        targetNodeId: N_No
    choiceMode: SinglePass
    jumpTargetNodeId: ""
    enterActions: []
    exitActions: []

  - nodeId: N_Yes
    nodeType: Dialogue
    dialogue:
      speakerId: Heroine
      textKey: line_yes
      speechRate: 1.0
      voiceAsset: ""
    choices: []
    jumpTargetNodeId: ""
    enterActions:
      - actionType: Add
        variable:
          name: Affinity
          type: Int
          scope: Session
        value: "10"
    exitActions: []

  - nodeId: N_No
    nodeType: Dialogue
    dialogue:
      speakerId: Heroine
      textKey: line_no
      speechRate: 1.0
      voiceAsset: ""
    choices: []
    jumpTargetNodeId: ""
    enterActions:
      - actionType: Subtract
        variable:
          name: Affinity
          type: Int
          scope: Session
        value: "5"
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
    targetNodeId: N_Choice
    priority: 0

  - sourceNodeId: N_Yes
    targetNodeId: N_End
    priority: 0

  - sourceNodeId: N_No
    targetNodeId: N_End
    priority: 0
```

---

## 8. 给“写故事 AI”的调用提示词（可直接复用）

将下面提示词给生成模型，可显著降低格式错误：

```text
你是 NarrRail 剧情脚本生成器。请严格输出 .nrstory YAML（不要输出解释文字、不要 Markdown 代码块）。

硬性要求：
1) 根字段必须是 meta/variables/nodes/edges。
2) schemaVersion 固定为 1。
3) entryNodeId 必须存在于 nodes.nodeId。
4) nodeId 全局唯一，variables.name 全局唯一。
5) 所有边和 choice 的 target 引用必须有效。
6) 条件运算符仅可使用 == != > >= < <=。
7) 动作仅可使用 Set/Add/Subtract/EmitEvent。
8) 字段类型符合规范，defaultValue/value/compareValue 使用字符串。

剧情需求：
<在这里填你的故事设定、角色、分支数、变量规则、结局要求>

输出前自检：
- 是否有孤立分支或无终点路径
- 是否存在无效引用
- 是否存在重复 ID
```

---
