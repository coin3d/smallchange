
if(SMALLCHANGE_USE_WX)
    set(Gui Wx)
    set(WIDGET wxWindow*)
    set(TARGET_LIB SoWx::SoWx)
else()
    set(Gui Qt)
    set(WIDGET QWidget*)
    set(TARGET_LIB SoQt::SoQt)
endif()

set(GUI_EXAMPLES
    cameracontrol
    femtest
    frustum
    legendtest
    lodexttest
    scenemgr
    scenery
    sc_viewer
    sotext2set
    tcbcurve
    tooltip
)

foreach(EXAMPLE ${GUI_EXAMPLES} )
    configure_file(${EXAMPLE}.cpp.in ${EXAMPLE}.cpp @ONLY)
    add_executable(${EXAMPLE} ${CMAKE_CURRENT_BINARY_DIR}/${EXAMPLE}.cpp)
    target_link_libraries(${EXAMPLE} ${TARGET_LIB} SmallChange)
endforeach()

set(NO_GUI_EXAMPLES
    envelope
    iv2scenegraph
    texturetext2
    tovertexarray
)

foreach(EXAMPLE ${NO_GUI_EXAMPLES} )
    add_executable(${EXAMPLE} ${EXAMPLE}.cpp)
    target_link_libraries(${EXAMPLE} ${TARGET_LIB} SmallChange)
endforeach()
