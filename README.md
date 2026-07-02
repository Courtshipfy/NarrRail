# NarrRail

[English](./README.en.md)

NarrRail 是一个面向 AVG（Adventure / Visual Novel）制作流程的工具链，当前由 **NarrRailEditor 创作端**、**NarrRail Story Converter Codex Skill**、**UE5.7 插件运行端** 和 **NarrRailUEHost 开发宿主工程** 组成。

## 1. NarrRailEditor

NarrRailEditor 用于剧情脚本创作、结构校验、预览审校和 `.nrstory` 导入导出。

### 当前能力

- 节点图编辑：`Dialogue` / `MultiDialogue` / `Choice` / `Jump` / `SetVariable` / `EmitEvent` / `Condition` / `End`
- Condition 节点多分支编辑：每个条件分支对应 `condition-N` 输出，未命中时走 `condition-fallback`
- 全局变量与预设角色管理
- `.nrstory` 导入导出
- 实时校验 + 手动校验
- 自动保存（localStorage 兜底）
- 预览模式（按运行语义推进，支持 Choice 穷举分支）

### 快速开始

线上入口：

- narr-rail.courtship.top

建议流程：

1. 打开线上 NarrRailEditor，登录并进入脚本库
2. 新建或打开 `.nrstory`，完成节点编辑与校验
3. 使用预览模式审校流程
4. 导出 `.nrstory` 并提交到故事仓库
5. 在 UE 中通过 `Sync Stories` 同步故事仓库并进行 PIE 验证

本地开发入口（`npm run dev`）仅用于编辑器功能开发与调试。详细说明见 `NarrRailEditor/README.md`。

## 2. NarrRail Story Converter Skill

NarrRail Story Converter 是项目级 Codex skill，用于把不同编剧产出的剧情文稿转换为 NarrRail `.nrstory` 文件。它不再作为独立工具目录维护，而是把转换规则、NarrRail 格式约束、项目画像模板和辅助脚本都封装进 skill。

### 目标能力

- 让 Codex 在处理编剧文稿时自动使用 NarrRail 转换流程
- 支持松散文稿、表格、对白稿、场景大纲等不同来源
- 提供项目级转换画像，描述角色、命名规则、分支识别规则和禁止推断项
- 输出 `.nrstory` 与转换说明，并用 NarrRailEditor 校验器验证
- 生成结果兼容现有 UE 插件和 NarrRailEditor 导入流程

### 安装

macOS / Linux / Windows PowerShell:

```shell
node -e "const fs=require('fs'),os=require('os'),path=require('path'),cp=require('child_process');const home=process.env.CODEX_HOME||path.join(os.homedir(),'.codex');const installer=path.join(home,'skills','.system','skill-installer','scripts','install-skill-from-github.py');if(!fs.existsSync(installer)){console.error('Codex skill installer not found: '+installer);process.exit(1)};const base=['--repo','Courtshipfy/NarrRail','--ref','main','--path','.codex/skills/narrrail-story-converter'];for(const c of process.platform==='win32'?[['py',['-3']],['python',[]],['python3',[]]]:[['python3',[]],['python',[]]]){const r=cp.spawnSync(c[0],[...c[1],installer,...base],{stdio:'inherit'});if(!r.error)process.exit(r.status??0)};console.error('Python not found. Install Python 3.');process.exit(1)"
```

安装后重启 Codex，然后可以直接提出类似请求：

```text
把这个编剧初稿转成 NarrRail .nrstory，并生成转换说明。
```

## 3. UE 插件

UE 插件负责运行时执行、蓝图集成、调试和故事仓库同步。

### 当前能力

- Runtime Core：状态机、变量、条件、动作
- Blueprint API：事件系统与辅助函数
- Story Repository Sync：从本地故事 Git 仓库同步 `.nrstory`
- Global Config Import：全局变量与预设角色导入为 `UNarrRailGlobalConfigAsset`
- Debugger：屏幕 HUD 与控制台命令
- Save/Load：规划中

### UE 侧联动流程

1. 将 NarrRailEditor 导出的 `.nrstory` 提交到本地故事 Git 仓库
2. 使用 UE5.7 打开 `NarrRailUEHost/NarrRailUEHost.uproject`
3. 在 `Project Settings > Plugins > NarrRail` 配置 `Story Repository Path`
4. 点击工具栏 `Sync Stories`
5. 插件默认执行 `git pull --ff-only`，随后同步仓库内所有 `.nrstory`
6. 在 PIE 中执行剧情并结合调试工具回归验证

同步规则：

- 默认资产根目录为 `/Game/NarrRailStories`
- 实际同步目录为 `/Game/NarrRailStories/<故事仓库文件夹名>`
- 仓库子目录会完整映射到 UE Content 目录
- 普通剧情文件生成或更新 `UNarrRailStoryAsset`
- `meta.configType: GlobalConfig` 文件生成或更新 `UNarrRailGlobalConfigAsset`
- 仓库中删除的 `.nrstory` 会在同步时删除对应资产，删除前会弹出确认

## 4. NarrRailUEHost 插件放置

开发时需要让 `NarrRailUEHost` 能找到本仓库的 `NarrRail/` 插件源码。

推荐做法：在 `NarrRailUEHost/Plugins/` 下放一个指向仓库根目录 `NarrRail/` 插件源码的目录链接，目标路径为：

```text
NarrRailUEHost/Plugins/NarrRail -> ../../NarrRail
```

`NarrRailUEHost/Build-NarrRailUEHost.cmd` 会自动检查并创建这个 Junction。普通 `.lnk` 快捷方式可能不会被 Unreal Engine 识别。

## 5. 核心文档

- `NarrRailEditor/README.md`：编辑器说明
- `.codex/skills/narrrail-story-converter/SKILL.md`：剧情文稿转换 skill
- `.codex/skills/narrrail-story-converter/references/`：NarrRail 格式与转换策略
- `Docs/02_runtime/SCRIPT_FORMAT.md`：脚本格式规范
- `Docs/03_ui_blueprint/BLUEPRINT_QUICKSTART.md`：Blueprint 快速开始
- `Docs/02_runtime/DEBUGGER_GUIDE.md`：调试指南
- `Docs/04_narrrail_ue_host/NARRRAIL_UE_HOST.md`：UE 宿主工程与故事仓库同步
- `Docs/01_architecture/TECH_ARCHITECTURE.md`：技术架构
- `Docs/06_planning/TASK_PLAN.md`：WBS 任务计划

## 6. 仓库结构

- `NarrRail/`：UE 插件源码
  - `Source/NarrRail/`：运行时模块（C++）
  - `Source/NarrRailEditor/`：编辑器模块（C++）
- `NarrRailUEHost/`：UE 开发宿主工程
- `NarrRailEditor/`：Web 编辑器
- `.codex/skills/narrrail-story-converter/`：剧情文稿到 `.nrstory` 的 Codex skill
- `Docs/`：项目文档

## 7. 语言策略

- **C++**：运行时与编辑器核心能力
- **Blueprint**：业务集成层，不承载核心逻辑
- **TypeScript / JavaScript / Vue / Svelte**：Web 创作端
- **Markdown / JavaScript**：Codex skill、转换规则与辅助脚本
