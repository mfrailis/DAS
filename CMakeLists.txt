cmake_minimum_required(VERSION 2.6)
project(libdas)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/resources/")
set(ODB_COMPILER_DIR  CACHE PATH "hint to where to find odb compiler executable")


find_package(Boost 1.47 REQUIRED)
find_package(ODB REQUIRED)
find_package(ODB_BOOST REQUIRED)
find_package(Python REQUIRED)
find_package(MySQL REQUIRED)
find_python_module(lxml REQUIRED)
find_python_module(MySQLdb REQUIRED)
find_python_module(collections)
if(NOT PY_COLLECTIONS_FOUND) 
  find_python_module(ordereddict)
endif()
find_package(Blitz REQUIRED)

set(COMMON_LIBRARIES
  boost_date_time
  ${ODB_LIBRARIES}
  ${ODB_BOOST_LIBRARIES}
)

set(CMAKE_BUILD_TYPE debug)
set(CMAKE_CXX_FLAGS_DEBUG "-g")

set(CPP_SOURCE_DIR ${CMAKE_SOURCE_DIR}/src/cpp)
set(CPP_INCLUDE_DIR ${CMAKE_SOURCE_DIR}/include)
set(CPP_INCLUDE_INTERNAL_DIR ${CPP_INCLUDE_DIR}/internal)
set(TEST_SOURCE_DIR ${CMAKE_SOURCE_DIR}/src/test)
set(EXAMPLES_SOURCE_DIR ${CMAKE_SOURCE_DIR}/src/examples)

set(PYTHON_SOURCE_DIR ${CMAKE_SOURCE_DIR}/src/python)

set(TYPE_PREFIX ddl_)

set(DAS_BUILD_DIR ${PROJECT_SOURCE_DIR}/build)

set(SWIG_SOURCE_DIR ${CMAKE_SOURCE_DIR}/src/swig)
set(SWIG_BUILD_DIR ${DAS_BUILD_DIR}/swig)


set(DDL_INSTANCE_DIR ${CMAKE_SOURCE_DIR}/ddl)
set(DDL_SCHEMA ${CMAKE_SOURCE_DIR}/resources/ddl.xsd)

set(DDL_SIGNATURE ddl_signature.txt)
set(DDL_HEADERS_DIR ${PROJECT_SOURCE_DIR}/include/ddl/types)
set(DDL_SOURCE_DIR ${DAS_BUILD_DIR}/src)

set(CONF_JSON_SCHEMA ${CMAKE_SOURCE_DIR}/resources/config_schema.json)
set(CONF_JSON ${CMAKE_SOURCE_DIR}/configure/config.json)

set(DB_ACCESS_JSON_SCHEMA ${CMAKE_SOURCE_DIR}/resources/access_schema.json)
set(DB_ACCESS_DIR $ENV{HOME}/.das)
set(DB_ACCESS_JSON ${DB_ACCESS_DIR}/access.json)

#set(ODB_SOURCE_DIR ${DAS_BUILD_DIR})
set(ODB_SOURCE_DIR ${DDL_HEADERS_DIR})
set(ODB_OUTPUT_DIR ${DAS_BUILD_DIR}/dbms)
set(DB_TARGETS_DIR ${DAS_BUILD_DIR}/db)


set(TARGET_DB_PREFIX "db-")


#TODO check for all user defined files.

set(DAS_QL_SRC 
  ${CPP_SOURCE_DIR}/ql/qlvisitor.cpp
  ${CPP_SOURCE_DIR}/ql/bnfc/Absyn.C
  ${CPP_SOURCE_DIR}/ql/bnfc/Lexer.C
  ${CPP_SOURCE_DIR}/ql/bnfc/Parser.C
)
set(DAS_SRC
  ${CPP_SOURCE_DIR}/info.cpp
  ${CPP_SOURCE_DIR}/transaction.cpp
  ${CPP_SOURCE_DIR}/db_bundle.cpp
  ${CPP_SOURCE_DIR}/storage_engine.cpp
  ${CPP_SOURCE_DIR}/database_config.cpp
  ${CPP_SOURCE_DIR}/column_buffer.cpp
  ${CPP_SOURCE_DIR}/image_buffer.cpp
#remove from now on
  ${CPP_SOURCE_DIR}/storage_engine_raw.cpp
)

set(UNIT_TEST_SRC
  ${TEST_SOURCE_DIR}/unit_tpl_all.hpp
  ${TEST_SOURCE_DIR}/unit_metadata.cpp
  ${TEST_SOURCE_DIR}/unit_association.cpp
  ${TEST_SOURCE_DIR}/unit_column.cpp
  ${TEST_SOURCE_DIR}/unit_column_array.cpp  
  ${TEST_SOURCE_DIR}/unit_image.cpp
  ${TEST_SOURCE_DIR}/unit_main.cpp  
  )


include_directories(${CPP_INCLUDE_DIR})
include_directories(${CMAKE_INCLUDE_PATH})
include_directories(${Boost_INCLUDE_DIR})
include_directories(${ODB_INCLUDE_DIR})
include_directories(${BLITZ_INCLUDE_DIR})
include_directories(${MYSQL_INCLUDE_DIR})
#include_directories(${ODB_SOURCE_DIR})

set(ODB_COMMON_TYPES ${ODB_SOURCE_DIR}/ddl_types.hpp)

file(MAKE_DIRECTORY ${ODB_OUTPUT_DIR})
file(MAKE_DIRECTORY ${DB_TARGETS_DIR})
file(MAKE_DIRECTORY ${DDL_HEADERS_DIR})
file(MAKE_DIRECTORY ${DDL_SOURCE_DIR})
file(MAKE_DIRECTORY ${SWIG_BUILD_DIR})

execute_process(
  COMMAND python configure.py
          ${DB_TARGETS_DIR}
	  ${DAS_BUILD_DIR}
	  ${DDL_HEADERS_DIR}
	  ${DDL_SOURCE_DIR}
	  ${SWIG_BUILD_DIR}
          ${CONF_JSON_SCHEMA}
          ${CONF_JSON}
          ${DDL_SCHEMA}
          ${DDL_INSTANCE_DIR}
          ${TARGET_DB_PREFIX}
  WORKING_DIRECTORY ${PYTHON_SOURCE_DIR}
  OUTPUT_VARIABLE CONFIG_MSG
  RESULT_VARIABLE CONFIG_STATUS
)
if(CONFIG_STATUS)
  message(FATAL_ERROR ${CONFIG_MSG})
endif()

add_subdirectory(${DAS_BUILD_DIR})

# add a target to generate API documentation with Doxygen
find_package(Doxygen)
if(DOXYGEN_FOUND)
  #configure_file(${CMAKE_CURRENT_SOURCE_DIR}/Doxyfile ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile @ONLY)
  add_custom_target(doc
    ${DOXYGEN_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/doc/Doxyfile
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    COMMENT "Generating API documentation with Doxygen" VERBATIM
    )
endif(DOXYGEN_FOUND)

find_package(SWIG)
find_package(PythonLibs)
if(SWIG_FOUND AND PYTHONLIBS_FOUND)
  include(${SWIG_USE_FILE})
  include_directories(${PYTHON_INCLUDE_PATH})
  
  include_directories(${SWIG_BUILD_DIR})
  
  set(CMAKE_SWIG_FLAGS "-Wall")
  
  set_source_files_properties(${SWIG_SOURCE_DIR}/pydas.i PROPERTIES CPLUSPLUS ON)
  set_source_files_properties(${SWIG_SOURCE_DIR}/pydas.i PROPERTIES SWIG_FLAGS "-includeall")
  swig_add_module(pydas python ${SWIG_SOURCE_DIR}/pydas.i)
  swig_link_libraries(pydas ${PYTHON_LIBRARIES} DAS_SO ${COMMON_LIBRARIES} ${ODB_MYSQL_LIBRARY})

endif()

add_executable(unit_tests ${UNIT_TEST_SRC})
target_link_libraries(unit_tests DAS_A ${COMMON_LIBRARIES} ${ODB_MYSQL_LIBRARY} boost_unit_test_framework)

install(DIRECTORY include/ DESTINATION include/das)

#add_library(das ${DAS_QL_SRC} ${DAS_SRC} ${TYPES_SRC} ${ODB_SRC})

#get_cmake_property(_variableNames VARIABLES)
#foreach (_variableName ${_variableNames})
#    message(STATUS "${_variableName}=${${_variableName}}")
#endforeach()