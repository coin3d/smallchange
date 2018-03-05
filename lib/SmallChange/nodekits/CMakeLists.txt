get_filename_component(MODULE_NAME ${CMAKE_CURRENT_SOURCE_DIR} NAME)
message(STATUS "Configuring ${MODULE_NAME}")

# define documentation
set(SM_DOCUMENTATION_FILES ${SM_DOCUMENTATION_FILES} ${CMAKE_CURRENT_SOURCE_DIR} CACHE INTERNAL "${PROJECT_NAME} documentation files")

add_library(${MODULE_NAME} OBJECT
  bitmapfont.cpp
  DynamicBaseKit.cpp
  GeoMarkerKit.cpp
  LegendKit.cpp
  NormalsKit.cpp
  PopupMenuKit.cpp
  SmAnnotationAxis.cpp
  SmAnnotationWall.cpp
  SmAxisDisplayKit.cpp
  SmAxisKit.cpp
  SmCameraControlKit.cpp
  SmDynamicObjectKit.cpp
  SmPieChart.cpp
  SmTooltipKit.cpp
  SmTrackPointKit.cpp
  SmWellLogKit.cpp
  SoCameraPathEditKit.cpp
  SoFEMKit.cpp
)
