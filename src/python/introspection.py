import odb as _odb
import os as _os

class DdlMap:
    def __init__(self):
        self._map_type = {}
        self._map_ddl = {}

    def add_type(self,ddl_name,ddl_type):
        if self._map_type.get(ddl_type,None) is None:
            self._map_type[ddl_type] = []
        self._map_type[ddl_type].append(ddl_name)
        if self._map_ddl.get(ddl_name,None) is None:
            self._map_ddl[ddl_name] = []
        self._map_ddl[ddl_name].append(ddl_type)

    def get_ddl_list(self, type_name):
        return self._map_type.get(type_name,[])

    def get_type_list(self, ddl_name):
        return self._map_ddl.get(ddl_name,[])

class DdlInfoGenerator(_odb.DdlVisitor):
    def __init__(self,type_list,ddl_map,db_map,source_dir):
        self._type_list = type_list
        self._ddl_map = ddl_map
        self._db_map = db_map
        self._src_dir = source_dir
        self._init_keywords = []
        self._init_columns = []
        self._init_images = []
        self._init_associations = []
        self._init_types = []
        self._DdlInfo_children = {}

        self._keywords = []
        self._columns = []
        self._associations = []

    def _clean_env(self):
        self._keywords = []
        self._columns = []
        self._associations = []

    def visit_datatype(self,data_type):
        self._clean_env()
        self._get_keywords(data_type.name)
        self._get_columns(data_type.name)
        self._get_associations(data_type.name)
        
        if data_type.name != "essentialMetadata":
            self._init_types.append('all_types_["'+data_type.name+'"].store_as_ = "'+self._get_store_as(data_type.name)+'";')
            self._init_types.append('all_types_["'+data_type.name+'"].ctor_.reset(new TypeCtorImp<'+data_type.name+'>);')
            self._init_types.append('all_types_["'+data_type.name+'"].type_ = &typeid('+data_type.name+');')

        self._init_keywords.append('all_types_["'+data_type.name+'"].keywords_.insert(std::pair<std::string,KeywordInfo>("das_id",KeywordInfo("das_id","int64","none","object id")));')
        for k in self._keywords:
            if k.description is None:
                d = ""
            else:
                d = k.description
            self._init_keywords.append('all_types_["'+data_type.name+'"].keywords_.insert(std::pair<std::string,KeywordInfo>("'+k.name+'",KeywordInfo("'+k.name+'","'+k.ktype+'","'+k.unit+'","'+d+'")));')

        for c in self._columns:
            if c.description is None:
                d = ""
            else:
                d = c.description
            self._init_columns.append('all_types_["'+data_type.name+'"].columns_.insert(std::pair<std::string,ColumnInfo>("'+c.name+'",ColumnInfo("'+c.name+'","'+c.ctype+'","'+c.unit+'","'+c.array_size+'","'+d+'",'+c.max_string_length+')));')

        for (ass_name,association) in self._associations:
            self._init_associations.append('all_types_["'+data_type.name+'"].associations_.insert(std::pair<std::string,AssociationInfo>("'+ass_name+'",'+_association_info_gen(data_type.name,ass_name,association)+'));')
        if data_type.data is not None and data_type.data.isImage():
            self._init_images.append('all_types_["'+data_type.name+'"].image_.reset(new ImageInfo("'+data_type.data.data_obj.pix_type+'",'+data_type.data.data_obj.dimensions+'));')

        ddl_list = self._ddl_map.get_ddl_list(data_type.name)
        for ddl in ddl_list:
            if self._DdlInfo_children.get(ddl,None) is None:
                self._DdlInfo_children[ddl] = []
            self._DdlInfo_children[ddl].append('types_["'+data_type.name+'"] = &all_types_["'+data_type.name+'"];')

    def visit_type_list(self,_):
        ddl_info_path = _os.path.join(self._src_dir, 'ddl_info.cpp')
        f = open(ddl_info_path+'.tmp', 'w')
        f.writelines('#include "'+ddl+'.hpp"\n' for ddl in set(self._db_map.values()))
        f.writelines(['#include "ddl/types/ddl_types.hpp"'])
        f.writelines(['\nvoid\n','DdlInfo::init()\n','{\n'])
        f.writelines("  "+l + "\n" for l in self._init_types)
        f.writelines("  "+l + "\n" for l in self._init_images)
        f.writelines("  "+l + "\n" for l in self._init_columns)
        f.writelines("  "+l + "\n" for l in self._init_keywords)
        f.writelines("  "+l + "\n" for l in self._init_associations)
        f.writelines(['\n'])
        f.writelines('  ddl_map_["'+db+'"] = '+ddl+'::get_instance();\n' for (db,ddl) in self._db_map.items())
        f.writelines(['}\n'])
        f.close()
        
        _odb.comp_mv(ddl_info_path,ddl_info_path+'.tmp')

        db_plf_path = _os.path.join(self._src_dir, 'database_plf.cpp')
        f = open(db_plf_path+'.tmp', 'w')
        f.writelines(['''
#include "plf/database.hpp"
#include "ddl/types.hpp"

boost::unordered_map< std::string, shared_ptr<das::plf::Functor> > das::plf::Database::f_;

namespace das{
  namespace plf{

    void Database::init()
    {
      if(!f_.empty()) return;
'''])
        for t in self._type_list.type_map.keys():
            if t != "essentialMetadata":
                f.writelines(['      f_.insert(std::pair< std::string, shared_ptr<Functor> >("'+t+'",shared_ptr<Functor>(new FunctorImp<'+t+'>)));\n']) 

        f.writelines(['''
    }
  }
}
'''])
        f.close()
        
        _odb.comp_mv(db_plf_path,db_plf_path+'.tmp')

        for ddl_name in self._DdlInfo_children.keys():
            ddl_path = _os.path.join(self._src_dir, ddl_name+'.hpp')
            f = open(ddl_path+'.tmp', 'w')
            const = ['  '+ddl_name+'()\n','  {\n']
            const.extend("    "+l+'\n' for l in self._DdlInfo_children[ddl_name])
            const.append('  }\n')
            _write_ddl_class(f,ddl_name,const)
            f.close()
            _odb.comp_mv(ddl_path,ddl_path+'.tmp')

    def _get_keywords(self, type_name):
        ddl_type = self._type_list.type_map[type_name]
        if ddl_type.metadata is not None:
            self._keywords.extend(ddl_type.metadata.keywords.values())
        if ddl_type.ancestor != ddl_type.name:
            self._get_keywords(ddl_type.ancestor)
            
    def _get_columns(self, type_name):
        ddl_type = self._type_list.type_map[type_name]
        if ddl_type.data is not None:
            if ddl_type.data.isImage():
                return
            self._columns.extend(ddl_type.data.data_obj.columns.values())
        if ddl_type.ancestor != ddl_type.name:
            self._get_columns(ddl_type.ancestor)

    def _get_associations(self, type_name):
        ddl_type = self._type_list.type_map[type_name]
        for (ass_name,association) in ddl_type.associated.items():
            self._associations.append((ass_name,association))
        if ddl_type.ancestor != ddl_type.name:
            self._get_associations(ddl_type.ancestor)

    def _get_store_as(self,type_name):
        ddl_type = self._type_list.type_map[type_name]
        if ddl_type.data is not None:
            return ddl_type.data.store_as
        elif ddl_type.ancestor != ddl_type.name:
            return self._get_store_as(ddl_type.ancestor)
        else:
            return "none"

def _write_ddl_class(f, ddl_name, constructor):
    source = [
'''#ifndef '''+ ddl_name.upper()+'''_HPP
#define '''+ ddl_name.upper()+'''_HPP

#include "ddl/info.hpp"

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
  get_keyword_info(const std::string &type_name, const std::string &keyword_name)
  const throw (std::out_of_range) {
    return types_.at(type_name)->get_keyword_info(keyword_name);
  }

  virtual
  const ColumnInfo&
  get_column_info(const std::string &type_name, const std::string &column_name)
  const throw (std::out_of_range) {
    return types_.at(type_name)->get_column_info(column_name);
  }

  virtual
  const ImageInfo&
  get_image_info(const std::string &type_name)
  const throw (std::out_of_range) {
    return types_.at(type_name)->get_image_info();
  }
    
  virtual
  const AssociationInfo&
  get_association_info(const std::string &type_name, const std::string &association_name)
  const throw (std::out_of_range) {
    return types_.at(type_name)->get_association_info(association_name);
  }  
    
  virtual
  const TypeInfo&
  get_type_info(const std::string &type_name)
  const throw (std::out_of_range) {
    return *(types_.at(type_name));
  }


private:
  boost::unordered_map< std::string, TypeInfo* > types_;
  static '''+ddl_name+'''* instance_;
''']
    source.extend(constructor)
    source.append(
'''
};

'''+ddl_name+'''* '''+ddl_name+'''::instance_ = 0;
#endif
''')
    f.writelines(l for l in source)

def _association_info_gen(base_name,ass_name,association):
    if association.multiplicity == 'many' :
        if association.relation == 'shared':
            return 'AssociationInfo("'+association.atype+'","'+base_name+'_'+ass_name+'","value","object_id",new AssociationAccessImp_many<'+base_name+','+association.atype+'>(&'+base_name+'::'+ass_name+',&'+base_name+'::'+ass_name+'))'
        else:
            return 'AssociationInfo("'+association.atype+'","'+association.atype+'","","'+base_name+'_'+ass_name+'",new AssociationAccessImp_many<'+base_name+','+association.atype+'>(&'+base_name+'::'+ass_name+',&'+base_name+'::'+ass_name+'))'
    else:
        if association.relation == 'shared':
            return 'AssociationInfo("'+association.atype+'","'+base_name+'","'+ass_name+'","",new AssociationAccessImp_one<'+base_name+','+association.atype+'>(&'+base_name+'::'+ass_name+',&'+base_name+'::'+ass_name+'))'
        else:
            return 'AssociationInfo("'+association.atype+'","'+association.atype+'","","'+base_name+'_'+ass_name+'",new AssociationAccessImp_one<'+base_name+','+association.atype+'>(&'+base_name+'::'+ass_name+',&'+base_name+'::'+ass_name+'))'
            
    
            


if __name__ == "__main__":
    import sys
    import ddl
    das_instance_file = sys.argv[1]
    output_dir = sys.argv[2]

    parser = ddl.DdlParser('../../ddl/ddl.xsd')
    instance = parser.parse_ddl(das_instance_file)

    ddl_map = DdlMap()
    ddl_map.add_type('ddl_test1',"essentialMetadata")
    ddl_map.add_type('ddl_test1',"lfiHkDaeSlowVoltage")
    ddl_map.add_type('ddl_test1',"testLogImage")
    ddl_map.add_type('ddl_test1',"testLog")
    
    ddl_map.add_type('ddl_test2',"essentialMetadata")
    ddl_map.add_type('ddl_test2',"lfiHkDaeSlowVoltage")
    
    db_map = { 'oracle1' : 'ddl_test1' , 'sqlite_cache' : 'ddl_test2', 'oracle2' : 'ddl_test1' }


    print "ddl_map"
    for (type_,list_) in ddl_map._map.items():
        print type_
        for l in list_:
            print "  ",l
            
    print ""
    print "db_map"
    for (db,ddl) in db_map.items():
        print db+"  "+ddl


    ddl_generator = DdlInfoGenerator(instance,ddl_map,db_map,output_dir)
    instance.accept(ddl_generator)
