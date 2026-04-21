# NarrRail 脚本格式规范 v1

## 1. 适用范围

本文档定义了 NarrRail 用于导入/导出和离线校验的脚本文件格式。
当前基准格式：YAML (`.narrrail.yaml` 或 `.narrrail.yml`)。

## 2. 根结构

```yaml
meta:
  schemaVersion: 1
  storyId: demo_story
  entryNodeId: N_Start

variables: []

nodes: []

edges: []
```

必需的根字段：
- `meta` (对象)
- `variables` (数组)
- `nodes` (数组)
- `edges` (数组)

## 3. 元数据（Meta）

| 字段 | 类型 | 必需 | 说明 |
|---|---|---|---|
| `schemaVersion` | int | 是 | 当前值为 `1` |
| `storyId` | string | 是 | 唯一的剧情 ID |
| `entryNodeId` | string | 是 | 必须存在于 `nodes[].nodeId` 中 |

## 4. 变量（Variables）

```yaml
- name: Affinity
  type: Int
  scope: Session
  defaultValue: "0"
```

| 字段 | 类型 | 必需 | 说明 |
|---|---|---|---|
| `name` | string | 是 | 在 `variables` 中唯一 |
| `type` | 枚举 | 是 | `Bool` / `Int` / `Float` / `String` |
| `scope` | 枚举 | 否 | `Session`（默认）/ `Global` |
| `defaultValue` | string | 否 | 根据变量类型解析 |

## 5. 节点（Nodes）

基础节点对象：

```yaml
- nodeId: N_Start
  nodeType: Dialogue
  dialogue:
    speakerId: Hero
    textKey: line_001
    speechRate: 1.0
    voiceAsset: ""
  choices: []
  jumpTargetNodeId: ""
  enterActions: []
  exitActions: []
```

| 字段 | 类型 | 必需 | 说明 |
|---|---|---|---|
| `nodeId` | string | 是 | 唯一的节点 ID |
| `nodeType` | 枚举 | 是 | `Dialogue` / `Choice` / `Jump` / `SetVariable` / `EmitEvent` / `End` |
| `dialogue` | 对象 | 否 | 用于 `Dialogue` 类型 |
| `choices` | 数组 | 否 | 用于 `Choice` 类型 |
| `jumpTargetNodeId` | string | 否 | 用于 `Jump` 类型 |
| `enterActions` | 数组 | 否 | 节点主体执行前的动作 |
| `exitActions` | 数组 | 否 | 离开节点前的动作 |

### 5.1 对话载荷（Dialogue Payload）

```yaml
speakerId: Hero
textKey: line_001
speechRate: 1.0
voiceAsset: ""
```

### 5.2 选项（Choice Option）

```yaml
- textKey: option_yes
  targetNodeId: N_Yes
  availability:
    logic: All
    terms: []
```

## 6. 边（Edges）

```yaml
- sourceNodeId: N_Start
  targetNodeId: N_Choice
  priority: 0
  condition:
    logic: All
    terms: []
```

| 字段 | 类型 | 必需 | 说明 |
|---|---|---|---|
| `sourceNodeId` | string | 是 | 必须存在 |
| `targetNodeId` | string | 是 | 必须存在 |
| `priority` | int | 否 | 数值越小优先级越高 |
| `condition` | 对象 | 否 | 空 terms 表示始终为真 |

## 7. 条件（Conditions）

表达式：

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

支持的运算符：
- `==` - 等于
- `!=` - 不等于
- `>` - 大于
- `>=` - 大于等于
- `<` - 小于
- `<=` - 小于等于

## 8. 动作（Actions）

```yaml
- actionType: Add
  variable:
    name: Affinity
    type: Int
    scope: Session
  value: "2"
  eventId: ""
```

| 字段 | 类型 | 必需 | 说明 |
|---|---|---|---|
| `actionType` | 枚举 | 是 | `Set` / `Add` / `Subtract` / `EmitEvent` |
| `variable` | 对象 | Set/Add/Subtract 需要 | 变量引用 |
| `value` | string | Set/Add/Subtract 需要 | 输入值 |
| `eventId` | string | EmitEvent 需要 | 事件标识符 |

## 9. 校验规则

硬错误（必须修复）：
- 重复的 `nodeId`
- 缺失 `entryNodeId`
- 无效的边引用
- 无效的选项目标引用
- 空变量名或重复变量名

警告（建议修复）：
- 孤立节点（除入口节点外）

## 10. 版本控制与兼容性

- 当前 `schemaVersion`：`1`
- 读取器行为：
  - 未知的更新版本：拒绝并显示明确错误
  - 已知的旧版本：在内存中迁移到最新版本
- 运行时资产的迁移入口位于 `UNarrRailStoryAsset::PostLoad()`。

## 11. 最小示例

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

## 12. 使用说明

### 12.1 创建脚本

1. 按照本规范创建 YAML 文件
2. 确保所有必需字段都已填写
3. 使用校验工具检查格式（待实现）

### 12.2 导入到 UE

1. 使用导入工具将 YAML 转换为 UE 资产（待实现）
2. 或在蓝图中使用 `UNarrRailBlueprintLibrary` 手动创建

### 12.3 导出为脚本

1. 使用导出工具将 UE 资产转换为 YAML（待实现）
2. 可用于版本控制和外部编辑

## 13. 完整示例：好感度系统

以下是一个完整的好感度系统示例，演示了变量、动作、条件分支的综合使用。

完整脚本见：`Tools/NarrRail.Tooling/affinity_demo.narrrail.yaml`

### 剧情流程

```
开场对话 → 询问散步 → 玩家选择
  ├─ 选择1: "好啊" → 好感度+10 → [>=10] 特殊结局
  │                              └─ [<10]  普通结局
  └─ 选择2: "不了" → 好感度-5  → 普通结局
```

### 关键代码片段

**变量定义：**
```yaml
variables:
  - name: Affinity
    type: Int
    scope: Session
    defaultValue: "0"
```

**带动作的节点：**
```yaml
- nodeId: N_A_Happy
  nodeType: Dialogue
  dialogue:
    speakerId: A
    textKey: "太好了！我很高兴你愿意和我一起。"
    speechRate: 1.0
    voiceAsset: ""
  enterActions:
    - actionType: Add
      variable:
        name: Affinity
        type: Int
        scope: Session
      value: "10"
      eventId: ""
```

**条件边（优先级）：**
```yaml
# 优先级 0：先检查 >= 10
- sourceNodeId: N_A_Happy
  targetNodeId: N_A_SpecialEnding
  priority: 0
  condition:
    logic: All
    terms:
      - variable:
          name: Affinity
          type: Int
          scope: Session
        operator: ">="
        compareValue: "10"

# 优先级 1：再检查 < 10
- sourceNodeId: N_A_Happy
  targetNodeId: N_A_NormalEnding
  priority: 1
  condition:
    logic: All
    terms:
      - variable:
          name: Affinity
          type: Int
          scope: Session
        operator: "<"
        compareValue: "10"
```

**选项节点：**
```yaml
- nodeId: N_B_Choice
  nodeType: Choice
  choices:
    - textKey: "好啊，我很乐意！"
      targetNodeId: N_A_Happy
      availability:
        logic: All
        terms: []
    - textKey: "不了，我还有事。"
      targetNodeId: N_A_Sad
      availability:
        logic: All
        terms: []
```

### 运行结果

**场景 1：选择"好啊"**
- 好感度：0 → 10
- 结局：特殊结局（"和你在一起真开心！"）

**场景 2：选择"不了"**
- 好感度：0 → -5
- 结局：普通结局（"那我先走了，再见。"）

## 14. 注意事项

1. 所有字符串字段使用 UTF-8 编码
2. 节点 ID 建议使用前缀（如 `N_`）便于识别
3. 文本键（textKey）建议使用统一的命名规范
4. 变量名建议使用驼峰命名法
5. 条件表达式支持嵌套逻辑（All/Any）
6. 动作按数组顺序依次执行
7. 边的优先级在多个条件满足时生效（数值越小越优先）
8. 负数值使用字符串表示（如 `"-5"`）
9. 条件边的优先级很重要：确保互斥条件按正确顺序检查
