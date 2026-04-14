# NarrRail 技术架构文档

> 版本：v0.1（2026-03-16）
> 适用范围：`I:\NarrRail`
> 文档目标：作为 NarrRail 插件的技术总览与持续维护文档，清晰梳理框架、职责、接口与开发约束。

## 1. 技术路线与语言策略

### 1.1 语言主次约束（强制）

- 主实现语言：`C++`（UE Runtime + UE Editor 核心能力）
- 主工具语言：`C#`（脚本处理、导入导出、批处理校验、构建辅助工具）
- 辅助语言：`Blueprint`（仅用于业务接入层、事件订阅层、UI 绑定层）
- 禁止做法：将核心执行逻辑、条件求值、存档恢复等关键能力下沉到蓝图。

### 1.2 分工原则

- C++：负责运行时正确性、性能、内存安全、UE 资产系统与编辑器扩展。
- C#：负责离线工具链、CLI、脚本格式处理、自动化流水线辅助能力。
- Blueprint：负责调用已暴露 API，不承载核心业务状态机。

## 2. 插件总体框架

## 2.1 当前模块（已存在）

- `NarrRail`：Runtime 模块（游戏运行时）
- `NarrRailEditor`：Editor 模块（编辑器功能）

## 2.2 目标模块（规划）

- `NarrRail`（C++）：剧情数据模型、状态机执行器、变量系统、存档系统、事件桥接
- `NarrRailEditor`（C++）：剧情图编辑器、属性面板、校验器、PIE 调试桥
- `NarrRail.Tooling`（C#）：剧本 DSL/JSON 校验、导入导出 CLI、Round-trip 校验、本地化抽取
- `NarrRail.Tests`（C++ + C#）：自动化测试与回归工具

## 2.3 建议目录结构（目标态）

```text
NarrRail/
  Source/
    NarrRail/                 # C++ Runtime
      Public/
      Private/
    NarrRailEditor/           # C++ Editor
      Public/
      Private/
  Tools/
    NarrRail.Tooling/         # C# CLI/工具链
      src/
      tests/
  Content/                    # 示例内容（可选）
docs/
  TASK_PLAN.md
  TECH_ARCHITECTURE.md
```

## 3. 分层设计

### 3.1 Runtime 分层（C++）

- `Domain`：节点、边、变量、条件、动作、剧情资产定义
- `Execution`：状态机（Start/Next/Choose/Stop/Pause/Resume）
- `State`：会话上下文、历史轨迹、运行时缓存
- `Persistence`：存档读写、版本迁移、异常恢复
- `Bridge`：Blueprint API 暴露与事件委托

### 3.2 Editor 分层（C++）

- `AssetEditor`：自定义资产编辑器壳
- `Graph`：节点创建、连线规则、复制粘贴、布局
- `Inspector`：属性编辑与批量操作
- `Validator`：结构校验、内容校验、问题导航
- `DebugBridge`：PIE 运行高亮、变量观察、单步调试

### 3.3 Tooling 分层（C#）

- `Parser`：DSL/JSON 解析
- `Schema`：格式约束与版本管理
- `Importer/Exporter`：文件与 UE 资产数据映射
- `Localization`：文本键抽取与语言包处理
- `CLI`：命令入口（validate/import/export/roundtrip）

## 4. 关键数据流

### 4.1 运行时数据流

1. 加载剧情资产（C++）
2. 资产预校验（C++）
3. 启动会话上下文（C++）
4. 状态机推进与条件求值（C++）
5. 触发事件给 Blueprint/UI（Bridge）
6. 持久化存档（C++）

### 4.2 脚本工作流

1. 编剧产出 DSL/JSON（文本）
2. C# 工具执行 `validate/import`
3. 导入为 UE 资产（C++ Editor）
4. 编辑器内调整与校验（C++ Editor）
5. 导出脚本 `export`
6. 执行 Round-trip 一致性检查（C#）

## 5. 接口与边界约束

### 5.1 C++ 对 Blueprint 暴露最小集

- `StartSession`
- `Next`
- `Choose`
- `SaveSession`
- `LoadSession`
- `GetCurrentNode`
- `GetVariable`

说明：Blueprint 仅组合调用，不实现状态机规则。

### 5.2 C# CLI 标准命令（目标）

- `narrrail validate <script-path>`
- `narrrail import <script-path> --out <asset-path>`
- `narrrail export <asset-path> --out <script-path>`
- `narrrail roundtrip <script-path>`
- `narrrail l10n extract <script-path>`

## 6. 质量与可维护性约束

### 6.1 编码要求

- Runtime/Editor 关键逻辑必须 C++ 单元/功能测试覆盖。
- C# 工具必须有解析与 round-trip 测试。
- 跨模块接口必须有错误码与日志。
- 任何 breaking change 必须更新格式版本与迁移说明。

### 6.2 性能要求（初版基线）

- 1000 节点剧情加载时间：需建立并持续跟踪基线。
- 条件求值：支持热路径优化与缓存。
- 编辑器大图操作：需跟踪交互响应时间。

## 7. 与任务计划联动（强制更新）

每次开发完成任务后，必须同步更新本文档以下部分：

- `8. 架构状态追踪`：更新子系统状态与实现说明
- `9. 接口变更日志`：记录新增/修改/废弃接口
- `10. 技术决策记录`：记录关键技术取舍

## 8. 架构状态追踪

| 子系统 | 目标实现语言 | 当前状态 | 对应任务 | 备注 |
|---|---|---|---|---|
| Runtime 数据模型 | C++ | 已完成 | NR-RUN-001-* | 节点/边/对白/选项/条件/动作/资产类 |
| Runtime 执行器 | C++ | 已完成 | NR-RUN-002-* | Start/Next/Choose/Pause/Resume/Stop |
| 变量与条件 | C++ | 基本完成 | NR-RUN-003-* | 容器/作用域/通知/比较/逻辑运算，缓存优化待实现 |
| Blueprint 接入层 | C++ | 已完成 | NR-RUN-005-* | 事件委托、蓝图函数库、完整 API 暴露 |
| 存档恢复 | C++ | 规划中 | NR-RUN-006-* | |
| 编辑器图编辑 | C++ | 规划中 | NR-ED-002-* | |
| 编辑器校验器 | C++ | 规划中 | NR-ED-004-* | |
| 工具链 CLI | C# | 规划中 | NR-IO-002/003/004/005-* | |

## 9. 接口变更日志

| 日期 | 模块 | 变更类型 | 接口/命令 | 描述 | 兼容性影响 |
|---|---|---|---|---|---|
| 2026-03-16 | Docs | 新增 | TECH_ARCHITECTURE v0.1 | 初版架构文档建立 | 无 |
| 2026-04-14 | Runtime | 新增 | UNarrRailVariableContainer | 统一变量容器类，支持类型安全读写、作用域、变更通知 | 无，新增功能 |
| 2026-04-14 | Runtime | 新增 | FNarrRailVariableDefinition | 变量定义结构，包含类型/作用域/默认值 | 无，新增功能 |
| 2026-04-14 | Runtime | 新增 | FNarrRailVariableResult | 变量操作结果结构，包含错误码和错误信息 | 无，新增功能 |
| 2026-04-14 | Runtime | 新增 | ENarrRailVariableError | 变量操作错误码枚举 | 无，新增功能 |
| 2026-04-14 | Runtime | 修改 | UNarrRailStorySession | 集成 UNarrRailVariableContainer，新增便捷变量访问接口 | 向后兼容，Context.VariableSnapshot 保留用于存档 |
| 2026-04-14 | Runtime | 新增 | 运行时事件委托 | OnSessionStarted/OnNodeEntered/OnNodeExited/OnSessionEnded/OnChoicesReady/OnChoiceSelected | 无，新增功能 |
| 2026-04-14 | Runtime | 新增 | UNarrRailBlueprintLibrary | 蓝图函数库，提供资产创建和会话管理辅助函数 | 无，新增功能 |

## 10. 技术决策记录（ADR 简版）

### ADR-001：核心逻辑使用 C++，Blueprint 仅做接入层

- 日期：2026-03-16
- 决策：状态机、条件系统、存档恢复、资产校验均由 C++ 实现。
- 原因：性能、可测性、可维护性、可控错误处理。
- 影响：Blueprint 研发速度会慢于“全蓝图”，但长期质量更可控。

### ADR-002：脚本工具链采用 C# 独立 CLI

- 日期：2026-03-16
- 决策：解析、校验、导入导出辅助能力通过 C# CLI 提供。
- 原因：文本处理生态更成熟，便于 CI 集成与批处理。
- 影响：需要维护跨语言数据契约与版本兼容。

### ADR-003：变量系统采用独立容器类 + 类型安全接口

- 日期：2026-04-14
- 决策：创建 `UNarrRailVariableContainer` 独立类管理变量，提供类型安全的读写接口，而非直接操作 `TMap<FName, FString>`。
- 原因：
  1. 类型安全：编译期检查类型匹配，避免运行时类型错误
  2. 错误处理：统一的错误码和错误信息，便于调试和用户反馈
  3. 作用域管理：支持全局/会话级变量隔离，`ResetSessionVariables()` 仅重置会话变量
  4. 变更通知：`OnVariableChanged` 委托支持 UI 实时更新和调试观察
  5. 可扩展性：未来可添加条件缓存、变量验证、序列化优化等功能
- 影响：
  - 正面：代码更安全、更易维护、错误信息更清晰
  - 负面：增加一层抽象，略微增加内存开销（可接受）
  - 兼容性：`Context.VariableSnapshot` 保留用于存档系统，通过 `GetSnapshot()/RestoreFromSnapshot()` 同步

## 11. 本周更新模板（复制使用）

```markdown
### 周更新（YYYY-MM-DD）
- 完成任务：
- 变更模块：
- 新增/调整接口：
- 测试结果：
- 风险与阻塞：
- 下周计划：
```

### 周更新（2026-03-17）
- 完成任务：`NR-RUN-001` 首批代码骨架（01/02/03/04/08/10）
- 变更模块：`NarrRail` Runtime
- 新增/调整接口：`UNarrRailStoryAsset`、`UNarrRailStoryValidator::ValidateStoryAsset`、`ENarrRailNodeType` 等数据结构
- 测试结果：完成静态代码自检；完整 UE 编译需在本机环境执行
- 风险与阻塞：当前会话运行 UBT 存在日志权限限制
- 下周计划：完成 `NR-RUN-001` 余项并开始 `NR-RUN-002` 执行器
- 进展补充：`UNarrRailStoryAsset` 新增版本迁移入口（`PostLoad`），数据契约新增变量与动作结构。
- 进展补充：Runtime 新增 `UNarrRailStorySession`，对外提供最小流程推进接口（Start/Next/Choose/Stop）。
- 进展补充：Runtime 会话执行器新增条件求值、动作执行、Pause/Resume；M2.1 脚本规范文档已建立（`docs/SCRIPT_FORMAT.md`）。
