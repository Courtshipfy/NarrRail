# 蓝图快速开始指南：创建简单对话

本指南将教你如何在蓝图中创建一个 A 和 B 互相对话的简单场景。

## 前提条件

- 已编译 NarrRail 插件
- 在 UE5.7 编辑器中打开 HostProject

## 步骤 1：创建对话资产（在蓝图中）

在你的 Actor 或 GameMode 蓝图的 BeginPlay 事件中：

### 1.1 创建空资产

```
节点：Create Story Asset
- Story Id: "SimpleDialogue"
- Entry Node Id: "Start"
```

### 1.2 添加对话节点

```
节点：Add Dialogue Node (连续添加多个)

节点 1:
- Node Id: "Start"
- Speaker Id: "A"
- Text Key: "你好，B！今天天气真不错。"

节点 2:
- Node Id: "B_Reply1"
- Speaker Id: "B"
- Text Key: "是啊，A。我也这么觉得。"

节点 3:
- Node Id: "A_Reply2"
- Speaker Id: "A"
- Text Key: "要不要一起去散步？"

节点 4:
- Node Id: "B_Reply2"
- Speaker Id: "B"
- Text Key: "好主意！我们走吧。"
```

### 1.3 添加结束节点

```
节点：Add End Node
- Node Id: "End"
```

### 1.4 连接节点（添加边）

```
节点：Add Edge (连续添加多个)

边 1: Start -> B_Reply1
边 2: B_Reply1 -> A_Reply2
边 3: A_Reply2 -> B_Reply2
边 4: B_Reply2 -> End
```

## 步骤 2：创建会话并启动

### 2.1 创建会话

```
节点：Create Story Session
- Story Asset: (连接步骤 1 创建的资产)
保存返回值到变量：Story Session
```

### 2.2 绑定事件

```
节点：Bind Event to On Node Entered (Story Session)
创建自定义事件：OnDialogueNodeEntered
参数：
- Node Id (Name)
- Node (NarrRail Node)
```

### 2.3 启动会话

```
节点：Start (Story Session)
```

## 步骤 3：显示对话内容

在 `OnDialogueNodeEntered` 自定义事件中：

### 3.1 检查节点类型

```
节点：Is Dialogue Node
- Node: (事件参数)
```

### 3.2 获取对话内容

```
节点：Get Dialogue From Node
- Node: (事件参数)
返回：Dialogue Payload (包含 Speaker Id 和 Text Key)
```

### 3.3 显示到 UI

```
节点：Print String (临时测试用)
- In String: 格式化字符串 "{Speaker Id}: {Text Key}"

或者：
更新你的 UI Widget，显示说话人和对话文本
```

### 3.4 推进到下一句

```
添加延迟或按键输入后：
节点：Next (Story Session)
```

## 完整蓝图流程示例

```
BeginPlay
  ↓
Create Story Asset
  ↓
Add Dialogue Node (Start, A, "你好...")
  ↓
Add Dialogue Node (B_Reply1, B, "是啊...")
  ↓
Add Dialogue Node (A_Reply2, A, "要不要...")
  ↓
Add Dialogue Node (B_Reply2, B, "好主意...")
  ↓
Add End Node (End)
  ↓
Add Edge (Start -> B_Reply1)
  ↓
Add Edge (B_Reply1 -> A_Reply2)
  ↓
Add Edge (A_Reply2 -> B_Reply2)
  ↓
Add Edge (B_Reply2 -> End)
  ↓
Create Story Session
  ↓
Bind Event to On Node Entered
  ↓
Start Session

---

OnDialogueNodeEntered 事件:
  ↓
Is Dialogue Node?
  ↓ (Yes)
Get Dialogue From Node
  ↓
Print String: "{Speaker}: {Text}"
  ↓
Delay 2 seconds (或等待玩家输入)
  ↓
Next (推进到下一句)
```

## 步骤 4：运行测试

1. 点击 Play (PIE)
2. 你应该看到对话依次显示：
   - A: 你好，B！今天天气真不错。
   - B: 是啊，A。我也这么觉得。
   - A: 要不要一起去散步？
   - B: 好主意！我们走吧。
   - (会话结束)

## 步骤 5：添加分支选项

现在让我们在对话中添加一个选择分支，让玩家可以选择不同的回应。

### 5.1 修改对话流程

我们将在 A 问"要不要一起去散步？"之后，让 B 可以选择"好啊"或"不了"。

在步骤 1 的基础上，修改节点创建：

```
保留前面的节点：
- Start (A: "你好，B！今天天气真不错。")
- B_Reply1 (B: "是啊，A。我也这么觉得。")
- A_Reply2 (A: "要不要一起去散步？")

删除 B_Reply2，改为添加选项节点：
```

### 5.2 创建选项节点

```
节点：Add Choice Node
- Node Id: "B_Choice"
- Choices: (使用 Make Array 创建选项数组)

选项 1:
  节点：Make Simple Choice
  - Text Key: "好啊，我们走吧！"
  - Target Node Id: "A_Happy"

选项 2:
  节点：Make Simple Choice
  - Text Key: "不了，我还有事。"
  - Target Node Id: "A_Sad"
```

### 5.3 添加分支后的对话节点

```
节点：Add Dialogue Node

节点 1 (选择"好啊"后):
- Node Id: "A_Happy"
- Speaker Id: "A"
- Text Key: "太好了！走吧。"

节点 2 (选择"不了"后):
- Node Id: "A_Sad"
- Speaker Id: "A"
- Text Key: "好吧，那下次再约。"
```

### 5.4 修改边连接

```
节点：Add Edge

保留前面的边：
边 1: Start -> B_Reply1
边 2: B_Reply1 -> A_Reply2

修改后续的边：
边 3: A_Reply2 -> B_Choice (连接到选项节点)
边 4: A_Happy -> End
边 5: A_Sad -> End

注意：选项节点的跳转由选项本身的 Target Node Id 控制，不需要手动添加边
```

### 5.5 绑定选项事件

在步骤 2 的基础上，添加新的事件绑定：

```
节点：Bind Event to On Choices Ready (Story Session)
创建自定义事件：OnChoicesReady
参数：
- Node Id (Name)
- Choices (Array of NarrRail Choice Option)
```

### 5.6 显示选项 UI

在 `OnChoicesReady` 自定义事件中：

```
ForEach Loop (遍历 Choices 数组)
  ↓
创建按钮 Widget
  ↓
设置按钮文本 = Choice.TextKey
  ↓
绑定按钮点击事件 → 调用 Choose(Session, Array Index)
  ↓
添加按钮到 UI 容器
```

简化版（用于测试）：

```
OnChoicesReady 事件:
  ↓
Print String: "可用选项："
  ↓
ForEach (Choices)
  ↓
Print String: "{Index}: {Choice.TextKey}"
  ↓
等待玩家输入（例如按键 1 或 2）
  ↓
Choose (Session, 选择的索引)
```

### 5.7 处理选项选择

```
玩家按键 1:
  ↓
Choose (Session, 0)  // 选择第一个选项

玩家按键 2:
  ↓
Choose (Session, 1)  // 选择第二个选项
```

## 完整分支对话流程示例

```
BeginPlay
  ↓
Create Story Asset
  ↓
Add Dialogue Node (Start, A, "你好...")
  ↓
Add Dialogue Node (B_Reply1, B, "是啊...")
  ↓
Add Dialogue Node (A_Reply2, A, "要不要...")
  ↓
Add Choice Node (B_Choice, [选项1: "好啊" -> A_Happy, 选项2: "不了" -> A_Sad])
  ↓
Add Dialogue Node (A_Happy, A, "太好了...")
  ↓
Add Dialogue Node (A_Sad, A, "好吧...")
  ↓
Add End Node (End)
  ↓
Add Edge (Start -> B_Reply1)
  ↓
Add Edge (B_Reply1 -> A_Reply2)
  ↓
Add Edge (A_Reply2 -> B_Choice)
  ↓
Add Edge (A_Happy -> End)
  ↓
Add Edge (A_Sad -> End)
  ↓
Create Story Session
  ↓
Bind Event to On Node Entered
  ↓
Bind Event to On Choices Ready
  ↓
Start Session

---

OnDialogueNodeEntered 事件:
  ↓
Get Dialogue From Node
  ↓
Print String: "{Speaker}: {Text}"
  ↓
Delay 2 seconds
  ↓
Next

---

OnChoicesReady 事件:
  ↓
ForEach (Choices)
  ↓
Print String: "{Index}: {TextKey}"
  ↓
等待玩家按键 1 或 2
  ↓
Choose (Session, 选择的索引)
```

## 运行效果

1. 点击 Play (PIE)
2. 对话流程：
   - A: 你好，B！今天天气真不错。
   - B: 是啊，A。我也这么觉得。
   - A: 要不要一起去散步？
   - **[选项出现]**
     - 0: 好啊，我们走吧！
     - 1: 不了，我还有事。
   - **[玩家选择 0]** → A: 太好了！走吧。
   - **[玩家选择 1]** → A: 好吧，那下次再约。
   - (会话结束)

## 可用的蓝图节点

### 资产创建
- `Create Story Asset` - 创建剧情资产
- `Add Dialogue Node` - 添加对话节点
- `Add Choice Node` - 添加选择节点
- `Add End Node` - 添加结束节点
- `Add Edge` - 连接节点
- `Make Simple Choice` - 创建简单选项

### 会话控制
- `Create Story Session` - 创建会话
- `Initialize` - 初始化会话
- `Start` - 启动会话
- `Next` - 推进到下一个节点
- `Choose` - 选择一个选项
- `Pause` - 暂停会话
- `Resume` - 恢复会话
- `Stop` - 停止会话

### 查询接口
- `Get Current Node` - 获取当前节点
- `Get Current Choices` - 获取当前选项
- `Get Session State` - 获取会话状态
- `Get Variable Container` - 获取变量容器
- `Get Variable Bool/Int/Float/String` - 获取变量值

### 事件
- `On Session Started` - 会话启动
- `On Node Entered` - 节点进入
- `On Node Exited` - 节点退出
- `On Session Ended` - 会话结束
- `On Choices Ready` - 选项准备就绪
- `On Choice Selected` - 选项被选择

### 辅助函数
- `Is Result Success` - 检查结果是否成功
- `Is Result Completed` - 检查是否完成
- `Get Dialogue From Node` - 获取对话内容
- `Is Dialogue Node` - 是否为对话节点
- `Is Choice Node` - 是否为选择节点
- `Is End Node` - 是否为结束节点

## 下一步

- 添加 UI Widget 显示对话框
- 添加角色头像和名字
- 添加选择分支
- 添加变量和条件判断
- 保存和加载进度

## 注意事项

1. 所有节点 ID 必须唯一
2. Entry Node Id 必须存在于节点列表中
3. 边的 Source 和 Target 必须是有效的节点 ID
4. 记得在 Next 之前检查会话状态
5. 使用事件系统可以实现更灵活的 UI 更新

## 步骤 6：添加好感度系统

现在让我们添加一个好感度系统，根据玩家的选择改变好感度，并根据好感度值显示不同的剧情分支。

### 6.1 创建变量

在创建资产后，添加变量定义：

```
节点：Add Variable Definition (Story Asset)
- Variable Name: "Affinity"
- Variable Type: Int
- Scope: Session
- Default Value: "0"
```

### 6.2 修改对话流程（带好感度）

我们将创建一个完整的好感度示例：

```
Start (A: "你好，B！")
  ↓
B_Reply1 (B: "你好，A。")
  ↓
A_Question (A: "今天天气不错，要一起去散步吗？")
  ↓
B_Choice (选项节点)
  选项 1: "好啊，我很乐意！" → A_Happy (好感度 +10)
  选项 2: "不了，我还有事。" → A_Sad (好感度 -5)
  ↓
A_CheckAffinity (检查好感度)
  如果 Affinity >= 10 → A_SpecialEnding (特殊结局)
  如果 Affinity < 10 → A_NormalEnding (普通结局)
  ↓
End
```

### 6.3 创建节点

```
节点：Add Dialogue Node

节点 1:
- Node Id: "Start"
- Speaker Id: "A"
- Text Key: "你好，B！今天天气真不错。"

节点 2:
- Node Id: "B_Reply1"
- Speaker Id: "B"
- Text Key: "你好，A。是啊，天气很好。"

节点 3:
- Node Id: "A_Question"
- Speaker Id: "A"
- Text Key: "要不要一起去散步？"

节点 4:
- Node Id: "A_Happy"
- Speaker Id: "A"
- Text Key: "太好了！我很高兴你愿意和我一起。"

节点 5:
- Node Id: "A_Sad"
- Speaker Id: "A"
- Text Key: "好吧...那下次再约吧。"

节点 6:
- Node Id: "A_SpecialEnding"
- Speaker Id: "A"
- Text Key: "和你在一起真开心！我们以后要经常见面。"

节点 7:
- Node Id: "A_NormalEnding"
- Speaker Id: "A"
- Text Key: "那我先走了，再见。"
```

### 6.4 创建选项节点（带动作）

```
节点：Add Choice Node
- Node Id: "B_Choice"
- Choices: (使用 Make Array)

选项 1:
  Make Simple Choice
  - Text Key: "好啊，我很乐意！"
  - Target Node Id: "A_Happy"

选项 2:
  Make Simple Choice
  - Text Key: "不了，我还有事。"
  - Target Node Id: "A_Sad"
```

### 6.5 添加动作到节点

**给 A_Happy 节点添加进入动作（好感度 +10）：**

```
节点：Add Node Enter Action
- Story Asset: (你的剧情资产)
- Node Id: "A_Happy"
- Action: (使用 Make Node Action 创建)

Make Node Action:
  - Action Type: Add
  - Variable: (使用 Make Variable Ref 创建)
    - Variable Name: "Affinity"
    - Variable Type: Int
    - Global Scope: false (不勾选)
  - Value: "10"
```

**给 A_Sad 节点添加进入动作（好感度 -5）：**

```
节点：Add Node Enter Action
- Story Asset: (你的剧情资产)
- Node Id: "A_Sad"
- Action: (使用 Make Node Action 创建)

Make Node Action:
  - Action Type: Add
  - Variable: (使用 Make Variable Ref 创建)
    - Variable Name: "Affinity"
    - Variable Type: Int
    - Global Scope: false (不勾选)
  - Value: "-5"
```

注意：Value 使用字符串 "-5" 表示减少 5 点好感度。

### 6.6 添加条件边

```
节点：Add Edge With Condition

边 1: Start -> B_Reply1 (无条件)
  使用普通的 Add Edge

边 2: B_Reply1 -> A_Question (无条件)
  使用普通的 Add Edge

边 3: A_Question -> B_Choice (无条件)
  使用普通的 Add Edge

边 4: A_Happy -> A_SpecialEnding (条件: Affinity >= 10)
  - Story Asset: (你的剧情资产)
  - Source Node Id: "A_Happy"
  - Target Node Id: "A_SpecialEnding"
  - Priority: 0
  - Condition: (使用 Make Condition Expression 创建)
    - Logic: All
    - Terms: (Make Array，包含一个 Make Condition Term)
      
      Make Condition Term:
        - Variable: Make Variable Ref
          - Variable Name: "Affinity"
          - Variable Type: Int
          - Global Scope: false
        - Operator: >= (GreaterOrEqual)
        - Compare Value: "10"

边 5: A_Happy -> A_NormalEnding (条件: Affinity < 10)
  - Story Asset: (你的剧情资产)
  - Source Node Id: "A_Happy"
  - Target Node Id: "A_NormalEnding"
  - Priority: 1
  - Condition: (使用 Make Condition Expression 创建)
    - Logic: All
    - Terms: (Make Array，包含一个 Make Condition Term)
      
      Make Condition Term:
        - Variable: Make Variable Ref
          - Variable Name: "Affinity"
          - Variable Type: Int
          - Global Scope: false
        - Operator: < (Less)
        - Compare Value: "10"

边 6: A_Sad -> A_NormalEnding (无条件)
  使用普通的 Add Edge

边 7: A_SpecialEnding -> End (无条件)
  使用普通的 Add Edge

边 8: A_NormalEnding -> End (无条件)
  使用普通的 Add Edge
```

### 6.7 显示好感度值（可选）

在 OnNodeEntered 事件中添加好感度显示：

```
OnNodeEntered 事件
  ↓
Get Variable Int (Session, "Affinity")
  ↓
Print String: "当前好感度: {Affinity}"
  ↓
(继续原有的对话显示逻辑)
```

## 好感度系统运行效果

**场景 1：选择"好啊，我很乐意！"**
```
1. A: "你好，B！今天天气真不错。"
2. B: "你好，A。是啊，天气很好。"
3. A: "要不要一起去散步？"
4. [选项] 玩家选择: "好啊，我很乐意！"
5. 好感度 +10 (0 → 10)
6. A: "太好了！我很高兴你愿意和我一起。"
7. (检查条件: Affinity >= 10 ✓)
8. A: "和你在一起真开心！我们以后要经常见面。" (特殊结局)
9. (会话结束)
```

**场景 2：选择"不了，我还有事。"**
```
1. A: "你好，B！今天天气真不错。"
2. B: "你好，A。是啊，天气很好。"
3. A: "要不要一起去散步？"
4. [选项] 玩家选择: "不了，我还有事。"
5. 好感度 -5 (0 → -5)
6. A: "好吧...那下次再约吧。"
7. (检查条件: Affinity < 10 ✓)
8. A: "那我先走了，再见。" (普通结局)
9. (会话结束)
```

## 进阶：多次对话累积好感度

如果想实现多次对话累积好感度，需要：

1. **使用全局变量**：
```
Add Variable Definition
- Variable Name: "Affinity"
- Variable Type: Int
- Scope: Global  ← 改为 Global
- Default Value: "0"
```

2. **不要在 Start 时重置变量**：
   - 全局变量会在多次会话间保持
   - 会话变量每次 Start 都会重置

3. **创建多个剧情资产**：
   - 第一次见面的对话
   - 第二次见面的对话
   - 根据累积好感度显示不同内容

## 注意事项

1. 所有节点 ID 必须唯一
2. Entry Node Id 必须存在于节点列表中
3. 边的 Source 和 Target 必须是有效的节点 ID
4. 记得在 Next 之前检查会话状态
5. 使用事件系统可以实现更灵活的 UI 更新
