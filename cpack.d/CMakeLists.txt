# PROJECT_NAME is used also for the target library name and the package name
string(TOLOWER "${PROJECT_NAME}" SM_SHORTNAME)
set(SM_URL "https://bitbucket.org/Coin3D/${SM_SHORTNAME}")
set(SM_CONTACT "coin-support@coin3d.org")

file(GLOB CONFIGS RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} *.cmake.in)
dump_variable(
CONFIGS
)

foreach(FILE ${CONFIGS})
  string(REGEX REPLACE ".cmake.in$" ".cmake" CFG ${FILE})
  configure_file(${FILE} ${CFG} @ONLY)
endforeach()
