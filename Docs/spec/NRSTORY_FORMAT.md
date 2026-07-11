# NarrRail Neutral Story Format Contract v1

本文档定义 NarrRail 主仓库拥有的中立故事格式契约。它面向 NarrRail 创作端、Script Conversion、Import Package Review、Project Preview，以及所有下游 Story Consumer。

主仓库拥有 `.nrstory`、GlobalConfig、`.nroutline` 的格式语义。Story Consumer 可以读取这些格式并映射到自己的运行时，但必须声明兼容版本，不能把平台专用概念反向定义为格式语义。

## 1. 文件类型

NarrRail Story Project 使用以下文件类型：

| 文件类型 | 后缀 | 说明 |
| --- | --- | --- |
| Story Script | `.nrstory` | 单个互动剧情脚本，包含节点和边 |
| GlobalConfig | `.nrstory` | 项目级变量和预设角色配置，使用 `meta.configType: GlobalConfig` 标识 |
| Story Outline | `.nroutline` | 项目级故事编排，引用多个 `.nrstory` |
| Legacy Outline | `.nrrail` | 旧总纲后缀，只用于兼容读取或迁移 |

当前基准序列化格式为 YAML。新文件应优先使用 `.nrstory` 和 `.nroutline`，不应再新增 `.nrrail`。

## 2. 版本

| 契约 | 字段 | 当前版本 | 兼容要求 |
| --- | --- | --- | --- |
| Story Script | `meta.schemaVersion` | `1` | Authoring 端可以读写；Consumer 必须显式声明支持 |
| GlobalConfig | `meta.schemaVersion` + `meta.configType: GlobalConfig` | `1` | 变量和预设角色语义由主仓库定义 |
| Story Outline | `meta.schemaVersion` | `1` | 当前主要由 authoring product 使用；Consumer 支持情况由兼容矩阵声明 |

格式变更必须通过独立 schema migration 或 compatibility ticket 记录。仅新增平台同步、资产映射或 UI 能力，不构成格式版本变更。

## 3. Story Script 根结构

```yaml
meta:
  schemaVersion: 1
  storyId: demo_story
  entryNodeId: N_Start

variables: []
nodes: []
edges: []
```

必需字段：

- `meta`
- `nodes`
- `edges`

`variables` 在 Story Script 中为保留字段，当前必须为空数组或省略。变量定义统一写在 GlobalConfig 文件中。

### 3.1 Meta

| 字段 | 类型 | 必需 | 说明 |
| --- | --- | --- | --- |
| `schemaVersion` | int | 是 | 当前值为 `1` |
| `storyId` | string | 是 | Story Project 内唯一剧情 ID |
| `entryNodeId` | string | 是 | 必须存在于 `nodes[].nodeId` |

## 4. GlobalConfig

GlobalConfig 也是 `.nrstory` 文件，通过 `meta.configType: GlobalConfig` 标识。

```yaml
meta:
  schemaVersion: 1
  configType: GlobalConfig

variables:
  - name: Affinity
    type: Int
    scope: Global
    defaultValue: "0"

presetSpeakers:
  - speakerId: Hero
    displayName: 主角
    color: "#7c3aed"
```

推荐文件名：

- `globalconfig.nrstory`
- `global-config.nrstory`

GlobalConfig 不需要 `nodes` 和 `edges`。Story Consumer 可以把 GlobalConfig 映射成运行时配置资产、内存配置或平台对象，但不得改变变量和预设角色字段语义。

### 4.1 变量定义

| 字段 | 类型 | 必需 | 说明 |
| --- | --- | --- | --- |
| `name` | string | 是 | 在 GlobalConfig 中唯一 |
| `type` | enum | 是 | `Bool` / `Int` / `Float` / `String` |
| `scope` | enum | 是 | 当前使用 `Global` |
| `defaultValue` | string | 否 | 按变量类型解析 |

变量引用统一使用：

```yaml
variable:
  name: Affinity
  type: Int
  scope: Global
```

## 5. Story Script 节点

节点基础结构：

```yaml
nodes:
  - nodeId: N_Start
    nodeType: Dialogue
    dialogue:
      speakerId: Hero
      textKey: line_001
      speechRate: 1.0
      voiceAsset: ""
    choices: []
    enterActions: []
    exitActions: []
```

支持的 `nodeType`：

- `Dialogue`
- `MultiDialogue`
- `Choice`
- `Jump`
- `SetVariable`
- `EmitEvent`
- `Condition`
- `End`

### 5.1 Dialogue

```yaml
dialogue:
  speakerId: Hero
  textKey: line_001
  speechRate: 1.0
  voiceAsset: ""
```

`speakerId` 可引用 GlobalConfig 中的 `presetSpeakers[].speakerId`。`textKey` 是对白文本键；本契约不规定本地化存储实现。

### 5.2 MultiDialogue

```yaml
multiDialogue:
  speakerId: Narrator
  lines:
    - textKey: line_001
    - textKey: line_002
```

- `speakerId` 可空，空值表示旁白。
- `lines` 至少 1 行。
- 每次推进显示一行；最后一行完成后的下一次推进才离开节点。

### 5.3 Choice

```yaml
choices:
  - textKey: option_yes
    targetNodeId: N_Yes
  - textKey: option_no
    targetNodeId: N_No
choiceMode: SinglePass
choiceCompletionTargetNodeId: ""
choiceTimer:
  enabled: true
  durationSeconds: 8
  timeoutChoiceTextKey: option_timeout
```

- Choice 选项不承载可见性条件；条件分流应由 Choice 前的 Condition 节点表达。
- `choiceMode` 可为 `SinglePass` 或 `ExhaustiveUntilComplete`。
- `ExhaustiveUntilComplete` 需要 `choiceCompletionTargetNodeId`。
- `choiceTimer` 可省略，省略等同于 `enabled: false`。
- 启用倒计时时，Choice 节点额外拥有 `sourceHandle: choice-timeout` 出口。
- 手动选择优先于倒计时。

### 5.4 Condition

```yaml
condition:
  branches:
    - label: HighAffinity
      logic: All
      terms:
        - variable:
            name: Affinity
            type: Int
            scope: Global
          operator: ">="
          compareValue: "10"
```

- `branches` 从上到下依次求值。
- 命中第一个满足条件的分支后立即离开 Condition 节点。
- 分支 `0` 对应 `sourceHandle: condition-0`，分支 `1` 对应 `condition-1`，以此类推。
- 没有分支命中时走 `sourceHandle: condition-fallback`。
- 旧的单条件 `condition.terms` 可以作为兼容读取结构，但新文件应使用 `condition.branches`。

### 5.5 EmitEvent

```yaml
eventType: inventory.add_item
params:
  itemId: key_red
  count: 1
```

`eventType` 必须非空。`params` 如果存在，必须是 object/dictionary。事件含义由 Story Project 和 Consumer 约定，但字段结构由本契约约束。

### 5.6 SetVariable 与动作

节点的 `enterActions`、`exitActions` 和 `SetVariable` 节点使用同一动作结构：

```yaml
enterActions:
  - actionType: Add
    variable:
      name: Affinity
      type: Int
      scope: Global
    value: "2"
```

| 字段 | 类型 | 必需 | 说明 |
| --- | --- | --- | --- |
| `actionType` | enum | 是 | `Set` / `Add` / `Subtract` / `EmitEvent` |
| `variable` | object | Set/Add/Subtract 需要 | 变量引用 |
| `value` | string | Set/Add/Subtract 需要 | 输入值 |
| `eventType` | string | EmitEvent 需要 | 结构化事件类型 |
| `params` | object | 否 | EmitEvent 参数字典，默认 `{}` |

## 6. 边

```yaml
edges:
  - sourceNodeId: N_Start
    targetNodeId: N_Choice
    priority: 0
    sourceHandle: ""
```

| 字段 | 类型 | 必需 | 说明 |
| --- | --- | --- | --- |
| `sourceNodeId` | string | 是 | 必须存在于 `nodes` |
| `targetNodeId` | string | 是 | 必须存在于 `nodes` |
| `priority` | int | 否 | 数值越小优先级越高 |
| `sourceHandle` | string | 否 | Choice/Condition/Branch 等多出口节点使用 |

`edges[].condition` 已废弃。条件判断必须由 Condition 节点表达。

## 7. Story Outline `.nroutline`

`.nroutline` 是 Story Project 编排层。单个 `.nrstory` 负责对白、选择、变量操作和局部条件；`.nroutline` 负责多个脚本的顺序、分支与汇合关系。

```yaml
meta:
  schemaVersion: 1
  railId: main_story
  title: 主线总纲
  entryNodeId: rail_start

nodes: []
edges: []
```

支持的总纲节点：

- `Story`：引用已有 `.nrstory` 的 `meta.storyId`
- `Branch`：根据全局变量快照选择下一条总纲路径
- `Note`：章节标记或备注，预览时记录提示后自动继续
- `End`：总纲结束

Branch 节点使用与 Condition 相同的条件项语义，出口句柄为 `branch-0`、`branch-1`、`branch-fallback`。

## 8. 校验期望

硬错误：

- 空或重复的 `nodeId`。
- `entryNodeId` 不存在。
- 边引用不存在的节点。
- Choice 目标引用不存在。
- Choice 倒计时配置无效。
- GlobalConfig 变量名为空或重复。
- 变量动作引用不存在的变量。
- `SetVariable` 节点没有变量动作。
- Condition/Branch 出边缺失或重复 `sourceHandle`。
- `condition-N` 或 `branch-N` 没有对应分支。
- 出现旧的 `edges[].condition`。

警告：

- 孤立或不可达节点。
- Condition/Branch 缺少 fallback。
- 条件分支内部存在明显矛盾。
- 多个分支可能同时满足，运行语义会命中第一个满足分支。
- `.nroutline` 引用的 `storyId` 无法在当前 Story Project 中解析。

## 9. 导入、导出与兼容

Authoring 端导出要求：

- 导出的新文件必须使用当前中立契约字段语义。
- 不应导出 `edges[].condition`。
- 不应把平台专用字段写入 `.nrstory`、GlobalConfig 或 `.nroutline`。
- 必须保留 Story Project 审查流程需要识别的结构错误和警告。

导入要求：

- 可读取当前版本文件。
- 可兼容读取已明确声明的 legacy 结构。
- 对高于支持范围的 `schemaVersion` 必须提示不兼容，而不是静默按当前版本解析。
- 对未知字段应尽量保留；无法保留时必须把丢弃行为作为转换/导入备注暴露给用户。

## 10. Story Consumer compatibility

Story Consumer 是读取 NarrRail 输出并在下游运行、展示或校验故事内容的实现。Consumer 包括 [`NarrRail-Unreal-Plugin`](https://github.com/Courtshipfy/NarrRail-Unreal-Plugin)。

Consumer 必须声明：

- 支持的 Story Script `schemaVersion`。
- 支持的 GlobalConfig `schemaVersion`。
- 是否支持 `.nroutline`。
- 是否保留未知字段。
- 不支持的节点类型、动作类型、事件约定或计时能力。

Consumer 可以：

- 把 `.nrstory` 映射成运行时资产或内存结构。
- 把 GlobalConfig 映射成平台配置对象。
- 忽略自身不消费的 authoring-only 信息。

Consumer 不得：

- 把平台资产路径、同步路径、Blueprint/API、PIE、HUD、Content 目录等细节写入中立格式契约。
- 私自扩展字段语义并要求主仓库按该平台解释。
- 静默接受无法正确执行的更高 schema 版本。
