# Coreria

A Rust-based project leveraging OpenGL for graphics rendering capabilities.

## Description

Coreria is an early-stage Rust application that provides a foundation for OpenGL-based graphics programming. Built with performance and safety in mind, it utilizes Rust's powerful type system and memory safety guarantees to create reliable graphics applications.

## Features

- **OpenGL Integration**: Built-in OpenGL bindings via the `gl` crate for graphics rendering
- **Rust Safety**: Leverages Rust's memory safety and concurrency features
- **Cross-platform**: Designed to work across multiple operating systems
- **Modern Architecture**: Uses Rust 2021 edition for the latest language features

## Installation

### Prerequisites

Before installing Coreria, ensure you have the following installed:

- **Rust**: Version 1.56.0 or later (Rust 2021 edition)
  - On Arch Linux: `sudo pacman -S rust`
  - On Windows: Download from [rustup.rs](https://rustup.rs/) or use `winget install Rustlang.Rustup`
  - On other systems: Visit [rustup.rs](https://rustup.rs/)

- **OpenGL Development Libraries**:
  - On Arch Linux: `sudo pacman -S mesa libgl`
  - On Windows: Usually included with graphics drivers

### Building from Source

1. Clone the repository:
```bash
git clone https://github.com/CGXDevTeam/Coreria.git
cd Coreria
```

2. Build the project:
```bash
cargo build --release
```

3. Run the application:
```bash
cargo run --release
```

## Usage

After building the project, you can run Coreria directly:

```bash
# Run in debug mode
cargo run

# Run in release mode (optimized)
cargo run --release
```

### Basic Example

Currently, Coreria outputs a simple greeting message. As the project develops, more features and examples will be added.

```bash
$ cargo run
   Compiling coreria v0.1.0
    Finished dev [unoptimized + debuginfo] target(s)
     Running `target/debug/coreria`
Hello, Coreria!
```

## Contributing

We welcome contributions to Coreria! To contribute:

1. **Fork the Repository**: Click the "Fork" button on the GitHub repository page
2. **Create a Feature Branch**: 
   ```bash
   git checkout -b feature/your-feature-name
   ```
3. **Make Your Changes**: Implement your feature or fix
4. **Follow Code Standards**: 
   - Run `cargo fmt` to format your code
   - Run `cargo clippy` to check for common mistakes
   - Ensure all tests pass with `cargo test`
5. **Commit Your Changes**:
   ```bash
   git commit -m "Add your descriptive commit message"
   ```
6. **Push to Your Fork**:
   ```bash
   git push origin feature/your-feature-name
   ```
7. **Submit a Pull Request**: Open a PR against the main repository

### Development Workflow

This project uses the `.specify` workflow system for feature planning and development. Key conventions include:

- Features are developed in numbered branches (e.g., `001-feature-name`)
- Feature specifications are maintained in the `specs/` directory
- Follow the constitution principles defined in `.specify/memory/constitution.md`

### Code Guidelines

- Write idiomatic Rust code
- Include tests for new functionality
- Update documentation for API changes
- Keep commits focused and atomic

## License

This project's license information will be added soon. Please check back later or contact the maintainers for licensing details.

## Contact Information

- **Repository**: [https://github.com/CGXDevTeam/Coreria](https://github.com/CGXDevTeam/Coreria)
- **Issues**: Report bugs or request features at [GitHub Issues](https://github.com/CGXDevTeam/Coreria/issues)
- **Team**: CGXDevTeam

For questions, suggestions, or discussions about the project, please open an issue on GitHub or reach out to the development team through the repository.

---

**Note**: Coreria is in active development. Features and documentation are subject to change as the project evolves.
