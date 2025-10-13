@echo off
REM ChaosForge Arena Windows Build Script

echo Building ChaosForge Arena (Windows version)...

REM Clean previous build
if exist "chaosforge_v2.exe" del "chaosforge_v2.exe"
del *.o 2>nul

REM Compile object files
echo Compiling main_windows.c...
gcc -Wall -c main_windows.c -o main_windows.o
if errorlevel 1 (
    echo Error compiling main_windows.c
    exit /b 1
)

echo Compiling player_controller.c...
gcc -Wall -c player_controller.c -o player_controller.o
if errorlevel 1 (
    echo Error compiling player_controller.c
    exit /b 1
)

echo Compiling combat_system.c...
gcc -Wall -c combat_system.c -o combat_system.o
if errorlevel 1 (
    echo Error compiling combat_system.c
    exit /b 1
)

echo Compiling physics_manager.c...
gcc -Wall -c physics_manager.c -o physics_manager.o
if errorlevel 1 (
    echo Error compiling physics_manager.c
    exit /b 1
)

echo Compiling game_state.c...
gcc -Wall -c game_state.c -o game_state.o
if errorlevel 1 (
    echo Error compiling game_state.c
    exit /b 1
)

REM Link executable
echo Linking chaosforge_win.exe...
gcc -o chaosforge_v2.exe main_windows.o player_controller.o combat_system.o physics_manager.o game_state.o -lopengl32 -lglu32 -lgdi32 -luser32 -lkernel32 -lm

if errorlevel 1 (
    echo Error linking executable
    echo.
    echo Make sure you have the required libraries installed:
    echo - OpenGL development libraries (opengl32, glu32)
    echo.
    echo These should be available with MinGW/MSYS2.
    exit /b 1
)

echo Build successful! chaosforge_v2.exe created.
echo.
echo To run the game, execute: chaosforge_v2.exe