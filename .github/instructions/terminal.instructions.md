---
applyTo: '**'
---

# Project Context and Coding Guidelines

## Terminal Environment
- **Primary OS**: Arch Linux
- **Package Manager**: pacman
- **Shell**: bash/zsh
- **Terminal Commands**: Use Arch Linux specific commands and paths

**Note for Windows contributors**: This repository is primarily documented for Arch Linux, however many contributors work on Windows. When providing commands for Windows users prefer PowerShell equivalents and use `choco`/`winget` when suggesting Windows package manager examples only if necessary. When running commands in CI or cross-platform scripts, prefer platform checks and provide both variants.

## System Guidelines
When providing terminal commands or system-level instructions:
- Use `pacman` for package management (e.g., `sudo pacman -S package-name`)
- Reference Arch Linux file paths and conventions
- Assume systemd for service management
- Use Arch Linux documentation and best practices

When providing Windows-specific guidance:
- Use `powershell.exe` semantics for inline command examples and `;` when joining commands on a single line.
- For package installation examples on Windows, prefer `winget` or `choco` with explicit notes (e.g., `winget install <package>`).
- Reference common Windows paths (e.g., `C:\Users\<name>\` or `D:\workspace`) and note differences from Unix-like paths.
- Avoid assuming systemd; prefer Windows Service Manager / `sc` or guidance that applies to both.
- If a command differs significantly between OSes, include both variants or show a small platform-detection snippet.

Example cross-platform pattern in documentation:

```powershell
# Windows / PowerShell example
if ($IsWindows) { 
	Write-Host "Running Windows-specific setup"
	winget install --id Git.Git -e --source winget
} else {
	echo "Assuming Arch Linux: sudo pacman -S git"
}
```

## Coding Guidelines
Provide project context and coding guidelines that AI should follow when generating code, answering questions, or reviewing changes, with Arch Linux as the primary development environment.
