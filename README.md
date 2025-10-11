<div align="center">
  <h1>🎮 Coreria Game Engine</h1>
  <h3><em>A custom game engine powering physics-based combat experiences</em></h3>
  
  <p>
    <img src="https://img.shields.io/badge/Engine-Rust%20%2B%20C-orange?style=flat-square" alt="Engine">
    <img src="https://img.shields.io/badge/Graphics-OpenGL-blue?style=flat-square" alt="Graphics">
    <img src="https://img.shields.io/badge/Physics-ODE-green?style=flat-square" alt="Physics">
    <img src="https://img.shields.io/badge/Platform-Windows%20%7C%20Linux-lightgrey?style=flat-square" alt="Platform">
  </p>
</div>

---

## 🌟 Overview

**Coreria** is a custom game engine designed for creating physics-intensive, real-time combat games. Built with a hybrid Rust/C architecture, it combines modern systems programming with battle-tested game development libraries to deliver high-performance gaming experiences.

### 🎯 Key Features

- **🔥 Real-Time Physics**: Powered by ODE (Open Dynamics Engine) for authentic momentum and collision systems
- **⚡ High Performance**: Rust core with C game logic for optimal speed and memory safety
- **🎨 Modern Graphics**: OpenGL rendering with Bevy integration for advanced visual effects
- **🥊 Combat-Focused**: Specialized systems for fighting games with ragdoll physics and destructible environments
- **🛠️ Spec-Driven Development**: Integrated development methodology with ChronoShift toolkit

---

## 🎮 ChaosForge: The Flagship Game

**ChaosForge** is a multiplayer arena deathmatch game that showcases the full power of the Coreria engine. Battle in a destructible 3D coliseum where physics and skill determine victory.

### ⚔️ Game Features

- **4-8 Player Combat**: Intense multiplayer battles with no login required
- **4 Unique Fighting Styles**:
  - 🥊 **Brawler**: Close-quarters powerhouse with devastating combos
  - ⚡ **Striker**: Lightning-fast attacks and mobility
  - 👻 **Phantom**: Elusive fighter with deceptive moves
  - 🛡️ **Titan**: Heavy defender with crushing ultimates
- **Dynamic Arena**: Shrinking boundaries force confrontation
- **True Physics**: Momentum-based combat with ragdoll deaths
- **Destructible Environment**: Break the arena as you fight

### 🎮 Controls

```
Movement: WASD
Sprint: Shift (50% speed boost, stamina-based)
Block: Ctrl (75% damage reduction)
Combat: Mouse buttons for combos and special moves
```

---

## ⚙️ Coreria Engine Architecture

The Coreria engine uses a hybrid architecture optimized for performance and maintainability:

```
┌─────────────────┐    ┌─────────────────┐
│   Rust Core     │    │   C Game Logic  │
│                 │    │                 │
│ • Memory Safety │◄──►│ • Game Systems  │
│ • Modern Tools  │    │ • Performance   │
│ • Bevy ECS      │    │ • OpenGL/GLFW   │
└─────────────────┘    └─────────────────┘
         │                       │
         └───────────┬───────────┘
                     ▼
         ┌─────────────────────┐
         │   Shared Libraries  │
         │                     │
         │ • OpenGL Rendering  │
         │ • ODE Physics       │
         │ • Audio Systems     │
         └─────────────────────┘
```

### 🔧 Technical Stack

- **Core Engine**: Rust with Bevy ECS and OpenGL bindings
- **Game Layer**: C with GLFW for windowing and input
- **Physics**: Open Dynamics Engine (ODE) for realistic simulations
- **Graphics**: OpenGL 3.3+ with modern shader pipeline
- **Build System**: Cargo for Rust, Make for C components

---

## 🛠️ ChronoShift: Spec-Driven Development

ChronoShift is our integrated development methodology that transforms how games are built. Instead of code-first development, specifications drive implementation.

### ✨ Spec-Kit Features

- **📋 Executable Specifications**: Specs that generate working code
- **🔄 Iterative Development**: Rapid prototyping and refinement
- **📚 Automatic Documentation**: Always up-to-date project docs
- **🎯 Feature Planning**: Structured approach to game development
- **🤖 AI-Assisted**: Works seamlessly with modern coding assistants

```bash
# Initialize a new game project
uvx --from git+https://github.com/github/spec-kit.git specify init my-game

# Generate specifications
specify create-spec "multiplayer combat system"

# Plan implementation
specify plan

# Generate development tasks
specify tasks
```

---

## 🚀 Quick Start

### Prerequisites

- **Rust** (latest stable) - [Install Rust](https://rustup.rs/)
- **C Compiler** (GCC/Clang)
- **OpenGL Development Libraries**
- **ODE Physics Library**
- **Python 3.11+** (for ChronoShift tools)

### 🎮 Play ChaosForge

```bash
# Clone the repository
git clone https://github.com/CGXDevTeam/Coreria.git
cd Coreria

# Build and run ChaosForge
cd chaosforge-game
make
./chaosforge
```

### ⚙️ Build Coreria Engine

```bash
# Build the Rust core
cargo build --release

# The engine will be available as a library for C games
```

### 🛠️ Install ChronoShift Tools

```bash
# Install the spec-driven development toolkit
cd ChronoShift/spec-kit
pip install -e .

# Verify installation
specify --help
```

---

## 📁 Project Structure

```
Coreria/
├── 🦀 src/                    # Rust engine core
│   └── main.rs               # Engine entry point
├── 🎮 chaosforge-game/        # ChaosForge game implementation
│   ├── main.c                # Game entry point
│   ├── combat_system.c       # Fighting mechanics
│   ├── physics_manager.c     # Physics integration
│   └── Makefile             # Build configuration
├── 🛠️ ChronoShift/           # Development methodology
│   └── spec-kit/            # Spec-driven development tools
├── 📋 specs/                 # Feature specifications
│   └── 003-build-coreria-a/  # Current development spec
├── ⚙️ Cargo.toml             # Rust dependencies
└── 📖 README.md              # This file
```

---

## 🔨 Development

### Building Components

**Rust Engine:**
```bash
cargo build          # Debug build
cargo build --release # Optimized build
cargo test           # Run tests
```

**ChaosForge Game:**
```bash
cd chaosforge-game
make                 # Build game
make clean          # Clean build artifacts
```

**ChronoShift Tools:**
```bash
cd ChronoShift/spec-kit
pip install -e .    # Editable install
python -m pytest    # Run tests
```

### 🧪 Testing

```bash
# Test the complete system
cargo test                    # Engine tests
cd chaosforge-game && make test  # Game tests (if available)
cd ChronoShift/spec-kit && python -m pytest  # Tool tests
```

---

## 🤝 Contributing

We welcome contributions! Whether you're interested in:

- 🎮 **Game Development**: Enhance ChaosForge or create new games
- ⚙️ **Engine Development**: Improve the Rust/C core systems  
- 🛠️ **Tooling**: Extend the ChronoShift development methodology
- 📚 **Documentation**: Help others understand and use Coreria

Please see our [Contributing Guide](CONTRIBUTING.md) for details.

---

## 📄 License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.

---

<div align="center">
  <p><strong>Built with ❤️ by the Coreria Team</strong></p>
  <p>
    <a href="#-overview">Overview</a> •
    <a href="#-chaosforge-the-flagship-game">ChaosForge</a> •
    <a href="#️-coreria-engine-architecture">Engine</a> •
    <a href="#️-chronoshift-spec-driven-development">ChronoShift</a> •
    <a href="#-quick-start">Quick Start</a>
  </p>
</div>
