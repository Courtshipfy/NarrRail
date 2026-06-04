# 蓝图快速开始指南：创建简单对话

本文说明如何在 Blueprint 中创建并运行一个最小 NarrRail 对话流程。这个流程适合验证插件是否正确编译、会话是否能启动、UI 是否能收到节点事件。

## 前提条件

- 已编译 NarrRail 插件
- 使用 UE5.7 打开 `NarrRailUEHost/NarrRailUEHost.uproject`
- 已启用 NarrRail 插件

## 方式一：使用同步后的故事资产

推荐生产流程是从 NarrRailEditor 导出 `.nrstory`，提交到故事仓库，再在 UE 中使用 `Sync Stories` 同步为资产。

1. 打开 `Project Settings > Plugins > NarrRail`
2. 设置 `Story Repository Path`
3. 点击工具栏 `Sync Stories`
4. 在 Content Browser 中找到生成的 `UNarrRailStoryAsset`
5. 在 Blueprint 中创建并运行 Session

Blueprint 流程：

```text
BeginPlay
  -> Create Story Session(StoryAsset)
  -> Bind Event to On Node Entered
  -> Bind Event to On Choices Ready
  -> Start
```

如果需要显式指定全局配置，也可以使用：

```text
Create Story Session With Global Config(StoryAsset, GlobalConfigAsset)
```

同步流程生成的 StoryAsset 会自动绑定同仓库的 GlobalConfig，通常不需要手动传入。

## 方式二：在 Blueprint 中临时创建测试资产

这个方式只适合快速测试 API，不建议作为正式创作流程。

### 1. 创建空剧情资产

```text
Create Story Asset
- Story Id: SimpleDialogue
- Entry Node Id: Start
```

### 2. 添加对话节点

```text
Add Dialogue Node
- Node Id: Start
- Speaker Id: A
- Text Key: 你好，B！今天天气不错。

Add Dialogue Node
- Node Id: B_Reply1
- Speaker Id: B
- Text Key: 是啊，A。我也这么觉得。

Add Dialogue Node
- Node Id: A_Reply2
- Speaker Id: A
- Text Key: 要不要一起去散步？

Add Dialogue Node
- Node Id: B_Reply2
- Speaker Id: B
- Text Key: 好主意，我们走吧。
```

### 3. 添加结束节点

```text
Add End Node
- Node Id: End
```

### 4. 连接节点

```text
Add Edge: Start -> B_Reply1
Add Edge: B_Reply1 -> A_Reply2
Add Edge: A_Reply2 -> B_Reply2
Add Edge: B_Reply2 -> End
```

## 创建并启动会话

```text
Create Story Session
- Story Asset: 上一步创建或同步得到的资产

Bind Event to On Node Entered
Bind Event to On Session Ended
Start
```

## 显示对话内容

在 `On Node Entered` 自定义事件中：

```text
Is Dialogue Node
Get Dialogue From Node
Print String 或更新 Widget
```

建议格式：

```text
{SpeakerId}: {TextKey}
```

## 推进剧情

给 UI 的继续按钮绑定：

```text
StorySession.Next
```

如果当前节点是 `Choice`，Session 会进入 `WaitingForChoice`，此时应等待用户选择，而不是继续调用 `Next`。

## 显示并选择选项

在 `On Choices Ready` 中：

1. 遍历传入的 `Choices`
2. 为每个选项创建按钮
3. 按钮点击时调用：

```text
StorySession.Choose(VisibleChoiceIndex)
```

`VisibleChoiceIndex` 是当前 UI 可见选项列表中的 0-based 索引。

## 存档与读档

NarrRail 插件提供快照 API，不直接决定项目的存档槽策略。推荐做法是把 NarrRail 快照放进项目自己的 `USaveGame`。

Session 快照：

```text
StorySession.GetSessionSnapshot
StorySession.RestoreSessionSnapshot
```

全局变量快照：

```text
Get NarrRail Global State
  -> GetGlobalStateSnapshot
  -> RestoreGlobalStateSnapshot
```

恢复顺序：

```text
Create Story Session
  -> Initialize / Initialize With Global Config
  -> RestoreGlobalStateSnapshot
  -> RestoreSessionSnapshot
```

`RestoreSessionSnapshot` 只恢复运行状态，不重放节点进入动作。`bRefreshPresenter` 为 true 时会刷新当前节点显示和选择列表。

示例工程 `NarrRailUEHost` 已在 `ANarrRailPlayerController` 中提供：

```text
SaveNarrRailState
LoadNarrRailState
```

它使用 `UNarrRailHostSaveGame` 保存 Session 快照和全局状态快照，可用于 PIE 中同步验证插件存档机制。

## 常见问题

### `Next` 返回 Current node requires Choose

当前节点是 `Choice`，需要调用 `Choose`。

### 会话直接结束

当前节点没有出边，或者已经进入 `End` 节点。检查 `Edges` 是否正确连接。

### 没有显示对话

确认已经绑定 `On Node Entered`，并且 `Start` 的返回结果是 Success。

### Condition 节点怎么处理

Condition 节点不需要 UI 手动处理。运行时会自动求值，并沿 `condition-0`、`condition-1` 或 `condition-fallback` 出边离开。
