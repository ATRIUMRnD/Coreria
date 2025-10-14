#!/usr/bin/env pwsh
# Enhanced Unified Build Script - Builds all integrated systems
# File: chaosforge-game/build_enhanced.ps1

Write-Host "=== ENHANCED CHAOSFORGE BUILD SYSTEM ===" -ForegroundColor Green
Write-Host "Integrating Combat + Animation + Physics + Multiplayer FFI" -ForegroundColor Yellow

# Check prerequisites
$gccVersion = try { (gcc --version 2>$null)[0] } catch { $null }
$cargoVersion = try { (cargo --version 2>$null) } catch { $null }

if (-not $gccVersion) {
    Write-Host "ERROR: GCC compiler not found. Please install MinGW/MSYS2" -ForegroundColor Red
    exit 1
}

if (-not $cargoVersion) {
    Write-Host "WARNING: Rust/Cargo not found. Building without Rust FFI integration" -ForegroundColor Yellow
    $buildRust = $false
} else {
    Write-Host "Found: $gccVersion" -ForegroundColor Green
    Write-Host "Found: $cargoVersion" -ForegroundColor Green
    $buildRust = $true
}

# Clean previous build
Write-Host "`nCleaning previous build..." -ForegroundColor Yellow
Get-ChildItem "*.exe", "*.o", "*.dll" -ErrorAction SilentlyContinue | Remove-Item -Force
if (Test-Path "combat_log.txt") { Remove-Item "combat_log.txt" -Force }
if (Test-Path "performance_log.txt") { Remove-Item "performance_log.txt" -Force }

# Build Rust multiplayer library if available
if ($buildRust) {
    Write-Host "`nBuilding Rust multiplayer library..." -ForegroundColor Yellow
    Set-Location "..\chaosforge-multiplayer"
    
    $rustResult = Start-Process -FilePath "cargo" -ArgumentList @("build", "--release", "--features", "ffi") -Wait -PassThru -NoNewWindow
    
    if ($rustResult.ExitCode -eq 0) {
        Write-Host "Rust library built successfully" -ForegroundColor Green
        
        # Copy DLL to game directory
        $dllSource = "target\release\chaosforge_multiplayer.dll"
        $dllTarget = "..\chaosforge-game\chaosforge_multiplayer.dll"
        
        if (Test-Path $dllSource) {
            Copy-Item $dllSource $dllTarget -Force
            Write-Host "Copied multiplayer DLL to game directory" -ForegroundColor Green
        }
        
        # Copy header if available
        $headerSource = "target\chaosforge_multiplayer.h"
        $headerTarget = "..\chaosforge-game\chaosforge_multiplayer.h"
        
        if (Test-Path $headerSource) {
            Copy-Item $headerSource $headerTarget -Force
            Write-Host "Copied C header to game directory" -ForegroundColor Green
        }
    } else {
        Write-Host "Rust library build failed, continuing without FFI integration" -ForegroundColor Yellow
        $buildRust = $false
    }
    
    Set-Location "..\chaosforge-game"
}

# Define source files and compilation order
$coreSources = @(
    "combat_system.c",
    "physics_manager.c", 
    "game_state.c",
    "player_controller.c",
    "gangbeast_avatar.c",
    "enhanced_combat_system.c"
)

$mainSources = @(
    "enhanced_unified_main.c"
)

$allSources = $coreSources + $mainSources

# Compile flags
$commonFlags = @(
    "-Wall", "-Wextra", "-O2", "-std=c99",
    "-DENHANCED_COMBAT_BUILD"
)

if ($buildRust) {
    $commonFlags += "-DMULTIPLAYER_FFI_AVAILABLE"
}

# Link libraries
$linkLibs = @(
    "-lopengl32", "-lglu32", "-lgdi32", "-luser32", "-lkernel32", "-lm"
)

if ($buildRust -and (Test-Path "chaosforge_multiplayer.dll")) {
    $linkLibs += @("-L.", "-lchaosforge_multiplayer")
}

Write-Host "`nCompiling source files..." -ForegroundColor Yellow

$objects = @()
$compileSuccess = $true

foreach ($source in $coreSources) {
    if (Test-Path $source) {
        $object = $source -replace "\.c$", ".o"
        Write-Host "  Compiling $source..." -ForegroundColor Cyan
        
        $compileArgs = $commonFlags + @("-c", $source, "-o", $object)
        $result = Start-Process -FilePath "gcc" -ArgumentList $compileArgs -Wait -PassThru -NoNewWindow
        
        if ($result.ExitCode -eq 0) {
            $objects += $object
            Write-Host "    ✓ Success" -ForegroundColor Green
        } else {
            Write-Host "    ✗ Failed" -ForegroundColor Red
            $compileSuccess = $false
        }
    } else {
        Write-Host "  Skipping $source (not found)" -ForegroundColor Yellow
    }
}

if (-not $compileSuccess) {
    Write-Host "`nBuild failed during compilation phase" -ForegroundColor Red
    exit 1
}

# Compile main file
foreach ($mainSource in $mainSources) {
    if (Test-Path $mainSource) {
        $mainObject = $mainSource -replace "\.c$", ".o"
        Write-Host "  Compiling $mainSource..." -ForegroundColor Cyan
        
        $compileArgs = $commonFlags + @("-c", $mainSource, "-o", $mainObject)
        $result = Start-Process -FilePath "gcc" -ArgumentList $compileArgs -Wait -PassThru -NoNewWindow
        
        if ($result.ExitCode -eq 0) {
            $objects += $mainObject
            Write-Host "    ✓ Success" -ForegroundColor Green
        } else {
            Write-Host "    ✗ Failed" -ForegroundColor Red
            exit 1
        }
    }
}

# Link executable
Write-Host "`nLinking enhanced executable..." -ForegroundColor Yellow

$linkArgs = @("-o", "chaosforge_enhanced.exe") + $objects + $linkLibs
$linkResult = Start-Process -FilePath "gcc" -ArgumentList $linkArgs -Wait -PassThru -NoNewWindow

if ($linkResult.ExitCode -eq 0) {
    Write-Host "✓ Enhanced executable created successfully!" -ForegroundColor Green
    
    # Display build summary
    Write-Host "`n=== BUILD SUMMARY ===" -ForegroundColor Green
    Write-Host "Executable: chaosforge_enhanced.exe" -ForegroundColor White
    Write-Host "Systems integrated:" -ForegroundColor White
    Write-Host "  ✓ Enhanced Combat System" -ForegroundColor Green
    Write-Host "  ✓ Avatar Animation System" -ForegroundColor Green
    Write-Host "  ✓ Performance Monitoring" -ForegroundColor Green
    
    if ($buildRust) {
        Write-Host "  ✓ Rust Multiplayer FFI" -ForegroundColor Green
        Write-Host "  ✓ Physics Integration" -ForegroundColor Green
    } else {
        Write-Host "  ⚠ Rust FFI (not available)" -ForegroundColor Yellow
    }
    
    $fileSize = (Get-Item "chaosforge_enhanced.exe").Length
    Write-Host "File size: $([math]::Round($fileSize / 1MB, 2)) MB" -ForegroundColor White
    
    # Create run script
    @"
@echo off
echo === CHAOSFORGE ENHANCED COMBAT SYSTEM ===
echo Starting enhanced game with integrated systems...
echo.
echo Real-time logging enabled:
echo - combat_log.txt (combat events and system status)
echo - performance_log.txt (FPS and timing metrics)
echo.
echo Controls:
echo   Left Click  - Light Punch
echo   Right Click - Heavy Kick
echo   WASD        - Movement
echo   SPACE       - Block
echo   F1          - Add AI Player
echo   F12         - Export Metrics
echo   ESC         - Exit
echo.
pause
chaosforge_enhanced.exe
pause
"@ | Out-File -FilePath "run_enhanced.bat" -Encoding ASCII

    Write-Host "`nCreated run_enhanced.bat for easy execution" -ForegroundColor Green
    
    Write-Host "`n=== READY TO RUN ===" -ForegroundColor Green
    Write-Host "Execute: .\run_enhanced.bat" -ForegroundColor White
    Write-Host "Or directly: .\chaosforge_enhanced.exe" -ForegroundColor White
    
} else {
    Write-Host "✗ Linking failed" -ForegroundColor Red
    
    Write-Host "`nTroubleshooting:" -ForegroundColor Yellow
    Write-Host "1. Ensure all required libraries are installed" -ForegroundColor White
    Write-Host "2. Check that OpenGL development libraries are available" -ForegroundColor White
    Write-Host "3. Verify MinGW/MSYS2 installation includes all required components" -ForegroundColor White
    
    if ($buildRust) {
        Write-Host "4. Ensure Rust library built correctly and DLL is accessible" -ForegroundColor White
    }
    
    exit 1
}

Write-Host "`nBuild completed!" -ForegroundColor Green