#ifndef DAS_DB_BUNDLE_HPP
#define DAS_DB_BUNDLE_HPP
#include <odb/database.hxx>
#include <odb/session.hxx>
#include <odb/transaction.hxx>
#include <memory>
#include <odb/tr1/memory.hxx>
using std::tr1::shared_ptr;

namespace das
{
  namespace tpl
  {
    typedef struct db_bundle
    {
      shared_ptr<odb::database> db_;
      shared_ptr<odb::session> session_;
    }
      DbBundle;
  }
}

#endif
