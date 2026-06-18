# NarrRail 脚本格式规范 v1

## 1. 适用范围

本文档定义 NarrRail 用于编辑器导入/导出、UE 插件导入和离线校验的脚本文件格式。

当前基准格式：YAML，统一文件后缀 `.nrstory`。

## 2. 文件类型

NarrRail 当前支持两类 `.nrstory` 文件：

- 剧情脚本：包含 `nodes` 和 `edges`，导入 UE 后生成 `UNarrRailStoryAsset`
- 全局配置：`meta.configType: GlobalConfig`，导入 UE 后生成 `UNarrRailGlobalConfigAsset`

## 3. 剧情脚本根结构

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

`variables` 在剧情脚本中为保留字段，当前必须为空数组或省略。变量定义统一写在全局配置文件中。

## 4. Meta

| 字段 | 类型 | 必需 | 说明 |
|---|---|---|---|
| `schemaVersion` | int | 是 | 当前值为 `1` |
| `storyId` | string | 剧情脚本必需 | 唯一剧情 ID |
| `entryNodeId` | string | 剧情脚本必需 | 必须存在于 `nodes[].nodeId` |
| `configType` | string | 全局配置必需 | 全局配置使用 `GlobalConfig` |

## 5. 变量与引用

剧情脚本不再定义变量，只引用全局配置中的变量。变量定义只允许出现在 `meta.configType: GlobalConfig` 的全局配置文件中。

| 字段 | 类型 | 必需 | 说明 |
|---|---|---|---|
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

## 6. 节点

基础结构：

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

## 7. Dialogue

```yaml
dialogue:
  speakerId: Hero
  textKey: line_001
  speechRate: 1.0
  voiceAsset: ""
```

## 8. MultiDialogue

```yaml
multiDialogue:
  speakerId: Narrator
  lines:
    - textKey: line_001
    - textKey: line_002
```

说明：

- `speakerId` 可空，空值表示旁白
- `lines` 至少 1 行
- 运行时每次 `Next` 推进一行，最后一行完成后的下一次 `Next` 才离开节点

## 9. Choice

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
  timeoutBehavior: SelectDefault
  defaultChoiceIndex: 0
  timeoutTargetNodeId: ""
```

说明：

- Choice 选项不再承载可见性条件
- 如需按条件进入不同选择集合，应在 Choice 前放置 Condition 节点
- `choiceMode` 可为 `SinglePass` 或 `ExhaustiveUntilComplete`
- `ExhaustiveUntilComplete` 需要 `choiceCompletionTargetNodeId`
- `choiceTimer` 为可选字段；省略时等同于 `enabled: false`
- `choiceTimer.durationSeconds` 为倒计时秒数，必须大于 0
- `choiceTimer.timeoutBehavior` 可为 `SelectDefault` 或 `JumpToNode`
- `SelectDefault` 超时时执行 `defaultChoiceIndex` 对应选项，索引从 0 开始
- `JumpToNode` 超时时直接跳转到 `timeoutTargetNodeId`
- 手动选择优先于倒计时；倒计时只在等待 Choice 输入时运行
- `ExhaustiveUntilComplete` 下，`SelectDefault` 不能指向已经选择过的分支；如果默认分支已经被选过，运行时应选择当前仍可选的第一个分支

## 10. Condition

Condition 是当前唯一支持条件分支的结构。边上的 `condition` 字段已废弃，不应再出现在导出的 `.nrstory` 中。

### 多分支结构

```yaml
- nodeId: N_CheckAffinity
  nodeType: Condition
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
      - label: LowAffinity
        logic: All
        terms:
          - variable:
              name: Affinity
              type: Int
              scope: Global
            operator: "<"
            compareValue: "0"
```

分支规则：

- `branches` 从上到下依次求值
- 命中第一个满足条件的分支后立即离开 Condition 节点
- 分支 `0` 对应出边 `sourceHandle: condition-0`
- 分支 `1` 对应出边 `sourceHandle: condition-1`
- 以此类推
- 没有分支命中时走 `sourceHandle: condition-fallback`

### Condition 边示例

```yaml
edges:
  - sourceNodeId: N_CheckAffinity
    sourceHandle: condition-0
    targetNodeId: N_SpecialEnding
    priority: 0

  - sourceNodeId: N_CheckAffinity
    sourceHandle: condition-1
    targetNodeId: N_BadEnding
    priority: 1

  - sourceNodeId: N_CheckAffinity
    sourceHandle: condition-fallback
    targetNodeId: N_NormalEnding
    priority: 2
```

校验规则：

- Condition 出边必须有 `sourceHandle`
- `condition-N` 必须对应存在的 `branches[N]`
- 同一个 Condition 节点不能有重复 `sourceHandle`
- 每个分支都应有对应出边
- 建议提供 `condition-fallback`；否则所有分支都不命中时运行时会报错

### 兼容结构

运行时仍能识别只有 `condition.terms` 的单条件结构，但新文件应使用 `condition.branches`。

单条件结构对应：

- true 分支：`condition-0`
- false/fallback 分支：`condition-fallback`

## 11. 条件项

```yaml
logic: All
terms:
  - variable:
      name: Affinity
      type: Int
      scope: Global
    operator: ">="
    compareValue: "10"
```

`logic`：

- `All`：所有条件项都满足
- `Any`：任一条件项满足

`operator`：

- `==`
- `!=`
- `>`
- `>=`
- `<`
- `<=`

## 12. 动作

```yaml
enterActions:
  - actionType: Add
    variable:
      name: Affinity
      type: Int
      scope: Global
    value: "2"
    eventId: ""
```

| 字段 | 类型 | 必需 | 说明 |
|---|---|---|---|
| `actionType` | enum | 是 | `Set` / `Add` / `Subtract` / `EmitEvent` |
| `variable` | object | Set/Add/Subtract 需要 | 变量引用 |
| `value` | string | Set/Add/Subtract 需要 | 输入值 |
| `eventId` | string | EmitEvent 需要 | 事件标识符 |

`SetVariable` 节点至少需要一个变量动作。

## 13. 边

```yaml
edges:
  - sourceNodeId: N_Start
    targetNodeId: N_Choice
    priority: 0
    sourceHandle: ""
```

| 字段 | 类型 | 必需 | 说明 |
|---|---|---|---|
| `sourceNodeId` | string | 是 | 必须存在于 `nodes` |
| `targetNodeId` | string | 是 | 必须存在于 `nodes` |
| `priority` | int | 否 | 数值越小优先级越高 |
| `sourceHandle` | string | 否 | Choice/Condition 专用出口句柄 |

禁止字段：

```yaml
edges:
  - condition: {}
```

`edges[].condition` 已废弃。条件判断必须由 Condition 节点表达。

## 14. 全局配置文件

全局配置文件示例：

```yaml
meta:
  schemaVersion: 1
  configType: GlobalConfig

variables:
  - name: Truth
    type: Bool
    scope: Global
    defaultValue: "false"

presetSpeakers:
  - speakerId: Hero
    displayName: 主角
    color: "#7c3aed"
```

说明：

- 全局配置文件也使用 `.nrstory` 后缀
- 推荐文件名：`globalconfig.nrstory` 或 `global-config.nrstory`
- UE 同步后生成 `UNarrRailGlobalConfigAsset`
- 全局配置不需要 `nodes` 和 `edges`
- 同步到同一故事仓库目录下的 `UNarrRailStoryAsset` 会自动绑定该 `UNarrRailGlobalConfigAsset`
- 运行时会把全局变量注册到 GameInstance 级 `UNarrRailGlobalStateSubsystem`，多个 Session 共享同一份全局变量状态

## 15. UE 故事仓库同步

UE 插件通过 `Sync Stories` 同步本地故事仓库：

- 配置位置：`Project Settings > Plugins > NarrRail`
- `Story Repository Path`：本地故事仓库路径
- `Pull Git Repository Before Sync`：默认开启，Git 仓库同步前执行 `git pull --ff-only`
- `Story Asset Root Path (UE Content)`：默认 `/Game/NarrRailStories`

同步目标：

```text
/Game/NarrRailStories/<故事仓库文件夹名>
```

同步行为：

- 扫描所有 `*.nrstory`
- 完整保留仓库子目录结构
- 普通剧情生成或更新 `UNarrRailStoryAsset`
- 全局配置生成或更新 `UNarrRailGlobalConfigAsset`
- 同一仓库下的剧情资产会自动引用该仓库的 GlobalConfig 资产
- 仓库删除的文件会删除对应 UE 资产，删除前弹确认

## 16. 校验规则

硬错误：

- 重复或空的 `nodeId`
- `entryNodeId` 不存在
- 无效边引用
- 无效 Choice 目标引用
- 无效 Choice 倒计时配置（非正数时长、默认选项越界、超时跳转目标不存在）
- 空变量名或重复变量名
- 变量动作引用不存在的变量
- `SetVariable` 节点没有变量动作
- Condition 出边缺失或重复 `sourceHandle`
- Condition `condition-N` 没有对应分支
- Condition 分支完全重复
- 出现旧的 `edges[].condition`

警告：

- 孤立节点
- Condition 缺少 `condition-fallback`
- Condition 分支内部存在明显矛盾
- 多个 Condition 分支可能同时满足，运行时会按顺序命中第一个分支

## 17. 最小剧情示例

该示例假设同一故事仓库的 GlobalConfig 中已经定义了 `Affinity`：

```yaml
meta:
  schemaVersion: 1
  configType: GlobalConfig

variables:
  - name: Affinity
    type: Int
    scope: Global
    defaultValue: "0"
```

```yaml
meta:
  schemaVersion: 1
  storyId: demo
  entryNodeId: N_Start

variables: []

nodes:
  - nodeId: N_Start
    nodeType: Dialogue
    dialogue:
      speakerId: Hero
      textKey: line_start
      speechRate: 1.0
      voiceAsset: ""
    choices: []
    enterActions: []
    exitActions: []

  - nodeId: N_Check
    nodeType: Condition
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

  - nodeId: N_High
    nodeType: End

  - nodeId: N_Normal
    nodeType: End

edges:
  - sourceNodeId: N_Start
    targetNodeId: N_Check
    priority: 0

  - sourceNodeId: N_Check
    sourceHandle: condition-0
    targetNodeId: N_High
    priority: 0

  - sourceNodeId: N_Check
    sourceHandle: condition-fallback
    targetNodeId: N_Normal
    priority: 1
```
