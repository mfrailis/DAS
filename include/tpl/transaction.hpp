#ifndef TRANSACTION_HPP
#define	TRANSACTION_HPP
#include <iostream>
#include <memory>
#include <typeinfo>
#include <odb/tr1/memory.hxx>
#include <odb/transaction.hxx>
#include <odb/database.hxx>
#include <odb/session.hxx>

#include "../exceptions.hpp"
#include "../internal/db_bundle.hpp"
#include "../internal/log.hpp"

using std::tr1::shared_ptr;
using std::tr1::weak_ptr;

namespace das
{
namespace tpl
{

class Database;

class Transaction
{
public:
    Transaction(const DbBundle &b,const shared_ptr<odb::session> &s);
    void commit();
    void rollback();
private:
     shared_ptr<odb::transaction> transaction_;
     shared_ptr<odb::session> session_;
     DbBundle b_;
};
    
}//namespace tpl
}//namespace das



#endif	/* TRANSACTION_HPP */

