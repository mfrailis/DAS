import odb as _odb
import os as _os

class DdlMap:
    def __init__(self):
        self._map = {}
    
    def add_type(self,ddl_name,ddl_type):
        if self._map.get(ddl_type,None) is None:
            self._map[ddl_type] = []
        self._map[ddl_type].append(ddl_name)
        
    def get_ddl_list(self, type_name):
        return self._map.get(type_name,[])
        
class DdlInfoGenerator(_odb.DdlVisitor):
    def __init__(self,ddl_map,type_list,db_map,source_dir):
        self._type_list = type_list
        self._ddl_map = ddl_map
        self._db_map = db_map
        self._src_dir = source_dir
        self._init_keywords = []
        self._init_columns = []
        self._DdlInfo_children = {}

        self._keywords = []
        self._columns = []


    def _clean_env(self):
        self._keywords = []
        self._columns = []

    def visit_datatype(self,data_type):
        self._clean_env()
        self._get_keywords(data_type.name)
        self._get_columns(data_type.name)

        for k in self._keywords:
            if k.description is None:
                d = ""
            else:
                d = k.description
            self._init_keywords.append('all_keywords_["'+data_type.name+'"]["'+k.name+'"] = KeywordInfo("'+k.name+'","'+k.ktype+'","'+k.unit+'","'+d+'");')
        for c in self._columns:
            if c.description is None:
                d = ""
            else:
                d = c.description
            self._init_columns.append('all_columns_["'+data_type.name+'"]["'+c.name+'"] = ColumnInfo("'+c.name+'","'+c.ctype+'","'+c.unit+'","'+d+'","'+c.max_string_length+'");')

        ddl_list = self._ddl_map.get_ddl_list(data_type.name)
        for ddl in ddl_list:
            if self._DdlInfo_children.get(ddl,None) is None:
                self._DdlInfo_children[ddl] = []
            self._DdlInfo_children[ddl].append('keywords_["'+data_type.name+'"] = &all_keywords_["'+data_type.name+'"];')
            if self._columns:
                self._DdlInfo_children[ddl].append('columns_["'+data_type.name+'"] = &all_columns_["'+data_type.name+'"];')
                
    def visit_type_list(self, type_list):
        f = open(_os.path.join(self._src_dir, 'ddl_info.cpp'), 'w')
        f.writelines('#include "'+ddl+'.hpp"\n' for ddl in set(self._db_map.values()))
        f.writelines(['\nvoid\n','DdlInfo::init()\n','{\n'])
        f.writelines("  "+l + "\n" for l in self._init_columns)
        f.writelines("  "+l + "\n" for l in self._init_keywords)
        f.writelines(['\n'])
        f.writelines('  ddl_map_["'+db+'"] = '+ddl+'::get_instance();\n' for (db,ddl) in self._db_map.items())
        f.writelines(['}\n'])
        f.close
 
        for ddl_name in self._DdlInfo_children.keys():
            f = open(_os.path.join(self._src_dir, ddl_name+'.hpp'), 'w')
            const = ['  '+ddl_name+'()\n','  {\n']
            const.extend("    "+l+'\n' for l in self._DdlInfo_children[ddl_name])
            const.append('  }\n')
            _write_ddl_class(f,ddl_name,const)
            f.close()
            

    def _get_keywords(self, type_name):
        ddl_type = self._type_list.type_map[type_name]
        if ddl_type.metadata is not None:
            self._keywords.extend(ddl_type.metadata.keywords.values())
        if ddl_type.ancestor is not None:
            self._get_keywords(ddl_type.ancestor.atype)
            
    def _get_columns(self, type_name):
        ddl_type = self._type_list.type_map[type_name]
        if ddl_type.data is not None:
            if ddl_type.data.isImage():
                return
            self._columns.extend(ddl_type.data.data_obj.columns.values())
        if ddl_type.ancestor is not None:
            self._get_columns(ddl_type.ancestor.atype)

def _write_ddl_class(f, ddl_name, constructor):
    source = [
'''#ifndef '''+ ddl_name.upper()+'''_HPP
#define '''+ ddl_name.upper()+'''_HPP

#include "ddl_info.hpp"

class '''+ddl_name+''' : public DdlInfo
{
public:
  static '''+ddl_name+'''*
  get_instance()
  {
    if(! instance_)
      {
	instance_ = new '''+ddl_name+'''();
      }
    return instance_;
  }

  virtual
  const KeywordInfo&
  get_keyword_info(std::string type_name, std::string keyword_name)
    throw(std::out_of_range) const
  {
    return keywords_.at(type_name)->at(keyword_name);
  }

  virtual
  const ColumnInfo&
  get_column_info(std::string type_name, std::string column_name)
    throw(std::out_of_range) const
  {
    return columns_.at(type_name)->at(column_name);
  }  
  
private:
  boost::unordered_map<string, DdlInfo::keyword_map* > keywords_;
  boost::unordered_map<string, DdlInfo::column_map* > columns_;
  static '''+ddl_name+'''* instance_;
''']
    source.extend(constructor)
    source.append(
'''
}

'''+ddl_name+'''* '''+ddl_name+'''::instance = 0;
#endif
''')
    f.writelines(l for l in source)


if __name__ == "__main__":
    import sys
    import ddl
    schema_file_name = sys.argv[1]
    das_instance_file = sys.argv[2]
    output_dir = sys.argv[3]

    parser = ddl.DdlParser(schema_file_name)
    instance = parser.parse_ddl(das_instance_file)

    m = DdlMap()
    m.add_type('ddl_test1',"essentialMetadata")
    m.add_type('ddl_test1',"lfiHkDaeSlowVoltage")
    m.add_type('ddl_test1',"testLogImage")
    m.add_type('ddl_test1',"testLog")
    
    m.add_type('ddl_test2',"essentialMetadata")
    m.add_type('ddl_test2',"lfiHkDaeSlowVoltage")
    
    db_map = { 'oracle1' : 'ddl_test1' , 'sqlite_cache' : 'ddl_test2', 'oracle2' : 'ddl_test1' }
    ddl_generator = DdlInfoGenerator(m,instance,db_map,output_dir)
    instance.accept(ddl_generator)
