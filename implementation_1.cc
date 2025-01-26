#include "text_editor.h"
#include <set>
#include <regex>
#include <fstream>
#include <sstream>
#include <iostream>
#include <algorithm>
#include <unordered_set>

TextEditor::TextEditor() {
    // Initialize member variables
    gapStart = 0;
    gapEnd = 0;
    isFileModified = false;
    currentLine = 0;
    currentColumn = 0;
}

TextEditor::~TextEditor() {
    // Destructor logic (if needed)
}

void TextEditor::openFile(const std::string& filename) {
    std::ifstream file(filename);
    if (!file) {
        std::cerr << "Could not open file: " << filename << std::endl;
        return;
    }

    // Clear existing buffer
    textBuffer.clear();
    gapStart = 0;
    gapEnd = 0;

    // Read file content into textBuffer
    std::string line;
    while (getline(file, line)) {
        textBuffer += line + "\n";
    }

    // Update file state
    currentFileName = filename;
    isFileModified = false;

    // Reset undo/redo stacks
    while (!undoStack.empty()) undoStack.pop();
    while (!redoStack.empty()) redoStack.pop();

    // Update status bar (optional)
    updateStatusBar();

    file.close();

    // Print success message
    std::cout << "The file \"" << filename << "\" opened successfully." << std::endl;
}


void TextEditor::viewFileContent() const {
    std::istringstream contentStream(textBuffer);
    std::string line;
    size_t lineNumber = 1;

    std::cout << "File Content of " << currentFileName << ":\n" << std::endl;

    while (getline(contentStream, line)) {
        std::cout << lineNumber << ": " << line << std::endl;
        ++lineNumber;
    }
}


bool TextEditor::createFile(const std::string& filename) {
    // Open a file stream with the given filename
    std::ofstream file(filename);

    // Check if the file stream is open (i.e., file is successfully created)
    if (!file.is_open()) {
        handleError("Failed to create file: " + filename);
        return false; // Return false if the file couldn't be created
    }

    // Print success message
    std::cout << "File \"" << filename << "\" created successfully." << std::endl;

    // Initialize editor state for a new file
    textBuffer.clear();
    currentFileName = filename;
    isFileModified = false;
    gapStart = 0;
    gapEnd = 0;
    while (!undoStack.empty()) undoStack.pop();
    while (!redoStack.empty()) redoStack.pop();
    updateStatusBar(); // Assuming this function updates the status bar

    // Close the file stream
    file.close();

    return true; // Return true if the file was successfully created
}



void TextEditor::saveFile(const std::string& filename) {
    std::ofstream file(filename);
    if (file.is_open()) {
        // Write textBuffer to file
        file << textBuffer;

        // Update file state
        isFileModified = false;
        currentFileName = filename;

        // Clear undo/redo stacks after saving (optional based on behavior you want)
        while (!undoStack.empty()) undoStack.pop();
        while (!redoStack.empty()) redoStack.pop();

        file.close();

        // Print success message
        std::cout << "File \"" << filename << "\" has been saved successfully." << std::endl;
    } else {
        handleError("Unable to open file for writing");
    }
}



void TextEditor::closeFile() {
    // Check if there are unsaved changes
    if (isFileModified) {
        // Prompt the user to save changes or discard them
        // This could be a simple console prompt or a more complex GUI dialog
        std::cout << "There are unsaved changes in " << currentFileName << ". Do you want to save them? (yes/no): ";
        std::string response;
        std::cin >> response;
        
        if (response == "yes") {
            saveFile(currentFileName);  // Save changes before closing
        }
    }

    // Reset the editor's state
    textBuffer.clear();
    currentFileName.clear();
    isFileModified = false;
    gapStart = 0;
    gapEnd = 0;
    while (!undoStack.empty()) undoStack.pop();
    while (!redoStack.empty()) redoStack.pop();

    // Optionally, update the status bar or other UI elements
    updateStatusBar();

    // File is now considered closed
}

void TextEditor::setCursorPosition(size_t position) {
    if (position <= textBuffer.length()) {
        currentColumn = position;
    } else {
        handleError("Invalid cursor position");
    }
}

void TextEditor::insertText(const std::string& text) {
    if (currentColumn <= textBuffer.length()) {
        // Insert the text at the current cursor position
        textBuffer.insert(currentColumn, text);

        // Update the cursor position
        currentColumn += text.length();

        // Mark the file as modified
        isFileModified = true;
    } else {
        handleError("Invalid cursor position");
    }
}

size_t TextEditor::calculateIndexFromLineCol(size_t line, size_t col) const {
    size_t index = 0;
    size_t currentLine = 1;
    while (currentLine < line && index < textBuffer.length()) {
        if (textBuffer[index] == '\n') {
            ++currentLine;
        }
        ++index;
    }
    // Add the column offset to the index, but do not exceed the textBuffer length
    return std::min(index + col - 1, textBuffer.length());
}

/*void TextEditor::setCursorPosition(size_t index) {
    if (index <= textBuffer.length()) {
        currentColumn = index;
    } else {
        handleError("Invalid index position");
    }
}*/


void TextEditor::deleteText(size_t position, size_t length) {
    if (position >= textBuffer.length()) {
        handleError("Invalid position for deletion");
        return;
    }

    // Adjust length to avoid deleting past the end of the buffer
    if (position + length > textBuffer.length()) {
        length = textBuffer.length() - position;
    }

    // Special case for deleting a single character
    if (length == 1) {
        textBuffer.erase(position, 1);
    }
    // Special case for deleting a whole line
    else if (length == 0) {
        size_t lineStart = textBuffer.rfind('\n', position);
        size_t lineEnd = textBuffer.find('\n', position);

        if (lineStart == std::string::npos) {
            lineStart = 0;  // This means we are at the first line
        } else {
            lineStart++;  // To start at the beginning of the line, not at the '\n'
        }

        if (lineEnd == std::string::npos) {
            lineEnd = textBuffer.length();  // This means we are at the last line
        }

        textBuffer.erase(lineStart, lineEnd - lineStart);
    }
    // Default case for deleting a specified range
    else {
        textBuffer.erase(position, length);
    }

    // Mark the file as modified
    isFileModified = true;
}


void TextEditor::copy(size_t position, size_t length) {
    if (position + length <= textBuffer.length()) {
        clipboard = textBuffer.substr(position, length);
    } else {
        handleError("Copy position and length exceed text buffer size");
    }
}

void TextEditor::paste(size_t position) {
    if (position <= textBuffer.length()) {
        textBuffer.insert(position, clipboard);
        isFileModified = true;
    } else {
        handleError("Paste position exceeds text buffer size");
    }
}


size_t TextEditor::findText(const std::string& text) const {
    size_t position = textBuffer.find(text);
    return position;  // Returns std::string::npos if not found
}

void TextEditor::replaceText(const std::string& oldText, const std::string& newText) {
    size_t pos = textBuffer.find(oldText);
    while (pos != std::string::npos) {
        textBuffer.replace(pos, oldText.length(), newText);
        pos = textBuffer.find(oldText, pos + newText.length());
        isFileModified = true;
    }
}


void TextEditor::goToLine(size_t lineNumber) {
    size_t currentLine = 1;  // Start at the first line
    size_t position = 0;
    size_t lineStart = 0;

    // Iterate over textBuffer to find the start of the specified line
    while (currentLine < lineNumber && position < textBuffer.length()) {
        if (textBuffer[position] == '\n') {
            currentLine++;
            lineStart = position + 1;  // Mark the start of the next line
        }
        position++;
    }

    if (currentLine == lineNumber) {
        // Set the cursor position to the beginning of the line
        currentColumn = position;

        // Find the end of the line
        size_t lineEnd = textBuffer.find('\n', lineStart);
        if (lineEnd == std::string::npos) {
            lineEnd = textBuffer.length();
        }

        // Display the selected line
        std::string line = textBuffer.substr(lineStart, lineEnd - lineStart);
        std::cout << "Line " << lineNumber << ": " << line << std::endl;

        // Optionally, update the status bar or other UI elements
        updateStatusBar();
    } else {
        handleError("Line number out of range");
    }
}


void TextEditor::scroll(size_t lines) {
    // Calculate the total number of lines in the text buffer
    size_t totalLines = std::count(textBuffer.begin(), textBuffer.end(), '\n') + 1;

    // Scroll down by the specified number of lines
    firstVisibleLine += lines;

    // Ensure we do not scroll past the end of the text buffer
    if (firstVisibleLine > totalLines) {
        firstVisibleLine = totalLines;
    }

    // Update the status bar or other UI elements to reflect the new scroll position
    updateStatusBar();
}


void TextEditor::undo() {
    if (!undoStack.empty()) {
        // Push the current state onto the redo stack
        redoStack.push(textBuffer);

        // Pop the last state from the undo stack and set it as the current text
        textBuffer = undoStack.top();
        undoStack.pop();

        // Update the editor state
        isFileModified = true;
        updateStatusBar();  // If you have a function to update the status bar
    } else {
        handleError("No more actions to undo");
    }
}

void TextEditor::redo() {
    if (!redoStack.empty()) {
        // Push the current state onto the undo stack
        undoStack.push(textBuffer);

        // Pop the last undone state from the redo stack and set it as the current text
        textBuffer = redoStack.top();
        redoStack.pop();

        // Update the editor state
        isFileModified = true;
        updateStatusBar();  // If you have a function to update the status bar
    } else {
        handleError("No more actions to redo");
    }
}


void TextEditor::displayStatusBar() const {
    std::cout << "Line: " << currentLine << ", Column: " << currentColumn;

    if (currentFileName.empty()) {
        std::cout << " - No file open";
    } else {
        std::cout << " - Editing: " << currentFileName;
        if (isFileModified) {
            std::cout << " [Modified]";
        } else {
            std::cout << " [Unmodified]";
        }
    }

    std::cout << std::endl;
}


void TextEditor::highlightSyntax() {
    // Syntax highlighting logic
    applySyntaxHighlighting();
}

/*void TextEditor::applySyntaxHighlighting() {
    // List of Python keywords (extend this list as needed)
    const std::set<std::string> pythonKeywords = {
        "def", "return", "if", "else", "for", "while", "break", "continue", "class",
        "import", "from", "in", "is", "and", "or", "not", "pass"
        // Add more keywords as necessary
    };

    std::istringstream iss(textBuffer);
    std::string word;

    while (iss >> word) {
        if (pythonKeywords.find(word) != pythonKeywords.end()) {
            // Print keyword in color (e.g., red)
            std::cout << "\033[31m" << word << "\033[0m" << " ";
        } else {
            // Print normal text
            std::cout << word << " ";
        }
    }
    std::cout << std::endl;
}*/


// Define a map from Python keywords to their ANSI color codes
const std::unordered_map<std::string, std::string> pythonKeywordsColors = {
   {"import", "\033[31m"}, // Red
    {"as", "\033[32m"},     // Green
    {"from", "\033[33m"},   // Yellow
    {"def", "\033[34m"},    // Blue
    {"return", "\033[35m"}, // Magenta
    {"if", "\033[36m"},     // Cyan
    {"else", "\033[94m"},   // Bright Blue
    {"for", "\033[95m"},    // Bright Magenta
    {"while", "\033[96m"},  // Bright Cyan
    {"break", "\033[91m"},  // Bright Red
    {"continue", "\033[92m"}, // Bright Green
    {"class", "\033[93m"},  // Bright Yellow
    {"try", "\033[41m"},    // Background Red
    {"except", "\033[42m"}, // Background Green
    {"raise", "\033[43m"},  // Background Yellow
    {"in", "\033[44m"},     // Background Blue
    {"not", "\033[45m"},    // Background Magenta
    {"is", "\033[46m"},     // Background Cyan
    // ... add more if needed
};

void TextEditor::applySyntaxHighlighting() {
    // Regex to match numbers
    std::regex numberRegex(R"(\b\d+(\.\d+)?\b)");

    // Use a string stream to read the buffer line by line
    std::istringstream iss(textBuffer);
    std::string line;
    size_t lineNumber = 1;  // Initialize line number

    while (std::getline(iss, line)) {
        // Print the line number followed by a colon and space for alignment
        std::cout << lineNumber << ": ";

        // Split the line into tokens
        std::istringstream lineStream(line);
        std::string token;
        while (lineStream >> token) {
            // Check for numbers and print them normally
            if (std::regex_match(token, numberRegex)) {
                std::cout << token << " ";
                continue;
            }

            // Check for keywords and apply their unique color
            bool isKeyword = false;
            for (const auto& kw : pythonKeywordsColors) {
                if (token == kw.first) {
                    std::cout << kw.second << token << "\033[0m" << " ";
                    isKeyword = true;
                    break;
                }
            }

            // If the token is not a keyword, print it normally
            if (!isKeyword) {
                std::cout << token << " ";
            }
        }
        std::cout << std::endl;
        lineNumber++;  // Increment line number after printing the line
    }
}




void TextEditor::updateStatusBar() {
    // Calculate the current line and column
    size_t line = 1, column = 1;
    for (size_t i = 0; i < currentColumn && i < textBuffer.length(); ++i) {
        if (textBuffer[i] == '\n') {
            line++;
            column = 1;
        } else {
            column++;
        }
    }

    // Update the current line and column
    currentLine = line;
    currentColumn = column;

    // Now display the updated status bar
    displayStatusBar();
}


void TextEditor::handleError(const std::string& errorMessage) {
    std::cerr << "Error: " << errorMessage << std::endl;
}
