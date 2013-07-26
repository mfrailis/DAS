import os as _os

import association_many_shared as _a_ms
import association_many_exclusive as _a_me
import association_one_shared as _a_os
import association_one_exclusive as _a_oe

class DdlVisitor:
  def name(self,name):
    pass

  def clean_env(self):
    pass
    
  def visit_type_list(self, type_list):
    pass
      
  def visit_datatype(self, datatype):
    pass
    
  def visit_associated(self, associated):
    pass
 
  def visit_keyword(self,keyword):
    pass 

  def visit_metadata(self, metadata):
    pass
   
  def visit_binary_table(self,binaryTable):
    pass
    
  def visit_column(self,column):
    pass
     
  def visit_data(self,data):
    pass
    
  def visit_image(self, image):
    pass  



class DdlOdbGenerator(DdlVisitor):
  
  
  KTYPE_MAP = {'byte':'signed char', 'char':'char', 'int16':'short', 'int32':'int', 
               'int64':'long long', 'float32':'float', 'float64':'double', 
               'boolean':'bool', 'string':'std::string', 'text' : 'CBLOB'}

  def __init__(self, header_dir, source_dir, instance):
      
    self._src_dir = source_dir
    self._hdr_dir = header_dir
    self._sources = []
    self._instance = instance
 
    self.clean_env()

  def clean_env(self):
    self._header = []
    self._src_header = ['#include "internal/log.hpp"']
    self._inherit = ""
    self._class_name = ""
    self._forward_section = []
    self._public_section = []
    self._type_defs = []
    self._protected_section = []
    self._private_section = []
    self._init_list = []
    self._default_init = []
    self._init = []
    self._friends = ["friend class odb::access;","friend class das::tpl::Transaction;","friend class das::tpl::DbBundle;"]
    self._has_associations = False
    self._store_as = None
    self._keyword_touples = []
    self._assoc_touples = []
    self._column_type = []
    self._src_body = []

  def name(self,name):
    self._class_name=name

  def generate(self):
    self.clean_env()
    self._instance.accept(self)

  def visit_type_list(self, _ ):  
    lines = []
#    lines.append('#include "ddl_BLOB.hpp"')
    for k in self._sources:
      lines.append('#include "ddl_' + k +'.hpp"')
      #TODO change mysql in common for multi-database support
      lines.append('#include "mysql/ddl_' + k +'-odb.hxx"')
     
    if len(lines):
      lines.insert(0,
'''
#ifndef DAS_DDL_TYPES_H
#define DAS_DDL_TYPES_H

''')
      lines.append('#endif')
      f = open(_os.path.join(self._hdr_dir, "ddl_types.hpp"), 'w')
      f.writelines(l + "\n" for l in lines)
      f.close()
      
  def visit_datatype(self, datatype):
    if datatype.name == "essentialMetadata":
      self.clean_env()
      return
    lines = []
    intro = []
    self._class_name = datatype.name
    hdr_name = "ddl_"+ self._class_name +".hpp"
    idr_name = "ddl_"+ self._class_name +".ipp"
    src_name = "ddl_"+ self._class_name +".cpp" 
    self._sources.append(self._class_name)
    macro = "DAS_" + datatype.name.upper() + "_HPP"
    lines.append("#ifndef " + macro + "\n#define " + macro)
    lines.append("#include <odb/core.hxx>")
    lines.append("#include <vector>")
    lines.append("#include <map>")
    lines.append('#include "../info.hpp"')
    if datatype.ancestor != "essentialMetadata":
      self._header.append('#include "ddl_' + datatype.ancestor + '.hpp"')
      self._inherit = datatype.ancestor
    else:
      self._header.append('#include "../../das_object.hpp"')
      self._inherit = "DasObject"

      
    if datatype.data is not None:
      self._header.append('#include "../column.hpp"')
      self._header.append('#include "../../internal/column_config.hpp"')
    
    intro.append("#pragma db object\nclass " + datatype.name)
    if self._inherit != "":
      intro[-1] += ": public "+self._inherit
    else:
      intro[-1] += ": public DasObject"
      lines.append('#include "../../das_object.hpp"')

    intro.append("{")
    
    if self._inherit == "":    
      self._private_section.insert(0,"#pragma db id auto")
      self._private_section.insert(1,"long long das_id_;")
    
    self._private_section.append('')

    exc_assoc = self._get_exclusive_associations(datatype.name)
    if exc_assoc != [] and self._assoc_touples == []:
      self._header.append("#include <odb/tr1/memory.hxx>")
      
    for exc in exc_assoc:
      self._header.append('#include "ddl_'+exc[1]+'.hpp"')
      self._forward_section.append('class '+exc[1]+';')
#      self._friends.append('friend class '+exc[1]+';')
      self._private_section.append("lazy_weak_ptr<"+exc[1]+"> "+exc[1]+"_"+exc[0]+"_;")
 
    all_assoc = self._get_all_associations(datatype.name)
    for ass_type in all_assoc:
      self._friends.append('friend class '+ass_type+';')

    h = open(_os.path.join(self._hdr_dir, hdr_name), 'w') 
    h.writelines(l + "\n" for l in lines)
    h.writelines(l + "\n" for l in self._header)
    h.writelines(['#include <memory>\n'])
    h.writelines(['using std::tr1::shared_ptr;\n'])
    h.writelines(['using std::tr1::weak_ptr;\n'])
    h.writelines(l + "\n" for l in self._forward_section)
    h.writelines(l + "\n" for l in self._column_type)
    h.writelines(l + "\n" for l in intro)
# public section
    h.writelines([" public:\n"])
    
    #factory method
    h.writelines(["  static","  shared_ptr<"+self._class_name+"> create(const std::string &name);\n"])
    h.writelines(["  static const\n","  shared_ptr<"+self._class_name+">& get_null_ptr();\n"])
    h.writelines("  "+l + "\n" for l in self._type_defs)
    h.writelines("  "+l + "\n" for l in self._public_section)

#    if datatype.data and datatype.data.store_as == 'File' and datatype.data.isTable():
#      h.writelines(['''
#  template<typename T>
#  blitz::Array<T,1>
#  get_column(const std::string &column, long long start = 0LL, long long length = -1LL);
#'''])

# protected section
    h.writelines([" protected:\n"])
    h.writelines("  "+l + "\n" for l in self._friends)
    h.writelines("  "+l + "\n" for l in self._protected_section)
    # default constructor.
    h.writelines(["  "+self._class_name+" ();\n"])
    h.writelines(["  virtual\n"])
    h.writelines(["  void\n"])
    h.writelines(["  update();\n"])
# private section
    h.writelines([" private:\n"])
    h.writelines(["  static void attach(const shared_ptr<"+self._class_name+"> &ptr, das::tpl::DbBundle &bundle);\n"])
    h.writelines(["  "+self._class_name+" (const std::string &name);\n"])
    h.writelines("  "+l + "\n" for l in self._private_section)
    for t in self._assoc_touples:
      if (t[0].multiplicity == 'many' and t[0].relation != 'shared') or (t[0].multiplicity == 'one' and t[0].relation != 'shared'):
        h.writelines(['  #pragma db inverse('+self._class_name+'_'+t[0].name+'_)\n'])
      h.writelines(['  '+t[2]+' '+t[0].name+'_;\n'])
#
#    h.writelines("  shared_ptr<"+l[1]+"> "+l[1]+"_"+l[0]+"_;\n" for l in exc_assoc)
    # default inizializer method
    h.writelines(["  void init();\n"])
    h.writelines(['  #pragma db transient\n'])
    h.writelines(['  weak_ptr<'+self._class_name+'> self_;\n'])
#    h.writelines(['  #pragma db transient\n'])
    h.writelines(['  static shared_ptr<'+self._class_name+'> null_ptr_;\n'])
    h.writelines(["};\n"])
    
    h.writelines(['#include "'+idr_name+'"\n'])
    h.writelines(["#endif"])
    h.close()

    i = open(_os.path.join(self._hdr_dir, idr_name), 'w')
    i.writelines([_def_factory_method(self._class_name)])
    for j in self._keyword_touples:
      i.writelines(l+'\n' for l in _def_getter(j[0],j[1],self._class_name))
      i.writelines(l+'\n' for l in _def_setter(j[0],j[1],self._class_name))

    i.writelines('''
template<>
struct das_traits<'''+self._class_name+'''>
{
    static const std::string name;
};
''')
#    if datatype.data and datatype.data.store_as == 'File' and datatype.data.isTable():
#      i.writelines(['''
#template<typename T>
#blitz::Array<T,1>
#'''+self._class_name+'''::get_column(const std::string &column, long long start, long long length)
#{
#  const ColumnInfo &info = get_column_info(column);
#  shared_ptr<'''+self._class_name+'''_column> &cl = this->*columns_.at(column);
#
#  if(!cl)
#  {
#    throw das::no_data_column();
#  }
#
#  if(start < 0 || start > cl->size())
#  {
#    throw das::bad_param();
#  }
#  long long len = length == -1 ? cl->size()-start : length;
#
#  DasDataIn<T> io(cl);
#  return io.get(info,start,len);
#
#}
#'''])
    #null pointer getter
    i.writelines(['''
inline const shared_ptr<'''+self._class_name+'''>&
'''+self._class_name+'''::get_null_ptr()
{
  if(!'''+self._class_name+'''::null_ptr_)
  {
    '''+self._class_name+'''::null_ptr_.reset(new '''+self._class_name+'''("I am a null object. If you see me, libdas has a bug: the developers apologize"));
  }
  return '''+self._class_name+'''::null_ptr_;
}
'''])
    i.close()

    self._src_header.append('#include "tpl/database.hpp"')
    self._src_header.append('#include "ddl/types/mysql/ddl_'+self._class_name+'-odb.hxx"')
    self._src_header.append('#include "exceptions.hpp"')
    self._src_header.append('#include "internal/db_bundle.ipp"')
    self._src_header.append('#include <boost/variant/apply_visitor.hpp>')

    s = open(_os.path.join(self._src_dir, src_name), 'w') 
    s.writelines(['#include "ddl/types/'+hdr_name+'"\n'])
    s.writelines(l + "\n" for l in self._src_header)
    # type traits static inizialization
    s.writelines(['const std::string das_traits<'+self._class_name+'>::name = "'+self._class_name+'";\n'])
    # public constructor with name argument
    s.writelines([self._class_name+"::"+self._class_name+" (const std::string &name)\n"])
    s.writelines("  "+l + "\n" for l in self._init_list)
    s.writelines(["{\n"])
    s.writelines(["  init();\n"])
    s.writelines(['  name_ = name;\n'])
    s.writelines(["}\n"])
    # default constructor.
    s.writelines([self._class_name+"::"+self._class_name+" ()\n"])
    s.writelines("  "+l + "\n" for l in self._init_list)
    s.writelines(["{\n"])
    s.writelines(["  init();\n"])
    s.writelines(["}\n"])
    # private section
    s.writelines(["void\n",self._class_name+"::init()\n"])
    s.writelines(["{\n"])
    s.writelines(['  type_name_ = "'+self._class_name+'";\n'])
    s.writelines("  "+l + "\n" for l in self._default_init)
    s.writelines(["}\n"])

    if self._has_associations:
      for i in self._assoc_touples:
        s.writelines(l+'\n' for l in _def_getter_assoc(i[0],i[1],i[2],self._class_name))
        s.writelines(l+'\n' for l in _def_setter_assoc(i[0],i[1],i[2],self._class_name))
      
      # persist pre
      s.writelines(['void\n',self._class_name+'::persist_associated_pre(das::tpl::DbBundle &db)\n{\n'])
      if self._inherit != 'DasObject':
        s.writelines(['  '+self._inherit+'::persist_associated_pre(db);\n'])
      for i in self._assoc_touples:
        if i[0].relation == 'shared':
          s.writelines([_def_persist_assoc(i[0],i[2])])
      s.writelines(['}\n'])
      
      #persist post
      s.writelines(['void\n',self._class_name+'::persist_associated_post(das::tpl::DbBundle &db)\n{\n'])
      if self._inherit != 'DasObject':
        s.writelines(['  '+self._inherit+'::persist_associated_post(db);\n'])
      for i in self._assoc_touples:
        if i[0].relation != 'shared':
          s.writelines([_def_persist_assoc(i[0],i[2])])
      s.writelines(['}\n'])

      s.writelines(['void\n',self._class_name+'::update()\n{\n'])
      s.writelines(['''  if(is_dirty_ && !is_new())
  {
    DAS_LOG_DBG("DAS debug INFO: UPD name:'" << name() << "' version:" << version() <<"...");
    
    if(bundle_.transaction_expired())
      set_dirty_columns();

    bundle_.lock_db(true)->update(*this);
    is_dirty_ = false;
    DAS_LOG_DBG("done.");
  }
'''])      
      if self._inherit != 'DasObject':
        s.writelines(['  '+self._inherit+'::update();\n'])
      if self._assoc_touples:
        s.writelines(['  das::tpl::DbBundle bundle = bundle_.lock();\n'])
        for i in self._assoc_touples:
          s.writelines([_def_update_assoc(i[0],i[2])])
      s.writelines(['}\n'])
    else:
      s.writelines(['''
void
'''+self._class_name+'''::update()
{
  if(is_dirty_){
    
    if(bundle_.transaction_expired())
      set_dirty_columns();

    bundle_.lock_db(true)->update(*this);
  }
}
'''])
    s.writelines(['''
void
'''+self._class_name+'''::attach(const shared_ptr<'''+self._class_name+'''> &ptr,das::tpl::DbBundle &bundle)
{
  bundle.attach(ptr);
'''])
    for i in self._assoc_touples:
      s.writelines([_def_attach_assoc(i[0],i[2])])
    s.writelines(['}\n'])

    # write other body methods
    s.writelines(self._src_body)
    
    s.writelines(['shared_ptr<'+self._class_name+'> '+self._class_name+'::null_ptr_;\n'])
    s.close()

    self.clean_env()
    
  def visit_associated(self, associated):
    if not self._has_associations:
      self._header.append("#include <odb/tr1/memory.hxx>")
      self._header.append("#include <odb/tr1/lazy-ptr.hxx>")
      self._header.append("#include <odb/transaction.hxx>")
      self._header.append("#include <odb/session.hxx>")
      self._header.append("using odb::tr1::lazy_weak_ptr;")
      self._header.append("using odb::tr1::lazy_shared_ptr;")
      self._header.append("using std::tr1::shared_ptr;")
      self._friends.append("friend class das::tpl::Database;")
      self._protected_section.extend(['virtual void persist_associated_pre (das::tpl::DbBundle &db);'])
      self._protected_section.extend(['virtual void persist_associated_post(das::tpl::DbBundle &db);'])
      self._has_associations = True 
	
    self._forward_section.append("class " + associated.atype + ";")
    self._header.append('#include "ddl_'+ associated.atype +'.hpp"')
    if associated.multiplicity == 'many':
      pub_type = associated.name+'_vector'
      priv_type = associated.name+'_lazy_shared_vec'
      self._private_section.append('typedef std::vector<lazy_shared_ptr<'+associated.atype+'> > '+priv_type+';')
      self._type_defs.append('typedef std::vector<shared_ptr<'+associated.atype+'> > '+pub_type+';' )
      if associated.relation != 'shared':
        self._src_header.append('#include <algorithm>')
    else:
      pub_type = 'shared_ptr<'+associated.atype+'>'
      priv_type = 'lazy_shared_ptr<'+associated.atype+'>'
    
    self._public_section.extend(_dec_getter_assoc(associated.name, pub_type))
    self._public_section.extend(_dec_setter_assoc(associated.name, pub_type))
    self._src_header.append('#include "tpl/database.hpp"')
#TODO: multi-database support    self._src_header.append('#include "dbms/common/ddl_'+associated.atype+'-odb.hxx"')
    self._src_header.append('#include "ddl/types/mysql/ddl_'+associated.atype+'-odb.hxx"')
    self._src_header.append('#include "exceptions.hpp"')
#    self._assoc_touples.append((associated.name,pub_type,priv_type,associated.atype))
    self._assoc_touples.append((associated,pub_type,priv_type))
 
  def visit_keyword(self,keyword):
    k_type = self.KTYPE_MAP[keyword.ktype]

    if keyword.ktype == 'string':
      self._private_section.append('#pragma db type("VARCHAR(256)")')
    if keyword.index is not None and keyword.index == "yes":
      self._private_section.append("#pragma db index")
    if keyword.default is not None:
      self._default_init.append(keyword.name+"_ = "+keyword.default+";")
    if k_type == 'CBLOB':
      self._private_section.append('#pragma db mysql:type("MEDIUMTEXT") oracle:type("CLOB") pgsql:type("TEXT") sqlite:type("TEXT") mssql:type("varbinary")')
      self._header.append('typedef std::string CBLOB;')

    self._private_section.append(k_type + " " + keyword.name + "_;")
    self._public_section.extend(_dec_getter(keyword.name, self.KTYPE_MAP[keyword.ktype]))
    self._public_section.extend(_dec_setter(keyword.name, self.KTYPE_MAP[keyword.ktype]))

    self._keyword_touples.append((keyword.name, self.KTYPE_MAP[keyword.ktype]))

   
  def visit_binary_table(self,_):
    

    self._protected_section.append("virtual void set_dirty_columns();")
#        self._header.append('#include "../../internal/das_io.hpp"')
    self._header.append('#include <odb/vector.hxx>')
    
    if self._store_as == 'File':
      self._protected_section.append('virtual ColumnFromFile* column_from_file(const std::string &col_name);')
      self._protected_section.append('virtual void column_from_file(const std::string &col_name, const ColumnFromFile &cf);')
      self._src_body.append('''
ColumnFromFile*
'''+self._class_name+'''::column_from_file(const std::string &col_name){
  get_column_info(col_name); // will throw if not present
  
  for(odb::vector<'''+self._class_name+'''_config>::iterator i = columns_.begin(); i != columns_.end(); ++i){
    if(i->column_name() == col_name)
      return i->column_from_file();
  }
  return NULL;
}

void
'''+self._class_name+'''::column_from_file(const std::string &col_name, const ColumnFromFile &cf){
  get_column_info(col_name); // will throw if not present
  for(odb::vector<'''+self._class_name+'''_config>::iterator i = columns_.begin(); i != columns_.end(); ++i){
    if(i->column_name() == col_name){
      i.modify().column_from_file(cf);
      return;
    }
  }
  '''+self._class_name+'''_config conf(col_name);
  conf.column_from_file(cf);
  columns_.push_back(conf);
}

void
'''+self._class_name+'''::set_dirty_columns()
{
  for(odb::vector<'''+self._class_name+'''_config>::iterator i = columns_.begin(); i != columns_.end(); ++i)
    i.modify();
}
''')
      self._private_section.append("odb::vector<"+self._class_name+"_config> columns_;")
      self._column_type = ['''
#pragma db object session(false)
class ColumnFromFile_'''+self._class_name+''' : public ColumnFromFile
{
public:
  ColumnFromFile_'''+self._class_name+'''(const long long &size,
	     const std::string &type,
	     const std::string &fname)
  : ColumnFromFile(size,type,fname) {}

  ColumnFromFile_'''+self._class_name+'''(const std::string &type)
  : ColumnFromFile(type) {}

  ColumnFromFile_'''+self._class_name+'''(const ColumnFromFile &cff)
  : ColumnFromFile(cff){}
private:
  ColumnFromFile_'''+self._class_name+'''(){}
  friend class odb::access;
};

#pragma db value
class '''+self._class_name+'''_config : public ColumnConfig
{
public:
  '''+self._class_name+'''_config(const std::string &col_name)
    : col_name_(col_name) {}

  virtual
  ColumnFromFile *
  column_from_file() const{
    // null if there's no data in this column
    return cff_.get();
  }

  virtual
  void
  column_from_file(const ColumnFromFile &cff){
    cff_.reset(new ColumnFromFile_'''+self._class_name+'''(cff));
  }

  virtual
  const std::string&
  column_name() const {
    return col_name_;
  }

private:
  friend class odb::access;
  std::string col_name_;
  // might become lazy
  shared_ptr<ColumnFromFile_'''+self._class_name+'''> cff_;
  '''+self._class_name+'''_config(){}


};

''']
    else:   
      self._private_section.append("odb::vector<ColumnFromBlob> columns_;")
    
# if we prepare the vector, this will be stored in th db even without data reference
#  def visit_column(self,column):
#    if self._store_as == 'File':
#      self._default_init.append('columns_.push_back('+self._class_name+'_config("'+column.name+'"));')
#    else:
#      self._default_init.append('columns_.push_back(ColumnBlob("'+column.name+'"));')


  def visit_data(self,data):
    if data.store_as == 'blob':
      self._store_as = 'Blob'
    else:
      self._store_as = 'File'

  def visit_image(self,_):
    self._protected_section.append("Image"+self._store_as+" image_;")
    self._header.append('#include "../image.hpp"')
    if self._init_list: # if is not empty
      self._init_list.append(', image_("")')
    else:
      self._init_list.append('  :image_("")')


  def has_inherit_column(self,key):
    obj = self._instance.type_map[key]
    if obj.data is not None:
      return obj.data.isTable()
    else:
      if obj.ancestor != obj.name:
        return self.has_inherit_column(obj.ancestor)
      else:
        return False

  def _get_exclusive_associations(self,type_name):
    exc_assoc = []
    for data_type in self._instance.type_map.values():
      for assoc in data_type.associated.values():
        if assoc.atype == type_name:
          if assoc.relation == 'exclusive' or assoc.relation == 'extend':
            exc_assoc.append((assoc.name,data_type.name))

    return exc_assoc

  def _get_all_associations(self,type_name):
    assoc_l = []
    for data_type in self._instance.type_map.values():
      for assoc in data_type.associated.values():
        if assoc.atype == type_name:
          assoc_l.append(data_type.name)
    return assoc_l

def _def_factory_method(class_name):
  return '''
inline shared_ptr<'''+class_name+'''>
'''+class_name+'''::create(const std::string &name)
{
  shared_ptr<'''+class_name+'''> ptr(new '''+class_name+'''(name));
  ptr->self_ = ptr;
  return ptr;
}
'''
  

def _def_getter(attribute_name, attribute_type, class_name):
  method_definition = ["inline const " + attribute_type + "&"]
  method_definition.extend([class_name+"::"+attribute_name + " () const","{"])
  method_definition.extend(["  return " + attribute_name + "_;","}"])
  return method_definition
  
def _def_setter(attribute_name, attribute_type, class_name):
  method_definition = ["inline void"]
  method_definition.extend([class_name+"::"+attribute_name + " (const "+attribute_type+" &"+attribute_name+")", "{"])
  method_definition.extend(["  "+attribute_name+"_ = "+attribute_name+";","  is_dirty_ = true;","}"])
  return method_definition


def _def_getter_assoc(association, pub_type, priv_type, class_name):
  if association.multiplicity == 'many':
    if association.relation == 'shared':
      return _a_ms.getter(association, pub_type, priv_type, class_name)
    else:
      return _a_me.getter(association, pub_type, priv_type, class_name)
  else:
    if association.relation == 'shared':
      return _a_os.getter(association, pub_type, priv_type, class_name)
    else:
      return _a_oe.getter(association, pub_type, priv_type, class_name)  



def _def_setter_assoc(association, pub_type, priv_type, class_name):
  if association.multiplicity == 'many':
    if association.relation == 'shared':
      return _a_ms.setter(association, pub_type, priv_type, class_name)
    else:
      return _a_me.setter(association, pub_type, priv_type, class_name)
  else:
    if association.relation == 'shared':
      return _a_os.setter(association, pub_type, priv_type, class_name)
    else:
      return _a_oe.setter(association, pub_type, priv_type, class_name) 
      

def _def_persist_assoc(association, priv_type):
  if association.multiplicity == 'many':
    if association.relation == 'shared':
      return _a_ms.persist(association, priv_type)
    else:
      return _a_me.persist(association, priv_type)
  else:
    if association.relation == 'shared':
      return _a_os.persist(association, priv_type)
    else:
      return _a_oe.persist(association, priv_type)


def _def_update_assoc(association, priv_type):
  if association.multiplicity == 'many':
    if association.relation == 'shared':
      return _a_ms.update(association, priv_type)
    else:
      return _a_me.update(association, priv_type)
  else:
    if association.relation == 'shared':
      return _a_os.update(association, priv_type)
    else:
      return _a_oe.update(association, priv_type)


def _def_attach_assoc(association, priv_type):
  if association.multiplicity == 'many':
    if association.relation == 'shared':
      return _a_ms.attach(association, priv_type)
    else:
      return _a_me.attach(association, priv_type)
  else:
    if association.relation == 'shared':
      return _a_os.attach(association, priv_type)
    else:
      return _a_oe.attach(association, priv_type)

def _dec_getter(attribute_name, attribute_type):
  method_declaration = ["const " + attribute_type + "&"]
  method_declaration.extend([attribute_name + " () const;\n"])
  return method_declaration
  
def _dec_setter(attribute_name, attribute_type):
  method_declaration = ["void"]
  method_declaration.extend([attribute_name + " (const "+attribute_type+" &"+attribute_name+");\n"])
  return method_declaration

def _dec_getter_assoc(attribute_name, attribute_type):
  src = [attribute_type]
  src.extend([attribute_name + " ();\n"])
  return src
  
def _dec_setter_assoc(attribute_name, attribute_type):
  method_declaration = ["void"]
  method_declaration.append(attribute_name + " ( "+attribute_type+" &"+attribute_name+");\n")
  return method_declaration

def _dec_persist_assoc():
  src = ['virtual void']
  src.append('persist_associated(das::tpl::Database *db);')


class DdlInheritanceValidator:
  def __init__(self, i):
    self._map = {}
    self._instance = i
    self._current_type = None
    self._keyword_violation = False
    self._column_violation = False
    self._data_type_violation = False

  def check_redefined_keywords(self):
    self._keyword_violation = False     
    for (name,dtype) in self._instance.type_map.items():
      self._current_type = name
      if dtype.metadata is not None and dtype.ancestor != dtype.name:
        for key_name in dtype.metadata.keywords.keys():
          self._check_keyword(key_name,dtype.ancestor)
    self._current_type = None
    return self._keyword_violation   
    
  def _check_keyword(self,key_name, type_name):
    obj = self._instance.type_map[type_name]
    if obj.metadata is None:
      if obj.ancestor != obj.name:
        return self._check_keyword(key_name,obj.ancestor)
    else:
      if obj.metadata.keywords.get(key_name,None) is not None:
        print "Error: type " + self._current_type + " redefines keyword " + key_name
        print "  previous declaration in ascendant type "+ type_name
        self._keyword_violation = True
      else:
        if obj.ancestor != obj.name:
          return self._check_keyword(key_name,obj.ancestor)         

  def check_redefined_columns(self):
    self._column_violation = False     
    for (name,dtype) in self._instance.type_map.items():
      self._current_type = name
      if dtype.ancestor != dtype.name and dtype.data is not None and dtype.data.isTable():
        for key_name in dtype.data.data_obj.columns.keys():
          self._check_column(key_name,dtype.ancestor)
    self._current_type = None
    return self._column_violation   
    
  def _check_column(self,key_name, type_name):
    obj = self._instance.type_map[type_name]
    if obj.data is None:
      if obj.ancestor != obj.name:
        return self._check_column(key_name,obj.ancestor)
    else:
      if obj.data.isTable() and obj.data.data_obj.columns.get(key_name,None) is not None:
        print "Error: type " + self._current_type + " redefines column " + key_name
        print "  previous declaration in ascendant type "+ type_name
        self._column_violation = True
      else:
        if obj.ancestor != obj.name:
          return self._check_column(key_name,obj.ancestor)         

  def check_ancestor_loop(self):
    errors = False
    for type_ in self._instance.type_map.values():
      errors = errors or self._ancestor_loop(type_,[])
    return errors
  
  def _ancestor_loop(self,type_,type_set):
    if type_.name == 'essentialMetadata':
      return False
    elif type_.name in type_set:
      print "Error: self inheritance or inheritance loops are forbidden."
      print "  type "+type_.name+" found in the inheritance chain started from type "+type_set[0]
      return True
    else:
      type_set.append(type_.name)
      return self._ancestor_loop(self._instance.type_map[type_.ancestor],type_set)

  def check_association_loop(self):
    for type_ in self._instance.type_map.values():
      for assoc_ in type_.associated.values():
        stack=[(type_.name,assoc_.name)]
        if self._association_loop(type_,assoc_,stack):
          return True
    return False

  def _association_loop(self,type_,assoc_,stack):
#    print type_.name,assoc_.atype
    if type_.name == assoc_.atype:
      print "Error: found associations loop:"
      for i in stack:
        print "  "+i[0]+"."+i[1]
      return True
    else:
      errors = False
      for assoc_n_ in self._instance.type_map[assoc_.atype].associated.values():
        stack.append((assoc_.atype,assoc_n_.name))
        errors = errors or self._association_loop(type_,assoc_n_,stack)
        if not errors:
          stack.pop()
      return errors

  def check_image_table_mismatch(self):
    self._data_type_violation = False  
    for (name,dtype) in self._instance.type_map.items():   
      self._current_type = name
      if dtype.data is not None:
        if dtype.ancestor != dtype.name:
          if dtype.data.isTable():
            self._check_table(dtype.ancestor,dtype.data.store_as)
          else:
            self._check_image(dtype.ancestor)
    self._current_type = None
    return self._data_type_violation
  
  def _check_table(self,type_name,store_type):
    dtype = self._instance.type_map[type_name]
    if dtype.data is not None:
      if dtype.data.isImage():
        print "Error data: type "+ self._current_type +" defines table data"
        print "  while ascendant type "+ type_name +" defines image data"
        self._data_type_violation = True
      elif dtype.data.store_as != store_type:
        print "Error. mismatch value for attribute 'storeAs' in type "+self._current_type
        print "  expected "+dtype.data.store_as+", found "+store_type
        self._data_type_violation = True
    if dtype.ancestor != dtype.name:
      self._check_table(dtype.ancestor,store_type)

  
  def _check_image(self,type_name):
    dtype = self._instance.type_map[type_name]
    if dtype.data is not None:
      if dtype.data.isTable():
        print "Error data: type "+ self._current_type +" defines image data"
        print "  while ascendant type "+ type_name +" defines table data" 
        self.data_type_violation = True
      else:
        print "Error in type "+self._current_type+": multiple images are not allowed."
        print "  previously image defined in ascendant type "+ type_name
        self.data_type_violation = True
    else:
      if dtype.ancestor != dtype.name:
        self._check_image(dtype.ancestor)


