# NarrRail UI 接口设计总结

## 设计目标

为 NarrRail 插件设计一个灵活的 UI 接口系统，使得不同风格的对话 UI（ADV 模式和 NVL 模式）可以通过实现统一接口来显示剧本内容。

## 架构设计

### 核心组件

```
┌─────────────────────────────────────────────────────────────┐
│                    UNarrRailStorySession                     │
│  (剧情会话执行器 - 负责剧情逻辑和状态管理)                    │
│                                                               │
│  - Start() / Next() / Choose()                               │
│  - RegisterDialoguePresenter()                               │
│  - OnNodeEntered / OnChoicesReady                            │
└───────────────────────┬─────────────────────────────────────┘
                        │ 注册并通知
                        ↓
┌─────────────────────────────────────────────────────────────┐
│          INarrRailDialoguePresenterInterface                 │
│  (对话显示器接口 - Blueprint 可实现)                          │
│                                                               │
│  - ShowDialogue(Request)                                     │
│  - ShowChoices(Request)                                      │
│  - HideDialogue()                                            │
│  - ClearDialogueHistory()                                    │
│  - GetDialogueMode() → ADV / NVL                             │
│  - IsDialogueActive()                                        │
│  - SkipDialogueAnimation()                                   │
└───────────────────────┬─────────────────────────────────────┘
                        │ 实现
                        ↓
        ┌───────────────┴───────────────┐
        ↓                               ↓
┌──────────────────┐          ┌──────────────────┐
│ WBP_Dialogue_ADV │          │ WBP_Dialogue_NVL │
│  (ADV 模式 UI)   │          │  (NVL 模式 UI)   │
│                  │          │                  │
│ - 底部对话框     │          │ - 全屏文本       │
│ - 单行显示       │          │ - 历史记录       │
│ - 打字机效果     │          │ - 滚动显示       │
└──────────────────┘          └──────────────────┘
```

## 文件结构

### 新增文件

1. **NarrRailDialoguePresenterInterface.h**
   - 路径：`NarrRail/Source/NarrRail/Public/Runtime/`
   - 内容：
     - `ENarrRailDialogueMode` 枚举（ADV/NVL）
     - `FNarrRailDialogueRequest` 结构体
     - `FNarrRailChoiceRequest` 结构体
     - `INarrRailDialoguePresenterInterface` 接口定义

2. **NarrRailDialoguePresenterInterface.cpp**
   - 路径：`NarrRail/Source/NarrRail/Private/Runtime/`
   - 内容：接口方法的默认实现

3. **UI_INTERFACE_GUIDE.md**
   - 路径：`docs/`
   - 内容：Blueprint 实现指南和示例

### 修改文件

1. **NarrRailStorySession.h**
   - 添加：`#include "Runtime/NarrRailDialoguePresenterInterface.h"`
   - 添加：`RegisterDialoguePresenter()` 方法
   - 添加：`UnregisterDialoguePresenter()` 方法
   - 添加：`GetDialoguePresenter()` 方法
   - 添加：`TScriptInterface<INarrRailDialoguePresenterInterface> DialoguePresenter` 成员变量

2. **NarrRailStorySession.cpp**
   - 实现：UI 注册方法
   - 修改：`AdvanceToNode()` 方法，添加自动通知 UI 的逻辑

## 数据流

### 对话显示流程

```
1. 用户调用 Session.Start() 或 Session.Next()
   ↓
2. Session 内部调用 AdvanceToNode(NodeId)
   ↓
3. AdvanceToNode 执行节点逻辑
   ↓
4. 触发 OnNodeEntered 事件
   ↓
5. 检查是否注册了 DialoguePresenter
   ↓
6. 如果是 Dialogue 节点：
   - 构建 FNarrRailDialogueRequest
   - 调用 DialoguePresenter.ShowDialogue(Request)
   ↓
7. UI 接收到请求，显示对话
   - ADV: 更新底部对话框
   - NVL: 追加到历史文本
```

### 选项显示流程

```
1. Session 推进到 Choice 节点
   ↓
2. AdvanceToNode 检测到 Choice 类型
   ↓
3. 构建 FNarrRailChoiceRequest
   ↓
4. 调用 DialoguePresenter.ShowChoices(Request)
   ↓
5. UI 显示选项按钮
   ↓
6. 用户点击选项
   ↓
7. UI 调用 Session.Choose(Index)
   ↓
8. Session 推进到目标节点
```

## 接口方法详解

### ShowDialogue

**用途**：显示一条对话

**输入**：`FNarrRailDialogueRequest`
- `NodeId`: 当前节点 ID
- `SpeakerId`: 说话人 ID（用于显示角色名）
- `TextContent`: 对话文本内容
- `SpeechRate`: 语速（用于调整打字机速度或语音播放速度）
- `VoiceAsset`: 语音资产（可选）
- `bAutoAdvance`: 是否自动推进（未来功能）

**实现要点**：
- ADV 模式：更新说话人名字和对话文本，启动打字机效果
- NVL 模式：格式化为 "[说话人]\n文本\n\n"，追加到历史记录

### ShowChoices

**用途**：显示选择分支

**输入**：`FNarrRailChoiceRequest`
- `NodeId`: 当前节点 ID
- `Choices`: 选项列表（`TArray<FNarrRailChoiceOption>`）

**实现要点**：
- 清空现有选项按钮
- 遍历 Choices，为每个选项创建按钮
- 绑定按钮点击事件到 `Session.Choose(Index)`

### HideDialogue

**用途**：隐藏对话框

**使用场景**：
- 会话结束时
- 切换场景时
- 进入战斗等其他系统时

### ClearDialogueHistory

**用途**：清空对话历史

**使用场景**：
- NVL 模式：章节切换时清空历史
- ADV 模式：通常不需要实现

### GetDialogueMode

**用途**：返回当前 UI 的显示模式

**返回值**：`ENarrRailDialogueMode::ADV` 或 `ENarrRailDialogueMode::NVL`

**用途**：
- 系统可以根据模式调整行为
- 例如：NVL 模式可能需要更多的历史管理

### IsDialogueActive

**用途**：检查对话是否正在显示中

**返回值**：`bool`

**使用场景**：
- 检查打字机效果是否完成
- 决定用户点击是跳过动画还是推进到下一句

### SkipDialogueAnimation

**用途**：跳过当前对话动画

**实现要点**：
- 立即显示完整文本
- 停止打字机 Timer
- 设置 `bIsTyping = false`

## 使用示例

### Blueprint 中的完整流程

```blueprint
// 1. 创建 Session
Event BeginPlay
├─ Create Object: UNarrRailStorySession → Session
├─ Session.Initialize(StoryAsset)
│
// 2. 创建并注册 UI
├─ Create Widget: WBP_Dialogue_ADV → DialogueWidget
├─ DialogueWidget.AddToViewport()
├─ Session.RegisterDialoguePresenter(DialogueWidget)
│
// 3. 启动剧情
└─ Session.Start()

// 4. 用户交互
Event OnMouseButtonDown
├─ 如果 Session.GetSessionState() == WaitingForChoice
│   └─ 不处理（等待用户选择选项）
├─ 否则
│   ├─ 如果 DialogueWidget.IsDialogueActive()
│   │   └─ DialogueWidget.SkipDialogueAnimation()
│   └─ 否则
│       └─ Session.Next()
```

### C++ 中的使用示例

```cpp
// 创建 Session
UNarrRailStorySession* Session = NewObject<UNarrRailStorySession>();
Session->Initialize(StoryAsset);

// 创建 UI Widget
UUserWidget* DialogueWidget = CreateWidget<UUserWidget>(GetWorld(), WBP_Dialogue_ADV_Class);
DialogueWidget->AddToViewport();

// 注册 UI
TScriptInterface<INarrRailDialoguePresenterInterface> Presenter;
Presenter.SetObject(DialogueWidget);
Presenter.SetInterface(Cast<INarrRailDialoguePresenterInterface>(DialogueWidget));
Session->RegisterDialoguePresenter(Presenter);

// 启动剧情
Session->Start();
```

## 扩展性

### 支持新的显示模式

如果需要添加新的显示模式（例如：聊天气泡模式），只需：

1. 在 `ENarrRailDialogueMode` 中添加新枚举值
2. 创建新的 Widget Blueprint
3. 实现 `INarrRailDialoguePresenterInterface` 接口
4. 在 `GetDialogueMode()` 中返回新的模式

### 支持自定义数据

如果需要传递额外的数据给 UI，可以：

1. 扩展 `FNarrRailDialogueRequest` 结构体
2. 在 `AdvanceToNode()` 中填充新字段
3. UI 在 `ShowDialogue()` 中读取新字段

## 优势

1. **解耦**：Session 不需要知道 UI 的具体实现
2. **灵活**：可以轻松切换不同的 UI 风格
3. **可测试**：可以创建 Mock UI 进行单元测试
4. **可扩展**：容易添加新的显示模式和功能
5. **Blueprint 友好**：完全支持 Blueprint 实现

## 注意事项

1. **线程安全**：所有接口调用都在 Game Thread 上
2. **生命周期**：确保 UI Widget 在 Session 之前销毁时调用 `UnregisterDialoguePresenter()`
3. **性能**：NVL 模式需要注意历史文本的内存占用
4. **本地化**：`TextContent` 应该是已经本地化的文本

## 未来改进

1. **动画系统集成**：支持角色立绘、表情变化
2. **音效系统**：支持对话音效、背景音乐控制
3. **特效系统**：支持屏幕震动、闪光等特效
4. **自动播放**：支持自动推进对话
5. **历史回顾**：支持查看对话历史和回退
6. **快进/跳过**：支持快进已读对话或跳过整个章节
