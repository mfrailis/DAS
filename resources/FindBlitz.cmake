# - Find Blitz library
# Find the native Blitz includes and library
# This module defines
#  BLITZ_INCLUDE_DIR, where to find Url.h, etc.
#  BLITZ_LIBRARY, libraries to link against to use ODB client C++.
#  BLITZ_FOUND, if false it means that blitz headers and library where not found


if (NOT $ENV{CPLUS_INCLUDE_PATH}  STREQUAL "")
   string (REPLACE ":" ";" INCL_PATHS $ENV{CPLUS_INCLUDE_PATH})
endif ()

if (NOT $ENV{LIBRARY_PATH}  STREQUAL "")
   string (REPLACE ":" ";" LIB_PATHS $ENV{LIBRARY_PATH})
endif ()

FIND_PATH(BLITZ_INCLUDE_DIR blitz/blitz.h HINTS ${INCL_PATHS})

FIND_LIBRARY(BLITZ_LIBRARY blitz HINTS ${LIB_PATHS})

# handle the QUIETLY and REQUIRED arguments and set ODB_FOUND to TRUE if
# all listed variables are TRUE
INCLUDE(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(BLITZ  DEFAULT_MSG
    BLITZ_INCLUDE_DIR
    BLITZ_LIBRARY)

IF(BLITZ_FOUND)
    MESSAGE(STATUS "Found BLITZ Include folder: ${BLITZ_INCLUDE_DIR}")
    MESSAGE(STATUS "Found BLITZ library: ${BLITZ_LIBRARY}")
ENDIF()

MARK_AS_ADVANCED(BLITZ_INCLUDE_DIR BLITZ_LIBRARY)
