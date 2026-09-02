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
void initDirectory(const fs::path& notesDir) {
        if (!fs::exists(notesDir)) {
            fs::create_directories(notesDir);
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
    if (fs::exists(filePath)) {
        cout << "[WARNING] Note '" << title << "' already exists! Use 'notelify edit \"" << title << "\"' instead.\n";
        return;
    }
    InputResult res = getMultiLineInput();
    if (res.cancelled) return;
    string content = res.content;

    ofstream file(filePath);
    if (file.is_open()) {
        file << content;
        file.close();
        cout << "[SUCCESS] Note saved to " << filePath.string() << "\n";
    } else {
        cout << "[ERROR] Could not save note.\n";
    }
}

// Command: notelify edit "TITLE"
void editNote(const fs::path& notesDir, const string& title) {
    if (!filenameChecker(title)){
        cout << "[ERROR] Invalid filename!\n";
        return;
    }
    fs::path filePath = notesDir / (title + ".txt");
    if (!fs::exists(filePath)) {
        cout << "[ERROR] Note '" << title << "' does not exist!\n";
        return;
    }

    cout << "\n--- Current Content of '" << title << "' ---\n";
    ifstream readFile(filePath);
    string line;
    while (getline(readFile, line)) {
        cout << line << "\n";
    }
    readFile.close();

    cout << "\n--- Enter New Content ---\n";
    InputResult res = getMultiLineInput();
    if (res.cancelled) return;
    string newContent = res.content;

    ofstream writeFile(filePath);
    if (writeFile.is_open()) {
        writeFile << newContent;
        writeFile.close();
        cout << "[SUCCESS] Note '" << title << "' updated.\n";
    } else {
        cout << "[ERROR] Could not update note.\n";
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
    if (!fs::exists(filePath)) {
        cout << "[ERROR] Note '" << title << "' does not exist!\n";
        return;
    }

    readNote(notesDir, title);

    cout << "\n--- Enter text to append ---\n";
    InputResult res = getMultiLineInput();
    if (res.cancelled) return;
    string addedContent = res.content;

    if (addedContent.empty()){
        cout << "[ERROR] Cannot append empty content!\n";
        return;
    }

    ofstream writeFile(filePath, std::ios::app);
    if (writeFile.is_open()) {
        writeFile << addedContent;
        writeFile.close();
        cout << "[SUCCESS] Content appended to '" << title << "'.\n";
    } else {
        cout << "[ERROR] Could not append to note.\n";
    }
}

// Command: notelify list
void listNotes(const fs::path& notesDir) {
    cout << "\n--- Your Notes ---\n";
    bool found = false;
        for (const auto& entry : fs::directory_iterator(notesDir)) {
            if (entry.is_regular_file() && entry.path().extension() == ".txt") {
                cout << "- " << entry.path().stem().string() << "\n";
                found = true;
            }
        }
    if (!found) {
        cout << "(No notes found)\n";
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
    cout << "Notelify - CLI Notes App (v2.1.0-beta)\n\n";
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
    initDirectory(notesDir);
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