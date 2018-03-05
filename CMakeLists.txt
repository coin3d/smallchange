cmake_minimum_required(VERSION 3.10.1)
project(SmallChange 
  VERSION 1.0.0
  DESCRIPTION "A collection of extensions to the Open Inventor API."
  LANGUAGES CXX
)

include(CMakePackageConfigHelpers)
include(GNUInstallDirs)

option(HAVE_DEBUG                   "Add debugging information during the configure process" ON)
option(SMALLCHANGE_BUILD_STATIC_LIB "Build SmallChange as a static library")

function(dump_variable)
  if (HAVE_DEBUG)
    foreach(f ${ARGN})
      message("${f} = ${${f}}")
    endforeach()
  endif(HAVE_DEBUG)
endfunction(dump_variable)

macro(dump_include_dirs)
  if (HAVE_DEBUG)
    get_property(dirs DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY INCLUDE_DIRECTORIES)
    foreach(dir ${dirs})
      message(STATUS "include dir = '${dir}'")
    endforeach()
  endif(HAVE_DEBUG)
endmacro(dump_include_dirs)

###############################################################################
# pre-reqs

find_package(Coin 4.0.0 REQUIRED)
find_package(Qt5 REQUIRED Core Widgets)
find_package(SoQt REQUIRED CONFIG)

dump_variable(
  Coin_INCLUDE_DIR
)

include(CheckIncludeFileCXX)
# Only the following defines are actually used inside the project sources:
# HAVE_SYS_STAT_H
# HAVE_UNISTD_H
# HAVE_WINDOWS_H
# consequentely the new config.h.cmake.in will be set based on the results of
# the following checks
dump_variable(HAVE_UNISTD_H HAVE_SYS_STAT_H HAVE_WINDOWS_H)
check_include_file_cxx(unistd.h HAVE_UNISTD_H)
check_include_file_cxx(sys/stat.h HAVE_SYS_STAT_H)
check_include_file_cxx(windows.h HAVE_WINDOWS_H)
dump_variable(HAVE_UNISTD_H HAVE_SYS_STAT_H HAVE_WINDOWS_H)
configure_file(config.h.cmake.in config.h)

###############################################################################
# Build setup

include_directories( BEFORE
  ${PROJECT_BINARY_DIR}
  ${PROJECT_SOURCE_DIR}/lib
  ${PROJECT_BINARY_DIR}/lib
  ${Coin_INCLUDE_DIR}
)

add_definitions(-DHAVE_CONFIG_H)
###############################################################################
# Build 

add_subdirectory(lib)
add_subdirectory(test-code)
add_subdirectory(docs)
