@echo off
echo Building ChaosForge Arena...

REM Clean previous build
if exist chaosforge.exe del chaosforge.exe
if exist *.o del *.o

REM Compile object files
echo Compiling main.c...
gcc -Wall -c main.c -o main.o
if errorlevel 1 (
    echo Error compiling main.c
    pause
    exit /b 1
)

echo Compiling player_controller.c...
gcc -Wall -c player_controller.c -o player_controller.o
if errorlevel 1 (
    echo Error compiling player_controller.c
    pause
    exit /b 1
)

echo Compiling combat_system.c...
gcc -Wall -c combat_system.c -o combat_system.o
if errorlevel 1 (
    echo Error compiling combat_system.c
    pause
    exit /b 1
)

echo Compiling physics_manager.c...
gcc -Wall -c physics_manager.c -o physics_manager.o
if errorlevel 1 (
    echo Error compiling physics_manager.c
    pause
    exit /b 1
)

echo Compiling game_state.c...
gcc -Wall -c game_state.c -o game_state.o
if errorlevel 1 (
    echo Error compiling game_state.c
    pause
    exit /b 1
)

REM Link executable
echo Linking chaosforge.exe...
gcc -o chaosforge.exe main.o player_controller.o combat_system.o physics_manager.o game_state.o -lopengl32 -lglu32 -lgdi32 -luser32 -lkernel32 -lm
if errorlevel 1 (
    echo Error linking executable
    echo.
    echo Make sure you have the required libraries installed:
    echo - OpenGL development libraries (opengl32, glu32)
    echo.
    echo These should be available with MinGW/MSYS2.
    pause
    exit /b 1
)

echo Build successful! chaosforge.exe created.
echo.
echo To run the game, execute: chaosforge.exe
pause
