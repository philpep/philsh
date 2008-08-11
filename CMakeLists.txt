#CMakeLists.txt

# Minimum version of CMake
cmake_minimum_required(VERSION 2.6)
if(COMMAND cmake_policy)
  cmake_policy(VERSION 2.6)
endif()
# Source dir
set(SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR})
set(BUILD_DIR ${CMAKE_CURRENT_BINARY_DIR})
# Project name - philsh
project(philsh C)
# Definition of the philsh source
set(philsh_src
  src/prompt.c
  src/shell.c
  src/int/cd.c
  src/int/env.c
  src/int/ls.c
  src/int/pwd.c
  src/int/uname.c
  src/int/which.c)

# Set the executable from the philsh_src
add_executable(philsh ${philsh_src})
# version
set(VERSION 0.1)
# CFLAGS
set(CFLAGS "-W -Wall -ansi")
set(CMAKE_C_FLAGS ${CFLAGS})
# Link Libraries
set(LIBRARIES_TO_LINK 
  readline)
target_link_libraries(philsh ${LIBRARIES_TO_LINK})
# replace strings in configs
set(PHILSH_VERSION ${VERSION})
set(PHILSH_COMPILE_MACHINE ${CMAKE_SYSTEM_PROCESSOR})
set(PHILSH_COMPILE_BY $ENV{USER})
set(PHILSH_COMPILE_FLAGS ${CFLAGS})
set(PHILSH_LINKED_LIBS ${LIBRARIES_TO_LINK})
# Messages
message("Using these CFLAGS : ${CFLAGS}")
message("Linking with theses libraries : ${LIBRARIES_TO_LINK}")
# Configure files
set(philsh_configure_files
  src/philsh_build.h.in)

macro(a_configure_file file)
    string(REGEX REPLACE ".in\$" "" outfile ${file})
    message(STATUS "Configuring ${outfile}")
    configure_file(${SOURCE_DIR}/${file}
                   ${BUILD_DIR}/${outfile}
                   ESCAPE_QUOTE
                   @ONLY)
endmacro()

foreach(file ${philsh_configure_files})
    a_configure_file(${file})
endforeach()