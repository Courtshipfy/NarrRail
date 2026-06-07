# `.nrstory` 可读 YAML v2 整改方案草案

## 1. 背景

当前 `.nrstory` 使用 YAML，但结构以编辑器图数据为中心：

```yaml
meta:
  schemaVersion: 1
  storyId: demo
  entryNodeId: N_Start
variables: []
nodes: []
edges: []
```

这种结构适合编辑器、UE 导入和离线校验，但对人工局部修改不友好。常见问题：

- 对白正文和跳转关系分散在 `nodes` / `edges` / `choices[].targetNodeId` 中。
- 手改一个分支时，经常需要同时修改节点、边、句柄。
- 文件阅读顺序不等于剧情阅读顺序。
- 节点坐标、sourceHandle 等编辑器概念暴露在正文附近，干扰编剧阅读。
- `textKey` 命名更像本地化 key，不适合直接承载正文文本。

本方案目标是新增一种更接近 Ren'Py `.rpy` 阅读体验的 `.nrstory` YAML v2 格式，同时不破坏现有 v1 文件。

## 2. 已确认设计选择

1. 文件继续使用 YAML，不引入自定义 DSL。
2. 允许保留编辑器元数据，但必须和正文分离。
3. 条件表达式使用短字符串形式，例如 `trust >= 3`。
4. v2 正文使用 `text` 字段，不再优先使用 `textKey`。

## 3. 目标

### 3.1 人类可读

剧情主体应按流程组织，局部修改对白、选项、跳转时，不需要理解完整 graph 数据结构。

### 3.2 编辑器可往返

编辑器导入 v2 后仍能生成现有 graph model。编辑器导出 v2 后，文件应保持稳定、可读、可 diff。

### 3.3 运行时语义不漂移

v2 只改变作者编辑格式，不改变 NarrRail 现有运行语义：

- Dialogue / MultiDialogue / Choice / Jump / SetVariable / EmitEvent / Condition / End 节点仍然存在。
- Choice 条件仍不放在选项内，复杂条件仍通过 Condition 节点表达。
- Condition 分支仍按顺序匹配，命中第一个满足分支。

### 3.4 v1 兼容

现有 `schemaVersion: 1` 文件继续可导入。v2 上线初期提供 v1/v2 双格式导出能力。

## 4. v2 根结构

建议根结构：

```yaml
meta:
  schemaVersion: 2
  storyId: demo_story
  entry: start

characters:
  alice: 爱丽丝
  bob: 鲍勃

variables:
  trust:
    type: Int
    default: 0
    scope: Session

story:
  start:
    say: alice
    text: 你好。
    next: ask_name

  ask_name:
    choice:
      - text: 你是谁？
        goto: who_are_you
      - text: 先离开
        goto: end

  who_are_you:
    lines:
      - alice: 我是爱丽丝。
      - bob: 我知道你会来。
    next: check_trust

  check_trust:
    when:
      - if: trust >= 3
        goto: good_end
      - else:
        goto: bad_end

  good_end:
    say: alice
    text: 我相信你。
    end: true

  bad_end:
    say: alice
    text: 现在还不行。
    end: true

_editor:
  layout:
    start: { x: 80, y: 120 }
    ask_name: { x: 360, y: 120 }
    who_are_you: { x: 640, y: 120 }
    check_trust: { x: 900, y: 120 }
```

说明：

- `meta.schemaVersion: 2` 用于区分 v1 图结构和 v2 可读结构。
- `meta.entry` 替代 v1 的 `entryNodeId`。
- `story` 是正文主体，key 即节点 ID。
- `_editor` 是可选编辑器元数据；用户可删除，删除后编辑器自动布局。
- `characters` 是可选显示名映射，不直接影响运行跳转。
- `variables` 暂定允许出现在剧情文件中，但需要和现有全局配置机制协调，见第 10 节。

## 5. 节点写法

### 5.1 Dialogue

```yaml
intro:
  say: alice
  text: 今天雨很大。
  next: choice_1
```

转换为 v1 graph model：

- `nodeId`: `intro`
- `nodeType`: `Dialogue`
- `dialogue.speakerId`: `alice`
- `dialogue.textKey`: 暂存为正文文本，编辑器内部仍可沿用 `textKey` 字段，导出 v2 时写回 `text`
- 普通边：`intro -> choice_1`

可选字段：

```yaml
intro:
  say: alice
  text: 今天雨很大。
  speechRate: 1.0
  voiceAsset: voice/alice/rain_001
  next: choice_1
```

### 5.2 MultiDialogue

推荐写法：

```yaml
scene_1:
  lines:
    - alice: 你来了。
    - bob: 我迟到了吗？
    - alice: 还不算。
  next: next_node
```

兼容旁白：

```yaml
scene_1:
  lines:
    - 远处传来钟声。
    - alice: 时间到了。
  next: next_node
```

转换规则：

- 有 `speaker: text` 的行转换为带 speaker 的台词。
- 纯字符串行作为旁白。
- 当前编辑器内部 `MultiDialogue` 只有统一 `speakerId + lines[].textKey`，如果要完整支持逐行 speaker，需要同步扩展编辑器数据模型；否则第一阶段只能导入为 `speakerId: ""` 的多行文本，逐行 speaker 需要降级处理。

建议第一阶段：

- 支持纯字符串行和统一 speaker 行。
- 对混合 speaker 行给出导入警告，或先拆成多个 Dialogue 节点。

### 5.3 Choice

```yaml
door_choice:
  choice:
    - text: 打开门
      goto: open_door
    - text: 离开
      goto: leave
```

穷举模式：

```yaml
topic_choice:
  choiceMode: ExhaustiveUntilComplete
  choice:
    - text: 询问过去
      goto: ask_past
    - text: 询问目的
      goto: ask_goal
  complete: after_all_topics
```

转换规则：

- 每个选项生成 `choices[index].textKey = text`。
- 每个选项生成一条 `sourceHandle: choice-{index}` 的边。
- `complete` 生成 `sourceHandle: choice-complete` 的边。

### 5.4 Jump

```yaml
skip_intro:
  goto: main_scene
```

转换为 `Jump` 节点。

注意：如果一个普通节点同时有正文和 `next`，不视为 Jump，而是当前节点的普通出边。

### 5.5 SetVariable

推荐可读写法：

```yaml
gain_trust:
  set:
    trust: trust + 1
  next: continue_scene
```

也允许显式动作写法：

```yaml
gain_trust:
  action:
    type: Add
    variable: trust
    value: 1
  next: continue_scene
```

第一阶段建议优先实现显式动作写法，因为它更容易无歧义转换到现有 `SetVariable` 节点：

```yaml
gain_trust:
  action:
    type: Add
    variable: trust
    value: "1"
  next: continue_scene
```

表达式写法 `trust: trust + 1` 可作为第二阶段增强。

### 5.6 EmitEvent

```yaml
play_sound:
  emit: DoorKnock
  params:
    volume: 0.8
  next: continue_scene
```

当前 v1 `EmitEvent` 只导出 `eventId`，如果要保留 `params`，需要同步扩展 v1 兼容结构或编辑器内部数据。

第一阶段建议：

- 支持 `emit`。
- `params` 先作为保留字段，导入后存在节点 data 中，但 UE 侧是否消费另行确认。

### 5.7 Condition

```yaml
check_trust:
  when:
    - if: trust >= 3
      goto: trusted_path
    - if: trust < 0
      goto: hostile_path
    - else:
      goto: neutral_path
```

转换规则：

- `when[]` 顺序对应 v1 `condition.branches[]` 顺序。
- `if` 字符串解析成结构化 terms。
- `else` 转换为 `condition-fallback` 出边，不生成 branch。
- 第 0 个 `if` 对应 `sourceHandle: condition-0`。
- 第 1 个 `if` 对应 `sourceHandle: condition-1`。

支持的第一阶段表达式：

```text
<variable> <operator> <literal>
```

允许 operator：

- `==`
- `!=`
- `>`
- `>=`
- `<`
- `<=`

literal 类型：

- number：`3`、`-1`、`0.5`
- bool：`true`、`false`
- string：建议使用引号，如 `"open"`

第一阶段不支持：

- 复杂算术：`trust + courage >= 5`
- 括号
- `and` / `or` 混合表达式
- 函数调用
- JS 执行

需要多个条件时，使用结构化写法：

```yaml
check_state:
  when:
    - all:
        - trust >= 3
        - has_key == true
      goto: unlock
    - any:
        - alarm == true
        - suspicion > 5
      goto: danger
    - else:
      goto: normal
```

转换：

- `all` -> `logic: All`
- `any` -> `logic: Any`

### 5.8 End

```yaml
end:
  end: true
```

转换为 `End` 节点。

## 6. 编辑器元数据 `_editor`

建议结构：

```yaml
_editor:
  layout:
    node_id: { x: 100, y: 200 }
  edgeStyle: straight
  focusMode: false
```

规则：

- `_editor` 可选。
- `_editor.layout` 中不存在的节点自动布局。
- `story` 中不存在但 `_editor.layout` 中存在的节点 ID 应忽略，并在导入提示中报告。
- 编辑器导出时默认保留 layout，减少每次导入后节点位置变化。
- 如果用户手动删除 `_editor`，编辑器不应报错。

## 7. v1/v2 转换策略

### 7.1 v2 -> 编辑器 graph model

导入流程：

1. YAML.parse。
2. 根据 `meta.schemaVersion` 判断格式。
3. 如果是 v1，走现有 importer。
4. 如果是 v2：
   - 遍历 `story`。
   - 根据字段判断节点类型。
   - 生成编辑器 nodes。
   - 根据 `next` / `goto` / `choice[].goto` / `when[].goto` 生成 edges。
   - 应用 `_editor.layout`。
   - 运行现有 validation。

节点类型判定优先级：

1. `end: true` -> End
2. `choice` -> Choice
3. `when` -> Condition
4. `action` / `set` -> SetVariable
5. `emit` -> EmitEvent
6. 仅 `goto` -> Jump
7. `lines` -> MultiDialogue
8. `say` / `text` -> Dialogue

### 7.2 编辑器 graph model -> v2

导出流程：

1. 按 graph model nodes 生成 `story` map。
2. 普通边写入 `next`。
3. Choice 分支边写入 `choice[].goto`。
4. Condition 分支边写入 `when[].goto`。
5. 节点 position 写入 `_editor.layout`。
6. 生成稳定字段顺序。

稳定字段顺序建议：

```yaml
meta
characters
variables
story
_editor
```

单个节点字段顺序建议：

```yaml
say
text
lines
choiceMode
choice
complete
when
action
emit
params
goto
next
end
```

## 8. 校验规则

v2 导入前校验：

- `meta.schemaVersion` 必须为 `2`。
- `meta.storyId` 必须存在。
- `meta.entry` 必须存在于 `story`。
- `story` 必须是 map。
- 每个 story key 必须是合法 node ID。
- 所有 `next` / `goto` / `choice[].goto` / `when[].goto` 必须引用存在节点。
- `choice` 必须是非空数组。
- `choice[].text` 必须非空。
- `choiceMode: ExhaustiveUntilComplete` 时必须提供 `complete`。
- `when` 必须包含至少一个 `if` / `all` / `any` 分支。
- `else` 分支最多一个，并且建议放在最后。
- 条件表达式必须符合白名单语法。

导出前校验：

- 沿用现有图校验。
- 如果 graph 中存在 v2 无法清晰表达的结构，应提示用户或降级导出为 v1。

## 9. 兼容与迁移

### 9.1 不破坏 v1

- v1 importer/exporter 保留。
- UE 插件可继续吃 v1。
- v2 上线初期，编辑器可以导入 v2，但默认导出策略需要讨论。

### 9.2 导出选项

建议提供：

- `导出 .nrstory（可读 v2）`
- `导出兼容 .nrstory（v1）`

短期默认建议：

- 如果打开的是 v1 文件，默认继续导出 v1。
- 如果打开的是 v2 文件，默认导出 v2。
- 新建文件默认 v2，但需要确认 UE 支持节奏。

### 9.3 UE 支持路线

阶段 1：

- 编辑器支持 v2。
- UE 仍使用 v1。
- 用户需要时从编辑器导出 v1 给 UE。

阶段 2：

- UE 插件增加 v2 parser，直接导入 v2。
- v1 作为兼容格式保留。

## 10. 变量策略待确认

现有 `SCRIPT_FORMAT.md` 中说明剧情脚本不再定义变量，变量定义统一写在全局配置文件中。但当前编辑器代码仍保留 `variables` 导入导出能力。

v2 有两种选择：

### 方案 A：剧情文件允许局部变量定义

```yaml
variables:
  trust:
    type: Int
    default: 0
    scope: Session
```

优点：

- 对单文件写作友好。
- 手写故事时不需要同时打开 global config。

缺点：

- 和现有“变量统一写全局配置”的规范冲突。
- UE 侧需要明确变量来源合并规则。

### 方案 B：剧情文件只引用变量

```yaml
uses:
  variables:
    - trust
```

优点：

- 和现有运行规范一致。
- UE 侧更简单。

缺点：

- 手写体验稍差。
- 单个 `.nrstory` 不够自解释。

建议：

- 第一阶段采用方案 B，保持运行规范一致。
- 编辑器可在导入 v2 时允许 `variables` 字段作为草稿能力，但导出时根据项目设置决定是否写出。

## 11. 实施步骤

### 阶段 0：文档定稿

- 确认本文档中的字段命名。
- 确认变量策略。
- 确认 MultiDialogue 是否支持逐行 speaker。
- 确认 `EmitEvent.params` 是否进入正式格式。

### 阶段 1：纯转换器

新增独立工具模块：

```text
NarrRailEditor/src/utils/nrstory-v2-importer.js
NarrRailEditor/src/utils/nrstory-v2-exporter.js
NarrRailEditor/src/utils/nrstory-expression-parser.js
```

要求：

- 不先改 UI。
- 先用单元样例验证 v2 -> graph model -> v2。
- 表达式 parser 只做白名单解析，不执行代码。

### 阶段 2：编辑器导入支持

- `importFromYAML` 根据 `meta.schemaVersion` 分派 v1/v2。
- v2 导入后复用现有 graph editor。
- `_editor.layout` 应用到节点位置。
- 缺失 layout 时自动布局。

### 阶段 3：编辑器导出支持

- 增加 v2 导出函数。
- UI 增加导出选项。
- 默认导出策略按第 9.2 节执行。

### 阶段 4：文档与 AI 写作规范

- 新增 `SCRIPT_FORMAT_V2.md` 或更新 `SCRIPT_FORMAT.md`。
- 新增 `NRSTORY_AI_WRITING_SPEC_V2.md`。
- 保留 v1 AI 规范，避免旧工具误生成 v2。

### 阶段 5：UE 支持评估

- 评估 UE 插件直接解析 v2 的成本。
- 如果成本可控，新增 v2 parser。
- 如果短期不做 UE v2，编辑器继续提供 v1 兼容导出。

## 12. 测试样例

需要至少覆盖：

1. 单 Dialogue 链。
2. MultiDialogue。
3. Choice 单选。
4. Choice `ExhaustiveUntilComplete`。
5. Jump。
6. SetVariable 显式 action。
7. EmitEvent。
8. Condition 单分支 + else。
9. Condition all / any。
10. 缺失 `_editor` 自动布局。
11. `_editor.layout` 包含无效节点 ID。
12. v1 文件导入不回归。

## 13. 风险

- v2 可读格式和图编辑器的自由连线能力不完全等价。某些复杂 graph 可能导出成 v2 时不够自然。
- 条件表达式如果设计过宽，会引入 parser 和安全风险。
- `text` 与现有 `textKey` 字段语义需要明确：是正文文本，还是本地化 key。
- MultiDialogue 逐行 speaker 是体验提升点，但会触及现有数据模型。
- UE 插件如果短期不支持 v2，团队需要明确“可读格式”和“运行格式”的导出流程。

## 14. 建议结论

建议采用：

```text
.nrstory YAML v2 + story 主体 + _editor 元数据 + v1/v2 双兼容
```

第一阶段不要实现完整 Ren'Py DSL，也不要允许任意表达式。先把 `.nrstory` 从“图数据文件”改造成“可手写剧情文件”，同时保留编辑器稳定往返能力。

