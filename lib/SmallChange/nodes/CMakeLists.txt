get_filename_component(MODULE_NAME ${CMAKE_CURRENT_SOURCE_DIR} NAME)
message(STATUS "Configuring ${MODULE_NAME}")

# define documentation
set(SM_DOCUMENTATION_FILES ${SM_DOCUMENTATION_FILES} ${CMAKE_CURRENT_SOURCE_DIR} CACHE INTERNAL "${PROJECT_NAME} documentation files")

add_library(${MODULE_NAME} OBJECT 
  AutoFile.cpp
  Coinboard.cpp
  CoinEnvironment.cpp
  InterleavedArraysShape.cpp
  PickCallback.cpp
  PickSwitch.cpp
#   SceneryView.cpp
  ShapeScale.cpp
  SkyDome.cpp
  SmBillboardClipPlane.cpp
  SmCoordinateSystem.cpp
  SmDepthBuffer.cpp
  SmHeadlight.cpp
  SmHQSphere.cpp
  SmLazyFile.cpp
  SmMarkerSet.cpp
  SmShadowText2.cpp
  SmTextureFont.cpp
  SmTextureText2.cpp
  SmTextureText2Collector.cpp
  SmTooltip.cpp
  SmTrack.cpp
  SoLODExtrusion.cpp
  SoPointCloud.cpp
  SoTCBCurve.cpp
  SoText2Set.cpp
  Switchboard.cpp
  SwitchboardOperator.cpp
  UTMCamera.cpp
  UTMCoordinate.cpp
  UTMPosition.cpp
  VertexArrayShape.cpp
  ViewpointWrapper.cpp
  ViewportRegion.cpp
)
