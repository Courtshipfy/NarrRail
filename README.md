# NarrRail

[English](./README.en.md)

NarrRail 是一个面向互动叙事创作者的剧本创作与辅助产品。主仓库现在以 **Story Project 创作工作台**、`.nrstory` / `.nroutline` 故事格式、导入审查、预览校验和创作辅助流程为中心。

UE 插件和示例工程仍保留在当前仓库中，但它们已经被重新定位为 **Story Consumer**：下游运行端读取 NarrRail 导出的故事文件，并声明与主仓库格式版本的兼容关系。后续会拆分到独立仓库 `NarrRail-Unreal-Plugin` 维护。

## 1. 产品定位

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

## 5. UE Consumer

`NarrRail/` 和 `NarrRailUEHost/` 目前仍在本仓库中，作为 UE Story Consumer 的源码和开发宿主工程。

当前定位：

- 读取并执行 `.nrstory`
- 导入 GlobalConfig
- 同步故事仓库到 UE Content
- 提供 Blueprint API、运行时调试和 PIE 验证能力

后续计划：

- clean copy 到 `NarrRail-Unreal-Plugin`
- 在新仓库维护 UE 插件、示例工程和 UE 专属文档
- 在主仓库只保留 Story Consumer compatibility matrix 和格式兼容说明

## 6. 重要文档

- `CONTEXT.md`：NarrRail 当前领域语言
- `Docs/adr/0001-split-unreal-consumer-repository.md`：UE consumer repo 拆分决策
- `Docs/agents/spec-kit.md`：大型执行票的 spec-kit 兼容工作流
- `specs/`：大型功能的 feature spec / plan / tasks / checklist 模板与产物
- `Docs/research/0037-authoring-product-code-boundaries.md`：authoring product 代码边界
- `Docs/research/0039-product-technology-shape.md`：Web / desktop / shared core 技术形态评估
- `Docs/02_runtime/SCRIPT_FORMAT.md`：`.nrstory` / `.nroutline` 格式规范
- `Docs/06_planning/OUTLINE_EXTENSION_MIGRATION.md`：`.nrrail` 到 `.nroutline` 迁移说明
- `NarrRailEditor/README.md`：当前创作端说明
- `.codex/skills/narrrail-story-converter/`：剧情文稿转换 skill

## 7. 当前执行入口

规划地图已经收束到 GitHub issues。优先入口：

- 按 spec-kit 工作流规格化大型功能
- Story Project IA / Dashboard Figma 原型
- NarrRail authoring design system
- UE consumer repo 拆分执行票
- Script Conversion / Import Package Review 执行票
- Project Dashboard / Review Queue / Preview 生产实现
- Structured Script View 生产实现

查看当前队列：

```bash
gh issue list --repo Courtshipfy/NarrRail --state open
```
