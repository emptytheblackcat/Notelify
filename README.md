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
