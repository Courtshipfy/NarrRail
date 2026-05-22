# NarrRail

[English](./README.en.md)

NarrRail 是面向 AVG（Adventure / Visual Novel）游戏的 UE5.7 插件，提供剧情运行时、脚本导入导出与流程调试能力。

## 功能概览

- ✅ **运行时核心**：状态机、变量、条件、动作
- ✅ **Blueprint API**：完整事件系统与辅助函数
- ✅ **脚本校验**：C# CLI 校验工具
- ✅ **YAML 导入**：支持将脚本拖拽导入 UE 内容浏览器
- ✅ **调试能力**：屏幕 HUD 与控制台命令
- 🚧 **存档/读档**：规划中
- ✅ **可视化编辑器（WebEditor）**：MVP 已可用
- 🚧 **YAML 导出**：进行中

## 仓库结构

- `NarrRail/`：插件源码
  - `Source/NarrRail/`：运行时模块（C++）
  - `Source/NarrRailEditor/`：编辑器模块（C++）
- `HostProject/`：开发宿主工程
- `Tools/NarrRail.Tooling/`：C# CLI 工具（脚本校验等）
- `Docs/`：项目文档

## WebEditor 文档入口

- 中文（默认）：`Tools/NarrRail.WebEditor/README.md`
- English：`Tools/NarrRail.WebEditor/README.en.md`
- 预览模式说明：`Tools/NarrRail.WebEditor/README_READING_MODE.md`
- 部署方案：`Docs/05_webeditor/WEBEDITOR_DEPLOYMENT_PLAN.md`
- 介绍页规范：`Docs/05_webeditor/INTRO_PAGE_CONTENT_STYLE_SPEC.md`

## 快速开始

### 1）脚本校验（CLI）

```bash
cd Tools/NarrRail.Tooling
dotnet run --project src/NarrRail.Tooling -- validate affinity_demo.nrstory
```

详见 `Tools/NarrRail.Tooling/README.md`。

### 2）导入脚本到 UE

1. 使用 `HostProject/NarrRailHost.uproject` 打开 UE 编辑器
2. 将 `Tools/NarrRail.Tooling/affinity_demo.nrstory` 拖入 Content Browser
3. 自动生成 `UNarrRailStoryAsset`

### 3）Blueprint 使用

详见 `Docs/03_ui_blueprint/BLUEPRINT_QUICKSTART.md`。

### 4）运行时调试

详见 `Docs/02_runtime/DEBUGGER_GUIDE.md`。

## 示例脚本

- `Tools/NarrRail.Tooling/test_valid.nrstory`：最小可用示例
- `Tools/NarrRail.Tooling/affinity_demo.nrstory`：完整好感度分支示例

## 核心文档

- `Docs/02_runtime/SCRIPT_FORMAT.md`：脚本格式规范
- `Docs/03_ui_blueprint/BLUEPRINT_QUICKSTART.md`：Blueprint 快速开始
- `Docs/02_runtime/DEBUGGER_GUIDE.md`：调试指南
- `Docs/01_architecture/TECH_ARCHITECTURE.md`：技术架构
- `Docs/06_planning/TASK_PLAN.md`：WBS 任务计划

## 作为项目插件安装

1. 将 `NarrRail` 目录复制到 UE 工程 `Plugins/NarrRail`
2. 生成项目文件
3. 编译工程并在编辑器中启用插件

## 使用 HostProject 开发

1. 使用 UE5.7 打开 `HostProject/NarrRailHost.uproject`
2. 运行 `HostProject/Build-HostProject.cmd` 编译
3. 在 `NarrRail/Source/...` 迭代并重新编译

## 语言策略

- **C++**：运行时与编辑器核心能力
- **C#**：脚本处理、校验、CLI 工具
- **Blueprint**：业务集成层（不承载核心逻辑）
