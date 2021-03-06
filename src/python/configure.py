import odb as _odb
import swig as _swig
import introspection as _info
import ddl as _ddl
import json as _j
import jsonschema as _js
import hashlib as _hash
import os as _os
import filecmp

class JsonConfigParser:
    def __init__(self,schema_path):
        f = open(schema_path,'r')
        self._schema = _j.load(f)
        f.close()

        self.ddl_map = _info.DdlMap()
        self.db_map = {}
        self._config = []
        self.ddl_files = set()
        self.sub_dirs = []

    def parse(self,config_file):
        f = open(config_file,'r')
        self._config = _j.load(f)
        f.close()
        _js.validate(self._config, self._schema, format_checker=_js.FormatChecker())
        aliases = []        
        for db in self._config:
            if db['alias'] in aliases:
                print 'ERROR: found repeated  "'+db['alias']+'" alias in configuration file'
                exit(1)
            aliases.append(db['alias'])
            self.db_map[_assemble_db(db)]=_assemble_ddl(db['ddl'])
            self.ddl_files.add(db['ddl'])


    def get_db_list(self):
        return self.db_map.keys()

    def generate_db_headers(self,output_dir):
        ddl_set = set(self.db_map.values())
        for ddl in ddl_set:
            f = open(_os.path.join(output_dir,ddl+'_types.h'),'w')
            f.write('#ifndef DDL_'+ddl+'_H\n')
            f.write('#define DDL_'+ddl+'_H\n\n')
            for t in self.ddl_map.get_type_list(ddl):
                if t != "essentialMetadata":
                    f.write('#include "ddl_'+t+'.hpp"\n')
            f.write('\n#endif\n')
            f.close()

    def generate_garbage_collectors(self,output_dir):
        for db in self._config:
            if db['storage_engine']['name'] == 'Raw':
                f = open(_os.path.join(output_dir,db['alias']+'_gc.cpp'),'w')  
                f.write('''
#include "tpl/database.hpp"
#include "transaction.hpp"
#include "ddl/types.hpp"
#include "internal/daemon.hpp"

int main(){
    shared_ptr<das::tpl::Database> db = das::tpl::Database::create("'''+db['alias']+'''");''')    
                for t in self.ddl_map.get_type_list(_assemble_ddl(db['ddl'])):
                    if t != 'essentialMetadata':
                        f.write('\n    das::data_gc::collect<'+t+'>(db);')
        
                f.write('\n    return 0;\n}\n')
                f.close()

    def generate_sub(self,db_dir,t_pre):
        for db in self._config:
            #db_str = db['host']+"_"+str(db['port'])+"_"+db['db_name']
            db_str = db['alias']
            has_gc = db['storage_engine']['name'] == 'Raw'
            self.sub_dirs.append(db_str)
            dir_name = _os.path.join(db_dir,db_str)
            if not _os.path.exists(dir_name):
                _os.mkdir(dir_name)
#            ddl_h   = self.db_map[_assemble_db(db)]+'_types.h'
#            ddl_sql = self.db_map[_assemble_db(db)]+'_types.sql'
            typelist=self.ddl_map.get_type_list(_assemble_ddl(db['ddl']))
            _generate_sub_cmake(dir_name,db_str,db['db_type'], t_pre,db,typelist,has_gc)
    
    def generate(self,db_dir_,cmake_dir,ddl_src_dir,types,prefix):
        db_vendors = {};
        for db in self._config:
            db_dir =  _os.path.join(db_dir_,db['db_type'])
            db_vendors[db['db_type']] = db_dir


        f = open(_os.path.join(cmake_dir,'CMakeLists.txt'),'w')
        f.write('cmake_minimum_required(VERSION 2.6)\n')
        f.write('set(TYPE_NAMES_ALL \n')
        for i in types:
            if i != "essentialMetadata":
                f.write(' '+i)
        f.write(')')
        f.write('''
set(TYPES_CPP ${DDL_SOURCE_DIR}/${TYPE_PREFIX}info.cpp ${DDL_SOURCE_DIR}/database_config.cpp ${DDL_SOURCE_DIR}/database_plf.cpp)
foreach(type_name ${TYPE_NAMES_ALL})
  list(APPEND TYPES_CPP ${DDL_SOURCE_DIR}/${TYPE_PREFIX}${type_name}.cpp)
endforeach()
''')

        for (db_type,db_dir_) in db_vendors.items():
            db_dir = "${ODB_OUTPUT_DIR}/"+db_type
            f.write('''
find_package(ODB_'''+db_type.upper()+''' REQUIRED)

file(MAKE_DIRECTORY '''+db_dir+''')
file(MAKE_DIRECTORY ${DDL_HEADERS_DIR}/'''+db_type+''')
list(APPEND ODB_CXX '''+db_dir+'''/aux_query-odb.cxx)
list(APPEND ODB_CXX '''+db_dir+'''/das_object-odb.cxx)
list(APPEND ODB_CXX '''+db_dir+'''/column-odb.cxx)
list(APPEND ODB_CXX '''+db_dir+'''/image-odb.cxx)

foreach(type_name ${TYPE_NAMES_ALL})
  add_custom_command(
    OUTPUT 
            '''+db_dir+'''/${TYPE_PREFIX}${type_name}-odb.cxx 
            ${DDL_HEADERS_DIR}/'''+db_type+'''/${TYPE_PREFIX}${type_name}-odb.hxx
            ${DDL_HEADERS_DIR}/'''+db_type+'''/${TYPE_PREFIX}${type_name}-odb.ixx
    COMMAND ${ODB_COMPILER}
            --output-dir '''+db_dir+'''
            --database '''+db_type+'''
            --generate-query
            --generate-session
            --include-regex "%${TYPE_PREFIX}${type_name}-odb.hxx%ddl/types/'''+db_type+'''/${TYPE_PREFIX}${type_name}-odb.hxx%"
            --include-regex "%(column.hpp)|(image.hpp)%../../../ddl/$1$2%"
            --include-regex "%(.*/)(column-odb.hxx)|(.*/)(image-odb.hxx)%$2$4%"
            --include-regex "%aux_query.hpp%../../../internal/aux_query.hpp%"
            --include-regex "%(.*)das_object-odb.hxx%das_object-odb.hxx%"
            --include-regex "%(.*)ddl_(.+).hxx%ddl_$2.hxx%"
            --include-regex "%ddl/(.+).hxx%$1.hxx%"
            --include-regex "%ddl_(.+).hpp%../ddl_$1.hpp%"
            --profile boost/unordered
            --profile boost/date-time/posix-time
            --profile boost/optional

            --default-pointer std::tr1::shared_ptr
	    -I${ODB_SOURCE_DIR}
            -I${CPP_INCLUDE_DIR}
            -I${ODB_INCLUDE_DIR}
            -x -I${Boost_INCLUDE_DIR} -x -I${BLITZ_INCLUDE_DIR}
	    ${ODB_SOURCE_DIR}/${TYPE_PREFIX}${type_name}.hpp
    

    COMMAND mv "'''+db_dir+'''/${TYPE_PREFIX}${type_name}-odb.hxx" "${DDL_HEADERS_DIR}/'''+db_type+'''/${TYPE_PREFIX}${type_name}-odb.hxx"
    COMMAND mv "'''+db_dir+'''/${TYPE_PREFIX}${type_name}-odb.ixx" "${DDL_HEADERS_DIR}/'''+db_type+'''/${TYPE_PREFIX}${type_name}-odb.ixx"
    DEPENDS ${DDL_LOCAL_SIGNATURE} ${ODB_SOURCE_DIR}/${TYPE_PREFIX}${type_name}.hpp
    COMMENT "Generating odb class for type ${type_name} for '''+db_type+''' DBMS"
    VERBATIM 
    )
  list(APPEND ODB_CXX '''+db_dir+'''/${TYPE_PREFIX}${type_name}-odb.cxx)
endforeach()
        
add_custom_command(
OUTPUT 
    '''+db_dir+'''/das_object-odb.cxx
    ${DDL_HEADERS_DIR}/'''+db_type+'''/das_object-odb.hxx
    ${DDL_HEADERS_DIR}/'''+db_type+'''/das_object-odb.ixx
COMMAND ${ODB_COMPILER}
    --output-dir '''+db_dir+'''
    --database '''+db_type+'''
    --generate-query
    --generate-session
    --include-regex "%(column.hpp)|(image.hpp)%../../../ddl/$1$2%"
    --include-regex "%aux_query.hpp%../../../internal/aux_query.hpp%"
    --include-regex "%ddl_(.+).hpp%../ddl_$1.hpp%"
    --include-regex "%(.*)(column-odb.hxx)|(.*)(image-odb.hxx)%$2$4%"
    --include-regex "%das_object-odb.hxx%ddl/types/'''+db_type+'''/das_object-odb.hxx%"
    --include-regex "%das_object.hpp%../../../das_object.hpp%"

    --profile boost/date-time/posix-time

    --default-pointer std::tr1::shared_ptr


    -I${ODB_SOURCE_DIR}
    -I${CPP_INCLUDE_DIR}
    -I${ODB_INCLUDE_DIR}
    -x -I${Boost_INCLUDE_DIR} -x -I${BLITZ_INCLUDE_DIR}
    ${CPP_INCLUDE_DIR}/das_object.hpp

COMMAND mv "'''+db_dir+'''/das_object-odb.hxx" "${DDL_HEADERS_DIR}/'''+db_type+'''/das_object-odb.hxx"
COMMAND mv "'''+db_dir+'''/das_object-odb.ixx" "${DDL_HEADERS_DIR}/'''+db_type+'''/das_object-odb.ixx"
COMMENT "Generating odb DasObject class for '''+db_type+''' DBMS"
VERBATIM
)

add_custom_command(
OUTPUT 
    '''+db_dir+'''/aux_query-odb.cxx
    ${DDL_HEADERS_DIR}/'''+db_type+'''/aux_query-odb.hxx
    ${DDL_HEADERS_DIR}/'''+db_type+'''/aux_query-odb.ixx
COMMAND ${ODB_COMPILER}
    --output-dir '''+db_dir+'''
    --database '''+db_type+'''
    --generate-query
    --generate-session
    --include-regex "%(column.hpp)|(image.hpp)%../../../ddl/$1$2%"
    --include-regex "%aux_query.hpp%../../../internal/aux_query.hpp%"
    --include-regex "%ddl_(.+).hpp%ddl/types/ddl_$1.hpp%"
    --include-regex "%aux_query-odb.hxx%ddl/types/'''+db_type+'''/aux_query-odb.hxx%"


    --default-pointer std::tr1::shared_ptr 
    -I${ODB_SOURCE_DIR}
    -I${CPP_INCLUDE_DIR}
    -I${ODB_INCLUDE_DIR}
    -x -I${Boost_INCLUDE_DIR}
    ${CPP_INCLUDE_INTERNAL_DIR}/aux_query.hpp
COMMAND mv "'''+db_dir+'''/aux_query-odb.hxx" "${DDL_HEADERS_DIR}/'''+db_type+'''/aux_query-odb.hxx"
COMMAND mv "'''+db_dir+'''/aux_query-odb.ixx" "${DDL_HEADERS_DIR}/'''+db_type+'''/aux_query-odb.ixx"
COMMENT "Generating odb auxiliary query on for '''+db_type+''' DBMS"
VERBATIM
)

add_custom_command(
OUTPUT 
    '''+db_dir+'''/column-odb.cxx
    ${DDL_HEADERS_DIR}/'''+db_type+'''/column-odb.hxx
    ${DDL_HEADERS_DIR}/'''+db_type+'''/column-odb.ixx
COMMAND ${ODB_COMPILER}
    --output-dir '''+db_dir+'''
    --database '''+db_type+'''
    --generate-query
    --generate-session
    --include-regex "%(column.hpp)|(image.hpp)%../../../ddl/$1$2%"
    --include-regex "%column-odb.hxx%ddl/types/'''+db_type+'''/column-odb.hxx%"

    --default-pointer std::tr1::shared_ptr
    -I${ODB_SOURCE_DIR}
    -I${CPP_INCLUDE_DIR}
    -I${ODB_INCLUDE_DIR}
    -x -I${Boost_INCLUDE_DIR} -x -I${BLITZ_INCLUDE_DIR}
    ${CPP_INCLUDE_DIR}/ddl/column.hpp
COMMAND mv "'''+db_dir+'''/column-odb.hxx" "${DDL_HEADERS_DIR}/'''+db_type+'''/column-odb.hxx"
COMMAND mv "'''+db_dir+'''/column-odb.ixx" "${DDL_HEADERS_DIR}/'''+db_type+'''/column-odb.ixx"
COMMENT "Generating odb column data support for '''+db_type+''' DBMS"
VERBATIM
)

add_custom_command(
OUTPUT 
    '''+db_dir+'''/image-odb.cxx
    ${DDL_HEADERS_DIR}/'''+db_type+'''/image-odb.hxx
    ${DDL_HEADERS_DIR}/'''+db_type+'''/image-odb.ixx
COMMAND ${ODB_COMPILER}
    --output-dir '''+db_dir+'''
    --database '''+db_type+'''
    --generate-query
    --generate-session
    --include-regex "%(column.hpp)|(image.hpp)%../../../ddl/$1$2%"
    --include-regex "%image-odb.hxx%ddl/types/'''+db_type+'''/image-odb.hxx%"

    --default-pointer std::tr1::shared_ptr 
    -I${ODB_SOURCE_DIR}
    -I${CPP_INCLUDE_DIR}
    -I${ODB_INCLUDE_DIR}
    -x -I${Boost_INCLUDE_DIR} -x -I${BLITZ_INCLUDE_DIR}
    ${CPP_INCLUDE_DIR}/ddl/image.hpp
COMMAND mv "'''+db_dir+'''/image-odb.hxx" "${DDL_HEADERS_DIR}/'''+db_type+'''/image-odb.hxx"
COMMAND mv "'''+db_dir+'''/image-odb.ixx" "${DDL_HEADERS_DIR}/'''+db_type+'''/image-odb.ixx"
COMMENT "Generating odb image data support for '''+db_type+''' DBMS"
VERBATIM
)
''')
        for l in self.sub_dirs:
            f.write("add_subdirectory(db/"+l+")\n")
        f.write(
'''

add_custom_target(
  odb-gen
  DEPENDS ${ODB_CXX}
)

add_custom_target(schema-all ALL)
''')

        for l in self.sub_dirs:
            f.write("add_dependencies(schema-all schema-"+l+')\n')


        f.write(
'''
add_custom_target('''+prefix+'''all)
'''
)
        for l in self.sub_dirs:
            f.write("add_dependencies("+prefix+"all "+prefix+l+")\n")
        f.write(
'''

foreach(type_ ${TYPE_NAMES_ALL} )
  message(STATUS "Generated type ${type_}")
endforeach()

add_library(DAS_SO SHARED ${DAS_QL_SRC} ${DAS_SRC} ${TYPES_CPP} ${ODB_CXX})
add_library(DAS_A STATIC ${DAS_QL_SRC} ${DAS_SRC} ${TYPES_CPP} ${ODB_CXX})

target_link_libraries(DAS_SO ${COMMON_LIBRARIES} ${ODB_MYSQL_LIBRARIES})
target_link_libraries(DAS_A ${COMMON_LIBRARIES} ${ODB_MYSQL_LIBRARIES})

set_target_properties(DAS_SO PROPERTIES OUTPUT_NAME das)
set_target_properties(DAS_A  PROPERTIES OUTPUT_NAME das)

install(
  TARGETS DAS_SO
  LIBRARY DESTINATION lib
  PERMISSIONS
    OWNER_READ
    OWNER_WRITE
    OWNER_EXECUTE
    GROUP_READ
    GROUP_EXECUTE
    WORLD_READ
    WORLD_EXECUTE 
)

install(
  TARGETS DAS_A 
  ARCHIVE DESTINATION lib
  PERMISSIONS
    OWNER_READ
    OWNER_WRITE
    OWNER_EXECUTE
    GROUP_READ
    GROUP_EXECUTE
    WORLD_READ
    WORLD_EXECUTE 
)

#add_executable(main_test ${TEST_SOURCE_DIR}/main.cpp)
#target_link_libraries(main_test DAS_SO)

add_executable(metadata_test EXCLUDE_FROM_ALL ${TEST_SOURCE_DIR}/metadata_test.cpp)
target_link_libraries(metadata_test DAS_SO)

add_executable(association_test EXCLUDE_FROM_ALL ${TEST_SOURCE_DIR}/association_test.cpp)
target_link_libraries(association_test DAS_SO)

add_executable(data_test EXCLUDE_FROM_ALL ${TEST_SOURCE_DIR}/data_test.cpp)
target_link_libraries(data_test boost_thread boost_random DAS_SO)

add_executable(array_column_test EXCLUDE_FROM_ALL ${TEST_SOURCE_DIR}/array_column_test.cpp)
target_link_libraries(array_column_test DAS_SO)

add_executable(rollback_test EXCLUDE_FROM_ALL ${TEST_SOURCE_DIR}/rollback_test.cpp)
target_link_libraries(rollback_test boost_thread DAS_SO)

add_executable(scratch EXCLUDE_FROM_ALL ${TEST_SOURCE_DIR}/scratch.cpp)
target_link_libraries(scratch DAS_SO)

add_executable(persistence_example EXCLUDE_FROM_ALL ${EXAMPLES_SOURCE_DIR}/persistence_example.cpp)
target_link_libraries(persistence_example DAS_SO)

add_executable(query_example EXCLUDE_FROM_ALL ${EXAMPLES_SOURCE_DIR}/query_example.cpp)
target_link_libraries(query_example DAS_SO)

add_executable(associations_example EXCLUDE_FROM_ALL ${EXAMPLES_SOURCE_DIR}/associations_example.cpp)
target_link_libraries(associations_example DAS_SO)

add_executable(data_example EXCLUDE_FROM_ALL ${EXAMPLES_SOURCE_DIR}/data_example.cpp)
target_link_libraries(data_example DAS_SO)

add_executable(column_example EXCLUDE_FROM_ALL ${EXAMPLES_SOURCE_DIR}/column_example.cpp)
target_link_libraries(column_example DAS_SO)

add_custom_target(examples 
  DEPENDS 
    persistence_example
    query_example
    associations_example
    data_example
    column_example
)

#add_custom_target(tests
#  DEPENDS
#   # main_test
#    metadata_test
#    association_test
#    data_test
#    array_column_test
#    rollback_test
#)
'''
)
        f.close()
        
def _assemble_ddl(path):
    return "ddl_"+_hash.sha1(path).hexdigest()

def _assemble_db(db):
#    uri=""+db['host']+str(db['port'])+db['db_name']
#    return "db_"+_hash.sha1(uri).hexdigest()
    return db['alias']

def _generate_sub_cmake(dir_name,db_str,db_type,prefix,db,typelist,has_gc):
    f = open(_os.path.join(dir_name,'CMakeLists.txt'),'w')
    f.write(
'''
cmake_minimum_required(VERSION 2.6)
set(SQL_DIR ${CMAKE_CURRENT_BINARY_DIR})
set(DDL_LOCAL_SIGNATURE ${CMAKE_CURRENT_BINARY_DIR}/${DDL_SIGNATURE})

set(TYPE_NAMES ''')
    for i in typelist:
        if i != "essentialMetadata":
            f.write(' '+i)
    f.write(''')

set(SQL_FILES)
set(SQL_DDL_FILES)
set(SQL_CONSTRAINT_FILES)
foreach(type_name ${TYPE_NAMES})
  add_custom_command(
    OUTPUT ${TYPE_PREFIX}${type_name}.sql.orig
    COMMAND ${ODB_COMPILER}
            --database '''+db_type+'''
	    --generate-schema-only
            --omit-drop
            --fkeys-deferrable-mode not_deferrable
            --sql-suffix .sql.orig
            --profile boost/unordered
            --profile boost/optional

            -I${ODB_SOURCE_DIR}
            -I${CPP_INCLUDE_DIR}
            -I${ODB_INCLUDE_DIR}
            -x -I${Boost_INCLUDE_DIR} -x -I${BLITZ_INCLUDE_DIR}
	    ${ODB_SOURCE_DIR}/${TYPE_PREFIX}${type_name}.hpp
    DEPENDS ${DDL_LOCAL_SIGNATURE}
    COMMENT "Generating schema for type ${type_name} on '''+db['alias']+'''"
    )
  add_custom_command(
    OUTPUT ${TYPE_PREFIX}${type_name}.const.sql ${TYPE_PREFIX}${type_name}.ddl.sql
    COMMAND python ${PYTHON_SOURCE_DIR}/'''+db_type+'''_ddl_splitter.py
            ${TYPE_PREFIX}${type_name}.sql.orig
            ${TYPE_PREFIX}${type_name}.ddl.sql
            ${TYPE_PREFIX}${type_name}.const.sql
    DEPENDS ${DDL_LOCAL_SIGNATURE} ${TYPE_PREFIX}${type_name}.sql.orig
    COMMENT "Splitting schema and constraints for type ${type_name} on '''+db['alias']+'''"
    )
  list(APPEND SQL_FILES ${TYPE_PREFIX}${type_name}.sql.orig)
  list(APPEND SQL_CONSTRAINT_FILES ${TYPE_PREFIX}${type_name}.const.sql)
  list(APPEND SQL_DDL_FILES ${TYPE_PREFIX}${type_name}.ddl.sql)
endforeach()

add_custom_command(
  OUTPUT  SIGNATURE_CHECK ${DDL_LOCAL_SIGNATURE}
  COMMAND python cache_check.py
          ${DDL_INSTANCE_DIR}/'''+db['ddl']+'''
          ${DDL_LOCAL_SIGNATURE}
  WORKING_DIRECTORY ${PYTHON_SOURCE_DIR}
  COMMENT "Checking DDL signature"
)

add_custom_command(
  OUTPUT  DBMS_'''+prefix+db_str+'''
  COMMAND python db_access.py
          ${CONF_JSON_SCHEMA}
          ${DB_ACCESS_JSON_SCHEMA}
          ${CONF_JSON}
          ${DB_ACCESS_JSON}
          "'''+db['alias']+'''"
          ${SQL_DIR}
          ${DDL_SCHEMA}
          ${DDL_INSTANCE_DIR}/'''+db['ddl']+'''
          ${TYPE_PREFIX}
  DEPENDS ${DDL_LOCAL_SIGNATURE} ${SQL_CONSTRAINT_FILES} ${SQL_DDL_FILES}
  WORKING_DIRECTORY ${PYTHON_SOURCE_DIR}
  COMMENT "Executing '''+db['db_type']+''' on '''+db['alias']+'''"
)
''')
    if has_gc:
        f.write('''
add_executable(das_'''+db_str+'''_gc ${DDL_SOURCE_DIR}/'''+db_str+'''_gc.cpp)
target_link_libraries(das_'''+db_str+'''_gc DAS_SO)

install(
  TARGETS das_'''+db_str+'''_gc
  RUNTIME DESTINATION bin
  PERMISSIONS
    OWNER_READ
    OWNER_WRITE
    OWNER_EXECUTE
    GROUP_READ
    GROUP_EXECUTE
    WORLD_READ
    WORLD_EXECUTE 
)

set(CRON_SCHED_'''+db_str+''' "0 1 1,11,21 * *" CACHE STRING "crontab expression for the garbage collector deamon")
set(CRON_JOB_'''+db_str+''' ${CMAKE_INSTALL_PREFIX}/bin/das_'''+db_str+'''_gc)
#set(CRON_COMMAND_'''+db_str+''' cat <\(crontab -l | grep -v ${CRON_JOB_'''+db_str+'''}\) <\(echo "${CRON_SCHED_'''+db_str+'''} ${CRON_JOB_'''+db_str+'''}"\) | crontab - )

find_program( CRONTAB_EXEC crontab)

if(NOT CRONTAB_EXEC)
  MESSAGE(SEND_ERROR "no crontab executable")
endif()

add_custom_command(
  OUTPUT CRON_TAB_'''+db_str+'''_current
  COMMAND ${CRONTAB_EXEC} -l | grep -v ${CRON_JOB_'''+db_str+'''} > crontab.txt || true
  VERBATIM
)

add_custom_command(
  OUTPUT CRON_TAB_'''+db_str+'''_append_new
  COMMAND echo "${CRON_SCHED_'''+db_str+'''} ${CRON_JOB_'''+db_str+'''}" >> crontab.txt
  DEPENDS CRON_TAB_'''+db_str+'''_current
  VERBATIM
)

add_custom_command(
  OUTPUT CRON_TAB_'''+db_str+'''_install
  COMMAND ${CRONTAB_EXEC} crontab.txt
  DEPENDS CRON_TAB_'''+db_str+'''_append_new
  VERBATIM
)

add_custom_target(
  cron_job-'''+db_str+'''
  DEPENDS CRON_TAB_'''+db_str+'''_install
)

''')

    f.write('''
add_custom_target(
  schema-'''+db_str+'''
  DEPENDS SIGNATURE_CHECK ${SQL_FILES}
)

add_custom_target(
  '''+prefix+db_str+'''
  DEPENDS SIGNATURE_CHECK ${SQL_FILES} DBMS_'''+prefix+db_str+'''
)
'''
)
    f.close()
 
def generate_database_config(db_list,filename):
    f = open(filename+'.tmp','w')
    f.writelines(['''
#include "internal/database_config.hpp"
namespace das{

  void 
  DatabaseConfig::prepare_config()
  {
'''])
    for db in db_list:
        f.writelines(['''
    {
      DatabaseInfo &info = db_map_["''',str(db['alias']),'''"];
      info.host = "''',str(db['host']),'''";
      info.port = ''',str(db['port']),''';
      info.db_name = "''',str(db['db_name']),'''";
      info.db_type = "''',str(db['db_type']),'''";
'''])
        if(db.has_key('mysql_socket')):
             f.writelines(['      info.mysql_socket = "'+str(db['mysql_socket'])+'";\n'])  
        f.writelines(l for l in  storage_engine_tree_visit(db['storage_engine'],''))
        f.writelines(['    }\n'])
    f.writelines(['''  }

}
'''])
    f.close()

    _odb.comp_mv(filename,filename+'.tmp')
      
def storage_engine_tree_visit(tree,path):
    src = []
    if type(tree) is dict:
        for (name,subtree) in tree.items():
            if path == '':
                pt = name
            else:
                pt = path + '.' + name
            src.extend(storage_engine_tree_visit(subtree,pt))
    elif type(tree) is list:
        for item in tree:
            src.extend(storage_engine_list_visit(item,path))
    else:
        if isinstance(tree, (int, long, float)):
           src.append('      info.storage_engine.put("'+path+'",'+str(tree)+');\n')
        else: 
            src.append('      info.storage_engine.put("'+path+'","'+str(tree)+'");\n')

    return src

def storage_engine_list_visit(tree,path):
    src = []
    if type(tree) is dict:
        for (name,subtree) in tree.items():
            if path == '':
                pt = name
            else:
                pt = path + '.' + name
            src.extend(storage_engine_tree_visit(subtree,pt))
    elif type(tree) is list:
        for item in tree:
            src.extend(storage_engine_list_visit(item,path))
    else:
        if isinstance(tree, (int, long, float)):
           src.append('      info.storage_engine.add("'+path+'",'+str(tree)+');\n')
        else: 
            src.append('      info.storage_engine.add("'+path+'","'+str(tree)+'");\n')

    return src

    
if __name__ == '__main__':
    import sys
    db_dir          = sys.argv[ 1]
    cmake_dir       = sys.argv[ 2]
    ddl_headers_dir = sys.argv[ 3]
    ddl_source_dir  = sys.argv[ 4]
    swig_dir        = sys.argv[ 5]
    conf_schema     = sys.argv[ 6]
    conf            = sys.argv[ 7]
    ddl_schema      = sys.argv[ 8]
    ddl_dir         = sys.argv[ 9]
    target_prefix   = sys.argv[10]

    parser = _ddl.DdlParser(ddl_schema)
    c = JsonConfigParser(conf_schema)
    c.parse(conf)
    type_list = _ddl.DdlTypeList()
    type_set = set()
    for ddl in c.ddl_files:
        temp_type_list = parser.parse_ddl(_os.path.join(ddl_dir,ddl))
        #check advanced constraints per ddl
        validator = _odb.DdlInheritanceValidator(temp_type_list)
        if validator.check_redefined_keywords():
            exit(1)
        if validator.check_image_table_mismatch():
            exit(1)
        if validator.check_redefined_columns():
            exit(1)
        if validator.check_ancestor_loop():
            exit(1)
        if validator.check_association_loop():
            exit(1)
        #check redefined types consistency
        for (name,ddl_type) in temp_type_list.type_map.items():
            c.ddl_map.add_type(_assemble_ddl(ddl),ddl_type.name)
            type_set.add(ddl_type.name)
            if type_list.type_map.get(name,None) is None:
                type_list.type_map[name] = ddl_type
            else:
                upg = type_list.type_map[name].upgrade(ddl_type)
                if not upg.is_copy:
                    print "Error: redefined type "+name+"  doesn't match the previously defined one"
                    exit(1)

    #generate odb classes
    odb_generator = _odb.DdlOdbGenerator(ddl_headers_dir,ddl_source_dir,type_list)
    odb_generator.generate()

    #generate info classes
    info_generator = _info.DdlInfoGenerator(type_list,c.ddl_map,c.db_map,ddl_source_dir)
    type_list.accept(info_generator)

    #generate database config method
    generate_database_config(c._config,ddl_source_dir+"/database_config.cpp")

    #generate swig sources
    swig_generator = _swig.DdlSwigGenerator(swig_dir,type_list)
    swig_generator.generate()

    #generate per database header
    #c.generate_db_headers(ddl_source_dir)

    #generate garbage collectors
    c.generate_garbage_collectors(ddl_source_dir)

    #subdirectories and CmakeLists.txt
    c.generate_sub(db_dir,target_prefix)
    c.generate(db_dir,cmake_dir,ddl_source_dir,type_set,target_prefix)

