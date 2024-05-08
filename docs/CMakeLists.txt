file(GLOB_RECURSE DOX "*.dox*")

set(SM_DOCUMENTATION_FILES ${CMAKE_SOURCE_DIR}/README.md ${DOX} ${SM_DOCUMENTATION_FILES} CACHE INTERNAL "${PROJECT_NAME} documentation files")

# Add a target to generate API documentation with Doxygen
if(SMALLCHANGE_BUILD_DOCUMENTATION)
  find_package(Doxygen)
  if(NOT DOXYGEN_FOUND)
    message(FATAL_ERROR "Doxygen is needed to build the documentation.")
  endif()

  if(NOT "${Coin_DOC_DIR}" STREQUAL "")
    get_filename_component(_coin_versioned_dir ${Coin_DOC_DIR} NAME)
    set(DOXYGEN_TAGFILES "${Coin_DOC_DIR}/html/Coin.tag=../../${_coin_versioned_dir}/html")
  endif()

# ############################################################################
# Setup documentation options
# ############################################################################
  set(GENERATE_HTMLHELP NO)
  set(DOXYGEN_GENERATE_MAN NO)
  set(GENERATE_QHP NO)
  set(GENERATE_TREEVIEW YES)
  set(HHC_PROGRAM)
  if(WIN32)
    if(SMALLCHANGE_BUILD_DOC_CHM)
      find_program(HHC_PROGRAM NAMES hhc.exe PATHS "C:/Program Files/HTML Help Workshop" "C:/Program Files (x86)/HTML Help Workshop" DOC "HTML Help Compiler program")
      if(NOT HHC_PROGRAM)
        message(FATAL_ERROR "Missing program HTML Help Compiler")
      else()
        set(GENERATE_HTMLHELP YES)
        set(GENERATE_TREEVIEW NO)
        mark_as_advanced(HHC_PROGRAM)
      endif()
    endif()
  else()
    if(SMALLCHANGE_BUILD_DOC_MAN)
      set(DOXYGEN_GENERATE_MAN YES)
    endif()
  endif()
  if(SMALLCHANGE_BUILD_DOC_QTHELP)
    find_program(QHG_LOCATION NAMES qhelpgenerator qhelpgenerator-qt5 DOC "Qt qhelpgenerator")
    if(NOT QHG_LOCATION)
      message(FATAL_ERROR "Missing program Qt qhelpgenerator")
    else()
      set(GENERATE_QHP YES)
      mark_as_advanced(QHG_LOCATION)
    endif()
  endif()

  set(DOXYGEN_INTERNAL_DOCS NO)
  set(DOXYGEN_EXTRACT_PRIVATE NO)
  set(DOXYGEN_WARN_IF_UNDOCUMENTED YES)
  set(DOXYGEN_EXCLUDE)
  if(SMALLCHANGE_BUILD_INTERNAL_DOCUMENTATION)
    set(DOXYGEN_EXTRACT_PRIVATE YES)
    set(DOXYGEN_WARN_IF_UNDOCUMENTED NO)
    set(DOXYGEN_INTERNAL_DOCS YES)
    set(SM_DOCUMENTATION_FILES
      ${SM_DOCUMENTATION_FILES}
      ${SM_INTERNAL_DOCUMENTATION_FILES}
      CACHE INTERNAL "SM_DOCUMENTATION_FILES"
    )
  else()
    set(SM_DOCUMENTATION_EXCLUDE_SYMBOLS SoProfilerP)
    foreach(XF ${SM_INTERNAL_DOCUMENTATION_FILES})
      set(DOXYGEN_EXCLUDE "${DOXYGEN_EXCLUDE}                         ${XF} \\\n")
    endforeach(XF)
  endif()
  set(DOXYGEN_INPUT)
  list(REMOVE_DUPLICATES SM_DOCUMENTATION_FILES)
  foreach(XF ${SM_DOCUMENTATION_FILES})
    set(DOXYGEN_INPUT "${DOXYGEN_INPUT}                         ${XF} \\\n")
  endforeach(XF)
  dump_variable(
  SM_DOCUMENTATION_FILES
  DOXYGEN_INPUT
  )
  set(DOXYGEN_EXCLUDE_SYMBOLS)
  foreach(XF ${SM_DOCUMENTATION_EXCLUDE_SYMBOLS})
    set(DOXYGEN_EXCLUDE_SYMBOLS "${DOXYGEN_EXCLUDE_SYMBOLS}                         ${XF} \\\n")
  endforeach(XF)

  set(DOXYFILE "${CMAKE_CURRENT_BINARY_DIR}/Doxyfile")
  set(DOXYGEN_OUTPUT "${CMAKE_BINARY_DIR}/html/index.html")
  dump_variable(DOXYGEN_TAGFILES)
  configure_file(smallchange.doxygen.cmake.in ${DOXYFILE} @ONLY)

# ############################################################################
# Setup documentation targets
# ############################################################################

  add_custom_command(
    OUTPUT ${DOXYGEN_OUTPUT}
    COMMAND ${CMAKE_COMMAND} -E echo_append "Generating API documentation with Doxygen "
    COMMAND ${DOXYGEN_EXECUTABLE} ${DOXYFILE}
    COMMAND ${CMAKE_COMMAND} -E echo "done."
    WORKING_DIRECTORY "${CMAKE_BINARY_DIR}"
    DEPENDS ${DOXYFILE}
  )
  add_custom_target(documentation ALL DEPENDS ${DOXYGEN_OUTPUT})

# ############################################################################
# Install built documentation files
# ############################################################################
  install(DIRECTORY "${CMAKE_BINARY_DIR}/html" DESTINATION ${CMAKE_INSTALL_DOCDIR} COMPONENT documentation REGEX ".*\\.(chm|qch)" EXCLUDE)
  if(GENERATE_HTMLHELP)
    install(FILES "${CMAKE_BINARY_DIR}/html/${PROJECT_NAME}-${PROJECT_VERSION_MAJOR}.chm" DESTINATION ${CMAKE_INSTALL_DOCDIR} COMPONENT documentation)
  endif()
  if(GENERATE_QHP)
    install(FILES "${CMAKE_BINARY_DIR}/html/${PROJECT_NAME}-${PROJECT_VERSION_MAJOR}.qch" DESTINATION ${CMAKE_INSTALL_DOCDIR} COMPONENT documentation)
  endif()
  if(DOXYGEN_GENERATE_MAN)
    install(DIRECTORY "${CMAKE_BINARY_DIR}/man/man3" DESTINATION ${CMAKE_INSTALL_MANDIR} COMPONENT documentation)
    message(STATUS "CMAKE_INSTALL_MANDIR ${CMAKE_INSTALL_MANDIR}")
  endif()
endif()


# ############################################################################
# Add a target to generate modern API documentation with Doxygen
# ############################################################################
if(SMALLCHANGE_BUILD_AWESOME_DOCUMENTATION)
  find_package(Doxygen)
  if(NOT DOXYGEN_FOUND)
    message(FATAL_ERROR "Doxygen is needed to build the documentation.")
  endif()

  if(NOT "${Coin_DOC_DIR}" STREQUAL "")
    get_filename_component(_coin_versioned_dir ${Coin_DOC_DIR} NAME)
    set(DOXYGEN_TAGFILES "${Coin_DOC_DIR}/html/Coin.tag=../../${_coin_versioned_dir}/html")
  endif()

  find_package(Git QUIET)
  if(GIT_FOUND AND EXISTS "${PROJECT_SOURCE_DIR}/.git")
    # Update submodules as needed
      option(GIT_SUBMODULE "Check submodules during build" ON)
      if(GIT_SUBMODULE)
          message(STATUS "Submodule update")
          execute_process(COMMAND ${GIT_EXECUTABLE} submodule update --init --recursive
                          WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
                          RESULT_VARIABLE GIT_SUBMOD_RESULT)
          if(NOT GIT_SUBMOD_RESULT EQUAL "0")
              message(FATAL_ERROR "git submodule update --init --recursive failed with ${GIT_SUBMOD_RESULT}, please checkout submodules")
          endif()
      endif()
  endif()


# ############################################################################
# Setup documentation options
# ############################################################################
  set(GENERATE_HTMLHELP NO)
  set(DOXYGEN_GENERATE_MAN NO)
  set(GENERATE_QHP NO)
  set(GENERATE_TREEVIEW YES)
  set(DOXYGEN_INTERNAL_DOCS NO)
  set(DOXYGEN_EXTRACT_PRIVATE NO)
  set(DOXYGEN_WARN_IF_UNDOCUMENTED NO)
  set(DOXYGEN_EXCLUDE)
  set(GITHUB_LINK "https://github.com/coin3d/smallchange")
  if(SMALLCHANGE_BUILD_INTERNAL_DOCUMENTATION)
    set(DOXYGEN_EXTRACT_PRIVATE YES)
    set(DOXYGEN_WARN_IF_UNDOCUMENTED NO)
    set(DOXYGEN_INTERNAL_DOCS YES)
    set(SM_DOCUMENTATION_FILES
      ${SM_DOCUMENTATION_FILES}
      ${SM_INTERNAL_DOCUMENTATION_FILES}
      CACHE INTERNAL "SM_DOCUMENTATION_FILES"
    )
  else()
    set(SM_DOCUMENTATION_EXCLUDE_SYMBOLS SoProfilerP)
    foreach(XF ${SM_INTERNAL_DOCUMENTATION_FILES})
      set(DOXYGEN_EXCLUDE "${DOXYGEN_EXCLUDE}                         ${XF} \\\n")
    endforeach(XF)
  endif()
  set(DOXYGEN_INPUT)
  list(REMOVE_DUPLICATES SM_DOCUMENTATION_FILES)
  foreach(XF ${SM_DOCUMENTATION_FILES})
    set(DOXYGEN_INPUT "${DOXYGEN_INPUT}                         ${XF} \\\n")
  endforeach(XF)
  dump_variable(
  SM_DOCUMENTATION_FILES
  DOXYGEN_INPUT
  )
  set(DOXYGEN_EXCLUDE_SYMBOLS)
  foreach(XF ${SM_DOCUMENTATION_EXCLUDE_SYMBOLS})
    set(DOXYGEN_EXCLUDE_SYMBOLS "${DOXYGEN_EXCLUDE_SYMBOLS}                         ${XF} \\\n")
  endforeach(XF)

  set(DOXYFILE_AWESOME "${CMAKE_CURRENT_BINARY_DIR}//Doxyfile_awesome")
  set(HTML_HEADER_AWESOME "${CMAKE_CURRENT_BINARY_DIR}/header_awesome.html")
  set(DOXYGEN_OUTPUT_AWESOME "${CMAKE_BINARY_DIR}/html_awesome/index.html")
  dump_variable(DOXYGEN_TAGFILES)
  configure_file(smallchange.doxygen.awesome.cmake.in ${DOXYFILE_AWESOME} @ONLY)
  configure_file(doxygen-awesome/doxygen-custom/header.html.cmake.in ${HTML_HEADER_AWESOME} @ONLY)

# ############################################################################
# Setup documentation targets
# ############################################################################

  add_custom_command(
    OUTPUT ${DOXYGEN_OUTPUT_AWESOME}
    COMMAND ${CMAKE_COMMAND} -E echo_append "Generating modern API documentation with Doxygen "
    COMMAND ${DOXYGEN_EXECUTABLE} ${DOXYFILE_AWESOME}
    COMMAND ${CMAKE_COMMAND} -E echo "done."
    WORKING_DIRECTORY "${CMAKE_BINARY_DIR}"
    DEPENDS ${DOXYFILE_AWESOME}
  )
  add_custom_target(documentation_awesome ALL DEPENDS ${DOXYGEN_OUTPUT_AWESOME})

# ############################################################################
# Install built documentation files
# ############################################################################
  install(DIRECTORY "${CMAKE_BINARY_DIR}/html_awesome" DESTINATION ${CMAKE_INSTALL_DOCDIR} COMPONENT documentation REGEX ".*\\.(chm|qch)" EXCLUDE)
endif()


