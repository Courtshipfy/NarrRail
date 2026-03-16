param(
    [string]$ProjectRoot = (Join-Path $PSScriptRoot "..\\HostProject"),
    [string]$PluginSource = (Join-Path $PSScriptRoot "..\\NarrRail"),
    [switch]$Force
)

$ErrorActionPreference = "Stop"

$projectRootFull = [System.IO.Path]::GetFullPath($ProjectRoot)
$pluginSourceFull = [System.IO.Path]::GetFullPath($PluginSource)
$pluginsDir = Join-Path $projectRootFull "Plugins"
$linkPath = Join-Path $pluginsDir "NarrRail"
$uprojectPath = Join-Path $projectRootFull "NarrRailHost.uproject"

if (-not (Test-Path -LiteralPath $pluginSourceFull)) {
    throw "Plugin source not found: $pluginSourceFull"
}

if (-not (Test-Path -LiteralPath $uprojectPath)) {
    throw "Host project not found: $uprojectPath"
}

New-Item -ItemType Directory -Path $pluginsDir -Force | Out-Null

if (Test-Path -LiteralPath $linkPath) {
    $existing = Get-Item -LiteralPath $linkPath -Force
    $isReparsePoint = ($existing.Attributes -band [System.IO.FileAttributes]::ReparsePoint) -ne 0

    if ($isReparsePoint -and -not $Force) {
        Write-Host "Plugin link already exists: $linkPath"
        Write-Host "Use -Force to recreate it."
        exit 0
    }

    if (-not $Force) {
        throw "Path exists and is not a managed link: $linkPath. Use -Force to replace it."
    }

    Remove-Item -LiteralPath $linkPath -Recurse -Force
}

New-Item -ItemType Junction -Path $linkPath -Target $pluginSourceFull | Out-Null

Write-Host "Created plugin junction:"
Write-Host "  $linkPath -> $pluginSourceFull"
Write-Host ""
Write-Host "Next:"
Write-Host "1) Open $uprojectPath in Unreal Engine 5.7."
Write-Host "2) If prompted, rebuild modules."
Write-Host "3) Enable NarrRail plugin (if not enabled by default)."

