C++ Command-Line Application


A lightweight command-line text editor with basic file operations, text editing, syntax highlighting (Python), and undo/redo functionality.

Usage

Commands

open: Open a file	open example.txt
create: Create a new file	create newfile.txt
save: Save the current file	save example.txt
close: Close the current file	close
insert: Insert text at a line/column	insert 3 5 "Hello World"
delete: Delete a character, line, or block	delete line 5 1
copy: Copy text to clipboard	copy 10 5
paste: Paste text from clipboard	paste 20
find: Search for text	find "error"
replace: Replace text	replace "old" "new"
goto: Navigate to a line	goto 15
scroll: Scroll up/down	scroll 5
highlight: Apply syntax highlighting	highlight
undo: Undo the last action	undo
redo: Redo the last undone action	redo
quit: Exit the editor	quit

Error Handling

The editor displays descriptive errors for
Invalid file paths.

>Out-of-bounds cursor positions.
>Unsupported commands.
>Limitations
>Command-line interface only (no GUI).
>Basic syntax highlighting (supports Python only).
>No support for large files (>1MB).

Future Enhancements Suggestions

>Add support for more programming languages (e.g., C++, JavaScript).
>Implement a graphical user interface (GUI).
>Optimize performance for large files using a gap buffer.
>Add regex support for search/replace.
