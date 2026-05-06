# 选项选择的解耦设计

## 问题

在接口解耦的设计中，UI 组件需要调用 `Session.Choose(Index)` 来选择选项，但是：
- UI 组件不应该直接持有 Session 的引用（破坏解耦）
- 外部创建 Session 的代码也不应该直接绑定到 UI 的按钮上（耦合太紧）

## 解决方案：通过 Request 传递 Session 引用

### 设计思路

在 `FNarrRailChoiceRequest` 结构体中添加 `Session` 字段：

```cpp
USTRUCT(BlueprintType)
struct FNarrRailChoiceRequest
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NarrRail")
    FName NodeId = NAME_None;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NarrRail")
    TArray<FNarrRailChoiceOption> Choices;

    // Session 对象（用于 UI 调用 Choose 方法）
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NarrRail")
    TObjectPtr<UNarrRailStorySession> Session = nullptr;
};
```

### 工作流程

```
1. Session 推进到 Choice 节点
   ↓
2. Session 构建 FNarrRailChoiceRequest
   - Request.Choices = 选项列表
   - Request.Session = this  // 传入自己的引用
   ↓
3. Session 调用 UI.ShowChoices(Request)
   ↓
4. UI 接收 Request，创建选项按钮
   ↓
5. 用户点击选项按钮
   ↓
6. UI 调用 Request.Session.Choose(Index)
   ↓
7. Session 推进到目标节点
```

### Blueprint 实现示例

**ShowChoices 实现：**
```
Event ShowChoices (Request)
├─ 保存 Request 到变量 CurrentChoiceRequest
├─ 清空选项容器
├─ ForEach: Request.Choices
│   ├─ 创建按钮 Widget
│   ├─ 设置按钮文本 = Choice.TextKey
│   ├─ 绑定按钮点击事件 → OnChoiceButtonClicked(Index)
│   └─ 添加到选项容器
└─ 显示选项容器
```

**按钮点击事件：**
```
Function OnChoiceButtonClicked (Index)
├─ 获取 CurrentChoiceRequest.Session
├─ 调用 Session.Choose(Index)
├─ 隐藏选项容器
└─ 清空 CurrentChoiceRequest
```

### 优势

1. **解耦**：UI 组件不需要在外部注入 Session 引用
2. **简洁**：不需要额外的事件委托或回调机制
3. **安全**：Session 引用只在需要时（显示选项时）传递
4. **灵活**：UI 可以保存 Request 用于后续操作（如重新显示选项）

### 注意事项

1. **生命周期**：UI 不应该长期持有 Session 引用，只在处理选项时使用
2. **空指针检查**：在调用 `Request.Session.Choose()` 前检查 Session 是否有效
3. **单次使用**：每次 ShowChoices 都会传入新的 Request，不要复用旧的 Request

### 对比其他方案

#### 方案 A：UI 持有 Session 引用（❌ 不推荐）
```
优点：实现简单
缺点：破坏解耦，UI 需要知道 Session 的存在
```

#### 方案 B：事件委托（❌ 复杂）
```
优点：完全解耦
缺点：需要额外的委托定义，Blueprint 实现复杂
```

#### 方案 C：通过 Request 传递（✅ 推荐）
```
优点：简单、解耦、Blueprint 友好
缺点：Session 引用在 Request 中传递（可接受）
```

## 完整示例

### C++ 中的使用

```cpp
// Session 内部（已实现）
FNarrRailChoiceRequest Request;
Request.NodeId = TargetNodeId;
Request.Choices = Node->Choices;
Request.Session = this;  // 传入自己
INarrRailDialoguePresenterInterface::Execute_ShowChoices(DialoguePresenter.GetObject(), Request);
```

### Blueprint 中的实现

**UI Widget (WBP_Dialogue_ADV):**

变量：
- `CurrentChoiceRequest`: FNarrRailChoiceRequest
- `ChoiceButtonArray`: Array of Button Widgets

函数：
```
ShowChoices (Request: FNarrRailChoiceRequest)
├─ Set CurrentChoiceRequest = Request
├─ Clear ChoiceButtonArray
├─ ForEachLoop: Request.Choices
│   ├─ Create Widget: WBP_ChoiceButton
│   ├─ Set Button Text = Choice.TextKey
│   ├─ Bind OnClicked → OnChoiceButtonClicked(LoopIndex)
│   ├─ Add to ChoiceButtonArray
│   └─ Add to ChoiceContainer
└─ Show ChoiceContainer

OnChoiceButtonClicked (Index: int)
├─ IsValid? CurrentChoiceRequest.Session
│   ├─ Yes: CurrentChoiceRequest.Session → Choose(Index)
│   └─ No: Print Error "Session is null"
├─ Hide ChoiceContainer
└─ Clear CurrentChoiceRequest
```

### 外部使用（游戏逻辑）

```
Event BeginPlay
├─ Create Object: UNarrRailStorySession → MySession
├─ MySession.Initialize(StoryAsset)
├─ Create Widget: WBP_Dialogue_ADV → MyUI
├─ MyUI.AddToViewport()
├─ MySession.RegisterDialoguePresenter(MyUI)
└─ MySession.Start()

// 不需要额外的绑定！
// UI 会自动通过 Request.Session 调用 Choose
```

## 总结

通过在 `FNarrRailChoiceRequest` 中传递 Session 引用，我们实现了：
- UI 组件不需要外部注入 Session
- 保持了接口的解耦性
- Blueprint 实现简单直观
- 符合 UE 的设计模式

这是一个在解耦和实用性之间的最佳平衡点。
