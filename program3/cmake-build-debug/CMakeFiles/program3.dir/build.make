# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 3.15

# Delete rule output on recipe failure.
.DELETE_ON_ERROR:


#=============================================================================
# Special targets provided by cmake.

# Disable implicit rules so canonical targets will work.
.SUFFIXES:


# Remove some rules from gmake that .SUFFIXES does not remove.
SUFFIXES =

.SUFFIXES: .hpux_make_needs_suffix_list


# Suppress display of executed commands.
$(VERBOSE).SILENT:


# A target that is always out of date.
cmake_force:

.PHONY : cmake_force

#=============================================================================
# Set environment variables for the build.

# The shell in which to execute make rules.
SHELL = /bin/sh

# The CMake executable.
CMAKE_COMMAND = /Applications/CLion.app/Contents/bin/cmake/mac/bin/cmake

# The command to remove a file.
RM = /Applications/CLion.app/Contents/bin/cmake/mac/bin/cmake -E remove -f

# Escaping for special characters.
EQUALS = =

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = "/Users/tioteo/Desktop/Cooper Union/Junior/Fall 2019/ComOS(C)/program3"

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = "/Users/tioteo/Desktop/Cooper Union/Junior/Fall 2019/ComOS(C)/program3/cmake-build-debug"

# Include any dependencies generated for this target.
include CMakeFiles/program3.dir/depend.make

# Include the progress variables for this target.
include CMakeFiles/program3.dir/progress.make

# Include the compile flags for this target's objects.
include CMakeFiles/program3.dir/flags.make

CMakeFiles/program3.dir/main.c.o: CMakeFiles/program3.dir/flags.make
CMakeFiles/program3.dir/main.c.o: ../main.c
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir="/Users/tioteo/Desktop/Cooper Union/Junior/Fall 2019/ComOS(C)/program3/cmake-build-debug/CMakeFiles" --progress-num=$(CMAKE_PROGRESS_1) "Building C object CMakeFiles/program3.dir/main.c.o"
	/Library/Developer/CommandLineTools/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -o CMakeFiles/program3.dir/main.c.o   -c "/Users/tioteo/Desktop/Cooper Union/Junior/Fall 2019/ComOS(C)/program3/main.c"

CMakeFiles/program3.dir/main.c.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing C source to CMakeFiles/program3.dir/main.c.i"
	/Library/Developer/CommandLineTools/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -E "/Users/tioteo/Desktop/Cooper Union/Junior/Fall 2019/ComOS(C)/program3/main.c" > CMakeFiles/program3.dir/main.c.i

CMakeFiles/program3.dir/main.c.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling C source to assembly CMakeFiles/program3.dir/main.c.s"
	/Library/Developer/CommandLineTools/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -S "/Users/tioteo/Desktop/Cooper Union/Junior/Fall 2019/ComOS(C)/program3/main.c" -o CMakeFiles/program3.dir/main.c.s

# Object files for target program3
program3_OBJECTS = \
"CMakeFiles/program3.dir/main.c.o"

# External object files for target program3
program3_EXTERNAL_OBJECTS =

program3: CMakeFiles/program3.dir/main.c.o
program3: CMakeFiles/program3.dir/build.make
program3: CMakeFiles/program3.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir="/Users/tioteo/Desktop/Cooper Union/Junior/Fall 2019/ComOS(C)/program3/cmake-build-debug/CMakeFiles" --progress-num=$(CMAKE_PROGRESS_2) "Linking C executable program3"
	$(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/program3.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
CMakeFiles/program3.dir/build: program3

.PHONY : CMakeFiles/program3.dir/build

CMakeFiles/program3.dir/clean:
	$(CMAKE_COMMAND) -P CMakeFiles/program3.dir/cmake_clean.cmake
.PHONY : CMakeFiles/program3.dir/clean

CMakeFiles/program3.dir/depend:
	cd "/Users/tioteo/Desktop/Cooper Union/Junior/Fall 2019/ComOS(C)/program3/cmake-build-debug" && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" "/Users/tioteo/Desktop/Cooper Union/Junior/Fall 2019/ComOS(C)/program3" "/Users/tioteo/Desktop/Cooper Union/Junior/Fall 2019/ComOS(C)/program3" "/Users/tioteo/Desktop/Cooper Union/Junior/Fall 2019/ComOS(C)/program3/cmake-build-debug" "/Users/tioteo/Desktop/Cooper Union/Junior/Fall 2019/ComOS(C)/program3/cmake-build-debug" "/Users/tioteo/Desktop/Cooper Union/Junior/Fall 2019/ComOS(C)/program3/cmake-build-debug/CMakeFiles/program3.dir/DependInfo.cmake" --color=$(COLOR)
.PHONY : CMakeFiles/program3.dir/depend

