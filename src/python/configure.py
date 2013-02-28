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
        self.db_map = {}
        self._config = []
        self.ddl_files = set()

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
        pass

def _assemble_ddl(path):
    return "ddl_"+_hash.sha1(path).hexdigest()

def _assemble_db(db):
    uri=""+db['ip']+str(db['port'])+db['db_name']
    return "db_"+_hash.sha1(uri).hexdigest()
    
if __name__ == '__main__':
    import sys
    configuration_file_name = sys.argv[1]
    output_dir = sys.argv[2]
    parser = _ddl.DdlParser('../../ddl/ddl.xsd')
    c = JsonConfigParser('../../ddl/schema.json')
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
            c.ddl_map.add_type(_assemble_ddl(ddl),ddl_type.name)
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

    print "ddl_map"
    for (type,list) in c.ddl_map._map.items():
        print type
        for l in list:
            print "  ",l
            
    print ""
    print "db_map"
    for (db,ddl) in c.db_map.items():
        print db+"  "+ddl

    #generate info classes
    info_generator = _info.DdlInfoGenerator(type_list,c.ddl_map,c.db_map,output_dir)
    type_list.accept(info_generator)
