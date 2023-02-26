configure_file(SmallChange/basic.h.cmake.in SmallChange/basic.h @ONLY)

set(SM_HEADERS
  ${CMAKE_CURRENT_BINARY_DIR}/SmallChange/basic.h
  SmallChange/SmallChange.h
  SmallChange/actions/SmToVertexArrayShapeAction.h
  SmallChange/actions/SoGenerateSceneGraphAction.h
  SmallChange/actions/SoTweakAction.h
  SmallChange/draggers/SmRangeTranslate1Dragger.h
  SmallChange/draggers/SoAngle1Dragger.h
  SmallChange/draggers/SoAngle1Manip.h
  SmallChange/elements/GLDepthBufferElement.h
  SmallChange/elements/UTMElement.h
  SmallChange/elements/SmColorGradientElement.h # re-added
  SmallChange/elements/SmSceneryViewElement.h  # re-added
  SmallChange/engines/CubicSplineEngine.h
  SmallChange/engines/Rot2Heading.h
  SmallChange/engines/SmInverseRotation.h
  SmallChange/eventhandlers/SmEventHandler.h
  SmallChange/eventhandlers/SmExaminerEventHandler.h
  SmallChange/eventhandlers/SmHelicopterEventHandler.h
  SmallChange/eventhandlers/SmPanEventHandler.h
  SmallChange/eventhandlers/SmSphereEventHandler.h
  SmallChange/misc/cameracontrol.h
  SmallChange/misc/Init.h
  SmallChange/misc/SbBox3.h
  SmallChange/misc/SbCubicSpline.h
  SmallChange/misc/SbHash.h
  SmallChange/misc/SbList.h
  SmallChange/misc/SbPlane.h
  SmallChange/misc/SbVec3.h
  SmallChange/misc/SmEnvelope.h
  SmallChange/misc/SmHash.h
  SmallChange/misc/SmSceneManager.h
  SmallChange/misc/SceneryGlue.h # re-addded
  SmallChange/nodekits/DynamicBaseKit.h
  SmallChange/nodekits/DynamicNodeKit.h
  SmallChange/nodekits/LegendKit.h
  SmallChange/nodekits/SbList.h
  SmallChange/nodekits/SmAnnotationAxis.h
  SmallChange/nodekits/SmAnnotationWall.h
  SmallChange/nodekits/SmAxisDisplayKit.h
  SmallChange/nodekits/SmAxisKit.h
  SmallChange/nodekits/SmCameraControlKit.h
  SmallChange/nodekits/SmDynamicObjectKit.h
  SmallChange/nodekits/SmGeoMarkerKit.h
  SmallChange/nodekits/SmNormalsKit.h
  SmallChange/nodekits/SmPieChart.h
  SmallChange/nodekits/SmPopupMenuKit.h
  SmallChange/nodekits/SmTooltipKit.h
  SmallChange/nodekits/SmTrackPointKit.h
  SmallChange/nodekits/SmWellLogKit.h
  SmallChange/nodekits/SoCameraPathEditKit.h
  SmallChange/nodekits/SoFEMKit.h
  SmallChange/nodekits/SmOceanKit.h
  SmallChange/nodekits/OceanKit.cpp
  SmallChange/nodekits/SmVesselKit.h
  SmallChange/nodekits/SmVesselKit.cpp
  SmallChange/nodes/AutoFile.h
  SmallChange/nodes/Coinboard.h
  SmallChange/nodes/CoinEnvironment.h
  SmallChange/nodes/InterleavedArraysShape.h
  SmallChange/nodes/PickCallback.h
  SmallChange/nodes/PickSwitch.h
  SmallChange/nodes/ShapeScale.h
  SmallChange/nodes/SkyDome.h
  SmallChange/nodes/SmBillboardClipPlane.h
  SmallChange/nodes/SmCoordinateSystem.h
  SmallChange/nodes/SmDepthBuffer.h
  SmallChange/nodes/SmHeadlight.h
  SmallChange/nodes/SmHQSphere.h
  SmallChange/nodes/SmLazyFile.h
  SmallChange/nodes/SmMarkerSet.h
  SmallChange/nodes/SmShadowText2.h
  SmallChange/nodes/SmSwitchboard.h
  SmallChange/nodes/SmSwitchboardOperator.h
  SmallChange/nodes/SmTextureFont.h
  SmallChange/nodes/SmTextureText2.h
  SmallChange/nodes/SmTextureText2Collector.h
  SmallChange/nodes/SmTooltip.h
  SmallChange/nodes/SmTrack.h
  SmallChange/nodes/SmVertexArrayShape.h
  SmallChange/nodes/SmViewpointWrapper.h
  SmallChange/nodes/SoLODExtrusion.h
  SmallChange/nodes/SoPointCloud.h
  SmallChange/nodes/SoTCBCurve.h
  SmallChange/nodes/SoText2Set.h
  SmallChange/nodes/UTMCamera.h
  SmallChange/nodes/UTMCoordinate.h
  SmallChange/nodes/UTMPosition.h
  SmallChange/nodes/ViewportRegion.h
  SmallChange/nodes/SceneryGL.h  # re-added
  SmallChange/nodes/SmColorGradient.h # re-added
  SmallChange/nodes/SmScenery.h # re-added
  SmallChange/nodes/SmSceneryView.h # re-added
        )
set(SM_SOURCES
  SmallChange/dummy.cpp
  SmallChange/actions/SoGenerateSceneGraphAction.cpp
  SmallChange/actions/SoTweakAction.cpp
  SmallChange/actions/ToVertexArrayShapeAction.cpp
  SmallChange/draggers/SmRangeTranslate1Dragger.cpp
  SmallChange/draggers/SoAngle1Dragger.cpp
  SmallChange/draggers/SoAngle1Manip.cpp
  SmallChange/elements/GLDepthBufferElement.cpp
  SmallChange/elements/UTMElement.cpp
  SmallChange/elements/ColorGradientElement.cpp # re-added
  SmallChange/elements/SmSceneryViewElement.cpp # re-added
  SmallChange/engines/CubicSplineEngine.cpp
  SmallChange/engines/Rot2Heading.cpp
  SmallChange/engines/SmInverseRotation.cpp
  SmallChange/eventhandlers/SmEventHandler.cpp
  SmallChange/eventhandlers/SmExaminerEventHandler.cpp
  SmallChange/eventhandlers/SmHelicopterEventHandler.cpp
  SmallChange/eventhandlers/SmPanEventHandler.cpp
  SmallChange/eventhandlers/SmSphereEventHandler.cpp
  SmallChange/misc/cameracontrol.cpp
  SmallChange/misc/Envelope.cpp
  SmallChange/misc/Init.cpp
  SmallChange/misc/SbCubicSpline.cpp
  SmallChange/misc/SceneManager.cpp
  SmallChange/misc/SceneryGlue.cpp
  SmallChange/nodekits/bitmapfont.cpp
  SmallChange/nodekits/DynamicBaseKit.cpp
  SmallChange/nodekits/GeoMarkerKit.cpp
  SmallChange/nodekits/LegendKit.cpp
  SmallChange/nodekits/NormalsKit.cpp
  SmallChange/nodekits/PopupMenuKit.cpp
  SmallChange/nodekits/SmAnnotationAxis.cpp
  SmallChange/nodekits/SmAnnotationWall.cpp
  SmallChange/nodekits/SmAxisDisplayKit.cpp
  SmallChange/nodekits/SmAxisKit.cpp
  SmallChange/nodekits/SmCameraControlKit.cpp
  SmallChange/nodekits/SmDynamicObjectKit.cpp
  SmallChange/nodekits/SmPieChart.cpp
  SmallChange/nodekits/SmTooltipKit.cpp
  SmallChange/nodekits/SmTrackPointKit.cpp
  SmallChange/nodekits/SmWellLogKit.cpp
  SmallChange/nodekits/SoCameraPathEditKit.cpp
  SmallChange/nodekits/SoFEMKit.cpp
  SmallChange/nodes/AutoFile.cpp
  SmallChange/nodes/Coinboard.cpp
  SmallChange/nodes/CoinEnvironment.cpp
  SmallChange/nodes/InterleavedArraysShape.cpp
  SmallChange/nodes/PickCallback.cpp
  SmallChange/nodes/PickSwitch.cpp
  SmallChange/nodes/SceneryView.cpp  # re-added
  SmallChange/nodes/ColorGradient.cpp  # re-added
  SmallChange/nodes/Scenery.cpp  # re-added
  SmallChange/nodes/SceneryGL.cpp # re-added
  SmallChange/nodes/ShapeScale.cpp
  SmallChange/nodes/SkyDome.cpp
  SmallChange/nodes/SmBillboardClipPlane.cpp
  SmallChange/nodes/SmCoordinateSystem.cpp
  SmallChange/nodes/SmDepthBuffer.cpp
  SmallChange/nodes/SmHeadlight.cpp
  SmallChange/nodes/SmHQSphere.cpp
  SmallChange/nodes/SmLazyFile.cpp
  SmallChange/nodes/SmMarkerSet.cpp
  SmallChange/nodes/SmShadowText2.cpp
  SmallChange/nodes/SmTextureFont.cpp
  SmallChange/nodes/SmTextureText2.cpp
  SmallChange/nodes/SmTextureText2Collector.cpp
  SmallChange/nodes/SmTooltip.cpp
  SmallChange/nodes/SmTrack.cpp
  SmallChange/nodes/SoLODExtrusion.cpp
  SmallChange/nodes/SoPointCloud.cpp
  SmallChange/nodes/SoTCBCurve.cpp
  SmallChange/nodes/SoText2Set.cpp
  SmallChange/nodes/Switchboard.cpp
  SmallChange/nodes/SwitchboardOperator.cpp
  SmallChange/nodes/UTMCamera.cpp
  SmallChange/nodes/UTMCoordinate.cpp
  SmallChange/nodes/UTMPosition.cpp
  SmallChange/nodes/VertexArrayShape.cpp
  SmallChange/nodes/ViewpointWrapper.cpp
  SmallChange/nodes/ViewportRegion.cpp
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

# define additional documentation
set(SM_DOCUMENTATION_FILES ${SM_DOCUMENTATION_FILES}
  ${CMAKE_CURRENT_SOURCE_DIR}/SmallChange
  ${CMAKE_CURRENT_SOURCE_DIR}/SmallChange/actions
  ${CMAKE_CURRENT_SOURCE_DIR}/SmallChange/draggers
  ${CMAKE_CURRENT_SOURCE_DIR}/SmallChange/elements
  ${CMAKE_CURRENT_SOURCE_DIR}/SmallChange/engines
  ${CMAKE_CURRENT_SOURCE_DIR}/SmallChange/eventhandlers
  ${CMAKE_CURRENT_SOURCE_DIR}/SmallChange/misc
  ${CMAKE_CURRENT_SOURCE_DIR}/SmallChange/nodekits
  ${CMAKE_CURRENT_SOURCE_DIR}/SmallChange/nodes
  CACHE INTERNAL "${PROJECT_NAME} documentation files")

if(SMALLCHANGE_BUILD_SHARED_LIBS)
  add_library(${PROJECT_NAME} SHARED ${SM_SOURCES} ${SM_HEADERS})
else()
  add_library(${PROJECT_NAME} STATIC ${SM_SOURCES} ${SM_HEADERS})
endif()

target_compile_definitions(${PROJECT_NAME} PRIVATE HAVE_CONFIG_H SMALLCHANGE_INTERNAL SMALLCHANGE_DEBUG=$<CONFIG:Debug>)
if(WIN32)
  if(MSVC)
    target_compile_definitions(${PROJECT_NAME} PRIVATE _CRT_NONSTDC_NO_DEPRECATE _CRT_SECURE_NO_DEPRECATE _CRT_SECURE_NO_WARNINGS _USE_MATH_DEFINES)
  endif()
  if(SMALLCHANGE_BUILD_SHARED_LIBS)
    configure_file(SmallChange/${PROJECT_NAME}.rc.cmake.in ${PROJECT_NAME}.rc)
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

target_include_directories(${PROJECT_NAME}
PUBLIC
  $<BUILD_INTERFACE:${CMAKE_BINARY_DIR}>
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
  $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}>
  $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)

if(WIN32 AND NOT MINGW)
  set_target_properties(${PROJECT_NAME} PROPERTIES VERSION ${PROJECT_VERSION} SOVERSION ${SMALLCHANGE_SO_VERSION})
else()
  set_target_properties(${PROJECT_NAME} PROPERTIES VERSION "${SMALLCHANGE_SO_VERSION}.${PROJECT_VERSION_MINOR}.${PROJECT_VERSION_PATCH}" SOVERSION ${SMALLCHANGE_SO_VERSION})
endif()

target_link_libraries(${PROJECT_NAME} Coin::Coin ${OPENGL_LIBRARIES})

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

install(DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/SmallChange/
  DESTINATION ${CMAKE_INSTALL_INCLUDEDIR} 
  COMPONENT development
  FILES_MATCHING PATTERN "*.h"
)
install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/SmallChange/
  DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
  COMPONENT development
  FILES_MATCHING PATTERN "*.h"
)
