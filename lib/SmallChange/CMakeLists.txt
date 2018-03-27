macro(build_components)
  foreach(_module ${ARGN})
    add_subdirectory(${_module})
    target_compile_definitions(${_module} PRIVATE -DHAVE_CONFIG_H)
    list(APPEND OBJS "$<TARGET_OBJECTS:${_module}>")
    install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/${_module} DESTINATION ${CMAKE_INSTALL_INCLUDEDIR} COMPONENT devel FILES_MATCHING PATTERN "*.h")
  endforeach()
endmacro()

if (NOT SmallChange_BUILD_STATIC_LIB)
  set(CMAKE_POSITION_INDEPENDENT_CODE ON)
endif(NOT SmallChange_BUILD_STATIC_LIB)

configure_file(basic.h.cmake.in basic.h @ONLY)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/basic.h DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}" COMPONENT devel)
     
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
  add_library(${PROJECT_NAME} STATIC
    ${OBJS}
  )
else(SmallChange_BUILD_STATIC_LIB)
  message(STATUS "Building Dynamic Shared Object library")
  add_library(${PROJECT_NAME} SHARED
    ${OBJS}
  )
  set_target_properties(${PROJECT_NAME} PROPERTIES 
    VERSION   ${PROJECT_VERSION}
    SOVERSION ${PROJECT_VERSION_MAJOR}
  )
endif(SmallChange_BUILD_STATIC_LIB)

###############################################################################
# installation part

install(TARGETS ${PROJECT_NAME}
  ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR} COMPONENT libs
  LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR} COMPONENT libs
)
