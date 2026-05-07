# NarrRail Host Project

开发用的 UE5.7 宿主项目，用于加载和测试 NarrRail 插件。

## 首次设置

1. **配置构建脚本**
   ```bash
   # 复制模板文件
   copy Build-HostProject.cmd.template Build-HostProject.cmd

   # 推荐：设置环境变量（仅需一次）
   setx UE57_ROOT "I:\UE_5.7"

   # 如果不设置 UE57_ROOT，脚本会默认使用 I:\UE_5.7
   ```

2. **构建项目**
   ```bash
   Build-HostProject.cmd
   ```

3. **打开编辑器**
   - 双击 `NarrRailHost.uproject`
   - 或通过 Epic Games Launcher 打开

## 注意事项

- `Build-HostProject.cmd` 是本地配置文件，不会提交到 Git
- 脚本优先读取 `UE57_ROOT`，未设置时回退到 `I:\UE_5.7`
- 可用 `setx UE57_ROOT "你的UE5.7路径"` 配置本机引擎路径
- 插件源码通过 Junction 链接到 `Plugins/NarrRail`（自动创建）

## 目录结构

```
HostProject/
├── Build-HostProject.cmd.template  # 构建脚本模板（提交到 Git）
├── Build-HostProject.cmd           # 本地构建脚本（不提交）
├── NarrRailHost.uproject          # 宿主项目文件
├── Plugins/                        # 插件目录（自动生成）
│   └── NarrRail/                  # Junction 链接到 ../NarrRail
└── Content/                        # 测试资源
```
