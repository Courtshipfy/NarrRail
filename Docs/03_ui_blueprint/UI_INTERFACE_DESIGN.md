# NarrRail UI 接口设计总结

## 设计目标

NarrRail 插件的 UI 层不直接承载剧情执行逻辑。运行时由 `UNarrRailStorySession` 负责推进节点、变量、条件和选择；UI 只通过接口和事件接收当前需要展示的内容。

目标：

- 支持不同展示风格，例如 ADV、NVL、全屏演出或调试 UI
- UI 可以用 Blueprint Widget 实现
- 运行时核心不依赖具体 UI Widget
- Dialogue、MultiDialogue、Choice、Condition 跳转后的显示路径保持一致

## 核心组件

### `UNarrRailStorySession`

职责：

- 初始化并运行 `UNarrRailStoryAsset`
- 提供 `Start` / `Next` / `Choose` / `Pause` / `Resume` / `Stop`
- 维护当前节点、历史、变量快照和会话状态
- 广播运行时事件
- 注册可选的对话显示器

关键事件：

- `OnSessionStarted`
- `OnNodeEntered`
- `OnNodeExited`
- `OnSessionEnded`
- `OnChoicesReady`
- `OnChoiceSelected`

### `INarrRailDialoguePresenterInterface`

职责：

- 由 UI Widget 或其他 UObject 实现
- 接收运行时构造好的显示请求
- 展示/隐藏对话和选项

当前接口文件：

```text
NarrRail/Source/NarrRail/Public/Runtime/NarrRailDialoguePresenterInterface.h
```

常见接入方式：

1. 创建一个 Widget Blueprint
2. 实现 `NarrRailDialoguePresenterInterface`
3. 在 BeginPlay 创建 `UNarrRailStorySession`
4. 调用 `RegisterDialoguePresenter`
5. 调用 `Start`
6. 用户点击继续时调用 `Next`
7. 用户选择选项时调用 `Choose`

## 推荐 Blueprint 集成方式

### 初始化

```text
Create Story Session
Register Dialogue Presenter
Bind Event to On Node Entered
Bind Event to On Choices Ready
Start
```

### 显示对话

`OnNodeEntered` 触发后：

- 判断节点类型是否为 `Dialogue` 或 `MultiDialogue`
- 从节点中读取 `Dialogue` 或当前 MultiDialogue 行
- 更新 Widget 中的说话人、正文、语速和语音资源

如果已经注册 Presenter，运行时也会尝试通过 Presenter 接口推送展示请求。

### 显示选择

`OnChoicesReady` 触发后：

- 使用传入的 `Choices` 生成按钮
- 按按钮索引调用 `Choose(Index)`
- 选择完成后由 Session 继续推进到目标节点

### 推进剧情

常规继续按钮：

```text
Next
```

选择按钮：

```text
Choose(VisibleChoiceIndex)
```

## 与节点类型的关系

- `Dialogue`：进入节点时展示单行对话
- `MultiDialogue`：同一个节点内按行推进，多次 `Next` 后才离开节点
- `Choice`：进入后进入 `WaitingForChoice`，等待 UI 调用 `Choose`
- `Condition`：不需要 UI 展示，运行时根据条件自动选择 `condition-N` 或 `condition-fallback`
- `SetVariable` / `EmitEvent` / `Jump`：通常不需要 UI 停留，执行后继续沿流程推进
- `End`：进入完成状态或触发会话结束

## UI 层边界

UI 可以做：

- 展示文本、角色名、头像、语音、打字机效果
- 展示选项按钮
- 调用 `Next` / `Choose`
- 监听运行时事件更新界面

UI 不应该做：

- 自己解析 `.nrstory`
- 自己判断 Condition 分支
- 自己修改边或节点流程
- 自己维护核心变量状态

## 发布前建议

- 为示例 Widget 提供一个最小 Blueprint 流程
- 文档中明确 `ChoiceIndex` 使用可见选项的 0-based 索引
- 对 `MultiDialogue` 当前行索引做 UI 示例
- 后续补充 GlobalConfig 与预设角色在 UI 中的消费示例
