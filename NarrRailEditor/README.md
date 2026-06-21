# NarrRailEditor

[English](./README.en.md)

NarrRailEditor 是基于 **Vue 3 + Svelte Flow** 的 NarrRail 剧情可视化编辑器，面向编剧和叙事设计流程，支持节点式编辑、`.nrstory` 导入导出、结构校验、自动保存、预览审校和脚本库管理。

## 技术栈

- **Vue 3**：主 UI 容器、属性面板、脚本库、状态管理
- **Svelte Flow (@xyflow/svelte)**：节点图编辑、连线和画布交互
- **YAML**：统一 `.nrstory` 序列化格式
- **Vite**：开发与构建

## 快速开始

线上入口：

- https://narr-rail.courtship.top

推荐流程：

1. 登录后进入脚本库
2. 新建或打开 `.nrstory`
3. 编辑节点并完成校验
4. 使用预览模式审校剧情流程
5. 导出 `.nrstory` 并提交到故事仓库
6. 在 UE 中通过 `Sync Stories` 同步故事仓库

## 本地开发

环境要求：

- Node.js `>= 20`
- npm `>= 9`

```bash
cd NarrRailEditor
npm ci
npm run dev -- --host 127.0.0.1
```

默认地址：`http://127.0.0.1:5173/`

跨平台约束：

- 不要提交 `node_modules`
- 不要在 Windows/macOS/Linux 之间复制复用 `node_modules`
- 切换平台后在本机重新执行 `npm ci`

生产构建：

```bash
npm run build
```

## 当前功能

### 图编辑器

- 节点类型：`Dialogue` / `Choice` / `Jump` / `SetVariable` / `EmitEvent` / `Condition` / `End`
- `Dialogue` 节点支持单行和多行台词编辑
- 节点创建、拖拽、连线、删除、框选
- 右键菜单快速建点
- 自动布局
- 鼠标滚轮缩放、触摸板拖拽/平移/缩放

### 属性与数据编辑

- 按节点类型展示动态属性表单
- 变量管理：脚本变量、全局变量、类型和默认值
- `SetVariable` 节点可选择已定义变量
- `Condition` 节点可选择已定义变量
- 预设角色管理

### Condition 节点

Condition 是 NarrRail 当前唯一承载条件判断的结构。边上的 `condition` 字段已经废弃，导入和校验都会拒绝旧边条件。

当前机制：

- 一个 Condition 节点可以包含多个条件分支
- 每个分支对应一个输出句柄：`condition-0`、`condition-1`、`condition-2` ...
- 所有分支从上到下依次匹配，命中第一个满足条件的分支
- 没有分支命中时走 `condition-fallback`
- 双击 Condition 节点可打开独立编辑窗口
- 编辑器会校验输出句柄是否缺失、重复或与分支数量不一致

### 导入导出

- 支持 `.nrstory` 导入
- 支持 `.nrstory` 导出
- 导出时不会写入旧的 `edges[].condition`
- 导入时如果发现旧边条件，会提示使用 Condition 节点控制条件分支
- 支持全局配置文件：`meta.configType: GlobalConfig`

### 校验与保存

- 实时校验 + 手动校验
- 检查入口节点、ID 唯一性、边引用、孤立节点、变量引用、Condition 分支出口
- 本地自动保存兜底

### 预览模式

- 按运行语义推进，而不是按画布坐标顺序推进
- 支持 `Dialogue` / `Choice` / `Jump` / `SetVariable` / `EmitEvent` / `Condition` / `End`
- 支持 `ExhaustiveUntilComplete` 分支穷举流程

## 文件命名约定

剧情脚本统一使用：

```text
*.nrstory
```

全局配置也使用 `.nrstory` 后缀，推荐文件名：

```text
globalconfig.nrstory
global-config.nrstory
```

全局配置文件需要在 `meta` 中声明：

```yaml
meta:
  schemaVersion: 1
  configType: GlobalConfig
variables: []
presetSpeakers: []
```

旧的双后缀命名（例如 `*.narrrail.yaml`、`*.narrrail.yml`、`*.narrrail.nrstory`）不再作为默认约定。

## 与 UE 插件配合

编辑器负责创作和导出 `.nrstory`；UE 插件负责从本地故事仓库同步资产。

UE 侧流程：

1. 在 `Project Settings > Plugins > NarrRail` 设置 `Story Repository Path`
2. 点击工具栏 `Sync Stories`
3. 插件默认对 Git 仓库执行 `git pull --ff-only`
4. 插件将仓库内容镜像到 `/Game/NarrRailStories/<故事仓库文件夹名>`
5. 普通剧情生成 `UNarrRailStoryAsset`
6. 全局配置生成 `UNarrRailGlobalConfigAsset`
7. 仓库删除的文件会同步删除对应资产，删除前会弹确认
