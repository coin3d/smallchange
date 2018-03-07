get_filename_component(MODULE_NAME ${CMAKE_CURRENT_SOURCE_DIR} NAME)
message(STATUS "Configuring ${MODULE_NAME}")

# define documentation
set(SM_DOCUMENTATION_FILES ${SM_DOCUMENTATION_FILES} ${CMAKE_CURRENT_SOURCE_DIR} CACHE INTERNAL "${PROJECT_NAME} documentation files")

add_library(${MODULE_NAME} OBJECT
SoGenerateSceneGraphAction.cpp
SoTweakAction.cpp
ToVertexArrayShapeAction.cpp
)

install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/ DESTINATION ${CMAKE_INSTALL_INCLUDEDIR} COMPONENT devel FILES_MATCHING PATTERN "*.h")
