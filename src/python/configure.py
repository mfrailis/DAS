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
        for db in self._config:
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

    def generate_sub(self,output_dir):
        for db in self._config:
            db_str = db['ip']+"_"+str(db['port'])+"_"+db['db_name']
            self.sub_dirs.append(db_str)
            dir_name = _os.path.join(output_dir,db_str)
            _os.mkdir(dir_name)
            ddl_h = self.db_map[_assemble_db(db)]+'_types.h'
            _generate_sub_cmake(dir_name,db_str,db['db_type'],ddl_h)

        
def _assemble_ddl(path):
    return "ddl_"+_hash.sha1(path).hexdigest()

def _assemble_db(db):
    uri=""+db['ip']+str(db['port'])+db['db_name']
    return "db_"+_hash.sha1(uri).hexdigest()

def _generate_sub_cmake(dir_name,db_str,db_type,ddl_h):
    f = open(_os.path.join(dir_name,'CMakeLists.txt'),'w')
    f.write(
'''
cmake_minimum_required(VERSION 2.8)
add_custom_command(
  OUTPUT  DB_'''+db_str+'''
  COMMAND odb --database '''+db_type+'''
          --generate-query --generate-schema
          --at-once ${ODB_SOURCE_DIR}/'''+ddl_h+'''
    	  -x -I${ODB_SOURCE_DIR}
	  -x -I${CPP_INCLUDE_DIR}
  WORKING_DIRECTORY ${MAKE_CURRENT_BINARY_DIR}
  COMMENT "Built odb classes for '''+db_str+'''"
)

add_custom_target(
  db-'''+db_str+'''
  DEPENDS DB_'''+db_str+'''
)
'''
)
    f.close()
        
def generate(output_dir,dirs):
    f = open(_os.path.join(output_dir,'CMakeLists.txt'),'w')
    f.write('cmake_minimum_required(VERSION 2.8)\n')
    for l in dirs:
        f.write("add_subdirectory("+l+")\n")
    f.write(
'''
add_custom_target(
  db-all ALL
  DEPENDS'''
)
    for l in dirs:
        f.write(" db-"+l)
    f.write(
'''
)
'''
)
    f.close()        

    
if __name__ == '__main__':
    import sys
    output_dir   = sys.argv[1]
    conf_schema = sys.argv[2]
    conf   = sys.argv[3]
    ddl_schema   = sys.argv[4]
    ddl_dir      = sys.argv[5]
    

    parser = _ddl.DdlParser(ddl_schema)
    c = JsonConfigParser(conf_schema)
    c.parse(conf)
    type_list = _ddl.DdlTypeList()
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
            if type_list.type_map.get(name,None) is None:
                type_list.type_map[name] = ddl_type
            else:
                upg = type_list.type_map[name].upgrade(ddl_type)
                if not upg.is_copy:
                    print "Error: redefined type "+name+"  doesn't match the previously defined one"
                    exit(1)

    #generate odb classes
    odb_generator = _odb.DdlOdbGenerator(output_dir,type_list)
    odb_generator.generate()

    #generate info classes
    info_generator = _info.DdlInfoGenerator(type_list,c.ddl_map,c.db_map,output_dir)
    type_list.accept(info_generator)

    #generate per database header
    c.generate_db_headers(output_dir)

    #subdirectories and CmakeLists.txt
    c.generate_sub(output_dir)
    generate(output_dir,c.sub_dirs)
