# NarrRail Authoring Overview 页面内容与样式规范（Draft v0.5）

> 更新：2026-05-22
> 状态：等待后续 Figma-first Story Project IA 重构
> 适配方向：NarrRail 主仓库已经重新定位为剧本创作与辅助产品，Overview 页面不应再以 UE Runtime 能力作为产品中心

适用范围：`NarrRailEditor/src/components/OverviewPage.vue`

## 1. 页面目标

介绍页用于在用户进入编辑器前，建立统一认知：

1. NarrRail 是面向 Narrative Creator 的 Story Project 创作工作台。
2. 主体验围绕 GitHub-backed Story Project、结构化剧本编辑、项目审查、项目预览与导出。
3. UE 插件是下游 Story Consumer 示例，不是 Overview 页面的一等产品主语。
4. 用户下一步操作清晰：返回项目入口、切换主题、进入创作工作台。

## 2. 信息架构（IA）

页面按以下区块组织，自上而下：

1. `Top Actions`
2. `Hero`
3. `Story Project Capability`
4. `Authoring Workflow`
5. `Script Conversion / Import Review`
6. `Story Consumer Compatibility`
7. `Role Value`

### 2.1 Top Actions

按钮顺序：

1. `返回主页`
2. `切换主题`
3. `进入编辑器`（主按钮）

要求：

1. 顶部吸附（sticky）可见。
2. 桌面端右对齐，移动端单列堆叠。

### 2.2 Hero 区

要求：

1. 一句话价值主张（标题 + 副标题）。
2. 明确 NarrRail 的创作链路：Story Project -> `.nrstory` / `.nroutline` -> Review Queue -> Preview -> Export。
3. 配置“总览配图占位区”（当前不使用真实图片）。

## 3. 内容细化规范

### 3.1 Story Project 能力区（必须覆盖）

1. GitHub-backed Story Project 作为权威存储。
2. `Stories/` + `Config/` 项目结构。
3. `.nrstory` 内容文件与 `.nroutline` 项目编排。
4. Project Review Queue 汇总跨文件问题。
5. Project Preview 以项目为单位审校流程。
6. Script Editor 支持 Graph View 和 Structured Script View 两种 `.nrstory` 视图。

补充：
- 预览模式文案建议强调“运行语义预览”，而非静态阅读。
- 全局配置文件命名需使用 `.nrstory`（`globalconfig.nrstory` / `global-config.nrstory`）。

### 3.2 Script Conversion / Import Review 区（必须覆盖）

1. 编剧提供 Word、Excel、TXT、剧本文稿或大纲。
2. 转换输出 Story Project Import Package，而不是直接信任单个生成文件。
3. `conversion-notes.md` 记录不确定、遗漏和需要人工确认的内容。
4. Import Package Review 先审查、校验、修正，再合入权威 Story Project。
5. hosted AI conversion service 是未来商业探索，不属于当前首屏承诺。

### 3.3 Story Consumer Compatibility 区（可弱化）

1. UE 插件作为 Story Consumer 读取导出的 `.nrstory`。
2. 主仓库保留格式兼容说明和版本契约。
3. UE 插件与示例工程后续迁移到 `NarrRail-Unreal-Plugin`。
4. 本区不得压过 Story Project 创作能力。

### 3.4 Workflow 区（必须是五步）

1. 创建或打开 GitHub-backed Story Project
2. 编写 `.nrstory`，并用 `.nroutline` 编排项目
3. 处理 Project Review Queue
4. 使用 Project Preview 审校流程
5. 导出给 Story Consumer

### 3.5 Role Value 区（建议保留）

角色最少覆盖：

1. 叙事策划 / 编剧
2. Narrative Designer
3. 项目负责人
4. 客户端 / 技术美术（作为 Story Consumer 集成角色）

## 4. 配图策略（当前阶段）

当前阶段不放真实配图，统一使用“配图占位卡片”。

占位卡片要求：

1. 保留未来图片应有的尺寸和位置。
2. 卡片内写明“该位置应该放什么图”与“该图用于说明什么”。
3. Hero、Story Project 区、Review Queue / Preview 区各保留一个占位。

后续接入真实配图时再替换占位内容，不改布局结构。

## 5. 主题与可读性

深浅主题要求：

1. 标题、正文、列表、卡片在深色模式下保持可读对比。
2. 占位卡片在深色模式下也有清晰边界。
3. 不依赖单一颜色传递关键信息。

## 6. 滚动与布局行为

1. 页面容器使用 `height: 100vh`。
2. 页面容器使用 `overflow-y: auto`，支持鼠标滚轮。
3. 使用 `overscroll-behavior: contain`，避免滚动串扰。
4. 1120px 以下：左右分栏改为单列。
5. 720px 以下：顶部按钮改为单列，流程卡片单列。

## 7. 文案风格

1. 先结果后机制：先写能力结果，再写实现方式。
2. 每条描述尽量 1-2 行，避免过长段落。
3. 术语统一：`Story Project`、`Narrative Creator`、`.nrstory`、`.nroutline`、`Project Review Queue`、`Project Preview`、`Story Consumer`。

## 8. 验收标准（DoD）

1. 介绍页可独立滚动浏览，内容长度明显超过一屏。
2. 页面包含完整六个区块，且内容不空泛。
3. Top Actions 三个按钮都可见且行为正确。
4. 配图区域全部为占位说明卡，位置尺寸稳定。
5. 深色/浅色模式下文本、卡片、边界都可读。
6. 页面文案与规范文档都为正常 UTF-8，无乱码。
