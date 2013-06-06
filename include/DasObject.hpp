#ifndef DAS_DASOBJECT_HPP
#define DAS_DASOBJECT_HPP

#include <odb/core.hxx>
#include <string>
#include "ddl/info.hpp"
#include "tpl/Database.hpp"

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

  const std::string&
  database ()
  {
    return database_;
  }

  DasObject()
  {
    type_name_ = "DasObject";
    database_ = "none";
    version_ = 0;
    das_id_ = 0;
  }

protected:
#pragma db transient
  std::string type_name_;
  
#pragma db transient
  std::string database_;
  
private:
  friend class odb::access;
  friend class das::tpl::Database;
  
#pragma db id auto
  long long das_id_;
  
#pragma db type("VARCHAR(256)")
  std::string dbUserId_;
  long long creationDate_;
  short version_;
  
#pragma db type("VARCHAR(256)")
#pragma db index
  std::string name_;
};


#endif
