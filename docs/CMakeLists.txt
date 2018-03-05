#add a target to generate API documentation with Doxygen
find_package(Doxygen)
find_program(DOXYGEN_HAS_DOT NAMES dot DOC "hierarchical graph drawing tool")
if (DOXYGEN_HAS_DOT)
  set(HAVE_DOT YES)
endif()

option(SM_BUILD_DOCUMENTATION "Create and install API documentation (requires Doxygen)" ${DOXYGEN_FOUND})
option(SM_QT_HELP             "Generate QTHelp Documentation files (requires Doxygen)" ON)

INCLUDE(CMakeDependentOption)
CMAKE_DEPENDENT_OPTION(SM_BUILD_INTERNAL_DOCUMENTATION "Document internal code not part of the API (depends on SM_BUILD_DOCUMENTATION)" OFF "SM_BUILD_DOCUMENTATION" OFF)
file(GLOB_RECURSE DOX *.dox*)
set(SM_DOCUMENTATION_FILES ${PROJECT_SOURCE_DIR}/README.md ${SM_DOCUMENTATION_FILES} CACHE INTERNAL "${PROJECT_NAME} documentation files")

if(SM_BUILD_DOCUMENTATION)
	if(NOT DOXYGEN_FOUND)
		message(FATAL_ERROR "Doxygen is needed to build the documentation.")
	endif()
	
	set(GENERATE_HTMLHELP NO)
	if (WIN32)
		set(DOXYGEN_GENERATE_MAN NO)
		find_program(HHC_PROGRAM NAMES hhc.exe PATHS "c:/Program Files/HTML Help Workshop" "c:/Program Files (x86)/HTML Help Workshop" DOC "HTML Help Compiler program")
		if(HHC_PROGRAM)
			option(SM_BUILD_DOCUMENTATION_CHM "Create compressed HTML help (requires HHC)" OFF)
            mark_as_advanced(HHC_PROGRAM)
		else()
			set(SM_BUILD_DOCUMENTATION_CHM OFF)
		endif()
		if(SM_BUILD_DOCUMENTATION_CHM)
			set(GENERATE_HTMLHELP YES)
			set(GENERATE_TREEVIEW NO)
		else()
			set(HHC_PROGRAM)
			set(GENERATE_TREEVIEW YES)
		endif()
	else()
      if (SM_HAVE_MAN STREQUAL "ON")
        set(DOXYGEN_GENERATE_MAN YES)
        install(DIRECTORY ${CMAKE_BINARY_DIR}/man/man3 DESTINATION ${CMAKE_INSTALL_MANDIR} COMPONENT documentation)
        message(STATUS "CMAKE_INSTALL_MANDIR ${CMAKE_INSTALL_MANDIR}")
      else()
        set(DOXYGEN_GENERATE_MAN NO)
      endif()
      
      if (SM_QT_HELP)
        find_program(QHG_LOCATION NAMES qhelpgenerator qhelpgenerator-qt5 DOC "Qt qhelpgenerator")
        if (QHG_LOCATION)
          set(GENERATE_QHP "YES")
        else()
          set(GENERATE_QHP "NO")
          message(FATAL_ERROR "Missing qhelpgenerator")
        endif ()
      endif(SM_QT_HELP)

      set(GENERATE_TREEVIEW YES)
      set(HHC_PROGRAM)
	endif()

	set(DOXYGEN_INTERNAL_DOCS NO)
	set(DOXYGEN_EXTRACT_PRIVATE NO)
	set(DOXYGEN_WARN_IF_UNDOCUMENTED YES)
	set(DOXYGEN_EXCLUDE)
	if(SM_BUILD_INTERNAL_DOCUMENTATION)
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
	set(DOXYFILE ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile)
	set(DOXYGEN_OUTPUT_DIR ${PROJECT_BINARY_DIR}/html)
	set(DOXYGEN_OUTPUT ${DOXYGEN_OUTPUT_DIR}/index.html)
# 	configure_file(smallchange.doxygen.cmake.in ${DOXYFILE} @ONLY)
	configure_file(smallchange.doxygen.in ${DOXYFILE} @ONLY)
	
	add_custom_command(
		OUTPUT ${DOXYGEN_OUTPUT}
		COMMAND ${CMAKE_COMMAND} -E echo_append "Generating API documentation with Doxygen"
		COMMAND ${DOXYGEN_EXECUTABLE} ${DOXYFILE}
		COMMAND ${CMAKE_COMMAND} -E echo "Done."
		WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
		DEPENDS ${DOXYFILE}
	)
	add_custom_target(documentation ALL DEPENDS ${DOXYGEN_OUTPUT})
	install(DIRECTORY ${DOXYGEN_OUTPUT_DIR} DESTINATION ${CMAKE_INSTALL_DOCDIR} COMPONENT documentation)
endif()
