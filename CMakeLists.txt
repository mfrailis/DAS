cmake_minimum_required(VERSION 2.8)
project(libdas)

set(CMAKE_BUILD_TYPE debug)
set(CMAKE_CXX_FLAGS_DEBUG "-g -DVDBG")

set(CPP_SOURCE_DIR ${CMAKE_SOURCE_DIR}/src/cpp)
set(CPP_INCLUDE_DIR ${CMAKE_SOURCE_DIR}/include)
set(CPP_INCLUDE_INTERNAL_DIR ${CPP_SOURCE_DIR})
set(TEST_SOURCE_DIR ${CMAKE_SOURCE_DIR}/src/test)

set(PYTHON_SOURCE_DIR ${CMAKE_SOURCE_DIR}/src/python)

set(TYPE_PREFIX ddl_)

set(DAS_BUILD_DIR ${PROJECT_SOURCE_DIR}/build)

set(ODB_SOURCE_DIR ${DAS_BUILD_DIR})
set(ODB_OUTPUT_DIR ${DAS_BUILD_DIR}/dbms)
set(DB_TARGETS_DIR ${DAS_BUILD_DIR}/db)

set(DDL_INSTANCE_DIR ${CMAKE_SOURCE_DIR}/ddl)
set(DDL_SCHEMA ${CMAKE_SOURCE_DIR}/resources/ddl.xsd)

set(DDL_SIGNATURE ddl_signature.txt)

set(CONF_JSON_SCHEMA ${CMAKE_SOURCE_DIR}/resources/config_schema.json)
set(CONF_JSON ${CMAKE_SOURCE_DIR}/configure/config.json)

set(DB_ACCESS_JSON_SCHEMA ${CMAKE_SOURCE_DIR}/resources/access_schema.json)
set(DB_ACCESS_DIR $ENV{HOME}/.das)
set(DB_ACCESS_JSON ${DB_ACCESS_DIR}/access.json)

set(TARGET_DB_PREFIX "db-")


#TODO check for all user defined files.
#set(ODB_HXX)
#set(ODB_CXX)
#set(TYPES_CPP)
set(DAS_QL_SRC 
  ${CPP_SOURCE_DIR}/ql/qlvisitor.cpp
  ${CPP_SOURCE_DIR}/ql/bnfc/Absyn.C
  ${CPP_SOURCE_DIR}/ql/bnfc/Lexer.C
  ${CPP_SOURCE_DIR}/ql/bnfc/Parser.C
)
set(DAS_SRC
  ${CPP_SOURCE_DIR}/info.cpp
  ${CPP_SOURCE_DIR}/tpl/transaction.cpp
)

include_directories(${CPP_INCLUDE_DIR})
#include_directories(${ODB_SOURCE_DIR})

set(ODB_COMMON_TYPES ${ODB_SOURCE_DIR}/ddl_types.hpp)

file(MAKE_DIRECTORY ${ODB_OUTPUT_DIR})
file(MAKE_DIRECTORY ${DB_TARGETS_DIR})

execute_process(
  COMMAND python configure.py
          ${DB_TARGETS_DIR}
	  ${DAS_BUILD_DIR}
          ${CONF_JSON_SCHEMA}
          ${CONF_JSON}
          ${DDL_SCHEMA}
          ${DDL_INSTANCE_DIR}
          ${TARGET_DB_PREFIX}
  WORKING_DIRECTORY ${PYTHON_SOURCE_DIR}
)

add_subdirectory(${DAS_BUILD_DIR})

#add_library(das ${DAS_QL_SRC} ${DAS_SRC} ${TYPES_SRC} ${ODB_SRC})

