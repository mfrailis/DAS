
def column_data_types(class_name):
    return ['''
#pragma db object session(false)
class ColumnFile_'''+class_name+''' : public ColumnFile
{
public:
  ColumnFile_'''+class_name+'''(const long long &size,
	     const std::string &type,
             const std::string &array_size,
	     const std::string &fname)
  : ColumnFile(size,type,array_size,fname) {}

  ColumnFile_'''+class_name+'''(const std::string &type, const std::string &array_size)
  : ColumnFile(type,array_size) {}

  ColumnFile_'''+class_name+'''(const ColumnFile &cff)
  : ColumnFile(cff){}

  virtual
  void
  persist(odb::database &db);
private:
  ColumnFile_'''+class_name+'''(){}
  friend class odb::access;
};

#pragma db value
class '''+class_name+'''_config : public ColumnConfig
{
public:
  '''+class_name+'''_config(const std::string &col_name)
    : col_name_(col_name) {}

  virtual
  ColumnFile *
  column_from_file() const{
    // null if there's no data in this column
    return cff_.get();
  }

  virtual
  void
  column_from_file(const ColumnFile &cff){
    cff_.reset(new ColumnFile_'''+class_name+'''(cff));
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
  shared_ptr<ColumnFile_'''+class_name+'''> cff_;
  '''+class_name+'''_config(){}


};

''']

def column_body_src(class_name,columns):
    res = []
    res.append('''
void
'''+class_name+'''::populate_column_map(std::map<std::string,Column*> &map){''')
    for col in columns:
        res.append('  map.insert(std::pair<std::string,Column*>("'+col+'",NULL));')
    res.append('''

  for(boost::unordered_map<std::string,'''+class_name+'''_config>::iterator i = columns_.begin(); i != columns_.end(); ++i)
    map[i->first] = i->second.column_from_file();
}

void
'''+class_name+'''::save_data(const std::string &path, das::TransactionBundle &tb){
  /* we need to remove empty columns in orded to avoid exceptions when odb will eventually
   * try to load them. This also helps to keep the database clean
   */ 
  boost::unordered_map<std::string,'''+class_name+'''_config>::iterator it = columns_.begin();
  while(it != columns_.end()){
    if(it->second.column_from_file()->size() == 0)
      it = columns_.erase(it);
    else
      ++it;
  }
  shared_ptr<das::StorageTransaction> e = das::StorageTransaction::create(bundle_.alias(),tb);
  e->add(this);
  e->save(path);
  tb.add(e);
}

void
'''+class_name+'''::save_data(das::TransactionBundle &tb){
  /* we need to remove empty columns in orded to avoid exceptions when odb will eventually
   * try to load them. This also helps to keep the database clean
   */ 
  boost::unordered_map<std::string,'''+class_name+'''_config>::iterator it = columns_.begin();
  while(it != columns_.end()){
    if(it->second.column_from_file()->size() == 0)
      it = columns_.erase(it);
    else
      ++it;
  }
  shared_ptr<das::StorageTransaction> e = das::StorageTransaction::create(bundle_.alias(),tb);
  e->add(this);
  e->save();
  tb.add(e);
}


Column*
'''+class_name+'''::column_ptr(const std::string &col_name){
  get_column_info(col_name); // will throw if not present

  boost::unordered_map<std::string,'''+class_name+'''_config>::iterator i = columns_.find(col_name);

  if(i == columns_.end())
    return NULL;
  else
    return i->second.column_from_file();

}

void
'''+class_name+'''::column_ptr(const std::string &col_name, const Column &c){
  const ColumnFile& cf = dynamic_cast<const ColumnFile&>(c);
  get_column_info(col_name); // will throw if not present
  '''+class_name+'''_config conf(col_name);
  conf.column_from_file(cf);

  columns_.erase(col_name);
  columns_.insert(std::pair<std::string,'''+class_name+'''_config>(col_name,conf));
  is_dirty_ = true; // it will force odb to update the columns table
}

void
'''+class_name+'''::set_dirty_columns()
{/*
  for(std::vector<'''+class_name+'''_config>::iterator i = columns_.begin(); i != columns_.end(); ++i)
    i.modify();
*/}

void
ColumnFile_'''+class_name+'''::persist(odb::database &db){
  db.persist(*this);
}

''')
    return res

def image_data_types(class_name,dim):
    res = ['''
#pragma db object session(false)
class ImageFile_'''+class_name+''' : public ImageFile
{
public:
  ImageFile_'''+class_name+'''(const std::string &pixel_type,
            const std::string &fname)
  : ImageFile(pixel_type,fname),
    size0_(0),''']
    for i in range(2,(dim-1)):
        res.extend(['    size'+str(i-1)+'_(1),'])            
    res.extend(['    size'+str(dim-2)+'''_(1)  {}

  ImageFile_'''+class_name+'''(const std::string &pixel_type)
  : ImageFile(pixel_type),
    size0_(0),'''])
    for i in range(2,(dim-1)):
        res.extend(['    size'+str(i-1)+'_(1),'])            
    res.extend(['    size'+str(dim-2)+'''_(1) {}

  ImageFile_'''+class_name+'''(const ImageFile &iff)
  : ImageFile(iff)
  {'''])
    for i in range(1,(dim)):
        res.extend(['    size'+str(i-1)+'_ = iff.extent('+str(i-1)+');'])           
    res.extend(['''

  }

  virtual unsigned int rank() const { return '''+str(dim-1)+''';}

  virtual 
  unsigned int
  extent(size_t rank) const
  {
    switch(rank){
      case 0: return size0_ + buff_.tiles();'''])
    for i in range(2,dim):
        res.extend(['      case '+str(i-1)+': return size'+str(i-1)+'_;'])            
    res.extend(['''      default: return 0;
    }
  }
  
  virtual
  unsigned int
  store_tiles() const {
    return size0_;
  }

  virtual
  void
  store_tiles(const unsigned int& tiles){
    size0_ = tiles;
  }

  virtual void
  extent(const size_t &rank, size_t value){
    switch(rank){'''])
    for i in range(2,dim):
        res.extend(['      case '+str(i-1)+': size'+str(i-1)+'_ = value;'])            
    res.extend(['''      default: return;
    }
  }

  virtual
  unsigned int
  num_elements() const
  {
    return buff_.num_elements() + '''])
    for i in range(1,(dim-1)):
        res.extend(['    size'+str(i-1)+'_ *'])            
    res.extend(['    size'+str(dim-2)+'''_;
  }

  virtual
  void
  persist(odb::database &db);

private:'''])
    for i in range(1,dim):
        res.extend(['  int size'+str(i-1)+'_;'])            
    res.extend(['''  ImageFile_'''+class_name+'''(){}
  friend class odb::access;
};

'''])
    return res

def image_body_src(class_name):
    return ['''
Image*
'''+class_name+'''::image_ptr(){
  return image_.get();
}

void
'''+class_name+'''::image_ptr(const Image &i){
  const ImageFile& iff = dynamic_cast<const ImageFile&>(i);
  image_.reset(new ImageFile_'''+class_name+'''(iff));
  is_dirty_ = true; // will force odb to update the reference
}

void
ImageFile_'''+class_name+'''::persist(odb::database &db){
  db.persist(*this);
}

void
'''+class_name+'''::save_data(const std::string &path, das::TransactionBundle &tb){
  shared_ptr<das::StorageTransaction> e = das::StorageTransaction::create(bundle_.alias(),tb);
  e->add(this);
  e->save(path);
  tb.add(e);
}

void
'''+class_name+'''::save_data(das::TransactionBundle &tb){
  shared_ptr<das::StorageTransaction> e = das::StorageTransaction::create(bundle_.alias(),tb);
  e->add(this);
  e->save();
  tb.add(e);
}

''']
