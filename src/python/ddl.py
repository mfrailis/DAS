
from lxml import etree as _et

try:
  from collections import OrderedDict as MapOrd
except:
  from ordereddict import OrderedDict as MapOrd


class TypeUpgrade:
  def __init__(self, name):
    self.name = name

    self.associated=[]
    self.metadata=[]
    self.data_columns=[]

    self.errors = False
    self.is_copy = True

class DdlTypeList:
  def __init__(self):
    self.type_map = MapOrd()
    
  def accept(self, visitor):
    for node in self.type_map.values():
      node.accept(visitor) 
    visitor.visit_type_list(self)
    

class DataType:
  def __init__(self, name):
    self.name = name 
    self.ancestor = None
    self.associated = MapOrd()
    self.metadata = None
    self.data = None
    
  def accept(self, visitor):
    visitor.name(self.name)
    for node in self.associated.values():
      node.accept(visitor)
    if self.metadata is not None:
      self.metadata.accept(visitor)
    if self.data is not None:
      self.data.accept(visitor)
    visitor.visit_datatype(self)

  def upgrade(self,data_type):
    diff = TypeUpgrade(self.name)
    if self.name != data_type.name:
      print "Error: different type names"
      diff.errors = True
      diff.is_copy = False
      return diff

    if self.ancestor != data_type.ancestor:
      print "Error: type attribute in ancestor element mismatch"
      diff.errors = True
          
    for (name,assoc) in self.associated.items():
      assoc2 = data_type.associated.get(name,None)
      if assoc2 is None:
        print "Error: expected associated element with name "+name
        diff.errors = True
      else:
        if assoc.atype != assoc2.atype:
          print "Error: type attribute mismatch in associated element "+name
          diff.errors = True
        if assoc.multiplicity != assoc2.multiplicity:
          print "Error: multiplicity attribute mismatch in associated element "+name
          diff.errors = True
    set1 = set(self.associated.keys())
    set2 = set(data_type.associated.keys())
    new_keys = set2.difference(set1)
    if new_keys:
      diff.is_copy = False
      for key in new_keys:
        diff.associated.append(data_type.associated[key])

    if self.metadata is None:
      if data_type is not None:
        print "Error: unexpected metadata element"
        diff.errors = True
    else:
      md = self.metadata.upgrade(data_type.metadata)
      if md.errors:
        diff.errors = True
      if not md.is_copy:
        diff.is_copy = False
      diff.metadata = md.metadata
      
    if self.data is None:
      if data_type.data is not None:
        print "Error: unexpected data element"
        diff.errors = True
    else:
      dt = self.data.upgrade(data_type.data)
      if dt.errors:
        diff.errors = True
      if not dt.is_copy:
        diff.is_copy = False
      diff.data_columns = dt.data_columns
      
    if diff.errors:
      diff.is_copy = False
    return diff
   

    
class Associated:
  def __init__(self, name, atype, multiplicity, relation):
    self.name = name
    self.atype = atype
    self.multiplicity = multiplicity
    self.relation = relation
    
  def accept(self, visitor):
    visitor.visit_associated(self)
    

class Metadata:
  def __init__(self):
    self.keywords = MapOrd()
    
  def accept(self, visitor):
    for node in self.keywords.values():
      node.accept(visitor)
    visitor.visit_metadata(self)
    
  def upgrade(self, metadata):
    diff = TypeUpgrade("")

    if metadata is None:
      print "Error: expected metadata element"
      diff.errors = True
      return diff

    for (name,keyword) in self.keywords.items():
      keyword2 = metadata.keywords.get(name,None)
      if keyword2 is None:
        print "Error: expected keyword element with name "+name
        diff.errors = True
      else:
        if keyword.ktype != keyword2.ktype:
          print "Error: type attribute mismatch in keyword "+name
          diff.errors = True
        if keyword.unit != keyword2.unit:
          print "Error: unit attribute mismatch in keyword "+name
          diff.errors = True
        if keyword.default != keyword2.default:
          print "Error: default attribure mismatch in keyword "+name
          diff.errors = True
        if keyword.index != keyword2.index:
          print "Error: index attribute mismatch in keyword "+name
          diff.errors = True
        if keyword.description != keyword2.description:
          print "Error: description attribute mismatch in keyword "+name
          diff.errors = True

    set1 = set(self.keywords.keys())
    set2 = set(metadata.keywords.keys())
    new_keys = set2.difference(set1)
    for key in new_keys:
      diff.metadata.append(metadata.keywords[key])

    if new_keys or diff.errors:
      diff.is_copy=False
    return diff


class Keyword:
  def __init__(self, name, ktype, unit, default, index, desc):
    self.name =  name
    self.ktype = ktype
    self.unit = unit
    self.default = default
    self.index = index
    self.description = desc
    
  def accept(self, visitor):
    visitor.visit_keyword(self)
    
    
class Data:
  def __init__(self, store_as):
    self.store_as = store_as
    self.data_obj = None
    
  def isTable(self):
    return isinstance(self.data_obj, BinaryTable)
    
  def isImage(self):
    return isinstance(self.data_obj, Image)
  
  #FIXME: is it necessary?
  def isEmtpy(self):
    return self.data_obj is None
  
  def accept(self, visitor):
    if self.data_obj is not None:
      visitor.visit_data(self)
      self.data_obj.accept(visitor)
       
  def upgrade(self,data):
    diff = TypeUpgrade("")
    if data is None:
      print "Error: expected data element"
      diff.errors = True
      diff.is_copy = False
      return diff
    
    if self.store_as != data.store_as:
      print "Error: storeAs attribute mismatch"
      diff.errors = True

    if self.isTable():
      if not data.isTable():
        print "Error: data element mismatch"
        diff.errors = True
      else:
        obj_diff = self.data_obj.upgrade(data.data_obj)
        diff.data_columns = obj_diff.data_columns
        if obj_diff.errors:
          diff.errors = True
        if not obj_diff.is_copy:
          diff.is_copy = False

    else:
      if not data.isImage():
        print "Error: data element mismatch"
        diff.errors = True
      else:
        obj_diff = self.data_obj.upgrade(data.data_obj)
        if obj_diff.errors:
          diff.errors = True
        if not obj_diff.is_copy:
          diff.is_copy = False

    return diff
       
class BinaryTable:
  def __init__(self):
    self.columns = MapOrd()
    
  def accept(self, visitor):
    for node in self.columns.values():
      node.accept(visitor)
    visitor.visit_binary_table(self)
    
  def upgrade(self,table):
    diff = TypeUpgrade("")
    if table is None:
      print "Error: expected binaryTable element"
      diff.errors = True
      return diff

    for (name,column) in self.columns.items():
      column2 = table.columns.get(name,None)
      if column2 is None:
        print "Error: expected column element with name "+name
        diff.errors = True
      else:
        if column.ctype != column2.ctype:
          print "Error: type attribute mismatch in column "+name
          diff.errors = True
        if column.unit != column2.unit:
          print "Error: unit attribute mismatch in column "+name
          diff.errors = True
        if column.max_string_length != column2.max_string_length:
          print "Error: maxStringLength attribute mismatch in column "+name
          diff.errors = True

    set1 = set(self.columns.keys())
    set2 = set(table.columns.keys())
    new_keys = set2.difference(set1)
    for key in new_keys:
      diff.data_columns.append(table.columns[key])

    if new_keys or diff.errors:
      diff.is_copy=False
    return diff
    
    
class Column:
  def __init__(self, name, ctype, unit, max_string_length, desc):
    self.name = name
    self.ctype = ctype
    self.unit = unit
    self.max_string_length = max_string_length
    self.description = desc

  def accept(self, visitor):
    visitor.visit_column(self)
    
    
class Image:
  def __init__(self, pix_type):
    self.pix_type = pix_type
    
  def accept(self, visitor):
    visitor.visit_image(self)

  def upgrade(self, image):
    diff = TypeUpgrade("")
    if image is None:
      print "Error: expected image element"
      diff.errors = True
    if image.pix_type != image.pix_type:
      print "Error: attribute pixType mismatch"
      diff.errors = True
    if diff.errors:
      diff.is_copy = False
    return diff

class DdlParser:
  
  NSPACE = "{http://oats.inaf.it/das}"
  
  def __init__(self, schema_file_name):
    self.schema = _et.XMLSchema(file=schema_file_name, attribute_defaults=True)
 
  def parse_ddl(self, xml_filename):
    tree = _et.parse(xml_filename)
    tree.xinclude()
    return self._parse(tree)

  def parse_ddl_from_string(self, xml_string):
    #note: xinclude are not expected in the string argument
    tree = _et.fromstring(xml_string)
    return self._parse(tree)

  def _parse(self,tree):
    self.schema.assertValid(tree)

    tlist = DdlTypeList()
    dtypes = tree.findall(DdlParser.NSPACE + 'type')
    for dtype in dtypes:
      ptype = self._parse_datatype(dtype)
      tlist.type_map[ptype.name] = ptype
    
    return tlist
    

  def serialize_tree(self,xml_filename):
    tree =  _et.parse(xml_filename)
    tree.xinclude()
    return _et.tostring(tree)
 
  def _parse_datatype(self, dtype):
    
    dt = DataType(dtype.get('name'))
    
    ancestor = dtype.get('ancestor')
    dt.ancestor = ancestor
    
    associated = dtype.findall(DdlParser.NSPACE + 'associated')
    for a in associated:
      name = a.get('name')
      atype = a.get('type')
      multi = a.get('multiplicity')
      relat = a.get('relation')
      dt.associated[name]=Associated(name, atype, multi,relat)
      
    meta = dtype.find(DdlParser.NSPACE + 'metadata')
    if meta is not None:
      dt.metadata = self._parse_metadata(meta)

    data = dtype.find(DdlParser.NSPACE + 'data')
    if data is not None:
      dt.data = self._parse_data(data)
      
    return dt
      
  def _parse_metadata(self, meta):
    metadata = Metadata() 
    keywords = meta.findall(DdlParser.NSPACE + 'keyword')
    for k in keywords:
      name = k.get('name')
      dtype = k.get('type')
      unit = k.get('unit')
      default = k.get('default')
      index = k.get('index')
      desc = k.get('description') 
      metadata.keywords[name] = Keyword(name, dtype, unit, default, index, desc)
      
    return metadata
  
  def _parse_data(self, data):
    d = Data(data.get('storeAs'))
    
    table = data.find(DdlParser.NSPACE + 'binaryTable')
    if table is not None:
      bt = BinaryTable()
      columns = table.findall(DdlParser.NSPACE + 'column')
      for c in columns:
        name = c.get('name')
        ctype = c.get('type')
        unit = c.get('unit')
        max_string_length = c.get('maxStringLength')
        desc = c.get('description')
        bt.columns[name]=Column(name, ctype, unit, max_string_length, desc)
      
      d.data_obj = bt
        
    else:
      image = data.find(DdlParser.NSPACE + 'image')
      im = Image(image.get('pixType'))
      d.data_obj = im
      
    return d
  
  
if __name__ == "__main__":  
  import sys
  schema_file_name = sys.argv[1]
  das_instance_file = sys.argv[2]
  
  parser = DdlParser(schema_file_name)
  instance = parser.parse_ddl(das_instance_file)
  
  
