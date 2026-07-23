# NarrRail

[English](./README.en.md)

NarrRail 是一个面向互动叙事创作者的剧本创作与辅助产品。主仓库现在以 **Story Project 创作工作台**、`.nrstory` / `.nroutline` 故事格式、导入审查、预览校验和创作辅助流程为中心。

UE 插件： [`NarrRail-Unreal-Plugin`](https://github.com/Courtshipfy/NarrRail-Unreal-Plugin)  
Godot 插件：[`NarrRail-Godot-Plugin`](https://github.com/Courtshipfy/NarrRail-Godot-Plugin)

# 1. 产品定位

NarrRail 的第一阶段用户是 Narrative Creator：编剧、叙事设计师，或需要组织互动剧情结构的创作者。

核心工作流：

```text
想法 / 大纲
  -> Story Project 组织
  -> 结构化剧本编辑
  -> 分支、变量、条件设计
  -> Project Review Queue 审查
  -> Project Preview 预览
  -> 导出 .nrstory / .nroutline 给 Story Consumer
```

## 2. Story Project

Story Project 是 NarrRail 的项目级创作工作区。第一阶段以 GitHub 仓库或仓库目录作为权威存储。

推荐结构：

```text
Stories/
  main_story.nroutline
  chapter_01.nrstory
Config/
  global-config.nrstory
```

- `.nrstory`：具体故事内容，包含节点、边、对白、选择、条件、变量操作、事件和结束节点。
- `.nroutline`：项目级故事编排，负责把多个 `.nrstory` 组织成章节、路线或总纲。
- `Config/global-config.nrstory`：项目级变量和预设角色配置。

旧的 `.nrrail` 总纲后缀是兼容格式。新文件应优先使用 `.nroutline`。

## 3. 当前创作端

`NarrRailEditor/` 是当前 Vue 3 + Vite 创作端。

当前能力：

- 节点图编辑：`Dialogue` / `MultiDialogue` / `Choice` / `Jump` / `SetVariable` / `EmitEvent` / `Condition` / `End`
- Condition 多分支：每个条件分支对应 `condition-N` 输出，未命中时走 `condition-fallback`
- 全局变量与预设角色管理
- `.nrstory` 导入导出
- `.nroutline` / legacy `.nrrail` 总纲能力正在迁移中
- 实时校验 + 手动校验
- 本地自动保存兜底
- 运行语义预览
- GitHub-backed 脚本库与配置同步能力

本地开发：

```bash
cd NarrRailEditor
npm ci
npm run dev -- --host 127.0.0.1
```

默认地址：`http://127.0.0.1:5173/`

验证：

```bash
cd NarrRailEditor
npm test
```

## 4. Script Conversion

NarrRail Story Converter 是项目级 Codex skill，用于把编剧提供的 Word、Excel、CSV、TXT、剧本文稿、场景大纲或松散文稿转换为 NarrRail 结构化故事文件。

短期目标：

- 从源文档生成 `.nrstory`
- 输出 `conversion-notes.md`
- 保守处理不确定内容
- 通过 NarrRail 校验规则检查结构

中期方向：

- 输出 Story Project Import Package
- 支持 `Config/conversion-profile.md`
- 进入 Import Package Review 后再合入权威 Story Project
- 未来再评估 NarrRail-hosted AI Conversion Service 和商业化

安装 skill：

```shell
node -e "const fs=require('fs'),os=require('os'),path=require('path'),cp=require('child_process');const home=process.env.CODEX_HOME||path.join(os.homedir(),'.codex');const installer=path.join(home,'skills','.system','skill-installer','scripts','install-skill-from-github.py');if(!fs.existsSync(installer)){console.error('Codex skill installer not found: '+installer);process.exit(1)};const base=['--repo','Courtshipfy/NarrRail','--ref','main','--path','.codex/skills/narrrail-story-converter'];for(const c of process.platform==='win32'?[['py',['-3']],['python',[]],['python3',[]]]:[['python3',[]],['python',[]]]){const r=cp.spawnSync(c[0],[...c[1],installer,...base],{stdio:'inherit'});if(!r.error)process.exit(r.status??0)};console.error('Python not found. Install Python 3.');process.exit(1)"
```

## 5. Story Consumer

主仓库不再携带 UE 插件源码和示例工程。Unreal 相关源码、Host、Blueprint、Debugger、PIE、Sync Stories 和资产同步文档都在：

<https://github.com/Courtshipfy/NarrRail-Unreal-Plugin>

主仓库保留的职责：

- 维护 `.nrstory` / GlobalConfig / `.nroutline` 中立格式契约
- 维护 Story Project 创作、导入审查、预览和转换工作流
- 记录 Story Consumer 的兼容边界
- 要求格式变更先通过主仓库 issue 和兼容说明处理

## 6. 重要文档

- `CONTEXT.md`：NarrRail 当前领域语言
- `Docs/adr/0001-split-unreal-consumer-repository.md`：UE consumer repo 拆分决策
- `Docs/agents/spec-kit.md`：大型执行票的 spec-kit 兼容工作流
- `.specify/`：官方 Spec Kit 脚本、模板、constitution 与 agent 集成配置
- `.agents/skills/speckit-*`：Codex 的 Spec Kit skills
- `specs/`：大型功能的 feature spec / plan / tasks / checklist 产物
- `Docs/research/0037-authoring-product-code-boundaries.md`：authoring product 代码边界
- `Docs/research/0039-product-technology-shape.md`：Web / desktop / shared core 技术形态评估
- `Docs/spec/NRSTORY_FORMAT.md`：`.nrstory` / `.nroutline` 中立格式契约
- `Docs/02_runtime/SCRIPT_FORMAT.md`：运行时侧格式说明和历史入口
- `Docs/story-consumers/NARRRAIL_UNREAL_PLUGIN.md`：Unreal Story Consumer 仓库入口
- `Docs/06_planning/OUTLINE_EXTENSION_MIGRATION.md`：`.nrrail` 到 `.nroutline` 迁移说明
- `NarrRailEditor/README.md`：当前创作端说明
- `.codex/skills/narrrail-story-converter/`：剧情文稿转换 skill

## 7. 当前执行入口

规划地图已经收束到 GitHub issues。优先入口：

- 按 spec-kit 工作流规格化大型功能
- Story Project IA / Dashboard Figma 原型
- NarrRail authoring design system
- Script Conversion / Import Package Review 执行票
- Project Dashboard / Review Queue / Preview 生产实现
- Structured Script View 生产实现

查看当前队列：

```bash
gh issue list --repo Courtshipfy/NarrRail --state open
```
