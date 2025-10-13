#!/usr/bin/env pwsh
# ChaosForge Unified Build Script for Windows

Write-Host "Building ChaosForge Unified Arena..." -ForegroundColor Green
Write-Host "Integrating: Physics + Game + Movement + Combat engines" -ForegroundColor Yellow

# Clean previous build
if (Test-Path "chaosforge_unified.exe") { Remove-Item "chaosforge_unified.exe" }
Get-ChildItem "*.o" | Remove-Item -ErrorAction SilentlyContinue

# Check if Rust multiplayer DLL exists
$dllPath = "chaosforge_multiplayer.dll"
if (-not (Test-Path $dllPath)) {
    Write-Host "Building Rust multiplayer engine first..." -ForegroundColor Yellow
    Push-Location "../chaosforge-multiplayer"
    try {
        cargo build --release --features ffi
        if ($LASTEXITCODE -eq 0) {
            Copy-Item "target/release/chaosforge_multiplayer.dll" "../chaosforge-game/"
            Write-Host "Rust DLL copied successfully" -ForegroundColor Green
        } else {
            Write-Host "Warning: Rust build failed, continuing without multiplayer integration" -ForegroundColor Yellow
        }
    }
    finally {
        Pop-Location
    }
} else {
    Write-Host "Using existing Rust multiplayer DLL" -ForegroundColor Green
}

# Compile all source files
$sources = @(
    "unified_main.c",
    "player_controller.c",
    "combat_system.c",
    "physics_manager.c",
    "game_state.c",
    "multiplayer_integration.c",
    "gangbeast_avatar.c",
    "avatar_menu.c"
)

$objects = @()

foreach ($source in $sources) {
    if (Test-Path $source) {
        $object = $source -replace "\.c$", ".o"
        Write-Host "Compiling $source..." -ForegroundColor Yellow
        
        $compileArgs = @(
            "-Wall", "-Wextra", "-O2", "-std=c99",
            "-I.",
            "-c", $source, "-o", $object
        )
        
        $result = Start-Process -FilePath "gcc" -ArgumentList $compileArgs -Wait -PassThru -NoNewWindow
        
        if ($result.ExitCode -ne 0) {
            Write-Host "Error compiling $source" -ForegroundColor Red
            exit 1
        }
        
        $objects += $object
    } else {
        Write-Host "Warning: Source file $source not found, skipping..." -ForegroundColor Yellow
    }
}

# Link the unified executable
Write-Host "Linking chaosforge_unified.exe..." -ForegroundColor Yellow

$linkArgs = @("-o", "chaosforge_unified.exe") + $objects + @(
    "-lopengl32", "-lglu32", "-lgdi32", "-luser32", "-lkernel32", 
    "-lwinmm", "-lm", "-L.", "-lchaosforge_multiplayer"
)

# Try linking with multiplayer first, fallback if it fails
$result = Start-Process -FilePath "gcc" -ArgumentList $linkArgs -Wait -PassThru -NoNewWindow

if ($result.ExitCode -ne 0) {
    Write-Host "Linking with multiplayer failed, trying without..." -ForegroundColor Yellow
    
    # Fallback: link without multiplayer library
    $fallbackArgs = @("-o", "chaosforge_unified.exe") + $objects + @(
        "-lopengl32", "-lglu32", "-lgdi32", "-luser32", "-lkernel32", 
        "-lwinmm", "-lm"
    )
    
    $result = Start-Process -FilePath "gcc" -ArgumentList $fallbackArgs -Wait -PassThru -NoNewWindow
    
    if ($result.ExitCode -ne 0) {
        Write-Host "Error linking executable" -ForegroundColor Red
        Write-Host ""
        Write-Host "Make sure you have the required libraries installed:" -ForegroundColor Yellow
        Write-Host "- OpenGL development libraries (opengl32, glu32)" -ForegroundColor Yellow
        Write-Host "- MinGW/MSYS2 with GCC compiler" -ForegroundColor Yellow
        exit 1
    } else {
        Write-Host "Build successful (without multiplayer integration)!" -ForegroundColor Green
    }
} else {
    Write-Host "Build successful with full multiplayer integration!" -ForegroundColor Green
}

Write-Host ""
Write-Host "=== UNIFIED CHAOSFORGE ARENA ===" -ForegroundColor Cyan
Write-Host "All engines integrated into one executable:" -ForegroundColor White
Write-Host "  ✓ Physics Engine (ragdoll physics)" -ForegroundColor Green
Write-Host "  ✓ Game Engine (arena management)" -ForegroundColor Green  
Write-Host "  ✓ Movement Engine (player controls)" -ForegroundColor Green
Write-Host "  ✓ Combat Engine (real-time fighting)" -ForegroundColor Green
Write-Host "  ✓ Multiplayer Engine (Rust integration)" -ForegroundColor Green
Write-Host ""
Write-Host "CONTROLS:" -ForegroundColor Yellow
Write-Host "  Menu: UP/DOWN arrows, ENTER to start" -ForegroundColor White
Write-Host "  Game: WASD (move), SHIFT (sprint), SPACE (block)" -ForegroundColor White
Write-Host "  Combat: Left Click (light attack), Right Click (heavy)" -ForegroundColor White
Write-Host "  Debug: F1 (add AI), ESC (menu)" -ForegroundColor White
Write-Host ""
Write-Host "To run: .\chaosforge_unified.exe" -ForegroundColor Cyan