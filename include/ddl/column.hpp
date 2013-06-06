#ifndef DAS_COLUMN_HPP
#define DAS_COLUMN_HPP
#include <odb/core.hxx>
#include <vector>
#include <string>

#pragma db value
class Column
{

 public:
  Column(const long long& size,
	 const std::string &type)
    :size_(size), type_(type)
  {}

  Column(const std::string &type)
    :size_(0), type_(type)
  {}

  const long long&
  size() const
  {
    return size_;
  }

  void
  size(const long long &size)
  {
    size_ = size;
  }

  const std::string&
  type() const
  {
    return type_;
  }

  void
  type(const std::string &type)
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
  ColumnFile(const long long &size,
	     const std::string &type,
	     const std::string &fname)
    : Column(size, type),  fname_(fname)
  {}
  ColumnFile(const std::string &type)
    : Column(type)
  {}
  const std::string&
  fname()
  {
    return fname_;
  }

  void
  fname(const std::string &fname)
  {
    fname_ = fname;
  }

 private:
  friend class odb::access;
  void save();
  ColumnFile()  {}
  std::string fname_;
};

#pragma db value
class ColumnBlob: public Column
{
public:
  ColumnBlob(const long long &size,
	     const std::string &type)
    : Column(size, type)
  {}

  ColumnBlob(const std::string &type)
    : Column(type)
  {}



 private:
  friend class odb::access;

  ColumnBlob()  {}

  #pragma db mysql:type("MEDIUMBLOB") oracle:type("BLOB") pgsql:type("BYTEA") sqlite:type("BLOB") mssql:type("varbinary")
  std::vector<char> buffer_;

};
#endif
