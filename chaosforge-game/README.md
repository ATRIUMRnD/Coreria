# ChaosForge Arena

ChaosForge Arena is a multiplayer physics-based combat game built on the Coreria engine. It features four distinct fighting styles and real-time combat simulation.

## 🎮 Game Features

- **Four Fighting Styles**: Brawler, Striker, Phantom, and Titan
- **Physics-Based Combat**: Realistic combat mechanics with ODE physics simulation
- **Real-Time Arena Combat**: Fast-paced multiplayer battles
- **Training Mode**: Practice against AI masters before entering the arena
- **Visual Effects**: Particle systems for attacks and respawns
- **Console System**: In-game debug console (F12 to toggle)

## 🛠️ Building the Game

### Windows (MinGW/MSYS2)

The game has been simplified to work with basic OpenGL libraries available on Windows:

#### Option 1: PowerShell Build Script
```powershell
.\build.ps1
```

#### Option 2: Batch File
```cmd
build.bat
```

#### Option 3: Make (if available)
```bash
make
```

### Required Libraries

The game now only requires basic OpenGL libraries that come with MinGW/MSYS2:
- `opengl32` - OpenGL library
- `glu32` - OpenGL Utility library
- `gdi32` - Windows GDI library
- `user32` - Windows User library
- `kernel32` - Windows Kernel library

## 🎯 Controls

### Menu Navigation
- **↑/↓ Arrow Keys**: Navigate fighting styles
- **Enter**: Select style and start game
- **C**: Copy selected style to clipboard
- **F12**: Toggle debug console

### In-Game Controls
- **Arrow Keys**: Camera controls
- **Mouse**: Click and drag to orbit camera
- **Mouse Wheel**: Zoom in/out
- **Space**: Spawn dynamic object (in training)
- **F12**: Toggle debug console

## 🚀 Running the Game

After building, run the executable:

```cmd
.\chaosforge.exe
```

The game will start with a style selection menu. Choose your fighting style and press Enter to begin training mode.

## 🏗️ Architecture

The game is structured with modular components:

- **main.c**: Core game loop, rendering, and Windows integration
- **player_controller.c**: Player spawning and style management
- **combat_system.c**: Combat mechanics and damage simulation
- **physics_manager.c**: Physics simulation and arena management
- **game_state.c**: Game state management and updates

## 🔧 Development Notes

This version has been simplified to work without external dependencies like GLFW or GLUT. It uses:
- Native Windows API for window management
- Basic OpenGL for rendering
- GLU for 3D utilities
- Manual text rendering (simplified)

For a full-featured version with proper text rendering and more advanced graphics, you would need to install additional libraries like GLFW3 and FreeType.
