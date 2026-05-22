# NarrRail UI 接口实现指南

## 概述

NarrRail 提供了 `INarrRailDialoguePresenterInterface` 接口，允许你创建自定义的对话 UI 组件。

支持两种主要的显示模式：
- **ADV 模式**：单行对话框，通常位于屏幕底部（如 `WBP_Dialogue_ADV`）
- **NVL 模式**：全屏文本，类似小说阅读（如 `WBP_Dialogue_NVL`）

## 接口方法

### 必须实现的方法

#### 1. ShowDialogue (显示对话)
```
输入：FNarrRailDialogueRequest
- NodeId: 节点 ID
- SpeakerId: 说话人 ID
- TextContent: 对话文本
- SpeechRate: 语速
- VoiceAsset: 语音资产
- bAutoAdvance: 是否自动推进

功能：显示一条对话
```

#### 2. ShowChoices (显示选项)
```
输入：FNarrRailChoiceRequest
- NodeId: 节点 ID
- Choices: 选项列表（TArray<FNarrRailChoiceOption>）

功能：显示选择分支
```

#### 3. HideDialogue (隐藏对话框)
```
功能：隐藏对话框（例如在会话结束时）
```

#### 4. ClearDialogueHistory (清空对话历史)
```
功能：清空对话历史（主要用于 NVL 模式）
```

#### 5. GetDialogueMode (获取显示模式)
```
返回：ENarrRailDialogueMode (ADV 或 NVL)

功能：告诉系统当前 UI 的显示模式
```

#### 6. IsDialogueActive (检查对话是否活跃)
```
返回：bool

功能：检查对话是否正在显示中（例如打字机效果未完成）
```

#### 7. SkipDialogueAnimation (跳过对话动画)
```
功能：跳过当前对话动画（例如立即显示完整文本）
```

## Blueprint 实现步骤

### 步骤 1：创建 Widget Blueprint

1. 在 Content Browser 中右键 → User Interface → Widget Blueprint
2. 命名为 `WBP_Dialogue_ADV` 或 `WBP_Dialogue_NVL`

### 步骤 2：实现接口

1. 打开 Widget Blueprint
2. 在 Class Settings → Interfaces → Add → 选择 `NarrRailDialoguePresenterInterface`
3. 在 Event Graph 中实现接口方法

### 步骤 3：实现 ShowDialogue

**ADV 模式示例：**
```
Event ShowDialogue
├─ 获取 Request.SpeakerId → 设置 SpeakerNameText
├─ 获取 Request.TextContent → 启动打字机效果
└─ 播放 Request.VoiceAsset（如果存在）
```

**NVL 模式示例：**
```
Event ShowDialogue
├─ 获取 Request.SpeakerId + Request.TextContent
├─ 格式化为 "[说话人]\n对话文本\n\n"
├─ 追加到 DialogueHistoryText
└─ 滚动到底部
```

### 步骤 4：实现 ShowChoices

```
Event ShowChoices
├─ 清空现有选项按钮
├─ 遍历 Request.Choices
│   ├─ 创建选项按钮
│   ├─ 设置按钮文本为 Choice.TextKey
│   └─ 绑定点击事件 → 调用 Request.Session.Choose(Index)
└─ 显示选项容器
```

**重要**：`Request.Session` 包含了 Session 对象的引用，UI 可以直接调用 `Request.Session.Choose(Index)` 来选择选项，无需在外部保存 Session 变量。

### 步骤 5：实现其他方法

```
Event GetDialogueMode
└─ Return: ADV (或 NVL)

Event IsDialogueActive
└─ Return: 打字机效果是否正在播放

Event SkipDialogueAnimation
└─ 立即显示完整文本，停止打字机效果

Event HideDialogue
└─ 播放淡出动画，隐藏对话框

Event ClearDialogueHistory
└─ 清空 DialogueHistoryText（NVL 模式）
```

## 使用示例

### 在 Blueprint 中注册 UI

```
Event BeginPlay
├─ Create Widget: WBP_Dialogue_ADV
├─ Add to Viewport
├─ 获取 StorySession
└─ StorySession → RegisterDialoguePresenter (传入 Widget)
```

### 完整流程示例

```
1. 创建 StorySession
   └─ Create Object: UNarrRailStorySession

2. 初始化 Session
   └─ Session → Initialize (传入 StoryAsset)

3. 创建并注册 UI
   ├─ Create Widget: WBP_Dialogue_ADV
   ├─ Add to Viewport
   └─ Session → RegisterDialoguePresenter (传入 Widget)

4. 启动剧情
   └─ Session → Start

5. 推进剧情
   ├─ 用户点击屏幕 → Session → Next
   └─ 用户选择选项 → Session → Choose(Index)
```

## ADV 模式 UI 组件建议

### 必需组件
- **SpeakerNameText**: 显示说话人名字
- **DialogueText**: 显示对话内容
- **NextIndicator**: 提示用户可以继续（箭头/图标）
- **ChoiceContainer**: 选项按钮容器
- **ChoiceButtonTemplate**: 选项按钮模板

### 推荐变量
- **TypewriterSpeed**: 打字机速度（字符/秒）
- **bIsTyping**: 是否正在打字
- **CurrentDialogueText**: 当前完整文本
- **DisplayedCharCount**: 已显示字符数

### 推荐动画
- **FadeIn**: 对话框淡入
- **FadeOut**: 对话框淡出
- **NextIndicatorBlink**: 继续提示闪烁

## NVL 模式 UI 组件建议

### 必需组件
- **DialogueHistoryText**: 显示所有对话历史
- **ScrollBox**: 滚动容器
- **ChoiceContainer**: 选项按钮容器
- **BackgroundOverlay**: 半透明背景

### 推荐变量
- **ParagraphSpacing**: 段落间距
- **TextMargin**: 文本边距
- **MaxHistoryLines**: 最大历史行数（性能优化）

### 推荐功能
- **自动滚动**: 新对话出现时自动滚动到底部
- **历史回顾**: 允许用户向上滚动查看历史
- **清空历史**: 在章节切换时清空历史

## 高级功能

### 打字机效果实现

```
Event ShowDialogue
├─ 保存 Request.TextContent 到 FullText
├─ 设置 DisplayedCharCount = 0
├─ 启动 Timer (每 1/TypewriterSpeed 秒)
│   ├─ DisplayedCharCount++
│   ├─ 更新 DialogueText = FullText.Left(DisplayedCharCount)
│   └─ 如果 DisplayedCharCount >= FullText.Len() → 停止 Timer
└─ 设置 bIsTyping = true
```

### 语音播放

```
Event ShowDialogue
├─ 如果 Request.VoiceAsset 有效
│   ├─ 停止当前语音
│   ├─ 播放 Request.VoiceAsset
│   └─ 调整播放速度为 Request.SpeechRate
└─ ...
```

### 选项按钮交互

```
选项按钮点击事件
├─ 获取选项索引
├─ 调用 Request.Session → Choose(Index)
├─ 隐藏选项容器
└─ 等待下一个对话
```

**注意**：不需要在 UI 组件中保存 Session 变量，直接使用 `ShowChoices` 传入的 `Request.Session` 即可。

## 调试技巧

1. **在 ShowDialogue 中添加 Print String**
   - 打印 SpeakerId 和 TextContent
   - 确认接口方法被正确调用

2. **检查接口实现**
   - 在 Class Settings → Interfaces 中确认接口已添加
   - 确保所有接口方法都有实现

3. **测试 Session 注册**
   - 在 RegisterDialoguePresenter 后打印确认信息
   - 使用 GetDialoguePresenter 检查是否注册成功

## 常见问题

### Q: UI 没有显示对话？
A: 检查以下几点：
1. 是否调用了 `RegisterDialoguePresenter`
2. Widget 是否已添加到 Viewport
3. ShowDialogue 方法是否正确实现
4. Session 是否成功启动（调用了 Start）

### Q: 如何支持多语言？
A: 在 ShowDialogue 中：
1. 使用 TextContent 作为本地化 Key
2. 调用 UE 的本地化系统获取翻译文本
3. 显示翻译后的文本

### Q: 如何实现自动播放？
A: 
1. 在 ShowDialogue 中检查 Request.bAutoAdvance
2. 如果为 true，启动 Timer 自动调用 Session.Next()
3. 延迟时间可以根据文本长度和语速计算

## 示例项目

参考 `HostProject/Content/UI/` 中的示例：
- `WBP_Dialogue_ADV`: ADV 模式实现
- `WBP_Dialogue_NVL`: NVL 模式实现
