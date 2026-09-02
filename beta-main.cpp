#include <iostream>
#include <string>
#include <fstream>
#include <filesystem>
#include <cstdlib>
#include <algorithm>
#include <cctype>
#include <vector>
#include <sstream>

namespace fs = std::filesystem;
using namespace std;

// Helper to check for Windows reserved names (CON, PRN, AUX, NUL, COM1-9, LPT1-9)
bool isWindowsReservedName(const string& filename) {
    string upperStem = "";
    for (char c : filename) {
        if (c == '.') break; // Windows checks the stem before extension (e.g. CON.txt is reserved)
        upperStem += static_cast<char>(toupper(static_cast<unsigned char>(c)));
    }
    static const vector<string> reserved = {
        "CON", "PRN", "AUX", "NUL",
        "COM1", "COM2", "COM3", "COM4", "COM5", "COM6", "COM7", "COM8", "COM9",
        "LPT1", "LPT2", "LPT3", "LPT4", "LPT5", "LPT6", "LPT7", "LPT8", "LPT9"
    };
    for (const auto& r : reserved) {
        if (upperStem == r) return true;
    }
    return false;
}

// Checker to make sure filename is valid
bool filenameChecker(const string& filename){
    if (filename.empty()) return false;
    if (filename[0] == ' ' || filename.back() == ' ') return false;
    if (filename.back() == '.') return false;
    if (filename == "." || filename == "..") return false;
    if (isWindowsReservedName(filename)) return false;

    for (char c : filename){
        if (!isalnum(static_cast<unsigned char>(c)) && c != ' ' && c != '-' && c != '_' && c != '.') return false;
    }
    return true;
}

// Get user's home directory
fs::path getNotesDirectory() {
    const char* userProfile = std::getenv("USERPROFILE"); // Windows
    const char* homeDir = std::getenv("HOME");           // Linux / macOS
    
    if (userProfile) {
        return fs::path(userProfile) / ".notelify";
    } else if (homeDir) {
        return fs::path(homeDir) / ".notelify";
    }
    return fs::path("notes"); // Fallback to local directory
}

// Initialize notes directory if it doesn't exist
bool initDirectory(const fs::path& notesDir) {
    try {
        if (!fs::exists(notesDir)) {
            if (!fs::create_directories(notesDir)) {
                cout << "[ERROR] Could not read notes directory: " << notesDir.string() << "\n";
                return false;
            }
        }
        return true;
    } catch (const fs::filesystem_error&) {
        cout << "[ERROR] Could not read notes directory: " << notesDir.string() << "\n";
        return false;
    }
}

struct InputResult {
    string content;
    bool cancelled;
};
// Helper to read multiple lines
InputResult getMultiLineInput() {
    cout << "Enter content below (type ':save' on a new line, or ':cancel' to abort):\n";
    cout << "--------------------------------------------------------\n";
    string fullText = "";
    string line;
    while (getline(cin, line)) {
        if (line == ":save") {
            return {fullText, false};
        }
        if (line == ":cancel") {
            cout << "[CANCELLED] Operation aborted.\n";
            return {"", true};
        }
        fullText += line + "\n";
    }
    
    // If getline loop breaks because cin hit EOF (Ctrl+D / Ctrl+Z)
    cin.clear(); 
    cout << "\n[CANCELLED] Stream closed (EOF). Operation aborted.\n";
    return {"", true};
}

// Command: notelify new "TITLE"
void createNote(const fs::path& notesDir, const string& title) {
    if (!filenameChecker(title)){
        cout << "[ERROR] Invalid filename!\n";
        return;
    }
    
    fs::path filePath = notesDir / (title + ".txt");
    fs::path tmpPath = notesDir / (title + ".tmp");

    try {
        if (fs::exists(filePath)) {
            cout << "[WARNING] Note '" << title << "' already exists! Use 'notelify edit \"" << title << "\"' instead.\n";
            return;
        }

        InputResult res = getMultiLineInput();
        if (res.cancelled) return; // Aborted! Nothing created.

        // Write content to a temporary file first
        ofstream writeFile(tmpPath);
        if (!writeFile.is_open()) {
            cout << "[ERROR] Could not create temp file! Aborting without saving.\n";
            return;
        }

        writeFile << res.content;
        writeFile.close();

        // Atomically move temp file to target note path
        fs::rename(tmpPath, filePath);
        cout << "[SUCCESS] Note saved to " << filePath.string() << "\n";

    } catch (const std::exception&) {
        if (fs::exists(tmpPath)) fs::remove(tmpPath);
        cout << "[ERROR] Unexpected filesystem error! Aborting creation.\n";
    }
}

// Command: notelify edit "TITLE"
void editNote(const fs::path& notesDir, const string& title) {
    if (!filenameChecker(title)) {
        cout << "[ERROR] Invalid filename!\n";
        return;
    }
    
    fs::path filePath = notesDir / (title + ".txt");
    fs::path tmpPath = notesDir / (title + ".tmp");

    try {
        if (!fs::exists(filePath)) {
            cout << "[ERROR] Note '" << title << "' does not exist!\n";
            return;
        }

        // Print existing content safely
        ifstream readFile(filePath);
        if (!readFile.is_open()) {
            cout << "[ERROR] Could not open note for reading! Aborting.\n";
            return;
        }

        cout << "\n--- Current Content of '" << title << "' ---\n";
        string line;
        while (getline(readFile, line)) {
            cout << line << "\n";
        }
        readFile.close();

        // Get new input
        cout << "\n--- Enter New Content ---\n";
        InputResult res = getMultiLineInput();
        if (res.cancelled) {
            return; // Aborted! Original file is untouched.
        }

        // Write to temp file FIRST so original note is never touched if this fails
        ofstream writeFile(tmpPath);
        if (!writeFile.is_open()) {
            cout << "[ERROR] Could not open temp file! Aborting without saving.\n";
            return;
        }

        writeFile << res.content;
        writeFile.close();

        // Atomic swap - replaces original ONLY if temp write was 100% successful
        fs::rename(tmpPath, filePath);
        cout << "[SUCCESS] Note '" << title << "' updated.\n";

    } catch (const std::exception& e) {
        // Clean up temp file if it exists, leaving original untouched
        if (fs::exists(tmpPath)) fs::remove(tmpPath);
        cout << "[ERROR] Unexpected filesystem error! Aborting.\n";
    }
}

// Command: notelify read "TITLE"
void readNote(const fs::path& notesDir, const string& title) {
    if (!filenameChecker(title)){
        cout << "[ERROR] Invalid filename!\n";
        return;
    }
    fs::path filePath = notesDir / (title + ".txt");
    if (!fs::exists(filePath)) {
        cout << "[ERROR] Note '" << title << "' does not exist!\n";
        return;
    }
    ifstream file(filePath);
    string line;
    cout << "\n--- Current Content of '" << title << "' ---\n";
    while (getline(file, line)) {
        cout << line << "\n";
    }
    cout << "\n";
}

// Command: notelify append "TITLE"
void appendNote(const fs::path& notesDir, const string& title) {
    if (!filenameChecker(title)){
        cout << "[ERROR] Invalid filename!\n";
        return;
    }

    fs::path filePath = notesDir / (title + ".txt");
    fs::path tmpPath = notesDir / (title + ".tmp");

    try {
        if (!fs::exists(filePath)) {
            cout << "[ERROR] Note '" << title << "' does not exist!\n";
            return;
        }

        readNote(notesDir, title);

        cout << "\n--- Enter text to append ---\n";
        InputResult res = getMultiLineInput();
        if (res.cancelled) return; // Aborted! Original file untouched.

        if (res.content.empty()){
            cout << "[ERROR] Cannot append empty content!\n";
            return;
        }

        // Copy existing content into temp file first
        fs::copy_file(filePath, tmpPath, fs::copy_options::overwrite_existing);

        // Append new content to the temp file
        ofstream writeFile(tmpPath, std::ios::app);
        if (!writeFile.is_open()) {
            cout << "[ERROR] Could not append to temp file! Aborting.\n";
            if (fs::exists(tmpPath)) fs::remove(tmpPath);
            return;
        }

        writeFile << res.content;
        writeFile.close();

        // Atomically replace original with updated temp file
        fs::rename(tmpPath, filePath);
        cout << "[SUCCESS] Content appended to '" << title << "'.\n";

    } catch (const std::exception&) {
        if (fs::exists(tmpPath)) fs::remove(tmpPath);
        cout << "[ERROR] Unexpected filesystem error! Aborting append.\n";
    }
}

// Command: notelify list
void listNotes(const fs::path& notesDir) {
    cout << "\n--- Your Notes ---\n";
    bool found = false;
    try {
        for (const auto& entry : fs::directory_iterator(notesDir)) {
            if (entry.is_regular_file() && entry.path().extension() == ".txt") {
                cout << "- " << entry.path().stem().string() << "\n";
                found = true;
            }
        }
        if (!found) {
            cout << "(No notes found)\n";
        }
    } catch (const fs::filesystem_error&) {
        cout << "[ERROR] Could not read notes directory: " << notesDir.string() << "\n";
    }
}

// Command: notelify delete "TITLE"
void deleteNote(const fs::path& notesDir, const string& title) {
    if (!filenameChecker(title)) {
        cout << "[ERROR] Invalid filename!\n";
        return;
    }
    fs::path filePath = notesDir / (title + ".txt");
        if (!fs::exists(filePath)) {
            cout << "[ERROR] Note '" << title << "' not found.\n";
            return;
        }

        cout << "Are you sure you want to delete '" << title << "'? (Y/N): ";
        string choice;
        getline(cin, choice);
        if (choice != "y" && choice != "Y") {
            cout << "[CANCELLED] Deletion aborted.\n";
            return;
        }

        fs::remove(filePath);
        cout << "[SUCCESS] Note '" << title << "' deleted.\n";
}

void printHelp() {
    std::cout << R"(
 __        __      _       _ _  __               
 \ \    /\ \ \___ | |_ ___| (_)/ _|_   _         
  \ \  /  \/ / _ \| __/ _ \ | | |_| | | |        
  / / / /\  / (_) | ||  __/ | |  _| |_| |        
 /_/  \_\ \/ \___/ \__\___|_|_|_|  \__, |  _____ 
                                   |___/  |_____|
)" << '\n';
    cout << "Notelify - CLI Notes App (v2.2.0-beta)\n\n";
    cout << "Usage:\n";
    cout << "  notelify new \"TITLE\"      Create a new note\n";
    cout << "  notelify edit \"TITLE\"     Modify/overwrite an existing note\n";
    cout << "  notelify append \"TITLE\"   Append new lines to an existing note\n";
    cout << "  notelify list             List all saved notes\n";
    cout << "  notelify read \"TITLE\"     Read a note\n";
    cout << "  notelify delete \"TITLE\"   Delete a note\n";
    cout << "  notelify help             Show this help message\n";
}

int main(int argc, char* argv[]) {
    fs::path notesDir = getNotesDirectory();
    if (!initDirectory(notesDir)) {
        return 1;
    }
    if (argc < 2) {
        printHelp();
        return 0;
    }
    string command = argv[1];
    if (command == "help") {
        printHelp();
    } else if (command == "list") {
        listNotes(notesDir);
    } else if (argc < 3) {
        cout << "[ERROR] Missing title argument! Usage: notelify " << command << " \"TITLE\"\n";
    } else if (command == "new") {
        createNote(notesDir, argv[2]);
    } else if (command == "edit") {
        editNote(notesDir, argv[2]);
    } else if (command == "append") {
        appendNote(notesDir, argv[2]);
    } else if (command == "read") {
        readNote(notesDir, argv[2]);
    } else if (command == "delete") {
        deleteNote(notesDir, argv[2]);
    } else {
        cout << "[ERROR] Invalid command! Use 'notelify help' to see all available commands.\n";
    }
    return 0;
}