
if(SMALLCHANGE_USE_WX)
    set(Gui Wx)
    set(WIDGET wxWindow*)
    set(TARGET_LIB SoWx::SoWx)
else()
    set(Gui Qt)
    set(WIDGET QWidget*)
    set(TARGET_LIB SoQt::SoQt)
endif()

configure_file(cameracontrol.cpp.in cameracontrol.cpp @ONLY)
add_executable(cameracontrol ${CMAKE_CURRENT_BINARY_DIR}/cameracontrol.cpp)
target_link_libraries(cameracontrol ${TARGET_LIB} SmallChange)

configure_file(femtest.cpp.in femtest.cpp @ONLY)
add_executable(femtest ${CMAKE_CURRENT_BINARY_DIR}/femtest.cpp)
target_link_libraries(femtest ${TARGET_LIB} SmallChange)

#configure_file(frustum.cpp.in frustum.cpp @ONLY)
#add_executable(frustum ${CMAKE_CURRENT_BINARY_DIR}/frustum.cpp)
#target_link_libraries(frustum ${TARGET_LIB} SmallChange)

configure_file(legendtest.cpp.in legendtest.cpp @ONLY)
add_executable(legendtest ${CMAKE_CURRENT_BINARY_DIR}/legendtest.cpp)
target_link_libraries(legendtest ${TARGET_LIB} SmallChange)

configure_file(lodexttest.cpp.in lodexttest.cpp @ONLY)
add_executable(lodexttest ${CMAKE_CURRENT_BINARY_DIR}/lodexttest.cpp)
target_link_libraries(lodexttest ${TARGET_LIB} SmallChange)

configure_file(scenemgr.cpp.in scenemgr.cpp @ONLY)
add_executable(scenemgr ${CMAKE_CURRENT_BINARY_DIR}/scenemgr.cpp)
target_link_libraries(scenemgr ${TARGET_LIB} SmallChange)

#configure_file(scenery.cpp.in scenery.cpp @ONLY)
#add_executable(scenery ${CMAKE_CURRENT_BINARY_DIR}/scenery.cpp)
#target_link_libraries(scenery ${TARGET_LIB} SmallChange)

add_executable(sc2iv sc2iv.cpp)
target_link_libraries(sc2iv ${TARGET_LIB} SmallChange)

configure_file(sotext2set.cpp.in sotext2set.cpp @ONLY)
add_executable(sotext2set ${CMAKE_CURRENT_BINARY_DIR}/sotext2set.cpp)
target_link_libraries(sotext2set ${TARGET_LIB} SmallChange)

configure_file(sc_viewer.cpp.in sc_viewer.cpp @ONLY)
add_executable(sc_viewer ${CMAKE_CURRENT_BINARY_DIR}/sc_viewer.cpp)
target_link_libraries(sc_viewer ${TARGET_LIB} SmallChange)

add_executable(vessel vessel.cpp)
target_link_libraries(vessel ${TARGET_LIB} SmallChange)

configure_file(tooltip.cpp.in tooltip.cpp @ONLY)
add_executable(tooltip ${CMAKE_CURRENT_BINARY_DIR}/tooltip.cpp)
target_link_libraries(tooltip ${TARGET_LIB} SmallChange)

add_executable(envelope envelope.cpp)
target_link_libraries(envelope SmallChange)

add_executable(iv2scenegraph iv2scenegraph.cpp)
target_link_libraries(iv2scenegraph SmallChange)

add_executable(texturetext2 texturetext2.cpp)
target_link_libraries(texturetext2 SmallChange)

add_executable(tovertexarray tovertexarray.cpp)
target_link_libraries(tovertexarray SmallChange)
