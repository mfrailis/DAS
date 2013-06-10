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
#include "internal/result.hpp"

#include <odb/mysql/database.hxx>
using std::tr1::shared_ptr;
using std::tr1::weak_ptr;

namespace das {
    namespace tpl {

        class Database {
        public:
            static
            shared_ptr<Database>
            create(const std::string& alias);

            Transaction
            begin();

            template<typename T>
            shared_ptr<T>
            load(const long long &id);

            template<typename T>
            shared_ptr<T>
            load(const std::string& name, int version = -1);

            template<typename T>
            long long
            persist(const shared_ptr<T> &obj, std::string path = "");

            template<typename T>
            void
            erase(const shared_ptr<T> &obj); //TODO

            template<typename T>
            Result<T>
            query(const std::string& expression, const std::string& ordering = "");

            template<typename T>
            std::vector<long long>
            query_id(const std::string& expression, const std::string& ordering = "");

            template<typename T>
            std::vector< std::pair<std::string, short> >
            query_name(const std::string& expression, const std::string& ordering);

            template<typename T>
            bool
            find(const std::string& name, int version = -1);

            template<typename T>
            void
            attach(const shared_ptr<T> &obj);

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
    }//namespace tpl
}//namespace das

#include "database.ipp"
#endif
