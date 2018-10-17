# tests performed on SoQt only
set(Gui "Qt")

configure_file(cameracontrol.cpp.in cameracontrol.cpp @ONLY)
add_executable(cameracontrol ${CMAKE_CURRENT_BINARY_DIR}/cameracontrol.cpp)
target_link_libraries(cameracontrol SmallChange Coin::Coin SoQt::SoQt)

add_executable(envelope envelope.cpp)
target_link_libraries(envelope SmallChange Coin::Coin)

add_executable(femtest femtest.cpp)
target_link_libraries(femtest SmallChange Coin::Coin SoQt::SoQt)

add_executable(iv2scenegraph iv2scenegraph.cpp)
target_link_libraries(iv2scenegraph SmallChange Coin::Coin SoQt::SoQt)

add_executable(legendtest legendtest.cpp)
target_link_libraries(legendtest SmallChange Coin::Coin SoQt::SoQt)

add_executable(lodexttest lodexttest.cpp)
target_link_libraries(lodexttest SmallChange Coin::Coin SoQt::SoQt)

configure_file(scenemgr.cpp.in scenemgr.cpp @ONLY)
add_executable(scenemgr ${CMAKE_CURRENT_BINARY_DIR}/scenemgr.cpp)
target_link_libraries(scenemgr SmallChange Coin::Coin SoQt::SoQt)

add_executable(sotext2set sotext2set.cpp)
target_link_libraries(sotext2set SmallChange Coin::Coin SoQt::SoQt)

add_executable(texturetext2 texturetext2.cpp)
target_link_libraries(texturetext2 SmallChange Coin::Coin)

configure_file(tooltip.cpp.in tooltip.cpp @ONLY)
add_executable(tooltip ${CMAKE_CURRENT_BINARY_DIR}/tooltip.cpp)
target_link_libraries(tooltip SmallChange Coin::Coin SoQt::SoQt)

add_executable(tovertexarray tovertexarray.cpp)
target_link_libraries(tovertexarray SmallChange Coin::Coin)

