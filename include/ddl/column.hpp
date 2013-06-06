#ifndef DAS_COLUMN_HPP
#define DAS_COLUMN_HPP

#pragma db value
class Column
{

 public:
  Column(long long size, std::string type)
    :size_(size), type_(type)
  {}
  
  Column(std::string type)
    :size_(0), type_(type)
  {}
  
  const long long&
  size() const
  {
    return size_;
  }

  void
  size(long long size)
  {
    size_ = size;
  }

  const std::string&
  type() const
  {
    return type_;
  }
  
  void
  type(std::string type)
  {
    type_ = type;
  }

 protected:
  long long size_;
  std::string type_;
  Column()  {}
 private:
  friend class odb::access;

};

#pragma db value
class ColumnFile: public Column
{
public:
  ColumnFile(long long size, std::string type, std::string fname)
    : Column(size, type),  fname_(fname)
  {}
  ColumnFile(std::string type)
    : Column(type)
  {}  
  const std::string&
  fname()
  {
    return fname_;
  }

  void
  fname(std::string fname)
  {
    fname_ = fname;
  }

 private:
  friend class odb::access;

  ColumnFile()  {}
  std::string fname_;
};

#pragma db value
class ColumnBlob: public Column
{
public:
  ColumnBlob(long long size, std::string type)
    : Column(size, type)
  {}
  
  ColumnBlob(std::string type)
    : Column(type)
  {} 
  


 private:
  friend class odb::access;

  ColumnBlob()  {}
 
  #pragma db mysql:type("MEDIUMBLOB") oracle:type("BLOB") pgsql:type("BYTEA") sqlite:type("BLOB") mssql:type("varbinary")
  std::vector<char> buffer_;

};
#endif
