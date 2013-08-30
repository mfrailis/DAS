#include "ddl/info.hpp"

boost::unordered_map< std::string, TypeInfo > DdlInfo::all_types_;

 DdlInfo* DdlInfo::instance_=0;
