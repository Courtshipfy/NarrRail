# NarrRail WebEditor

[中文](#中文文档) | [English](#english)

---

## 中文文档

基于 **Vue 3 + Svelte Flow** 的 NarrRail 剧情可视化编辑器。  
面向编剧与叙事设计同学，支持节点式编辑、`.nrstory` 导入导出、实时校验、自动保存与运行语义预览。

## 技术栈

- **Vue 3**：主 UI 容器（工具栏、属性面板、状态栏、脚本库）
- **Svelte Flow (@xyflow/svelte)**：图编辑核心（节点、连线、交互）
- **YAML**：脚本内容序列化（统一后缀 `.nrstory`）
- **Vite**：开发与构建

## 快速开始

### 开发模式

```bash
cd Tools/NarrRail.WebEditor
npm install
npm run dev -- --host
```

常见访问地址：

- `http://localhost:5173/`
- `http://<你的局域网IP>:5173/`

### 生产构建

```bash
npm run build
```

构建输出位于 `dist/`。

## 当前功能（实现状态）

### 1) 图编辑器核心

- 6 类节点：`Dialogue` / `Choice` / `Jump` / `SetVariable` / `EmitEvent` / `End`
- 节点创建、拖拽、连线、删除、框选
- 右键菜单快速建点
- 自动排布（含 Choice 分支布局优化）
- 连线样式切换（直线 / 曲线）

### 2) 属性与数据编辑

- 节点属性编辑（按类型动态表单）
- 边属性编辑：`priority`、`condition.logic`、`condition.terms`
- 变量与预设角色管理

### 3) 脚本导入导出

- 支持 `.nrstory` 导入
- 支持 `.nrstory` 导出
- 导入后自动布局
- 基础双向映射（图结构 ↔ 脚本结构）

### 4) 校验与保存

- 实时校验 + 手动校验
- 入口节点、ID 唯一性、边引用、孤立节点等检查
- 本地自动保存（含 localStorage 兜底）

### 5) 预览模式（Preview Mode）

- 从图编辑模式切换到预览模式
- 按运行语义推进，不按画布坐标顺序
- 支持：`Dialogue` / `MultiDialogue` / `Choice` / `Jump` / `SetVariable` / `EmitEvent` / `End`
- 支持 `ExhaustiveUntilComplete` 分支穷举流程
- End 到达后不会重复触发
- 标签样式：`[选择] [变量] [事件] [跳转] [结束]`，并与节点类型配色对齐

## 文件命名约定（重要）

### 剧情脚本

- 后缀统一：`.nrstory`

### 全局配置（变量/预设角色）

- 推荐文件名：
  - `globalconfig.nrstory`
  - `global-config.nrstory`

> 旧双后缀命名（如 `*.narrrail.yaml` / `*.narrrail.yml` / `*.narrrail.nrstory`）已不再作为默认约定。

## 目录结构（简）

```text
Tools/NarrRail.WebEditor/
├── src/
│   ├── App.vue
│   ├── components/
│   │   ├── ScriptLibraryPage.vue
│   │   ├── ReadModePanel.vue
│   │   ├── PropertyPanel.vue
│   │   ├── VariablePanel.vue
│   │   └── ...
│   ├── nodes/
│   │   ├── DialogueNode.svelte
│   │   ├── ChoiceNode.svelte
│   │   ├── JumpNode.svelte
│   │   ├── SetVariableNode.svelte
│   │   ├── EmitEventNode.svelte
│   │   └── EndNode.svelte
│   └── utils/
├── index.html
└── README.md
```

## 相关文档

- 预览模式说明：`Tools/NarrRail.WebEditor/README_READING_MODE.md`
- 介绍页规范：`Docs/05_webeditor/INTRO_PAGE_CONTENT_STYLE_SPEC.md`
- 部署方案：`Docs/05_webeditor/WEBEDITOR_DEPLOYMENT_PLAN.md`

---

## English

NarrRail WebEditor is a **Vue 3 + Svelte Flow** visual story editor for narrative production.
It supports graph editing, `.nrstory` import/export, validation, autosave, and runtime-like preview.

## Tech Stack

- **Vue 3** for app-level UI
- **Svelte Flow** for graph editing interactions
- **YAML** serialization with unified `.nrstory` extension
- **Vite** for dev/build

## Quick Start

### Development

```bash
cd Tools/NarrRail.WebEditor
npm install
npm run dev -- --host
```

### Build

```bash
npm run build
```

## Implemented Features

- Node graph editor (create/connect/move/delete/select)
- Node types: `Dialogue`, `Choice`, `Jump`, `SetVariable`, `EmitEvent`, `End`
- Property editing for nodes/edges/variables/speakers
- `.nrstory` import/export
- Real-time and manual validation
- Local autosave fallback
- Preview Mode with runtime-like flow execution
  - supports `ExhaustiveUntilComplete`
  - end-state duplicate-trigger protection

## File Naming Convention

- Story files: `.nrstory`
- Global config files:
  - `globalconfig.nrstory`
  - `global-config.nrstory`

## License

Part of the NarrRail repository and follows repository-level licensing.
