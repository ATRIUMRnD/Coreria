@echo off
echo Building ChaosForge Arena - New Engine...

echo Compiling new_main.c...
gcc -c new_main.c -o new_main.o -I. -Wall

if %errorlevel% neq 0 (
    echo Error compiling new_main.c
    pause
    exit /b 1
)

echo Linking chaosforge_new.exe...
gcc new_main.o -o chaosforge_new.exe -lopengl32 -lglu32 -lgdi32 -luser32 -lkernel32

if %errorlevel% neq 0 (
    echo Error linking chaosforge_new.exe
    pause
    exit /b 1
)

echo Build successful! Run chaosforge_new.exe
echo.
echo Controls:
echo - WASD: Fly camera movement
echo - QE: Up/Down movement  
echo - Right Mouse: Look around
echo - Shift: Speed boost, Ctrl: Slow mode
echo - Mouse Wheel: Adjust speed
echo - UP/DOWN: Menu navigation
echo - ENTER: Start game
echo - ESC: Toggle menu
echo - F12: Refresh
echo.
pause
