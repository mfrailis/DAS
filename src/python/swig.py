import os as _os

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



class DdlSwigGenerator(DdlVisitor):
  
  
  KTYPE_MAP = {'byte':'signed char', 'char':'char', 'int16':'short', 'int32':'int', 
               'int64':'long long', 'float32':'float', 'float64':'double', 
               'boolean':'bool', 'string':'std::string', 'text' : 'std::string'}

  def __init__(self, src_dir, instance):
      
    self._src_dir = src_dir
    self._source = []
    self._instance = instance
 
    self.clean_env()

  def clean_env(self):
    self._header = []
    self._src_header = ['#include "internal/log.hpp"']
    self._inherit = ""
    self._class_name = ""
    self._forward_section = []
    self._public_section = []
    self._protected_section = []
    self._private_section = []
    self._init_list = []
    self._default_init = []
    self._friends = ["friend class odb::access;","friend class das::Transaction;","friend class das::TransactionBundle;","friend class das::DbBundle;","friend class das::tpl::Database;"]
    self._has_associations = False
    self._store_as = None

  def name(self,name):
    self._class_name=name

  def generate(self):
    self.clean_env()
    self._instance.accept(self)

  def visit_type_list(self, _ ):
    intro = ['''
%{
#define DAS_SWIG_BINDING
#include "ddl/types.hpp"
#include "tpl/database.hpp"
%}

%import "das_optional.i"
%import "das_object.i"
%import "database.i"
%import "result.i"

%include <std_string.i>

#define SWIG_SHARED_PTR_SUBNAMESPACE tr1
%include <std_shared_ptr.i>
''']

    f = open(_os.path.join(self._src_dir, "ddl_types.i"), 'w')
    f.writelines(l + "\n" for l in intro)
    f.writelines(l + "\n" for l in self._source)
    f.close()
      
  def visit_datatype(self, datatype):
    if datatype.name == "essentialMetadata":
      self.clean_env()
      return

    if datatype.ancestor != "essentialMetadata":
      self._inherit = datatype.ancestor
    else:
      self._inherit = "DasObject"
    
    self._source.append('%shared_ptr('+self._class_name+')')
    self._source.append('class '+self._class_name+': public '+self._inherit)
    self._source.append('{')
    self._source.append(' public:')

    for l in self._public_section:
      self._source.append('  '+l)

    self._source.append('  static')
    self._source.append('  std::tr1::shared_ptr<'+self._class_name+'>')
    self._source.append('  create(const std::string &name, const std::string &db_alias);')
    self._source.append(' private:')
    self._source.append('  '+self._class_name+'(const std::string &name, const std::string &db_alias);')
    self._source.append('};')

    self._source.append(''' 
%template(Result_'''+self._class_name+''') das::tpl::Result<'''+self._class_name+'''>;
%template(Result_'''+self._class_name+'''_iterator) das::result_iterator_wrapper<'''+self._class_name+'''>;

%extend '''+self._class_name+''' {

  static 
    std::tr1::shared_ptr<'''+self._class_name+'''>
    load(std::tr1::shared_ptr<das::tpl::Database> db, const long long &id) {
    return db->load<'''+self._class_name+'''>(id);
  }

  static 
    std::tr1::shared_ptr<'''+self._class_name+'''>
    load(std::tr1::shared_ptr<das::tpl::Database> db, std::string& name, int version = -1) {
    return db->load<'''+self._class_name+'''>(name, version);
  }

  long long
    persist(std::tr1::shared_ptr<das::tpl::Database> db, std::string path = "") {
    return db->persist($self->get_shared_ptr(), path);
  }

  static 
    das::tpl::Result<'''+self._class_name+'''>
    query(std::tr1::shared_ptr<das::tpl::Database> db, const std::string& expression, 
          const std::string& ordering = "", bool last_version_only = false) {
    return db->query<'''+self._class_name+'''>(expression, ordering, last_version_only);
  }

  void
    erase(std::tr1::shared_ptr<das::tpl::Database> db) {
    db->erase($self->get_shared_ptr());
  }    

  void
    attach(std::tr1::shared_ptr<das::tpl::Database> db) {
    db->attach($self->get_shared_ptr());
  }

};

''')

    self.clean_env()
    
 
  def visit_keyword(self,keyword):
    self._public_section.extend(_dec_getter(keyword.name, self.KTYPE_MAP[keyword.ktype]))
    self._public_section.extend(_dec_setter(keyword.name, self.KTYPE_MAP[keyword.ktype]))


def _dec_getter(attribute_name, attribute_type):
  method_declaration = ["das::optional< " + attribute_type + " >"]
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

