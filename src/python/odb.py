import ddl as _d
import os as _os

class DdlVisitor:
 
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
  
  
  KTYPE_MAP = {'byte':'char', 'char':'char', 'int16':'short', 'int32':'int', 
               'int64':'long long', 'float32':'float', 'float64':'double', 
               'boolean':'bool', 'string':'std::string', 'text' : 'CBLOB'}

  def __init__(self, source_dir, instance):
    if not _os.path.exists(source_dir):
      _os.makedirs(source_dir)
      
    self._src_dir = source_dir  
    self._sources = []
    self._instance = instance
 
    self._header = []
    self._inherit = ""
    self._class_name = ""
    self._forward_section = []
    self._public_section = []
    self._protected_section = []
    self._private_section = []
    self._init_list = []
    self._default_init = []
    self._const_init = []
    self._has_associations = False
    self._store_as = None

  def clean_env(self):
    self._header = []
    self._inherit = ""
    self._class_name = ""
    self._forward_section = []
    self._public_section = []
    self._protected_section = []
    self._private_section = []
    self._init_list = []
    self._default_init = []
    self._const_init = []
    self._has_associations = False
    self._store_as = None

  def generate(self):
    self.clean_env()
    self._instance.accept(self)

  def visit_type_list(self, _ ):  
    lines = []
#    lines.append('#include "ddl_BLOB.hpp"')
    for k in self._sources:
      lines.append('#include "' + k +'"')
      
    if len(lines):
      lines.insert(0,
'''
#ifndef DAS_DDL_TYPES_H
#define DAS_DDL_TYPES_H

''')
      lines.append('#endif')
      f = open(_os.path.join(self._src_dir, "ddl_types.hpp"), 'w')
      f.writelines(l + "\n" for l in lines)
      f.close()
      
  def visit_datatype(self, datatype):
    if datatype.name == "essentialMetadata":
      self.clean_env()
      return
    lines = []
    intro = []
    self._class_name = datatype.name
    filename = "ddl_"+ self._class_name +".hpp"
    self._sources.append(filename)
    macro = "DAS_" + datatype.name.upper() + "_H"
    lines.append("#ifndef " + macro + "\n#define " + macro)
    lines.append("#include <odb/core.hxx>")
    lines.append("#include <vector>")
    lines.append("#include <map>")
    lines.append('#include "ddl/info.hpp"')
    if datatype.ancestor != "essentialMetadata":
      self._header.append('#include "ddl_' + datatype.ancestor + '.hpp"')
      self._inhetit = datatype.ancestor
    else:
      self._header.append('#include "DasObject.hpp"')
      self._inherit = "DasObject"
      
    if datatype.data is not None:
      self._header.append('#include "ddl/column.hpp"')
    
    for t in datatype.associated.values():
      self._forward_section.append("class " + t.name + ";")

    if self._class_name  == "essentialMetadata":
       intro.append("#pragma db object abstract\nclass " + datatype.name)   
    else:
      intro.append("#pragma db object\nclass " + datatype.name)
    if self._inherit != "":
      intro[-1] += ": public "+self._inherit
    else:
      intro[-1] += ": public DasObject"
      lines.append('#include "DasObject.hpp"')

    intro.append("{")
    
    if self._inherit == "":    
      self._private_section.insert(0,"#pragma db id auto")
      self._private_section.insert(1,"long long das_id_;")
    
    self._private_section.append('')

     
    f = open(_os.path.join(self._src_dir, filename), 'w') 
    f.writelines(l + "\n" for l in lines)
    f.writelines(l + "\n" for l in self._header)
    f.writelines(l + "\n" for l in self._forward_section)
    f.writelines(l + "\n" for l in intro)
    f.writelines([" public:\n"])
    f.writelines("  "+l + "\n" for l in self._public_section)
    f.writelines(["  "+self._class_name+" ()\n"])
    f.writelines("  "+l + "\n" for l in self._init_list)
    f.writelines(["  {\n"])
    f.writelines(['    type_name_ = "'+self._class_name+'";\n'])
    f.writelines("    "+l + "\n" for l in self._default_init)
    f.writelines(["  }\n"])
    f.writelines([" protected:\n"])
    f.writelines(["  friend class odb::access;\n"])
    f.writelines("  "+l + "\n" for l in self._protected_section)
    f.writelines([" private:\n"])
    f.writelines("  "+l + "\n" for l in self._private_section)
    f.writelines(["};\n"])
    f.writelines(l + "\n" for l in self._const_init)
    f.writelines(["#endif"])
    f.close()
    self.clean_env()
    
  
  def visit_associated(self, associated):
    if not self._has_associations:
      self._header.append("#include <odb/tr1/memory.hxx>")
      self._header.append("#include <odb/tr1/lazy-ptr.hxx>")
      self._header.append("using odb::tr1::lazy_weak_ptr;")  
      self._has_associations = True 
	
    self._forward_section.append("class " + associated.atype + ";")
    # see Remove multiplicity attribute discussion
    weak_refs = "std::vector<lazy_weak_ptr<" + associated.atype + "> >" 
    #shared_refs = "std::vector<shared_ptr<" + associated.atype + "> >"
    self._private_section.append(weak_refs + " " + associated.name + "_;")
    self._public_section.extend(_getter_template(associated.name, weak_refs))
 
 
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
    self._public_section.extend(_getter_template(keyword.name, self.KTYPE_MAP[keyword.ktype]))

   
  def visit_binary_table(self,_):
    if self._inherit != "":
      if not self.has_inherit_column(self._inherit):
        self._protected_section.append("std::map<std::string, Column"+self._store_as+"> columns_;")
    else:
      self._protected_section.append("std::map<std::string, Column"+self._store_as+"> columns_;")
    
  def visit_column(self,column):
#    self._default_init.append('columns_["'+column.name+'"] = Column'+self._store_as+'("'+column.ctype+'");')
    self._default_init.append('columns_.insert(std::pair<std::string,Column'+self._store_as+'>("'+column.name+'",Column'+self._store_as+'("'+column.ctype+'")));')

  def visit_data(self,data):
    if data.store_as == 'blob':
      self._store_as = 'Blob'
    else:
      self._store_as = 'File'

  def visit_image(self,_):
    self._protected_section.append("Image"+self._store_as+" image_;")
    self._header.append('#include "ddl/image.hpp"')
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
    
#  def createXMLsingleton(self):
#    shutil.copyfile("../cpp/ddl_info.hpp", _os.path.join(self._src_dir, "ddl_info.hpp"))
#    shutil.copyfile("../cpp/ddl_info.cpp", _os.path.join(self._src_dir, "ddl_info.cpp"))

#  def createXMLcolumn(self):
#    shutil.copyfile("../cpp/ddl_column.hpp", _os.path.join(self._src_dir, "ddl_column.hpp"))   

def _getter_template(attribute_name, attribute_type):
  method_definition = ["const " + attribute_type + "&"]
  method_definition.extend([attribute_name + " () const","{"])
  method_definition.extend(["  return " + attribute_name + "_;","}"])
  return method_definition
  
def _setter_template(attribute_name, attribute_type):
  method_definition = ["void"]
  method_definition.extend([attribute_name + " ("+attribute_type+" "+attribute_name+")","{"])
  method_definition.extend(["  "+attribute_name+"_ = "+attribute_name+";","}"])
  return method_definition


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

      
           
if __name__ == "__main__":  
  import sys
  schema_file_name = sys.argv[1]
  das_instance_file = sys.argv[2]
  output_dir = sys.argv[3]

  parser = _d.DdlParser(schema_file_name)
  instance = parser.parse_ddl(das_instance_file)
  
  validator = DdlInheritanceValidator(instance)
  if validator.check_redefined_keywords():
    exit(1)
  if validator.check_image_table_mismatch():
    exit(1)
  if validator.check_redefined_columns():
    exit(1)
  g = DdlOdbGenerator(output_dir, instance)
  g.generate()
