# NarrRail Host Project

开发用的 UE5.7 宿主项目，用于加载和测试 NarrRail 插件。

## 首次设置

1. **配置构建脚本**
   ```bash
   # 复制模板文件
   copy Build-HostProject.cmd.template Build-HostProject.cmd
   
   # 编辑 Build-HostProject.cmd，修改引擎路径
   # 将 YOUR_ENGINE_PATH_HERE 改为你的 UE5.7 安装路径
   # 例如: D:\Engine\UE_5.7
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
- 每台开发机器需要根据自己的引擎安装路径配置
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
