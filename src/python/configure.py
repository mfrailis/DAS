import odb as _odb
import introspection as _info
import ddl as _ddl
import json as _j
import jsonschema as _js
import hashlib as _hash

class JsonConfigParser:
    def __init__(self,schema_path):
        f = open(schema_path,'r')
        self._schema = _j.load(f)
        f.close()

        self.ddl_map = _info.DdlMap()
        self._db_map = {}
        self._config = []
        self.ddl_files = set()

    def parse(self,config_file):
        f = open(config_file,'r')
        self._config = _j.load(f)
        f.close()
        #TODO do validation
        #_js.validate(config_file,self._schema)
        for db in self._config:
            self._db_map[_assemble_name(db)]=_assemble_ddl(db['ddl'])
            self.ddl_files.add(db['ddl'])
    def get_db_list(self):
        return self._db_map.keys()
        pass

    def get_database_map(self):
        return self._db_map

def _assemble_ddl(path):
    return "ddl_"+_hash.sha1(path).hexdigest()

def _assemble_db(db):
    uri=""+db['ip']+db['port']+db['db_name']
    return "db_"+_hash.sha1(uri).hexdigest()
    
if __name__ == '__main__':
    import sys
    configuration_file_name = sys.argv[1]
    output_dir = sys.argv[2]
    #FIXME
    parser = _ddl.DdlParser(sys.argv[3])
    #FIXME json schema file path
    c = JsonConfigParser(sys.argv[4])
    c.parse(configuration_file_name)
    type_list = _ddl.DdlTypeList()
    for ddl in c.ddl_files:
        temp_type_list = parser.parse_ddl(ddl)
        #check advanced constraints per ddl
        validator = _odb.DdlInheritanceValidator(temp_type_list)
        if validator.check_redefined_keywords():
            exit(1)
        if validator.check_image_table_mismatch():
            exit(1)
        if validator.check_redefined_columns():
            exit(1)
        #check redefined types consistency
        for (name,ddl_type) in temp_type_list.type_map.items():
            c.ddl_map.add_type(_assemble_ddl(ddl),ddl_type)
            if type_list.type_map.get(name,None) is None:
                type_list.type_map[name] = ddl_type
            else:
                upg = type_list.type_map[name].upgrade(ddl_type)
                if not upg.is_copy:
                    print "Error: redefined type "+name+"  doesn't match the previously defined one"
                    exit(1)
    #FIME redoundant
    #generate odb classes
    valid = _odb.DdlInheritanceValidator(type_list)
    odb_generator = _odb.DdlOdbGenerator(output_dir,valid)
    type_list.accept(odb_generator)

    #generate info classes
    info_generator = _info.DdlInfoGenerator(c.ddl_map(),type_list,c.get_database_map(),output_dir)
    type_list.accept(info_generator)
