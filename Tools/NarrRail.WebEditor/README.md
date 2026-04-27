# NarrRail Web Editor

基于 **Vue 3 + Svelte Flow** 的 NarrRail 剧情可视化编辑器。  
面向编剧与关卡叙事同学，支持节点式编辑、YAML 导入导出、实时校验与本地自动保存。

---

## 技术栈

- **Vue 3**：整体 UI 容器（工具栏、属性面板、状态栏等）
- **Svelte Flow (@xyflow/svelte)**：图编辑核心（节点、连线、交互）
- **YAML**：脚本解析与生成
- **Vite**：开发与构建

---

## 快速开始

### 开发模式（推荐）

```bash
cd Tools/NarrRail.WebEditor
npm install
npm run dev -- --host
```

启动后通常会显示：

- `Local: http://localhost:5173/`
- `Network: http://<你的本机IP>:5173/`

> 若 `localhost` 不可访问，请使用 `Network` 地址。

### 生产构建

```bash
npm run build
```

构建产物在 `dist/`。  
可直接打开 `dist/index.html` 进行离线使用。

---

## 当前功能（已实现）

### 编辑器核心

- ✅ 基础布局（工具栏 + 图编辑区 + 属性面板 + 变量面板 + 状态栏）
- ✅ 6 种节点类型：
  - `Dialogue`
  - `Choice`
  - `Jump`
  - `SetVariable`
  - `EmitEvent`
  - `End`
- ✅ 右键画布创建节点
- ✅ 节点拖拽、连线、删除（含边级联删除）
- ✅ 选中节点后属性编辑
- ✅ 选中边后可编辑：
  - `priority`
  - `condition.logic`
  - `condition.terms`（JSON）

### 交互增强

- ✅ 左键拖拽框选多个节点
- ✅ 多选节点后可整体拖动
- ✅ 鼠标中键拖动画布
- ✅ 连线样式切换：**直线 / 曲线**
  - 当前默认：**直线**
- ✅ 焦点模式（可开关）
  - 当前默认：**关闭**
- ✅ 自动排布（层级排布 + Choice 分支扇出优化）
- ✅ 导入 YAML 后自动执行一次自动排布

### YAML 导入导出

- ✅ 导出符合当前脚本规范（重点：`Choice` 节点使用 `choices` 根字段）
- ✅ 导入兼容两种 Choice 格式：
  - 新格式：`choices: []`
  - 旧格式：`choice: { choices: [] }`
- ✅ Choice 选项通过 `sourceHandle=choice-{index}` 映射目标节点

### 校验与状态

- ✅ 实时验证（节点/边/入口/Choice 映射/条件结构等）
- ✅ 手动“验证”按钮输出详细错误与警告
- ✅ 状态栏右下角显示：
  - 实时验证状态与 `E/W` 数量
  - 自动保存状态

### 本地存储

- ✅ 自动保存到 `localStorage`
- ✅ 默认每 **30 秒** 保存一次
- ✅ 存储键格式：`narrrail_<storyId>`
- ✅ 保存内容：`nodes / edges / meta / variables / lastModified`

---

## 项目结构（当前）

```text
NarrRail.WebEditor/
├── package.json
├── vite.config.js
├── index.html
├── src/
│   ├── main.js
│   ├── App.vue
│   ├── components/
│   │   ├── Toolbar.vue
│   │   ├── GraphEditor.svelte
│   │   ├── GraphEditorWrapper.vue
│   │   ├── PropertyPanel.vue
│   │   ├── VariablePanel.vue
│   │   └── StatusBar.vue
│   ├── nodes/
│   │   ├── DialogueNode.svelte
│   │   ├── ChoiceNode.svelte
│   │   ├── JumpNode.svelte
│   │   ├── SetVariableNode.svelte
│   │   ├── EmitEventNode.svelte
│   │   └── EndNode.svelte
│   ├── utils/
│   │   ├── yaml-exporter.js
│   │   ├── yaml-importer.js
│   │   ├── validation.js
│   │   └── storage.js
│   └── styles/
│       └── editor.css
└── README.md
```

---

## 使用流程

### 常规创作流

1. 打开编辑器（`npm run dev` 或 `dist/index.html`）
2. 新建剧情或导入 YAML
3. 添加/连接节点
4. 编辑节点与边属性
5. 查看实时验证结果（右下角）
6. 导出 YAML
7. 导入 UE 测试

### 迭代修改流

1. 在 UE 中发现问题
2. 回到编辑器导入原 YAML
3. 修改图结构/属性
4. 导出覆盖
5. 在 UE 重新验证

---

## 开发进度（更新）

- [x] 阶段 0：项目初始化
- [x] 阶段 1：基础框架
- [x] 阶段 2：图编辑器核心（节点/连线/删除/多选/中键平移）
- [x] 阶段 3：YAML 导入导出（含 Choice 规范修正与兼容导入）
- [x] 阶段 4：属性编辑面板（节点 + 边属性）
- [x] 阶段 5：本地存储与自动保存
- [x] 阶段 6：前端验证（实时 + 手动）

---

## 已知限制 / 待优化

- 条件编辑目前 `terms` 仍以 JSON 文本输入为主（后续可视化表单化）
- 超大图（数百节点）仍需继续优化布局与渲染性能
- 目前无完整撤销/重做历史系统（规划中）

---

## 浏览器兼容性

推荐：

- Chrome 90+
- Edge 90+
- Firefox 88+
- Safari 14+（macOS）

---

## 故障排除

### 无法启动

- 检查 Node.js 版本（建议 16+）
- 重新执行 `npm install`
- 检查 5173 端口占用

### 无法访问 localhost

- 使用 `npm run dev -- --host`
- 改用 Network 地址访问

### YAML 导入/导出异常

- 先点击“验证”查看错误
- 检查入口节点、节点 ID 唯一性、边引用有效性
- 检查 `Choice` 节点字段是否为 `choices`（不是 `choice.choices`）

---

## 许可证

本项目是 NarrRail UE5 插件的一部分，遵循仓库统一许可证。

---

## 参考资料

- Svelte Flow: https://svelteflow.dev/
- Vue 3: https://vuejs.org/
- Vite: https://vitejs.dev/