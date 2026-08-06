#include <iostream>
#include <string>
#include <fstream>
#include <filesystem>
#include <cstdlib>
#include <algorithm>
#include <cctype>

namespace fs = std::filesystem;
using namespace std;

string sanitizeFilename(const string& rawTitle) {
    string cleanTitle = "";
    
    for (char c : rawTitle) {
        if (c == ' ') {
            cleanTitle += '-'; // Convert spaces to dashes
        } else if (std::isalnum(static_cast<unsigned char>(c)) || c == '-' || c == '_') {
            cleanTitle += static_cast<char>(std::tolower(static_cast<unsigned char>(c))); // Keep alphanumeric, dashes, underscores, and lowercase them
        }
        // Skips invalid OS path characters like / \ : * ? " < > |
    }

    // If the title becomes completely empty after stripping invalid chars, give it a fallback name
    if (cleanTitle.empty()) {
        cleanTitle = "untitled-note";
    }

    return cleanTitle;
}

// Get directory C:\Users\[username]\.notelify
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
void initDirectory(const fs::path& notesDir) {
    if (!fs::exists(notesDir)) {
        fs::create_directories(notesDir);
    }
}
// Helper to read multiple lines until the user types :save on a new line
string getMultiLineInput() {
    cout << "Enter content below (type ':save' on a new line when done):\n";
    cout << "--------------------------------------------------------\n";
    string fullText = "";
    string line;
    while (true) {
        getline(cin, line);
        if (line == ":save") {
            break;
        }
        fullText += line + "\n";
    }
    return fullText;
}

// Command: notelify new "TITLE"
void createNote(const fs::path& notesDir, const string& rawTitle) {
    string title = sanitizeFilename(rawTitle);
    fs::path filePath = notesDir / (title + ".txt");
    if (fs::exists(filePath)) {
        cout << "[WARNING] Note '" << title << "' already exists! Use 'notelify edit \"" << rawTitle << "\"' instead.\n";
        return;
    }
    
    string content = getMultiLineInput();
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
void editNote(const fs::path& notesDir, const string& rawTitle) {
    string title = sanitizeFilename(rawTitle);
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
    string newContent = getMultiLineInput();

    ofstream writeFile(filePath);
    if (writeFile.is_open()) {
        writeFile << newContent;
        writeFile.close();
        cout << "[SUCCESS] Note '" << title << "' updated.\n";
    } else {
        cout << "[ERROR] Could not update note.\n";
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

// Command: notelify read "TITLE"
void readNote(const fs::path& notesDir, const string& rawTitle) {
    string title = sanitizeFilename(rawTitle);
    fs::path filePath = notesDir / (title + ".txt");
    if (!fs::exists(filePath)) {
        cout << "[ERROR] Note '" << title << "' does not exist!\n";
        return;
    }
    ifstream file(filePath);
    string line;
    cout << "\n--- " << title << " ---\n";
    while (getline(file, line)) {
        cout << line << "\n";
    }
    cout << "-------------------\n";
}

// Command: notelify delete "TITLE"
void deleteNote(const fs::path& notesDir, const string& rawTitle) {
    string title = sanitizeFilename(rawTitle);
    fs::path filePath = notesDir / (title + ".txt");
    if (fs::exists(filePath)) {
        fs::remove(filePath);
        cout << "[SUCCESS] Note '" << title << "' deleted.\n";
    } else {
        cout << "[ERROR] Note '" << title << "' not found.\n";
    }
}

void printHelp() {
    cout << "Usage:\n";
    cout << "  notelify new \"TITLE\"      Create a new note\n";
    cout << "  notelify edit \"TITLE\"     Modify/overwrite an existing note\n";
    cout << "  notelify list             List all saved notes\n";
    cout << "  notelify read \"TITLE\"     Read a note\n";
    cout << "  notelify delete \"TITLE\"   Delete a note\n";
}

int main(int argc, char* argv[]) {
    fs::path notesDir = getNotesDirectory();
    initDirectory(notesDir);
    if (argc < 2) {
        printHelp();
        return 0;
    }
    string command = argv[1];
    if (command == "new" && argc >= 3) {
        createNote(notesDir, argv[2]);
    } else if (command == "edit" && argc >= 3) {
        editNote(notesDir, argv[2]);
    } else if (command == "list") {
        listNotes(notesDir);
    } else if (command == "read" && argc >= 3) {
        readNote(notesDir, argv[2]);
    } else if (command == "delete" && argc >= 3) {
        deleteNote(notesDir, argv[2]);
    } else {
        printHelp();
    }
    return 0;
}