cmake_minimum_required(VERSION 3.10.1)
project(SmallChange
  VERSION 1.0.0
  DESCRIPTION "A collection of extensions to the Open Inventor API."
  LANGUAGES CXX
)
# PROJECT_NAME is used also for the target library name and the package name
string(TOLOWER "${PROJECT_NAME}" SM_SHORTNAME)
set(SM_URL "https://bitbucket.org/Coin3D/${SM_SHORTNAME}")
set(SM_CONTACT "coin-support@coin3d.org")

include(CheckIncludeFileCXX)
include(CMakePackageConfigHelpers)
include(GNUInstallDirs)

option(HAVE_DEBUG                   "Add debugging information during the configure process" ON)
option(SmallChange_BUILD_STATIC_LIB "Build SmallChange as a static library")

function(versionize)
  foreach(dir ${ARGN})
    set(name      "CMAKE_INSTALL_${dir}")
    set(full-name "CMAKE_INSTALL_FULL_${dir}")
    set(value      "${${name}}/${PROJECT_NAME}")
    set(full-value "${CMAKE_INSTALL_PREFIX}/${value}")
    set(${name}      ${value}      PARENT_SCOPE)
    set(${full-name} ${full-value} PARENT_SCOPE)
  endforeach()
endfunction()

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

dump_variable(
CMAKE_INSTALL_FULL_INCLUDEDIR
CMAKE_INSTALL_FULL_DOCDIR
)
versionize(INCLUDEDIR)
dump_variable(
CMAKE_INSTALL_FULL_INCLUDEDIR
CMAKE_INSTALL_FULL_DOCDIR
)

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

install(FILES ${CMAKE_CURRENT_BINARY_DIR}/config.h DESTINATION ${CMAKE_INSTALL_INCLUDEDIR} COMPONENT devel)

###############################################################################
# Generates config

set(SM_DOCDIR     "${CMAKE_INSTALL_DOCDIR}")
set(SM_INCLUDEDIR "${CMAKE_INSTALL_INCLUDEDIR}")
set(SM_LIBDIR     "${CMAKE_INSTALL_LIBDIR}")
configure_package_config_file(
  ${PROJECT_NAME}Config.cmake.in ${PROJECT_NAME}Config.cmake
  INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/${PROJECT_NAME}-${PROJECT_VERSION}
  PATH_VARS SM_DOCDIR SM_INCLUDEDIR SM_LIBDIR
)

install(
	FILES ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}Config.cmake
	DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/${PROJECT_NAME}-${PROJECT_VERSION}
	COMPONENT devel
)

write_basic_package_version_file(
	${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}ConfigVersion.cmake
	VERSION ${PROJECT_VERSION}
	COMPATIBILITY ExactVersion
)

install(
	FILES ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}ConfigVersion.cmake
	DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/${PROJECT_NAME}-${PROJECT_VERSION}
	COMPONENT devel
)

# CPACK section moved in the package-config directory. 
# Please see the README file inside.
add_subdirectory(cpack.d)
