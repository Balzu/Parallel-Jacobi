# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 3.8

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
CMAKE_COMMAND = /home/balzu/Scaricati/clion/clion-2017.2.3/bin/cmake/bin/cmake

# The command to remove a file.
RM = /home/balzu/Scaricati/clion/clion-2017.2.3/bin/cmake/bin/cmake -E remove -f

# Escaping for special characters.
EQUALS = =

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = /home/balzu/Documenti/Università/SPM/MyCode/final_project

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /home/balzu/Documenti/Università/SPM/MyCode/final_project/cmake-build-debug

# Include any dependencies generated for this target.
include CMakeFiles/final_project.dir/depend.make

# Include the progress variables for this target.
include CMakeFiles/final_project.dir/progress.make

# Include the compile flags for this target's objects.
include CMakeFiles/final_project.dir/flags.make

CMakeFiles/final_project.dir/jacobi_pt.cpp.o: CMakeFiles/final_project.dir/flags.make
CMakeFiles/final_project.dir/jacobi_pt.cpp.o: ../jacobi_pt.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/balzu/Documenti/Università/SPM/MyCode/final_project/cmake-build-debug/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building CXX object CMakeFiles/final_project.dir/jacobi_pt.cpp.o"
	/usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/final_project.dir/jacobi_pt.cpp.o -c /home/balzu/Documenti/Università/SPM/MyCode/final_project/jacobi_pt.cpp

CMakeFiles/final_project.dir/jacobi_pt.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/final_project.dir/jacobi_pt.cpp.i"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/balzu/Documenti/Università/SPM/MyCode/final_project/jacobi_pt.cpp > CMakeFiles/final_project.dir/jacobi_pt.cpp.i

CMakeFiles/final_project.dir/jacobi_pt.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/final_project.dir/jacobi_pt.cpp.s"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/balzu/Documenti/Università/SPM/MyCode/final_project/jacobi_pt.cpp -o CMakeFiles/final_project.dir/jacobi_pt.cpp.s

CMakeFiles/final_project.dir/jacobi_pt.cpp.o.requires:

.PHONY : CMakeFiles/final_project.dir/jacobi_pt.cpp.o.requires

CMakeFiles/final_project.dir/jacobi_pt.cpp.o.provides: CMakeFiles/final_project.dir/jacobi_pt.cpp.o.requires
	$(MAKE) -f CMakeFiles/final_project.dir/build.make CMakeFiles/final_project.dir/jacobi_pt.cpp.o.provides.build
.PHONY : CMakeFiles/final_project.dir/jacobi_pt.cpp.o.provides

CMakeFiles/final_project.dir/jacobi_pt.cpp.o.provides.build: CMakeFiles/final_project.dir/jacobi_pt.cpp.o


# Object files for target final_project
final_project_OBJECTS = \
"CMakeFiles/final_project.dir/jacobi_pt.cpp.o"

# External object files for target final_project
final_project_EXTERNAL_OBJECTS =

final_project: CMakeFiles/final_project.dir/jacobi_pt.cpp.o
final_project: CMakeFiles/final_project.dir/build.make
final_project: CMakeFiles/final_project.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=/home/balzu/Documenti/Università/SPM/MyCode/final_project/cmake-build-debug/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Linking CXX executable final_project"
	$(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/final_project.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
CMakeFiles/final_project.dir/build: final_project

.PHONY : CMakeFiles/final_project.dir/build

CMakeFiles/final_project.dir/requires: CMakeFiles/final_project.dir/jacobi_pt.cpp.o.requires

.PHONY : CMakeFiles/final_project.dir/requires

CMakeFiles/final_project.dir/clean:
	$(CMAKE_COMMAND) -P CMakeFiles/final_project.dir/cmake_clean.cmake
.PHONY : CMakeFiles/final_project.dir/clean

CMakeFiles/final_project.dir/depend:
	cd /home/balzu/Documenti/Università/SPM/MyCode/final_project/cmake-build-debug && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/balzu/Documenti/Università/SPM/MyCode/final_project /home/balzu/Documenti/Università/SPM/MyCode/final_project /home/balzu/Documenti/Università/SPM/MyCode/final_project/cmake-build-debug /home/balzu/Documenti/Università/SPM/MyCode/final_project/cmake-build-debug /home/balzu/Documenti/Università/SPM/MyCode/final_project/cmake-build-debug/CMakeFiles/final_project.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : CMakeFiles/final_project.dir/depend

