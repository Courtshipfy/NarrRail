# NarrRail WebEditor

[English](./README.en.md)

基于 **Vue 3 + Svelte Flow** 的 NarrRail 剧情可视化编辑器。  
面向编剧与叙事设计同学，支持节点式编辑、`.nrstory` 导入导出、实时校验、自动保存与运行语义预览。

## 技术栈

- **Vue 3**：主 UI 容器（工具栏、属性面板、状态栏、脚本库）
- **Svelte Flow (@xyflow/svelte)**：图编辑核心（节点、连线、交互）
- **YAML**：脚本内容序列化（统一后缀 `.nrstory`）
- **Vite**：开发与构建

## 快速开始

### 线上入口（默认）

- https://narr-rail.courtship.top

### 使用流程（推荐）

1. 登录后进入脚本库
2. 新建/打开 `.nrstory`
3. 编辑节点并完成校验
4. 使用预览模式审校剧情流程
5. 导出 `.nrstory` 并回流到 UE

### 本地开发（仅开发者）

环境要求：

- Node.js `>= 20`（建议 LTS）
- npm `>= 9`

重要约束（跨平台稳定启动）：

- `node_modules` 不要提交到 Git，也不要在 Win/Mac 之间拷贝复用
- 切换平台后请在本机重新安装依赖（`npm ci` 或 `npm install`）

macOS / Linux：

```bash
cd Tools/NarrRail.WebEditor
npm ci
npm run dev -- --host 127.0.0.1
```

Windows（PowerShell / CMD）：

```bash
cd Tools/NarrRail.WebEditor
npm ci
npm run dev -- --host 127.0.0.1
```

默认地址：`http://127.0.0.1:5173/`

常见问题：

- `Cannot find module @rollup/rollup-<platform>`：
  删除 `node_modules` 后重新执行 `npm ci`
- `node_modules/.bin/vite: Permission denied`（macOS/Linux）：
  执行 `chmod +x node_modules/.bin/vite` 后重试

生产构建：

```bash
npm run build
```

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
├── README.md
└── README.en.md
```

## 相关文档

- 预览模式说明：`Tools/NarrRail.WebEditor/README_READING_MODE.md`
- 介绍页规范：`Docs/05_webeditor/INTRO_PAGE_CONTENT_STYLE_SPEC.md`
- 部署方案：`Docs/05_webeditor/WEBEDITOR_DEPLOYMENT_PLAN.md`

## 许可证

本项目是 NarrRail 仓库的一部分，遵循仓库统一许可证。
