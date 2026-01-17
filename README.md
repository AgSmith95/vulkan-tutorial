# Purpose

Trying to follow the official Vulkan Tutorial.

# Build

## From Command Line

Build:
```
	mkdir build
	cmake -B build
	cmake --build build
```

Launch:
```
	./build/vulkan-tutorial
```

Debug:
```
	gdb ./build/vulkan-tutorial
```

## From VSCode

1. Open folder in VSCode
2. Install extension "CMake Tools" - https://marketplace.visualstudio.com/items?itemName=ms-vscode.cmake-tools
3. Configure build with "CMake Tools" extension ("Show all Commands" Ctrl+Shift+P):
	1. CMake: Scan for Kits
	2. CMake: Select Kit -> select compiler
	3. CMake: Configure
4. Build with "CMake Tools":
	* "Show all Commands" (Ctrl+Shift+P) -> CMake: Build
	* Press "CMake: Build" button (cog icon, should be on the bottom of the VSCode window on a strip [Status Bar])
5. Launch:
	* "Show all Commands" (Ctrl+Shift+P) -> CMake: Run Without Debugging
	* Press "CMake: Launch the seected target ..." (play icon on a [Status Bar])
6. Debug:
	* "Show all Commands" (Ctrl+Shift+P) -> CMake: Debug
	* Press "CMake: Launch the debugger for the selected target ..." button (bug icon on a [Status Bar])

# Resources:
1. https://vulkan-tutorial.com/Introduction
