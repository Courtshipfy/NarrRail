# NarrRail 宿主项目指南

## 目标

在同一仓库中提供 UE5.7 宿主项目，便于你在编辑器中运行、调试和验证插件，同时保持插件代码单一事实来源。

## 目录结构

- `NarrRail/`：插件源码（单一事实来源）
- `NarrRailUEHost/`：用于编辑器内测试的宿主项目
- `NarrRailUEHost/Build-NarrRailUEHost.cmd`：双击即可执行的构建入口（仅 CMD）

## 一次性设置

无需额外的独立初始化脚本。

`Build-NarrRailUEHost.cmd` 会自动检查并创建：

- `NarrRailUEHost/Plugins/NarrRail` -> `NarrRail`（Junction）

推荐先设置一次 UE5.7 引擎路径环境变量：

- `setx UE57_ROOT "I:\UE_5.7"`

说明：脚本会优先读取 `UE57_ROOT`；若未设置，则回退使用 `I:\UE_5.7`。

## 日常工作流

1. 打开 `NarrRailUEHost/NarrRailUEHost.uproject`。
2. 在 `NarrRail/Source/...` 中修改代码。
3. 双击 `NarrRailUEHost/Build-NarrRailUEHost.cmd` 编译。
4. 在编辑器中使用 PIE 或测试地图验证功能。

## 构建入口

- `NarrRailUEHost/Build-NarrRailUEHost.cmd`

默认行为：

- 引擎根路径：优先 `UE57_ROOT`，否则 `I:\UE_5.7`
- 构建目标：`UnrealEditor Win64 Development`
- 工程路径：`NarrRailUEHost/NarrRailUEHost.uproject`
- 若插件链接缺失：自动创建 Junction

## 常见问题

### 为什么只提供 CMD 入口

为了避免依赖缺失的 `.ps1` 文件，并保证从资源管理器可直接运行构建流程。