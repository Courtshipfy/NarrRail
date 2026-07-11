# NarrRailUEHost 指南

## 目标

`NarrRailUEHost` 是 NarrRail UE 插件的开发宿主工程，用于在 UE5.7 中运行、调试、导入和同步故事资产，同时保持 `NarrRail/` 插件源码作为单一事实来源。

兼容矩阵、版本关系和最小安装/试用步骤见 `Docs/04_narrrail_ue_host/UNREAL_PLUGIN_COMPATIBILITY.md`。该文档后续应随 UE consumer 迁移到 `NarrRail-Unreal-Plugin`。

## 目录结构

- `NarrRail/`：插件源码
- `NarrRailUEHost/`：UE 开发宿主工程
- `NarrRailUEHost/Build-NarrRailUEHost.cmd`：构建入口
- `NarrRailUEHost/Plugins/NarrRail`：指向仓库根目录 `NarrRail/` 的 Junction，由构建脚本自动创建

## 一次性设置

推荐设置 UE5.7 引擎路径环境变量：

```cmd
setx UE57_ROOT "I:\UE_5.7"
```

`Build-NarrRailUEHost.cmd` 会优先读取 `UE57_ROOT`；如果未设置，则回退到 `I:\UE_5.7`。

## 日常开发流程

1. 打开 `NarrRailUEHost/NarrRailUEHost.uproject`
2. 修改 `NarrRail/Source/...` 中的插件代码
3. 双击或运行 `NarrRailUEHost/Build-NarrRailUEHost.cmd`
4. 在 UE 编辑器中使用 PIE 或测试地图验证功能

## 故事仓库同步

这次重构后，推荐通过插件工具栏的 `Sync Stories` 同步本地故事仓库，而不是逐个拖拽导入文件。

### 配置位置

打开：

```text
Project Settings > Plugins > NarrRail
```

配置项：

- `Story Repository Path`：本地故事仓库路径。为空或无效时，点击 `Sync Stories` 会弹出系统文件夹选择窗口，并保存选择结果。
- `Pull Git Repository Before Sync`：默认开启。如果仓库是 Git 工作区，同步前执行 `git pull --ff-only`。
- `Story Asset Root Path (UE Content)`：UE Content 下的同步根目录，默认 `/Game/NarrRailStories`。

### 同步目标目录

插件会在 `Story Asset Root Path` 下追加故事仓库文件夹名，生成唯一资产目录：

```text
/Game/NarrRailStories/<故事仓库文件夹名>
```

例如本地仓库路径为：

```text
D:/Projects/NarrRailStories/Courtshipy
```

同步目标为：

```text
/Game/NarrRailStories/Courtshipy
```

### 同步规则

- 扫描故事仓库内所有 `*.nrstory`
- 保留仓库内的相对子目录结构并映射到 UE Content
- 普通剧情文件生成或更新 `UNarrRailStoryAsset`
- `meta.configType: GlobalConfig` 文件生成或更新 `UNarrRailGlobalConfigAsset`
- 只管理 `/Game/NarrRailStories/<故事仓库文件夹名>` 目录内由同步流程创建的资产
- 如果仓库中删除了 `.nrstory`，同步时会删除对应 UE 资产
- 删除多余资产前会弹出确认窗口
- 单个文件导入失败不会中断整个同步，结束后会汇总错误

### Git 拉取规则

当 `Pull Git Repository Before Sync` 开启且 `Story Repository Path` 是 Git 工作区时，插件会先执行：

```cmd
git -C "<StoryRepositoryPath>" pull --ff-only
```

如果拉取失败，同步会中止并显示 Git 输出。常见原因包括：

- 本机未安装 Git 或 Git 不在 `PATH`
- 本地有未提交改动导致无法 fast-forward
- 远端分支需要手动处理冲突

## 全局配置同步

全局配置文件也放在故事仓库中，并使用 `.nrstory` 后缀。文件需要声明：

```yaml
meta:
  schemaVersion: 1
  configType: GlobalConfig
variables: []
presetSpeakers: []
```

同步后会生成 `UNarrRailGlobalConfigAsset`，用于承载全局变量和预设角色。

## 构建入口

```cmd
NarrRailUEHost\Build-NarrRailUEHost.cmd
```

默认行为：

- 构建目标：`UnrealEditor Win64 Development`
- 工程路径：`NarrRailUEHost/NarrRailUEHost.uproject`
- 自动检查并创建 `NarrRailUEHost/Plugins/NarrRail` Junction

## 常见问题

### 为什么使用 Junction 而不是普通快捷方式

Unreal Engine 不一定会识别 `.lnk` 快捷方式。Junction 或 symlink 更接近真实目录，适合插件源码联动开发。

### `Sync Stories` 没有显示路径选择窗口

如果 `Story Repository Path` 已经有效，插件会直接使用保存的路径。需要修改路径时，到 `Project Settings > Plugins > NarrRail` 中清空或更改该字段。

### 同步后旧资产还在

插件只管理当前故事仓库对应的唯一目录。确认资产位于 `/Game/NarrRailStories/<故事仓库文件夹名>` 下，并且删除确认窗口没有被取消。
