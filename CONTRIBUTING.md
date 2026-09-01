# Contributing to Coreria Game Engine

Thank you for your interest in contributing to Coreria! This document provides guidelines and instructions for contributing to the project.

## 🚀 Getting Started

### Prerequisites

- **Rust**: Install the latest stable version from [rustup.rs](https://rustup.rs/)
- **C Compiler**: GCC on Linux/macOS or MSVC on Windows  
- **Git**: For version control
- **Optional**: ODE (Open Dynamics Engine) for physics features

### Development Environment Setup

1. **Clone the repository**:
   ```bash
   git clone https://github.com/CGXDevTeam/Coreria.git
   cd Coreria
   ```

2. **Build the project**:
   ```bash
   cargo build
   ```

3. **Run tests**:
   ```bash
   cargo test
   ```

4. **Run the engine**:
   ```bash
   cargo run
   ```

5. **Run ChaosForge game**:
   ```bash
   cargo run --features multiplayer chaosforge
   ```

## 🏗️ Project Structure

```
Coreria/
├── src/                    # Core Rust engine
│   ├── main.rs            # Engine entry point
│   ├── engine/            # Core engine systems  
│   ├── graphics/          # OpenGL rendering
│   ├── physics/           # Physics simulation
│   ├── input/             # Input handling
│   └── ffi/               # C/Rust FFI bridge
├── chaosforge-multiplayer/ # Multiplayer game crate
├── chaosforge-game/       # C game implementation
├── specs/                 # Feature specifications
├── ChronoShift/           # SDD methodology tools
└── docs/                  # Documentation
```

## 📋 Contribution Guidelines

### Code Style

**Rust Code**:
- Follow the [Rust Style Guide](https://doc.rust-lang.org/1.0.0/style/README.html)
- Use `cargo fmt` to format code
- Run `cargo clippy` to catch common mistakes
- Add documentation comments (`///`) for public APIs

**C Code**:
- Use K&R style indentation (4 spaces)
- Follow consistent naming conventions
- Add header comments for functions
- Keep functions focused and well-documented

### Commit Messages

Follow conventional commits format:
```
type(scope): description

[optional body]

[optional footer]
```

Types: `feat`, `fix`, `docs`, `style`, `refactor`, `test`, `chore`

Examples:
```
feat(physics): add ODE integration for ragdoll physics
fix(graphics): resolve texture loading crash on Windows
docs(readme): update build instructions for Linux
```

### Pull Request Process

1. **Fork** the repository
2. **Create** a feature branch: `git checkout -b feature/your-feature`
3. **Make** your changes following the style guidelines
4. **Write** tests for new functionality
5. **Ensure** all tests pass: `cargo test`
6. **Commit** your changes with clear messages
7. **Push** to your fork: `git push origin feature/your-feature`
8. **Create** a pull request with:
   - Clear description of changes
   - Link to related issues
   - Screenshots/videos if UI changes

## 🧪 Testing

### Running Tests

```bash
# Run all tests
cargo test

# Run tests for specific crate
cargo test -p chaosforge-multiplayer

# Run with output
cargo test -- --nocapture

# Run specific test
cargo test test_physics_integration
```

### Writing Tests

- Add unit tests in the same file as the code
- Add integration tests in `tests/` directory
- Use descriptive test names
- Test both success and failure cases

Example:
```rust
#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn test_engine_initialization() {
        let engine = CoreiaEngine::new();
        assert_eq!(engine.state(), EngineState::Initializing);
    }
}
```

## 🐛 Reporting Issues

When reporting bugs, please include:

- **Environment**: OS, Rust version, graphics card
- **Steps to reproduce** the issue
- **Expected behavior** vs **actual behavior**  
- **Error messages** or logs
- **Minimal code example** if applicable

Use the issue templates when available.

## 💡 Feature Requests

For new features:

1. **Check** existing issues to avoid duplicates
2. **Describe** the problem the feature would solve
3. **Propose** a solution or API design
4. **Consider** implementation complexity and breaking changes

## 📖 Documentation

Help improve documentation by:

- Fixing typos and grammar
- Adding examples to API docs
- Writing tutorials and guides
- Updating README files
- Adding inline code comments

## 🌟 Areas for Contribution

### High Priority
- **ODE Physics Integration**: Complete the physics engine bindings
- **Audio System**: Implement 3D spatial audio
- **Networking**: Enhance multiplayer capabilities
- **Cross-platform**: Linux and macOS support

### Medium Priority  
- **Asset Pipeline**: Model/texture loading improvements
- **Performance**: Optimization and profiling tools
- **Debug Tools**: Visual debugging and profiling
- **Documentation**: API docs and tutorials

### Good First Issues
Look for issues labeled `good-first-issue` or `help-wanted`.

## 🤝 Community

- **Discussions**: Use GitHub Discussions for questions
- **Issues**: Report bugs and request features
- **Pull Requests**: Contribute code changes
- **Code of Conduct**: Be respectful and inclusive

## 📄 License

By contributing to Coreria, you agree that your contributions will be licensed under the MIT License.

## 🏷️ Release Process

1. Maintainers review and merge PRs
2. Version numbers follow [SemVer](https://semver.org/)
3. Releases are tagged and published automatically
4. Changelog is updated for each release

---

Thank you for contributing to Coreria! Your efforts help make this engine better for everyone. 🎮✨