#!/usr/bin/env pwsh
<#
.SYNOPSIS
    Task management script for Coreria project features
.DESCRIPTION
    Manages feature development tasks, builds, and testing for the Coreria project
.PARAMETER FeatureName
    Name of the feature to work on (e.g., "011-enhance-chaosforge-multiplayer")
.PARAMETER Action
    Action to perform: build, test, clean, run, or setup
.EXAMPLE
    .\tasks.ps1 -FeatureName "011-enhance-chaosforge-multiplayer" -Action build
#>

param(
    [Parameter(Mandatory=$true)]
    [string]$FeatureName,
    
    [Parameter(Mandatory=$false)]
    [ValidateSet("build", "test", "clean", "run", "setup", "status")]
    [string]$Action = "build"
)

# Set error action preference
$ErrorActionPreference = "Stop"

# Get project root directory
$ProjectRoot = Split-Path -Parent $PSScriptRoot
Write-Host "Project Root: $ProjectRoot" -ForegroundColor Green

# Feature-specific configurations
$FeatureConfigs = @{
    "011-enhance-chaosforge-multiplayer" = @{
        "RustProject" = "chaosforge-multiplayer"
        "CProject" = "chaosforge-game"
        "Features" = @("ffi", "multiplayer-physics")
        "Description" = "Enhanced multiplayer support with Rust backend"
    }
    "010-there-s-a" = @{
        "CProject" = "chaosforge-game"
        "Description" = "Fix gray blob and implement style selector overlay"
    }
}

function Write-FeatureHeader {
    param([string]$Feature, [string]$Action)
    
    Write-Host "=" * 60 -ForegroundColor Cyan
    Write-Host "CORERIA PROJECT - FEATURE TASK RUNNER" -ForegroundColor Cyan
    Write-Host "=" * 60 -ForegroundColor Cyan
    Write-Host "Feature: $Feature" -ForegroundColor Yellow
    Write-Host "Action:  $Action" -ForegroundColor Yellow
    Write-Host "Time:    $(Get-Date -Format 'yyyy-MM-dd HH:mm:ss')" -ForegroundColor Gray
    Write-Host ""
}

function Build-RustProject {
    param([string]$ProjectPath, [string[]]$Features)
    
    Write-Host "Building Rust project: $ProjectPath" -ForegroundColor Green
    
    if (-not (Test-Path $ProjectPath)) {
        throw "Rust project directory not found: $ProjectPath"
    }
    
    Push-Location $ProjectPath
    try {
        # Clean previous build
        Write-Host "Cleaning previous build..." -ForegroundColor Yellow
        cargo clean
        
        # Build with features
        $FeatureFlag = if ($Features) { "--features " + ($Features -join ",") } else { "" }
        Write-Host "Building with features: $($Features -join ', ')" -ForegroundColor Yellow
        
        $BuildCommand = "cargo build $FeatureFlag"
        Write-Host "Executing: $BuildCommand" -ForegroundColor Gray
        Invoke-Expression $BuildCommand
        
        if ($LASTEXITCODE -eq 0) {
            Write-Host "Rust build successful" -ForegroundColor Green
        } else {
            throw "Rust build failed with exit code $LASTEXITCODE"
        }
    }
    finally {
        Pop-Location
    }
}

function Build-CProject {
    param([string]$ProjectPath)
    
    Write-Host "Building C project: $ProjectPath" -ForegroundColor Green
    
    if (-not (Test-Path $ProjectPath)) {
        throw "C project directory not found: $ProjectPath"
    }
    
    Push-Location $ProjectPath
    try {
        # Look for build script
        $BuildScript = $null
        $PossibleScripts = @("build_windows.bat", "build.bat", "build.ps1", "Makefile")
        
        foreach ($Script in $PossibleScripts) {
            if (Test-Path $Script) {
                $BuildScript = $Script
                break
            }
        }
        
        if (-not $BuildScript) {
            throw "No build script found in $ProjectPath"
        }
        
        Write-Host "Using build script: $BuildScript" -ForegroundColor Yellow
        
        if ($BuildScript.EndsWith(".bat")) {
            & ".\$BuildScript"
        } elseif ($BuildScript.EndsWith(".ps1")) {
            & ".\$BuildScript"
        } elseif ($BuildScript -eq "Makefile") {
            make
        }
        
        if ($LASTEXITCODE -eq 0) {
            Write-Host "C build successful" -ForegroundColor Green
        } else {
            throw "C build failed with exit code $LASTEXITCODE"
        }
    }
    finally {
        Pop-Location
    }
}

function Test-Projects {
    param([hashtable]$Config)
    
    Write-Host "Running tests..." -ForegroundColor Green
    
    if ($Config.RustProject) {
        $RustPath = Join-Path $ProjectRoot $Config.RustProject
        if (Test-Path $RustPath) {
            Push-Location $RustPath
            try {
                Write-Host "Running Rust tests..." -ForegroundColor Yellow
                cargo test
                if ($LASTEXITCODE -eq 0) {
                    Write-Host "Rust tests passed" -ForegroundColor Green
                } else {
                    Write-Host "Rust tests failed" -ForegroundColor Red
                }
            }
            finally {
                Pop-Location
            }
        }
    }
    
    if ($Config.CProject) {
        Write-Host "C project testing not implemented yet" -ForegroundColor Yellow
    }
}

function Show-Status {
    param([hashtable]$Config)
    
    Write-Host "Project Status:" -ForegroundColor Green
    Write-Host "Description: $($Config.Description)" -ForegroundColor Gray
    
    if ($Config.RustProject) {
        $RustPath = Join-Path $ProjectRoot $Config.RustProject
        Write-Host "Rust Project: $RustPath" -ForegroundColor Yellow
        Write-Host "  Exists: $(Test-Path $RustPath)" -ForegroundColor Gray
        Write-Host "  Features: $($Config.Features -join ', ')" -ForegroundColor Gray
    }
    
    if ($Config.CProject) {
        $CPath = Join-Path $ProjectRoot $Config.CProject
        Write-Host "C Project: $CPath" -ForegroundColor Yellow
        Write-Host "  Exists: $(Test-Path $CPath)" -ForegroundColor Gray
    }
}

# Main execution
try {
    Write-FeatureHeader -Feature $FeatureName -Action $Action
    
    # Get feature configuration
    if (-not $FeatureConfigs.ContainsKey($FeatureName)) {
        throw "Unknown feature: $FeatureName. Available features: $($FeatureConfigs.Keys -join ', ')"
    }
    
    $Config = $FeatureConfigs[$FeatureName]
    
    switch ($Action) {
        "build" {
            if ($Config.RustProject) {
                $RustPath = Join-Path $ProjectRoot $Config.RustProject
                Build-RustProject -ProjectPath $RustPath -Features $Config.Features
            }
            
            if ($Config.CProject) {
                $CPath = Join-Path $ProjectRoot $Config.CProject
                Build-CProject -ProjectPath $CPath
            }
        }
        
        "test" {
            Test-Projects -Config $Config
        }
        
        "clean" {
            Write-Host "Cleaning projects..." -ForegroundColor Green
            
            if ($Config.RustProject) {
                $RustPath = Join-Path $ProjectRoot $Config.RustProject
                if (Test-Path $RustPath) {
                    Push-Location $RustPath
                    cargo clean
                    Pop-Location
                    Write-Host "Rust project cleaned" -ForegroundColor Green
                }
            }
            
            if ($Config.CProject) {
                Write-Host "C project cleaning not implemented yet" -ForegroundColor Yellow
            }
        }
        
        "status" {
            Show-Status -Config $Config
        }
        
        "setup" {
            Write-Host "Setting up development environment..." -ForegroundColor Green
            Write-Host "This would install dependencies and configure the environment" -ForegroundColor Yellow
        }
        
        "run" {
            Write-Host "Running projects..." -ForegroundColor Green
            
            if ($Config.CProject) {
                $CPath = Join-Path $ProjectRoot $Config.CProject
                $ExePath = Join-Path $CPath "chaosforge_win.exe"
                if (Test-Path $ExePath) {
                    Write-Host "Starting C application..." -ForegroundColor Yellow
                    Start-Process -FilePath $ExePath -WorkingDirectory $CPath
                } else {
                    Write-Host "Executable not found: $ExePath" -ForegroundColor Red
                }
            }
        }
    }
    
    Write-Host ""
    Write-Host "Task completed successfully!" -ForegroundColor Green

} catch {
    Write-Host ""
    Write-Host "Task failed: $($_.Exception.Message)" -ForegroundColor Red
    exit 1
}
