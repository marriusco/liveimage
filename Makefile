# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 3.2

# Default target executed when no arguments are given to make.
default_target: all
.PHONY : default_target

# Allow only one "make -f Makefile2" at a time, but pass parallelism.
.NOTPARALLEL:

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
CMAKE_COMMAND = /usr/bin/cmake

# The command to remove a file.
RM = /usr/bin/cmake -E remove -f

# Escaping for special characters.
EQUALS = =

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = /home/pi/liveimage

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /home/pi/liveimage

#=============================================================================
# Targets provided globally by CMake.

# Special rule for the target edit_cache
edit_cache:
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --cyan "No interactive CMake dialog available..."
	/usr/bin/cmake -E echo No\ interactive\ CMake\ dialog\ available.
.PHONY : edit_cache

# Special rule for the target edit_cache
edit_cache/fast: edit_cache
.PHONY : edit_cache/fast

# Special rule for the target rebuild_cache
rebuild_cache:
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --cyan "Running CMake to regenerate build system..."
	/usr/bin/cmake -H$(CMAKE_SOURCE_DIR) -B$(CMAKE_BINARY_DIR)
.PHONY : rebuild_cache

# Special rule for the target rebuild_cache
rebuild_cache/fast: rebuild_cache
.PHONY : rebuild_cache/fast

# The main all target
all: cmake_check_build_system
	$(CMAKE_COMMAND) -E cmake_progress_start /home/pi/liveimage/CMakeFiles /home/pi/liveimage/CMakeFiles/progress.marks
	$(MAKE) -f CMakeFiles/Makefile2 all
	$(CMAKE_COMMAND) -E cmake_progress_start /home/pi/liveimage/CMakeFiles 0
.PHONY : all

# The main clean target
clean:
	$(MAKE) -f CMakeFiles/Makefile2 clean
.PHONY : clean

# The main clean target
clean/fast: clean
.PHONY : clean/fast

# Prepare targets for installation.
preinstall: all
	$(MAKE) -f CMakeFiles/Makefile2 preinstall
.PHONY : preinstall

# Prepare targets for installation.
preinstall/fast:
	$(MAKE) -f CMakeFiles/Makefile2 preinstall
.PHONY : preinstall/fast

# clear depends
depend:
	$(CMAKE_COMMAND) -H$(CMAKE_SOURCE_DIR) -B$(CMAKE_BINARY_DIR) --check-build-system CMakeFiles/Makefile.cmake 1
.PHONY : depend

#=============================================================================
# Target rules for targets named liveimage

# Build rule for target.
liveimage: cmake_check_build_system
	$(MAKE) -f CMakeFiles/Makefile2 liveimage
.PHONY : liveimage

# fast build rule for target.
liveimage/fast:
	$(MAKE) -f CMakeFiles/liveimage.dir/build.make CMakeFiles/liveimage.dir/build
.PHONY : liveimage/fast

jpeger.o: jpeger.cpp.o
.PHONY : jpeger.o

# target to build an object file
jpeger.cpp.o:
	$(MAKE) -f CMakeFiles/liveimage.dir/build.make CMakeFiles/liveimage.dir/jpeger.cpp.o
.PHONY : jpeger.cpp.o

jpeger.i: jpeger.cpp.i
.PHONY : jpeger.i

# target to preprocess a source file
jpeger.cpp.i:
	$(MAKE) -f CMakeFiles/liveimage.dir/build.make CMakeFiles/liveimage.dir/jpeger.cpp.i
.PHONY : jpeger.cpp.i

jpeger.s: jpeger.cpp.s
.PHONY : jpeger.s

# target to generate assembly for a file
jpeger.cpp.s:
	$(MAKE) -f CMakeFiles/liveimage.dir/build.make CMakeFiles/liveimage.dir/jpeger.cpp.s
.PHONY : jpeger.cpp.s

main.o: main.cpp.o
.PHONY : main.o

# target to build an object file
main.cpp.o:
	$(MAKE) -f CMakeFiles/liveimage.dir/build.make CMakeFiles/liveimage.dir/main.cpp.o
.PHONY : main.cpp.o

main.i: main.cpp.i
.PHONY : main.i

# target to preprocess a source file
main.cpp.i:
	$(MAKE) -f CMakeFiles/liveimage.dir/build.make CMakeFiles/liveimage.dir/main.cpp.i
.PHONY : main.cpp.i

main.s: main.cpp.s
.PHONY : main.s

# target to generate assembly for a file
main.cpp.s:
	$(MAKE) -f CMakeFiles/liveimage.dir/build.make CMakeFiles/liveimage.dir/main.cpp.s
.PHONY : main.cpp.s

outfilefmt.o: outfilefmt.cpp.o
.PHONY : outfilefmt.o

# target to build an object file
outfilefmt.cpp.o:
	$(MAKE) -f CMakeFiles/liveimage.dir/build.make CMakeFiles/liveimage.dir/outfilefmt.cpp.o
.PHONY : outfilefmt.cpp.o

outfilefmt.i: outfilefmt.cpp.i
.PHONY : outfilefmt.i

# target to preprocess a source file
outfilefmt.cpp.i:
	$(MAKE) -f CMakeFiles/liveimage.dir/build.make CMakeFiles/liveimage.dir/outfilefmt.cpp.i
.PHONY : outfilefmt.cpp.i

outfilefmt.s: outfilefmt.cpp.s
.PHONY : outfilefmt.s

# target to generate assembly for a file
outfilefmt.cpp.s:
	$(MAKE) -f CMakeFiles/liveimage.dir/build.make CMakeFiles/liveimage.dir/outfilefmt.cpp.s
.PHONY : outfilefmt.cpp.s

pnger.o: pnger.cpp.o
.PHONY : pnger.o

# target to build an object file
pnger.cpp.o:
	$(MAKE) -f CMakeFiles/liveimage.dir/build.make CMakeFiles/liveimage.dir/pnger.cpp.o
.PHONY : pnger.cpp.o

pnger.i: pnger.cpp.i
.PHONY : pnger.i

# target to preprocess a source file
pnger.cpp.i:
	$(MAKE) -f CMakeFiles/liveimage.dir/build.make CMakeFiles/liveimage.dir/pnger.cpp.i
.PHONY : pnger.cpp.i

pnger.s: pnger.cpp.s
.PHONY : pnger.s

# target to generate assembly for a file
pnger.cpp.s:
	$(MAKE) -f CMakeFiles/liveimage.dir/build.make CMakeFiles/liveimage.dir/pnger.cpp.s
.PHONY : pnger.cpp.s

sock.o: sock.cpp.o
.PHONY : sock.o

# target to build an object file
sock.cpp.o:
	$(MAKE) -f CMakeFiles/liveimage.dir/build.make CMakeFiles/liveimage.dir/sock.cpp.o
.PHONY : sock.cpp.o

sock.i: sock.cpp.i
.PHONY : sock.i

# target to preprocess a source file
sock.cpp.i:
	$(MAKE) -f CMakeFiles/liveimage.dir/build.make CMakeFiles/liveimage.dir/sock.cpp.i
.PHONY : sock.cpp.i

sock.s: sock.cpp.s
.PHONY : sock.s

# target to generate assembly for a file
sock.cpp.s:
	$(MAKE) -f CMakeFiles/liveimage.dir/build.make CMakeFiles/liveimage.dir/sock.cpp.s
.PHONY : sock.cpp.s

sockserver.o: sockserver.cpp.o
.PHONY : sockserver.o

# target to build an object file
sockserver.cpp.o:
	$(MAKE) -f CMakeFiles/liveimage.dir/build.make CMakeFiles/liveimage.dir/sockserver.cpp.o
.PHONY : sockserver.cpp.o

sockserver.i: sockserver.cpp.i
.PHONY : sockserver.i

# target to preprocess a source file
sockserver.cpp.i:
	$(MAKE) -f CMakeFiles/liveimage.dir/build.make CMakeFiles/liveimage.dir/sockserver.cpp.i
.PHONY : sockserver.cpp.i

sockserver.s: sockserver.cpp.s
.PHONY : sockserver.s

# target to generate assembly for a file
sockserver.cpp.s:
	$(MAKE) -f CMakeFiles/liveimage.dir/build.make CMakeFiles/liveimage.dir/sockserver.cpp.s
.PHONY : sockserver.cpp.s

v4ldevice.o: v4ldevice.cpp.o
.PHONY : v4ldevice.o

# target to build an object file
v4ldevice.cpp.o:
	$(MAKE) -f CMakeFiles/liveimage.dir/build.make CMakeFiles/liveimage.dir/v4ldevice.cpp.o
.PHONY : v4ldevice.cpp.o

v4ldevice.i: v4ldevice.cpp.i
.PHONY : v4ldevice.i

# target to preprocess a source file
v4ldevice.cpp.i:
	$(MAKE) -f CMakeFiles/liveimage.dir/build.make CMakeFiles/liveimage.dir/v4ldevice.cpp.i
.PHONY : v4ldevice.cpp.i

v4ldevice.s: v4ldevice.cpp.s
.PHONY : v4ldevice.s

# target to generate assembly for a file
v4ldevice.cpp.s:
	$(MAKE) -f CMakeFiles/liveimage.dir/build.make CMakeFiles/liveimage.dir/v4ldevice.cpp.s
.PHONY : v4ldevice.cpp.s

# Help Target
help:
	@echo "The following are some of the valid targets for this Makefile:"
	@echo "... all (the default if no target is provided)"
	@echo "... clean"
	@echo "... depend"
	@echo "... edit_cache"
	@echo "... liveimage"
	@echo "... rebuild_cache"
	@echo "... jpeger.o"
	@echo "... jpeger.i"
	@echo "... jpeger.s"
	@echo "... main.o"
	@echo "... main.i"
	@echo "... main.s"
	@echo "... outfilefmt.o"
	@echo "... outfilefmt.i"
	@echo "... outfilefmt.s"
	@echo "... pnger.o"
	@echo "... pnger.i"
	@echo "... pnger.s"
	@echo "... sock.o"
	@echo "... sock.i"
	@echo "... sock.s"
	@echo "... sockserver.o"
	@echo "... sockserver.i"
	@echo "... sockserver.s"
	@echo "... v4ldevice.o"
	@echo "... v4ldevice.i"
	@echo "... v4ldevice.s"
.PHONY : help



#=============================================================================
# Special targets to cleanup operation of make.

# Special rule to run CMake to check the build system integrity.
# No rule that depends on this can have commands that come from listfiles
# because they might be regenerated.
cmake_check_build_system:
	$(CMAKE_COMMAND) -H$(CMAKE_SOURCE_DIR) -B$(CMAKE_BINARY_DIR) --check-build-system CMakeFiles/Makefile.cmake 0
.PHONY : cmake_check_build_system

