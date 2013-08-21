#include "ddl/info.hpp"


boost::unordered_map< std::string, DdlInfo::Keyword_map > DdlInfo::all_keywords_;
boost::unordered_map< std::string, DdlInfo::Column_map  > DdlInfo::all_columns_;
boost::unordered_map< std::string, DdlInfo::Association_map > DdlInfo::all_associations_;
boost::unordered_map< std::string, ImageInfo > DdlInfo::all_images_;

 DdlInfo* DdlInfo::instance_=0;
