import odb as _odb
import introspection as _info
import ddl as _ddl

class ConfigParser:
    def __init__(self):
        self._ddl_map = _info.DdlMap()
        self._db_map = {}
        self._ddl_map.add_type('ddl_test1',"essentialMetadata")
        self._ddl_map.add_type('ddl_test1',"lfiHkDaeSlowVoltage")
        self._ddl_map.add_type('ddl_test1',"testLogImage")
        self._ddl_map.add_type('ddl_test1',"testLog")
        
        self._ddl_map.add_type('ddl_test2',"essentialMetadata")
        self._ddl_map.add_type('ddl_test2',"lfiHkDaeSlowVoltage")

        self._db_map = { 'oracle1' : 'ddl_test1' , 'sqlite_cache' : 'ddl_test2', 'oracle2' : 'ddl_test1' }
        #DUMMY START

        #DUMMY END
        #TODO
        pass
    
    def get_db_list(self):
        return self._db_map.keys()
        pass

    def get_ddl_file_list(self):
        #DUMMY
        return ['/home/sartor/workspace/CIWS/DAS/ddl/dasInstance.xml']
        #TODO
        pass
    
    def get_schema_file(self):
        #DUMMY
        return '/home/sartor/workspace/CIWS/DAS/ddl/ddl.xsd'
        #TODO        
        pass

    def parse(self,config_file):
        #TODO
        pass

    def get_ddl_map(self):
        return self._ddl_map

    def get_database_map(self):
        return self._db_map
    
if __name__ == '__main__':
    import sys
    configuration_file_name = sys.argv[1]
    output_dir = sys.argv[2]
    #FIXME
    parser = _ddl.DdlParser(sys.argv[3])    
    c = ConfigParser()
    c.parse(configuration_file_name)
    type_list = _ddl.DdlTypeList()
    for ddl in c.get_ddl_file_list():
 #       parser = _ddl.DdlParser(c.get_schema_file)
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
    info_generator = _info.DdlInfoGenerator(c.get_ddl_map(),type_list,c.get_database_map(),output_dir)
    type_list.accept(info_generator)
