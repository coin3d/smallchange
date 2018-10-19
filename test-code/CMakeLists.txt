# tests performed on SoQt only
set(Gui "Qt")

configure_file(cameracontrol.cpp.in cameracontrol.cpp @ONLY)
add_executable(cameracontrol ${CMAKE_CURRENT_BINARY_DIR}/cameracontrol.cpp)
target_link_libraries(cameracontrol SoQt::SoQt SmallChange)

add_executable(envelope envelope.cpp)
target_link_libraries(envelope SmallChange)

add_executable(femtest femtest.cpp)
target_link_libraries(femtest SoQt::SoQt SmallChange)

add_executable(iv2scenegraph iv2scenegraph.cpp)
target_link_libraries(iv2scenegraph SoQt::SoQt SmallChange)

add_executable(legendtest legendtest.cpp)
target_link_libraries(legendtest SoQt::SoQt SmallChange)

add_executable(lodexttest lodexttest.cpp)
target_link_libraries(lodexttest SoQt::SoQt SmallChange)

configure_file(scenemgr.cpp.in scenemgr.cpp @ONLY)
add_executable(scenemgr ${CMAKE_CURRENT_BINARY_DIR}/scenemgr.cpp)
target_link_libraries(scenemgr SoQt::SoQt SmallChange)

add_executable(sotext2set sotext2set.cpp)
target_link_libraries(sotext2set SoQt::SoQt SmallChange)

add_executable(texturetext2 texturetext2.cpp)
target_link_libraries(texturetext2 SmallChange)

configure_file(tooltip.cpp.in tooltip.cpp @ONLY)
add_executable(tooltip ${CMAKE_CURRENT_BINARY_DIR}/tooltip.cpp)
target_link_libraries(tooltip SoQt::SoQt SmallChange)

add_executable(tovertexarray tovertexarray.cpp)
target_link_libraries(tovertexarray SmallChange)

