<#
.SYNOPSIS
Builds AnycubicSlicerNext for Windows
"
.DESCRIPTION
Port of build_release_macos.sh to PowerShell for Windows build system
#>


# Command line parameters
param(
    [switch]$d,
    [switch]$s,
    [switch]$n,
    [string]$c,
    [string]$g,
    [switch]$h
)

$ErrorActionPreference = "Stop"
$ProgressPreference = "SilentlyContinue"

# Handle help parameter
if ($h) {
    Write-Host "Usage: .\build_release_win.ps1 [-d] [-s] [-n] [-x] [-c <config>] [-g <proxy>] [-h]"
    Write-Host "   -d: Build deps only"
    Write-Host "   -s: Build slicer only"
    Write-Host "   -n: Nightly build"
    Write-Host "   -c: Set build configuration (default: Release)"
    Write-Host "   -g: Set GitHub proxy"
    Write-Host "   -h: Show this help message"
    exit 0
}


function Get-VisualStudioVersion {
    $vswhere = "${env:ProgramFiles(x86)}\Microsoft Visual Studio\Installer\vswhere.exe"
    
    if (Test-Path $vswhere) {
        $vsInfo = & $vswhere -latest -products * -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -format json | ConvertFrom-Json
        
        # 解析完整版本号
        $versionParts = $vsInfo.installationVersion.Split('.')
        $majorVersion = $versionParts[0]
        
        # 生成CMake兼容的生成器名称
        switch ($majorVersion) {
            "17" { return "Visual Studio 17 2022" }
            "16" { return "Visual Studio 16 2019" }
            default { return $null }
        }
    }
    
    $vsPaths = @(
        "${env:ProgramFiles(x86)}\Microsoft Visual Studio\2022\",
        "${env:ProgramFiles(x86)}\Microsoft Visual Studio\2019\"
    )

    foreach ($path in $vsPaths) {
        if (Test-Path (Join-Path $path "Common7\IDE\devenv.exe")) {
            $version = [System.IO.DirectoryInfo]::new($path).Name
            switch ($version) {
                "2022" { return "Visual Studio 17 2022" }
                "2019" { return "Visual Studio 16 2019" }
                default { return $null }
            }
        }
    }
    
    return $null
}



# Set environment variables from parameters
if ($d) { $script:BUILD_TARGET = "deps" }
if ($s) { $script:BUILD_TARGET = "slicer" }
if ($b) { $script:BUILD_ONLY = "1" }
if ($c) { $script:BUILD_CONFIG = $c }
if ($g) { $script:GHPROXY = $g }


if ($script:GHPROXY) {
    # Ensure proxy URL ends with slash
    if (-not ($script:GHPROXY.EndsWith('/'))) {
        $script:GHPROXY += '/'
    }

    # 如果设置了代理，也去确保ippicv的下载使用代理
    # NOTE: 如果opencv版本有变更这里也需要更新
    $IPPICV_COMMIT="a56b6ac6f030c312b2dce17430eef13aed9af274"
    $env:OPENCV_IPPICV_URL=$script:GHPROXY + "https://raw.githubusercontent.com/opencv/opencv_3rdparty/${IPPICV_COMMIT}/ippicv/"
}

# Set default values
$script:ARCH = "x64"


$VS_VERSION = Get-VisualStudioVersion
if ($VS_VERSION) {
    Write-Host "检测到已安装: $VS_VERSION"
    $script:CMAKE_GENERATOR = $VS_VERSION
} else {
    Write-Host "未检测到Visual Studio"
    return 1
}


$script:CMAKE_GENERATOR = if ($env:CMAKE_GENERATOR) { "-G $env:CMAKE_GENERATOR" } else { "-G $script:CMAKE_GENERATOR" }

if (-not $script:BUILD_CONFIG) { $script:BUILD_CONFIG = "Release" }
if (-not $script:BUILD_TARGET) { $script:BUILD_TARGET = "all" }



# Show build parameters
Write-Host "Build params:"
Write-Host " - ARCH: $($script:ARCH)"
Write-Host " - BUILD_CONFIG: $($script:BUILD_CONFIG)"
Write-Host " - BUILD_TARGET: $($script:BUILD_TARGET)"
Write-Host " - CMAKE_GENERATOR: $($script:CMAKE_GENERATOR)"
Write-Host " - GHPROXY: $($script:GHPROXY)"
Write-Host


# Set project directories
$script:PROJECT_DIR = $PSScriptRoot
$script:DEPS_DIR = Join-Path $script:PROJECT_DIR "deps"

function Build_Deps {
    param(
        [string]$DEPS_INSTALL_DIR,
        [string]$DEPS_BUILD_DIR,
        [string]$CONFIG
    )

    if (Test-Path (Join-Path $DEPS_INSTALL_DIR ".finished")) {
        Write-Host "Deps already built, skipping build"
        return
    }

    Write-Host "Building deps... $script:GHPROXY $DEPS_INSTALL_DIR $DEPS_BUILD_DIR $CONFIG"


    $script:ghproxy_arg = if ($script:GHPROXY) { "-DGHPROXY=$($script:GHPROXY)"; }
 
    cmake -S "${DEPS_DIR}" -B "${DEPS_BUILD_DIR}" `
            "${local:CMAKE_GENERATOR}" `
            -DDESTDIR="$DEPS_INSTALL_DIR" -DDEP_DOWNLOAD_DIR="${DEPS_DIR}/DL_CACHE" `
            -DCMAKE_BUILD_TYPE:STRING="$CONFIG" $script:ghproxy_arg

    cmake --build "${DEPS_BUILD_DIR}" --config "$CONFIG" 
    if ($LASTEXITCODE -eq 0) {
        New-Item -ItemType File -Path (Join-Path $DEPS_INSTALL_DIR ".finished") -Force
    }
}


function Build_Slicer {
    param(
        [string]$DEPS_INSTALL_DIR,
        [string]$PROJECT_BUILD_DIR,
        [string]$CONFIG
    )
    
    Write-Host "Building slicer..."
    
    
    $cmakeArgs = @(
        "-S", "$PROJECT_DIR",
        "-B", "$PROJECT_BUILD_DIR",
        "${local:CMAKE_GENERATOR}",
        "-DCMAKE_PREFIX_PATH=$DEPS_INSTALL_DIR/usr/local",
        "-DCMAKE_BUILD_TYPE:STRING=$CONFIG",
        "-DCMAKE_INSTALL_RPATH=${DEPS_INSTALL_DIR}/usr/local"
    )
        
    Write-Host "Running cmake configuration..."
    & cmake $cmakeArgs
    
    # Check if cmake configuration was successful
    if ($LASTEXITCODE -ne 0) {
        Write-Error "CMake configuration failed with exit code $LASTEXITCODE"
        return
    }
   
    
    Write-Host "Building with cmake..."
    & cmake --build "$PROJECT_BUILD_DIR" --config "$CONFIG"
    
    # Check if build was successful
    if ($LASTEXITCODE -ne 0) {
        Write-Error "CMake build failed with exit code $LASTEXITCODE"
        return
    }
    
    Write-Host "Verify localization with gettext..."
    Push-Location $PROJECT_DIR/scripts
    try {
        & "$PROJECT_DIR/scripts/run_gettext.bat"
        
        # Check if script execution was successful
        if ($LASTEXITCODE -ne 0) {
            Write-Warning "Gettext verification failed with exit code $LASTEXITCODE"
        }
    } finally {
        Pop-Location
    }
}

$DEPS_BUILD_DIR = Join-Path $DEPS_DIR "build_$($local:BUILD_CONFIG)"
$DEPS_INSTALL_DIR = Join-Path $PROJECT_DIR "deps_install\$($local:BUILD_CONFIG)"
$PROJECT_BUILD_DIR = Join-Path $PROJECT_DIR "build_slicer\$($local:BUILD_CONFIG)"

switch ($local:BUILD_TARGET) {
    'all' {
        Build_Deps $DEPS_INSTALL_DIR $DEPS_BUILD_DIR $local:BUILD_CONFIG
        Build_Slicer $DEPS_INSTALL_DIR $PROJECT_BUILD_DIR $local:BUILD_CONFIG
    }
    'deps' {
        Build_Deps $DEPS_INSTALL_DIR $DEPS_BUILD_DIR $local:BUILD_CONFIG
    }
    'slicer' {
        Build_Slicer $DEPS_INSTALL_DIR $PROJECT_BUILD_DIR $local:BUILD_CONFIG
        
    }
    default {
        Write-Error "Unknown target: $($local:BUILD_TARGET). Available targets: deps, slicer, all."
        exit 1
    }
}


