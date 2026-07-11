# NarrRail Unreal Plugin Compatibility and Setup

> 文档状态：UE Story Consumer 文档。本文随 `NarrRail-Unreal-Plugin` 维护，并声明对主仓库中立格式契约的兼容关系。

本文档说明当前 Unreal 插件版本与 NarrRail 中立故事格式契约之间的兼容关系，并给出安装、检查和试用插件的最小步骤。

中立格式契约由主仓库维护：

- <https://github.com/Courtshipfy/NarrRail/blob/main/Docs/spec/NRSTORY_FORMAT.md>
- <https://github.com/Courtshipfy/NarrRail/blob/main/Docs/02_runtime/SCRIPT_FORMAT.md> 是运行时侧历史入口

Unreal 插件是 Story Consumer。它读取主仓库定义的 `.nrstory` / GlobalConfig 格式，并映射为 UE 资产和运行时对象；它不拥有格式语义。

## 1. Versioning model

NarrRail 使用两条独立版本线：

| 版本线 | 所属仓库 | 当前值 | 说明 |
| --- | --- | --- | --- |
| Unreal plugin version | 当前主仓库，未来 `NarrRail-Unreal-Plugin` | `0.1.0-beta` | 来自 `NarrRail/NarrRail.uplugin` 的 `VersionName` |
| Story Script schema version | 主仓库 | `1` | 来自 `.nrstory` 的 `meta.schemaVersion` |
| GlobalConfig schema version | 主仓库 | `1` | 来自 GlobalConfig `.nrstory` 的 `meta.schemaVersion` |
| Story Outline schema version | 主仓库 | `1` | 来自 `.nroutline` 的 `meta.schemaVersion`，当前 UE consumer 不导入 |

插件版本升级不等于格式版本升级。格式字段、语义或 schema 变化必须先在主仓库通过 issue 和兼容说明确定，然后 UE consumer 再声明支持。

## 2. Compatibility matrix

| Unreal plugin version | UE version | Story Script `.nrstory` | GlobalConfig `.nrstory` | `.nroutline` | Legacy `.nrrail` | Notes |
| --- | --- | --- | --- | --- | --- | --- |
| `0.1.0-beta` | UE `5.7` | Supports `schemaVersion: 1` | Supports `schemaVersion: 1` with `meta.configType: GlobalConfig` | Not supported for UE import/runtime | Not supported for UE import/runtime | Current technical preview. Story repository sync scans `*.nrstory` only. |

## 3. Supported story features

| Area | Support | Notes |
| --- | --- | --- |
| YAML `.nrstory` import | Supported | Imported through the editor module and repository sync |
| Story repository sync | Supported | Mirrors `*.nrstory` into `/Game/NarrRailStories/<repository-folder-name>` |
| GlobalConfig import | Supported | Generates `UNarrRailGlobalConfigAsset` |
| Automatic StoryAsset to GlobalConfig binding | Supported | Repository sync binds story assets to the repository GlobalConfig |
| Node types | Supported | `Dialogue`, `MultiDialogue`, `Choice`, `Jump`, `SetVariable`, `EmitEvent`, `Condition`, `End` |
| Variables | Supported | `Bool`, `Int`, `Float`, `String` global variables |
| Actions | Supported | `Set`, `Add`, `Subtract`, `EmitEvent` |
| Multi-branch Condition | Supported | Uses `condition-N` and `condition-fallback` source handles |
| Legacy single Condition | Compatible read path | Maps true to `condition-0` and false to `condition-fallback` |
| Deprecated `edges[].condition` | Rejected | Use Condition nodes instead |
| Choice modes | Partially supported | `SinglePass` and `ExhaustiveUntilComplete` are represented by runtime data |
| Choice timer / `choice-timeout` | Not declared supported | Do not rely on UE runtime timeout routing until implemented and documented |
| `.nroutline` project preview | Not supported | Authoring product feature only for now |
| Unknown field preservation | Not guaranteed | Current importer maps supported fields into UE assets; unsupported fields may be dropped |

## 4. Setup from this package

The plugin and sample host are stored in this package with the same directory names planned for the standalone repository:

```text
NarrRail/
NarrRailUEHost/
```

### 4.1 Prerequisites

- Windows development environment for Unreal Engine.
- Unreal Engine `5.7`.
- Git available on `PATH` if using `Pull Git Repository Before Sync`.
- A local Story Project or story repository containing `.nrstory` files.

### 4.2 Configure the host project

From `NarrRailUEHost/`, copy the local build script template:

```cmd
copy Build-NarrRailUEHost.cmd.template Build-NarrRailUEHost.cmd
```

Set the UE engine path:

```cmd
setx UE57_ROOT "I:\UE_5.7"
```

If `UE57_ROOT` is not set, the script currently falls back to `I:\UE_5.7`.

### 4.3 Build and open

```cmd
Build-NarrRailUEHost.cmd
```

Then open:

```text
NarrRailUEHost/NarrRailUEHost.uproject
```

The build script checks or creates:

```text
NarrRailUEHost/Plugins/NarrRail -> ../../NarrRail
```

Use a Junction or symlink; a normal Windows `.lnk` shortcut may not be recognized by Unreal.

## 5. Try story repository sync

1. Open `NarrRailUEHost/NarrRailUEHost.uproject`.
2. Open `Project Settings > Plugins > NarrRail`.
3. Set `Story Repository Path` to the folder that contains `.nrstory` files.
4. Keep `Pull Git Repository Before Sync` enabled only if the folder is a Git work tree and local changes are clean.
5. Keep `Story Asset Root Path (UE Content)` as `/Game/NarrRailStories` unless the project needs a custom location.
6. Click `Sync Stories`.
7. Inspect generated assets under `/Game/NarrRailStories/<repository-folder-name>`.
8. Use the generated `UNarrRailStoryAsset` in PIE or Blueprint/C++ runtime flow.

Sync currently scans `*.nrstory` only. `.nroutline` and `.nrrail` files should remain in the authoring project until UE support is explicitly implemented.

## 6. Requesting format changes

Unreal-side issues may reveal format needs, but format changes must start in the main repository because the main repository owns the neutral story contract.

Use this flow:

1. Open or reference a main-repo issue describing the requested `.nrstory`, GlobalConfig, or `.nroutline` change.
2. Explain the Unreal use case as a Story Consumer requirement.
3. Wait for the main contract, validation, migration, and compatibility decision.
4. Update the Unreal plugin compatibility matrix only after the format change is accepted.

Do not add UE-only fields to `.nrstory`, GlobalConfig, or `.nroutline` as a private extension without a main-repo compatibility decision.
