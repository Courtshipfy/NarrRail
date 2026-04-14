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
