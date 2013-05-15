import odb as _odb
import introspection as _info
import ddl as _ddl
import json as _j
import jsonschema as _js
import hashlib as _hash
import os as _os

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

    def generate_sub(self,db_dir,t_pre):
        for db in self._config:
            #db_str = db['host']+"_"+str(db['port'])+"_"+db['db_name']
            db_str = db['alias']
            self.sub_dirs.append(db_str)
            dir_name = _os.path.join(db_dir,db_str)
            if not _os.path.exists(dir_name):
                _os.mkdir(dir_name)
#            ddl_h   = self.db_map[_assemble_db(db)]+'_types.h'
#            ddl_sql = self.db_map[_assemble_db(db)]+'_types.sql'
            typelist=self.ddl_map.get_type_list(_assemble_ddl(db['ddl']))
            _generate_sub_cmake(dir_name,db_str,db['db_type'], t_pre,db,typelist)
    
    def generate(self,db_dir_,cmake_dir,types,prefix):
        db_vendors = {};
        for db in self._config:
            db_dir =  _os.path.join(db_dir_,db['db_type'])
            if not _os.path.exists(db_dir):
                _os.mkdir(db_dir)        
            db_vendors[db['db_type']] = db_dir


        f = open(_os.path.join(cmake_dir,'CMakeLists.txt'),'w')
        f.write('cmake_minimum_required(VERSION 2.8)\n')
        f.write('set(TYPE_NAMES_ALL \n')
        for i in types:
            if i != "essentialMetadata":
                f.write(' '+i)
        f.write(')')
        f.write('''
set(TYPES_CPP ${TYPE_PREFIX}info.cpp)
foreach(type_name ${TYPE_NAMES_ALL})
  list(APPEND TYPES_CPP ${TYPE_PREFIX}${type_name}.cpp)
endforeach()
''')

        for (db_type,db_dir_) in db_vendors.items():
            db_dir = "${ODB_OUTPUT_DIR}/"+db_type
            f.write('''
file(MAKE_DIRECTORY '''+db_dir+''')
list(APPEND ODB_CXX '''+db_dir+'''/aux_query-odb.cxx)
list(APPEND ODB_CXX '''+db_dir+'''/DasObject-odb.cxx)
list(APPEND ODB_CXX '''+db_dir+'''/column-odb.cxx)
list(APPEND ODB_CXX '''+db_dir+'''/image-odb.cxx)

foreach(type_name ${TYPE_NAMES_ALL})
  add_custom_command(
    OUTPUT '''+db_dir+'''/${TYPE_PREFIX}${type_name}-odb.cxx
    COMMAND odb
            --output-dir '''+db_dir+'''
            --database '''+db_type+'''
            --generate-query
            --generate-session
            --include-regex "%(column.hpp)|(image.hpp)%ddl/$1$2%"
            --include-regex "%aux_query.hpp%../src/cpp/aux_query.hpp%"
            --include-regex "%ddl/(.+).hxx%$1.hxx%"
    --include-regex "%ddl_(.+).hpp%../../ddl_$1.hpp%"


            --default-pointer std::tr1::shared_ptr
	    -I${ODB_SOURCE_DIR}
            -I${CPP_INCLUDE_DIR}
	    ${ODB_SOURCE_DIR}/${TYPE_PREFIX}${type_name}.hpp
    DEPENDS ${DDL_LOCAL_SIGNATURE}
    COMMENT "Generating odb class for type ${type_name} for '''+db_type+''' DBMS"
    VERBATIM
    )
  list(APPEND ODB_CXX '''+db_dir+'''/${TYPE_PREFIX}${type_name}-odb.cxx)
endforeach()
        
add_custom_command(
OUTPUT '''+db_dir+'''/DasObject-odb.cxx
COMMAND odb
    --output-dir '''+db_dir+'''
    --database '''+db_type+'''
    --generate-query
    --generate-session
    --include-regex "%(column.hpp)|(image.hpp)%ddl/$1$2%"
    --include-regex "%aux_query.hpp%../src/cpp/aux_query.hpp%"
    --include-regex "%ddl_(.+).hpp%../../ddl_$1.hpp%"


    --default-pointer std::tr1::shared_ptr
    -I${ODB_SOURCE_DIR}
    -I${CPP_INCLUDE_DIR}
    ${CPP_INCLUDE_DIR}/DasObject.hpp
COMMENT "Generating odb DasObject class for '''+db_type+''' DBMS"
VERBATIM
)

add_custom_command(
OUTPUT '''+db_dir+'''/aux_query-odb.cxx
COMMAND odb
    --output-dir '''+db_dir+'''
    --database '''+db_type+'''
    --generate-query
    --generate-session
    --include-regex "%(column.hpp)|(image.hpp)%ddl/$1$2%"
    --include-regex "%aux_query.hpp%../src/cpp/aux_query.hpp%"
    --include-regex "%ddl_(.+).hpp%../../ddl_$1.hpp%"


    --default-pointer std::tr1::shared_ptr 
    -I${ODB_SOURCE_DIR}
    -I${CPP_INCLUDE_DIR}
    ${CPP_INCLUDE_INTERNAL_DIR}/aux_query.hpp
COMMENT "Generating odb auxiliary query on for '''+db_type+''' DBMS"
VERBATIM
)

add_custom_command(
OUTPUT '''+db_dir+'''/column-odb.cxx
COMMAND odb
    --output-dir '''+db_dir+'''
    --database '''+db_type+'''
    --generate-query
    --generate-session
    --include-regex "%(column.hpp)|(image.hpp)%ddl/$1$2%"
    --default-pointer std::tr1::shared_ptr
    -I${ODB_SOURCE_DIR}
    -I${CPP_INCLUDE_DIR}
    ${CPP_INCLUDE_DIR}/ddl/column.hpp
COMMENT "Generating odb column data support for '''+db_type+''' DBMS"
VERBATIM
)

add_custom_command(
OUTPUT '''+db_dir+'''/image-odb.cxx
COMMAND odb
    --output-dir '''+db_dir+'''
    --database '''+db_type+'''
    --generate-query
    --generate-session
    --include-regex "%(column.hpp)|(image.hpp)%ddl/$1$2%"
    --default-pointer std::tr1::shared_ptr 
    -I${ODB_SOURCE_DIR}
    -I${CPP_INCLUDE_DIR}
    ${CPP_INCLUDE_DIR}/ddl/image.hpp
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

add_custom_target(
  schema-all ALL
  DEPENDS'''
)
        for l in self.sub_dirs:
            f.write("\n      schema-"+l)
        f.write(
'''
)
'''
)
        f.write(
'''
add_custom_target(
  '''+prefix+'''all ALL
  DEPENDS'''
)
        for l in self.sub_dirs:
            f.write("\n      "+prefix+l)
        f.write(
'''
)


#foreach(odb_ ${ODB_CXX})
#  message(STATUS "odb  ${odb_}")
#endforeach()

#foreach(type_ ${TYPES_CPP})
#  message(STATUS "type ${type_}")
#endforeach()

#foreach(das_src_ ${DAS_SRC})
#  message(STATUS "das  ${das_src_}")
#endforeach()

#foreach(das_ql_src_ ${DAS_QL_SRC})
#  message(STATUS "ql   ${das_ql_src_}")
#endforeach()

foreach(type_ ${TYPE_NAMES_ALL} )
  message(STATUS "Generated type ${type_}")
endforeach()

add_library(das SHARED ${DAS_QL_SRC} ${DAS_SRC} ${TYPES_CPP} ${ODB_CXX})

add_executable(test ${TEST_SOURCE_DIR}/main.cpp)
target_link_libraries(test das)
target_link_libraries(test odb)
target_link_libraries(test odb-mysql)
'''
)
        f.close()
        
def _assemble_ddl(path):
    return "ddl_"+_hash.sha1(path).hexdigest()

def _assemble_db(db):
#    uri=""+db['host']+str(db['port'])+db['db_name']
#    return "db_"+_hash.sha1(uri).hexdigest()
    return db['alias']

def _generate_sub_cmake(dir_name,db_str,db_type,prefix,db,typelist):
    f = open(_os.path.join(dir_name,'CMakeLists.txt'),'w')
    f.write(
'''
cmake_minimum_required(VERSION 2.8)
set(SQL_DIR ${CMAKE_CURRENT_BINARY_DIR})
set(DDL_LOCAL_SIGNATURE ${CMAKE_CURRENT_BINARY_DIR}/${DDL_SIGNATURE})

set(TYPE_NAMES ''')
    for i in typelist:
        if i != "essentialMetadata":
            f.write(' '+i)
    f.write(''')

set(SQL_FILES)
foreach(type_name ${TYPE_NAMES})
  add_custom_command(
    OUTPUT ${TYPE_PREFIX}${type_name}.sql
    COMMAND odb
            --database '''+db_type+'''
	    --generate-schema-only
            --omit-drop
            -I${ODB_SOURCE_DIR}
            -I${CPP_INCLUDE_DIR}
	    ${ODB_SOURCE_DIR}/${TYPE_PREFIX}${type_name}.hpp
    DEPENDS ${DDL_LOCAL_SIGNATURE}
    COMMENT "Generating schema for type ${type_name} on '''+db['alias']+'''"
    )
  list(APPEND SQL_FILES ${TYPE_PREFIX}${type_name}.sql)
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
  DEPENDS ${DDL_LOCAL_SIGNATURE}
  WORKING_DIRECTORY ${PYTHON_SOURCE_DIR}
  COMMENT "Executing '''+db['db_type']+''' on '''+db['alias']+'''"
)



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
        
      

    
if __name__ == '__main__':
    import sys
    db_dir        = sys.argv[1]
    cmake_dir     = sys.argv[2]
    conf_schema   = sys.argv[3]
    conf          = sys.argv[4]
    ddl_schema    = sys.argv[5]
    ddl_dir       = sys.argv[6]
    target_prefix = sys.argv[7]

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
    odb_generator = _odb.DdlOdbGenerator(cmake_dir,type_list)
    odb_generator.generate()

    #generate info classes
    info_generator = _info.DdlInfoGenerator(type_list,c.ddl_map,c.db_map,cmake_dir)
    type_list.accept(info_generator)

    #generate per database header
    c.generate_db_headers(cmake_dir)

    #subdirectories and CmakeLists.txt
    c.generate_sub(db_dir,target_prefix)
    c.generate(db_dir,cmake_dir,type_set,target_prefix)

