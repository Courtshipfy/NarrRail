# NarrRail Web Editor

基于 Vue 3 + Svelte Flow 的 NarrRail 剧情可视化编辑器。

## 技术栈

- **Vue 3**: UI 框架（工具栏、属性面板等）
- **Svelte Flow**: 节点编辑器核心（@xyflow/svelte）
- **YAML**: YAML 解析与生成
- **Vite**: 构建工具

## 快速开始

### 开发模式

```bash
cd Tools/NarrRail.WebEditor
npm install
npm run dev
```

浏览器会自动打开 http://localhost:5173

### 生产构建

```bash
npm run build
```

生成的 `dist/index.html` 是一个单文件应用，可以直接双击打开。

## 功能特性

### 当前版本 (v0.1.0 - 基础框架)

- ✅ 基础 UI 布局（工具栏 + 画布 + 属性面板 + 状态栏）
- ✅ Vue 3 + Svelte 混用架构
- ✅ YAML 导入/导出功能
- ✅ 前端验证功能
- ✅ localStorage 自动保存
- 🚧 Svelte Flow 图编辑器集成（开发中）
- 🚧 节点创建与编辑（开发中）
- 🚧 自定义节点类型（开发中）

### 计划功能

- 节点类型：Dialogue, Choice, Jump, SetVariable, EmitEvent, End
- 边条件编辑
- 变量管理
- 实时验证
- 错误高亮

## 项目结构

```
NarrRail.WebEditor/
├── package.json            # 项目配置
├── vite.config.js          # Vite 配置
├── index.html              # 应用入口
├── src/
│   ├── main.js            # Vue 应用入口
│   ├── App.vue            # 根组件
│   ├── components/
│   │   ├── Toolbar.vue    # 工具栏
│   │   ├── GraphEditor.svelte  # Svelte Flow 图编辑器
│   │   ├── PropertyPanel.vue   # 属性面板
│   │   └── StatusBar.vue       # 状态栏
│   ├── utils/
│   │   ├── yaml-exporter.js   # YAML 导出
│   │   ├── yaml-importer.js   # YAML 导入
│   │   ├── storage.js         # localStorage 封装
│   │   └── validation.js      # 前端验证
│   └── styles/
│       └── editor.css         # 全局样式
└── README.md
```

## 使用流程

### 编剧工作流

1. 双击 `dist/index.html` 打开编辑器（或运行 `npm run dev`）
2. 点击"新建"创建新剧情
3. 右键画布 → 添加节点（开发中）
4. 连接节点
5. 编辑节点属性（右侧面板）
6. 点击"验证"检查错误
7. 点击"导出"下载 YAML 文件
8. 将 YAML 文件拖到 UE Content Browser
9. 在 UE 中测试剧情

### 迭代修改流程

1. 在 UE 中发现问题
2. 打开编辑器，点击"导入"
3. 选择之前导出的 YAML 文件
4. 修改节点/边
5. 重新导出
6. 拖到 UE 覆盖旧文件
7. UE 自动重新导入

## 开发进度

- [x] 阶段 0: 项目初始化
- [x] 阶段 1: 基础框架
- [ ] 阶段 2: 图编辑器核心（进行中）
- [ ] 阶段 3: YAML 导入/导出（基础完成，待测试）
- [ ] 阶段 4: 属性编辑面板（基础完成，待扩展）
- [ ] 阶段 5: 本地存储（已完成）
- [ ] 阶段 6: 前端验证（已完成）

## 浏览器兼容性

推荐使用以下浏览器：

- Chrome 90+
- Firefox 88+
- Edge 90+
- Safari 14+ (macOS)

## 故障排除

### 编辑器无法启动

- 确保已安装 Node.js 16+
- 运行 `npm install` 安装依赖
- 检查端口 5173 是否被占用

### 导出的 YAML 无法导入 UE

- 点击"验证"检查错误
- 确保所有必填字段已填写
- 确保节点 ID 唯一
- 确保边引用的节点存在

## 许可证

本项目是 NarrRail UE5 插件的一部分，遵循相同的许可证。

## 参考资料

- [Svelte Flow 官方文档](https://svelteflow.dev/)
- [Vue 3 官方文档](https://vuejs.org/)
- [Vite 官方文档](https://vitejs.dev/)
