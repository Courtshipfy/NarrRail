# NarrRail 调试器使用指南

## 概述

NarrRail 调试器提供了强大的运行时调试功能，可以查看会话状态、节点信息、变量值等。

**核心特性：**
- ✅ 自动查找活跃会话（无需手动设置）
- ✅ 多会话检测与警告
- ✅ **屏幕 HUD 显示（narrrail.debug.hud）**
- ✅ 蓝图调试节点
- ✅ 控制台命令

## 快速开始

### 方式 1：屏幕 HUD 显示（推荐）

最直观的调试方式，实时显示所有信息。

**步骤：**
1. 运行游戏（PIE）
2. 按 ` 键打开控制台
3. 输入 `narrrail.debug.hud`
4. 屏幕左上角会实时显示调试信息

**再次输入 `narrrail.debug.hud` 关闭显示。**

### 方式 2：控制台命令

适合需要详细日志的场景。
### 方式 2：控制台命令

适合需要详细日志的场景。

```
narrrail.debug.all       # 查看完整调试信息
narrrail.debug.session   # 查看会话状态
narrrail.debug.node      # 查看当前节点
narrrail.debug.vars      # 查看所有变量
```

### 方式 3：蓝图调试节点

适合在特定事件中触发调试。

```
OnNodeEntered
  ↓
Print Current Node
  ↓
Print All Variables
```

## HUD 显示

### 使用方法

**开启：**
```
narrrail.debug.hud
```

**关闭：**
```
narrrail.debug.hud  (再次输入)
```

### 显示内容

HUD 会实时显示以下信息：

1. **Session State（会话状态）**
   - 当前状态（Running/Paused/Completed 等）
   - 当前节点 ID
   - 历史记录数量

2. **Current Node（当前节点）**
   - 节点类型
   - 对话内容（Speaker + Text）
   - 进入动作数量

3. **Variables（变量）**
   - 所有变量的当前值
   - 实时更新

4. **Current Choices（当前选项）**
   - 可用选项列表
   - 目标节点

5. **Last Choice（最后选择）**
   - 上次选择的索引
   - 选项文本
   - 目标节点

6. **Node History（节点历史）**
   - 最近 5 条历史记录

### 多会话场景

如果检测到多个会话，HUD 会显示：
```
NarrRail Debug: Multiple sessions detected (2)
  [0] Player1_Story - Running
  [1] Player2_Story - WaitingForChoice
```

### 屏幕截图示例

```
=== NarrRail Debug ===

Session State:
  State: Running
  Current Node: N_Choice
  History Count: 3

Current Node:
  Type: Choice
  Speaker: A
  Text: 要不要一起去散步？

Variables:
  Affinity = 10
  HasKey = true

Current Choices:
  [0] 好啊，我很乐意！ -> N_Happy
  [1] 不了，我还有事。 -> N_Sad

Last Choice:
  [0] 好啊，我很乐意！ -> N_Happy

Node History (last 5):
  [0] N_Start
  [1] N_Dialogue1
  [2] N_Choice
```

## 控制台命令（详细）

如果你的游戏中有多个会话（例如多人游戏），调试器会报错并列出所有会话。

**解决方案：给会话命名**
```
Create Story Session → 保存为 Session
  ↓
Set Debug Name (Session, "Player1_Story")
  ↓
Start Session
```

然后在蓝图中显式传递会话：
```
Print Session State (Session)
Print Current Node (Session)
```

## 控制台命令（详细）

所有命令都会自动查找唯一会话。

| 命令 | 功能 | 输出位置 |
|------|------|----------|
| `narrrail.debug.hud` | **开启/关闭屏幕 HUD 显示** | 屏幕左上角 |
| `narrrail.debug.session` | 打印会话状态 | 日志 |
| `narrrail.debug.node` | 打印当前节点信息 | 日志 |
| `narrrail.debug.vars` | 打印所有变量 | 日志 |
| `narrrail.debug.history` | 打印节点历史（最近10条） | 日志 |
| `narrrail.debug.choices` | 打印当前可用选项 | 日志 |
| `narrrail.debug.lastchoice` | 打印最后一次选择 | 日志 |
| `narrrail.debug.all` | 打印完整调试信息 | 日志 |
| `narrrail.debug.sessions` | 列出所有活跃会话 | 日志 |

## 多会话场景

所有节点都支持可选的 Session 参数，留空则自动查找。

### 基础调试

- **Print Session State** - 打印会话状态
  - Session (可选) - 会话对象
  - Verbose - 是否显示详细信息

- **Print Current Node** - 打印当前节点
  - Session (可选)

- **Print All Variables** - 打印所有变量
  - Session (可选)

- **Print Node History** - 打印节点历史
  - Session (可选)
  - Max Count - 最多显示多少条（0 = 全部）

- **Print Current Choices** - 打印当前选项
  - Session (可选)

- **Print Last Choice** - 打印最后选择
  - Session (可选)

### 高级调试

- **Print Full Debug Info** - 打印完整调试信息
  - Session (可选)

- **Show Debug On Screen** - 在屏幕显示调试信息
  - World Context Object - 世界上下文
  - Session (可选)
  - Display Time - 显示时长（秒）

- **Print Story Structure** - 打印剧情资产结构
  - Story Asset - 剧情资产

- **Print All Active Sessions** - 列出所有活跃会话

### 会话管理

- **Set Debug Name** - 设置会话调试名称
  - Debug Name - 名称字符串

- **Get Debug Name** - 获取会话调试名称
  - 返回：名称字符串

## 输出示例

### 会话状态
```
======== SESSION STATE ========
State: Running
Current Node: N_Choice
History Count: 3
========================================
```

### 当前节点
```
======== CURRENT NODE ========
Node ID: N_Choice
Node Type: Choice
  Choices: 2
    [0] 好啊，我很乐意！ -> N_Happy
    [1] 不了，我还有事。 -> N_Sad
========================================
```

### 变量
```
======== VARIABLES ========
  Affinity = 10
  HasKey = true
  PlayerName = Alice
========================================
```

### 节点历史
```
======== NODE HISTORY ========
  [0] N_Start
  [1] N_Dialogue1
  [2] N_Choice
========================================
```

### 最后选择
```
======== LAST CHOICE ========
  Choice Node: N_Choice
  Choice Index: 0
  Choice Text: 好啊，我很乐意！
  Target Node: N_Happy
========================================
```

## 多会话错误示例

当检测到多个会话时：
```
[NarrRail Debug] Error: MULTIPLE SESSIONS DETECTED (2 sessions)! This is not supported by the debugger.
[NarrRail Debug] Error: Active sessions:
[NarrRail Debug] Error:   - Session 0: Player1_Story
[NarrRail Debug] Error:   - Session 1: Player2_Story
[NarrRail Debug] Error: Please use SetDebugName() to name your sessions, or pass the session explicitly to debug functions.
```

## 最佳实践

### 1. 单会话游戏（推荐）

直接使用，无需任何设置：
```
Create Story Session → Start → (使用控制台命令调试)
```

### 2. 多会话游戏

给每个会话命名：
```
Create Story Session → Set Debug Name ("Player1") → Start
Create Story Session → Set Debug Name ("Player2") → Start
```

然后显式传递会话给调试函数：
```
Print Session State (Player1Session)
Print Session State (Player2Session)
```

### 3. 开发时调试

在 OnNodeEntered 事件中添加自动调试：
```
OnNodeEntered
  ↓
Print Current Node
  ↓
Print All Variables
```

### 4. 屏幕显示

在关键节点显示调试信息：
```
OnChoicesReady
  ↓
Show Debug On Screen (Self, Session, 5.0)
```

## 注意事项

1. **自动查找限制**：只在单会话场景下自动查找，多会话必须显式传递
2. **性能影响**：调试输出会影响性能，发布版本应禁用
3. **日志级别**：使用 `LogNarrRailDebug` 日志类别
4. **会话生命周期**：会话销毁时自动从全局列表移除

## 故障排除

### 问题：控制台命令无输出

**原因：** 没有活跃会话

**解决：**
1. 确保已创建并初始化会话
2. 使用 `narrrail.debug.sessions` 查看活跃会话列表

### 问题：多会话错误

**原因：** 同时存在多个会话

**解决：**
1. 给每个会话命名：`Set Debug Name`
2. 显式传递会话给调试函数

### 问题：蓝图中找不到调试节点

**原因：** Editor 模块未加载

**解决：**
1. 确保在编辑器中运行（PIE）
2. 检查插件是否正确编译
