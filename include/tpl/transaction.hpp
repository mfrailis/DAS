#ifndef TRANSACTION_HPP
#define	TRANSACTION_HPP
#include <iostream>
#include <memory>
#include <typeinfo>
#include <odb/tr1/memory.hxx>
#include <odb/transaction.hxx>
#include <odb/database.hxx>
#include <odb/session.hxx>

#include "exceptions.hpp"
#include "interal/db_bundle.hpp"
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
    Transaction(const DbBundle &bundle);
    void commit();
private:
     WeakDbBundle w_bundle_;
     shared_ptr<odb::transaction> transaction_;
};
    
}//namespace tpl
}//namespace das



#endif	/* TRANSACTION_HPP */

