
[CmdletBinding()]
param(
    [string]$EngineRoot,
    [ValidateSet("Win64")]
    [string]$Platform = "Win64",
    [ValidateSet("Development", "DebugGame", "Debug")]
    [string]$Configuration = "Development",
    [string]$Target = "UnrealEditor",
    [switch]$Clean,
    [switch]$SkipSetup
)

$ErrorActionPreference = "Stop"

function Test-EngineRoot {
    param([string]$Path)

    if ([string]::IsNullOrWhiteSpace($Path)) {
        return $false
    }

    $buildBat = Join-Path $Path "Engine\Build\BatchFiles\Build.bat"
    return (Test-Path -LiteralPath $buildBat)
}

function Resolve-EngineRoot {
    param(
        [string]$ExplicitEngineRoot,
        [string]$ProjectFile
    )

    $candidates = New-Object System.Collections.Generic.List[string]

    if (-not [string]::IsNullOrWhiteSpace($ExplicitEngineRoot)) {
        $candidates.Add($ExplicitEngineRoot)
    }

    if (-not [string]::IsNullOrWhiteSpace($env:UE_ENGINE_ROOT)) {
        $candidates.Add($env:UE_ENGINE_ROOT)
    }

    $engineAssociation = $null
    try {
        $uprojectJson = Get-Content -LiteralPath $ProjectFile -Raw | ConvertFrom-Json
        $engineAssociation = $uprojectJson.EngineAssociation
    } catch {
        Write-Host "Warning: failed to parse EngineAssociation from $ProjectFile"
    }

    if (-not [string]::IsNullOrWhiteSpace($engineAssociation)) {
        $candidates.Add("C:\Program Files\Epic Games\UE_$engineAssociation")
        $candidates.Add("D:\Epic Games\UE_$engineAssociation")
        $candidates.Add("E:\Epic Games\UE_$engineAssociation")
    }

    $registryPath = "HKCU:\Software\Epic Games\Unreal Engine\Builds"
    if (Test-Path -LiteralPath $registryPath) {
        $builds = Get-ItemProperty -LiteralPath $registryPath
        foreach ($prop in $builds.PSObject.Properties) {
            if ($prop.Name -like "PS*") {
                continue
            }
            if (-not [string]::IsNullOrWhiteSpace($prop.Value)) {
                if ($prop.Name -eq $engineAssociation) {
                    $candidates.Insert(0, $prop.Value)
                } else {
                    $candidates.Add($prop.Value)
                }
            }
        }
    }

    foreach ($candidate in $candidates) {
        if (Test-EngineRoot -Path $candidate) {
            return [System.IO.Path]::GetFullPath($candidate)
        }
    }

    throw "Unable to find Unreal Engine root. Pass -EngineRoot, or set UE_ENGINE_ROOT."
}

$scriptDir = Split-Path -Parent $PSCommandPath
$projectRoot = [System.IO.Path]::GetFullPath((Join-Path $scriptDir ".."))
$repoRoot = [System.IO.Path]::GetFullPath((Join-Path $projectRoot ".."))
$uproject = Join-Path $projectRoot "NarrRailHost.uproject"
$pluginRoot = Join-Path $repoRoot "NarrRail"
$setupScript = Join-Path $repoRoot "scripts\Setup-HostProject.ps1"

if (-not (Test-Path -LiteralPath $uproject)) {
    throw "Host project file not found: $uproject"
}

if (-not $SkipSetup) {
    if (-not (Test-Path -LiteralPath $setupScript)) {
        throw "Setup script not found: $setupScript"
    }
    & $setupScript
}

if ($Clean) {
    $cleanPaths = @(
        (Join-Path $projectRoot "Binaries"),
        (Join-Path $projectRoot "Intermediate"),
        (Join-Path $projectRoot "Saved"),
        (Join-Path $pluginRoot "Binaries"),
        (Join-Path $pluginRoot "Intermediate")
    )

    foreach ($path in $cleanPaths) {
        if (Test-Path -LiteralPath $path) {
            Remove-Item -LiteralPath $path -Recurse -Force
            Write-Host "Removed: $path"
        }
    }
}

$resolvedEngineRoot = Resolve-EngineRoot -ExplicitEngineRoot $EngineRoot -ProjectFile $uproject
$buildBat = Join-Path $resolvedEngineRoot "Engine\Build\BatchFiles\Build.bat"

Write-Host "EngineRoot: $resolvedEngineRoot"
Write-Host "Project: $uproject"
Write-Host "Target: $Target $Platform $Configuration"

& $buildBat $Target $Platform $Configuration "-Project=$uproject" "-WaitMutex" "-NoHotReloadFromIDE"

if ($LASTEXITCODE -ne 0) {
    throw "Build failed with exit code $LASTEXITCODE"
}

Write-Host "Build succeeded."
