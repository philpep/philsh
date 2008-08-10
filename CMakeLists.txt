#CMakeLists.txt

# Minimum version of CMake
cmake_minimum_required(VERSION 2.6)
if(COMMAND cmake_policy)
  cmake_policy(VERSION 2.6)
endif()
# Source dir
set(SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR})
set(BUILD_DIR ${CMAKE_CURRENT_BINARY_DIR})
# Project name - wmfs
project(philsh C)
# Definition of the wmfs source
set(philsh_src
  src/prompt.c
  src/shell.c
  src/int/cd.c
  src/int/env.c
  src/int/ls.c
  src/int/pwd.c
  src/int/uname.c
  src/int/which.c)

# Set the executable from the wmfs_src
add_executable(philsh ${philsh_src})
# CFLAGS
set(CFLAGS "-W -Wall -ansi")
set(CMAKE_C_FLAGS ${CFLAGS})
# Link Libraries
set(LIBRARIES_TO_LINK 
  readline)
target_link_libraries(philsh ${LIBRARIES_TO_LINK})

# Messages
message("Using these CFLAGS : ${CFLAGS}")
message("Linking with theses libraries : ${LIBRARIES_TO_LINK}")

