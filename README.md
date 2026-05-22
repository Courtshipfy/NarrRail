# NarrRail

[English](./README.en.md)

NarrRail 是一个面向 AVG（Adventure / Visual Novel）制作流程的工具链，当前优先提供 **NarrRailEditor 创作端**，并配套 **UE5.7 插件运行端**。

---

## 1. NarrRailEditor

NarrRailEditor 是当前最成熟的入口，用于剧情脚本创作、结构校验与预览审校。

### 1.1 你可以做什么

- 节点图编辑（Dialogue / Choice / Jump / SetVariable / EmitEvent / End）
- `.nrstory` 导入导出
- 实时校验 + 手动校验
- 自动保存（localStorage 兜底）
- 预览模式（按运行语义推进，支持 Choice 穷举分支）

### 1.2 快速开始（NarrRailEditor）

线上入口：

- https://narr-rail.courtship.top

建议流程：

1. 打开线上 NarrRailEditor，登录并进入脚本库
2. 新建或打开 `.nrstory`，完成节点编辑与校验
3. 使用预览模式进行流程审校（包含分支与结束路径）
4. 导出 `.nrstory` 并提交到仓库（可选）
5. 在 UE 中导入脚本资产并进行 PIE 调试验证

> 本地开发入口（`npm run dev`）仅用于编辑器功能开发与调试，不作为日常创作入口。

### 1.3 NarrRailEditor 文档入口

- 中文（默认）：`Tools/NarrRail.WebEditor/README.md`
- English：`Tools/NarrRail.WebEditor/README.en.md`
- 预览模式说明：`Tools/NarrRail.WebEditor/README_READING_MODE.md`
- 部署方案：`Docs/05_webeditor/WEBEDITOR_DEPLOYMENT_PLAN.md`
- 介绍页规范：`Docs/05_webeditor/INTRO_PAGE_CONTENT_STYLE_SPEC.md`

---

## 2. UE 插件（运行端）

UE 插件负责运行时执行、蓝图集成、调试与资产导入能力。

### 2.1 当前能力

- ✅ Runtime Core：状态机、变量、条件、动作
- ✅ Blueprint API：事件系统与辅助函数
- ✅ YAML 导入：拖拽脚本到 UE Content Browser
- ✅ Debugger：屏幕 HUD 与控制台命令
- 🚧 Save/Load：规划中

### 2.2 与 NarrRailEditor 的联动流程（UE 侧）

1. 从 NarrRailEditor 导出最新 `.nrstory`
2. 使用 UE5.7 打开 `HostProject/NarrRailHost.uproject`
3. 将脚本导入/同步为 `UNarrRailStoryAsset`
4. 运行 `HostProject/Build-HostProject.cmd`（如需重新编译插件）
5. 在 PIE 中执行剧情并结合调试工具回归验证

### 2.3 脚本校验（CLI）

```bash
cd Tools/NarrRail.Tooling
dotnet run --project src/NarrRail.Tooling -- validate affinity_demo.nrstory
```

详见：`Tools/NarrRail.Tooling/README.md`

---

## 3. 示例脚本

- `Tools/NarrRail.Tooling/test_valid.nrstory`：最小可用示例
- `Tools/NarrRail.Tooling/affinity_demo.nrstory`：完整好感度分支示例

---

## 4. 核心文档

- `Docs/02_runtime/SCRIPT_FORMAT.md`：脚本格式规范
- `Docs/03_ui_blueprint/BLUEPRINT_QUICKSTART.md`：Blueprint 快速开始
- `Docs/02_runtime/DEBUGGER_GUIDE.md`：调试指南
- `Docs/01_architecture/TECH_ARCHITECTURE.md`：技术架构
- `Docs/06_planning/TASK_PLAN.md`：WBS 任务计划

---

## 5. 仓库结构

- `NarrRail/`：插件源码
  - `Source/NarrRail/`：运行时模块（C++）
  - `Source/NarrRailEditor/`：编辑器模块（C++）
- `HostProject/`：开发宿主工程
- `Tools/NarrRail.WebEditor/`：Web 编辑器
- `Tools/NarrRail.Tooling/`：C# CLI 工具（脚本校验等）
- `Docs/`：项目文档

---

## 6. 语言策略

- **C++**：运行时与编辑器核心能力
- **C#**：脚本处理、校验、CLI 工具
- **Blueprint**：业务集成层（不承载核心逻辑）
