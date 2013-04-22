#ifndef DAS_MYSQL_DATABASE_HPP
#define DAS_MYSQL_DATABASE_HPP

#include "../Database.hpp"
#include <odb/mysql/database.hxx>

namespace das { namespace tpl { namespace mysql
{

  class Database : public das::tpl::Database{
public:
    Database(/*const std::string& db_id,*/
	     const std::string& user,
	     const std::string& password,
	    const std::string& database,
	    const std::string& host = "",
	    int                port = 0)
    {
	db_.reset(new odb::mysql::database(user,password,database,host,port));
//	db_id_ = db_id;
    }
};

}}}
#endif
