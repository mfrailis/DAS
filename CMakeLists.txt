cmake_minimum_required(VERSION 2.8)
project(das-lib)

set(CPP_SOURCE_DIR ${CMAKE_SOURCE_DIR}/src/cpp)
set(CPP_INCLUDE_DIR ${CMAKE_SOURCE_DIR}/include)
set(PYTHON_SOURCE_DIR ${CMAKE_SOURCE_DIR}/src/python)

set(DAS_BUILD_DIR ${PROJECT_SOURCE_DIR}/build)

set(ODB_SOURCE_DIR ${DAS_BUILD_DIR})
set(ODB_OUTPUT_DIR ${DAS_BUILD_DIR}/odb)

set(DDL_INSTANCE_DIR ${CMAKE_SOURCE_DIR}/ddl/schemas)
set(DDL_SCHEMA ${CMAKE_SOURCE_DIR}/ddl/ddl.xsd)

set(CONF_JSON_SCHEMA ${CMAKE_SOURCE_DIR}/configure/schema.json)
set(CONF_JSON ${CMAKE_SOURCE_DIR}/configure/config.json)

set(DB_ACCESS_JSON_SCHEMA ${CMAKE_SOURCE_DIR}/configure/access_schema.json)
set(DB_ACCESS_DIR $ENV{HOME}/.das)
set(DB_ACCESS_JSON ${DB_ACCESS_DIR}/access.json)

set(TARGET_DB_PREFIX "db-")

#TODO check for all user defined files.

include_directories(${CPP_SOURCE_DIR})
include_directories(${ODB_OUTPUT_DIR})

set(ODB_COMMON_TYPES ${ODB_SOURCE_DIR}/ddl_types.hpp)

add_custom_command(
  OUTPUT  odb-common
  COMMAND mkdir -p ${ODB_OUTPUT_DIR}
  COMMAND odb --multi-database dynamic 
          --database common
          --generate-query --generate-schema
          --output-dir ${ODB_OUTPUT_DIR}
          --at-once ${ODB_COMMON_TYPES}
	  -x -I${CPP_INCLUDE_DIR}
  COMMENT "Built common odb classes"
)

execute_process(
  COMMAND python configure.py ${DAS_BUILD_DIR} ${CONF_JSON_SCHEMA} ${CONF_JSON}
          ${DDL_SCHEMA} ${DDL_INSTANCE_DIR} ${TARGET_DB_PREFIX}
  WORKING_DIRECTORY ${PYTHON_SOURCE_DIR}
)

add_subdirectory(${DAS_BUILD_DIR})

add_custom_target(
  odb-only ALL
  DEPENDS odb-common
  COMMENT "generate type classes"
)


#add_library (ddl_runtime_lib ${CPP_SOURCE_DIR}/ddl_info.hpp classes-odb)
#add_executable(ddl_runtime_info ${CPP_SOURCE_DIR}/main.cpp)
#target_link_libraries(ddl_runtime_info ddl_runtime_lib xerces-c)

