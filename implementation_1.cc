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
    currentLine = 1;
    currentColumn = 1;
    firstVisibleLine = 1;
}

TextEditor::~TextEditor() {
    // Destructor logic (if needed)
}

void TextEditor::openFile(const std::string& filename) {
    std::ifstream file(filename);
    if (!file) {
        std::cerr << "Error: Could not open file: " << filename << std::endl;
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

    // Update status bar
    updateStatusBar();

    file.close();

    std::cout << "File \"" << filename << "\" opened successfully." << std::endl;
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
    std::ofstream file(filename);
    if (!file.is_open()) {
        handleError("Failed to create file: " + filename);
        return false;
    }

    std::cout << "File \"" << filename << "\" created successfully." << std::endl;

    // Initialize editor state for a new file
    textBuffer.clear();
    currentFileName = filename;
    isFileModified = false;
    gapStart = 0;
    gapEnd = 0;
    while (!undoStack.empty()) undoStack.pop();
    while (!redoStack.empty()) redoStack.pop();
    updateStatusBar();

    file.close();
    return true;
}

void TextEditor::saveFile(const std::string& filename) {
    std::ofstream file(filename);
    if (file.is_open()) {
        file << textBuffer;
        isFileModified = false;
        currentFileName = filename;

        // Clear undo/redo stacks after saving
        while (!undoStack.empty()) undoStack.pop();
        while (!redoStack.empty()) redoStack.pop();

        file.close();
        std::cout << "File \"" << filename << "\" saved successfully." << std::endl;
    } else {
        handleError("Unable to open file for writing");
    }
}

void TextEditor::closeFile() {
    if (isFileModified) {
        std::cout << "There are unsaved changes in " << currentFileName << ". Do you want to save them? (yes/no): ";
        std::string response;
        std::cin >> response;

        if (response == "yes") {
            saveFile(currentFileName);
        }
    }

    // Reset editor state
    textBuffer.clear();
    currentFileName.clear();
    isFileModified = false;
    gapStart = 0;
    gapEnd = 0;
    while (!undoStack.empty()) undoStack.pop();
    while (!redoStack.empty()) redoStack.pop();

    updateStatusBar();
    std::cout << "File closed." << std::endl;
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
        textBuffer.insert(currentColumn, text);
        currentColumn += text.length();
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
    return std::min(index + col - 1, textBuffer.length());
}

void TextEditor::deleteText(size_t position, size_t length) {
    if (position >= textBuffer.length()) {
        handleError("Invalid position for deletion");
        return;
    }

    if (position + length > textBuffer.length()) {
        length = textBuffer.length() - position;
    }

    textBuffer.erase(position, length);
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
    size_t currentLine = 1;
    size_t position = 0;
    size_t lineStart = 0;

    while (currentLine < lineNumber && position < textBuffer.length()) {
        if (textBuffer[position] == '\n') {
            currentLine++;
            lineStart = position + 1;
        }
        position++;
    }

    if (currentLine == lineNumber) {
        currentColumn = position;
        size_t lineEnd = textBuffer.find('\n', lineStart);
        if (lineEnd == std::string::npos) {
            lineEnd = textBuffer.length();
        }

        std::string line = textBuffer.substr(lineStart, lineEnd - lineStart);
        std::cout << "Line " << lineNumber << ": " << line << std::endl;
        updateStatusBar();
    } else {
        handleError("Line number out of range");
    }
}

void TextEditor::scroll(size_t lines) {
    size_t totalLines = std::count(textBuffer.begin(), textBuffer.end(), '\n') + 1;
    firstVisibleLine += lines;

    if (firstVisibleLine > totalLines) {
        firstVisibleLine = totalLines;
    }

    updateStatusBar();
}

void TextEditor::undo() {
    if (!undoStack.empty()) {
        redoStack.push(textBuffer);
        textBuffer = undoStack.top();
        undoStack.pop();
        isFileModified = true;
        updateStatusBar();
    } else {
        handleError("No more actions to undo");
    }
}

void TextEditor::redo() {
    if (!redoStack.empty()) {
        undoStack.push(textBuffer);
        textBuffer = redoStack.top();
        redoStack.pop();
        isFileModified = true;
        updateStatusBar();
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
    applySyntaxHighlighting();
}

const std::unordered_map<std::string, std::string> pythonKeywordsColors = {
    {"import", "\033[31m"}, {"as", "\033[32m"}, {"from", "\033[33m"},
    {"def", "\033[34m"}, {"return", "\033[35m"}, {"if", "\033[36m"},
    {"else", "\033[94m"}, {"for", "\033[95m"}, {"while", "\033[96m"},
    {"break", "\033[91m"}, {"continue", "\033[92m"}, {"class", "\033[93m"},
    {"try", "\033[41m"}, {"except", "\033[42m"}, {"raise", "\033[43m"},
    {"in", "\033[44m"}, {"not", "\033[45m"}, {"is", "\033[46m"}
};

void TextEditor::applySyntaxHighlighting() {
    std::regex numberRegex(R"(\b\d+(\.\d+)?\b)");
    std::istringstream iss(textBuffer);
    std::string line;
    size_t lineNumber = 1;

    while (std::getline(iss, line)) {
        std::cout << lineNumber << ": ";

        std::istringstream lineStream(line);
        std::string token;
        while (lineStream >> token) {
            if (std::regex_match(token, numberRegex)) {
                std::cout << token << " ";
                continue;
            }

            bool isKeyword = false;
            for (const auto& kw : pythonKeywordsColors) {
                if (token == kw.first) {
                    std::cout << kw.second << token << "\033[0m" << " ";
                    isKeyword = true;
                    break;
                }
            }

            if (!isKeyword) {
                std::cout << token << " ";
            }
        }
        std::cout << std::endl;
        lineNumber++;
    }
}

void TextEditor::updateStatusBar() {
    size_t line = 1, column = 1;
    for (size_t i = 0; i < currentColumn && i < textBuffer.length(); ++i) {
        if (textBuffer[i] == '\n') {
            line++;
            column = 1;
        } else {
            column++;
        }
    }

    currentLine = line;
    currentColumn = column;
    displayStatusBar();
}

void TextEditor::handleError(const std::string& errorMessage) {
    std::cerr << "Error: " << errorMessage << std::endl;
}
