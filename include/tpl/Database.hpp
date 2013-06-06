#ifndef DAS_DATABASE_HPP
#define DAS_DATABASE_HPP
/* WARNING: this class is non the database-indipendent interface
 * but the mysql relate one
 */
#include <iostream>
#include <memory>
#include <odb/tr1/memory.hxx>
#include <odb/traits.hxx>
#include <odb/transaction.hxx>
#include <odb/database.hxx>
#include <odb/session.hxx>

#include "../build/dbms/mysql/aux_query-odb.hxx"

#include "ddl/info.hpp"
#include "exceptions.hpp"
#include "../src/cpp/aux_query.hpp"
#include "../src/cpp/ql/qlvisitor.hpp"
#include "DasObject.hpp"
#include "transaction.hpp"
#include "internal/db_bundle.ipp"
#include "internal/database_config.hpp"

#include <odb/mysql/database.hxx>
using std::tr1::shared_ptr;
using std::tr1::weak_ptr;
/*
  template<class T>
  struct object_traits
  {
  static const std::string table_name;
  };
 */

namespace das {
    namespace tpl {

        namespace mysql {
            class Database;
        }

        class Database {
        public:

            static
            shared_ptr<Database>
            create(const std::string& alias);

            Transaction
            begin();

            template<typename T>
            typename odb::object_traits<T>::pointer_type
            load(const typename odb::object_traits<const T>::id_type& id);

            template<typename T>
            typename odb::object_traits<T>::pointer_type
            load(const std::string& name, int version = -1);

            template<typename T>
            typename odb::object_traits<T>::id_type
            persist(typename odb::object_traits<T>::pointer_type& obj, std::string path = "");

            template<typename T>
            void
            erase(T obj); //TODO

            template<typename T>
            odb::result<T>
            query(const std::string& expression, const std::string& ordering = "");

            template<typename T>
            bool
            find(const std::string& name, int version = -1);


            template<typename T>
            void
            attach(typename odb::object_traits<T>::pointer_type& obj);

            void
            flush();

        protected:

            Database(const std::string &db_alias,
                    const shared_ptr<odb::database> &db,
                    const shared_ptr<odb::session> &session)
            : bundle_(db_alias, db, session) {
            }
        private:
            friend class Transaction;

            odb::transaction*
            auto_begin(bool &is_auto);

            void
            auto_catch(bool is_auto, odb::transaction *t);

            void
            auto_commit(bool is_auto, odb::transaction *t);

            DbBundle bundle_;
            DdlInfo *info_;
        };

#include "database.ipp"
    }//namespace tpl
}//namespace das


#endif
