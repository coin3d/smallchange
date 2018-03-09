cmake_minimum_required(VERSION 3.10.1)
project(SmallChange 
  VERSION 1.0.0
  DESCRIPTION "A collection of extensions to the Open Inventor API."
  LANGUAGES CXX
)

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

configure_package_config_file(
  ${PROJECT_NAME}Config.cmake.in ${PROJECT_NAME}Config.cmake
  INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/${PROJECT_NAME}-${PROJECT_VERSION}
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

###############################################################################
# CPACK (defaults as commented set() statements)

# commons
###############################################################################
# set(CPACK_PACKAGE_NAME              "${PROJECT_NAME}")
# set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "${PROJECT_DESCRIPTION}")
set(CPACK_PACKAGE_VERSION_MAJOR     "${PROJECT_VERSION_MAJOR}")
set(CPACK_PACKAGE_VERSION_MINOR     "${PROJECT_VERSION_MINOR}")
set(CPACK_PACKAGE_VERSION_PATCH     "${PROJECT_VERSION_PATCH}")
# set(CPACK_PACKAGE_VERSION           "${CPACK_PACKAGE_VERSION_MAJOR}.${CPACK_PACKAGE_VERSION_MINOR}.${CPACK_PACKAGE_VERSION_PATCH}")
set(CPACK_RESOURCE_FILE_LICENSE     "${CMAKE_CURRENT_SOURCE_DIR}/COPYING")
set(CPACK_PACKAGE_DESCRIPTION_FILE  "${CMAKE_CURRENT_SOURCE_DIR}/README.md")
set(CPACK_RESOURCE_FILE_LICENSE     "${CMAKE_CURRENT_SOURCE_DIR}/COPYING")

# components
###############################################################################
set(CPACK_COMPONENT_DEVEL_DEPENDS libs)

# Debian
###############################################################################
# set(CPACK_DEB_COMPONENT_INSTALL ON)
# set(CPACK_DEBIAN_DOCUMENTATION_PACKAGE_NAME "libcoin80-doc")
# set(CPACK_DEBIAN_DOCUMENTATION_PACKAGE_SECTION "doc")
# set(CPACK_DEBIAN_DEVEL_PACKAGE_DEPENDS "libcoin80")
# set(CPACK_DEBIAN_DEVEL_PACKAGE_NAME "libcoin80-dev")
# set(CPACK_DEBIAN_DEVEL_PACKAGE_SECTION "libdevel")
# set(CPACK_DEBIAN_PACKAGE_HOMEPAGE "https://bitbucket.org/Coin3D/coin")
# set(CPACK_DEBIAN_PACKAGE_NAME "libcoin80")
# set(CPACK_DEBIAN_PACKAGE_SECTION "devel")
# set(CPACK_DEBIAN_RUNTIME_PACKAGE_NAME "libcoin80-dev")
# set(CPACK_DEBIAN_RUNTIME_PACKAGE_SECTION "libs")

# Windows
###############################################################################
set(CPACK_PACKAGE_INSTALL_DIRECTORY "${PROJECT_NAME}-${PROJECT_VERSION}")
set(CPACK_NSIS_PACKAGE_NAME         "${PROJECT_NAME} ${PROJECT_VERSION}")
set(CPACK_NSIS_URL_INFO_ABOUT       ${CPACK_DEBIAN_PACKAGE_HOMEPAGE})

# RPM
###############################################################################
# set(CPACK_RPM_PACKAGE_NAME           ${CPACK_PACKAGE_NAME})
set(CPACK_RPM_COMPONENT_INSTALL ON)
set(CPACK_RPM_PACKAGE_GROUP          "Development/Libraries")
set(CPACK_RPM_PACKAGE_LICENSE        "3-clause BSD")
set(CPACK_RPM_MAIN_COMPONENT         "libs")
set(CPACK_RPM_PACKAGE_URL            "https://bitbucket.org/Coin3D/smallchange")
set(CPACK_RPM_LIBS_PACKAGE_PROVIDES  "${PROJECT_NAME}")
set(CPACK_RPM_DEVEL_PACKAGE_REQUIRES "${PROJECT_NAME}")
# workaround for erroneous inclusion of system directories
# see https://stackoverflow.com/questions/24983181/cpack-generates-rpms-with-files-entries-that-conflict-with-the-rpm-spec-how-to
set(CPACK_RPM_EXCLUDE_FROM_AUTO_FILELIST_ADDITION "/usr/lib64/cmake")

if(WIN32 AND CMAKE_SIZEOF_VOID_P EQUAL 8)
	set(CPACK_NSIS_INSTALL_ROOT "$PROGRAMFILES64")
endif()

include(CPack)

dump_variable(
CPACK_PACKAGE_NAME
CPACK_PACKAGE_VERSION
CPACK_RPM_PACKAGE_NAME
)
