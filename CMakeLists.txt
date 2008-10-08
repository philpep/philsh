#CMakeLists.txt

CONFIGURE_FILE(
  "${CMAKE_CURRENT_SOURCE_DIR}/cmake_uninstall.cmake.in"
  "${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake"
  IMMEDIATE @ONLY)

ADD_CUSTOM_TARGET(uninstall
  "${CMAKE_COMMAND}" -P "${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake")

# Minimum version of CMake
cmake_minimum_required(VERSION 2.6)
if(COMMAND cmake_policy)
  cmake_policy(VERSION 2.6)
endif()
# Source dir
set(SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR})
set(BUILD_DIR ${CMAKE_CURRENT_BINARY_DIR})
option(GENERATE_MANPAGES "generate manpages" ON)
# Project name - philsh
set(PROJECT_NAME philsh)
project(${PROJECT_NAME} C)
# Definition of the philsh source
set(philsh_src
  src/prompt.c
  src/exec.c
  src/exec_instruction.c
  src/file_instruction.c
  src/readconfig.c
  src/philsh.c
  src/int/alias.c
  src/int/cd.c
  src/int/whoami.c
  src/int/env.c
  src/int/pwd.c
  src/int/uname.c
  src/int/which.c)

# Find exterbal programs
macro(a_find_program var prg req)
    set(required ${req})
    find_program(${var} ${prg})
    if(NOT ${var})
        message(STATUS "${prg} not found.")
        if(required)
            message(FATAL_ERROR "${prg} is required to build wmfs")
        endif()
    else()
        message(STATUS "${prg} -> ${${var}}")
    endif()
endmacro()
find_program(GIT_EXECUTABLE git)
# programs needed for man pages
a_find_program(ASCIIDOC_EXECUTABLE asciidoc FALSE)
a_find_program(XMLTO_EXECUTABLE xmlto FALSE)
a_find_program(GZIP_EXECUTABLE gzip FALSE)
a_find_program(CAT_EXECUTABLE cat TRUE)

# Set the executable from the philsh_src
add_executable(philsh ${philsh_src})
# version
set(VERSION 0.2)
# Mail
set(MAIL "<philippe.pepiot@gmail.com>")
# Http site
set(HTTP "http://tux-atome.fr/philsh.git")
# Buildin
set(BUILTIN "which, cd, whoami, env et uname")
# CFLAGS
set(CFLAGS "-W -Wall -ansi -ggdb -fomit-frame-pointer")
set(CMAKE_C_FLAGS ${CFLAGS})
# Link Libraries
set(LIBRARIES_TO_LINK 
  readline)
#  confuse)
target_link_libraries(philsh ${LIBRARIES_TO_LINK})
# replace strings in configs
set(PHILSH_VERSION ${VERSION})
set(PHILSH_COMPILE_MACHINE ${CMAKE_SYSTEM_PROCESSOR})
set(PHILSH_COMPILE_BY $ENV{USER})
set(PHILSH_COMPILE_FLAGS ${CFLAGS})
set(PHILSH_LINKED_LIBS ${LIBRARIES_TO_LINK})
set(PHILSH_MAIL ${MAIL})
set(PHILSH_BUILTIN ${BUILTIN})
set(PHILSH_HTTP ${HTTP})
# Messages
message("Using these CFLAGS : ${CFLAGS}")
message("Linking with theses libraries : ${LIBRARIES_TO_LINK}")

# sets
# {{{ Install path and configuration variables
if(DEFINED PREFIX)
    set(PREFIX ${PREFIX} CACHE PATH "install prefix")
    set(CMAKE_INSTALL_PREFIX ${PREFIX})
else()
    set(PREFIX ${CMAKE_INSTALL_PREFIX} CACHE PATH "install prefix")
endif()

# {{{ Check if documentation can be build
if(GENERATE_MANPAGES)
    if(NOT ASCIIDOC_EXECUTABLE OR NOT XMLTO_EXECUTABLE OR NOT GZIP_EXECUTABLE)
        if(NOT ASCIIDOC_EXECUTABLE)
            SET(missing "asciidoc")
        endif()
        if(NOT XMLTO_EXECUTABLE)
            SET(missing ${missing} " xmlto")
        endif()
        if(NOT GZIP_EXECUTABLE)
            SET(missing ${missing} " gzip")
        endif()

        message(STATUS "Not generating manpages. Missing: " ${missing})
        set(GENERATE_MANPAGES OFF)
    endif()
endif()

# set man path
set(PHILSH_MAN_SRCS
  ${SOURCE_DIR}/philsh.1.txt)

if(DEFINED PHILSH_MAN_PATH)
   set(PHILSH_MAN_PATH ${PHILSH_MAN_PATH} CACHE PATH "philsh manpage directory")
else()
   set(PHILSH_MAN_PATH ${PREFIX}/share/man CACHE PATH "philsh manpage directory")
endif()

# {{{ Manpages
if(GENERATE_MANPAGES)
    foreach(txtfile ${PHILSH_MAN_SRCS})
        string(REGEX REPLACE ".txt\$" ".xml" xmlfile ${txtfile})
        string(REPLACE ${SOURCE_DIR}
                       ${BUILD_DIR} xmlfile ${xmlfile})
        string(REGEX REPLACE ".xml\$" ".gz" gzfile ${xmlfile})
        string(REGEX REPLACE ".gz\$" "" manfile ${gzfile})

        add_custom_command(
            COMMAND ${CAT_EXECUTABLE} ${txtfile}
                    | ${ASCIIDOC_EXECUTABLE} -d manpage -b docbook -o ${xmlfile} -
            WORKING_DIRECTORY ${BUILD_DIR}
            OUTPUT  ${xmlfile}
            DEPENDS ${txtfile}
            VERBATIM)
        add_custom_command(
            COMMAND ${XMLTO_EXECUTABLE} man ${xmlfile}
            OUTPUT  ${manfile}
            DEPENDS ${xmlfile})
        add_custom_command(
            COMMAND ${CAT_EXECUTABLE} ${manfile}
                    | ${GZIP_EXECUTABLE}
                    > ${gzfile}
            OUTPUT  ${gzfile}
            DEPENDS ${manfile}
            VERBATIM)

        set(MAN_FILES ${MAN_FILES} ${gzfile})

        # figure out in what section to install to from filename
        string(REGEX REPLACE "^.*\\.([0-9])\\.gz\$" "\\1" section ${gzfile})
        set(PHILSH_MAN${section}_FILES ${WMFS_MAN${section}_FILES} ${gzfile})
    endforeach()

    add_custom_target(man ALL DEPENDS ${MAN_FILES})
endif()
# }}}

# Configure files
set(philsh_configure_files
  src/philsh_build.h.in)

macro(a_configure_file file)
    string(REGEX REPLACE ".in\$" "" outfile ${file})
    message(STATUS "Configuring ${outfile}")
    configure_file(${SOURCE_DIR}/${file}
                   ${SOURCE_DIR}/${outfile}
                   ESCAPE_QUOTE
                   @ONLY)
endmacro()

foreach(file ${philsh_configure_files})
    a_configure_file(${file})
endforeach()

# installs - sets
set(PROJECT_DATA_PATH share/${PROJECT_NAME})
set(PROJECT_TODO ${SOURCE_DIR}/TODO)
set(PROJECT_README ${SOURCE_DIR}/README)
set(PROJECT_RC ${SOURCE_DIR}/philshrc.example)

# installs
install(TARGETS ${PROJECT_NAME} RUNTIME DESTINATION bin)
if(GENERATE_MANPAGES)
  install(FILES ${PHILSH_MAN1_FILES} DESTINATION ${PHILSH_MAN_PATH}/man1)
endif()
install(FILES ${PROJECT_TODO} ${PROJECT_README} ${PROJECT_RC} DESTINATION
  ${PROJECT_DATA_PATH})
