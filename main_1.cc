#include "text_editor.h"
#include <iostream>
#include <string>

int main() {
    TextEditor editor;
    std::string command;
    std::string filename;
    std::string text;
    size_t position, length, lines;

    std::cout << "Welcome to the Text Editor!\n";

    while (true) {
        std::cout << "Enter command: ";
        std::cin >> command;

        if (command == "open") {
            std::cout << "Enter the file name with format (ex: sample.txt): ";
            std::cin >> filename;
            editor.openFile(filename);
            editor.viewFileContent(); 
        } else if (command == "create") {
            std::cin >> filename;
            if (editor.createFile(filename)) {
                // No additional success message needed here, createFile already prints it
          } else {
                std::cout << "Failed to create the file \"" << filename << "\"." << std::endl;
          }
        } else if (command == "save") {
            std::cout << "Enter the file name to save with format (ex: sample.txt): ";
            std::cin >> filename;
            editor.saveFile(filename);
        } else if (command == "close") {
            editor.closeFile();
        } else if (command == "insert") {
            std::cout << "Enter the line and column numbers where you want to insert text: ";
            size_t line, col;
            std::cin >> line >> col;
    
            // Convert line and column to an index in textBuffer
            size_t index = editor.calculateIndexFromLineCol(line, col);
    
            std::cin.ignore();  // To consume the newline character
            std::cout << "Enter the text to insert: ";
            std::getline(std::cin, text);
    
            editor.setCursorPosition(index);  // Set the cursor position before inserting
            editor.insertText(text);
            editor.updateStatusBar();
        } else if (command == "delete") {
            std::cout << "Enter 'char' to delete a character, 'line' to delete a whole line, or 'block' to delete a text block: ";
            std::string deleteType;
            std::cin >> deleteType;

            if (deleteType == "char" || deleteType == "line" || deleteType == "block") {
                std::cout << "Enter the line and column numbers from where you want to start deletion: ";
                size_t line, col;
                std::cin >> line >> col;
        
                // Convert line and column to an index in textBuffer
                size_t index = editor.calculateIndexFromLineCol(line, col);
        
                if (deleteType == "char") {
                    // Delete a single character
                    editor.deleteText(index, 1);
                }
                else if (deleteType == "line") {
                    // Delete an entire line
                    editor.deleteText(index, 0);
                }
                else if (deleteType == "block") {
                    std::cout << "Enter the length of the text block you want to delete: ";
                    size_t length;
                    std::cin >> length;
                    editor.deleteText(index, length);
                }
            }
            else {
                std::cout << "Invalid deletion type." << std::endl;
            }
        } else if (command == "copy") {
            std::cout << "Enter the start position and length of text to copy: ";
            size_t start, length;
            std::cin >> start >> length;
            editor.copy(start, length);
            std::cout << "Text copied to clipboard." << std::endl;
        } else if (command == "paste") {
            std::cout << "Enter the position to paste the text: ";
            size_t pastePos;
            std::cin >> pastePos;
            editor.paste(pastePos);
            std::cout << "Text pasted." << std::endl;
        } else if (command == "find") {
            std::cout << "Enter the text to find: ";
            std::string searchText;
            std::cin.ignore();  // To consume the newline character
            std::getline(std::cin, searchText);

            size_t foundPosition = editor.findText(searchText);
            if (foundPosition != std::string::npos) {
                std::cout << "Text found at position: " << foundPosition << std::endl;
        }   else {
                std::cout << "Text not found." << std::endl;
                 }
        } else if (command == "replace") {
            std::cout << "Enter the text to find: ";
            std::string oldText;
            std::cin.ignore();  // To consume the newline character
            std::getline(std::cin, oldText);

            std::cout << "Enter the text to replace with: ";
            std::string newText;
            std::getline(std::cin, newText);

            editor.replaceText(oldText, newText);
            std::cout << "Text replaced." << std::endl;
        } else if (command == "goto") {
            std::cout << "Enter the line number to navigate to: ";
            size_t lineNumber;
            std::cin >> lineNumber;

            editor.goToLine(lineNumber);
            std::cout << "Navigated to line number " << lineNumber << "." << std::endl;
        } else if (command == "scroll") {
            std::cin >> lines;
            editor.scroll(lines);
        } else if (command == "highlight") {
              editor.highlightSyntax();
              std::cout << "Syntax highlighting applied." << std::endl;
        } else if (command == "undo") {
              editor.undo();
              std::cout << "Last action undone." << std::endl;
              editor.updateStatusBar();
        }
          else if (command == "redo") {
              editor.redo();
              std::cout << "Last action redone." << std::endl;
              editor.updateStatusBar();
          } else if (command == "quit") {
            break;
        } else {
            std::cout << "Unknown command." << std::endl;
        }     
    }

    return 0;
}
