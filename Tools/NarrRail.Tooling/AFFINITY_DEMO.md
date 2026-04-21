# 好感度系统示例脚本

这个脚本演示了 BLUEPRINT_QUICKSTART.md 步骤 6 中的完整好感度系统。

## 剧情流程

```
N_Start (A: "你好，B！今天天气真不错。")
  ↓
N_B_Reply1 (B: "你好，A。是啊，天气很好。")
  ↓
N_A_Question (A: "要不要一起去散步？")
  ↓
N_B_Choice (选项节点)
  ├─ 选项1: "好啊，我很乐意！" → N_A_Happy (好感度 +10)
  │    ├─ [Affinity >= 10] → N_A_SpecialEnding (特殊结局)
  │    └─ [Affinity < 10]  → N_A_NormalEnding (普通结局)
  │
  └─ 选项2: "不了，我还有事。" → N_A_Sad (好感度 -5)
       └─ N_A_NormalEnding (普通结局)
```

## 运行场景

### 场景 1：选择"好啊，我很乐意！"

1. A: "你好，B！今天天气真不错。"
2. B: "你好，A。是啊，天气很好。"
3. A: "要不要一起去散步？"
4. **[玩家选择]** "好啊，我很乐意！"
5. **好感度变化：** 0 → 10
6. A: "太好了！我很高兴你愿意和我一起。"
7. **条件判断：** Affinity >= 10 ✓
8. A: "和你在一起真开心！我们以后要经常见面。" **(特殊结局)**
9. 会话结束

### 场景 2：选择"不了，我还有事。"

1. A: "你好，B！今天天气真不错。"
2. B: "你好，A。是啊，天气很好。"
3. A: "要不要一起去散步？"
4. **[玩家选择]** "不了，我还有事。"
5. **好感度变化：** 0 → -5
6. A: "好吧...那下次再约吧。"
7. A: "那我先走了，再见。" **(普通结局)**
8. 会话结束

## 关键特性

### 1. 变量系统
- 变量名：`Affinity`
- 类型：`Int`
- 作用域：`Session`（会话级，每次启动重置）
- 默认值：`0`

### 2. 动作系统
- **N_A_Happy** 节点的 `enterActions`：好感度 +10
- **N_A_Sad** 节点的 `enterActions`：好感度 -5

### 3. 条件分支
- **N_A_Happy → N_A_SpecialEnding**：条件 `Affinity >= 10`，优先级 0
- **N_A_Happy → N_A_NormalEnding**：条件 `Affinity < 10`，优先级 1

优先级数值越小越优先，所以会先检查 `>= 10` 的条件。

## 验证脚本

```bash
narrrail validate affinity_demo.narrrail.yaml
```

输出：
```
Validating: affinity_demo.narrrail.yaml

✓ Validation passed
```

## 在 UE 中使用

1. 将此脚本导入为 UE 资产（待实现 M2.3）
2. 或在蓝图中按照 BLUEPRINT_QUICKSTART.md 手动创建
3. 创建会话并启动
4. 绑定事件监听节点进入和选项准备
5. 在 PIE 中测试两种分支

## 扩展建议

### 多次对话累积好感度

将变量作用域改为 `Global`：

```yaml
variables:
  - name: Affinity
    type: Int
    scope: Global  # 改为 Global
    defaultValue: "0"
```

这样好感度会在多次会话间保持，可以创建多个剧情资产，累积好感度。

### 更复杂的条件

使用 `Any` 逻辑和多个条件项：

```yaml
condition:
  logic: Any  # 满足任一条件即可
  terms:
    - variable:
        name: Affinity
        type: Int
        scope: Session
      operator: ">="
      compareValue: "10"
    - variable:
        name: IsVIP
        type: Bool
        scope: Global
      operator: "=="
      compareValue: "true"
```

### 更多动作类型

```yaml
enterActions:
  - actionType: Set  # 直接设置值
    variable:
      name: Affinity
      type: Int
      scope: Session
    value: "100"
    eventId: ""
  
  - actionType: EmitEvent  # 触发自定义事件
    variable: null
    value: ""
    eventId: "OnSpecialMoment"
```
