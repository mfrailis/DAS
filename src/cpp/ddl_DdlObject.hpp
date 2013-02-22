#ifndef DAS_DDLOBJECT_HPP
#define DAS_DDLOBJECT_HPP

#include <string>
#include "ddl_info.hpp"

class DdlObject
{
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

protected:
  std::string type_name_;  
};


#endif
