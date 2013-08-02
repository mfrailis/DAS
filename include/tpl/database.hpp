#ifndef DAS_DATABASE_HPP
#define DAS_DATABASE_HPP
/* WARNING: this class is not the database-indipendent interface
 * but the mysql related one
 */
#include <iostream>
#include <memory>
#include <odb/tr1/memory.hxx>
#include <odb/traits.hxx>
#include <odb/transaction.hxx>
#include <odb/database.hxx>
#include <odb/session.hxx>

#include "../ddl/types/mysql/aux_query-odb.hxx"

#include "../ddl/info.hpp"
#include "../exceptions.hpp"
#include "../internal/aux_query.hpp"
#include "../internal/qlvisitor.hpp"
#include "../das_object.hpp"
#include "transaction.hpp"
#include "../internal/db_bundle.ipp"
#include "../internal/database_config.hpp"
#include "../internal/result.hpp"

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
            erase(const shared_ptr<T> &obj);

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

            void
            begin_session();

            void
            end_session();

        protected:

            Database(const std::string &db_alias,
                    const shared_ptr<odb::database> &db)
            : bundle_(db_alias, db) {
            }
        private:
            friend class Transaction;
            
            weak_ptr<TransactionBundle> tb_;
            shared_ptr<odb::session> extended_;
            DbBundle bundle_;
            DdlInfo *info_;
        };


    }//namespace tpl
}//namespace das

#include "../internal/database.ipp"
#endif
