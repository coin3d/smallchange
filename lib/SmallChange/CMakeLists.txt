macro(build_components)
  foreach(_module ${ARGN})
    add_subdirectory(${_module})
    list(APPEND OBJS "$<TARGET_OBJECTS:${_module}>")
    install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/${_module}/ DESTINATION ${CMAKE_INSTALL_INCLUDEDIR} COMPONENT devel FILES_MATCHING PATTERN "*.h")
  endforeach()
endmacro()

set(LIBNAME "SmallChange")

if (NOT SmallChange_BUILD_STATIC_LIB)
  set(CMAKE_POSITION_INDEPENDENT_CODE ON)
endif(NOT SmallChange_BUILD_STATIC_LIB)

configure_file(basic.h.cmake.in basic.h @ONLY)
     
build_components(
  actions
  draggers
  elements
  engines
  eventhandlers
  misc
  nodekits
  nodes
)

if(SmallChange_BUILD_STATIC_LIB)
  message(STATUS "Building STATIC library")
  add_library(${LIBNAME} STATIC
    ${OBJS}
  )
else(SmallChange_BUILD_STATIC_LIB)
  message(STATUS "Building Dynamic Shared Object library")
  add_library(${LIBNAME} SHARED
    ${OBJS}
  )
  set_target_properties(${LIBNAME} PROPERTIES 
    VERSION   ${PROJECT_VERSION}
    SOVERSION ${PROJECT_VERSION_MAJOR}
  )
endif(SmallChange_BUILD_STATIC_LIB)

###############################################################################
# installation part

install(TARGETS ${LIBNAME}
  ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR} COMPONENT libs
  LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR} COMPONENT libs
)
