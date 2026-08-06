# Notelify

> A simple, lightweight, and cross-platform CLI note-taking app written in C++17.

**Notelify** lets you quickly create, read, edit, list, and delete plain-text notes directly from your terminal. It automatically handles cross-platform directory paths and cleans up note titles so you never have to deal with broken file paths.

---

## Features

- **Directory Handling:** Saves notes safely in your home directory (`~/.notelify` on Linux/macOS or `%USERPROFILE%\.notelify` on Windows).
- **Filename Sanitization:** Converts titles to lowercase, replaces spaces with dashes (`-`), and strips out invalid OS characters.
- **Multi-Line Input:** Type notes freely and save using the `:save` sentinel on a new line.
- **Zero External Dependencies:** Built using standard modern C++ (`std::filesystem`, `fstream`, `cstdlib`).
- **Overwrite Protection:** Prevents accidentally overwriting existing notes when creating new ones.

---

## Getting Started

> Download the latest release at the GitHub repo or compile it manually:

### Prerequisites
- A C++17 compatible compiler (`g++`, `clang++`, or MSVC).

### Building

Clone the repository and compile `main.cpp`:

```bash
git clone https://github.com/emptytheblackcat/notelify.git
cd notelify

# Compile with C++17 standard flag
g++ -std=c++17 main.cpp -o notelify
```

## Important Installation Notice: Adding to System PATH

To run Notelify from any folder in your terminal without typing `./notelify` or navigating to its folder every time, you need to manually add the executable to your system's PATH variable:

Windows (PowerShell / CMD)
 * Move `notelify-windows.exe` to a permanent folder (e.g., `C:\Tools\` and rename it to `notelify.exe`).
 * Search "Edit the system environment variables" in the Windows Start menu.
 * Click Environment Variables... at the bottom right.
 * Under User variables, select Path and click Edit... -> New.
 * Paste `C:\Tools\` (or the folder path where `notelify.exe` lives) and click OK.
 * Restart your terminal!
 
Linux and macOS (Bash / Zsh)
 * Open your terminal in the directory where you downloaded the binary.
 * Make the binary executable and move it to `/usr/local/bin`:
```bash
chmod +x notelify-linux # or notelify-mac
sudo mv notelify-linux /usr/local/bin/notelify
```

 You're done! Run notelify from any directory in your terminal.
