#include "ddl/info.hpp"


boost::unordered_map< std::string, DdlInfo::Keyword_map > DdlInfo::all_keywords_;
boost::unordered_map< std::string, DdlInfo::Column_map  > DdlInfo::all_columns_;
DdlInfo* DdlInfo::instance_=0;
