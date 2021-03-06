# - Find ODB library
# Find the native ODB includes and library
# This module defines
#  ODB_INCLUDE_DIR, where to find Url.h, etc.
#  ODB_LIBRARIES, libraries to link against to use ODB client C++.
#  ODB_FOUND, If false, do not try to use ODB client C++.


if (NOT $ENV{CPLUS_INCLUDE_PATH}  STREQUAL "")
   string (REPLACE ":" ";" INCL_PATHS $ENV{CPLUS_INCLUDE_PATH})
endif ()

if (NOT $ENV{LIBRARY_PATH}  STREQUAL "")
   string (REPLACE ":" ";" LIB_PATHS $ENV{LIBRARY_PATH})
endif ()

FIND_PROGRAM(ODB_COMPILER odb HINT ${ODB_COMPILER_DIR})

FIND_PATH(ODB_INCLUDE_DIR odb/core.hxx HINTS ${INCL_PATHS})

FIND_LIBRARY(ODB_CORE_LIBRARY odb HINTS ${LIB_PATHS})

# handle the QUIETLY and REQUIRED arguments and set ODB_FOUND to TRUE if
# all listed variables are TRUE
INCLUDE(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(ODB  DEFAULT_MSG
    ODB_CORE_LIBRARY 
    ODB_INCLUDE_DIR ODB_COMPILER)

IF(ODB_FOUND)
    SET(ODB_LIBRARIES ${ODB_CORE_LIBRARY})
    SET(ODB_COMPILER ${ODB_COMPILER})
    MESSAGE(STATUS "Found ODB Compiler: ${ODB_COMPILER}")
    MESSAGE(STATUS "Found ODB Include folder: ${ODB_INCLUDE_DIR}")
    MESSAGE(STATUS "Found ODB Core Libraries: ${ODB_LIBRARIES}")
ENDIF()

MARK_AS_ADVANCED(ODB_INCLUDE_DIR ODB_LIBRARIES ODB_COMPILER ODB_CORE_LIBRARY)
