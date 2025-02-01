#ifndef TEXT_EDITOR_H
#define TEXT_EDITOR_H

#include <string>
#include <vector>
#include <stack>
#include <unordered_map>

class TextEditor {
public:
    TextEditor();
    ~TextEditor();

    void openFile(const std::string& filename);
    void viewFileContent() const;
    bool createFile(const std::string& filename);
    void saveFile(const std::string& filename);
    void closeFile();

    size_t calculateIndexFromLineCol(size_t line, size_t col) const;

    void setCursorPosition(size_t index);
    void insertText(const std::string& text);
    void deleteText(size_t position, size_t length);

    void copy(size_t position, size_t length);
    void paste(size_t position);

    size_t findText(const std::string& text) const;
    void replaceText(const std::string& oldText, const std::string& newText);
    void goToLine(size_t lineNumber);
    void scroll(size_t lines);
    void highlightSyntax();
    void undo();
    void redo();
    void updateStatusBar();
    void displayStatusBar() const;

private:
    std::string clipboard;
    std::string textBuffer;
    size_t gapStart, gapEnd;
    std::string currentFileName;
    bool isFileModified;
    std::stack<std::string> undoStack;
    std::stack<std::string> redoStack;
    std::unordered_map<std::string, std::string> syntaxRules;
    size_t currentLine, currentColumn;
    size_t firstVisibleLine;
    size_t firstVisibleColumn;
    void applySyntaxHighlighting();
    void handleError(const std::string& errorMessage);
};

#endif // TEXT_EDITOR_H
