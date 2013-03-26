#ifndef DAS_DASOBJECT_HPP
#define DAS_DASOBJECT_HPP

#include <odb/core.hxx>
#include <string>
#include "ddl/info.hpp"

#pragma db object abstract
class DasObject
{
public:
  const KeywordInfo&
  get_keyword_info (std::string keyword_name) throw(std::out_of_range)
  {
    return DdlInfo::get_instance()->
      get_keyword_info(type_name_, keyword_name);
  }

  const ColumnInfo&
  get_column_info (std::string column_name) throw(std::out_of_range)
  {
    return DdlInfo::get_instance()->
      get_column_info(type_name_, column_name);
  }  

  const std::string&
  dbUserId () const
  {
    return dbUserId_;
  }

  const long long&
  creationDate () const
  {
    return creationDate_;
  }

  void
  creationDate (long long creationDate)
  {
    creationDate_ = creationDate;
  }

  const short&
  version () const
  {
    return version_;
  }

  const std::string&
  name () const
  {
    return name_;
  }

  void
  name (std::string name)
  {
    name_ = name;
  }

  DasObject()
  {
    type_name_ = "DasObject";
    version_ = 0;
    das_id_ = 0;
  }

protected:
  std::string type_name_; 
private:
  friend class odb::access;

  #pragma db id auto
  long long das_id_;

  std::string dbUserId_;
  long long creationDate_;
  short version_;

  #pragma db index
  std::string name_;
};


#endif
