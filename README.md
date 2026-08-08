# Notelify

> A simple, lightweight, and cross-platform CLI note-taking app written in C++17.

**Notelify** lets you quickly create, read, edit, append to, list, and delete plain-text notes directly from your terminal. It automatically handles cross-platform directory paths and includes built-in filename validation to keep your notes organized and secure.

**Check the latest release [HERE](https://github.com/emptytheblackcat/Notelify/releases)**

---

## Features

- **Directory Handling:** Saves notes safely in your home directory (`~/.notelify` on Linux/macOS or `%USERPROFILE%\.notelify` on Windows).
- **Strict Filename Validation:** Rejects invalid OS characters, relative path references (`.` and `..`), leading/trailing spaces, trailing dots, and invalid Windows filenames.
- **Multi-Line Input & Control:** Type notes freely and save using the `:save` keyword on a new line, or safely abort anytime with `:cancel`.
- **Stream Termination Guard:** Handles EOF (`Ctrl+D` / `Ctrl+Z`) gracefully to avoid saving incomplete content.
- **Zero External Dependencies:** Built using standard modern C++ (`std::filesystem`, `fstream`, `cstdlib`).

---

## Usage

```text
  notelify new "TITLE"      Create a new note
  notelify edit "TITLE"     Modify/overwrite an existing note
  notelify append "TITLE"   Append new lines to an existing note
  notelify read "TITLE"     Read a note
  notelify list             List all saved notes
  notelify delete "TITLE"   Delete a note (with confirmation)
  notelify help             Show usage instructions
```

---

## Getting Started

**Pre-compiled Binaries:**
- **Download the latest release from the Releases page:**
  - notelify-windows.zip (Windows x86_64)
  - notelify-linux.zip (Linux x86_64)
  - notelify-macos.zip (macOS x86_64 / ARM64)

**Building from Source:**

 - **Prerequisites:** A C++17 compatible compiler (g++, clang++, or cl).

 - **Compilation:**
   - Clone the repository and compile using C++17:

```bash
git clone https://github.com/emptytheblackcat/notelify.git
cd notelify

# Compile with C++17 standard flag
g++ -std=c++17 main.cpp -o notelify
```

---

## Important Installation Notice: Adding to System PATH
To run Notelify from any folder in your terminal without typing `./notelify` or navigating to its folder every time, you need to manually add the executable to your system's PATH variable:

Windows (PowerShell / CMD)
 * Extract `notelify-windows.zip`.
 * Move `notelify.exe` to a permanent folder (e.g., `C:\Tools\`).
 * Search "Edit the system environment variables" in the Windows Start menu.
 * Click Environment Variables... at the bottom right.
 * Under User variables, select Path and click Edit... -> New.
 * Paste `C:\Tools\` (or the folder path where `notelify.exe` lives) and click OK.
 * Restart your terminal!
 
Linux and macOS (Bash / Zsh)
 * Extract `notelify-linux.zip` (or `notelify-macos.zip`).
 * Open your terminal in the directory where the binary lives.
 * Make the binary executable and move it to `/usr/local/bin`:
```bash
chmod +x notelify
sudo mv notelify /usr/local/bin/notelify
```

Verify your installation by running:

```bash
notelify help
```

---

 ## Known Issues

There are currently no known issues.

> Found a bug not listed here? Please [Open an Issue](https://github.com/emptytheblackcat/Notelify/issues)!
