# NarrRail 作者工作台设计系统 v1

状态：**浅色 Foundations 与 Components 已 Approved for Spec；深色仍为 Candidate**。本文是 #44 的工程交接稿，记录设计决议、Figma 语义变量、组件/状态规则与生产实现边界；它不表示运行时代码已经完成。

## 来源与适用范围

- 工作文件：[NarrRail Authoring Workbench Design](https://www.figma.com/design/92M2rwxpRTFbzGgB2AepYm)，`01 Design System Workspace`。
- 浅色规格画板：[Foundations / #44 / Approved for Spec](https://www.figma.com/design/92M2rwxpRTFbzGgB2AepYm?node-id=120-36)。原有 `Foundations / #44` 占位画板和已确认画面保持原样。
- 浅色组件规格画板：[Components / #44 / Approved for Spec](https://www.figma.com/design/92M2rwxpRTFbzGgB2AepYm?node-id=128-2)，包含控件四态与 invalid/loading、Workbench Shell、ScriptRow、ReviewItem 及工作区映射；这是可写规格的视觉来源，不等同已发布的 Figma 组件变体库。
- 深色候选画板：[Dark Theme Samples / #44 / Candidate v1](https://www.figma.com/design/92M2rwxpRTFbzGgB2AepYm?node-id=131-2)，包含 Shell、ScriptRow、ReviewItem 代表样本，绑定独立的 Dark 变量集合。
- 已确认的视觉基础：[阅读优先的写作面](https://www.figma.com/design/92M2rwxpRTFbzGgB2AepYm?node-id=67-2)、[Structured Script](https://www.figma.com/design/92M2rwxpRTFbzGgB2AepYm?node-id=77-2)、[Review Queue](https://www.figma.com/design/92M2rwxpRTFbzGgB2AepYm?node-id=96-2)。
- 已确认的产品信息架构：[Dashboard/IA 决议](https://github.com/Courtshipfy/NarrRail/issues/71#issuecomment-5294194269)。[Dashboard 候选画面](https://www.figma.com/design/92M2rwxpRTFbzGgB2AepYm?node-id=102-2)仍是 Candidate，不是批准稿。
- 本版只覆盖桌面作者工作台；#67 决议的主规格画布为 1440×960，最低设计宽度 1280px，不定义移动或触屏布局。Figma 中已确认的脚本与队列示例画布为 1440×900；那是示例尺寸，不是强制的运行时窗口高度。

## 设计原则与工作区关系

内容优先、低干扰、渐进显现与一致性沿用 [`README_UI_STYLE.md`](../../NarrRailEditor/README_UI_STYLE.md)。v1 将其具体化为两种共享同一套语义 token 的工作区：

| 工作区 | 主体 | 辅助上下文 | 避免 |
| --- | --- | --- | --- |
| Dashboard、Review Queue、项目设置 | 项目概况或集中处理列表 | 可按需要出现的详情区 | 把每个入口或状态堆成等权卡片 |
| Structured Script | 居中的连续阅读/编辑列，目标宽度约 760px | 当前行附近、由悬停/选中/键盘焦点唤起的柔和上下文 | 常驻右侧 Inspector、整行盒子、永久显露的危险操作 |

项目抬头包含项目切换、仓库/分支与同步状态。左侧是可折叠的 Stories 资源树；点击 `.nrstory` 直接打开中央编辑器。Outline 是独立的项目结构入口，仅在 `.nroutline` 存在时出现；缺少 Outline 不影响单剧本写作、校验与预览。Review Queue 有专用导航和计数，Dashboard 只放摘要与入口，剧本行附近只放低干扰标记。Preview、Import、Export、Sync 位于顶部动作区；Config 从项目设置进入中央工作区。

浅色工作台外壳可作为结构基线；早期三栏原型中的右侧 Inspector 仅适用于需要集中详情的工作区，不适用于主写作面。窄桌面空间先折叠可选详情区；在 1280px 设计宽度，Stories 导航可由约 220px 折为约 58px，中央阅读列仍以约 760px 为目标，剩余空间保留弹性留白及行附近上下文，不固定成右侧 Inspector。

## 字体与密度

- 剧本/阅读正文：`LXGW WenKai TC Regular`；已确认画面的正文示例为 16px、27px 行高。
- 导航、按钮、输入、队列和系统状态：UI 字体栈为 `Inter, "Noto Sans", system-ui, sans-serif`；Figma 画板以 Inter 英文和 Noto Sans 中文样例核对。剧本正文仍用霞鹜文楷，不能把两套字体职责混用。
- 已确认的 Script 标题示例为 22px/34px，Review Queue 标题示例为 18px/26px，辅助 UI 文字约 11–15px。这些是画面证据，不是完整 type scale。
- 在 1440×900 确认稿中，折叠导航栏约 58px 宽。行结构靠淡行号、安静的轨道点、细微流向与留白表达；不能仅靠颜色区分错误、警告和待审查。

## 语义 token 合约

Figma 中有 `NR Semantic Light / #44 Approved for Spec` 与 `NR Semantic Dark / #44 Candidate` 两个各只有单一模式的变量集合，语义变量名逐项对应，**不会在 Figma 中自动切换主题**。这是 Starter 限制下的交接策略；最终工程主题切换按同名 CSS custom properties 映射。浅色 canvas 与 error 强调色可直接追溯至已确认画面；其余浅色值经 #44 样本和对比度验收后批准，深色值继续作为候选。

| Figma 名称 → CSS 名称 | 用途 | Light 已批准 / Dark 候选 |
| --- | --- | --- |
| `surface/canvas` → `--nr-surface-canvas` | 工作区底色 | `#FBFAF7` / `#171D1C`；浅色值为确认画面示例 |
| `surface/workspace` → `--nr-surface-workspace` | 中央内容与列表底面 | `#FFFDF9` / `#202826` |
| `surface/context` → `--nr-surface-context` | 行附近上下文、选中项柔和底色 | `#F3EDE4` / `#2A3834` |
| `text/primary`、`text/secondary`、`text/muted` → 同名前缀 `--nr-text-*` | 正文、说明、弱提示 | `#26332F` / `#F3F0E8`；`#53625C` / `#C9D1C9`；`#606B64` / `#A2AEA6` |
| `border/subtle`、`border/focus` → `--nr-border-*` | 弱分隔、键盘焦点 | `#DED8CE` / `#43524C`；`#2C7180` / `#83C9D4` |
| `accent/active` → `--nr-accent-active` | 当前导航、可操作重点 | `#2F5B55` / `#84C1AD` |
| `status/error`、`status/error-text`、`status/warning`、`status/review`、`status/success` → `--nr-status-*` | 校验与审查语义；浅色 error 强调色只用于图形/边界，小字用 error-text | `#B85D4F` / `#F09485`；`#9D493C` / `#F09485`；`#925F28` / `#D9AC65`；`#6D8F83` / `#93BDB0`；`#3D7566` / `#85C9AB`。浅色 error 强调色为确认画面示例 |
| `sync/idle`、`sync/active`、`sync/failed` → `--nr-sync-*` | 仓库/项目同步；不等同 ReviewItem 严重级别 | `#758079` / `#A2AEA6`；`#356D85` / `#85BED7`；`#B85D4F` / `#F09485` |
| `motion/*` → `--nr-motion-*` | 时长与缓动 | 时长、用途和 reduced-motion 规则见下节与 Figma Foundations；CSS 实现留给后续工作 |

深色主题沿用相同语义名称与组件状态，不应只做整页反色。新候选画板已有工作台外壳、ScriptRow 和 ReviewItem 样本；原有 `Dark Theme Samples / #44` 仍是 Exploration 占位。深色值与样本尚未批准为生产规格，但这不阻塞 #44 的浅色设计交接。

## 组件与状态

基础控件只做工作台必要范围：当前 Figma 规格画板具体示范 Button、Input、StatusPill 的 default、hover、keyboard focus、disabled，并补 Input invalid/loading。IconButton、Textarea、Select/Segmented、Tabs、Tooltip、Modal/Popover、Panel/Toolbar，以及 Empty/Loading/Error 依本节同样的语义与状态规则在后续页面按需细化，不要求 #44 制成穷尽组件库。可选中组件还需 selected；隐藏的行操作必须能由键盘焦点唤起，不能只依赖 hover。

NarrRail 特有组件优先于扩充通用控件库：

| 组件 | v1 要固定的语言 | 后续实现归属 |
| --- | --- | --- |
| Workbench Shell、Workspace Navigation Item | 项目抬头、可折叠导航、选中状态、中央工作区 | #62 |
| Repo/Branch/Sync Indicator、Project Health Summary | 状态与可达入口；Dashboard 保持摘要级 | #62 |
| Story/Outline/Config Asset Row | 资源类型与可用状态；Outline 可缺席 | #62 |
| ScriptRow | 对话、旁白/动作、事件、变量、条件、选择、跳转、审查共用一条阅读流；hover、focus、selected、editing、invalid、linked-review 及增删/拖动入口 | #61 |
| ReviewItem | error/warning/review 严重性、来源、定位、消息、建议操作；open/acknowledged/resolved/ignored 状态 | #63 |
| Preview Control Bar、Export/Validation Status | 清楚区分当前剧本预览与可选项目预览；不在 v1 设计完整流程 | #60 与后续 Export |

ScriptRow 的控件贴近当前行，通过柔和色洗与留白提示上下文；同一行骨架容纳对话、旁白/动作、事件、变量、条件、选择、跳转和审查。`selected` 是持续的当前行上下文，hover 与 keyboard focus 都显露增行/拖动等局部操作；editing 保留输入焦点，invalid 表示校验问题，linked-review 是可与 selected 并存的审查关联。删除保持弱化并要求确认；审查跳转与解决后应返回原行及焦点。

ReviewItem 在 Dashboard 只呈现计数和队列入口，在 ScriptRow 是行附近的低干扰标记，在 Review Queue 才是带严重级别、来源/目标位置、消息和建议动作的处理项。三处共享同一 ReviewItem 身份与 open/acknowledged/resolved/ignored 状态；集中处理可有选中项详情区，但不得复制成写作界面的常驻通用 Inspector。队列动作更新摘要和行内标记，并保留源目标定位。

Input 的 invalid 态保留原输入和键盘焦点，用错误边界、图形与相邻文字共同说明原因；loading 态保留可编辑性，用非颜色进度提示和文字，不阻塞写作或夺走焦点。其余基础控件按真实工作区需要扩展，v1 的视觉样本不是穷尽的 Figma 组件变体库。

## 动效与可访问性

| token | 时长 | 用途 |
| --- | --- | --- |
| `motion/instant` | 80ms | 焦点和按压反馈 |
| `motion/fast` | 150ms | 悬停显现、状态 pill 更新 |
| `motion/normal` | 220ms | 可选详情区折叠、视图转场 |
| `motion/slow` | 320ms | 少量工作区级展开 |

`motion/ease-standard` 用于常规空间连续性，`motion/ease-emphasized` 只用于小幅完成反馈。行插入/删除、队列解决、同步/校验反馈不能阻塞输入或夺走写作焦点。支持 `prefers-reduced-motion`：去掉非必要位移和轻跳，保留即时状态变化；错误与审查始终有文字/图标，聚焦状态可由键盘清晰识别。

按 sRGB 相对亮度公式，以三种浅色底面中最不利的 `surface/context`（`#F3EDE4`）复算：`text/muted` 4.77:1、`status/warning` 4.64:1、`status/error-text` 5.23:1，可用于普通小字；原确认画面的 `status/error` 3.83:1、`status/review` 3.05:1，可作为图形/边界提示，不直接用于小字。普通文字至少 4.5:1，图形状态至少 3:1；同步 idle 及其他低对比强调色也须配正常文字色的状态标签，不能靠颜色单独表达语义。Figma 中 135 处浅色小字按所在不透明底面复算均不低于 4.77:1，禁用态不再靠整体透明度削弱文字。静态 Figma 板不能证明实际键盘行为，后续实现仍需交互测试。

## 工程映射与迁移

Figma 语义名称统一映射为 CSS custom properties（例如 `surface/canvas` → `--nr-surface-canvas`），在容器主题属性或类上覆写深色值。Vue 组件按工作区职责拆分，不能把 Dashboard、Script Editor、Review Queue 的不同上下文处理硬塞进一个通用右侧面板。设计 token 是新 UI 的目标合约，**不是当前代码已经实现的 API**。

现有 [`editor.css`](../../NarrRailEditor/src/styles/editor.css) 仅有 `--nr-bg`、`--nr-text`、`--nr-noise-color` 等少数全局变量，大量颜色与动画仍直接写在选择器里。生产实现时逐步映射/替换，而不是在 #44 一次性改动运行时代码。旧样式指南保留低干扰、内容优先、渐进显现、危险操作弱化、弹窗与输入焦点原则；本文件在工作区结构、主题语义、NarrRail 组件状态和动效 token 上优先。若个别页面需偏离，须记录原因。

## #44 完成门槛

- [x] `Foundations / #44 / Approved for Spec` 已建立浅色语义变量、字体/密度/状态/动效说明及深色候选映射；已回读节点与渲染画板。
- [x] `Components / #44 / Approved for Spec` 已建立 Button、Input、StatusPill 四态与 Input invalid/loading、Workbench Shell、ScriptRow、ReviewItem 及跨工作区规则；已截图核对。可发布的 Figma 组件属性/变体库不属于 #44 最小验收范围。
- [x] `Dark Theme Samples / #44 / Candidate v1` 已建立外壳、ScriptRow、ReviewItem 代表样本并绑定 Dark 变量集合，明确不自动切换模式。
- [x] 核对浅色焦点、文字对比、错误/警告/审查及 reduced-motion 规则；135 处小字、图形阈值与禁用态复查通过，浅色两板已命名 `Approved for Spec`。
- [x] 本文记录最终浅色值、深色候选值、双语 UI 字体、1280px 关系、组件状态及 Vue/CSS 映射，足以支撑 #42 与后续实现规格。

后续实现仍须验证运行时键盘顺序与焦点保持、真实 1280px 响应、双语字体回退、主题切换和生产组件属性；它们不是 #44 静态设计稿可验证的行为。#42 可据此制作 Dashboard 基础画面及 setup-incomplete、syncing、review-items 状态；本文不代替 #42 的关键画面验收。
