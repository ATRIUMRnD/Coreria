#!/usr/bin/env pwsh
# ChaosForge Arena Build Script for Windows

Write-Host "Building ChaosForge Arena..." -ForegroundColor Green

# Clean previous build
if (Test-Path "chaosforge.exe") { Remove-Item "chaosforge.exe" }
Get-ChildItem "*.o" | Remove-Item -ErrorAction SilentlyContinue

# Compile object files
$sources = @("main.c", "player_controller.c", "combat_system.c", "physics_manager.c", "game_state.c")
$objects = @()

foreach ($source in $sources) {
    $object = $source -replace "\.c$", ".o"
    Write-Host "Compiling $source..." -ForegroundColor Yellow
    
    $result = Start-Process -FilePath "gcc" -ArgumentList @("-Wall", "-c", $source, "-o", $object) -Wait -PassThru -NoNewWindow
    
    if ($result.ExitCode -ne 0) {
        Write-Host "Error compiling $source" -ForegroundColor Red
        exit 1
    }
    
    $objects += $object
}

# Link executable
Write-Host "Linking chaosforge.exe..." -ForegroundColor Yellow

$linkArgs = @("-o", "chaosforge.exe") + $objects + @("-lopengl32", "-lglu32", "-lgdi32", "-luser32", "-lkernel32", "-lm")
$result = Start-Process -FilePath "gcc" -ArgumentList $linkArgs -Wait -PassThru -NoNewWindow

if ($result.ExitCode -ne 0) {
    Write-Host "Error linking executable" -ForegroundColor Red
    Write-Host ""
    Write-Host "Make sure you have the required libraries installed:" -ForegroundColor Yellow
    Write-Host "- OpenGL development libraries (opengl32, glu32)" -ForegroundColor Yellow
    Write-Host ""
    Write-Host "These should be available with MinGW/MSYS2." -ForegroundColor Yellow
    exit 1
}

Write-Host "Build successful! chaosforge.exe created." -ForegroundColor Green
Write-Host ""
Write-Host "To run the game, execute: .\chaosforge.exe" -ForegroundColor Cyan
