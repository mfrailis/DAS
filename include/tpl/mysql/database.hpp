#ifndef DAS_MYSQL_DATABASE_HPP
#define DAS_MYSQL_DATABASE_HPP

#include "../Database.hpp"
#include <odb/mysql/database.hxx>

namespace das { namespace tpl { namespace mysql
{

class Database : public das::tpl::Database
{
public:
    friend shared_ptr<Database> create(const std::string&);

};

}//namespace mysql
}//namespace tpl
}//namespace das
#endif
