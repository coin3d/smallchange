configure_file(basic.h.cmake.in basic.h @ONLY)

set(SM_BUILD_COMPONENTS
  actions
  draggers
  elements
  engines
  eventhandlers
  misc
  nodekits
  nodes
)

if(WIN32)
  if(MINGW)
    set(SMALLCHANGE_DEFAULT_SHARED_POSTFIX "")
    set(SMALLCHANGE_DEFAULT_STATIC_POSTFIX "")
  else()
    # On Windows the major version number is part of the library name
    set(SMALLCHANGE_DEFAULT_SHARED_POSTFIX ${PROJECT_VERSION_MAJOR})
    set(SMALLCHANGE_DEFAULT_STATIC_POSTFIX ${PROJECT_VERSION_MAJOR}s)
  endif()
  if(SMALLCHANGE_BUILD_SHARED_LIBS)
    set(SMALLCHANGE_DEFAULT_POSTFIX ${SMALLCHANGE_DEFAULT_SHARED_POSTFIX})
    set(SMALLCHANGE_PKG_FLAGS "-DSMALLCHANGE_DLL" CACHE INTERNAL "Compilation flags for package config" FORCE)
  else()
    set(SMALLCHANGE_DEFAULT_POSTFIX ${SMALLCHANGE_DEFAULT_STATIC_POSTFIX})
    set(SMALLCHANGE_PKG_FLAGS "-DSMALLCHANGE_NOT_DLL" CACHE INTERNAL "Compilation flags for package config" FORCE)
  endif()
  set(CMAKE_RELEASE_POSTFIX ${SMALLCHANGE_DEFAULT_POSTFIX})
  set(CMAKE_MINSIZEREL_POSTFIX ${SMALLCHANGE_DEFAULT_POSTFIX})
  set(CMAKE_RELWITHDEBINFO_POSTFIX ${SMALLCHANGE_DEFAULT_POSTFIX})
  set(CMAKE_DEBUG_POSTFIX ${SMALLCHANGE_DEFAULT_POSTFIX}d)
elseif(APPLE)
else()
  set(CMAKE_POSITION_INDEPENDENT_CODE ON)
endif()
set(SMALLCHANGE_PKG_LIBS "-l${PROJECT_NAME}" CACHE INTERNAL "Link libraries for package config" FORCE)

foreach(_module ${SM_BUILD_COMPONENTS})
  add_subdirectory(${_module})
  target_compile_definitions(${_module} PRIVATE HAVE_CONFIG_H SMALLCHANGE_INTERNAL SMALLCHANGE_DEBUG=$<CONFIG:Debug>)
  if(WIN32)
    if(MSVC)
      target_compile_definitions(${_module} PRIVATE _CRT_NONSTDC_NO_DEPRECATE _CRT_SECURE_NO_DEPRECATE _CRT_SECURE_NO_WARNINGS _USE_MATH_DEFINES)
    endif()
    if(SMALLCHANGE_BUILD_SHARED_LIBS)
      target_compile_definitions(${_module} PRIVATE SMALLCHANGE_MAKE_DLL INTERFACE SMALLCHANGE_DLL)
    else()
      target_compile_definitions(${_module} PRIVATE INTERFACE SMALLCHANGE_NOT_DLL)
      if(MSVC)
        set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /Z7")                       # Override default /Zi to embed
        set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "${CMAKE_CXX_FLAGS_RELWITHDEBINFO} /Z7")     # debugging info in the .lib.
      endif()
    endif()
  endif()
  list(APPEND OBJS $<TARGET_OBJECTS:${_module}>)
endforeach()

if(SMALLCHANGE_BUILD_SHARED_LIBS)
  add_library(${PROJECT_NAME} SHARED ${OBJS})
else()
  add_library(${PROJECT_NAME} STATIC ${OBJS})
endif()

if(WIN32)
  if(MSVC)
    target_compile_definitions(${PROJECT_NAME} PRIVATE _CRT_NONSTDC_NO_DEPRECATE _CRT_SECURE_NO_DEPRECATE _CRT_SECURE_NO_WARNINGS _USE_MATH_DEFINES)
  endif()
  if(SMALLCHANGE_BUILD_SHARED_LIBS)
    configure_file(${PROJECT_NAME}.rc.cmake.in ${PROJECT_NAME}.rc)
    target_sources(${PROJECT_NAME} PRIVATE "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}.rc")
    target_compile_definitions(${PROJECT_NAME} PRIVATE SMALLCHANGE_MAKE_DLL INTERFACE SMALLCHANGE_DLL)
  else()
    target_compile_definitions(${PROJECT_NAME} INTERFACE SMALLCHANGE_NOT_DLL)
    if(MSVC)
      set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /Z7")                       # Override default /Zi to embed
      set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "${CMAKE_CXX_FLAGS_RELWITHDEBINFO} /Z7")     # debugging info in the .lib.
    endif()
  endif()
endif()

if(WIN32 AND NOT MINGW)
  set_target_properties(${PROJECT_NAME} PROPERTIES VERSION ${PROJECT_VERSION} SOVERSION ${SMALLCHANGE_SO_VERSION})
else()
  set_target_properties(${PROJECT_NAME} PROPERTIES VERSION "${SMALLCHANGE_SO_VERSION}.${PROJECT_VERSION_MINOR}.${PROJECT_VERSION_PATCH}" SOVERSION ${SMALLCHANGE_SO_VERSION})
endif()

target_link_libraries(${PROJECT_NAME} Coin::Coin ${OPENGL_LIBRARY})

# ############################################################################
# Install targets
# ############################################################################

if(NOT CMAKE_VERSION VERSION_LESS 3.12)
  install(TARGETS ${PROJECT_NAME} EXPORT ${PROJECT_NAME}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR} COMPONENT development
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR} COMPONENT runtime NAMELINK_COMPONENT development
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR} COMPONENT runtime
  )
  
  if(SMALLCHANGE_BUILD_SHARED_LIBS)
    if(MSVC)
      install(FILES $<TARGET_PDB_FILE:${PROJECT_NAME}> DESTINATION ${CMAKE_INSTALL_BINDIR} CONFIGURATIONS Debug RelWithDebInfo COMPONENT debug)
    endif()
  endif()
else()
  install(TARGETS ${PROJECT_NAME} EXPORT ${PROJECT_NAME}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR} COMPONENT development
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR} COMPONENT runtime NAMELINK_SKIP
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR} COMPONENT runtime
  )
  
  if(SMALLCHANGE_BUILD_SHARED_LIBS)
    install(TARGETS ${PROJECT_NAME} EXPORT ${PROJECT_NAME} LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR} COMPONENT development NAMELINK_ONLY)
    if(MSVC)
      install(FILES $<TARGET_PDB_FILE:${PROJECT_NAME}> DESTINATION ${CMAKE_INSTALL_BINDIR} CONFIGURATIONS Debug RelWithDebInfo COMPONENT debug)
    endif()
  endif()
endif()

install(FILES "${CMAKE_CURRENT_BINARY_DIR}/basic.h" DESTINATION ${CMAKE_INSTALL_INCLUDEDIR} COMPONENT development)

foreach(_module ${SM_BUILD_COMPONENTS})
  install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/${_module} DESTINATION ${CMAKE_INSTALL_INCLUDEDIR} COMPONENT development FILES_MATCHING PATTERN "*.h")
endforeach()
